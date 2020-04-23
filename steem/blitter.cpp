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

DOMAIN: Emu
FILE: blitter.cpp
DESCRIPTION: High level emulation of the Atari BLiTTER (Bit-Block Transfer
Processor) chip present on the Mega ST (hopefully) and the STE.
When the Blitter uses the bus, the CPU can't and is essentially paralysed.
This limits the speed gains offered by the chip, but the Blitter can also
efficiently manipulate data it blits. The ST Blitter can access the full
address bus range.
The I/O part is in device_map.
Bus arbitration timing between the CPU and the Blitter is rather precisely
emulated, it is needed for correctly rendering some games and demos.
The disk DMA/Blitter arbitration (disk DMA has higher priority) is more
or less emulated, depending on the type of disk image. 
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop
#include <computer.h>
#include <interface_stvl.h>

#define LOGSECTION LOGSECTION_BLITTER

void Blitter_Start_Line();
void Blitter_Blit_Word();
void Blitter_ReadSource(MEM_ADDRESS SrcAdr);
WORD Blitter_DPeek(MEM_ADDRESS ad);
void Blitter_DPoke();
 

void Blitter_CheckRequest() {
  // this function is called by CPU timing functions
  // the latency is probably due to the 'restart' possibility, see
  // iow.cpp
  if(Blitter.Busy) // blit mode, autostart, rare in demos, frequent in GEM
  {
    if(Blitter.Request==1&&!(Blitter.BusAccessCounter&BIT_6)) // ~TMOUT
    {
      Blitter.BusAccessCounter&=0x7F; // counter is 7 bit
      Blitter.Request++;
      Blitter.TimeToSwapBus=ACT+4; // latency!
    }
    else if(Blitter.Request==2&&(ACT-Blitter.TimeToSwapBus>=0))
    {
      Blitter.BusAccessCounter&=0x7F;
#if defined(SSE_DEBUG)
      TRACE_LOG("PC %X F%d y%d c%d AutoBlt Hop%d Op%X %dx%d=%d from %X(%d,%d) to %X(%d,%d) NF%d FX%d Sk%d Msk %X %X %X\n",
        old_pc,TIMING_INFO,Blitter.Hop,Blitter.Op,Blitter.XCount,Blitter.YCount,Blitter.XCount*Blitter.YCount,Blitter.SrcAdr.d32,Blitter.SrcXInc,Blitter.SrcYInc,Blitter.DestAdr.d32,Blitter.DestXInc,Blitter.DestYInc,Blitter.NFSR,Blitter.FXSR,Blitter.Skew,Blitter.EndMask[0],Blitter.EndMask[1],Blitter.EndMask[2]);
#endif
      Blitter_Draw();
    }
  }
  // hog mode + blit mode, start, restart triggered
  else if((ABSOLUTE_CPU_TIME-Blitter.TimeToSwapBus)>=0)
  {
    Blitter.BusAccessCounter&=0x7F;
    if(Blitter.Request==3) // restarting the blit
      Blitter_Draw();
    else
      Blitter_Start_Now(); // starting a blit (init line)
  }
}


void Blitter_Start_Now() {
  Blitter.Request=0;
  Blitter.YCounter=Blitter.YCount;
  if(Blitter.YCounter==0)
    Blitter.YCounter=65536;
  /*Only want to start the line if not in the middle of one.
    Lethal Xcess: blit could be interrupted before writing 1st word! */
  if(!Blitter.LineStarted)
  {
    Blitter_Start_Line();
#ifdef SSE_DEBUG
    Blitter.nWordsBlitted=0;
#endif
  }
  Blitter_Draw();
}


