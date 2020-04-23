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
FILE: glue.cpp
DESCRIPTION: The Glue (or GLU for General Logic Unit) is an Atari custom
chip with various functions like address decoding, interrupt propagation,
bus arbitration, bus error, video timings.
This file is essentially a high-level emulation of the video timings, and
of "Shifter tricks" manipulating those timings.
Address decoding is handled in the cpu and device_map files.
On the STE, the Glue and the MMU have been merged together, producing the
GSTMCU. In Steem, we do as if they were still separate, STE emulation uses
the same Glue and the Mmu objects, which also have added features (and not
an object derived from both).
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <conditions.h>
#include <draw.h>
#include <gui.h>
#include <emulator.h>
#include <computer.h>
#include <debug.h>
#include <display.h>
#include <parameters.h>
#include <debug_framereport.h>
#include <interface_stvl.h>
#include <osd.h>


#define LOGSECTION LOGSECTION_INTERRUPTS


void update_ipl(COUNTER_VAR when) {
  // The IPL (Interrupt Priority Level) lines go from the GLUE to the CPU.
  // The interrupt priority logic inside the GLUE is instant.
  // This function is called every time IPL could be changed: interrupt pending
  // or cleared, and by the MFP updater.
  BYTE level; // can be only 0, 2, 4, 6 (HW: reversed bits, active-low)
              // Only IPL2 and IPL1 are used, IPL0 is always high
  if(Mfp.Irq)
    level=6; // IPL1 and 2 low
  else if(Glue.vbl_pending)
    level=4; // IPL2 low
  else if(Glue.hbl_pending)
    level=2; // IPL1 low
  else
    level=0;
  if(level!=ipl_timing[ipl_timing_index].ipl) // only real changes
  {
    ipl_timing_index++; // byte 0-255 should overflow
    ipl_timing[ipl_timing_index].ipl=level;
    ipl_timing[ipl_timing_index].time=when;
  }
}


void HBLInterrupt() {
  // Horizontal interrupt, set pending at the end of HSYNC, IPL 2
#ifdef SSE_DEBUG
  log_to_section(LOGSECTION_INTERRUPTS,Str("INTERRUPT: HBL at PC=")+HEXSl(pc,6)+" "+scanline_cycle_log());
  Debug.nHbis++; // knowing how many in the frame is interesting
#if defined(SSE_DEBUGGER_FRAME_INTERRUPTS)
  Debug.FrameInterrupts|=1;
#endif
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
  if(FRAME_REPORT_MASK2 & FRAME_REPORT_MASK_INT)
    FrameEvents.Add(scan_y,(short)LINECYCLES,'I',0x20);
#endif
  //ASSERT(!(scan_y==-63&&LINECYCLES==488));
#if defined(SSE_DEBUGGER_FAKE_IO)
  if(TRACE_MASK2&TRACE_CONTROL_IRQ_SYNC)
    TRACE_LOG(PRICV " (%d %d %d) HBI #%d %d Vec %X\n",
      ACT,TIMING_INFO,Debug.nHbis,LPEEK(0x0068));
#endif
#if defined(SSE_DEBUGGER)
  pc_history_y[pc_history_idx]=scan_y;
  pc_history_c[pc_history_idx]=(short)LINECYCLES;
  pc_history[pc_history_idx++]=0x99000001+(2<<16);
  if(pc_history_idx>=HISTORY_SIZE)
    pc_history_idx=0;
#endif
#endif//dbg
#if defined(SSE_STATS)
  Stats.nHbi++;
#endif
  M68K_UNSTOP;
  Cpu.ProcessingState=TMC68000::EXCEPTION;
  CPU_BUS_IDLE(2); //n
  UPDATE_SR;
  WORD saved_sr=SR; //copy sr
  CPU_BUS_IDLE(4); //nn
  if(!SUPERFLAG) //set S
    change_to_supervisor_mode();
  CLEAR_T;
  PSWI=2; // update ipl mask
  iabus=areg[7]-2;
  dbus=pcl; // stack PC low word;
  CPU_BUS_ACCESS_WRITE; // ns 12
  iabus-=4;
  areg[7]=iabus;
  // start autovector IACK bus cycle
  // e-clock wait-states
  BUS_JAM_TIME(6); //n ni
  if(OPTION_68901)
  {
    BYTE e_clock_wait_states=Cpu.SyncEClock();
    BUS_JAM_TIME(e_clock_wait_states); //ni * ?
  }
  else
  {
    INTERRUPT_START_TIME_WOBBLE; //Steem 3.2
  }
  // trigger event such as scanline in case hbl is pending again: Monaco GP
  // timing of this: European Demos WS1 C2
  while(cpu_cycles<=0)
  {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
    if(TRACE_MASK2&TRACE_CONTROL_EVENT)
      TRACE_EVENT(event_vector);
#endif
    event_vector();
    prepare_next_event();
  }
  BUS_JAM_TIME(4); // ni 
  time_of_last_hbl_interrupt=ABSOLUTE_CPU_TIME; //after wobble or e-clock cycles
  Glue.hbl_pending=false;
  update_ipl(time_of_last_hbl_interrupt);
  CPU_BUS_IDLE(4); //nn
  dbus=saved_sr; // SR written between two parts of PC
  CPU_BUS_ACCESS_WRITE; // ns
  iabus+=2;
  dbus=pch; // PC high word 
  CPU_BUS_ACCESS_WRITE; // nS
  iabus=0x0068;
  CPU_BUS_ACCESS_READ; // nV
  effective_address_h=DPEEK(iabus);
  iabus+=2;
  CPU_BUS_ACCESS_READ; // nv
  effective_address_l=DPEEK(iabus);
  Cpu.ProcessingState=TMC68000::NORMAL;
  m68kSetPC(effective_address,2);
  Glue.m_Status.hbi_done=true;
  debug_check_break_on_irq(BREAK_IRQ_HBL_IDX);
  interrupt_depth++;
}


void VBLInterrupt() {
  // Vertical interrupt, set pending at the end of VSYNC, IPL 4
#ifdef SSE_DEBUG
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
  if(FRAME_REPORT_MASK2 & FRAME_REPORT_MASK_INT)
    FrameEvents.Add(scan_y,(short)LINECYCLES,'I',0x40);
#endif
#if defined(SSE_DEBUGGER_FRAME_INTERRUPTS)
  Debug.FrameInterrupts|=2;
#endif
  log_to_section(LOGSECTION_INTERRUPTS,EasyStr("INTERRUPT: VBL at PC=")
    +HEXSl(pc,6)+" time is "+ABSOLUTE_CPU_TIME
    +" ("+(ABSOLUTE_CPU_TIME-cpu_time_of_last_vbl)+" cycles into screen)");
#if defined(SSE_DEBUGGER_FAKE_IO)
  if(TRACE_MASK2&TRACE_CONTROL_IRQ_SYNC)
#endif
    TRACE_LOG(PRICV " (%d %d %d) ird %X VBI Vec %X sr %X\n",ACT,TIMING_INFO,IRD,
      LPEEK(0x0070),SR);
#if defined(SSE_DEBUGGER) 
  pc_history_y[pc_history_idx]=scan_y;
  pc_history_c[pc_history_idx]=(short)LINECYCLES;
  pc_history[pc_history_idx++]=0x99000001+(4<<16);
  if(pc_history_idx>=HISTORY_SIZE)
    pc_history_idx=0;
#endif
#endif//dbg
#if defined(SSE_STATS)
  Stats.nVbi++;
#endif
  M68K_UNSTOP;
  Cpu.ProcessingState=TMC68000::EXCEPTION;
  CPU_BUS_IDLE(2); //n
  UPDATE_SR;
  WORD saved_sr=SR; //copy sr
  CPU_BUS_IDLE(4); //nn
  if(!SUPERFLAG) //set S
    change_to_supervisor_mode();
  CLEAR_T;
  PSWI=4; // update ipl mask
  iabus=areg[7]-2;
  dbus=pcl; // stack PC low word;
  CPU_BUS_ACCESS_WRITE; // ns 12
  iabus-=4;
  areg[7]=iabus;
  // start autovector IACK bus cycle
  // between 10 and 18 cycles for autovector
  // = 4 + 6 + eclock ws (max 8)
  // e-clock wait-states
  BUS_JAM_TIME(6); //n ni
  if(OPTION_68901)
  {
    BYTE e_clock_wait_states=Cpu.SyncEClock();
    BUS_JAM_TIME(e_clock_wait_states); // ni * ?
  }
  else
  {
    INTERRUPT_START_TIME_WOBBLE; //Steem 3.2
  }
  // trigger event (vbl pending again?)
  while(cpu_cycles<=0)
  {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
    if(TRACE_MASK2&TRACE_CONTROL_EVENT)
      TRACE_EVENT(event_vector);
#endif
    event_vector();
    prepare_next_event();
  }
  BUS_JAM_TIME(4); // ni
  time_of_last_vbl_interrupt=ABSOLUTE_CPU_TIME;
  Glue.vbl_pending=false;
  update_ipl(time_of_last_vbl_interrupt);
  CPU_BUS_IDLE(4); //nn nn
  dbus=saved_sr; // SR written between two parts of PC
  CPU_BUS_ACCESS_WRITE; // ns
  iabus+=2;
  dbus=pch; // PC high word 
  CPU_BUS_ACCESS_WRITE; // nS
  iabus=0x0070;
  CPU_BUS_ACCESS_READ; // nV
  effective_address_h=DPEEK(iabus);
  iabus+=2;
  CPU_BUS_ACCESS_READ; // nv
  effective_address_l=DPEEK(iabus);
  Cpu.ProcessingState=TMC68000::NORMAL;
  m68kSetPC(effective_address,2);
  debug_check_break_on_irq(BREAK_IRQ_VBL_IDX);
  interrupt_depth++;
}

#undef LOGSECTION
#define LOGSECTION LOGSECTION_VIDEO


TGlue::TGlue() {
  ZeroMemory(this,sizeof(TGlue));
  previous_video_freq=video_freq=60; // 0 
  Restore();
  CurrentScanline.Cycles=scanline_time_in_cpu_cycles_8mhz[1];
}


void TGlue::Restore() {
  DE_cycles[2]=160;
  DE_cycles[0]=DE_cycles[1]=DE_cycles[2]<<1; // = 320
  Freq[FREQ_50]=50;
  Freq[FREQ_60]=60;
  Freq[FREQ_71]=MONO_HZ;
  m_ShiftMode&=3; 
  m_SyncMode&=3;
  if(video_freq!=Freq[FREQ_50]&&video_freq!=Freq[FREQ_60]
    &&video_freq!=Freq[FREQ_71])
    video_freq=(screen_res<2) 
      ? ((m_SyncMode&2) ? Freq[FREQ_50] : Freq[FREQ_60]) : Freq[FREQ_71];
}


