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
FILE: ior.cpp
DESCRIPTION: The 68000 uses memory-mapped I/O
On the ST, addresses from $ff8000 onwards are mapped to peripherals.
They are decoded by the Glue and some other chips like the Blitter.
For performance, we could have different IO functions for STF/STE (as usual
see trade-off with code bloat though, same consideration for one unique
R/W function).
This file handles reading from device registers.
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


#define LOGSECTION LOGSECTION_IO

WORD io_read(MEM_ADDRESS addr) {
#ifdef DEBUG_BUILD
  DEBUG_CHECK_READ_IO_W(addr);
#endif
  //ASSERT(!(addr&1));
  union {
    WORD d16;
    BYTE d8[2];
  };
  BYTE& hibyte=d8[HI];
  BYTE& lobyte=d8[LO];
  d16=0xffff; //default return value
  //ASSERT((BUS_MASK&BUS_MASK_WRITE)==0);
  //ASSERT(BUS_MASK&BUS_MASK_WORD);

  if(!SUPERFLAG 
#ifdef SSE_MMU_MONSTER_ALT_RAM
    && (addr&0xfffffe)!=0xfffe00 // alt-RAM doesn't require supervisor mode (grr)
#endif
    )
    exception(BOMBS_BUS_ERROR,EA_READ,addr);

  bool lds=((BUS_MASK&BUS_MASK_LOBYTE)==BUS_MASK_LOBYTE);
  bool uds=((BUS_MASK&BUS_MASK_HIBYTE)==BUS_MASK_HIBYTE);
  act=ACT; // this gets updated in case of waitstates

  // Main switch: address groups
  switch(addr&0xffff00) {

  //////////////////////
  // RAM (MMU CONFIG) //
  //////////////////////

  case 0xff8000:

    if(addr==0xff8000)
      lobyte=(mem_len>FOUR_MEGS) ? (BYTE)(MEMCONF_2MB|(MEMCONF_2MB<<2))
        : Mmu.MemConfig;
    else if(addr>((IS_STE||(ST_MODEL==STF)) ? 0xff800eU : 0xff800cU)) //forbidden range
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
    else if(IS_STE) //v402, not tested...
      lobyte=0;
    break;

  //////////////////////////////
  // Video (MMU-GLUE-Shifter) //
  //////////////////////////////

  case 0xff8200:

    if(IS_STE)
    {
      // Below $10 - Odd bytes return value or 0, even bytes return 0xfe/0x7e
      // Above $40 - Unused return 0
      lobyte=0;
      hibyte=(addr>0xff8240) ? 0 : 0xfe; //?
    }
    if(addr>=0xff8240&&addr<0xff8280) // Shifter
    {  
      DWORD shifter_reg=(addr-0xff8240)>>1;
      // Shifter access -> wait states possible
      if(cpu_cycles&3)
      {
        BUS_JAM_TIME(cpu_cycles&3);
      }
      Blitter.BlitCycles=0;
      switch(shifter_reg) {
      case 16:
        hibyte=(Shifter.m_ShiftMode&3);
        if(IS_STF)
          d16|=(0xFCFF&dbus);
        break;
      case 18:
        if(IS_STE)
          d16=shifter_hscroll;
        else
          d16=dbus; // Shifter unused bits replaced with data bus bits
        break;
      default: 
        if(shifter_reg<16) // palette
        {
          d16=STpal[shifter_reg];
          if(IS_STF)
            // Shifter unused bits replaced with data bus bits
            // eg UMD 8730, Wings of Death-SUP
            d16|=(0xF888&dbus);
        }
        else if(IS_STF)
          d16=dbus; 
        break;
      }//sw
    }
    else if(addr>0xff820e&&addr<0xff8240) //forbidden gap
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
    else if(addr>0xff827e) //forbidden area after SHIFTER
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
    else if(addr==0xff820a && uds) // GLUE synchronization mode
    {
      hibyte&=~3;
      hibyte|=Glue.m_SyncMode;
    }
    else if(lds) // MMU video registers
    {
      switch(addr) { 
      case 0xff8200:  //high byte of screen memory address
        lobyte=Mmu.u_vbase.d8[B2];
        break;

      case 0xff8202:  //mid byte of screen memory address
        lobyte=Mmu.u_vbase.d8[B1];
        break;

      case 0xff8204: // video counter high byte
#if defined(SSE_VID_STVL1)       
        if(OPTION_C3)
          lobyte=Stvl.vcount.d8[B2];
        else
#endif          
        {
          DU32 vc;
          vc.d32=Mmu.ReadVideoCounter((short)LINECYCLES);
          lobyte=vc.d8[B2];
        }
#if defined(SSE_STATS)
        Stats.nReadvc++;
        Stats.nReadvc1++;
#endif
#ifdef DEBUG_BUILD
        if(stem_runmode==STEM_MODE_CPU)
        {
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
          if((FRAME_REPORT_MASK1&FRAME_REPORT_MASK_VCOUNT))
            FrameEvents.Add(scan_y,(short)LINECYCLES,'c',(0x0500|lobyte));
#endif
#ifdef ENABLE_LOGFILE
          LOG_TO(LOGSECTION_VIDEO,Str("VIDEO: ")+HEXSl(old_pc,6)+
            " - Read Shifter draw pointer as $"+HEXSl(Mmu.VideoCounter,6)+
            " on "+scanline_cycle_log());
#endif
        }
#endif
        break;
      case 0xff8206: // video counter mid byte
#if defined(SSE_VID_STVL1)       
        if(OPTION_C3)
          lobyte=Stvl.vcount.d8[B1];
        else
#endif          
        {
          DU32 vc;
          vc.d32=Mmu.ReadVideoCounter((short)LINECYCLES);
          lobyte=vc.d8[B1];
        }
#if defined(SSE_STATS)
        Stats.nReadvc++;
        Stats.nReadvc1++;
#endif
#ifdef DEBUG_BUILD
        if(stem_runmode==STEM_MODE_CPU)
        {
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
          if((FRAME_REPORT_MASK1&FRAME_REPORT_MASK_VCOUNT))
            FrameEvents.Add(scan_y,(short)LINECYCLES,'c',(0x0700|lobyte));
#endif
#ifdef ENABLE_LOGFILE
          LOG_TO(LOGSECTION_VIDEO,Str("VIDEO: ")+HEXSl(old_pc,6)+
            " - Read Shifter draw pointer as $"+HEXSl(Mmu.VideoCounter,6)+
            " on "+scanline_cycle_log());
#endif
        }
#endif
        break;
      case 0xff8208: // video counter low byte
#if defined(SSE_VID_STVL1)       
        if(OPTION_C3)
          lobyte=Stvl.vcount.d8[B0];
        else
#endif          
        {
          DU32 vc;
          vc.d32=Mmu.ReadVideoCounter((short)LINECYCLES);
          lobyte=vc.d8[B0];
        }
#if defined(SSE_STATS)
        Stats.nReadvc++;
        Stats.nReadvc1++;
#endif
#ifdef DEBUG_BUILD
        if(stem_runmode==STEM_MODE_CPU)
        {
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
          if((FRAME_REPORT_MASK1&FRAME_REPORT_MASK_VCOUNT))
            FrameEvents.Add(scan_y,(short)LINECYCLES,'c',(0x0900|lobyte));
#endif
#ifdef ENABLE_LOGFILE
          LOG_TO(LOGSECTION_VIDEO,Str("VIDEO: ")+HEXSl(old_pc,6)+
            " - Read Shifter draw pointer as $"+HEXSl(Mmu.VideoCounter,6)+
            " on "+scanline_cycle_log());
#endif
        }
#endif
        break;

      case 0xff820c:  //low byte of screen memory address
        if(IS_STE)
          lobyte=Mmu.u_vbase.d8[B0];
        break;

      case 0xff820e: // LINEWID
        if(IS_STE)
          lobyte=Mmu.linewid;
        else if(ST_MODEL!=STF)
          exception(BOMBS_BUS_ERROR,EA_READ,addr);
        break;
      }//sw
    }//if(lds)
    break;

  ////////////////////////
  // Disk (MMU-DMA-FDC) //
  ////////////////////////

  case 0xff8600: {

    const BYTE &drive=Psg.SelectedDrive;

    // test for bus error (there's a STF/STFM difference)
    if(addr>((IS_STE||(ST_MODEL==STF))?0xff860eU:0xff860cU)||addr<0xff8604U
      ||addr<0xff8608U&&(BUS_MASK&BUS_MASK_WORD)!=BUS_MASK_WORD)
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
    switch(addr) {
    case 0xff8604:
      // sector counter
      if(Dma.mcr&Dma.CR_COUNT_OR_REGS)
      {
        ; // keep ffff?
      }
      // HD access
      else if(Dma.mcr&TDma::CR_HDC_OR_FDC)
      {
#ifdef ENABLE_LOGFILE
#ifdef DEBUG_BUILD
        if(stem_runmode==STEM_MODE_CPU)
          LOG_TO(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+
            " - Reading low byte of HDC register #"+((Dma.mcr&BIT_1)?1:0));
#endif
#endif
#if defined(SSE_ACSI)
        //ASSERT(Dma.mcr&Dma.CR_DRQ_FDC_OR_HDC);
        if(ACSI_EMU_ON)
          lobyte=AcsiHdc[acsi_dev].IORead();
#endif
      }
      else if(!(Dma.mcr&Dma.CR_DRQ_FDC_OR_HDC)) // we can't
      {
        TRACE_LOG("No FDC access DMA MCR %x\n",Dma.mcr);
      }
      // Read FDC register
      else
        lobyte=Fdc.IORead((Dma.mcr&(Dma.CR_A1|Dma.CR_A0))/2);
      break;

    case 0xff8606:  // DMA status
      d16=(Dma.sr&b00000111);
      break;

    case 0xff8608:  // DMA Base and Counter High
      lobyte=Mmu.uDmaCounter.d8[B2];
      break;

    case 0xff860a:  // DMA Base and Counter Mid
      lobyte=Mmu.uDmaCounter.d8[B1];
      break;

    case 0xff860c:  // DMA Base and Counter Low
      lobyte=Mmu.uDmaCounter.d8[B0];
      break;
      
    case 0xff860e: //frequency/density control
#if defined(SSE_MEGASTE)
      if(IS_MEGASTE)
        lobyte=MegaSte.FdHd;
#elif defined(SSE_DISK_HD)
      lobyte=(FloppyDisk[drive].Density==2) ? 3 : 0;
#else
      if(FloppyDisk[drive].STT_File)
        d16=0;
      else
      {
        TFloppyDisk *floppy=&(FloppyDisk[drive]);
        hibyte=BYTE((floppy->BytesPerSector * floppy->SectorsPerTrack)>7000);
        lobyte=0;
      }
#endif
      break;

    }//sw
#if USE_PASTI 
/*  Pasti handles all Dma reads - this cancels the first value
    of ior_byte, but allows to go through TRACE and update our variables..
*/
#if defined(SSE_DISK_GHOST)
    if(!Fdc.Lines.CommandWasIntercepted)
#endif
    {
      if(hPasti&&(pasti_active||FloppyDrive[drive].ImageType.Extension==EXT_STX))
      {
        struct pastiIOINFO pioi;
        pioi.stPC=pc;
        pioi.cycles=ABSOLUTE_CPU_TIME;
        // pasti uses odd addresses for byte access
        if(addr<0xff8608)
        {
          pioi.addr=addr;
          pasti->Io(PASTI_IOREAD,&pioi);
          d16=WORD(pioi.data);
        }
        else
        {
          pioi.addr=addr;
          if((BUS_MASK&BUS_MASK_LOBYTE))
            pioi.addr+=1;
          pasti->Io(PASTI_IOREAD,&pioi);
          if((BUS_MASK&BUS_MASK_WORD)==BUS_MASK_WORD)
            d16=WORD(pioi.data);
          else if((BUS_MASK&BUS_MASK_LOBYTE))
            lobyte=(BYTE)pioi.data;
          else
            hibyte=(BYTE)pioi.data;
        }
        pasti_handle_return(&pioi);
      }
    }
#endif//USE_PASTI 
/*  Media change (changing the floppy disk) on the ST is managed in
    an intricate way, using a timed interrupt to check "write protect"
    status. A change in this status indicates that a disk is being moved
    before the diode in the drive that detects "write protect".    */
    if(floppy_mediach[drive]&&(FloppyDrive[drive].ImageType.Manager==MNGR_PASTI
      ||FloppyDrive[drive].ImageType.Manager==MNGR_CAPS)&&addr==0xFF8604
      &&!(Dma.mcr&Dma.CR_COUNT_OR_REGS)&&!(Dma.mcr & (Dma.CR_A0|Dma.CR_A1)))
    {
      lobyte&=~FDC_STR_WP;
      if(floppy_mediach[drive]/10!=1)
        lobyte|=FDC_STR_WP;
    }
    break;
  }

  /////////
  // PSG //
  /////////

  case 0xff8800:

    BUS_JAM_TIME(1); // GLUE delays DTACK by one cycle (often rounded up)

    if(!(addr & 2))
    { //read data / register select, mirrored at 4,8,12,...
      if(psg_reg_select==PSGR_PORT_A && !(psg_reg[PSGR_MIXER]&BIT_6))
      {
        // Drive A, drive B, side, RTS, DTR, strobe and monitor GPO
        // are normally set by ST
        hibyte=psg_reg[PSGR_PORT_A];
        // Parallel port 0 joystick fire (strobe)
        if(stick[N_JOY_PARALLEL_0]&BIT_4) {
          if(stick[N_JOY_PARALLEL_0]&BIT_7)
            hibyte&=~BIT_5;
          else
            hibyte|=BIT_5;
        }
      }
      else if(psg_reg_select==PSGR_PORT_B && !(psg_reg[PSGR_MIXER]&BIT_7))
      {
        if(!(stick[N_JOY_PARALLEL_0]&BIT_4)
          &&!(stick[N_JOY_PARALLEL_1]&BIT_4)
          &&ParallelPort.IsOpen())
        {
          ParallelPort.NextByte();
          UpdateCentronicsBusyBit();
          hibyte=ParallelPort.ReadByte();
        }
        else
          hibyte=BYTE(0xff&~((stick[N_JOY_PARALLEL_0]&b1111)
            |((stick[N_JOY_PARALLEL_1]&b1111)<<4)));
      }
      else
        hibyte=psg_reg_data;
    }
    break;

  ///////////////////////
  // STE Digital Sound //
  ///////////////////////

  case 0xff8900:

#undef  LOGSECTION
#define LOGSECTION LOGSECTION_SOUND

    if(addr>0xff893e||IS_STF)
      exception(BOMBS_BUS_ERROR,EA_READ,addr);

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
        lobyte=shifter_sound_mode;
        break;
      case 0xff8922:          // MicroWire data
      case 0xff8924:          // MicroWire mask
      {
        WORD dat=0;
        WORD mask=Microwire.Mask;
        if(Microwire.StartTime)
        {
          int nShifts=DWORD(ABSOLUTE_CPU_TIME-Microwire.StartTime)
            /CPU_CYCLES_PER_MW_SHIFT;
          if(nShifts>15)
            Microwire.StartTime=0;
          else
          {
            dat=WORD(Microwire.Data<<nShifts);
            while(nShifts--)
            {
              bool lobit=((mask&BIT_15)!=0);
              mask<<=1;
              mask|=(int)lobit;
            }//wend
          }//if
        }//if
        d16=(addr==0xff8922) ? dat : mask;
        break;
      }//case
      }//sw
    }//if
    else if(lds)
    {
      switch(addr) {
      case 0xff8900:   //DMA control register
#ifdef ENABLE_LOGFILE
#ifdef DEBUG_BUILD
        if(stem_runmode==STEM_MODE_CPU)
#endif
          LOG_TO(LOGSECTION_SOUND,Str("SOUND: ")+HEXSl(old_pc,6)+
            " - Read DMA sound control as $"+HEXSl(Mmu.sound_control,2));
#endif
        lobyte=Mmu.sound_control;
        break;
      case 0xff8902:   //HiByte of frame start address
        lobyte=Mmu.u_next_sound_frame_start.d8[B2];
        break;
      case 0xff8904:   //MidByte of frame start address
        lobyte=Mmu.u_next_sound_frame_start.d8[B1];
        break;
      case 0xff8906:   //LoByte of frame start address
        lobyte=Mmu.u_next_sound_frame_start.d8[B0];
        break;
      case 0xff8908:   //HiByte of frame address counter
        lobyte=Mmu.u_sound_fetch_address.d8[B2];
        break;
      case 0xff890a:   //MidByte of frame address counter
        lobyte=Mmu.u_sound_fetch_address.d8[B1];
        break;
      case 0xff890c:   //LoByte of frame address counter
        lobyte=Mmu.u_sound_fetch_address.d8[B0];
#ifdef DEBUG_BUILD
        if(stem_runmode==STEM_MODE_CPU)
#endif
        TRACE_LOG("F%d Y%d PC%X C%d Read sound frame counter %X (%X->%X)\n",FRAME,scan_y,old_pc,(short)LINECYCLES,ste_sound_fetch_address,ste_sound_start,ste_sound_end);
        break;
      case 0xff890e:   //HiByte of frame end address
        lobyte=Mmu.u_next_sound_frame_end.d8[B2];
        break;
      case 0xff8910:   //MidByte of frame end address
        lobyte=Mmu.u_next_sound_frame_end.d8[B1];
        break;
      case 0xff8912:   //LoByte of frame end address
        lobyte=Mmu.u_next_sound_frame_end.d8[B0];
        break;
      }//sw
    }//if
    break;

  /////////////
  // Blitter //
  /////////////

  case 0xff8a00: 
  {

#ifdef DISABLE_BLITTER

    exception(BOMBS_BUS_ERROR,EA_READ,addr);
    break;

#else

    if(!SSEConfig.Blitter)
    {
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
      break;
    }
    int Offset=(addr-0xFF8A00)>>1;
    // uds or lds or both OK
    if(Offset<0x10)
      d16=Blitter.HalfToneRAM[Offset];
    else switch(Offset) {
    case 0x10:
      d16=Blitter.SrcXInc;
      break;
    case 0x11:
      d16=Blitter.SrcYInc;
      break;
    case 0x12:
      d16=Blitter.SrcAdr.d16[HI];
      break;
    case 0x13:
      d16=Blitter.SrcAdr.d16[LO];
      break;
    case 0x14:
      d16=Blitter.EndMask[0];
      break;
    case 0x15:
      d16=Blitter.EndMask[1];
      break;
    case 0x16:
      d16=Blitter.EndMask[2];
      break;
    case 0x17:
      d16=Blitter.DestXInc;
      break;
    case 0x18:
      d16=Blitter.DestYInc;
      break;
    case 0x19:
      d16=Blitter.DestAdr.d16[HI];
      break;
    case 0x1a:
      d16=Blitter.DestAdr.d16[LO];
      break;
    case 0x1b:
      d16=Blitter.XCount;
      break;
    case 0x1c:
      d16=Blitter.YCount;
      break;
    case 0x1d:
      hibyte=Blitter.Hop;
      lobyte=Blitter.Op;
      break;
    case 0x1e:
      hibyte=(BYTE)((Blitter.LineNumber&0xF)|(Blitter.Smudge<<5)
        |(Blitter.Hog<<6)|(Blitter.rBusy<<7));
      lobyte=(BYTE)(Blitter.Skew|(Blitter.NFSR<<6)|(Blitter.FXSR<<7));
      break;
    case 0x1f:
      d16=0;
      break;
    default:
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
    }//sw
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
    if(FRAME_REPORT_MASK2 & FRAME_REPORT_MASK_BLITTER)
      FrameEvents.Add(scan_y,(short)LINECYCLES,'b',((Offset<<16)|d16));
#endif

#endif//#ifdef DISABLE_BLITTER
    break;
  }//case

  //////////////
  // MEGA STE //
  //////////////