void Blitter_Draw() {
  Blitter.Request=0;
  // because we use the same variables for the bus, not a set for each chip
  MEM_ADDRESS abus_b4_blit=abus;
  WORD dbus_b4_blit=dbus;
  BYTE ds_mask_b4_blit=BUS_MASK;
  if(Blitter.YCount==0)
  { // NO BLIT/BLIT FINISHED
    Blitter.rBusy=Blitter.Busy=Blitter.Hog=0;
#ifdef SSE_DEBUG
    TRACE_LOG("Nothing to blit, %d/%d words blitted, phase %d\n",Blitter.nWordsBlitted,Blitter.nWordsToBlit,Blitter.BlittingPhase);
    Blitter.nWordsBlitted=0;
#endif
#ifdef ENABLE_LOGFILE
    DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" - Blitter_Draw YCount==0 changing GPIP bit from "+
      ((Mfp.reg[MFPR_GPIP]&MFP_GPIP_BLITTER_BIT)!=0)+" to 0");
#endif
    mfp_gpip_set_bit(MFP_GPIP_BLITTER_BIT,0);
    return;
  }
  Blitter.Busy=1;
#ifdef ENABLE_LOGFILE
  DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)
    +" - Blitter_Draw changing GPIP bit from "+
    ((Mfp.reg[MFPR_GPIP]&MFP_GPIP_BLITTER_BIT)!=0)+" to 1");
#endif
  mfp_gpip_set_bit(MFP_GPIP_BLITTER_BIT,true);
  Blitter.TimeAtBlit=ACT; // to record #blit cycles
#if defined(SSE_MEGASTE)
  //MegaSte.MemCache.Reset(); // too slow anyway
  if(IS_MEGASTE) // The Bus Grant signal goes through PAL U002 and U011
    CPU_BUS_IDLE(2); // tests were made with 2 but it could be 4
#endif
  CPU_BUS_IDLE(BLITTER_START_WAIT); // not bus_jam
#ifdef ENABLE_LOGFILE
  DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)+" ------------- BLITTING NOW --------------");
  DBG_LOG(EasyStr("SrcAdr=$")+HEXSl(Blitter.SrcAdr.d32,6)+", SrcXInc="+Blitter.SrcXInc+", SrcYInc="+Blitter.SrcYInc);
  DBG_LOG(EasyStr("DestAdr=$")+HEXSl(Blitter.DestAdr.d32,6)+", DestXInc="+Blitter.DestXInc+", DestYInc="+Blitter.DestYInc);
  DBG_LOG(EasyStr("XCount=")+int(Blitter.XCount?Blitter.XCount:65536)+", YCount="+Blitter.YCount);
  DBG_LOG(EasyStr("Skew=")+Blitter.Skew+", NFSR="+(int)Blitter.NFSR+", FXSR="+(int)Blitter.FXSR);
  DBG_LOG(EasyStr("Hog=")+Blitter.Hog+", Op="+Blitter.Op+", Hop="+Blitter.Hop);
#endif
  Blitter.HasBus=1;
#if defined(SSE_DEBUGGER) 
  // write the BLiT in history
  pc_history_y[pc_history_idx]=scan_y;
  pc_history_c[pc_history_idx]=(short)LINECYCLES;
  pc_history[pc_history_idx++]=0x98764321;
  if(pc_history_idx>=HISTORY_SIZE)
    pc_history_idx=0;
#endif
  while(Blitter.HasBus  DEBUG_ONLY( && runstate==RUNSTATE_RUNNING))
  {
    while(Blitter.HasBus && cpu_cycles>0 
      DEBUG_ONLY( && runstate==RUNSTATE_RUNNING))
    {
      Blitter_Blit_Word();
      if(Blitter.Busy)
      {
        // time to stop?
        if(!Blitter.Hog && (Blitter.BusAccessCounter&BIT_6)) // TMOUT
        {
#ifdef SSE_DEBUG
          TRACE_LOG("Blit paused, %d/%d words blitted, phase %d, xc %d\n",Blitter.nWordsBlitted,Blitter.nWordsToBlit,Blitter.BlittingPhase,Blitter.XCounter);
#endif
          BUS_JAM_TIME(BLITTER_END_WAIT); //arbitration
          Blitter.HasBus=0;
          Blitter.Request=1; // blit not finished
          Blitter.BlitCycles=ACT-Blitter.TimeAtBlit;
          Blitter.BusAccessCounter&=0x7F;
        }
      }
      else // finished
      {
        Blitter.HasBus=0;
        break;
      }
    }
    while(cpu_cycles<=0) 
    {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
      if(TRACE_MASK2&TRACE_CONTROL_EVENT)
        TRACE_EVENT(event_vector);
#endif
      event_vector();
      prepare_next_event();
    }
  }
  // restore bus data
  abus=abus_b4_blit;
  dbus=dbus_b4_blit;
  BUS_MASK=ds_mask_b4_blit;
}