void TGlue::AdaptScanlineValues(int CyclesIn) { 
  // on set sync or shift mode
  // on IncScanline (CyclesIn=-1)
  //ASSERT(!OPTION_C3);
  if(FetchingLine() && !(CurrentScanline.Tricks&TRICK_0BYTE_LINE))
  {
    //currently in HIRES
    if((m_ShiftMode&2)) 
    {
      if(CyclesIn<=ScanlineTiming[LINE_STOP_LIMIT][FREQ_71])
      {
        CurrentScanline.EndCycle=ScanlineTiming[DE_OFF][FREQ_71];
        if(CyclesIn<=ScanlineTiming[LINE_START_LIMIT][FREQ_71])
        {
          CurrentScanline.StartCycle=ScanlineTiming[DE_ON
            +hscroll][FREQ_71];
        }
      }
    } 
    // not in HIRES
    else if(CyclesIn<=ScanlineTiming[LINE_STOP_LIMIT][FREQ_60] 
      && !(CurrentScanline.Tricks&(TRICK_0BYTE_LINE|TRICK_LINE_MINUS_106
      |TRICK_LINE_PLUS_44|TRICK_LINE_MINUS_2)))
    {
      CurrentScanline.EndCycle=(m_SyncMode&2) ? ScanlineTiming[DE_OFF][FREQ_50]
        :ScanlineTiming[DE_OFF][FREQ_60];
      if(CyclesIn<=ScanlineTiming[LINE_START_LIMIT][FREQ_60]
        && !(CurrentScanline.Tricks
          &(TRICK_LINE_PLUS_26|TRICK_LINE_PLUS_20|TRICK_0BYTE_LINE)))
      {
        CurrentScanline.StartCycle=(m_SyncMode&2) 
          ?ScanlineTiming[DE_ON+hscroll][FREQ_50]
          :ScanlineTiming[DE_ON+hscroll][FREQ_60];
      }
    }
/*  With regular HSCROLL, fetching starts earlier and ends at the same time
    as without scrolling.
    The extra words should be counted at the start of the line, not the end, 
    therefore it should "simply" be part of CurrentScanline.Bytes, but it
    complicates emulation... :)
*/
    if(IS_STE && CyclesIn<=CurrentScanline.StartCycle)
    {
      if(Mmu.ExtraBytesForHscroll)
        CurrentScanline.Bytes-=Mmu.ExtraBytesForHscroll;
      Mmu.ExtraBytesForHscroll=0;
      if(hscroll)
      {
        Mmu.ExtraBytesForHscroll=(m_ShiftMode&2)?2:8-m_ShiftMode*4;
        CurrentScanline.Bytes+=Mmu.ExtraBytesForHscroll;
      } 
    }
    // we can't say =80 or =160 because of various tricks changing those numbers
    // a bit tricky, saves a variable + rewriting
    if(CyclesIn<ScanlineTiming[LINE_STOP_LIMIT][FREQ_71])
    {
      if((m_ShiftMode&2) && !(CurrentScanline.Tricks&TRICK_80BYTE_LINE))
      {
        CurrentScanline.Bytes-=80;
        CurrentScanline.Tricks|=TRICK_80BYTE_LINE; // each line in HIRES
      }
      else if(!(m_ShiftMode&2) && (CurrentScanline.Tricks&TRICK_80BYTE_LINE))
      {
        CurrentScanline.Bytes+=80;
        CurrentScanline.Tricks&=~TRICK_80BYTE_LINE;
      }
    }
  }
  if(CyclesIn<=cycle_of_scanline_length_decision)
  {
    CurrentScanline.Cycles=scanline_time_in_cpu_cycles
      [((m_ShiftMode&2)&&((CyclesIn==-1)||PreviousScanline.Cycles!=224))
        ? 2 : ( (m_SyncMode&2)!=2  )];
    prepare_next_event();
  }
  // call it when .Cycles <>0
  if(OPTION_68901)
     Mfp.ComputeNextTimerB();
}