#if defined(SSE_MEGASTE)

  case 0xFF8C00:
    if(IS_MEGASTE && lds && (addr&0x80))
    { // 1 3 5 7 -> 0 2 4 6 -> 0 1 2 3
      int reg=(addr&0x7)>>1;
      lobyte=MegaSte.Scc[reg]; // what was written... TODO
    }
    else 
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
    break;

  case 0xFF8E00:
    if(IS_MEGASTE && lds && addr<=0xFF8E20)
    {
      switch(addr&0xFF) {
      case 0x00:
        lobyte=MegaSte.VmeSysMask; // VME bus not emulated
        break;
      case 0x02:
        lobyte=MegaSte.VmeSysStat;
        break;
      case 0x04:
        lobyte=MegaSte.VmeSysInt;
        break;
      case 0x0c:
        lobyte=MegaSte.VmeMask;
        break;
      case 0x0e:
        lobyte=MegaSte.VmeStat;
        break;
      case 0x20:
        lobyte&=~3;
        lobyte|=(MegaSte.MemCache.ScuReg&3);
        break;
      }//sw
    }
    else 
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
    break;

#endif


  //////////////
  // GAMECART //
  //////////////

  case 0xff9000:
    if(IS_STF||addr>0xff9000)
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
    if(uds)
      hibyte=Glue.gamecart;
    break;

  /////////////////////////////////////////////
  // STE controllers - Mega STE DIP switches //
  /////////////////////////////////////////////

  case 0xff9200:
  {
    bool Illegal=false;
#if defined(SSE_MEGASTE)
    if(IS_MEGASTE)
      d16=0xBFFF; // DIP switches - HD FD enabled is DIP7 bit6
    else
#endif
      d16=JoyReadSTEAddress(addr,&Illegal);
    if(Illegal||IS_STF) // thx Petari
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
    break;
  }

  ///////////////////////////////
  // Falcon 256 colour palette //
  ///////////////////////////////

  case 0xff9800:
  case 0xff9900:
  case 0xff9a00:
  case 0xff9b00:
    if(emudetect_falcon_mode)
    {
      int n=(addr-0xff9800)/4;
      DWORD val=emudetect_falcon_stpal[n];
      d16=(addr&2)?(val>>16):(val&0xffff);
    }
    else
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
    break;

  //////////////////////////
  // Secret Emu Registers //
  //////////////////////////

  case 0xffc100:

#ifdef DEBUG_BUILD
    if(addr==0xffc122)
      d16=(WORD)runstate;
#endif
    if(emudetect_called)
    {
      switch(addr) {
      case 0xffc100:
        hibyte=(stem_version_text[0]-'0');
        {
          Str minor_ver=(char*)stem_version_text+2;
          for(INT_PTR i=0;i<minor_ver.Length();i++) {
            if(minor_ver[i]<'0'||minor_ver[i]>'9') {
              minor_ver.SetLength(i);
              break;
            }
          }
          int ver=atoi(minor_ver.RPad(2,'0'));
          lobyte=BYTE(((ver/10)<<4)|(ver%10));
          break;
        }
        break;
      case 0xffc102:
        hibyte=BYTE(slow_motion);
        lobyte=BYTE(slow_motion_speed/10);
        break;
      case 0xffc104:
        hibyte=BYTE(fast_forward);
        lobyte=BYTE(n_cpu_cycles_per_second/1000000);
        break;
      case 0xffc106:
        hibyte=BYTE(0 DEBUG_ONLY(+1));
        lobyte=snapshot_loaded;
        break;
      case 0xffc108:
        if(run_speed_ticks_per_second)
          d16=(WORD)(100000/run_speed_ticks_per_second);
        break;
      case 0xffc10a:
        if(avg_frame_time && Glue.video_freq)
          d16=(WORD)((((12000/avg_frame_time)*100)/Glue.video_freq));
        break;
      // 32bit - TODO x64?
      case 0xffc10c:
        d16=HIWORD(ABSOLUTE_CPU_TIME);
        break;
      case 0xffc10e:
        d16=LOWORD(ABSOLUTE_CPU_TIME);
        break;
      case 0xffc110:
        d16=HIWORD(cpu_time_of_last_vbl);
        break;
      case 0xffc112:
        d16=(LOWORD(cpu_time_of_last_vbl));
        break;
      case 0xffc114:
        d16=HIWORD(cpu_timer_at_start_of_hbl);
        break;
      case 0xffc116:
        d16=LOWORD(cpu_timer_at_start_of_hbl);
        break;
      case 0xffc118:
        d16=((short)(scan_y));
        break;
      case 0xffc11a:
        hibyte=emudetect_write_logs_to_printer;
        lobyte=emudetect_falcon_mode;
        break;
      case 0xffc11c:
        hibyte=((emudetect_falcon_mode_size-1)+(emudetect_falcon_extra_height?2:0));
        lobyte=emudetect_overscans_fixed;
        break;
      default: 
        d16=0;
      }//sw
      break;
    }//if(emudetect_called)
    exception(BOMBS_BUS_ERROR,EA_READ,addr);

  /////////
  // MFP //
  /////////

  case 0xfffa00:
  {
    if(addr<0xfffa40) {
      BUS_JAM_TIME(4);
      if(lds) 
      {
        if(addr==0xfffa00) 
        {
          lobyte=BYTE(Mfp.reg[MFPR_GPIP]&~Mfp.reg[MFPR_DDR]);
          lobyte|=BYTE(mfp_gpip_input_buffer & Mfp.reg[MFPR_DDR]);
/*  Keep the HDC bit set as long as we deem a real hard disk would
    take to trigger IRQ. We do it this way to avoid adding overhead
    (events, block DMA transfers...), knowing that generally GPIP
    is polled by programs. It is a bit risky.*/
          if(ADAT && ACSI_EMU_ON && AcsiHdc[acsi_dev].Active==2&&!(lobyte&32)
            &&AcsiHdc[acsi_dev].time_of_irq-ACT>0)
            lobyte|=32; // active-low: inactive

#if defined(SSE_DONGLE)
/*  Some dongles modify the GPIP register.
    The dongle for BAT2 on the ST is simplistic. It just permanently changes
    a bit.
    The dongle for Music Master looks more like the dongle for BAT2 on the
    Amiga. The program changes a line and checks the effect on another line
    at different times.
*/
          switch(DONGLE_ID) {
#if defined(SSE_DONGLE_BAT2)
          case TDongle::BAT2:
            lobyte&=~BIT_2; //CTS
            break;
#endif
#if defined(SSE_DONGLE_MUSIC_MASTER)
          case TDongle::MUSIC_MASTER:
          { //inspired by WinUAE
            int bit=(ACT-Dongle.Timing>200)?(Dongle.Value&1):(Dongle.Value&2);
            if(bit)
              lobyte|=BIT_1; //DCD
            else
              lobyte&=~BIT_1;
          }
          break;
#endif
          }//sw
#endif
        }
        else if(addr<0xfffa30)
        {
          int n=(addr-0xfffa00)>>1;
          if(n>=MFPR_TADR && n<=MFPR_TDDR)
          { // timer data registers
            //ASSERT(act==ACT);
            // It seems the MFP IRQ triggers before the counter is updated
            // maybe due to the 0=256 feature?
            // Down-TLN less flicker
            // <4 for Overscan Demos STE, but it can flicker on real STE, just not always
            // TODO test program
            mfp_calc_timer_counter(n-MFPR_TADR,act-3);
            lobyte=BYTE(mfp_timer_counter[n-MFPR_TADR]/64);
            if(n==MFPR_TBDR && Mfp.get_timer_control_register(1)==8)
            { // timer B event count mode
              //TRACE_OSD("READ TB");
              if(!OPTION_68901)
              {
                if((ABSOLUTE_CPU_TIME-time_of_next_timer_b)>4)
                {
                  if(lobyte==0)
                    lobyte=Mfp.reg[MFPR_TBDR];
                  else
                    lobyte--;
                }
              }
              else
              {// tuned with TIMERB07.TOS: Timer B seems to have specific delay
               // also depends on TB_TIME_WOBBLE, it's high-level approximation
                COUNTER_VAR t;
                if(OPTION_C3)
                { // timer B tick is up-to-date but counter could be forward
                  t=act-time_of_next_timer_b;
                  if(t>=0&&t<=8)
                    lobyte=Mfp.tbctr_old;
                }
                else
                { // need to check if timer B should tick
                  t=act-time_of_next_timer_b-8; // We Were boot
                  if(t>=0)
                  {
                    if(!lobyte)
                      lobyte=Mfp.reg[MFPR_TBDR];
                    else
                      lobyte--;
                  }
                  else
                  {// counter too forward?
                    t=act-Mfp.time_of_last_tb_tick;
                    if(t>=0&&t<8)
                      lobyte=Mfp.tbctr_old;
                  }
                }
              }
            }
#ifdef ENABLE_LOGFILE
#ifdef DEBUG_BUILD
            if(stem_runmode==STEM_MODE_CPU)
#endif
              LOG_TO(LOGSECTION_MFP_TIMERS,Str("MFP: ")+HEXSl(old_pc,6)+
              " - Read timer "+char('A'+(n-MFPR_TADR))+" counter as "+lobyte); 
#endif
          }
          else if(n>=MFPR_SCR)
            lobyte=RS232_ReadReg(n);
          else 
            lobyte=Mfp.reg[n];
        }
      }
      else
      { // Even address byte access causes bus error
        DEBUG_ONLY(if(stem_runmode==STEM_MODE_CPU))
        exception(BOMBS_BUS_ERROR,EA_READ,addr);
      }
      break;
    }
    else // max allowed address in range is 0xfffa3f
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
    break;
  }

  ///////////////////////////
  // ACIAs (IKBD and MIDI) //
  ///////////////////////////

  case 0xfffc00:
  case 0xfffd00:
#undef LOGSECTION
#define LOGSECTION LOGSECTION_ACIA
  {
    TMC6850::BusJam();
    if(SSEConfig.Mega&&lds&&addr>=0xFFFC20&&addr<=0xFFFC3E)
    {
#if defined(SSE_MEGASTF_RTC)
      d16=MegaRtc.Read(addr);
#endif
      break;
    }
    else if(uds)
    {
      int acia_num;
      switch(addr) {// ACIA registers
      case 0xfffc00: // Keyboard ACIA Control
      case 0xfffc04: // MIDI ACIA Control
        acia_num=(addr&0xf)>>2;
        //ASSERT(acia_num==0||acia_num==1);
        if(OPTION_C1)
        {
          hibyte=acia[acia_num].sr;
          break;
        }//C1
        hibyte=0;
        if(acia[acia_num].rx_not_read||acia[acia_num].overrun==ACIA_OVERRUN_YES) 
          hibyte|=BIT_0; //full bit
        if(acia[acia_num].tx_flag==0)
          hibyte|=BIT_1; //empty bit
        if(acia[acia_num].irq)
          hibyte|=BIT_7; //irq bit
        if(acia[acia_num].overrun==ACIA_OVERRUN_YES)
          hibyte|=BIT_5; //overrun
        break;
      case 0xfffc02:  // Keyboard ACIA Data
      case 0xfffc06:  // MIDI ACIA Data
        acia_num=((addr&0xf)-2)>>2;
        //ASSERT(acia_num==0||acia_num==1);
#ifdef DEBUG_BUILD
        if(stem_runmode!=STEM_MODE_CPU)
        {
          hibyte=(OPTION_C1)?acia[ACIA_IKBD].rdr:acia[ACIA_IKBD].data;
          break;
        }
#endif
        if(OPTION_C1)
        {
          // Update status BIT 5 (overrun)
          if(acia[acia_num].overrun==ACIA_OVERRUN_COMING) // keep this, it's right
          {
            acia[acia_num].overrun=ACIA_OVERRUN_YES;
            acia[acia_num].sr|=BIT_5; // set overrun (only now, conform to doc)
            TRACE_LOG("%d %d %d PC %X reads ACIA %d RDR %X, OVR\n",TIMING_INFO,old_pc,acia_num,acia[acia_num].rdr);
          }
          // no overrun, normal
          else
          {
            /*"The Overrun indication is reset after the reading of data from the
            Receive Data Register."
            ACIA02.TOS: reading ACIA RDR once after overrun bit is set is enough
            to clear overrun*/
            acia[acia_num].overrun=ACIA_OVERRUN_NO;
            acia[acia_num].sr&=~BIT_0;
            acia[acia_num].sr&=~BIT_5;
            TRACE_LOG("%d %d %d PC %X CPU reads ACIA %d RDR %X\n",TIMING_INFO,old_pc,acia_num,acia[acia_num].rdr);
          }
          ACIA_CHECK_IRQ(acia_num);
          hibyte=acia[acia_num].rdr;
          break;
        }//C1
        {//scope
          acia[acia_num].rx_not_read=0;
#ifdef ENABLE_LOGFILE
          BYTE old_irq=acia[acia_num].irq;
#endif
          if(acia[acia_num].overrun==ACIA_OVERRUN_COMING) 
          {
            acia[acia_num].overrun=ACIA_OVERRUN_YES;
            if(acia[acia_num].rx_irq_enabled) acia[acia_num].irq=true;
            LOG_ONLY(log_to_section(acia_num?LOGSECTION_MIDI:LOGSECTION_IKBD,
              EasyStr("ACIA ")+Str(acia_num)+": "+HEXSl(old_pc,6)+
              " - OVERRUN! Read data ($"+HEXSl(acia[acia_num].data,2)+
              "), changing ACIA IRQ bit from "+old_irq+" to "+acia[acia_num].irq); )
          }
          else 
          {
            acia[acia_num].overrun=ACIA_OVERRUN_NO;
            // IRQ should be off for receive, but could be set for tx empty interrupt
            acia[acia_num].irq=(acia[acia_num].tx_irq_enabled && acia[acia_num].tx_flag==0);
#ifdef ENABLE_LOGFILE
            if(acia[acia_num].irq!=old_irq)
              log_to_section(acia_num?LOGSECTION_MIDI:LOGSECTION_IKBD,
                Str("ACIA ")+Str(acia_num)+": "+HEXSl(old_pc,6)+" - Read data ($"+HEXSl(acia[acia_num].data,2)+
                "), changing ACIA IRQ bit from "+old_irq+" to "+acia[acia_num].irq);
#endif
          }
          mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(acia[ACIA_IKBD].irq||acia[ACIA_MIDI].irq));
          hibyte=acia[acia_num].data;
          break;
        }
        break;
      }
    }
    break;
  }