void Blitter_Start_Line() {
  if(Blitter.YCounter<=0) 
  { // Blit finished?
    Blitter.rBusy=Blitter.Hog=Blitter.Busy=Blitter.HasBus=0; // hog bit also reset (BLTBENCH.TOS)
#ifdef SSE_DEBUG
    TRACE_LOG("Blit done, %d/%d words blitted, phase %d\n",Blitter.nWordsBlitted,Blitter.nWordsToBlit,Blitter.BlittingPhase);
    if(Blitter.nWordsBlitted!=Blitter.nWordsToBlit)
      TRACE_LOG("ERROR: %d to blit\n",Blitter.nWordsToBlit);
    //ASSERT(Blitter.nWordsBlitted==Blitter.nWordsToBlit);
    Blitter.nWordsBlitted=0;
#endif
#ifdef ENABLE_LOGFILE
    DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)
      +" - Blitter_Start_Line changing GPIP bit from "
      + ((Mfp.reg[MFPR_GPIP]&MFP_GPIP_BLITTER_BIT)!=0)+" to 0");
#endif
    mfp_gpip_set_bit(MFP_GPIP_BLITTER_BIT,0);
#if defined(SSE_MEGASTE)
    //MegaSte.MemCache.Reset(); // too slow anyway
#endif
    CPU_BUS_IDLE(BLITTER_END_WAIT);
/*  Record # blit cycles during which the CPU could work without
    accessing the bus. More like real emulation, but it has a cost.
    A bit hacky.
*/
    Blitter.BlitCycles=ACT-Blitter.TimeAtBlit;
    Blitter.LineStarted=false;
#ifdef ENABLE_LOGFILE
    DBG_LOG(Str("BLITTER: ")+HEXSl(old_pc,6)
      +" ------------- BLITTING DONE --------------");
#endif
#ifdef DEBUG_BUILD
    if(stop_on_blitter_flag && runstate==RUNSTATE_RUNNING) 
    {
      runstate=RUNSTATE_STOPPING;
      runstate_why_stop="BLiT";
    }
#endif
  }
  else 
  { //prepare next line
    Blitter.Mask=Blitter.EndMask[0]; //SS mask for 1st word
    Blitter.Last=0;
    Blitter.BlittingPhase=(BYTE)((Blitter.FXSR
      &&((Blitter.Op%5)!=0&&(Blitter.Hop>1||(Blitter.Hop==1&&Blitter.Smudge))))
      ? TBlitter::PRIME : TBlitter::READ_SOURCE);
    Blitter.LineStarted=true;
  }
}