void TGlue::CheckSideOverscan() {
/*  Various GLU and Shifter tricks can change the border size and the number 
    of bytes fetched from video RAM. 
    Those tricks can be used with two goals: use a larger display area
    (fullscreen demos are more impressive than borders-on demos), and/or
    scroll the screen by using "sync lines" (eg Enchanted Land, No Buddies Land).
    This function is a big extension of Steem's original draw_check_border_removal()
    with some additions inspired by Hatari v1.6 in Steem SSE v3.4 and other
    additions later (some my own R&D!)
*/
  int t=0;
  short CyclesIn=(short)LINECYCLES;
  short r0cycle=-1,r1cycle,r2cycle;

  /////////////
  // NO SYNC //
  /////////////

  if(m_SyncMode&1) // we emulate no genlock -> nothing displayed
    CurrentScanline.Tricks=TRICK_0BYTE_LINE;

  //////////////////////
  // HIGH RES EFFECTS //
  //////////////////////

/*  Monoscreen by Dead Braincells
    012:R0000 020:R0002                       -> 0byte
    024:R0000 032:R0002 164:R0000 192:R0002   -> right off, left off
*/
  if(screen_res==2) 
  {
    if(!freq_change_this_scanline)
      return;
    char fetched_bytes_mod=0;
    if(!(CurrentScanline.Tricks&TRICK_0BYTE_LINE)
      && CyclesIn>=ScanlineTiming[LINE_START_LIMIT][FREQ_71]
      && !(ShiftModeAtCycle(ScanlineTiming[LINE_START_LIMIT][FREQ_71]) & 2))
    {
      CurrentScanline.Tricks|=TRICK_0BYTE_LINE;
      fetched_bytes_mod=-80; // hack
      draw_line_off=true;
    }
    else if( !(CurrentScanline.Tricks&2) 
      && CyclesIn>=ScanlineTiming[LINE_STOP_LIMIT][FREQ_71]
      && !(ShiftModeAtCycle(ScanlineTiming[DE_OFF][FREQ_71])&2))
    {
      CurrentScanline.Tricks|=TRICK_HIRES_OVERSCAN;
      fetched_bytes_mod=14;
    }
    shifter_draw_pointer+=fetched_bytes_mod;
    CurrentScanline.Bytes+=fetched_bytes_mod;
    return;
  }

  /////////////////
  // 0-BYTE LINE //
  /////////////////

/*  Various shift mode or sync mode switches trick the GLUE into passing a 
    scanline in video RAM while the monitor is still displaying a line.
    This is a way to implement "hardware" downward vertical scrolling 
    on a computer where it's not foreseen (No Buddies Land).
    0-byte lines can also be combined with other sync lines (Forest, etc.).

    Normal lines:

    Video RAM
    [][][][][][][][][][][][][][][][] (1)
    [][][][][][][][][][][][][][][][] (2)
    [][][][][][][][][][][][][][][][] (3)
    [][][][][][][][][][][][][][][][] (4)


    Screen
    [][][][][][][][][][][][][][][][] (1)
    [][][][][][][][][][][][][][][][] (2)
    [][][][][][][][][][][][][][][][] (3)
    [][][][][][][][][][][][][][][][] (4)


    0-byte line:

    Video RAM
    [][][][][][][][][][][][][][][][] (1)
    [][][][][][][][][][][][][][][][] (2)
    [][][][][][][][][][][][][][][][] (3)
    [][][][][][][][][][][][][][][][] (4)


    Screen
    [][][][][][][][][][][][][][][][] (1)
    -------------------------------- (0-byte line)
    [][][][][][][][][][][][][][][][] (2)
    [][][][][][][][][][][][][][][][] (3)
    [][][][][][][][][][][][][][][][] (4)
*/

  if(!(CurrentScanline.Tricks&(TRICK_0BYTE_LINE|TRICK_LINE_PLUS_26
    |TRICK_LINE_PLUS_20|TRICK_80BYTE_LINE|TRICK_LINE_PLUS_24
    |TRICK_LINE_PLUS_44)))
  {

/*  Test previous scanline for 0byte. Must do it here because timings
    can go beyond 512 -> interference with scanline routines. 
    We use the values in LJBK's table, taking care not to break
    emulation of other cases.
    When DE has been negated and the GLU misses HSYNC due to some trick,
    the GLU will fail to trigger next line, until next HSYNC.
    Does the monitor miss one HSYNC too? Anyway, the beam goes down.
    shift mode:
    No line 1 460-472 there's no HSYNC ON, no HBL interrupt
    Beyond/Pax Plax Parallax STF
    No line 2 474-512 there's no HSYNC OFF, no HBL interrupt
    No Buddies Land, Pulsion 172 WS2 (unaligned)
*/
    if((!(PreviousScanline.Tricks&TRICK_LINE_PLUS_44)
      &&ShiftModeAtCycle(ScanlineTiming[HSYNC_ON2][FREQ_50])&2) ||
      (CyclesIn>=ScanlineTiming[HSYNC_OFF2][FREQ_50] 
      && (ShiftModeAtCycle(ScanlineTiming[HSYNC_OFF2][FREQ_50])&2)))
    {
      CurrentScanline.Tricks|=TRICK_0BYTE_LINE;
      VCount--;
      video_last_draw_line++; // TODO scan_y-- is worse
    }
/*  We test for 0byte line at the start of the scanline, affecting
    current scanline.

    Two timings may be targeted.

    HIRES HSYNC ON: premature HSYNC, DE is never asserted
    (ljbk) 0byte line   16-40 on STF
    Nostalgia/Lemmings STF

    LORES DE ON: if mode/frequency isn't right at the timing, the line won't start.
    This can be done by shift or sync mode switches.
    Nostalgia/Lemmings STE
    Forest
*/
    else if(CyclesIn>=ScanlineTiming[LINE_START_LIMIT_PLUS26][FREQ_71]  // removed if STF ...
      && (ShiftModeAtCycle(ScanlineTiming[LINE_START_LIMIT_PLUS26][FREQ_71])&2))
      CurrentScanline.Tricks|=TRICK_0BYTE_LINE;
    else if(CyclesIn>=ScanlineTiming[LINE_START_LIMIT][FREQ_50] 
      && FreqAtCycle(ScanlineTiming[LINE_START_LIMIT][FREQ_50])!=50 
      && FreqAtCycle(ScanlineTiming[LINE_START_LIMIT][FREQ_60])!=60)
      CurrentScanline.Tricks|=TRICK_0BYTE_LINE;
 }

  if( (CurrentScanline.Tricks&TRICK_0BYTE_LINE) 
    && !(TrickExecuted&TRICK_0BYTE_LINE))
  { 
    draw_line_off=true;
    memset(PCpal,0,sizeof(long)*16);
    CurrentScanline.Bytes=0;
    TrickExecuted|=TRICK_0BYTE_LINE;
  }

  ////////////////////////////////////////
  //  LEFT BORDER OFF (line +26, +20)   //
  ////////////////////////////////////////

/*  To "kill" the left border, the program sets bit 1 of shift mode so that the 
    GLU thinks that it's a high resolution line starting.
*/

  if(!(TrickExecuted&(TRICK_LINE_PLUS_20|TRICK_LINE_PLUS_26|TRICK_0BYTE_LINE)))
  {
    if(!(CurrentScanline.Tricks
      &(TRICK_LINE_PLUS_20|TRICK_LINE_PLUS_26|TRICK_LINE_PLUS_24)))
    {
      r2cycle=PreviousChangeToHi(ScanlineTiming[LINE_START_LIMIT_PLUS2][FREQ_71]);
      if(r2cycle>=ScanlineTiming[LINE_START_LIMIT_MINUS12][FREQ_71]
        && r2cycle!=-1 && r2cycle<=ScanlineTiming[LINE_START_LIMIT][FREQ_71])
      {
        r0cycle=NextChangeToLo(r2cycle); // 0 or 1
        if(r0cycle>ScanlineTiming[LINE_START_LIMIT][FREQ_71] 
          && r0cycle<=ScanlineTiming[LINE_START_LIMIT_PLUS26][FREQ_71])
        {
/*  Only on the STE, it is possible to create stable long overscan lines (no 
    left, no right border) without a "Shifter reset" switch, aka stabiliser.
    Those shift mode switches for left border removal produce a 20 bytes bonus
    instead of 26, and the total overscan line is 224 bytes instead of 230. 
    224/8=28,no rest => no Shifter confusion.
    The cycle of going high res is 512, which is also the HSYNC cycle, this is
    possible because the HSYNC decision happens one half-cycle before the 
    register change! In this high level emulation that runs at 8MHz, we're pragmatic.
    Test cases: MOLZ, Riverside, EPSS, Hard as Ice, We Were...
    The "DE" decision is made earlier on the STE than on the STF because of possible HSCROLL.
    If there's no HSCROLL, or the STE is in medium resolution, Shifter prefetch is delayed,
    by using a chain of flips flops.
    By 4 cycles for no HSCROLL in high res, by 8 cycles for HSCROLL in med res, by 16 cycles
    for no HSCROLL in low or med res.
    If you switch to low res after the DE decision has been made in high res (emulator cycle 10),
    but before the delay for high res is finished (emulator cycle 14), the GLUE keeps on
    delaying for 16 cycles.
    This is the real explanation for the line +20 trick at emulator cycle 12.
    If there is HSCROLL, we use the same flag but it's a different effect: the line starts
    as HIRES and the Shifter scrolls as for a LORES line.
*/
          if(IS_STE && (r0cycle==ScanlineTiming[LINE_PLUS_20A][FREQ_50]
            || hscroll
              &&r0cycle==ScanlineTiming[LINE_PLUS_20B][FREQ_50]))
            CurrentScanline.Tricks|=TRICK_LINE_PLUS_20;
          else
            CurrentScanline.Tricks|=TRICK_LINE_PLUS_26;
        }
      }
    }

    if(CurrentScanline.Tricks&(TRICK_LINE_PLUS_26|TRICK_LINE_PLUS_20))
    {
      if(CurrentScanline.Tricks&TRICK_LINE_PLUS_20)
      {
        CurrentScanline.Bytes+=20;
        if(SideBorderSize==VERY_LARGE_BORDER_SIDE)
          left_border=13; // there's still some significant left border!
        else
        {
          left_border=0;
          shifter_pixel+=8;
        }
        TrickExecuted|=TRICK_LINE_PLUS_20;
        if(hscroll)
        {
          CurrentScanline.StartCycle=ScanlineTiming[LINE_PLUS_20C][FREQ_50];
          CurrentScanline.Bytes-=Mmu.ExtraBytesForHscroll; //-2
          Mmu.ExtraBytesForHscroll=8;
          CurrentScanline.Bytes+=Mmu.ExtraBytesForHscroll; //+8
        }
        else
          CurrentScanline.StartCycle=ScanlineTiming[LINE_PLUS_20D][FREQ_50];
      }
      else 
      {
/*  A 'left off' grants 26 more bytes, that is 52 pixels (in low res) from cycle
    14 to 66 at 50hz. Confirmed on real STE: Overscan Demos F6
    There's no "shift" hiding the first 4 pixels but the shift is necessary for
    Steem in 384 x 270 display mode: border = 32 pixels instead of 52.
    16 pixels skipped by manipulating video counter, 4 more to skip (low res).
*/
        CurrentScanline.Bytes+=26;
        if(SideBorderSize!=VERY_LARGE_BORDER_SIDE)
        {
          shifter_pixel+=4;
          shifter_draw_pointer+=8;
        }
        TrickExecuted|=TRICK_LINE_PLUS_26;
        left_border=0;
        CurrentScanline.StartCycle=ScanlineTiming[DE_ON
          +hscroll][FREQ_71];
        if(HSCROLL && !hscroll)
          shifter_draw_pointer-=8; // Hard as Ice

        // additional shifts for left off
        // explained by the late timing of R0
        /////////////////////////////////////////////////////////
        if(hscroll && r0cycle==ScanlineTiming[LINE_PLUS_26A][FREQ_50])
        { // Big Wobble, D4/Tekila
          SHIFT_SDP(4); // 2 words lost in the Shifter
          if(SideBorderSize==VERY_LARGE_BORDER_SIDE)
            left_border=16; // quite a border caused by this technique
        }
        else if(r0cycle==ScanlineTiming[LINE_PLUS_26B][FREQ_50]) 
        { // Closure STE, DOLB, Kryos, Xmas 2004
          SHIFT_SDP(4); // 2 words lost in the Shifter (DOLB: frame starts with 1 word in the Shifter)
          if(SideBorderSize==VERY_LARGE_BORDER_SIDE)
            left_border=10; // there's some border caused by this technique
          else
          {
            SHIFT_SDP(-8);
            shifter_pixel+=8;
          }
          if(hscroll)
            SHIFT_SDP(-6); // display starts earlier
        }
        else if(Mmu.WS[OPTION_WS]==2 && IS_STF 
          && r0cycle==ScanlineTiming[LINE_PLUS_26C][FREQ_50]) 
        { // Closure STF WS2, Omega WS2
          SHIFT_SDP(6); // 3 words lost in the Shifter
          if(SideBorderSize==VERY_LARGE_BORDER_SIDE) 
            left_border=12; // real border length depends on wake state
          else
          {
            SHIFT_SDP(-8);
            shifter_pixel+=5;
          }
        }
      }//+20 or +26
    }
  }

  ////////////////
  // BLACK LINE //
  ////////////////

/*  A sync switch at cycle 34 keeps HBLANK asserted for this line.
    Video memory is fetched, but black pixels are displayed.
    This is handy to hide ugly effects of tricks in "sync lines".

    HBLANK OFF 60hz 36 50hz 40

    ljbk table
    switch to 60: 26-28 [WU1,3] 28-30 [WU2,4]
    switch back to 50: 38-...[WU1,3] 40-...[WU2,4]
*/
  if(!draw_line_off)
  {
    if(CyclesIn>=ScanlineTiming[HBLANK_OFF][FREQ_50] 
    && FreqAtCycle(ScanlineTiming[HBLANK_OFF][FREQ_60])==50 
      && FreqAtCycle(ScanlineTiming[HBLANK_OFF][FREQ_50])==60)
      CurrentScanline.Tricks|=TRICK_BLACK_LINE;

    if(CurrentScanline.Tricks&TRICK_BLACK_LINE)
    {
      //ASSERT( !(CurrentScanline.Tricks&TRICK_0BYTE_LINE) );
      //TRACE_LOG("%d BLK\n",scan_y);
      draw_line_off=true;
      memset(PCpal,0,sizeof(long)*16); // all colours black
    }
  }

  //////////////////////
  // MED RES OVERSCAN //
  //////////////////////

/*  Overscan (230byte lines) is possible in medium resolution too.
    There can be a plane shift if resolution is changed from 2 to 0 then
    1, according to cycles run in low resolution.
    No Cooper Greetings, 20 cycles, lines 183, 200 36 cycles, shift=2
    Dragonnels/reset, 16 cycles
    PYM/Best Part of Creation, 28 cycles
*/
  if(!left_border && !(TrickExecuted&(TRICK_OVERSCAN_MED_RES|TRICK_0BYTE_LINE)))
  {
    r1cycle=CycleOfLastChangeToShiftMode(1);
    if(r1cycle>ScanlineTiming[MEDRES_OA][FREQ_50] 
    && r1cycle<=ScanlineTiming[MEDRES_OB][FREQ_50])
    {
      r0cycle=PreviousShiftModeChange(r1cycle);
      if(r0cycle!=-1 && !ShiftModeChangeAtCycle(r0cycle))
      {
        CurrentScanline.Tricks|=TRICK_OVERSCAN_MED_RES;
        TrickExecuted|=TRICK_OVERSCAN_MED_RES;
        int cycles_in_low_res=r1cycle-r0cycle;
        SHIFT_SDP(-(((cycles_in_low_res)/2)%8)/2);
      }
    }
  }

  /////////////////////
  // 4BIT HARDSCROLL //
  /////////////////////

/*  When the left border is removed, a MED/LO switch causes the Shifter to
    shift the line by a number of bytes and pixels dependent on the cycles
    at which the switch occurs. 
    PYM/Let's Do The Twist Again
    D4/NGC
    D4/Nightmare
    By convenience we also do Closure WS1,3,4 here
*/
  if(!left_border && !(CurrentScanline.Tricks
    &(TRICK_0BYTE_LINE|TRICK_4BIT_SCROLL)))
  {
    r1cycle=CycleOfLastChangeToShiftMode(1);
    if(r1cycle>=ScanlineTiming[MEDRES_OC][FREQ_50] 
    && r1cycle<=ScanlineTiming[MEDRES_OB][FREQ_50])
    {
      r0cycle=NextShiftModeChange(r1cycle);
      if(r0cycle>r1cycle && r0cycle<=ScanlineTiming[MEDRES_OB][FREQ_50] 
        && !ShiftModeChangeAtCycle(r0cycle))
      {
        char cycles_in_med_res=(char)(r0cycle-r1cycle);
        char cycles_in_low_res=0;
        r0cycle=PreviousShiftModeChange(r1cycle);
        if(r0cycle>=0 && !ShiftModeChangeAtCycle(r0cycle))
          cycles_in_low_res=(char)(r1cycle-r0cycle);
        char shift_in_bytes=8-cycles_in_med_res/2+cycles_in_low_res/4;
        if(IS_STF && Mmu.WS[OPTION_WS]!=2
          && r1cycle==ScanlineTiming[MEDRES_OC][FREQ_50])
        {
          shift_in_bytes+=2;
          if(SideBorderSize==VERY_LARGE_BORDER_SIDE)
            left_border=13; // the technique leaves some significant border
          else
          {
            SHIFT_SDP(-8);
            shifter_pixel+=4;
          }
        }
        if(IS_STF || cycles_in_low_res)
          CurrentScanline.Tricks|=TRICK_4BIT_SCROLL;
        TrickExecuted|=TRICK_4BIT_SCROLL;
        SHIFT_SDP(shift_in_bytes);
        // the numbers came from Hatari, maybe from demo author?
        if(r1cycle==ScanlineTiming[LINE_PLUS_26A][FREQ_50]
          || r1cycle==ScanlineTiming[LINE_PLUS_26C][FREQ_50])
          Shifter.HblPixelShift=13+8-cycles_in_med_res-8; // -7,-3,1, 5, done in Render()
      }
    }
  }

  /////////////////
  // LINE +4, +6 //
  /////////////////

  // don't know any case, we leave that bit just in case someone would want to use this
  if(IS_STE && !hscroll 
    && CyclesIn>ScanlineTiming[LINE_START_LIMIT][FREQ_60] 
    && !(TrickExecuted&(TRICK_0BYTE_LINE | TRICK_LINE_PLUS_26
    | TRICK_LINE_PLUS_20 | TRICK_4BIT_SCROLL | TRICK_OVERSCAN_MED_RES
    | TRICK_LINE_PLUS_4 | TRICK_LINE_PLUS_6)))
  {
    t=FreqAtCycle(ScanlineTiming[LINE_START_LIMIT][FREQ_60]==50)
      ? (ScanlineTiming[LINE_START_LIMIT][FREQ_50]+4) 
      : ScanlineTiming[LINE_START_LIMIT][FREQ_50];
    if(ShiftModeChangeAtCycle(t)==2)
    {
      CurrentScanline.Tricks|=TRICK_LINE_PLUS_6;
      left_border-=2*6; 
      CurrentScanline.Bytes+=6;
      TrickExecuted|=TRICK_LINE_PLUS_6;
    }
    else if(ShiftModeChangeAtCycle(t+4)==2)
    {
      CurrentScanline.Tricks|=TRICK_LINE_PLUS_4;
      left_border-=2*4; 
      CurrentScanline.Bytes+=4;
      TrickExecuted|=TRICK_LINE_PLUS_4;
    }
  }

  /////////////
  // LINE +2 //
  /////////////

/*  A line that starts as a 60hz line and ends as a 50hz line gains 2 bytes 
    because 60hz lines start and stop 4 cycles before 50hz lines.
    This is used in some demos, but most cases are accidents, especially
    on the STE: the GLU checks frequency earlier because of possible horizontal
    scrolling, and this may interfere with the trick that removes top or bottom
    border.

    Forest, Beeshift, LoSTE screens, Closure, Mindbomb/No Shit
    STE: BIG Demo #1, Decade menu, nordlicht stniccc 2015, NPG/World of Music
*/
  if(!(CurrentScanline.Tricks&
    (TRICK_0BYTE_LINE|TRICK_LINE_PLUS_2|TRICK_LINE_PLUS_4|TRICK_LINE_PLUS_6
      |TRICK_LINE_PLUS_20|TRICK_LINE_PLUS_26)))
  {
    if(CyclesIn>=ScanlineTiming[LINE_START_LIMIT][FREQ_60] 
      && FreqAtCycle(ScanlineTiming[LINE_START_LIMIT][FREQ_60])==60 
      && ((CyclesIn<ScanlineTiming[LINE_STOP_LIMIT][FREQ_60] && video_freq==50)
      || CyclesIn>=ScanlineTiming[LINE_STOP_LIMIT][FREQ_60] &&
      FreqAtCycle(ScanlineTiming[LINE_STOP_LIMIT][FREQ_60])==50))
      CurrentScanline.Tricks|=TRICK_LINE_PLUS_2;
  }
  if((CurrentScanline.Tricks&TRICK_LINE_PLUS_2)
    && !(TrickExecuted&TRICK_LINE_PLUS_2))
  {
    CurrentScanline.Bytes+=2;
#if defined(SSE_SHIFTER_UNSTABLE)
 /* In NPG_WOM, there's an accidental +2 on STE, but the scroll flicker isn't
    ugly.
    It's because those 2 bytes unbalance the Shifter by one word, and the screen
    gets shifted, also next frame, and the graphic above the scroller is ugly.
    It also fixes nordlicht_stniccc2015_partyversion.
    update: on real STE, flicker depends on some unidentified state
 */
    if(OPTION_UNSTABLE_SHIFTER && 
      (DPEEK(0x111a6)==0x07f0   // nordlicht_stniccc2015_partyversion
      ||DPEEK(0xeea6)==0x22d8)) // NPG_WOM
    {
      Shifter.Preload=1;
    }
#endif
    TrickExecuted|=TRICK_LINE_PLUS_2;
  }

  ///////////////
  // LINE -106 //
  ///////////////

/*  A shift mode switch to 2 before cycle 174 (end of HIRES line) causes 
    the line to stop there. 106 bytes are not fetched.
    
    ljbk table
    R2 64 [...] -> 172 [WS1] 174 [WS3,4] 176 [WS2]
*/
  if(!(CurrentScanline.Tricks&(TRICK_LINE_MINUS_106|TRICK_0BYTE_LINE))
    && !((CurrentScanline.Tricks&TRICK_80BYTE_LINE)&&(m_ShiftMode&2))
    && CyclesIn>=ScanlineTiming[DE_OFF][FREQ_71]
    && (ShiftModeAtCycle(ScanlineTiming[LINE_STOP_LIMIT][FREQ_71])&2))
     CurrentScanline.Tricks|=TRICK_LINE_MINUS_106;
  if((CurrentScanline.Tricks&TRICK_LINE_MINUS_106)
    && !(TrickExecuted&TRICK_LINE_MINUS_106))
  {
    TrickExecuted|=TRICK_LINE_MINUS_106;
    if((CurrentScanline.Tricks&TRICK_80BYTE_LINE) && !(m_ShiftMode&2))
    {
      CurrentScanline.Bytes+=80;
      CurrentScanline.Tricks&=~TRICK_80BYTE_LINE;
    }
    CurrentScanline.Bytes-=106;
/*  The MMU won't fetch anything more, and as long as the ST is in high res,
    the scanline is black, but Steem renders the full scanline in colour.
    It's in fact data of next scanline.
*/
    draw_line_off=true;
    memset(PCpal,0,sizeof(long)*16); // all colours black
  }

  /////////////////////
  // DESTABILISATION //
  /////////////////////

/*  Detect MED/LO switches during DE.
    Note we have 1-4 words in the Shifter, and there's no restabilising at the
    start of next scanline.
    In fact, this is a real Shifter trick and ideally would be in shifter.cpp!
*/
/*
ljbk:
detect unstable: switch MED/LOW - Beeshift
- 3 (screen shifted by 12 pixels because only 1 word will be read before the 4 are available to draw the bitmap);
- 2 (screen shifted by 8 pixels because only 2 words will be read before the 4 are available to draw the bitmap);
- 1 (screen shifted by 4 pixels because only 3 words will be read before the 4 are available to draw the bitmap);
- 0 (screen shifted by 0 pixels because the 4 words will be read to draw the bitmap);
*/
#if defined(SSE_SHIFTER_UNSTABLE)
  if(OPTION_UNSTABLE_SHIFTER && !(CurrentScanline.Tricks &(TRICK_UNSTABLE
    |TRICK_0BYTE_LINE|TRICK_LINE_PLUS_26|TRICK_LINE_MINUS_106|TRICK_LINE_MINUS_2
    |TRICK_LINE_PLUS_2|TRICK_4BIT_SCROLL))&& FetchingLine()) 
  {
    int mode;
    r1cycle=NextShiftModeChange(ScanlineTiming[LINE_START_LIMIT][FREQ_50]); 
    if(r1cycle>ScanlineTiming[LINE_START_LIMIT][FREQ_50]
      && r1cycle<ScanlineTiming[LINE_STOP_LIMIT][FREQ_50]
      && (r1cycle-ScanlineTiming[DESTAB_A][FREQ_50])%8==0 //generic, dangerous! (84+, 204+)
    && (mode=ShiftModeChangeAtCycle(r1cycle))!=0)
    {
      r0cycle=NextShiftModeChange(r1cycle,0);
      int cycles_in_med_or_high=r0cycle-r1cycle;
      if(r0cycle<=ScanlineTiming[LINE_STOP_LIMIT][FREQ_50]
        &&cycles_in_med_or_high>0)
      {
        Shifter.Preload=((cycles_in_med_or_high/4)%4);
        if((mode&2)&&Shifter.Preload&1)
          Shifter.Preload+=(4-Shifter.Preload)*2; // there's a low-level explanation
        CurrentScanline.Tricks|=TRICK_UNSTABLE;
      }
    }
  }

  //  Shift due to unstable Shifter, apply effect
  if(OPTION_UNSTABLE_SHIFTER && Shifter.Preload 
    && CyclesIn>ScanlineTiming[DE_ON][FREQ_50] // arbitrary, the idea is after left off check
    && !(TrickExecuted&TRICK_UNSTABLE) && !(CurrentScanline.Tricks
    &(TRICK_0BYTE_LINE|TRICK_LINE_PLUS_2|TRICK_LINE_PLUS_26))
    // if only 1 word it gets restabilised in WS1!
    && !((Shifter.Preload%4)==1&&IS_STF&&Mmu.WS[OPTION_WS]==1)) 
  {
    // 1. planes
    int shift_sdp=-((Shifter.Preload)%4)*2;
    SHIFT_SDP(shift_sdp);
    // 2. pixels
    // Beeshift, the full frame shifts in the border
    // Dragon: shifts -4, just like the 230 byte lines below
    if(left_border)
    {
      left_border-=(Shifter.Preload%4)*4;
      right_border+=(Shifter.Preload%4)*4;
    }
    //TRACE_LOG("Y%d Preload %d shift SDP %d pixels %d lb %d rb %d\n",scan_y,Preload,shift_sdp,HblPixelShift,left_border,right_border);
    TrickExecuted|=TRICK_UNSTABLE;
  }
#endif

  /////////////
  // LINE -2 //
  /////////////
  
/*  DE ends 4 cycles before normal because freq has been set to 60hz:
    2 bytes fewer are fetched from video memory.
    Thresholds/WU states (from table by ljbk)

      60hz  66 - 380 WU1,3
            68 - 382 WU2,4

      50hz  382 -... WU1,3
            384 -... WU2,4
*/
  if (!(CurrentScanline.Tricks
    &(TRICK_0BYTE_LINE | TRICK_LINE_MINUS_106 | TRICK_LINE_MINUS_2))
    && CyclesIn > ScanlineTiming[LINE_STOP_LIMIT][FREQ_60]
    && FreqAtCycle(ScanlineTiming[LINE_START_LIMIT][FREQ_60]) != 60
    && FreqAtCycle(ScanlineTiming[LINE_STOP_LIMIT][FREQ_60]) == 60)
     CurrentScanline.Tricks |= TRICK_LINE_MINUS_2;
   
  if((CurrentScanline.Tricks&TRICK_LINE_MINUS_2)
    &&!(TrickExecuted&TRICK_LINE_MINUS_2))
  {
    CurrentScanline.Bytes-=2; 
    TrickExecuted|=TRICK_LINE_MINUS_2;
//    TRACE_LOG("-2 y %d c %d s %d e %d ea %d\n",scan_y,LINECYCLES,scanline_drawn_so_far,overscan_add_extra,ExtraAdded);
  }

  /////////////////////////////////
  // RIGHT BORDER OFF (line +44) // 
  /////////////////////////////////

/*  A sync switch to 0 (60hz) at cycle 384 (end of display for 50hz)
    makes the GLUE fail to stop the line (DE still on).
    DE will stop only at cycle of HSYNC, 472.
    This is 88 cycles later and the reason why the trick grants 44 more
    bytes of video memory for the scanline.

    Because a 60hz line stops at cycle 380, the sync switch must hit just
    after that and right before the test for end of 50hz line occurs.
    That's why cycle 384 is targeted, but according to wake-up state other
    timings may work.
    Obviously, the need to hit the GLU/Shifter registers at precise cycles
    on every useful scanline was impractical.

    WS thresholds (from table by ljbk) 

    Switch to 60hz  382 - 384 WS1,3
                    384 - 386 WS2,4 Nostalgia menu
*/
/*  We used the following to calibrate (Beeshift3).

LJBK:
So going back to the tests, the switchs are done at the places indicated:
-71/50 at 295/305;
-71/50 at 297/305;
-60/50 at 295/305;
and the MMU counter at $FFFF8209.w is read at the end of that line.
It can be either $CC: 204 or $A0: 160.
The combination of the three results is then tested:
WS4: CC A0 CC
WS3: CC A0 A0
WS2: CC CC CC Right Border is always open
WS1: A0 A0 A0 no case where Right Border was open

Emulators_cycle = (cycle_Paulo + 83) mod 512

WS                           1         2         3         4
 
-71/50 at 386/396            N         Y         Y         Y
-71/50 at 388/396            N         Y         N         N
-60/50 at 386/396            N         Y         N         Y

Tests are arranged to be efficient.
*/
  if(!(CurrentScanline.Tricks&(TRICK_0BYTE_LINE|TRICK_LINE_MINUS_2|
    TRICK_LINE_MINUS_106|TRICK_LINE_PLUS_44)))
  {
    bool dont_test=(CyclesIn<ScanlineTiming[LINE_STOP_LIMIT_MINUS2][FREQ_50])
      || (FreqAtCycle(ScanlineTiming[LINE_STOP_LIMIT][FREQ_60])==60);
    if(!dont_test)
    {
      t=ScanlineTiming[LINE_PLUS_44_R][FREQ_50];
      if((!(m_SyncMode&2)
        &&(CyclesIn>ScanlineTiming[LINE_STOP_LIMIT][FREQ_60])
        &&(CyclesIn<=ScanlineTiming[LINE_STOP_LIMIT][FREQ_50]))
        ||((CyclesIn>=ScanlineTiming[LINE_STOP_LIMIT][FREQ_50])
        &&FreqAtCycle(ScanlineTiming[LINE_STOP_LIMIT][FREQ_50])==60)
        ||CyclesIn>=t&&(ShiftModeAtCycle(t)&2))
      {
        CurrentScanline.Tricks|=TRICK_LINE_PLUS_44;
      }
      if((CurrentScanline.Tricks&TRICK_LINE_PLUS_44)
        &&!(TrickExecuted&TRICK_LINE_PLUS_44))
      {
        right_border=0;
        TrickExecuted|=TRICK_LINE_PLUS_44;
        CurrentScanline.Bytes+=44;
        CurrentScanline.EndCycle=ScanlineTiming[HSYNC_ON][FREQ_50];
      }
    }
  }

  ////////////////
  // STABILISER //
  ////////////////

/*  A stabiliser is a HI/LO or MED/LO switch that resets the unbalanced Shifter.
    It does that by accelerating the pixel counter, which demo coders couldn't
    know at the time, it's a recent insight by ijor.
    For high-level emulation, the stabiliser isn't vital.
*/
  if(!(CurrentScanline.Tricks&TRICK_STABILISER) 
    && CyclesIn>ScanlineTiming[STAB_A][FREQ_50])
  { 
    r2cycle=NextShiftModeChange(ScanlineTiming[STAB_A][FREQ_50]);
    if(r2cycle>ScanlineTiming[STAB_A][FREQ_50] 
      && r2cycle<ScanlineTiming[STAB_B][FREQ_50]) 
    {
      if(!ShiftModeChangeAtCycle(r2cycle))
        r2cycle=NextShiftModeChange(r2cycle);
      if(r2cycle>-1 && r2cycle<ScanlineTiming[STAB_B][FREQ_50] 
        && ShiftModeChangeAtCycle(r2cycle))
      {
        r0cycle=NextShiftModeChange(r2cycle,0);
        if(r0cycle>-1 && r0cycle<ScanlineTiming[STAB_C][FREQ_50]) 
          CurrentScanline.Tricks|=TRICK_STABILISER;
      }
      else if(CyclesIn>ScanlineTiming[STAB_D][FREQ_50]
        && ShiftModeAtCycle(ScanlineTiming[STAB_D][FREQ_50])==1)
      {
        r0cycle=NextShiftModeChange(ScanlineTiming[STAB_D][FREQ_50],0);
        if(r0cycle>-1 && r0cycle<ScanlineTiming[STAB_C][FREQ_50]) 
          CurrentScanline.Tricks|=TRICK_STABILISER;
      }
    }
  }
#if defined(SSE_SHIFTER_UNSTABLE)
  if(CurrentScanline.Tricks&TRICK_STABILISER)
    Shifter.Preload=0; // it's a simplification
#endif

  ////////////////////////
  //  NON-STOPPING LINE //
  ////////////////////////

/*  In the Enchanted Land hardware tests, a HI switch at end of display
    when the right border has been removed causes the GLUE to miss HSYNC,
    and DE stays asserted for the rest of the line (24 bytes), then the next 
    scanline, not stopping until HSYNC of that line (232 bytes).
    The result of the test is written to $204-$20D and the line +26 trick 
    timing during the game depends on it. This STF/STE distinction, which 
    was the point of the test, is emulated in Steem SSE.

      STF 464/472 => R2 4 (wouldn't work on the STE)
    000204 : 00bd 0003
    000208 : 0059 000d
    00020c : 000d 0001

      STE 460/468 => R2 0
    000204 : 00bd 0002
    000208 : 005a 000c
    00020c : 000d 0001

    The program reads the video counter at the end of the scanline, so we can't
    delay test until next line, unlike 0byte.
*/

  if(!right_border && !(NextScanline.Tricks&TRICK_0BYTE_LINE) 
    && !(CurrentScanline.Tricks&TRICK_LINE_PLUS_24))
  {
    r2cycle=ScanlineTiming[HSYNC_ON2][FREQ_50]+(short)CurrentScanline.Cycles;
    if(CyclesIn>=r2cycle && (ShiftModeAtCycle(r2cycle)&2))
    {
      TRACE_LOG("Enchanted Land HW test F%d Y%d R2 %d R0 %d\n",FRAME,scan_y,PreviousShiftModeChange(466),NextShiftModeChange(466,0));
      CurrentScanline.Bytes+=24; // "double" right off
      CurrentScanline.Tricks|=TRICK_LINE_PLUS_24; // recycle left off 60hz bit!
      // the following isn't necessary for the game
      NextScanline.Tricks|=TRICK_LINE_PLUS_26|TRICK_LINE_PLUS_44; //+2
      NextScanline.Bytes=232;
      time_of_next_timer_b+=512; // no timer B for this line (too late?)
    }
  }
}


