/*---------------------------------------------------------------------------
PROJECT: Steem SSE
Atari ST emulator
Copyright (C) 2020 by Anthony Hayward and Russel Hayward + SSE

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see https://www.gnu.org/licenses/.

DOMAIN: I/O
FILE: iow.cpp
DESCRIPTION: The 68000 uses memory-mapped I/O
On the ST, addresses from $ff8000 onwards are mapped to peripherals.
They are decoded by the Glue and some other chips like the Blitter.
For performance, we could have different IO functions for STF/STE (as usual
see trade-off with code bloat though, same consideration for one unique
R/W function).
This file handles writing to device registers.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <computer.h>
#include <interface_stvl.h>
#include <stjoy.h>
#include <draw.h>
#include <gui.h>
#include <debug.h>
#include <debug_framereport.h>
#include <loadsave.h>

#define ACIA_CYCLES_NEEDED_TO_START_TX 512 // not OPTION_C1

#define LOGSECTION LOGSECTION_IO

void io_write(MEM_ADDRESS addr, DU16 uio_src_w) {
  WORD &io_src_w=uio_src_w.d16;
  BYTE &hibyte=uio_src_w.d8[HI];
  BYTE &lobyte=uio_src_w.d8[LO];
#ifdef DEBUG_BUILD
  DEBUG_CHECK_WRITE_IO_W(addr,io_src_w);
#endif
  if(!SUPERFLAG 
#ifdef SSE_MMU_MONSTER_ALT_RAM
    && (addr&0xfffffe)!=0xfffe00 // alt-RAM doesn't require supervisor mode
#endif    
    )
    exception(BOMBS_BUS_ERROR,EA_WRITE,addr);

  bool lds=((BUS_MASK&BUS_MASK_LOBYTE)!=0);
  bool uds=((BUS_MASK&BUS_MASK_HIBYTE)!=0);
  act=ACT; // this gets updated in case of waitstates

#if defined(DEBUG_BUILD)
  if(uds)
    log_io_write(addr,hibyte);
  if(lds)
    log_io_write(addr+1,lobyte);
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
  if( (TRACE_MASK_IO&TRACE_CONTROL_IO_W) // iow in control mask browser
      && ( (addr&0xffff00)!=0xFFFA00 || logsection_enabled[LOGSECTION_MFP] ) //mfp
      && ( (addr&0xffff00)!=0xfffc00 || logsection_enabled[LOGSECTION_ACIA] ) //acia
      && ( (addr&0xffff00)!=0xff8600 || logsection_enabled[LOGSECTION_DMA] ) //dma
      && ( (addr&0xffff00)!=0xff8800 || logsection_enabled[LOGSECTION_SOUND] )//psg
      && ( (addr&0xffff00)!=0xff8900 || logsection_enabled[LOGSECTION_SOUND] )//dma
      && ( (addr&0xffff00)!=0xff8a00 || logsection_enabled[LOGSECTION_BLITTER] )
      && ( (addr&0xffff00)!=0xff8200 || logsection_enabled[LOGSECTION_VIDEO] )//shifter
      && ( (addr&0xffff00)!=0xff8000 || logsection_enabled[LOGSECTION_MMU] )) //MMU
#endif
  {
    if(uds&&lds)
      //TRACE_LOG(PRICV " PC %X IOW %06X: %04X\n",act,old_pc,addr,io_src_w);
      TRACE_LOG("PC %X IOW %06X: %04X\n",old_pc,addr,io_src_w);
    else if(uds)
      //TRACE_LOG(PRICV " PC %X IOW %06X: %02X\n",act,old_pc,addr,hibyte);
      TRACE_LOG("PC %X IOW %06X: %02X\n",old_pc,addr,hibyte);
    else if(lds)
      //TRACE_LOG(PRICV " PC %X IOW %06X: %02X\n",act,old_pc,addr+1,lobyte);
      TRACE_LOG("PC %X IOW %06X: %02X\n",old_pc,addr+1,lobyte);
  }
#endif

  // Main switch: address groups
  switch(addr&0xffff00) {

  //////////////////////
  // RAM (MMU CONFIG) //
  //////////////////////

  case 0xff8000:

#undef LOGSECTION
#define LOGSECTION LOGSECTION_MMU

    if(lds && addr==0xff8000) //Memory Configuration
    {
      if(mem_len<=FOUR_MEGS||mem_len==0xC00000)
      {
        Mmu.MemConfig=lobyte&0xF;
        Mmu.bank_length[0]=mmu_bank_length_from_config
          [(Mmu.MemConfig & b1100)>>2];
        Mmu.bank_length[1]= (ST_MODEL==STFM)? Mmu.bank_length[0]:
          mmu_bank_length_from_config[(Mmu.MemConfig & b0011)];
        TRACE_LOG("PC %X write %X to MMU (bank 0: %d bank 1: %d)\n",pc,lobyte,
          mmu_bank_length_from_config[(Mmu.MemConfig & b1100)>>2]/1024,
          mmu_bank_length_from_config[(Mmu.MemConfig & b0011)]/1024);
        Mmu.Confused=false;
        if(SSEConfig.bank_length[0] && Mmu.bank_length[0]!=SSEConfig.bank_length[0])
          Mmu.Confused=true;
        if(SSEConfig.bank_length[1] && Mmu.bank_length[1]!=SSEConfig.bank_length[1])
          Mmu.Confused=true;
        himem=(MEM_ADDRESS)mem_len;
#if defined(SSE_MMU_MONSTER_ALT_RAM)
        if(himem==0xC00000) //12MB monSTer
        {
          himem=FOUR_MEGS;
          Mmu.Confused=false;
        }
#endif
        TRACE_LOG("MMU PC %X Byte %X RAM %dK Bank 0 %d Bank 1 %d confused %d\n",
          old_pc,lobyte,mem_len/1024,SSEConfig.bank_length[0]/1024,SSEConfig.bank_length[1]/1024,Mmu.Confused);
      }
    }
    //forbidden range
    else if(addr>((IS_STE||(ST_MODEL==STF))?0xff800eU:0xff800cU))  
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    break;

#undef LOGSECTION
#define LOGSECTION LOGSECTION_IO

  //////////////////////////////
  // Video (MMU-GLUE-Shifter) //
  //////////////////////////////

  // On the STE, the MMU and the GLUE functions are taken over by the GSTMCU.
  // Its databus is 10bit because the MCU inherits from the GLUE (2bit, UDS)
  // and the MMU (8bit, LDS).
  case 0xff8200: 
    if((addr>=0xff8210&&addr<0xff8240)||addr>=0xff8280)
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    else if(addr>=0xff8240&&addr<0xff8280) // Shifter
    {
      switch(addr) { // Glue registers
      case 0xff8260:
        if(uds) // czietz: if you write a byte on $FF8261, the Glue isn't affected
          Glue.SetShiftMode(hibyte);
        break;
      case 0xff8264:
        if(IS_STE && lds) 
          Glue.hscroll=((io_src_w&0xf)!=0);
        break;
      }//sw
      int shifter_reg=(addr-0xff8240)>>1;
      // Shifter access -> wait states possible
      if(cpu_cycles&3)
      {
        BUS_JAM_TIME(cpu_cycles&3);
      }
      Blitter.BlitCycles=0;
      // The Shifter only knows CS as DS, not LDS and UDS, it works with words.
      // This and the fact that the MC68000 puts the same byte on both the low
      // order and high order of the data bus when writing a single byte 
      // explain some strange video effects (palette, hscroll, shift mode...).
      switch(shifter_reg) {
      case 16:
        Shifter.m_ShiftMode=(hibyte&3);
#if defined(SSE_VID_STVL2)
        Stvl.shift_mode=Shifter.m_ShiftMode;
#endif
#if defined(SSE_HIRES_COLOUR)
        screen_res=Shifter.m_ShiftMode; // mush if 3...
        HIRES_COLOUR=!(Shifter.m_ShiftMode&2)&&(Glue.m_ShiftMode&2);
#endif
        break;
      case 18:
        if(IS_STE)
        {
          BYTE former_hscroll=shifter_hscroll;
          shifter_hscroll=(io_src_w&0xf);
#if defined(SSE_STATS)
          if(shifter_hscroll)
            Stats.nHscroll++;
#endif
          if(!OPTION_C3)
          {
            /*  should new HSCROLL apply on current line
                It's a bit complicated (hacky) because of the "real-time but 
                not quite"rendering. No problem in (more CPU-intensive) STVL.
                Cases to check: Krig, We Were dist, D4/Tekila...
            */
            int cycles_in=(int)(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl);
            if(cycles_in<=Glue.CurrentScanline.StartCycle+24) 
            {
              Glue.AdaptScanlineValues(cycles_in); // ST Magazin
              if(left_border>=BORDER_SIDE)
              { // Don't do this if left border removed!
                left_border=BORDER_SIDE;
                if(HSCROLL)
                  left_border+=16;
                if(Glue.hscroll)
                  left_border-=16;
              }
              // update shifter_pixel for new HSCROLL
              if(shifter_pixel)
              {
                shifter_pixel-=former_hscroll/(1<<screen_res);
                shifter_pixel+=HSCROLL/(1<<screen_res);
              }
            }
          }
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
          if(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_HSCROLL)
            FrameEvents.Add(scan_y,(short)LINECYCLES,(lds)?'H':'h',shifter_hscroll);