void Blitter_Blit_Word() {
  switch(Blitter.BlittingPhase) {
  case TBlitter::PRIME: // = prefetch = FXSR = Force Extra Source Read
    abus=Blitter.SrcAdr.d32;
    BLT_BUS_ACCESS_READ;
    Blitter_ReadSource(abus);
    Blitter.SrcAdr.d32+=Blitter.SrcXInc;
    Blitter.BlittingPhase++;
    break;
  case TBlitter::READ_SOURCE:
    if(Blitter.XCounter==1) // last word
    {
      Blitter.Last=1;
      if(Blitter.XCount>1)
        Blitter.Mask=Blitter.EndMask[2];
    }
    if((Blitter.Op%5)!=0&&(Blitter.Hop>1||(Blitter.Hop==1&&Blitter.Smudge))) 
    {
      if(Blitter.NFSR && Blitter.Last) // NFSR = No Final Source Read
      {
        if(Blitter.SrcXInc>=0)
          Blitter.SrcBuffer<<=16;
        else 
          Blitter.SrcBuffer>>=16;
      }
      if(!Blitter.Last || !Blitter.NFSR || Blitter.XCount==1)
      {
        abus=Blitter.SrcAdr.d32;
        BLT_BUS_ACCESS_READ;
        Blitter_ReadSource(Blitter.SrcAdr.d32);
      }
      if(Blitter.NFSR && Blitter.XCounter==2 // 4 cycles before (doesn't matter)
        || Blitter.Last && !Blitter.NFSR)
        Blitter.SrcAdr.d32+=Blitter.SrcYInc;
      else if(!(Blitter.NFSR && Blitter.XCounter==1))
        Blitter.SrcAdr.d32+=Blitter.SrcXInc;
    }
#if !defined(SSE_LEAN_AND_MEAN)
    Blitter.LineNumber&=0xf;
#endif
    switch(Blitter.Hop) {
    case 0:
      Blitter.SrcDat=0xffff; //fill
      break;
    case 1:
      if(Blitter.Smudge)  //strange but as documented
        Blitter.SrcDat=Blitter.HalfToneRAM[(Blitter.SrcBuffer>>Blitter.Skew)&0xf];
      else
        Blitter.SrcDat=Blitter.HalfToneRAM[Blitter.LineNumber];
      break;
    default:
      Blitter.SrcDat=(WORD)(Blitter.SrcBuffer>>Blitter.Skew);
      if(Blitter.Hop==3) 
      {
        if(Blitter.Smudge==0)
          Blitter.SrcDat&=Blitter.HalfToneRAM[Blitter.LineNumber];
        else
          Blitter.SrcDat&=Blitter.HalfToneRAM[Blitter.SrcDat&0xf];
      }
    }//sw
    Blitter.BlittingPhase++;
    break;
  case TBlitter::READ_DEST:
    Blitter.DestDat=0;
    if(Blitter.NeedDestRead||Blitter.Mask!=0xffff) 
    {
      abus=Blitter.DestAdr.d32;
      BLT_BUS_ACCESS_READ;
      Blitter.DestDat=Blitter_DPeek(Blitter.DestAdr.d32);
      dbus=Blitter.DestDat;
      Blitter.NewDat=Blitter.DestDat & WORD(~(Blitter.Mask));
    }
    else
      Blitter.NewDat=0;
    switch(Blitter.Op) {
    case 0: // 0 0 0 0    - Target will be zeroed out (blind copy)
      Blitter.NewDat|=WORD(0) & Blitter.Mask; 
      break;
    case 1: // 0 0 0 1    - Source AND Target         (inverse copy)
      Blitter.NewDat|=WORD(Blitter.SrcDat & Blitter.DestDat) & Blitter.Mask; 
      break;
    case 2: // 0 0 1 0    - Source AND NOT Target     (mask copy)
      Blitter.NewDat|=WORD(Blitter.SrcDat & ~Blitter.DestDat) & Blitter.Mask; 
      break;
    case 3: // 0 0 1 1    - Source only               (replace copy)
      Blitter.NewDat|=Blitter.SrcDat & Blitter.Mask; 
      break;
    case 4: // 0 1 0 0    - NOT Source AND Target     (mask copy)
      Blitter.NewDat|=WORD(~Blitter.SrcDat & Blitter.DestDat) & Blitter.Mask; 
      break;
    case 5: // 0 1 0 1    - Target unchanged          (null copy)
      Blitter.NewDat|=Blitter.DestDat & Blitter.Mask; 
      break;
    case 6: // 0 1 1 0    - Source XOR Target         (xor copy)
      Blitter.NewDat|=WORD(Blitter.SrcDat ^ Blitter.DestDat) & Blitter.Mask; 
      break;
    case 7: // 0 1 1 1    - Source OR Target          (combine copy)
      Blitter.NewDat|=WORD(Blitter.SrcDat|Blitter.DestDat) & Blitter.Mask; 
      break;
    case 8: // 1 0 0 0    - NOT Source AND NOT Target (complex mask copy)
      Blitter.NewDat|=WORD(~Blitter.SrcDat & ~Blitter.DestDat) & Blitter.Mask; 
      break;
    case 9: // 1 0 0 1    - NOT Source XOR Target     (complex combine copy)
      Blitter.NewDat|=WORD(~Blitter.SrcDat ^ Blitter.DestDat) & Blitter.Mask; 
      break;
    case 10: // 1 0 1 0    - NOT Target                (reverse, no copy)
      Blitter.NewDat=Blitter.DestDat^Blitter.Mask; 
      break;  // ~DestAdr & Blitter.Mask
    case 11: // 1 0 1 1    - Source OR NOT Target      (mask copy)
      Blitter.NewDat|=WORD(Blitter.SrcDat|~Blitter.DestDat) & Blitter.Mask; 
      break;
    case 12: // 1 1 0 0    - NOT Source                (reverse direct copy)
      Blitter.NewDat|=WORD(~Blitter.SrcDat) & Blitter.Mask; 
      break;
    case 13: // 1 1 0 1    - NOT Source OR Target      (reverse combine)
      Blitter.NewDat|=WORD(~Blitter.SrcDat|Blitter.DestDat) & Blitter.Mask; 
      break;
    case 14: // 1 1 1 0    - NOT Source OR NOT Target  (complex reverse copy)
      Blitter.NewDat|=WORD(~Blitter.SrcDat|~Blitter.DestDat) & Blitter.Mask; 
      break;
    case 15: // 1 1 1 1    - Target is set to "1"      (blind copy)
      Blitter.NewDat|=WORD(0xffff) & Blitter.Mask; 
      break;
    }//sw
    Blitter.BlittingPhase++;
    break;
  case TBlitter::WRITE_DEST:
    abus=Blitter.DestAdr.d32;
    dbus=Blitter.NewDat;
    BLT_BUS_ACCESS_WRITE;
    Blitter_DPoke();
#ifdef SSE_DEBUG
    Blitter.nWordsBlitted++;
#endif
    if(Blitter.Last)
      Blitter.DestAdr.d32+=Blitter.DestYInc;
    else
      Blitter.DestAdr.d32+=Blitter.DestXInc;
    Blitter.Mask=Blitter.EndMask[1];
    if((--Blitter.XCounter)<=0) 
    {
      Blitter.LineNumber+=(Blitter.DestYInc>=0) ? 1 : -1;
      Blitter.LineNumber&=0xf;
      Blitter.YCounter--;
      Blitter.YCount=(WORD)Blitter.YCounter;
      Blitter.XCounter=int(Blitter.XCount?Blitter.XCount:65536);  //init blitter for line
      Blitter.LineStarted=false; // line finished
      Blitter_Start_Line();
    }
    if(Blitter.BlittingPhase!=TBlitter::PRIME)
      Blitter.BlittingPhase=TBlitter::READ_SOURCE;
    break;
  default: // recover from bug (old snapshot...)
    Blitter.rBusy=Blitter.BlittingPhase=Blitter.Busy=0;
  }//sw
}