void TGlue::CheckVerticalOverscan() {
/*  Top and bottom border.
    Using video_freq_at_start_of_vbl is an approximation.
*/
  short CyclesIn=(short)LINECYCLES;
  enum{NO_LIMIT=0,LIMIT_TOP,LIMIT_MIDDLE,LIMIT_BOTTOM};
  BYTE on_overscan_limit;
  switch(scan_y) {
  case -30:
    on_overscan_limit=LIMIT_TOP;
    break;
  case -1:
    on_overscan_limit=LIMIT_MIDDLE;
    break;
  default:
    on_overscan_limit=LIMIT_BOTTOM;
    break;
  }
  int t=0;
  if(emudetect_overscans_fixed && on_overscan_limit!=LIMIT_MIDDLE) 
  {
    CurrentScanline.Tricks|= (on_overscan_limit==LIMIT_TOP) 
        ? TRICK_TOP_OVERSCAN: TRICK_BOTTOM_OVERSCAN;
  }
  // 50hz frame overscan
  else if(on_overscan_limit && video_freq_at_start_of_vbl==50)
  {
    t=ScanlineTiming[VERT_OVSCN_LIMIT][FREQ_50];
    if(CyclesIn>=t &&FreqAtCycle(t)!=50||CyclesIn<t && video_freq!=50)
    {
/*  If a program changes the frequency right before the normal frame
    starts, vertical DE won't be asserted (and there will be no timer B)
    during the frame, so it's a 0byte frame! Hard as Ice bug.
*/
      if(on_overscan_limit==LIMIT_MIDDLE && !de_v_on)
      {
        TRACE_LOG("0byte frame 50hz\n");
        de_start_line=de_end_line+1; // no VDE this frame
      }
      else
        CurrentScanline.Tricks|=(on_overscan_limit==LIMIT_TOP)
          ?TRICK_TOP_OVERSCAN:TRICK_BOTTOM_OVERSCAN;
    }
#if defined(SSE_DEBUGGER_TOPOFF)
    if(!(CurrentScanline.Tricks&(TRICK_TOP_OVERSCAN|TRICK_BOTTOM_OVERSCAN))
      && freq_change_this_scanline
      )
    {
      if((DEBUGGER_CONTROL_MASK2&DEBUGGER_CONTROL_TOPOFF)&&(on_overscan_limit==LIMIT_TOP))
      {
        runstate=RUNSTATE_STOPPING;
        SET_WHY_STOP("Top off missed");
      }
      else if((DEBUGGER_CONTROL_MASK2&DEBUGGER_CONTROL_BOTTOMOFF)&&(on_overscan_limit==LIMIT_BOTTOM))
      {
        runstate=RUNSTATE_STOPPING;
        SET_WHY_STOP("Bottom off missed");
      }
    }
#endif
  }
  // 60hz frame overscan
  else if(on_overscan_limit!=LIMIT_TOP && video_freq_at_start_of_vbl==60)
  {
    t=ScanlineTiming[VERT_OVSCN_LIMIT][FREQ_60];
    if(CyclesIn>=t && FreqAtCycle(t)==50)
    {
#ifdef SSE_BETA // hypothetical...
      if(on_overscan_limit==LIMIT_MIDDLE && !de_v_on)
      {
        TRACE_LOG("frame 60->50Hz\n"); 
        de_start_line=63; // becomes a 50hz frame?
        de_end_line=263-1;
        nLines=313;
        scan_y-=63-34;
      }
      else
#endif
        CurrentScanline.Tricks|=TRICK_BOTTOM_OVERSCAN_60HZ;
    }
  }
  if(CurrentScanline.Tricks&
    (TRICK_TOP_OVERSCAN|TRICK_BOTTOM_OVERSCAN|TRICK_BOTTOM_OVERSCAN_60HZ))
  {
    if(!OPTION_68901)
      time_of_next_timer_b=time_of_next_event+cpu_cycles_from_hbl_to_timer_b
      +TB_TIME_WOBBLE;
    if(on_overscan_limit==LIMIT_TOP)
    {
      video_first_draw_line=-29;
      de_start_line=34;
    }
    else // bottom border off
    {
/*  At 60hz, fewer scanlines are displayed in the bottom border than 50hz due
    to vertical blank and sync (It's a girl 2 last screen).
*/
      video_last_draw_line=(CurrentScanline.Tricks&TRICK_BOTTOM_OVERSCAN_60HZ)
        ? 226 : 247; //vsync
      de_end_line=(CurrentScanline.Tricks&TRICK_BOTTOM_OVERSCAN_60HZ)
        ? 259 : 309;
    }
  }
#if defined(SSE_DEBUGGER_FRAME_REPORT) && defined(SSE_DEBUGGER_TRACE_CONTROL)
  if(TRACE_ENABLED(LOGSECTION_VIDEO)&&(TRACE_MASK1 & TRACE_CONTROL_VERTOVSC) && CyclesIn>=t) 
  {
    FrameEvents.ReportLine();
    TRACE_LOG("F%d y%d freq at %d %d at %d %d switch %d to %d, %d to %d, %d to %d overscan %X\n",FRAME,scan_y,t,FreqAtCycle(t),t-2,FreqAtCycle(t-2),PreviousFreqChange(PreviousFreqChange(t)),FreqChangeAtCycle(PreviousFreqChange(PreviousFreqChange(t))),PreviousFreqChange(t),FreqChangeAtCycle(PreviousFreqChange(t)),NextFreqChange(t),FreqChangeAtCycle(NextFreqChange(t)),CurrentScanline.Tricks);
  //  ASSERT( scan_y!=199|| (CurrentScanline.Tricks&TRICK_BOTTOM_OVERSCAN) );
    //ASSERT( scan_y!=199|| video_last_draw_line==247 );
  }
#endif
}