#endif
#if defined(SSE_VID_STVL2)
          Stvl.hscroll=shifter_hscroll;
          Stvl.hscroll_complement=16-shifter_hscroll;
#endif
        }
        break;
      default: 
        if(shifter_reg<16) // palette - if not, do nothing
        {
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
          if(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_PAL)
            FrameEvents.Add(scan_y,(short)LINECYCLES,'P',(shifter_reg<<16)|io_src_w);
#endif
          Shifter.SetPal(shifter_reg,io_src_w);
          STpal[shifter_reg]=io_src_w;
          STpal[shifter_reg]&=(IS_STF)?0x777:0xFFF;
        }
        break;
       }//sw
    }
    else if(addr==0xff820a&&uds) // GLUE synchronization mode
    {
      Glue.SetSyncMode(hibyte);
    }
    else if(lds) // MMU video registers
    {
      switch(addr) {
      case 0xff8200:  //high byte of screen memory address
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
        if(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_VIDEOBASE)
          FrameEvents.Add(scan_y,(short)LINECYCLES,'V',lobyte);
#endif
#if defined(SSE_MMU_MONSTER_ALT_RAM)
        if(mem_len<14*0x100000)  // no limit only for 14MB hack
          lobyte&=b00111111;
#else
        if(mem_len<=FOUR_MEGS)
          lobyte&=b00111111;
#endif
        Mmu.u_vbase.d8[B2]=lobyte;
        if(!extended_monitor)
          Mmu.u_vbase.d8[B0]=0;
#if defined(SSE_VID_STVL2)
        Stvl.vbase.d32=vbase;
#endif
#ifdef ENABLE_LOGFILE
        LOG_TO(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Set screen base to "+HEXSl(vbase,6));
#endif
        break;
      case 0xff8202:  //mid byte of screen memory address
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
        if(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_VIDEOBASE)
          FrameEvents.Add(scan_y,(short)LINECYCLES,'M',lobyte);
#endif
        if(!extended_monitor)
        {
          Mmu.u_vbase.d8[B1]=lobyte;
          Mmu.u_vbase.d8[B0]=0;
        }
#if defined(SSE_VID_STVL2)
        Stvl.vbase.d32=vbase;
#endif
#ifdef ENABLE_LOGFILE
        LOG_TO(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Set screen base to "+HEXSl(vbase,6));
#endif
        break;
      case 0xff8206:  //mid byte of video counter
#if defined(SSE_STATS) // we record this one
        if(lobyte)
        {
          Stats.nScreensplit++; 
          Stats.nScreensplit1++; 
        }
        //no break
#endif
      case 0xff8204:  //high byte of video counter          
      case 0xff8208:  //low byte of video counter
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
        if(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_VCOUNT)
          FrameEvents.Add(scan_y,(short)LINECYCLES,'C',(((addr+1)&0xF)<<8)|lobyte);
#endif
        if(!OPTION_C3)
        {
          Mmu.WriteVideoCounter(addr+1,lobyte);
#ifdef ENABLE_LOGFILE
          LOG_TO(LOGSECTION_VIDEO,Str("VIDEO: ")+HEXSl(old_pc,6)+" - Set video counter to "+
            HEXSl(Mmu.VideoCounter,6)+" at "+scanline_cycle_log());
#endif
#if defined(SSE_VID_STVL2)
          Stvl.vcount.d32=Mmu.VideoCounter;
#endif
        }
        break;
      case 0xff820c:  //low byte of screen memory address
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
        if(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_VIDEOBASE)
          FrameEvents.Add(scan_y,(short)LINECYCLES,'v',lobyte);
#endif
        if(IS_STE)
        {
#if defined(SSE_STATS)
          if(lobyte)
            Stats.nVscroll++;
#endif
          Mmu.u_vbase.d8[B0]=(lobyte&0xFE);
#if defined(SSE_VID_STVL2)
          Stvl.vbase.d32=vbase;
#endif
#ifdef ENABLE_LOGFILE
          LOG_TO(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Set screen base to "+HEXSl(vbase,6));
#endif
        }
        break;
      case 0xff820e: // LINEWID
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
        if(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_HSCROLL)
          FrameEvents.Add(scan_y,(short)LINECYCLES,'L',lobyte);
#endif
        if(IS_STE)
        {
          if(!OPTION_C3)
            Shifter.Render((short)LINECYCLES,DISPATCHER_LINEWIDTH); // eg Beat Demo //TODO
          Mmu.linewid=lobyte;
          if(LINECYCLES<Glue.CurrentScanline.EndCycle+MMU_PREFETCH_LATENCY)
            LINEWID=Mmu.linewid;
#if defined(SSE_VID_STVL2)
          Stvl.linewid=lobyte;
#endif
#ifdef ENABLE_LOGFILE
          LOG_TO(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Set Mmu.linewid to "+
            (Mmu.linewid)+" at "+scanline_cycle_log());
#endif
        }
        else if(ST_MODEL!=STF) // crash on STFM, Mega ST
          exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
        break;
      }//sw
    }//if lds
#undef LOGSECTION
#define LOGSECTION LOGSECTION_IO
    break;

  ////////////////////////
  // Disk (MMU-DMA-FDC) //
  ////////////////////////

  case 0xff8600:
  {
    // test for bus error
    if(addr>((IS_STE||(ST_MODEL==STF))?0xff860eU:0xff860cU)||addr<0xff8604U
      ||addr<0xff8608U&&(BUS_MASK&BUS_MASK_WORD)!=BUS_MASK_WORD)
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    switch(addr) {
    case 0xff8604:
      if(Dma.mcr&Dma.CR_COUNT_OR_REGS)
      {
        Dma.Counter=(io_src_w&0xff); // high byte isn't implemented
        if(Dma.Counter)
          Dma.sr|=Dma.SR_COUNT;
        else
          Dma.sr&=BYTE(~Dma.SR_COUNT); //status register bit for 0 count 
        Dma.ByteCount=0;
#ifdef ENABLE_LOGFILE
        LOG_TO(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+" - Set DMA sector count to "+Dma.Counter);
#endif
        break;
      }
      // HD access
      if(Dma.mcr&Dma.CR_HDC_OR_FDC)
      {
#ifdef ENABLE_LOGFILE
        LOG_TO(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+" - Writing $xx"+HEXSl(io_src_w,2)+" to HDC register #"+((Dma.mcr & BIT_1)?1:0));
#endif
#if defined(SSE_ACSI)
        if(ACSI_EMU_ON)
        {
          int device=acsi_dev;
          if(!(Dma.mcr&Dma.CR_A0)&&(io_src_w>>5)<TAcsiHdc::MAX_ACSI_DEVICES)
            device=(io_src_w>>5); // assume new command
          AcsiHdc[device].IOWrite((Dma.mcr&Dma.CR_A0),io_src_w&0xff);
        }
#endif
        break;
      }
      // Write FDC register
      if(Dma.mcr&Dma.CR_DRQ_FDC_OR_HDC)
        Fdc.IOWrite((Dma.mcr&(Dma.CR_A1|Dma.CR_A0))>>1,io_src_w&0xff);
      break;
    case 0xff8606:  //DMA mode
      // detect toggling of bit 8
      if((Dma.mcr&Dma.CR_WRITE)^(io_src_w&Dma.CR_WRITE))
      {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
        if(TRACE_MASK3 & TRACE_CONTROL_FDCREGS)
          TRACE_LOG("DMA Reset\n");
#endif
        Dma.Counter=0;
        Dma.ByteCount=0;
        Dma.sr=Dma.SR_NO_ERROR;
        Dma.Request=false;
        Dma.Fifo_idx=0;
        Dma.BufferInUse=0;
      }
      Dma.mcr=(io_src_w&0x1FF);
      break;
    case 0xff8608:  // DMA Base High - (not limited to 4MB)
      Mmu.uDmaCounter.d16[HI]=(io_src_w&0xff); // high byte=0?
#ifdef ENABLE_LOGFILE
      LOG_TO(LOGSECTION_FDC,EasyStr("FDC: ")+HEXSl(old_pc,6)+" - Set DMA address to "+HEXSl(dma_address,6));
#endif
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
      if(TRACE_MASK3 & TRACE_CONTROL_FDCREGS)
        TRACE_LOG("DMA base address: %X\n",dma_address);
#endif
      break;
    case 0xff860a:  // DMA Base Mid
/*
DMA pointer has to be initialized in order low, mid, high, why
Short answer. Writing to a lower byte might increase (not clear) the upper one.
This will happen when bit 7 (uppermost bit) of the written byte is changed from one to zero.
The reason is how exactly the counters are implemented on Mmu. They use a ripple carry.
This saves quite some logic at the cost of not being fully synchronous.
The upper bit of the lowermost byte, inverted, clocks directly the middle byte of
the counter. Any change from high to low on that bit, anytime, would clock the counter
on the middle byte. Similarly, the inverted uppermost bit of the middle byte clocks the
high byte of the counter.

Actually, the ripple carry is not only across bytes, but across nibbles as well.
But between nibbles doesn't have that collateral effect because both nibbles are written
at the same time, and the written value overrides the ripple carry.

In case you are wondering, the video counter uses the same ripple mechanism. But there
is no such effect because they are read only on the ST.
(ijor)
*/
      if(ST_MODEL==STF&&(dma_address&0x008000)&&!(io_src_w&0x80) // 1 to 0
        &&(!pasti_active && FloppyDrive[DRIVE].ImageType.Extension!=EXT_STX))
      {
        DU16 tmp;
        tmp.d16=Mmu.uDmaCounter.d8[B2]+1;
        io_write(0xff8608,tmp);
      }
      Mmu.uDmaCounter.d8[B1]=(io_src_w&0xff);
      break;
    case 0xff860c:  // DMA Base Low
      if(ST_MODEL==STF&&(dma_address&0x000080)&&!(io_src_w&0x80) // 1 to 0
        &&!pasti_active && FloppyDrive[DRIVE].ImageType.Extension!=EXT_STX)
      { 
        DU16 tmp;
        tmp.d16=Mmu.uDmaCounter.d8[B1]+1;
        io_write(0xff860a,tmp);
      }
      Mmu.uDmaCounter.d8[B0]=(io_src_w&0xfe);
      break;
    case 0xff860e: // frequency/density control
#if defined(SSE_MEGASTE)
      // TOS will check for HD even if DIP switch not set
      MegaSte.FdHd=(IS_MEGASTE) ? (lobyte&0x03) : 0;
#if defined(SSE_DISK_CAPS)
      Caps.fdc.clockfrq=(MegaSte.FdHd&BIT_0) ? (CpuNormalHz*2) : CpuNormalHz;
#endif
#endif
      break; //else ignore
    }//sw
#if USE_PASTI 
/*  Pasti handles all DMA writes, still we want to update our variables
    and go through TRACE.*/
    if(hPasti&&(pasti_active||FloppyDrive[DRIVE].ImageType.Extension==EXT_STX))
    {
      struct pastiIOINFO pioi;
      pioi.stPC=pc; //debug info only
      pioi.cycles=ABSOLUTE_CPU_TIME;
      //    LOG_TO(LOGSECTION_PASTI,Str("PASTI: IO write addr=$")+HEXSl(addr,6)+" data=$"+
      //         HEXSl(io_src_b,2)+" ("+io_src_b+") pc=$"+HEXSl(pc,6)+" cycles="+pioi.cycles);
#if defined(SSE_DISK_GHOST)
      if(OPTION_GHOST_DISK && Fdc.Lines.CommandWasIntercepted
        && addr==0xff8604&&!(Dma.mcr&(BIT_1+BIT_2+BIT_3))) // FDC commands
      {
        TRACE_LOG("Pasti doesn't get command %X\n",Fdc.cr);
      }
      else
#endif
      {
        // pasti expects byte writes to odd addresses
        if(addr<0xff8608)
        {
          pioi.addr=addr;
          pioi.data=io_src_w;
          pasti->Io(PASTI_IOWRITE,&pioi);
        }
        else
        {
          pioi.addr=addr;
          if(BUS_MASK&BUS_MASK_HIBYTE)
          {
            pioi.data=io_src_w>>8;
            pasti->Io(PASTI_IOWRITE,&pioi);
          }
          if(BUS_MASK&BUS_MASK_LOBYTE)
          {
            pioi.addr++;
            pioi.data=(io_src_w&0xFF);
            pasti->Io(PASTI_IOWRITE,&pioi);
          }
        }
        pasti_handle_return(&pioi);
      }
    }
#endif
    break;
  }

  /////////
  // PSG //
  /////////

  case 0xff8800:

    BUS_JAM_TIME(1); // GLUE delays DTACK by one cycle (often rounded up)

    if((addr&BIT_1)==0) //read data / register select
    {
      psg_reg_select=hibyte;
      psg_reg_data=(psg_reg_select<16) ? (psg_reg[psg_reg_select]) : (0xFF);
    }
    else  //write data
    {
      if(psg_reg_select>15)
        break;
      psg_reg_data=hibyte;
      BYTE old_val=psg_reg[psg_reg_select];
      psg_set_reg(psg_reg_select,old_val,hibyte);
#if defined(SSE_DEBUGGER_MUTE_SOUNDCHANNELS)
      if(psg_reg_select<PSGR_AMPLITUDE_A||psg_reg_select>PSGR_AMPLITUDE_C
        ||(!((1<<9)&d2_dpeek(FAKE_IO_START+20)))) // negate vol change (samples)
#endif
      psg_reg[psg_reg_select]=hibyte;
      if(psg_reg_select==PSGR_PORT_A  && (psg_reg[PSGR_MIXER]&BIT_6))
      {
#if USE_PASTI
        if(hPasti)
          pasti->WritePorta(hibyte,ABSOLUTE_CPU_TIME);
#endif
#if defined(SSE_DISK_CAPS)
        if(Caps.Active)
          Caps.WritePsgA(hibyte);
#endif
        SerialPort.SetDTR((hibyte & BIT_4)!=0);
        SerialPort.SetRTS((hibyte & BIT_3)!=0);
#if defined(SSE_DONGLE_JEANNEDARC)
        if(DONGLE_ID==TDongle::JEANNEDARC)
        {
          BYTE Old=Dongle.Value&0xFF;
          BYTE New=(hibyte&(BIT_4|BIT_3));
          Dongle.Value=New;
          mfp_gpip_set_bit(1,!(New && New<Old));
        }
#endif
        Psg.SelectedSide=((hibyte&BIT_0)==0); //0:side 1, 1:side 0
        if((old_val&(BIT_1+BIT_2))!=(hibyte&(BIT_1+BIT_2)))
        {
          Psg.SelectedDrive=floppy_current_drive();
          Dma.UpdateRegs();
          FloppyDrive[0].Motor((Fdc.str&FDC_STR_MO)!=0&&Psg.CurrentDrive()==0);
          FloppyDrive[1].Motor((Fdc.str&FDC_STR_MO)!=0&&Psg.CurrentDrive()==1);
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
          if(TRACE_MASK3 & TRACE_CONTROL_FDCPSG)
            TRACE_FDC("PC %X PSG %X -> %c%d:\n",old_pc,hibyte,'A'+Psg.SelectedDrive,Psg.SelectedSide);
#endif
#ifdef ENABLE_LOGFILE
          if((psg_reg[PSGR_PORT_A]&BIT_1)==0)  //drive 0
          {
            log_to_section(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+" - Set current drive to A:");
          }
          else if((psg_reg[PSGR_PORT_A]&BIT_2)==0)  //drive 1
          {
            log_to_section(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+" - Set current drive to B:");
          }
          else                              //who knows?
          {
            log_to_section(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+" - Unset current drive - guess A:");
          }
#endif
        }//if
#if defined(SSE_DRIVE_FREEBOOT)
        Psg.CheckFreeboot();
#endif
      }
      else if(psg_reg_select==PSGR_PORT_B && (psg_reg[PSGR_MIXER]&BIT_7))
      {
#if defined(SSE_SOUND_CARTRIDGE)
/*  Wings of Death, Lethal Xcess could use the Pro Sound Centronics adapter
    to play 8bit samples on the STF. */
        if(DONGLE_ID==TDongle::PROSOUND)
          mv16_fetch(hibyte<<3);
        else 
#endif
        {
          if(ParallelPort.IsOpen()) 
          {
            if(ParallelPort.OutputByte(hibyte)==0) 
            {
              log_write("ARRRGGHH: Lost printer character, printer not responding!!!!");
              BRK(printer char lost);
            }
//            UpdateCentronicsBusyBit();
          }
          UpdateCentronicsBusyBit();
        }
      }
      else if(psg_reg_select==PSGR_MIXER)
        UpdateCentronicsBusyBit();
    }
    break;

  ///////////////////////
  // STE Digital Sound //
  ///////////////////////

  case 0xff8900:
#undef LOGSECTION
#define LOGSECTION LOGSECTION_SOUND
    if(IS_STF||addr>0xff893e)
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    if((addr&0xff8920)==0xff8920) // Shifter sound registers
    {
      // Shifter access -> wait states possible (new in v4)
      if(cpu_cycles&3)
      {
        BUS_JAM_TIME(cpu_cycles&3);
      }
      Blitter.BlitCycles=0;
      switch(addr) {
      case 0xff8920:   //Sound mode control
        Shifter.sound_set_mode(lobyte);
        break;
      case 0xff8922: // Set MicroWire_Data
      {
#if defined(SSE_STATS)
        Stats.nMicrowire++;
        if(LITTLE_PC>rom_addr)
          Stats.nMicrowireT++;
#endif
        if(OPTION_MICROWIRE
          && abs_quick(ACT-Microwire.StartTime)<MW_LATENCY_CYCLES)
        { // XMas2004-pdx
          TRACE_LOG("%X Microwire write %X at %X denied, %d cycles after write\n",
            old_pc,io_src_w,addr,(int)(ACT-Microwire.StartTime));
          break;
        }
        Microwire.Data=io_src_w;
        Microwire.StartTime=ABSOLUTE_CPU_TIME;
        int dat=Microwire.Data & Microwire.Mask;
        int b;
        for(b=15;b>=10;b--) 
        {
          if( (Microwire.Mask&(3<<(b-1)))==(3<<(b-1)) )
          {
            if((dat & (1<<b))&&(dat & (1<<(b-1)))==0) 
            {
              int dat_b=b-2;
              for(;dat_b>=8;dat_b--) 
              { // Find start of data
                if(Microwire.Mask & (1<<dat_b)) 
                  break;
              }
              dat>>=dat_b-8; // Move 9 highest bits of data to the start
              int nController=(dat>>6) & b0111;
              switch(nController) {
              case b0011: // Master Volume
              case b0101: // Left Volume
              case b0100: // Right Volume
                if(nController==b0011)
                {
                  // 20 is practically silent!
                  Microwire.volume=(dat & b00111111);
                  if(Microwire.volume>47) 
                    Microwire.volume=0; // 47 101111
                  if(Microwire.volume>40) 
                    Microwire.volume=40;
                }
                else 
                {
                  BYTE new_val=(dat & b00011111);
                  if(new_val>23) 
                    new_val=0;
                  if(new_val>20) 
                    new_val=20;
                  if(nController==b0101) 
                    Microwire.volume_l=new_val;
                  if(nController==b0100) 
                    Microwire.volume_r=new_val;
                }
                if(!OPTION_HACKS)
                {
                  Microwire.top_val_l=128;
                  Microwire.top_val_r=128;
                }
                else
                {
                  long double lv,rv,mv;
                  lv=Microwire.volume_l;lv=lv*lv*lv*lv;
                  lv/=(20.0*20.0*20.0*20.0);
                  rv=Microwire.volume_r;rv=rv*rv*rv*rv;
                  rv/=(20.0*20.0*20.0*20.0);
                  mv=Microwire.volume;  mv=mv*mv*mv*mv*mv*mv*mv*mv;
                  mv/=(40.0*40.0*40.0*40.0*40.0*40.0*40.0*40.0);
                  // lv rv and mv are numbers between 0 and 1
                  Microwire.top_val_l=BYTE(128.0*lv*mv);
                  Microwire.top_val_r=BYTE(128.0*rv*mv);
                }
                TRACE_LOG("%X Microwire volume:  master %d L %d R %d\n",
                  old_pc,Microwire.volume,Microwire.volume_l,Microwire.volume_r);
#ifdef ENABLE_LOGFILE
                log_to_section(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound set volume master="+Microwire.volume+
                  " l="+Microwire.volume_l+" r="+Microwire.volume_r);
#endif
                break;
              case b0010: // Treble
                TRACE_LOG("%X DMA snd Treble $%X\n",old_pc,dat);
                dat&=0xF;
                if(dat>0xC)
                  dat=0x6;
                Microwire.treble=(BYTE)dat;
                break;
              case b0001: // Bass
                TRACE_LOG("%X DMA snd Bass $%X\n",old_pc,dat);
                dat&=0xF;
                if(dat>0xC)
                  dat=0x6;
                Microwire.bass=(BYTE)dat;
                break;
              case b0000: // Mixer
              {
                BYTE old=Microwire.mixer;
                Microwire.mixer=(dat&b00000011); // 1=PSG too, anything else only DMA
                if(Microwire.mixer!=old)
                {
                  TRACE_LOG("%X STE SND mixer %X->%X\n",old_pc,old,Microwire.mixer);
                  if(OPTION_SAMPLED_YM)
                    Psg.LoadFixedVolTable(true);
                }
#ifdef ENABLE_LOGFILE
                log_to_section(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound mixer is set to "+Microwire.mixer);
#endif
              }
                break;
              }//sw
            }//if
            break;
          }//if
        }//nxt
        break;
      }
      case 0xff8924:  // Set MicroWire_Mask
        if(OPTION_MICROWIRE
          &&abs_quick(ACT-Microwire.StartTime)<MW_LATENCY_CYCLES)
        {
          TRACE_LOG("Microwire write %X at %X denied, %d cycles after write\n",
            io_src_w,addr,(int)(ACT-Microwire.StartTime));
          break;
        }
        Microwire.Mask=io_src_w;
        break;
      default:
        TRACE_LOG("STE SND %X %X\n",addr,io_src_w);
      }//sw
    }//if
    else if(lds) // MCU sound registers
    {
      switch(addr) {
      case 0xff8900:  //DMA control register
        Mmu.sound_set_control(lobyte);
        break;
      case 0xff8902:   //HiByte of frame start address
      case 0xff8904:   //MidByte of frame start address
      case 0xff8906:   //LoByte of frame start address
        switch(addr&0x6) {
        case 0x2:
          Mmu.u_next_sound_frame_start.d8[B2]=(lobyte&0x3F);
          break;
        case 0x4:
          Mmu.u_next_sound_frame_start.d8[B1]=lobyte;
          break;
        case 0x6:
          Mmu.u_next_sound_frame_start.d8[B0]=(lobyte&0xFE);
          break;
        }//sw
        //TRACE_LOG("DMA frame start %X\n",next_ste_sound_start);
        if((Mmu.sound_control & BIT_0)==0) 
          ste_sound_fetch_address=ste_sound_start=next_ste_sound_start;
#ifdef ENABLE_LOGFILE
        LOG_TO(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound start address set to "+HEXSl(next_ste_sound_start,6));
#endif
        break;
      case 0xff890e:   //HiByte of frame end address
      case 0xff8910:   //MidByte of frame end address
      case 0xff8912:   //LoByte of frame end address
        switch(addr&0xe) {
        case 0xe:
          Mmu.u_next_sound_frame_end.d8[B2]=(lobyte&0x3F);
          break;
        case 0x0:
          Mmu.u_next_sound_frame_end.d8[B1]=lobyte;
          break;
        case 0x2:
          Mmu.u_next_sound_frame_end.d8[B0]=(lobyte&0xFE);
          break;
        }//sw
        //TRACE_LOG("PC %X DMA frame end %X\n",old_pc,next_ste_sound_end);
        if((Mmu.sound_control & BIT_0)==0) 
          ste_sound_end=next_ste_sound_end;
#ifdef ENABLE_LOGFILE
        LOG_TO(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound end address set to "+HEXSl(next_ste_sound_end,6));
#endif
        break;
      }//sw
    }//lds
    break;

  /////////////
  // Blitter //
  /////////////

#undef LOGSECTION
#define LOGSECTION LOGSECTION_BLITTER

  case 0xff8a00:
  {

#ifdef DISABLE_BLITTER
    
    exception(BOMBS_BUS_ERROR,EA_WRITE,addr);

#else

    // Mega ST and STE have a blitter, we don't emulate STF + added blitter
    if(!SSEConfig.Blitter)
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    MEM_ADDRESS Offset=(addr-0xFF8A00)>>1;
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
    if(FRAME_REPORT_MASK2 & FRAME_REPORT_MASK_BLITTER)
      FrameEvents.Add(scan_y,(short)LINECYCLES,'B',((Offset<<16)|io_src_w));
#endif
    if(Offset<0x10)
      Blitter.HalfToneRAM[Offset]=io_src_w;
    else if(Offset<0x1d&&(BUS_MASK&BUS_MASK_WORD)!=BUS_MASK_WORD)
      ; //no write but no bus error
    else switch(Offset) {
      // word access only
    case 0x10:
      Blitter.SrcXInc=(io_src_w&0xFFFE);
#ifdef ENABLE_LOGFILE
      DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter SrcXInc to "+Blitter.SrcXInc);
#endif
      break;
    case 0x11:
      Blitter.SrcYInc=(io_src_w&0xFFFE);
#ifdef ENABLE_LOGFILE
      DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter SrcYInc to "+Blitter.SrcYInc);
#endif
      break;
    case 0x12:
      Blitter.SrcAdr.d8[B2]=lobyte;
      break;
    case 0x13:
      Blitter.SrcAdr.d16[LO]=(io_src_w&0xFFFE);
#ifdef ENABLE_LOGFILE
      DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter SrcAdr to "+HEXSl(Blitter.SrcAdr.d32,6));
#endif
      break;
    case 0x14:
      Blitter.EndMask[0]=io_src_w;
      break;
    case 0x15:
      Blitter.EndMask[1]=io_src_w;
      break;
    case 0x16:
      Blitter.EndMask[2]=io_src_w;
      break;
    case 0x17:
      Blitter.DestXInc=(io_src_w&0xFFFE);
      break;
    case 0x18:
      Blitter.DestYInc=(io_src_w&0xFFFE);
      break;
    case 0x19:
      Blitter.DestAdr.d8[B2]=lobyte;
      break;
    case 0x1a:
      Blitter.DestAdr.d16[LO]=(io_src_w&0xFFFE);
#ifdef ENABLE_LOGFILE
      DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter DestAdr to "+HEXSl(Blitter.DestAdr.d32,6));
#endif
      break;
    case 0x1b:
      Blitter.XCount=io_src_w;
      Blitter.XCounter=Blitter.XCount;
      if(Blitter.XCounter==0)
        Blitter.XCounter=65536;
#ifdef ENABLE_LOGFILE
      DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter XCount to "+Blitter.XCount);      break;
#endif
      break;
    case 0x1c:
      Blitter.YCount=io_src_w;
#ifdef ENABLE_LOGFILE
      DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter YCount to "+Blitter.YCount);
#endif
      break;
      // byte access OK
    case 0x1d:
      if(uds)
      {
        Blitter.Hop=(hibyte & (BIT_0|BIT_1));
#ifdef ENABLE_LOGFILE
        DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter Hop to "+Blitter.Hop);
#endif
      }
      if(lds)
      {
        Blitter.Op=(lobyte&0xF);
        Blitter.NeedDestRead=(Blitter.Op&&(Blitter.Op!=3)&&(Blitter.Op!=12)&&(Blitter.Op!=15));
#ifdef ENABLE_LOGFILE
        DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter Op to "+Blitter.Op);
#endif
      }
      break;
    case 0x1e:
      if(uds)
      {
        Blitter.LineNumber=(hibyte & (BIT_0|BIT_1|BIT_2|BIT_3));
        Blitter.Smudge=((hibyte & BIT_5)!=0); // persistent
        Blitter.Hog=((hibyte & BIT_6)!=0); // volatile
        Blitter.Busy=0; // line
        Blitter.BusAccessCounter=0; // TMOUT reset when Busy line cleared
        if(!Blitter.rBusy) // register
        {
          if(hibyte & BIT_7) 
          { //start new
#if defined(SSE_DEBUG)
            if(Blitter.YCount)
            {
              if(!Blitter.LineStarted)
                Blitter.nWordsToBlit=Blitter.XCount*Blitter.YCount;
              TRACE_LOG("PC %X F%d y%d c%d Blt %X Hop%d Op%X %dx%d=%d from %X(%d,%d) to %X(%d,%d) NF%d FX%d Sk%d Msk %X %X %X\n",
                old_pc,TIMING_INFO,hibyte,Blitter.Hop,Blitter.Op,Blitter.XCount,Blitter.YCount,Blitter.nWordsToBlit,Blitter.SrcAdr.d32,Blitter.SrcXInc,Blitter.SrcYInc,Blitter.DestAdr.d32,Blitter.DestXInc,Blitter.DestYInc,Blitter.NFSR,Blitter.FXSR,Blitter.Skew,Blitter.EndMask[0],Blitter.EndMask[1],Blitter.EndMask[2]);
            }
#endif//dbg
            if(Blitter.YCount)
            {
              Blitter.rBusy=1;
              Blitter.Request=1;
              Blitter.TimeToSwapBus=ACT+BLITTER_LATCH_LATENCY;
#if defined(SSE_STATS)
              Stats.nBlit++;
              Stats.nBlit1++;
              if(Blitter.Hog)
                Stats.nBlith++;
              if(LITTLE_PC>rom_addr)
                Stats.nBlitT++; 
#endif
            }
          }
        }
        else //there's already a blit in progress
        { 
          if(hibyte & BIT_7) // Restart
          { 
 /* busy bit was already set, but by setting it again the blitter starts
    blitting after the same latency as if it was starting for the first time
    possible explanation from schematics (based on sheets 1 and 10 of '4082.pdf'):
    BUSY and hence TMOUT are reset for a while by LINEW when the program
    writes on the register containing BUSY bit 
    = HW trick used by Atari to allow prematurely restarting the blitter
 */
#if defined(SSE_DEBUG)
            TRACE_LOG("PC %X F%d y%d c%d ReBlt %X Hop%d Op%X %dx%d=%d from %X(%d,%d) to %X(%d,%d) NF%d FX%d Sk%d Msk %X %X %X\n",
              old_pc,TIMING_INFO,hibyte,Blitter.Hop,Blitter.Op,Blitter.XCount,Blitter.YCount,Blitter.XCount*Blitter.YCount,Blitter.SrcAdr.d32,Blitter.SrcXInc,Blitter.SrcYInc,Blitter.DestAdr.d32,Blitter.DestXInc,Blitter.DestYInc,Blitter.NFSR,Blitter.FXSR,Blitter.Skew,Blitter.EndMask[0],Blitter.EndMask[1],Blitter.EndMask[2]);
#endif
#ifdef ENABLE_LOGFILE
            DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Blitter restarted - swapping bus to Blitter at "+ABSOLUTE_CPU_TIME);
#endif
            mfp_gpip_set_bit(MFP_GPIP_BLITTER_BIT,false);
            Blitter.Request=3; //3 : use Blit_Draw(), no init
            Blitter.TimeToSwapBus=ACT+BLITTER_LATCH_LATENCY;
          }
          else // Stop
          {    // e.g. Lethal XCess interrupts blit for Timer B that removes border
#ifdef SSE_DEBUG
            TRACE_LOG("Blit stopped, %d/%d words blitted, phase %d, xc %d\n",Blitter.nWordsBlitted,Blitter.nWordsToBlit,Blitter.BlittingPhase,Blitter.XCounter);
#endif
            //TRACE_LOG("PC %X F%d y%d c%d Stop Blit %dx%d, words blitted: %d\n",old_pc,     TIMING_INFO,Blitter.XCounter,Blitter.YCounter,Blitter.nWordsBlitted);
            Blitter.Request=0;
            Blitter.rBusy=0;
#ifdef ENABLE_LOGFILE
            DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Blitter clear busy changing GPIP bit from "+
              ((Mfp.reg[MFPR_GPIP]&MFP_GPIP_BLITTER_BIT)!=0)+" to 0");
#endif
            mfp_gpip_set_bit(MFP_GPIP_BLITTER_BIT,0);
          }
        }
      }//uds
      if(lds)
      {
        Blitter.Skew=(lobyte&0xF); //persistent
        Blitter.NFSR=((lobyte & BIT_6)!=0); // persistent
        Blitter.FXSR=((lobyte & BIT_7)!=0); // persistent
#ifdef ENABLE_LOGFILE
        DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Set blitter Skew to "+Blitter.Skew+", NFSR to "+(int)Blitter.NFSR+", FXSR to "+(int)Blitter.FXSR);
#endif
      }
    case 0x1f:
      break;
    default:
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    }//sw

#endif//DISABLE_BLITTER
    
    break;
  }

  //////////////
  // MEGA STE //
  //////////////

#if defined(SSE_MEGASTE)

  case 0xFF8C00:
    if(IS_MEGASTE && lds && (addr&0x80))
    { // 1 3 5 7 -> 0 2 4 6 -> 0 1 2 3
      int reg=(addr&0x7)>>1;
      MegaSte.Scc[reg]=lobyte; // TODO
    }
    else 
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    break;

  case 0xFF8E00:
    if(IS_MEGASTE && lds && addr<=0xFF8E20)
    {
      switch(addr&0xFF) {
      case 0x00:
        MegaSte.VmeSysMask=lobyte; // VME bus not emulated
        break;
      case 0x02:
        //VmeSysStat is RO
        exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
        break;
      case 0x04:
        MegaSte.VmeSysInt=lobyte;
        break;
      case 0x0c:
        MegaSte.VmeMask=lobyte;
        break;
      case 0x0e:
        //VmeStat is RO
        exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
        break;
      case 0x20:
#undef LOGSECTION
#define LOGSECTION LOGSECTION_MMU
        MegaSte.MemCache.ScuReg=(lobyte&3);
        TRACE_LOG("Cache register %d",MegaSte.MemCache.ScuReg);
#undef LOGSECTION
#define LOGSECTION LOGSECTION_IO
        break;
      }//sw
    }
    else
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    break;

#endif

  //////////////
  // GAMECART //
  //////////////

  case 0xff9000:
    if(IS_STF||addr>0xff9000)
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    if(uds)
    {
      Glue.gamecart=(hibyte&1); // undocumented, spotted by czietz (not tested)
      Glue.cartbase=(Glue.gamecart) ? 0xD80000 : 0xFA0000;
      Glue.cartend=(Glue.gamecart) ? 0xE00000 : 0xFC0000;
    }
    break;

  /////////////////////
  // STE controllers //
  /////////////////////
/*
Joystick 0 and Joystick 2 direction bits are read/write. If written to they will
be driven until a read is performed. Similarly, they will not be driven after a
read until a write is performed. 
*/
  case 0xff9200:
    if(addr==0xff9202 && IS_STE)
      paddles_ReadMask=lobyte;
    else
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    break;

  ///////////////////////////////
  // Falcon 256 colour palette //
  ///////////////////////////////

  case 0xff9800:
  case 0xff9900:
  case 0xff9a00:
  case 0xff9b00:
    if(emudetect_falcon_mode)
    {
      DWORD src=io_src_w&0xFEFE;
      int n=(addr-0xff9800)/4;
      DWORD val=emudetect_falcon_stpal[n];
      if(addr&2)
      {
        val&=0x0000FFFF;
        val|=(src<<16);
      }
      else
      {
        val&=0xFFFF0000;
        val|=src;
      }
      emudetect_falcon_stpal[n]=val;
      emudetect_falcon_palette_convert(n);
    }
    else
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    break;

  //////////////////////////
  // Secret Emu Registers //
  //////////////////////////

/*
  Secret addresses:
    poke byte into FFC123 - stops program running
    poke long into FFC1F0 - logs the string at the specified memory address,
                            which must be null-terminated
*/
  case 0xffc100: //secret Steem registers!
  {
#ifdef DEBUG_BUILD
    if(addr==0xffc122&&lds)
    { //stop
      if(runstate==RUNSTATE_RUNNING)
      {
        runstate=RUNSTATE_STOPPING;
        SET_WHY_STOP("Software break - write to $FFC123");
        break;
      }
    }
    else if(addr==0xffc1f4&&uds)
    {
      logfile_wipe();
      break;
    }
#endif
    if(emudetect_called)
    {
      switch(addr) {
      // 100.l = create disk image
      case 0xffc104:
        if(uds)
          emudetect_reset();
        if(lds)
          new_n_cpu_cycles_per_second=(int)lobyte*1000000;
        break;
      case 0xffc106:
        if(lds)
          snapshot_loaded=(lobyte!=0);
        break;
      case 0xffc108: // Run speed percent
        run_speed_ticks_per_second=100000/MAX((int)(io_src_w),50);
        break;
      case 0xffc11a:
        if(uds)
          emudetect_write_logs_to_printer=(hibyte!=0);
#if !defined(SSE_NO_FALCONMODE)
        if(lds && extended_monitor==0 && screen_res<2 && BytesPerPixel>1)
          emudetect_falcon_mode=BYTE(lobyte);
#endif
        break;
      case 0xffc11c:
        if(uds)
        {
          emudetect_falcon_mode_size=(hibyte&1)+1;
          emudetect_falcon_extra_height=((hibyte&2)!=0);
          // Make sure we don't mess up video memory. It is possible that the height of
          // scanlines is doubled, if we change to 400 with double height lines then arg!
          draw_set_jumps_and_source();
        }
        if(lds)
          emudetect_overscans_fixed=(lobyte!=0);
        break;
      case 0xffc100:
      {
        DWORD ad=m68k_src_l;
        Str Name=read_string_from_memory(ad,500);
        ad+=(int)Name.Length()+1;
        int Param[10]={0,0,0,0,0,0,0,0,0,0};
        Str Num;
        for(int n=0;n<10;n++)
        {
          Num=read_string_from_memory(ad,16);
          if(Num.Length()==0)
            break;
          ad+=(int)Num.Length()+1;
          Param[n]=atoi(Num);
        }
        WORD Sides=2,TracksPerSide=80,SectorsPerTrack=9;
        if(Param[0]==1||Param[0]==2) 
          Sides=(WORD)Param[0];
        if(Param[1]>=10&&Param[1]<=FLOPPY_MAX_TRACK_NUM+1) 
          TracksPerSide=(WORD)Param[1];
        if(Param[2]>=1&&Param[2]<=FLOPPY_MAX_SECTOR_NUM) 
          SectorsPerTrack=(WORD)Param[2];
        GUIEmudetectCreateDisk(Name,Sides,TracksPerSide,SectorsPerTrack);
      }
      break;
      case 0xffc1f0:
#ifdef DEBUG_BUILD
        log_write(Str("ST -- ")+read_string_from_memory(m68k_src_l,500));
#else
        if(emudetect_write_logs_to_printer) {
          // This can't be turned on unless you call emudetect, so 0xffc1f0 will still work normally
          Str Text=read_string_from_memory(m68k_src_l,500);
          for(INT_PTR i=0;i<Text.Length();i++) 
            ParallelPort.OutputByte(Text[i]);
          ParallelPort.OutputByte(13);
          ParallelPort.OutputByte(10);
        }
#endif
        break;
      }//sw
      if(addr<0xffc120)
        break; // No exception!
    }//emudetect
    exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
  }

  /////////
  // MFP //
  /////////

  case 0xfffa00:
  {
    if(addr<0xfffa40) // not arbitrary, enforced by GLU
    {
      BUS_JAM_TIME(4);
      if(lds)
      {
        if(addr<0xfffa30)
        {
          int n=(addr-0xfffa00)>>1;
#ifdef DEBUG_BUILD
          if(stem_runmode==STEM_MODE_CPU)
#endif
          if(cpu_cycles<=0)
          {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
            if(TRACE_MASK2&TRACE_CONTROL_EVENT)
              TRACE_EVENT(event_vector);
#endif
            event_vector(); // event could alter mfp regs + IPL
          }
          Mfp.UpdateNextIrq(act-4); // update first before the write
          if(n==MFPR_GPIP||n==MFPR_AER||n==MFPR_DDR)
          {
            // The output from the AER is eored with the GPIP/input buffer state
            // and that input goes into a 1-0 transition detector. So if the result
            // used to be 1 and now it is 0 an interrupt will occur (if the
            // interrupt is enabled of course).
            BYTE old_gpip=BYTE(Mfp.reg[MFPR_GPIP]&~Mfp.reg[MFPR_DDR]);
            old_gpip|=BYTE(mfp_gpip_input_buffer & Mfp.reg[MFPR_DDR]);
            BYTE old_aer=Mfp.reg[MFPR_AER];
            if(n==MFPR_GPIP)  // Write to GPIP (can only change bits set to 1 in DDR)
            {
              lobyte&=Mfp.reg[MFPR_DDR];
              // Don't change the bits that are 0 in the DDR
              lobyte|=BYTE(mfp_gpip_input_buffer & ~Mfp.reg[MFPR_DDR]);
              mfp_gpip_input_buffer=lobyte;
            }
            else
            {
              Mfp.reg[n]=lobyte;
              // maybe Timer B's AER bit changed?              
              if(OPTION_68901&&n==MFPR_AER)
              {
                if((old_aer&8)!=(lobyte&8))
                {
#if defined(SSE_VID_STVL1)                  
                  if(OPTION_C3)
                    StvlUpdate();
                  else
#endif
                    Mfp.ComputeNextTimerB();
                }
              }
            }
            BYTE new_gpip=BYTE(Mfp.reg[MFPR_GPIP]&~Mfp.reg[MFPR_DDR]);
            new_gpip|=BYTE(mfp_gpip_input_buffer & Mfp.reg[MFPR_DDR]);
            BYTE new_aer=Mfp.reg[MFPR_AER];
            for(int bit=0;bit<8;bit++)
            {
              int irq=mfp_gpip_irq[bit];
              if(mfp_interrupt_enabled[irq])
              {
                BYTE mask=BYTE(1<<bit);
                bool old_1_to_0_detector_input
                  =((old_gpip&mask)^(old_aer&mask))==mask;
                bool new_1_to_0_detector_input
                  =((new_gpip&mask)^(new_aer&mask))==mask;
                if(old_1_to_0_detector_input && new_1_to_0_detector_input==0)
                {
                  // Transition the right way! Set pending (interrupts happen later)
                  if(OPTION_68901)
                    mfp_set_pending(irq,ACT); // update timing, Irq...
                  else
                    // Don't need to call set_pending routine here as this can never
                    // happen soon after an interrupt
                    Mfp.reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)]|=mfp_interrupt_i_bit(irq);
                }//if
              }//if
            }//nxt
          }
          else if(n>=MFPR_IERA && n<=MFPR_IERB)  //enable
          {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
            if(TRACE_MASK_IO & TRACE_CONTROL_IO_W)
              TRACE_MFP(PRICV " PC %X MFP IER%c %X -> %X\n",ACT,old_pc,'A'+n-MFPR_IERA,Mfp.reg[n],lobyte);
#endif
            Mfp.reg[n]=lobyte;
            MFP_CALC_INTERRUPTS_ENABLED;
            for(int t=0;t<4;t++)
            {
              bool new_enabled=(mfp_interrupt_enabled[mfp_timer_irq[t]]
                &&(Mfp.get_timer_control_register(t)&7));
              if(new_enabled && mfp_timer_enabled[t]==0)
              {
                // Timer should have been running but isn't, must put into future
                COUNTER_VAR stage=(mfp_timer_timeout[t]-ABSOLUTE_CPU_TIME);
                if(stage<=0)
                  stage+=((-stage/mfp_timer_period[t])+1)*mfp_timer_period[t];
                else
                  stage%=mfp_timer_period[t];
                mfp_timer_timeout[t]=ABSOLUTE_CPU_TIME+stage;
              }
#ifdef ENABLE_LOGFILE
              if(new_enabled!=mfp_timer_enabled[t])
                LOG_TO(LOGSECTION_MFP_TIMERS,Str("MFP: ")+HEXSl(old_pc,6)+
                  " - Timer "+char('A'+t)+" enabled="+new_enabled);
#endif
              mfp_timer_enabled[t]=new_enabled;
            }
            *Mfp.ipr&=*Mfp.ier; //no pending on disabled registers
            //Mfp.reg[MFPR_IPRA]&=Mfp.reg[MFPR_IERA]; //no pending on disabled registers
            //Mfp.reg[MFPR_IPRB]&=Mfp.reg[MFPR_IERB]; //no pending on disabled registers
          }
          else if(n>=MFPR_IPRA && n<=MFPR_ISRB) //can only clear bits in IPR, ISR
          {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
            if(TRACE_MASK_IO & TRACE_CONTROL_IO_W)
            {
              if(n>=MFPR_IPRA && n<=MFPR_IPRB)
                TRACE_MFP(PRICV " PC %X MFP IPR%c %X -> %X\n",ACT,old_pc,'A'+n-MFPR_IPRA,Mfp.reg[n],Mfp.reg[n]&lobyte);
              else
                TRACE_MFP(PRICV " PC %X MFP ISR%c %X -> %X\n",ACT,old_pc,'A'+n-MFPR_ISRA,Mfp.reg[n],Mfp.reg[n]&lobyte);
            }
#endif
            Mfp.reg[n]&=lobyte;
          }
          else if(n>=MFPR_TADR && n<=MFPR_TDDR) //have to set counter as well as data register
          {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
            if(TRACE_MASK_IO & TRACE_CONTROL_IO_W)
              TRACE_MFP(PRICV " PC %X MFP T%cDR %X -> %X\n",ACT,old_pc,'A'+n-MFPR_TADR,Mfp.reg[n],lobyte);
#endif
            mfp_set_timer_reg(n,Mfp.reg[n],lobyte);
            Mfp.reg[n]=lobyte;
          }
          else if(n==MFPR_TACR||n==MFPR_TBCR) //wipe low-bit on set
          {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
            if(TRACE_MASK_IO & TRACE_CONTROL_IO_W)
              TRACE_MFP(PRICV " PC %X MFP T%cCR %X -> %X\n",ACT,old_pc,'A'+n-MFPR_TACR,Mfp.reg[n],lobyte);
#endif
            lobyte&=BYTE(0xf);
            mfp_set_timer_reg(n,Mfp.reg[n],lobyte);
            Mfp.reg[n]=lobyte;
          }
          else if(n==MFPR_TCDCR)
          {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
            if(TRACE_MASK_IO & TRACE_CONTROL_IO_W)
              TRACE_MFP(PRICV " PC %X MFP TCDCR %X -> %X\n",ACT,old_pc,Mfp.reg[n],lobyte);
#endif
            lobyte&=BYTE(b01110111);
            mfp_set_timer_reg(n,Mfp.reg[n],lobyte);
            Mfp.reg[n]=lobyte;
          }
          else if(n==MFPR_VR)
          {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
            if(TRACE_MASK_IO & TRACE_CONTROL_IO_W)
              TRACE_MFP(PRICV " PC %X MFP VR %X -> %X\n",ACT,old_pc,Mfp.reg[n],lobyte);
#endif
            Mfp.reg[n]=lobyte;
            if(!MFP_S_BIT) // clearing this bit clears In Service registers
              *Mfp.isr=0;
              //Mfp.reg[MFPR_ISRA]=Mfp.reg[MFPR_ISRB]=0;
          }
          else if(n>=MFPR_SCR)
            RS232_WriteReg(n,lobyte);
          else
          {
            //ASSERT(n<16);
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
            if((TRACE_MASK_IO & TRACE_CONTROL_IO_W)&&(n>=MFPR_IMRA && n<=MFPR_IMRB))
              TRACE_MFP(PRICV " PC %X MFP IMR%c %X -> %X (IER%c %X)\n",ACT,old_pc,'A'+n-MFPR_IMRA,Mfp.reg[n],lobyte,'A'+n-MFPR_IMRA,Mfp.reg[MFPR_IERA+n-MFPR_IMRA]);
#endif
            Mfp.reg[n]=lobyte;
          }
          // randomisation for spurious interrupt, we would need subcycle precision?
          COUNTER_VAR t=(OPTION_SPURIOUS)?(act-((act&8)!=0)):act; 
          Mfp.UpdateNextIrq(t);
          prepare_next_event();
        }//if(addr<0xfffa30) 
      }//lds
      else
        exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    }//if(addr<0xfffa40)
    else // beyond allowed range
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    break;
  }

#undef LOGSECTION
#define LOGSECTION LOGSECTION_IO

  ///////////////////////////
  // ACIAs (IKBD and MIDI) //
  ///////////////////////////

  case 0xfffc00:
  case 0xfffd00: // GSTMCU schematics: those addresses all trigger a VPA cycle
#undef LOGSECTION
#define LOGSECTION LOGSECTION_ACIA
  {  
    TMC6850::BusJam();
    int acia_num;
#if defined(SSE_MEGASTF_RTC)
    if(SSEConfig.Mega && lds && addr>=0xFFFC20&&addr<=0xFFFC3E)
      MegaRtc.Write(addr,lobyte);
    else
#endif
    if(uds) 
    {
      switch(addr) {
      case 0xfffc00: // Keyboard ACIA Control
      case 0xfffc04: // MIDI ACIA Control
        acia_num=(addr&0xf)>>2;
        //ASSERT(acia_num==0 || acia_num==1);
        acia[acia_num].cr=hibyte; // no option test
        if((hibyte&3)==3)  // 'Master reset'
        {
#ifdef ENABLE_LOGFILE
          LOG_TO(acia_num?LOGSECTION_MIDI:LOGSECTION_IKBD,Str("ACIA ")+Str(acia_num)+": "+HEXSl(old_pc,6)+" - ACIA reset "); 
#endif
          ACIA_Reset(acia_num,0);
        }
        else
          ACIA_SetControl(acia_num,hibyte); // TOS: $95 for MIDI, $96 for IKBD
        break;
      case 0xfffc02:  // Keyboard ACIA Data
      case 0xfffc06:  // MIDI ACIA Data
        acia_num=((addr&0xf)-2)>>2;
        //ASSERT(acia_num==0 || acia_num==1);
        acia[acia_num].tdr=hibyte;
        TRACE_LOG("ACIA %d PC %X TDR %X\n",acia_num,old_pc,hibyte);
        if(OPTION_C1)
        {
          acia[acia_num].sr&=~BIT_1; // clear TDRE bit
          ACIA_CHECK_IRQ(acia_num); // writing on TDR clears the TX IRQ
          // line was free
          if(!acia[acia_num].LineTxBusy)
          {
            // delay before transmission starts
            // "within 1-bit time of the trailing edge of the Write command"
            int copy2tdr_delay=16*((acia[acia_num].cr&1)?16:64);
            //ASSERT(copy2tdr_delay==1024||copy2tdr_delay==256); // IKBD - MIDI
            acia[acia_num].time_of_event_outgoing=ACT+copy2tdr_delay;
            if(acia[acia_num].time_of_event_outgoing-time_of_event_acia<=0)
              time_of_event_acia=acia[acia_num].time_of_event_outgoing;
            acia[acia_num].LineTxBusy=2; // indicates we're waiting for TDR->TDRS
          }
          break;
        }//option C1
        {
          bool TXEmptyAgenda=(agenda_get_queue_pos(acia_num==ACIA_IKBD?
            agenda_acia_tx_delay_IKBD:agenda_acia_tx_delay_MIDI)>=0);
          if(TXEmptyAgenda==0)
          {
            if(acia[acia_num].tx_irq_enabled)
            {
              acia[acia_num].irq=false;
              mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(acia[ACIA_IKBD].irq || acia[ACIA_MIDI].irq));
            }
            agenda_add(acia_num==ACIA_IKBD?agenda_acia_tx_delay_IKBD
              :agenda_acia_tx_delay_MIDI,
              ACIAClockToHBLS(acia[acia_num].clock_divide),0);
          }
          acia[acia_num].tx_flag=true; //flag for transmitting
          //Steem 3.2, not C1, different paths for IKBD and MIDI:
          if(acia_num==ACIA_IKBD)
          {
            // If send new byte before last one has finished being sent
            if(abs_quick(ABSOLUTE_CPU_TIME-acia[acia_num].last_tx_write_time)
              <ACIA_CYCLES_NEEDED_TO_START_TX)
            {
                // replace old byte with new one
                int n=agenda_get_queue_pos(agenda_ikbd_process);
                if(n>=0)
                {
#ifdef ENABLE_LOGFILE
                  LOG_TO(LOGSECTION_IKBD,Str("IKBD: ")+HEXSl(old_pc,6)+" - Received new command before old one was sent, replacing "+
                    HEXSl(agenda[n].param,2)+" with "+HEXSl(hibyte,2));
#endif
                  agenda[n].param=hibyte;
                }
            }
            else
            {
              // there is a delay before the data gets to the IKBD
              acia[acia_num].last_tx_write_time=ABSOLUTE_CPU_TIME;
              agenda_add(agenda_ikbd_process,IKBD_HBLS_FROM_COMMAND_WRITE_TO_PROCESS,hibyte);
            }
          }
          else
          {
            //ASSERT(acia_num==ACIA_MIDI);
            MIDIPort.OutputByte(hibyte);
          }
          break;
        }
      default:
        break;  //all writes allowed
      }//sw
    }//if
    break;
  }

#undef LOGSECTION
#define LOGSECTION LOGSECTION_IO

  /////////////
  // Alt-RAM //
  /////////////
    
#if defined(SSE_MMU_MONSTER_ALT_RAM)
/*  MonSTer board special register to activate alt-RAM.
    Specify size in megabytes.
*/
  case 0xfffe00: 
  {
    int offset=addr&0xFF;
    if(mem_len!=0xC00000||(bus_mask&BUS_MASK_WORD)!=BUS_MASK_WORD
      ||offset>3) //don't know reg size
      exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
    else if(offset==0)
    {
      if(io_src_w<=8) // max 8MB
        Mmu.MonSTerHimem=(4+io_src_w)*0x100000;
    }
    break;
  }
#endif
  default: //unrecognised
    exception(BOMBS_BUS_ERROR,EA_WRITE,addr);
  }//switch(addr&0xffff00)
}


void io_write_b(MEM_ADDRESS addr,BYTE io_src_b) { 
  // called by d2_poke() (debugger) and PatchPoke() (regular)
  DU16 udb;
  udb.d8[LO]=udb.d8[HI]=io_src_b;
  io_write(addr&0xfffffe,udb);
}


void io_write_w(MEM_ADDRESS addr,WORD io_src_w) {
  // called by d2_dpoke(), d2_lpoke() (debugger) and PatchPoke() (regular)
  DU16 udb;
  udb.d16=io_src_w;
  io_write(addr&0xfffffe,udb);
}