void Blitter_ReadSource(MEM_ADDRESS SrcAdr) {
  if(Blitter.SrcXInc>=0) 
  {
    Blitter.SrcBuffer<<=16; //shift former value to the left
    Blitter.SrcBuffer|=Blitter_DPeek(SrcAdr); //load new value on the right
  }
  else 
  {
    Blitter.SrcBuffer>>=16; //shift former value to the right
    Blitter.SrcBuffer|=Blitter_DPeek(SrcAdr)<<16; //load new value on the left
  }
}


WORD Blitter_DPeek(MEM_ADDRESS ad) {
  ad&=0xfffffe; // 23bit address
  DEBUG_CHECK_READ_W(ad);
  if(ad>=himem) 
  {
    if(ad>=MEM_IO_BASE) 
    {
      WORD RetVal=0xffff;
      TRY_M68K_EXCEPTION
        RetVal=io_read(ad);
      CATCH_M68K_EXCEPTION
        int ncycles=(64+4+2);
        for(int i=0;i<ncycles;i+=2)
        {
          BUS_JAM_TIME(2); // just in case, avoid too long CPU timings
        }
      END_M68K_EXCEPTION
        return RetVal;
    }
    if(ad>=rom_addr && ad<rom_addr+tos_len) 
      return ROM_DPEEK(ad-rom_addr);
    if(cart && ad>=Glue.cartbase && ad<Glue.cartend)
    {
      DWORD cartbus=abus-Glue.cartbase;
      if(Glue.gamecart && cartbus>256*1024)
        cartbus-=(256*1024-64*1024);
      dbus=CART_DPEEK(cartbus);
    }
#if defined(SSE_MMU_MONSTER_ALT_RAM)
    if(ad<Mmu.MonSTerHimem)
      return DPEEK(ad);
#endif
    if(ad<FOUR_MEGS)
    {
      switch(OPTION_VLE) { // reflect MMU/Shifter bus
      case 1: // OPTION_C2
        Mmu.UpdateVideoCounter((short)LINECYCLES);
        if(Mmu.VideoCounter<himem)
          dbus=DPEEK(Mmu.VideoCounter);
        break;
#if defined(SSE_VID_STVL1)
      case 2: // OPTION_C3
        dbus=Stvl.rambus.d16;
        break;
#endif
      }
    }
  }
  else
    return DPEEK(ad);
  return 0xffff;
}