void TGlue::EndHBL() {
/*  1. Finish horizontal overscan : correct -2 & +2 effects
    Those tests are much like EndHBL() in Hatari (v1.6)
    2. Detect loaded Shifter (hack)
*/
  if((CurrentScanline.Tricks&(TRICK_LINE_PLUS_2|TRICK_LINE_PLUS_26))
    && !(CurrentScanline.Tricks&(TRICK_LINE_MINUS_2|TRICK_LINE_MINUS_106))
    && CurrentScanline.EndCycle==ScanlineTiming[DE_OFF][FREQ_60]) 
  {
    CurrentScanline.Tricks&=~TRICK_LINE_PLUS_2;
    shifter_draw_pointer-=2;
    CurrentScanline.Bytes-=2;
  } 
  if(CurrentScanline.Tricks&TRICK_LINE_MINUS_2     
    && (CurrentScanline.StartCycle==ScanlineTiming[DE_ON][FREQ_60]
    || CurrentScanline.EndCycle!=ScanlineTiming[DE_OFF][FREQ_60]))
  {
    CurrentScanline.Tricks&=~TRICK_LINE_MINUS_2;
    shifter_draw_pointer+=2;
    CurrentScanline.Bytes+=2;
  }
#if defined(SSE_SHIFTER_UNSTABLE) // just hacks as the full story could be more complicated
  if(OPTION_UNSTABLE_SHIFTER && IS_STF)
  {
    if((CurrentScanline.Tricks&TRICK_LINE_PLUS_26)
      &&!(CurrentScanline.Tricks&(TRICK_STABILISER|TRICK_LINE_MINUS_2|TRICK_LINE_MINUS_106))
      && (LPEEK(8)==0x118E  // Ventura Board and Sphere + Ultimate Dist
      || LPEEK(8)==0x8B64 // Lame Trop Falcon
      || LPEEK(0x24)==0xF194)) // Overdrive/Dragon
    {
      if(!Shifter.Preload)
      {
        Shifter.Preload=1;
        CurrentScanline.Tricks|=TRICK_UNSTABLE; // so it shows in frame report
      }
    }
  }
#endif
}