#undef LOGSECTION

  /////////////
  // Alt-RAM //
  /////////////

#if defined(SSE_MMU_MONSTER_ALT_RAM)
  case 0xfffe00: 
  {
    int offset=addr&0xFF;
    if(mem_len!=0xC00000||(bus_mask&BUS_MASK_WORD)!=BUS_MASK_WORD
      ||offset>8||(offset==8&&!SUPERFLAG))
      exception(BOMBS_BUS_ERROR,EA_READ,addr);
    else switch(offset) {
    case 0:
      d16=(WORD)((Mmu.MonSTerHimem) ? (Mmu.MonSTerHimem/0x100000-4) : 0);
      break;
    case 8:
      d16=1; // "firmware version"
      break;
    default:
      d16=0;
    }//sw
    break;
  }//case
#endif

  default: //not in allowed area
    exception(BOMBS_BUS_ERROR,EA_READ,addr);

  }//sw
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
#define LOGSECTION LOGSECTION_IO
  if(((TRACE_MASK_IO&TRACE_CONTROL_IO_R)
    //&& (old_pc<rom_addr) 
    &&((addr&0xffff00)!=0xFFFA00||logsection_enabled[LOGSECTION_MFP]) //mfp
    &&((addr&0xffff00)!=0xfffc00||logsection_enabled[LOGSECTION_IKBD]) //acia
    &&((addr&0xffff00)!=0xff8600||logsection_enabled[LOGSECTION_DMA]) //dma
    &&((addr&0xffff00)!=0xff8800||logsection_enabled[LOGSECTION_SOUND])//psg
    &&((addr&0xffff00)!=0xff8900||logsection_enabled[LOGSECTION_SOUND])//dma
    &&((addr&0xffff00)!=0xff8a00||logsection_enabled[LOGSECTION_BLITTER])
    &&((addr&0xffff00)!=0xff8200||logsection_enabled[LOGSECTION_VIDEO])//shifter
    &&((addr&0xffff00)!=0xff8000|| (((1<<13)&d2_dpeek(FAKE_IO_START+24))))))//MMU
    //TRACE_LOG(PRICV " PC %X IOR %X : %X\n",ACT,old_pc,addr,d16);
    TRACE_LOG("PC %X IOR %X: %X\n",old_pc,addr,d16);
#endif
  return d16;
}


BYTE io_read_b(MEM_ADDRESS addr) {
  // this is only called by d2_peek()
  DEBUG_CHECK_READ_IO_B(addr);
  BYTE ior_byte;
  DU16 ior_word;
  if(addr&1)
  {
    ior_word.d16=io_read(addr&0xfffffe);
    ior_byte=ior_word.d8[LO];
  }
  else
  {
    ior_word.d16=io_read(addr&0xfffffe);
    ior_byte=ior_word.d8[HI];
  }
  return ior_byte;
}