void Blitter_DPoke() {
  abus&=0xfffffe; // 23bit address but can write only to RAM or IO
  if(abus>=MEM_IO_BASE) 
  {
    TRY_M68K_EXCEPTION
      io_write(abus,udbus);
    CATCH_M68K_EXCEPTION
      int ncycles=(64+4+2);
      for(int i=0;i<ncycles;i+=2)
      {
        BUS_JAM_TIME(2);
      }
    END_M68K_EXCEPTION
  }
  else if(abus>=MEM_FIRST_WRITEABLE && abus<himem) 
  {
#if defined(SSE_VID_CHECK_VIDEO_RAM)
/*  If we're going to blit in video RAM of the current scanline,
    we check whether we need to render before. Some programs write
    just after the memory has been fetched, but Steem renders at
    shift mode changes, and if nothing happens, at the end of the line.
    So if we do nothing it will render wrong memory.
    The test isn't perfect and will cause some "false alerts" but
    we have performance in mind: we test for each blitted word, it is rare
    when the address bus is around the current scanline.
    Fixes Appendix checkers (demo is buggy).
    If STVL was released, we could eliminate this part.
*/
    if(OPTION_C2 && abus>=shifter_draw_pointer && abus<
      shifter_draw_pointer_at_start_of_line+LINECYCLES/2 && Glue.FetchingLine())
      Shifter.Render((short)LINECYCLES,DISPATCHER_CPU);
#endif
    DPEEK(abus)=dbus;
    DEBUG_CHECK_WRITE_W(abus);
  }
#if defined(SSE_MMU_MONSTER_ALT_RAM)
  else if(abus<Mmu.MonSTerHimem)
  {
    DEBUG_CHECK_WRITE_W(abus);
    DPEEK(abus)=dbus;
  }
#endif
}