bool TGlue::FetchingLine() {
  // does the current scan_y involve fetching by the MMU?
  // notice < video_last_draw_line, not <=
  // should be inline but then the h file must know the variables
  return 
#if defined(SSE_VID_STVL1)
    (OPTION_C3) ? Stvl.vde :
#endif
    (scan_y>=video_first_draw_line && scan_y<video_last_draw_line);
}


void TGlue::IncScanline() {

  //ASSERT(!OPTION_C3);
  Debug.ShifterTricks|=CurrentScanline.Tricks; // for frame
  if(screen_res>=2) // being in hires is no trick
    Debug.ShifterTricks&=~TRICK_80BYTE_LINE;
#if defined(SSE_STATS)
  Stats.mskOverscan1=Debug.ShifterTricks;
  Stats.mskOverscan|=Debug.ShifterTricks;
#endif
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
  if((FRAME_REPORT_MASK2 & FRAME_REPORT_MASK_SHIFTER_TRICKS) 
    && CurrentScanline.Tricks)
    FrameEvents.Add(scan_y,(short)CurrentScanline.Cycles,'T',CurrentScanline.Tricks);
  if((FRAME_REPORT_MASK2 & FRAME_REPORT_MASK_SHIFTER_TRICKS_BYTES))
    FrameEvents.Add(scan_y,(short)CurrentScanline.Cycles,'#',CurrentScanline.Bytes);
#endif

  if(VCount<nLines)
    VCount++;

  Shifter.IncScanline();
  PreviousScanline=CurrentScanline; // auto-generated

  de_v_on=(VCount>=de_start_line && VCount<=de_end_line);

  CurrentScanline=NextScanline;
  if(CurrentScanline.Tricks)
    ; // don't change #bytes
  else if(de_v_on)
    //Start with 160, it's adapted in AdaptScanlineValues if necessary
    CurrentScanline.Bytes=160; 
  else
    NextScanline.Bytes=0;

  Mmu.ExtraBytesForHscroll=0;
  Mmu.no_LW=FALSE;
  m_Status.timerb_start=m_Status.timerb_end=0;
  AdaptScanlineValues(-1);
  TrickExecuted=0;

#if defined(SSE_HARDWARE_OVERSCAN)
/*  Emulate hardware overscan scanlines of the LaceScan circuit.
    It is a hack that intercepts the 'DE' line between the GLUE and the 
    MMU. Hence only possible on the STF/Mega ST (on the STE, GLUE and
    MMU are one chip).
    Compared with software overscan, 6 more bytes are fetched at 50hz.
    We also emulate the AutoSwitch Overscan circuit, which uses the GLUE clock
    to time DE, so that 224 bytes are fetched at 50hz and 60hz. As it is
    divisible by 4, GEM isn't troubled. LaceScan produces 234 bytes at 60hz.
*/
  if(OPTION_HWOVERSCAN && SSEConfig.OverscanOn)
  {
    if(FetchingLine()) // there are also more fetching lines
    {
      left_border=right_border=0; // using Steem's existing system
      if(COLOUR_MONITOR)
      {
        if(OPTION_HWOVERSCAN==LACESCAN)
          CurrentScanline.Bytes=(video_freq_at_start_of_vbl==60)?234:236;
        else
          CurrentScanline.Bytes=224;
        if(SideBorderSize!=VERY_LARGE_BORDER_SIDE)
          shifter_draw_pointer+=8; // as for "left off", skip non displayed border
      }
      else
      {
        CurrentScanline.Bytes=(OPTION_HWOVERSCAN==LACESCAN)?100:96;
        TrickExecuted=CurrentScanline.Tricks=TRICK_HIRES_OVERSCAN; // needed by Shifter.DrawScanlineToEnd()
      }
    }
  }
#endif
  NextScanline.Tricks=0; // eg for 0byte lines mess
  m_Status.scanline_done=false;
  LINEWID=Mmu.linewid;
}


/*  Argh! those horrible functions still there.
    An attempt at replacing them with a table proved less efficient anyway
    (see R419), so we should try to optimise them instead... TODO
*/

void TGlue::AddFreqChange(BYTE f) {
  // Replacing macro ADD_SHIFTER_FREQ_CHANGE(video_freq)
  shifter_freq_change_idx++;
#if defined(SSE_VC_INTRINSICS)
  BITRESET(shifter_freq_change_idx,5); //stupid, I guess
#else
  shifter_freq_change_idx&=31;
#endif
  shifter_freq_change_time[shifter_freq_change_idx]=act;
  shifter_freq_change[shifter_freq_change_idx]=f;                    
}


void TGlue::AddShiftModeChange(BYTE mode) {
  // called only by SetShiftMode
  shifter_mode_change_idx++;
#if defined(SSE_VC_INTRINSICS)
  BITRESET(shifter_mode_change_idx,5); //stupid, I guess
#else
  shifter_mode_change_idx&=31;
#endif
  shifter_mode_change_time[shifter_mode_change_idx]=act;
  shifter_mode_change[shifter_mode_change_idx]=mode;                    
}


int TGlue::FreqChangeAtCycle(int cycle) {
  // if there was a change at this cycle, return it, otherwise -1
  COUNTER_VAR t=cycle+LINECYCLE0; // convert to absolute
  int i,j;
  // loop while it's later than cycle, with safety
  for(i=shifter_freq_change_idx,j=0;
  j<32 && shifter_freq_change_time[i]-t>0;
  j++,i--,i&=31);
  // here, we're on the right cycle, or before
  int rv=(j<32 && !(shifter_freq_change_time[i]-t))
    ?shifter_freq_change[i]:-1;
  return rv;
}


int TGlue::ShiftModeChangeAtCycle(int cycle) {
  // if there was a change at this cycle, return it, otherwise -1
  COUNTER_VAR t=cycle+LINECYCLE0; // convert to absolute
  int i,j;
  // loop while it's later than cycle, with safety
  for(i=shifter_mode_change_idx,j=0;
  j<32 && shifter_mode_change_time[i]-t>0;
  j++,i--,i&=31);
  // here, we're on the right cycle, or before
  int rv=(j<32 && !(shifter_mode_change_time[i]-t))
    ?shifter_mode_change[i]:-1;
  return rv;

}


int TGlue::FreqAtCycle(int cycle) {
  //ASSERT(cycle<=LINECYCLES);
  COUNTER_VAR t=cycle+LINECYCLE0; // convert to absolute
  int i,j;
  for(i=shifter_freq_change_idx,j=0
    ; shifter_freq_change_time[i]-t>0 && j<32
    ; i--,i&=31,j++) ;
  if(shifter_freq_change_time[i]-t<=0 && shifter_freq_change[i]>0)
    return shifter_freq_change[i];
  return video_freq_at_start_of_vbl;
}


int TGlue::ShiftModeAtCycle(int cycle) {
  // what was the shift mode at this cycle?
  COUNTER_VAR t=cycle+LINECYCLE0; // convert to absolute
  int i,j;
  for(i=shifter_mode_change_idx,j=0
    ; shifter_mode_change_time[i]-t>0 && j<32
    ; i--,i&=31,j++) ;
  if(shifter_mode_change_time[i]-t<=0)
    return shifter_mode_change[i];
  return m_ShiftMode; // we don't have at_start_of_vbl
}


#ifdef SSE_DEBUG

int TGlue::NextFreqChange(int cycle,int value) {
  // return cycle of next change after this cycle
  COUNTER_VAR t=cycle+LINECYCLE0; // convert to absolute
  int idx,i,j;
  for(idx=-1,i=shifter_freq_change_idx,j=0
    ; shifter_freq_change_time[i]-t>0 && j<32
    ; i--,i&=31,j++)
    if(value==-1 || shifter_freq_change[i]==value)
      idx=i;
  if(idx!=-1 && shifter_freq_change_time[idx]-t>0)
    return (int)(shifter_freq_change_time[idx]-LINECYCLE0);
  return -1;
}

#endif


short TGlue::NextShiftModeChange(int cycle,int value) {
  // return cycle of next change after this cycle
  // if value=-1, return any change
  // if none is found, return -1
  //ASSERT(value>=-1 && value <=2);
  COUNTER_VAR t=cycle+LINECYCLE0; // convert to absolute
  short i,j,rv=-1;
  // we start from now, go back in time
  for(i=shifter_mode_change_idx,j=0; j<32; i--,i&=31,j++)
  {
    COUNTER_VAR a=shifter_mode_change_time[i]-t;
    if(a>0 && a<1024) // as long as it's valid, it's better...
    {
      if(value==-1 || shifter_mode_change[i]==value)
        rv=(short)(shifter_mode_change_time[i]-LINECYCLE0); // in linecycles
    }
    else
      break; // as soon as it's not valid, we're done
  }
  return rv;
}


short TGlue::NextChangeToHi(int cycle) {
  // return cycle of next change after this cycle
  // if none is found, return -1

  COUNTER_VAR t=cycle+LINECYCLE0; // convert to absolute
  short i,j,rv=-1;
  // we start from now, go back in time
  for(i=shifter_mode_change_idx,j=0; j<32; i--,i&=31,j++)
  {
    COUNTER_VAR a=shifter_mode_change_time[i]-t;
    if(a>0 && a<1024) // as long as it's valid, it's better...
    {
      if(shifter_mode_change[i]&2) //HI
        rv=(short)(shifter_mode_change_time[i]-LINECYCLE0); // in linecycles
    }
    else
      break; // as soon as it's not valid, we're done
  }
  return rv;
}


short TGlue::NextChangeToLo(int cycle) {
  COUNTER_VAR t=cycle+LINECYCLE0; // convert to absolute
  short i,j,rv=-1;
  // we start from now, go back in time
  for(i=shifter_mode_change_idx,j=0; j<32; i--,i&=31,j++)
  {
    COUNTER_VAR a=shifter_mode_change_time[i]-t;
    if(a>0 && a<1024) // as long as it's valid, it's better...
    {
      if(!(shifter_mode_change[i]&2)) //not HI (also MED)
        rv=(short)(shifter_mode_change_time[i]-LINECYCLE0); // in linecycles
    }
    else
      break; // as soon as it's not valid, we're done
  }
  return rv;
}


short TGlue::PreviousChangeToHi(int cycle) {
  COUNTER_VAR t=cycle+LINECYCLE0; // convert to absolute
  short idx,i,j;
  for(idx=-1,i=shifter_mode_change_idx,j=0
    ; idx==-1 && j<32
    ; i--,i&=31,j++)
    if(shifter_mode_change_time[i]-t<0&&(shifter_mode_change[i]&2))
      idx=i;
  if(idx!=-1)
    idx=(short)(shifter_mode_change_time[idx]-LINECYCLE0);
  return idx;
}


short TGlue::PreviousChangeToLo(int cycle) {
  COUNTER_VAR t=cycle+LINECYCLE0; // convert to absolute
  short idx,i,j;
  for(idx=-1,i=shifter_mode_change_idx,j=0
    ; idx==-1 && j<32
    ; i--,i&=31,j++)
    if(shifter_mode_change_time[i]-t<0&&!(shifter_mode_change[i]&2))
      idx=i;
  if(idx!=-1)
    idx=(short)(shifter_mode_change_time[idx]-LINECYCLE0);
  return idx;
}


#if defined(SSE_DEBUG) 

int TGlue::PreviousFreqChange(int cycle) {
  // return cycle of previous change before this cycle
  COUNTER_VAR t=cycle+LINECYCLE0; // convert to absolute
  int i,j;
  for(i=shifter_freq_change_idx,j=0
    ; shifter_freq_change_time[i]-t>=0 && j<32
    ; i--,i&=31,j++) ;
  if(shifter_freq_change_time[i]-t<0)
    return (int)(shifter_freq_change_time[i]-LINECYCLE0);
  return -1;
}

#endif


short TGlue::PreviousShiftModeChange(int cycle) {
  // return cycle of previous change before this cycle
  COUNTER_VAR t=cycle+LINECYCLE0; // convert to absolute
  int i,j;
  for(i=shifter_mode_change_idx,j=0
    ; shifter_mode_change_time[i]-t>=0 && j<32
    ; i--,i&=31,j++) ;
  if(shifter_mode_change_time[i]-t<0)
    return (short)(shifter_mode_change_time[i]-LINECYCLE0);
  return -1;
}


short TGlue::CycleOfLastChangeToShiftMode(int value) {
  int i,j;
  for(i=shifter_mode_change_idx,j=0
    ; shifter_mode_change[i]!=value && j<32
    && (shifter_mode_change_time[i] - LINECYCLE0)>0 
    ; i--,i&=31,j++) ;
  if(shifter_mode_change[i]==value)
    return (short)(shifter_mode_change_time[i]-LINECYCLE0);
  return -1;
}


void TGlue::GetNextVideoEvent() {
/*  Instead of following a frame event plan as in old Steem versions, we compute
    video timings on the go.
    This function is called by run's prepare_next_event(), so it's called a lot.
    It increases emulation load but it allows us to easily handle sync and mode 
    changes.
*/ 
  // ASSERT(!OPTION_C3);
  // VBI is set pending some cycles into first scanline of frame, 
  // when VSYNC stops.
  // The video counter will be reloaded again.
  if(!m_Status.vbi_done&&!VCount)
  {
    event_vector=event_trigger_vbi;
    video_event.time=ScanlineTiming[ENABLE_VBI][video_freq_idx];
    if(!m_Status.hbi_done)
    {
      hbl_pending=true; 
      hbl_pending_time=cpu_timer_at_start_of_hbl;
      update_ipl(hbl_pending_time);
    }
  }
  // Video counter is reloaded 3 lines before the end of the frame (colour)
  // VBLANK is already on since a couple of scanlines. (? TODO)
  // VSYNC will start, which will trigger reloading of the Video Counter
  // by the Mmu.
  else if(!m_Status.vc_reload_done && VCount==(nLines==501?nLines-1:nLines-3))
  {
    event_vector=event_start_vbl;
    int i=(nLines==501)?2:video_freq_idx; // monochrome: 1 line
    video_event.time=ScanlineTiming[RELOAD_SDP][i];
  }
  // event_vbl_interrupt() is Steem's internal frame or vbl routine, called
  // when all the cycles of the frame have elapsed. It normally happens during
  // the ST's VSYNC (between VSYNC start and VSYNC end).
  else if(!m_Status.vbl_done && VCount==nLines-1)
  {
    event_vector=event_vbl_interrupt;
    video_event.time=CurrentScanline.Cycles;
    m_Status.vbi_done=false;
  }  
  // default event = scanline
  else
  {
    event_vector=event_scanline;
    video_event.time=CurrentScanline.Cycles;
  }
#if defined(SSE_DEBUG)
  video_event.event=event_vector;
#endif
  if(SSEConfig.CpuBoosted  && event_vector!=event_scanline)
    video_event.time=(COUNTER_VAR)((double)video_event.time
      *cpu_cycles_multiplier);
  time_of_next_event=video_event.time+cpu_timer_at_start_of_hbl;
}


void TGlue::Reset(bool Cold) {
  m_SyncMode=0; // 60hz
  if(OPTION_68901) 
    m_ShiftMode=0; // LORES
  video_freq_idx=FREQ_60;
  CurrentScanline.Cycles=scanline_time_in_cpu_cycles[video_freq_idx];
  if(Cold)
  {
    cpu_timer_at_start_of_hbl=0;
    VCount=0;
    if(!OPTION_68901) // boot hires no C2 because of 400x TB... 
      Shifter.m_ShiftMode=m_ShiftMode=screen_res;
    hbl_pending_time=vbl_pending_time=0;
  }
  *(BYTE*)(&m_Status)=0;
  gamecart=0;
  cartbase=0xFA0000 , cartend=0xFC0000;
  hscroll=false;
  vbl_pending=false;
#if defined(SSE_VID_STVL1)
  StvlUpdate();
#endif
}


/*  SetShiftMode() and SetSyncMode() are called when a program writes
    on addresses $FF8260 (shift) or $FF820A (sync). 
*/

void TGlue::SetShiftMode(BYTE NewRes) {
/*
  The ST possesses three modes  of  video  configuration:
  320  x  200  resolution  with 4 planes, 640 x 200 resolution
  with 2 planes, and 640 x 400 resolution with 1  plane.   The
  modes  are  set through the Shift Mode Register (read/write,
  reset: all zeros).

  ff 8260   R/W             |------xx|   Shift Mode
                                   ||
                                   00       320 x 200, 4 Plane
                                   01       640 x 200, 2 Plane
                                   10       640 x 400, 1 Plane
                                   11       Reserved

  FF8260 is both in the GLU and the Shifter. It is needed in the GLU
  because sync signals are different in mode 2 (71hz).
  It is needed in the Shifter because it needs to know in how many bit planes
  memory has to be decoded, and where it must send the video signal (RGB, 
  Mono).
  For the GLU, '3' is interpreted as '2'. Case: The World is my Oyster screen #2

  Writes on ShiftMode have a 2 cycle resolution as far as the GLU is
  concerned, 4 for the Shifter. The GLU's timing matters for some video effects,
  that's why the write is handled by the Glue object.
    
  In monochrome, frequency is 71hz, a line is transmitted in 28µs.
  There are 500 scanlines + vsync = 1 scanline time.
*/

  short CyclesIn=(short)LINECYCLES;
#if defined(DEBUG_BUILD)
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
  if(!OPTION_C3 &&(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_SHIFTMODE))
    FrameEvents.Add(scan_y,CyclesIn,'R',NewRes&3); 
#endif
   LOG_TO(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Changed screen res to "+
     NewRes+" at scanline "+scan_y+", cycle "+LINECYCLES);
#endif
  int OldRes=m_ShiftMode;
  // Only two lines physically exist in the Glue and the Shifter, not a full byte
  NewRes&=3; 
  m_ShiftMode=NewRes; // GLUE's copy
  Shifter.m_ShiftMode=m_ShiftMode; // Shifter's copy
  if(screen_res> 2|| emudetect_falcon_mode!=EMUD_FALC_MODE_OFF
    || OPTION_C3 && screen_res==2)
    return; // if not, bad display in high resolution
  if(extended_monitor)
  {
    screen_res=(BYTE)(NewRes & 1);
    return;
  }
#if defined(SSE_VID_STVL2)
  Stvl.mde0=(m_ShiftMode&1);
  Stvl.mde1=((m_ShiftMode&2)==2);
#endif
  if(!OPTION_C3)
  {
    if(NewRes==3) 
      NewRes=2;
    if(NewRes!=OldRes)
      AddShiftModeChange(NewRes); // add time & mode
    AddFreqChange((NewRes&2) ? MONO_HZ : video_freq);
    Shifter.Render(CyclesIn,DISPATCHER_SET_SHIFT_MODE);
    if(screen_res==2 && !COLOUR_MONITOR)
    {
      freq_change_this_scanline=true;
      return;
    }
  }
  int old_screen_res=screen_res;
  screen_res=(BYTE)(NewRes & 1); // only for 0 or 1 - note could weird things happen?
  if(screen_res!=old_screen_res)
  {
    shifter_x=(screen_res>0) ? 640 : 320;
    if(draw_lock)
    {
      if(screen_res==0) 
        draw_scanline=draw_scanline_lowres; // the ASM function
      if(screen_res==1) 
        draw_scanline=draw_scanline_medres;
#ifdef WIN32
      if(draw_store_dest_ad)
      {
        draw_store_draw_scanline=draw_scanline;
        draw_scanline=draw_scanline_1_line[screen_res];
      }
#endif
    }
    if(video_mixed_output==3 && (act-cpu_timer_at_res_change<30))
      video_mixed_output=0; //cancel!
    else if(scan_y<-30) // not displaying anything: no output to mix...
      ; // eg Pandemonium/Chaos Dister
    else if(!video_mixed_output)
      video_mixed_output=3;
    else if(video_mixed_output<2)
      video_mixed_output=2;
    cpu_timer_at_res_change=act;
  }
  if(OPTION_C3)
    return;
  freq_change_this_scanline=true; // all switches are interesting
  if(video_last_draw_line==400 && !(m_ShiftMode&2) && screen_res<2)
  {
    video_last_draw_line>>=1; // simplistic?
    draw_line_off=true; // Steem's original flag for black line
  }
  AdaptScanlineValues(CyclesIn);
}


void TGlue::SetSyncMode(BYTE NewSync) {
/*
    ff 820a   R/W             |------xx|   Sync Mode
                                     ||
                                     | ----   External/_Internal Sync
                                      -----   50 Hz/_60 Hz Field Rate

    Only bit 1 is of interest:  1:50 Hz 0:60 Hz.
    Normally, 50hz for Europe, 60hz for the USA.
    At 50hz, the ST displays 313 lines every frame, instead of 312.5 like
    in the PAL standard (one frame with 312 lines, one with 313, etc.) 
    Sync mode is abused to create overscan (3 of the 4 borders).
    If set, bit 0 paralyses the video logic, unless there's real external
    sync.
*/
  short CyclesIn=(short)LINECYCLES;
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
  if(!OPTION_C3 &&(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_SYNCMODE))
      FrameEvents.Add(scan_y,CyclesIn,'S',NewSync&3); 
#endif
  m_SyncMode=(NewSync&3); // 2bit
#if defined(SSE_VID_STVL2)
  Stvl.exts=(m_SyncMode&1);
  Stvl.pal=((m_SyncMode&2)==2);
#endif
  if(OPTION_C2 && FetchingLine())
    CheckSideOverscan(); // force check to adapt timer B to right off...
  video_freq_idx=(screen_res>=2) ? 2 : ((NewSync&2)?0:1); //TODO
  //ASSERT(video_freq_idx>=0 && video_freq_idx<NFREQS);
  BYTE new_freq=Freq[video_freq_idx];
  //ASSERT(new_freq==50||new_freq==60||new_freq==MONO_HZ);
  if(video_freq!=new_freq)
    freq_change_this_scanline=true;  
#ifdef ENABLE_LOGFILE
  LOG_TO(LOGSECTION_VIDEO,EasyStr("VIDEO: ")+HEXSl(old_pc,6)+" - Changed frequency to "+new_freq+
    " at "+scanline_cycle_log());
#endif
  video_freq=new_freq;
  if(OPTION_C3)
    return;
  AddFreqChange(new_freq);
  AdaptScanlineValues(CyclesIn);
}


void TGlue::Update() {
/*  Update GLU timings according to ST model and wakeup state. We do it when
    player changes options, not at each scanline in CheckSideOverscan().
    v4: The new CPU interrupt model shifts some timings, including the HBL one,
    which is annoying as this is our base for 'emulator cycles' and would better
    stay 0. So we shift other timings instead!
*/
  char hbl_modifier;
  if(OPTION_68901) // is TRUE, if there was an option, depends on interrupt model
    hbl_modifier=0;
  else
    hbl_modifier=-8; // not a little shift either!
  if(Mmu.WS[OPTION_WS]==1 && IS_STF)       
    hbl_modifier+=4; // HBITMG.TOS
  char WU_res_modifier=Mmu.ResMod[IS_STE?3:OPTION_WS]; //-2, 0, 2
  char WU_sync_modifier=Mmu.FreqMod[IS_STE?3:OPTION_WS]; // 0 or 2
  // DE (Display Enable)
  ScanlineTiming[DE_ON][FREQ_71]=14+hbl_modifier;
  ScanlineTiming[DE_ON][FREQ_60]=60+hbl_modifier;
  ScanlineTiming[DE_ON][FREQ_50]=64+hbl_modifier;
  // Decision time, depends on wakestate
  ScanlineTiming[LINE_START_LIMIT][FREQ_71] 
    =ScanlineTiming[DE_ON][FREQ_71]+WU_res_modifier; // GLUE tests shift mode
  ScanlineTiming[LINE_START_LIMIT][FREQ_60]
    =ScanlineTiming[DE_ON][FREQ_60]+WU_sync_modifier; // GLUE tests sync mode
  ScanlineTiming[LINE_START_LIMIT][FREQ_50]
    =ScanlineTiming[DE_ON][FREQ_50]+WU_sync_modifier; // GLUE tests sync mode
  for(int f=0;f<NFREQS;f++)
  {
    ScanlineTiming[DE_OFF][f]=ScanlineTiming[DE_ON][f]+DE_cycles[f];
    ScanlineTiming[LINE_STOP_LIMIT][f] = ScanlineTiming[LINE_START_LIMIT][f]
      + DE_cycles[f];
    ScanlineTiming[LINE_STOP_LIMIT_MINUS2][f]
      =ScanlineTiming[LINE_STOP_LIMIT][f]-2;
  }
  ScanlineTiming[LINE_START_LIMIT_PLUS26][FREQ_71]
    =ScanlineTiming[LINE_START_LIMIT][FREQ_71]+26;
  ScanlineTiming[LINE_START_LIMIT_MINUS12][FREQ_71]
    =ScanlineTiming[LINE_START_LIMIT][FREQ_71]-12;
  ScanlineTiming[HSYNC_ON2][FREQ_50]=ScanlineTiming[LINE_START_LIMIT][FREQ_71]
    -54;
  ScanlineTiming[HSYNC_OFF2][FREQ_50]=ScanlineTiming[HSYNC_ON2][FREQ_50]+40;
  // On the STE, decision occurs sooner due to hardscroll possibility
  // but prefetch starts sooner only if HSCROLL <> 0.
  if(IS_STE)
  {
    ScanlineTiming[LINE_START_LIMIT][FREQ_71]= 
      ScanlineTiming[DE_ON_HSCROLL][FREQ_71]=ScanlineTiming[DE_ON][FREQ_71]-4-1; // 3615 nitrowave
    ScanlineTiming[LINE_START_LIMIT_MINUS12][FREQ_71]=0; //512 almost same time as hsync!
    ScanlineTiming[LINE_START_LIMIT][FREQ_60]=
      ScanlineTiming[DE_ON_HSCROLL][FREQ_60]=ScanlineTiming[DE_ON][FREQ_60]-16;
    ScanlineTiming[LINE_START_LIMIT][FREQ_50]=
      ScanlineTiming[DE_ON_HSCROLL][FREQ_50]=ScanlineTiming[DE_ON][FREQ_50]-16;
   ScanlineTiming[HSYNC_OFF2][FREQ_50]-=1; // approx.
  }
  ScanlineTiming[LINE_START_LIMIT_PLUS2][FREQ_71]
    =ScanlineTiming[LINE_START_LIMIT][FREQ_71]+2;
  // HBLANK (decision)
  // There's a -4 difference for 60hz but timings are the same on STE
  // HBLANK in high res is ignored by the STF Shifter
  ScanlineTiming[HBLANK_OFF][FREQ_50]=36+hbl_modifier+WU_sync_modifier;
  ScanlineTiming[HBLANK_OFF][FREQ_60]=ScanlineTiming[HBLANK_OFF][FREQ_50]-4;
  // HSYNC
  // notice 472+40=512 hbl pending = start of scanline
  ScanlineTiming[HSYNC_ON][FREQ_50]=472+hbl_modifier; 
  ScanlineTiming[HSYNC_ON1][FREQ_50]=ScanlineTiming[HSYNC_ON][FREQ_50]
    +WU_sync_modifier;
  if(IS_STE)
  {
    ScanlineTiming[HSYNC_ON1][FREQ_50]-=2;  //?
    ScanlineTiming[HSYNC_ON2][FREQ_50]-=2;  //?
  }
  ScanlineTiming[HSYNC_ON][FREQ_60]=ScanlineTiming[HSYNC_ON][FREQ_50]-4;
  ScanlineTiming[HSYNC_ON1][FREQ_60]=ScanlineTiming[HSYNC_ON1][FREQ_50]-4;
  ScanlineTiming[HSYNC_OFF][FREQ_50]=ScanlineTiming[HSYNC_ON][FREQ_50]+40;
  ScanlineTiming[HSYNC_OFF][FREQ_60]=ScanlineTiming[HSYNC_ON][FREQ_60]+40;  
#ifdef SSE_BETA  //not used
  ScanlineTiming[HSYNC_ON2][FREQ_60]=ScanlineTiming[HSYNC_ON2][FREQ_50];
  ScanlineTiming[HSYNC_OFF2][FREQ_60]=ScanlineTiming[HSYNC_OFF2][FREQ_50];
  ScanlineTiming[HSYNC_ON][FREQ_71]=204+hbl_modifier+WU_res_modifier;
  ScanlineTiming[HSYNC_OFF][FREQ_71]=ScanlineTiming[HSYNC_ON][FREQ_71]+24;
#endif
  // Reload video counter
  ScanlineTiming[RELOAD_SDP][FREQ_50]=70+hbl_modifier;
  ScanlineTiming[RELOAD_SDP][FREQ_60]=ScanlineTiming[RELOAD_SDP][FREQ_50] -4; //?
  ScanlineTiming[RELOAD_SDP][FREQ_71]=0;//14;
  // Enable VBI - VBITMG.TOS, idealised!
  ScanlineTiming[ENABLE_VBI][FREQ_50]=((hbl_modifier&4)||IS_STE) ? 68 : 64;
  ScanlineTiming[ENABLE_VBI][FREQ_60]=ScanlineTiming[ENABLE_VBI][FREQ_50]-4;
  ScanlineTiming[ENABLE_VBI][FREQ_71]=0;
  // 508 or 512 cycles?
  cycle_of_scanline_length_decision=62+hbl_modifier+WU_sync_modifier;
  if(IS_STE)
    cycle_of_scanline_length_decision+=4;
  // Top and bottom border
  ScanlineTiming[VERT_OVSCN_LIMIT][FREQ_50]
    =ScanlineTiming[HSYNC_ON1][FREQ_50]+40-2;
  ScanlineTiming[VERT_OVSCN_LIMIT][FREQ_60]
    =ScanlineTiming[VERT_OVSCN_LIMIT][FREQ_50]-4;
  // misc
  ScanlineTiming[LINE_PLUS_20A][FREQ_50]=12+hbl_modifier;
  ScanlineTiming[LINE_PLUS_20B][FREQ_50]=16+hbl_modifier;
  ScanlineTiming[LINE_PLUS_20C][FREQ_50]=10+hbl_modifier;
  ScanlineTiming[LINE_PLUS_20D][FREQ_50]=26+hbl_modifier;
  ScanlineTiming[LINE_PLUS_26A][FREQ_50]=20+hbl_modifier;
  ScanlineTiming[LINE_PLUS_26B][FREQ_50]=24+hbl_modifier;
  ScanlineTiming[LINE_PLUS_26C][FREQ_50]=28+hbl_modifier;
  ScanlineTiming[MEDRES_OA][FREQ_50]=24+hbl_modifier;
  ScanlineTiming[MEDRES_OB][FREQ_50]=48+hbl_modifier;
  ScanlineTiming[MEDRES_OC][FREQ_50]=16+hbl_modifier;
  ScanlineTiming[DESTAB_A][FREQ_50]=92+hbl_modifier;
//  ScanlineTiming[DESTAB_B][FREQ_50]=212+hbl_modifier;
  ScanlineTiming[DESTAB0][FREQ_50]=72+hbl_modifier;
  ScanlineTiming[STAB_A][FREQ_50]=440+hbl_modifier;
  ScanlineTiming[STAB_B][FREQ_50]=468+hbl_modifier;
  ScanlineTiming[STAB_C][FREQ_50]=472+hbl_modifier;
  ScanlineTiming[STAB_D][FREQ_50]=448+hbl_modifier;
  ScanlineTiming[RENDER_CYCLE][FREQ_50]=CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN
    +8+hbl_modifier;
  ScanlineTiming[LINE_PLUS_44_R][FREQ_50]=
    ScanlineTiming[LINE_STOP_LIMIT][FREQ_50]-WU_sync_modifier+WU_res_modifier+2;
  if(IS_STE)
    ScanlineTiming[LINE_PLUS_44_R][FREQ_50]-=2;
  if(OPTION_SHIFTER_WU<-SHIFTER_MAX_WU_SHIFT 
    || OPTION_SHIFTER_WU>SHIFTER_MAX_WU_SHIFT)
    OPTION_SHIFTER_WU=SHIFTER_DEFAULT_WAKEUP;
#if defined(SSE_VID_STVL1) 
  if(hStvl)
    StvlUpdate();
#endif
}


void TGlue::Vbl() {
  // event_vbl_interrupt() called at cycle 6X with STVL, 0 without
  if(!OPTION_C3) 
  {
    cpu_timer_at_start_of_hbl=time_of_next_event;
    scan_y=-scanlines_above_screen[video_freq_idx]; // needed for Debugger frame by frame C2
  }
  else if(video_freq_idx==2)
    scan_y--; // for traces
  //int h=0; scan_y/=h; //crash test
#if defined(SSE_HARDWARE_OVERSCAN)
  if(OPTION_HWOVERSCAN && SSEConfig.OverscanOn)
  {
    short start;
    // hack to get correct display
    if(COLOUR_MONITOR)
    {
      start=(border==3)?-39:-30;
      video_last_draw_line=245;
    }
    else
    {
      start=-30-1;
      video_last_draw_line=471;
    }
    scan_y=start;
    video_first_draw_line=start+1;
  }
#endif
  m_Status.hbi_done=m_Status.vc_reload_done=false;
  m_Status.vbl_done=true;
  // Stopping now if emu thread makes snapshots more compatible
  if(m_Status.stop_emu==1)
    runstate=RUNSTATE_STOPPING;
  else
  {
    if(m_Status.stop_emu==2) // only possible with option No OSD on stop
      m_Status.stop_emu--;
#if defined(SSE_STATS) // reset one frame stats
    Stats.nPal=Stats.nTimerbtick=Stats.nBlit1=Stats.nHbi1=Stats.nReadvc1
      =Stats.nScreensplit1=Stats.nLinePlus16=0;
#endif
  }
#if defined(SSE_DEBUGGER_FAKE_IO) && defined(SSE_DEBUGGER_OSD_CONTROL)
  if(OSD_MASK2&OSD_CONTROL_MODES)
    TRACE_OSD("R%d S%d",Shifter.m_ShiftMode,m_SyncMode);
#endif
  // pushing current freq to avoid spurious overscan detection on timer rollover
  // we don't mess with act here, it would trigger spurious interrupts!
  AddFreqChange(video_freq); 
  AddShiftModeChange(m_ShiftMode); 
}
