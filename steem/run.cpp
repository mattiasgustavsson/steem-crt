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
FILE: run.cpp
DESCRIPTION: This file contains Steem's run() function, the routine that
actually makes Steem go (optionally in a distinct thread).
Also included here is the code for the event system that allows time-critical
Steem functions (such as VBLs, HBLs and MFP timers) to be scheduled to the
nearest cycle. Speed limiting and drawing is also handled here, in 
event_scanline and event_vbl_interrupt.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <computer.h>
#include <debug.h>
#include <gui.h>
#include <draw.h>
#include <osd.h>
#include <interface_stvl.h>
#include <stjoy.h>
#include <shortcutbox.h>
#include <palette.h>
#include <translate.h>
#include <debugger.h>
#include <debug_framereport.h>
#include <infobox.h>


EVENTPROC event_mfp_timer_timeout[4]={event_timer_a_timeout,
  event_timer_b_timeout,event_timer_c_timeout,event_timer_d_timeout};
EVENTPROC event_vector;
#ifdef SHOW_DRAW_SPEED
extern HWND  StemWin;
#endif
COUNTER_VAR time_of_next_event;
COUNTER_VAR cpu_time_of_start_of_event_plan;
COUNTER_VAR time_of_next_timer_b=0;
COUNTER_VAR time_of_last_hbl_interrupt, time_of_last_vbl_interrupt;
COUNTER_VAR cpu_timer_at_res_change;
int runstate;
#ifdef DEBUG_BUILD
int stem_runmode;
#endif
// fast_forward_max_speed=(1000 / (max %/100)); 0 for unlimited
int fast_forward=0,fast_forward_max_speed=0;
int slow_motion=0,slow_motion_speed=100;
int run_speed_ticks_per_second=1000;
int run_start_time;
DWORD avg_frame_time=0,avg_frame_time_timer,frame_delay_timeout,timer;
DWORD speed_limit_wait_till;
int avg_frame_time_counter=0;
DWORD auto_frameskip_target_time;
int frameskip=1,frameskip_count=1,nframes_this_second;
DWORD start_of_this_second;
bool disable_speed_limiting=false;
bool fast_forward_stuck_down=false;
bool flashlight_flag=false;
UNIX_ONLY( bool RunWhenStop=false; )


void run() {
#ifdef WIN32  
  HWND h_fs_win_button=(OptionBox.Handle!=NULL)
    ? GetDlgItem(OptionBox.Handle,IDC_TOGGLE_FULLSCREEN) : NULL;
  if(h_fs_win_button)
    EnableWindow(h_fs_win_button,FALSE);
#endif    
#ifdef SSE_HD6301_LL
  Ikbd.Crashed=0;
  mousek=0;
#endif
  StatusInfo.MessageIndex=TStatusInfo::MESSAGE_NONE;
#if !defined(SSE_GUI_TOOLBAR)
  REFRESH_STATUS_BAR;
#endif
#if defined(SSE_EMU_THREAD)
  SoundLock.Unlock();
  VideoLock.Unlock();
#endif
#ifdef UNIX // temp?
  OPTION_SAMPLED_YM=OPTION_MAME_YM=(Psg.LoadFixedVolTable()==true);
  SSEOptions.low_pass_frequency=YM_LOW_PASS_FREQ;
#endif
  // before each run, to avoid some crashes (mostly snapshot-related),
  // restore the stable parts of emulation objects
  ComputerRestore();
#if defined(SSE_STATS_CPU)
  Stats.myCpuUsage.GetUsage(); // mark start of emulation
#endif
  bool ExcepHappened;
  Disp.RunStart();
  GUIRunStart();
  DEBUG_ONLY(debug_run_start(); )
#ifndef DISABLE_STEMDOS
  if(LITTLE_PC==rom_addr) 
    stemdos_set_drive_reset();
#endif
  ikbd_run_start(LITTLE_PC==rom_addr);
  runstate=RUNSTATE_RUNNING;
#if defined(SSE_GUI_TOOLBAR)
  REFRESH_STATUS_BAR;
#endif
  Glue.m_Status.stop_emu=0;
#ifdef WIN32
  // Make timer accurate to 1ms
  TIMECAPS tc;
  tc.wPeriodMin=1;
  timeGetDevCaps(&tc,sizeof(TIMECAPS));
  timeBeginPeriod(tc.wPeriodMin);
#endif
  timer=timeGetTime();
  Sound_Start();
  Glue.AddFreqChange(Glue.video_freq);
  init_screen();
  if(bad_drawing==0)
  {
    draw_begin();
    DEBUG_ONLY(debug_update_drawing_position(); )
  }
  PortsRunStart();
  DEBUG_ONLY(stem_runmode=STEM_MODE_CPU;)
#ifdef ENABLE_LOGFILE
  log_write(">>> Start Emulation <<<");
#endif
#if defined(SSE_DEBUG_TRACE)
  Debug.TraceGeneralInfos(TDebug::START);
#endif
  DEBUG_ONLY(debug_first_instruction=true; ) // Don't break if running from breakpoint
  timer=timeGetTime();
  run_start_time=timer; // For log speed limiting
  osd_init_run(true);
  frameskip_count=1;
  if(Glue.video_freq)
    speed_limit_wait_till=timer+(run_speed_ticks_per_second/Glue.video_freq);
  nframes_this_second=0+1;
#if defined(SSE_OSD_FPS_INFO)
  Debug.frame_no_change=0;
#endif
  start_of_this_second=timer;//timeGetTime();
  avg_frame_time_counter=0;
  avg_frame_time_timer=timer;
  // I don't think this can do any damage now, it just checks its
  // list and updates cpu_timer and cpu_cycles
  DEBUG_ONLY(prepare_next_event(); )
  ioaccess=0;
  if(Blitter.Busy)
    Blitter_Draw();
#if defined(SSE_EMU_THREAD) && !defined(_DEBUG) // want to see exception
  try  // apart thread or not
#endif
  {
#if defined(SSE_EMU_THREAD) && defined(SSE_MAIN_LOOP3) 
    //_se_translator_function old_se_f=
    _set_se_translator(trans_func);
#endif
    //for(;;); // TEST stuck in infinite loop
    //int a=0; int b=5/a;  printf("yoho %d",b);// TEST SEH exception
    do {
      ExcepHappened=0;
      TRY_M68K_EXCEPTION
        while(runstate==RUNSTATE_RUNNING) 
        {
          // cpu_cycles is the amount of cycles before next event.
          // So it is *decremented* by instruction timings, not incremented.
          while(cpu_cycles>0&&runstate==RUNSTATE_RUNNING)
          {
#ifdef DEBUG_BUILD
            pc_history_y[pc_history_idx]=scan_y;
            pc_history_c[pc_history_idx]=(short)LINECYCLES;
            pc_history[pc_history_idx++]=(pc&0x00FFFFFF);
            if(pc_history_idx>=HISTORY_SIZE) 
              pc_history_idx=0;
#endif
            m68kProcess();
#ifdef DEBUG_BUILD
            debug_first_instruction=0;
            CHECK_BREAKPOINT
#endif
          }//wend
#ifdef DEBUG_BUILD
          if(runstate!=RUNSTATE_RUNNING) 
            break;
          stem_runmode=STEM_MODE_INSPECT;
#endif
          for(int i=0;cpu_cycles<=0 && (runstate==RUNSTATE_RUNNING);i++) // get out of buggy loop
          {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
            if(TRACE_MASK2&TRACE_CONTROL_EVENT)
              TRACE_EVENT(event_vector);
#endif
            event_vector();
            prepare_next_event();
            if(//!OPTION_EMUTHREAD && // also for emuthread: avoids killing the thread
              i==10) // get out of buggy loop
            {
              PeekEvent();
              i=0; // i business to avoid load
            }
          }
          CHECK_BREAKPOINT
          DEBUG_ONLY(stem_runmode=STEM_MODE_CPU; )
        }//while (runstate==RUNSTATE_RUNNING)
      CATCH_M68K_EXCEPTION
        m68k_exception e=ExceptionObject;
        ExcepHappened=true;
#ifndef DEBUG_BUILD
        e.crash();
#else
        stem_runmode=STEM_MODE_INSPECT;
        bool alertflag=false;
        if(crash_notification!=CRASH_NOTIFICATION_NEVER)
        {
          alertflag=true;
          TRY_M68K_EXCEPTION
            if(e.bombs>8)
              alertflag=false;
#if defined(SSE_DEBUGGER_EXCEPTION_NOT_TOS)
            else if(crash_notification==CRASH_NOTIFICATION_NOT_TOS
              && e.u_pc.d32>=rom_addr && e.u_pc.d32<rom_addr+tos_len) {
              alertflag=false;
#endif
            }
            else if(crash_notification==CRASH_NOTIFICATION_BOMBS_DISPLAYED
              && LPEEK(e.bombs*4)<rom_addr) //not bombs routine
              alertflag=false;
          CATCH_M68K_EXCEPTION
            alertflag=true;
          END_M68K_EXCEPTION
        }
        DEBUG_ONLY(stem_runmode=STEM_MODE_CPU;) // avoid wrong timing in bus_jam
        if(alertflag==0)
          e.crash();
        else
        {
          bool was_locked=draw_lock;
          draw_end();
          draw(false);
#if defined(SSE_DEBUGGER_ALERTS_IN_STATUS_BAR)
          char crash_msg[60];
          sprintf(crash_msg,"Exception %d bombs",e.bombs); // can become HALT
          BoilerStatusBarMsg(crash_msg);
          runstate=RUNSTATE_STOPPING;
          e.crash(); //crash
          debug_trace_crash(e);
          ExcepHappened=0;
          if(Debug.PromptOnBreakpoint)
#endif
          {
            if(IDOK==Alert(
              "Exception - do you want to crash (OK)\nor trace? (CANCEL)",
              EasyStr("Exception ")+e.bombs,MB_OKCANCEL|MB_ICONEXCLAMATION)) 
            {
                e.crash();
                if(was_locked)
                  draw_begin();
            }
            else
            {
              runstate=RUNSTATE_STOPPING;
              e.crash(); //crash
              debug_trace_crash(e);
              ExcepHappened=0;
            }
          }
        }
        if(debug_num_bk)
          breakpoint_check();
        if(runstate!=RUNSTATE_RUNNING)
          ExcepHappened=0;
#endif
      END_M68K_EXCEPTION
    } while(ExcepHappened);
    //_set_se_translator(old_se_f);
#if defined(SSE_STATS)
    Stats.run_time=timeGetTime()-run_start_time; // milliseconds run
#if defined(SSE_STATS_CPU)
    Stats.tCpuUsage=Stats.myCpuUsage.GetUsage(); // collect CPU%
#endif
    if(InfoBox.Page==INFOPAGE_STATS && InfoBox.IsVisible()) // show?
      InfoBox.CreatePage(InfoBox.Page); // note exceptions caught by run() handler
#endif
  }
#if defined(SSE_EMU_THREAD) // apart thread or not
#if defined(SSE_MAIN_LOOP3)
  catch(SE_Exception e) {
    e.handle_exception();
  }
#endif
#if defined(BCC_BUILD) || defined(MINGW_BUILD)
  catch(...) {
    Alert(T("Unknown exception"),T("STEEM CRASHED!"),MB_ICONEXCLAMATION|MB_OK);
    TRACE2("Unknown exception\n");
  }
#endif
#endif
  PortsRunEnd();
  Sound_Stop();
  if(FullScreen)
    Disp.RunEnd();
  runstate=RUNSTATE_STOPPED;
#if defined(SSE_GUI_TOOLBAR)
  REFRESH_STATUS_BAR;
#endif
  Glue.m_Status.stop_emu=0;
  GUIRunEnd();
  draw_end();
  CheckResetDisplay();
#ifdef DEBUG_BUILD
  if(redraw_on_stop)
    draw(0);
  else if(runstate_why_stop=="Run until")
    update_display_after_trace();
  debug_run_until=DRU_OFF;
  debug_run_end();
#endif
#ifdef ENABLE_LOGFILE
  log_write(">>> Stop Emulation <<<");
#endif
#if defined(SSE_DEBUG_TRACE)
  Debug.TraceGeneralInfos(TDebug::STOP);
#endif
#ifdef WIN32
  timeEndPeriod(tc.wPeriodMin); // Finished with accurate timing
#endif
  ONEGAME_ONLY(OGHandleQuit(); )
#ifdef UNIX
  if(RunWhenStop)
  {
    CLICK_PLAY_BUTTON();
    RunWhenStop=0;
  }
#endif
#ifdef WIN32
  h_fs_win_button=(OptionBox.Handle!=NULL)
    ? GetDlgItem(OptionBox.Handle,IDC_TOGGLE_FULLSCREEN) : NULL;
  if(h_fs_win_button)
    EnableWindow(h_fs_win_button,TRUE);
  RunMessagePosted=false;
#endif
}


void prepare_next_event() {
  if(OPTION_C3)
  {
    event_vector=event_dummy;
    time_of_next_event=ACT+EIGHT_MILLION;
  }
  else
    Glue.GetNextVideoEvent();
  // check timers for timeouts
  PREPARE_EVENT_CHECK_FOR_TIMER_TIMEOUTS(0);
  PREPARE_EVENT_CHECK_FOR_TIMER_TIMEOUTS(1);
  PREPARE_EVENT_CHECK_FOR_TIMER_TIMEOUTS(2);
  PREPARE_EVENT_CHECK_FOR_TIMER_TIMEOUTS(3);
  if(!OPTION_C3)
  {
    PREPARE_EVENT_CHECK_FOR_TIMER_B;
  }
  PREPARE_EVENT_CHECK_FOR_DEBUG;
  PREPARE_EVENT_CHECK_FOR_PASTI;
  PREPARE_EVENT_CHECK_FOR_FLOPPY;
  PREPARE_EVENT_CHECK_FOR_ACIA;
  // It is safe for events to be in past, whatever happens events
  // cannot get into a constant loop.
  // If a timer is set to shorter than the time for an MFP interrupt then it will
  // happen a few times, but eventually will go into the future (as the interrupt can
  // only fire once, when it raises the IPL).
  //ASSERT(!(time_of_next_event&1));
  int oo=(int)(time_of_next_event-cpu_timer);
  // cpu_timer must always be set to the next 4 cycle boundary after time_of_next_event
  //SS: this is still true after rounding refactoring
  //guess (!) it's because it enforces CPU R/W cycle, which is still 4 cycles
  //(clocks) in our reckoning, but still don't see how exactly
  oo=(oo+3) & -4;
  cpu_cycles+=oo;cpu_timer+=oo;
}


#define LOGSECTION LOGSECTION_MFP_TIMERS

inline void handle_timeout(int tn) {
  DBG_LOG(Str("MFP: Timer ")+char('A'+tn)+" timeout at "+ABSOLUTE_CPU_TIME
    +" timeout was "+mfp_timer_timeout[tn]+" period was "+mfp_timer_period[tn]);
  if(mfp_timer_period_change[tn])
  {
    MFP_CALC_TIMER_PERIOD(tn);
    mfp_timer_period_change[tn]=0;
  }
  act=ACT;
  COUNTER_VAR new_timeout=mfp_timer_timeout[tn];
  COUNTER_VAR cmp;
  if(OPTION_68901)
    new_timeout-=Mfp.Wobble[tn]; //get the correct timing (no drift)
  do
  {
    new_timeout+=mfp_timer_period[tn];
    cmp=new_timeout-act;
  } while(cmp<0 || cmp==0&&cpu_cycles_multiplier<32.0);
  if(OPTION_68901)
  {
    mfp_timer_period_current_fraction[tn]+=mfp_timer_period_fraction[tn]; 
    // this guarantees that we're always at the right cycle, despite
    // the inconvenience of a ratio
    if(mfp_timer_period_current_fraction[tn]>=1000) {
      mfp_timer_period_current_fraction[tn]-=1000;
      new_timeout+=1; 
    }
    new_timeout+=Mfp.Wobble[tn]=(rand() % MFP_TIMERS_WOBBLE);
    Mfp.Counter[tn]=Mfp.reg[MFPR_TADR+tn]; // load counter
    BYTE prescale_index=(Mfp.get_timer_control_register(tn)&7);
    Mfp.Prescale[tn]=(BYTE)mfp_timer_prescale[prescale_index]; // load prescale (bad if 0)
  }//C2
  mfp_interrupt_pend(mfp_timer_irq[tn],mfp_timer_timeout[tn]);
  //ASSERT(new_timeout-ACT>0);
  mfp_timer_timeout[tn]=new_timeout;
#if defined(SSE_STATS)
  Stats.nMfpTimeout[tn]++;
  DWORD divisor=Mfp.Prescale[tn]*BYTE_00_TO_256(Mfp.Counter[tn]);
  if(divisor) // should be
    Stats.fTimer[tn]=MFP_XTAL/divisor; //Hz
#endif
}


void event_timer_a_timeout() {
  handle_timeout(0);
}


void event_timer_b_timeout() {
  handle_timeout(1);
}


void event_timer_c_timeout() {
  handle_timeout(2);
}


void event_timer_d_timeout() {
  handle_timeout(3);
}


#undef LOGSECTION
#define LOGSECTION LOGSECTION_INTERRUPTS

void event_timer_b() {
#if defined(SSE_VID_STVL1) 
  if(OPTION_C3)
    time_of_next_timer_b=ACT+Stvl.tick8;
#endif
  Mfp.time_of_last_tb_tick=time_of_next_timer_b;
  if(!OPTION_C3&&scan_y<video_first_draw_line)
  {
    if(!OPTION_68901)
      time_of_next_timer_b=cpu_timer_at_start_of_hbl+160000*SSEConfig.CpuBoost;
  }
  else if(OPTION_C3||scan_y<video_last_draw_line) 
  {
    if(Mfp.reg[MFPR_TBCR]==8) 
    { // timer B tick
#if defined(SSE_STATS)
      Stats.nTimerbtick++;
#endif
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
      if(FRAME_REPORT_MASK2 & FRAME_REPORT_MASK_TIMER_B)
        FrameEvents.Add(scan_y,(short)(time_of_next_timer_b-LINECYCLE0), "TB",
        mfp_timer_counter[1]/64);
#endif
      mfp_timer_counter[1]-=64;
      Mfp.tbctr_old=Mfp.Counter[1];
      Mfp.Counter[1]--;
      LOG_TO(LOGSECTION_MFP_TIMERS,EasyStr("MFP: Timer B counter decreased to ")
        +(mfp_timer_counter[1]/64)+" at "+scanline_cycle_log());
      if(mfp_timer_counter[1]<64) 
      {
#if defined(SSE_STATS)
        Stats.nTimerb++;
#endif
        DBG_LOG(EasyStr("MFP: Timer B timeout at ")+scanline_cycle_log());
#ifdef SSE_DEBUG
#if defined(SSE_DEBUGGER_FAKE_IO) //timers only when checked in mask
        if(TRACE_MASK2&TRACE_CONTROL_IRQ_TB)
#endif
          if(mfp_interrupt_enabled[8]) 
            TRACE_LOG("F%d y%d c%d Timer B pending\n",TIMING_INFO); //?
#endif
        mfp_timer_counter[1]=BYTE_00_TO_256(Mfp.reg[MFPR_TBDR])*64;
        Mfp.Counter[1]=Mfp.reg[MFPR_TBDR];
        mfp_interrupt_pend(MFP_INT_TIMER_B,time_of_next_timer_b);
      }
#if defined(SSE_INT_MFP_EVENT_IRQ) // interrupts in event count mode
/*  Besides generating a count pulse, the active transition of the auxiliary
    input signal will also produce an interrupt on the I3 or I4 interrupt
    channel, if the interrupt channel is enabled.
*/
      mfp_interrupt_pend(3,time_of_next_timer_b);
#endif
    }
    if(!OPTION_68901 && !OPTION_C3)
      time_of_next_timer_b=cpu_timer_at_start_of_hbl
      +cpu_cycles_from_hbl_to_timer_b
      +scanline_time_in_cpu_cycles_at_start_of_vbl + TB_TIME_WOBBLE;
  }
  else
  {
    if(!OPTION_68901)
      time_of_next_timer_b=cpu_timer_at_start_of_hbl+160000*SSEConfig.CpuBoost;
  }
  if(OPTION_68901 && !OPTION_C3)
  {
    time_of_next_timer_b=cpu_timer_at_start_of_hbl+160000*SSEConfig.CpuBoost;  //put into future
    if(Mfp.reg[MFPR_AER]&8)
      Glue.m_Status.timerb_start=1;
    else
      Glue.m_Status.timerb_end=1;
  }
}

#undef LOGSECTION


void event_scanline_sub() {
/*  We take some tasks out of event_scanline(), so we can execute them from
    event_vbl_interrupt().
*/
#define LOGSECTION LOGSECTION_AGENDA
  CHECK_AGENDA;
#undef LOGSECTION
#if defined(SSE_HD6301_LL)
  if(OPTION_C1)
  {
    //ASSERT(HD6301_OK);
    //ASSERT(mode==STEM_MODE_CPU);
    hd6301_run_cycles(ACT);
    if(Ikbd.Crashed DEBUG_ONLY(&& stem_runmode==STEM_MODE_CPU))
    {
      TRACE("6301 CRASH\n");
      REFRESH_STATUS_BAR;
      runstate=RUNSTATE_STOPPING;
    }
  }
#endif
#if defined(SSE_DISK_CAPS)
  if(Caps.Active==1)
    Caps.Hbl();
#endif
  if(IS_STE && ste_sound_on_this_screen)
  {
#if defined(SSE_VID_STVL_SREQ)
    if(!OPTION_C3 || SSEConfig.Stvl<0x101)
#endif
    Mmu.sound_fetch();
    Shifter.sound_play();
#if defined(SSE_VID_STVL_SREQ)
    if(!OPTION_C3 || SSEConfig.Stvl<0x101)
#endif
    if(!Glue.FetchingLine()) // no DE this line
      Mmu.sound_fetch(); // make sure FIFO is filled
  }
}


void event_scanline() {
  event_scanline_sub();
#if defined(SSE_VID_STVL1)
  if(OPTION_C3)
  { // copy scanline from  STVL buffer to video memory, twice if line doubled
    time_of_next_event=ACT+Stvl.tick8;
    for(int i=0;i<Stvl.hsync;i++) // in case of 'No Buddies Land' type 0byte lines
    if(draw_lock && Stvl.render_y>render_vstart && Stvl.render_y<=render_vend)
    {
      BYTE *source_start=Stvl.draw_mem_ptr_min+render_hstart;
      BYTE *source_end=source_start+render_scanline_length;
      if(draw_mem_line_ptr>=draw_mem && source_end<Stvl.draw_mem_ptr_max) //+8
      {
        if(draw_mem_line_ptr+render_scanline_length<Disp.VideoMemoryEnd)
          memcpy(draw_mem_line_ptr,source_start,render_scanline_length);
        draw_mem_line_ptr+=draw_line_length;
        if(COLOUR_MONITOR && (draw_med_low_double_height||SCANLINES_OK))
        {
          if(!SCANLINES_OK 
            && draw_mem_line_ptr+render_scanline_length<Disp.VideoMemoryEnd)
          {
            memcpy(draw_mem_line_ptr,source_start,render_scanline_length);
          }
          draw_mem_line_ptr+=draw_line_length;
        }
#if defined(SSE_DEBUGGER)
        if(draw_mem_line_ptr+render_scanline_length<Disp.VideoMemoryEnd)
          ZeroMemory(draw_mem_line_ptr,render_scanline_length);
        Stvl.dbg_npixels=0;
#endif
      }
    }
    time_of_next_timer_b=time_of_next_event+160000*SSEConfig.CpuBoost;  //put into future
#if defined(SSE_DEBUGGER_TOPOFF) // many false alerts just like with C2
    if((DEBUGGER_CONTROL_MASK2&DEBUGGER_CONTROL_TOPOFF)
      && scan_y==-29 && !Stvl.vde && freq_change_this_scanline)
    {
      runstate=RUNSTATE_STOPPING;
      SET_WHY_STOP("Top off missed");
    }
    if((DEBUGGER_CONTROL_MASK2&DEBUGGER_CONTROL_BOTTOMOFF)
      && scan_y==200 && !Stvl.vde && freq_change_this_scanline)
    {
      runstate=RUNSTATE_STOPPING;
      SET_WHY_STOP("Bottom off missed");
    }
#endif
  }
  else
#endif
  if(scan_y<video_first_draw_line-1)
  {
    if(scan_y>=draw_first_scanline_for_border)
    {
      if(bad_drawing==0)
        Shifter.DrawScanlineToEnd();
      if(!OPTION_68901)
        time_of_next_timer_b=time_of_next_event+160000*SSEConfig.CpuBoost;  //put into future
    }
  }
  else if(scan_y<video_first_draw_line) 
  { //next line is first visible
    if(bad_drawing==0) 
      Shifter.DrawScanlineToEnd();
    if(!OPTION_68901)
      time_of_next_timer_b=time_of_next_event+cpu_cycles_from_hbl_to_timer_b
        +TB_TIME_WOBBLE;
  }
  else if(scan_y<video_last_draw_line-1) 
  {
    if(bad_drawing==0)
      Shifter.DrawScanlineToEnd();
    if(!OPTION_68901)
      time_of_next_timer_b=time_of_next_event+cpu_cycles_from_hbl_to_timer_b
      +TB_TIME_WOBBLE;
  }
  else if(scan_y<draw_last_scanline_for_border)
  {
    if(bad_drawing==0)
      Shifter.DrawScanlineToEnd();
    if(!OPTION_68901)
      time_of_next_timer_b=time_of_next_event+160000*SSEConfig.CpuBoost;  //put into future
  }
#ifdef ENABLE_LOGFILE
  LOG_TO(LOGSECTION_VIDEO,EasyStr("VIDEO: Event Scanline at end of line ")+scan_y+" sdp is $"+HEXSl(shifter_draw_pointer,6));
#endif
  if(OPTION_C2)
  {
    if(Glue.FetchingLine())
      Glue.EndHBL(); // check for +2 -2 errors + unstable Shifter
    if((scan_y==-30||scan_y==-1||scan_y==video_last_draw_line-1&&scan_y<245)
      &&Glue.CurrentScanline.Cycles>224)
      Glue.CheckVerticalOverscan(); // check top & bottom borders
  }
  if(freq_change_this_scanline) 
  {
    if(OPTION_C3||
      shifter_freq_change_time[shifter_freq_change_idx]<time_of_next_event-16
      && shifter_mode_change_time[shifter_mode_change_idx]
      <time_of_next_event-16)
    {
      freq_change_this_scanline=0;
    }
    if(draw_line_off)
    {
      palette_convert_all();
      draw_line_off=0;
    }
  }
  scanline_drawn_so_far=0;
#if 0 && defined(DEBUG_BUILD)
/*  Enforce register limitations, so that "report SDP" isn't messed up
    in the debug build.
*/
#if defined(SSE_MMU_MONSTER_ALT_RAM)
  if(mem_len<14*0x100000) 
#else
  if(mem_len<=FOUR_MEGS) 
#endif
    shifter_draw_pointer&=0x3FFFFE;
#endif
#if defined(SSE_VID_STVL1) 
  if(OPTION_C3)
    shifter_draw_pointer=shifter_draw_pointer_at_start_of_line=Stvl.vcount.d32;
  else
#endif
  if(!emudetect_falcon_mode && Glue.FetchingLine())
  {
#if 0
    //looks nice but takes more CPU power (another CheckSideOverscan round)
    Mmu.UpdateVideoCounter((short)LINECYCLES);
    shifter_draw_pointer=shifter_draw_pointer_at_start_of_line=Mmu.VideoCounter;
#else
    short added_bytes=Glue.CurrentScanline.Bytes;
    // extra words for HSCROLL are included in Bytes
    if(IS_STE && added_bytes && !Mmu.no_LW)
      added_bytes+=((WORD)LINEWID)<<1; 
    shifter_draw_pointer_at_start_of_line+=added_bytes;
#if defined(SSE_MMU_MONSTER_ALT_RAM)
  if(mem_len<14*0x100000) 
#else
  if(mem_len<=FOUR_MEGS) 
#endif
    shifter_draw_pointer_at_start_of_line&=0x3FFFFE; // Leavin' Teramis
    shifter_draw_pointer=shifter_draw_pointer_at_start_of_line;
#endif
  }
  else 
    shifter_draw_pointer_at_start_of_line=shifter_draw_pointer;
  Mmu.VideoCounter=shifter_draw_pointer_at_start_of_line;
  cpu_timer_at_start_of_hbl=time_of_next_event; 
#if defined(SSE_VID_STVL1)
  if(OPTION_C3)
  {
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
    if((FRAME_REPORT_MASK2 & FRAME_REPORT_MASK_SHIFTER_TRICKS_BYTES))
      FrameEvents.Add(scan_y,Stvl.video_linecycles,'#',Stvl.dbg_fetched_words*2);
#endif
    Glue.CurrentScanline.Cycles=Stvl.video_linecycles;
    scan_y++;
  }
  else
#endif
    Glue.IncScanline(); // will call Shifter.IncScanline()
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
  if(Glue.FetchingLine() && (FRAME_REPORT_MASK1&FRAME_REPORT_MASK_VC_LINES)) 
    FrameEvents.Add(scan_y,0,"VC",shifter_draw_pointer); 
#endif
#ifdef DEBUG_BUILD
  if(debug_run_until==DRU_SCANLINE)
  {
    if(debug_run_until_val==scan_y)
    {
      if(runstate==RUNSTATE_RUNNING) 
      {
        runstate=RUNSTATE_STOPPING;
        runstate_why_stop="Run until";
      }
#if defined(SSE_DEBUGGER_FRAME_REPORT)
      FrameEvents.Report();
#endif
    }
  }
#endif
  if(OPTION_68901||OPTION_C3 // always TRUE
    || abs_quick(cpu_timer_at_start_of_hbl-time_of_last_hbl_interrupt)
    >=CYCLES_FROM_START_OF_HBL_IRQ_TO_WHEN_PEND_IS_CLEARED)
  {
    Glue.hbl_pending=true;
    Glue.hbl_pending_time=cpu_timer_at_start_of_hbl;
    update_ipl(Glue.hbl_pending_time);
  }
#if defined(SSE_VID_DD_3BUFFER_WIN)
  // DirectDraw Check for Window VSync at each ST scanline!
  if(OPTION_3BUFFER_WIN && !FullScreen)
    Disp.BlitIfVBlank();
#endif
  Glue.m_Status.hbi_done=false;
  Glue.m_Status.scanline_done=true;
  Glue.m_Status.vc_reload_done=false;
#if defined(SSE_VID_STVL1)
  if(OPTION_C3)
    prepare_next_event();
#endif
}

#undef LOGSECTION


void event_start_vbl() {
  // This happens about 60 cycles into scanline 247 (50Hz)
  Glue.m_Status.vc_reload_done=true; // checked this line
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
  if(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_VIDEOBASE)
    FrameEvents.Add(scan_y,(short)LINECYCLES,'r',video_freq_idx); // "reload"
#endif
  Glue.vsync=true;
#if defined(SSE_OSD_FPS_INFO)
  if(OPTION_OSD_FPSINFO)
    Debug.vcount_at_vsync=shifter_draw_pointer; // can be >vbase+32KB if overscan
#endif
  // As soon as VSYNC is asserted, the MMU keeps on copying VBASE to VCOUNT
  // We don't emulate the continuous copy but we copy at start and stop
  // (event_start_vbl(),event_trigger_vbi())
  Mmu.VideoCounter=shifter_draw_pointer=vbase;
  shifter_draw_pointer_at_start_of_line=shifter_draw_pointer;
  shifter_pixel=shifter_hscroll;
  left_border=right_border=BORDER_SIDE;
  Glue.m_Status.vbl_done=false;
  //TRACE("F%d y%d vcount %d vsync on\n",FRAME,scan_y, Glue.VCount);
}


void event_vbl_interrupt() {
  //TRACE("F%d y%d finish frame\n",FRAME,scan_y);
  // called  after the last scanline of the frame, before the vertical interrupt
  // STVL: called at vertical interrupt
#if defined(SSE_DONGLE)
/*  When pressing some button of his cartridge, the player triggered an MFP
    interrupt.
    By releasing the button, the concerned bit should change state. We use
    no counter but do it at first VBL for simplicity.
*/
  if(cart)
  {
    switch(DONGLE_ID) {
#if defined(SSE_DONGLE_URC) 
    case TDongle::URC:
      if(!(Mfp.reg[MFPR_GPIP]&0x40))
        mfp_gpip_set_bit(MFP_GPIP_RING_BIT,true); // Ultimate Ripper
      break;
#endif
#if defined(SSE_DONGLE_MULTIFACE) // cart + monochrome
    case TDongle::MULTIFACE:
      if(!(Mfp.reg[MFPR_GPIP]&0x80))
        mfp_gpip_set_bit(MFP_GPIP_MONO_BIT,true);
      break;
#endif
    }
  }//if
#endif
/*  With GLU/video event refactoring, we call event_scanline() one time fewer,
    if we did now it would mess up some timings, so we call the sub
    with some HBL-dependent tasks: DMA sound, HD6301 & CAPS emu.
    Important for Relapse STE sound
*/
#if defined(SSE_VID_STVL1)
  if(OPTION_C3)
    time_of_next_event=ACT+Stvl.tick8;
  else
#endif
  {
    event_scanline_sub();
    Glue.VCount=0;
  }
#if defined(STEEM_CRT)
  bool VSyncing=((OPTION_WIN_VSYNC)
    && (fast_forward==0&&slow_motion==0));
#elif defined(SSE_VID_VSYNC_WINDOW)
  bool VSyncing=((OPTION_WIN_VSYNC&&!FullScreen||FSDoVsync&&FullScreen)
    && (fast_forward==0&&slow_motion==0));
#else
  bool VSyncing=(FSDoVsync && FullScreen && fast_forward==0 && slow_motion==0);
#endif
  bool BlitFrame=0;
  if(!OPTION_C3&&extended_monitor==0)
  {
    // Rest of screen is black
    INT_PTR x=Disp.VideoMemoryEnd-draw_dest_ad-1;
    if(x>0 && x<Disp.VideoMemorySize)
      ZeroMemory(draw_dest_ad,x);
    scanline_drawn_so_far=0;
    shifter_draw_pointer_at_start_of_line=shifter_draw_pointer;
  }
  //-------- display to screen -------
  LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: Finished frame, blitting at ")+(timeGetTime()-run_start_time)+" timer="+(timer-run_start_time));
  //ASSERT(draw_lock);
  if(draw_lock) 
  {
    draw_end();
    if(VSyncing==0&&!OPTION_3BUFFER_WIN)
      draw_blit();
    BlitFrame=true;
  }
  else if(bad_drawing&2) 
  {
//    TRACE2("F%d bad_drawing %d\n",FRAME,bad_drawing);
    // bad_drawing bits: & 1 - bad drawing option selected  & 2 - bad-draw next screen
    //                   & 4 - temporary bad drawing because of extended monitor.
    draw(0);
    bad_drawing&=(~2);
  }
  if(floppy_mediach[0]) 
    floppy_mediach[0]--;  //counter for media change
  if(floppy_mediach[1]) 
    floppy_mediach[1]--;  //counter for media change
#ifdef ENABLE_LOGGING
  LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: Finished blitting at ")+(timeGetTime()-run_start_time)+" timer="+(timer-run_start_time));
  log_to_section(LOGSECTION_VIDEO,EasyStr("VIDEO: VBL interrupt - next screen is in freq ")+Glue.video_freq);
#endif
#ifdef SHOW_DRAW_SPEED
  {
    HDC dc=GetDC(StemWin);
    if(dc!=NULL) {
      char buf[16];
      ultoa(avg_frame_time*10/12,buf,10);
      TextOut(dc,2,MENUHEIGHT+2,buf,strlen(buf));
      ReleaseDC(StemWin,dc);
    }
  }
#endif

  //------------ Shortcuts -------------
  if((--shortcut_vbl_count)<0) 
  {
    ShortcutsCheck();
    shortcut_vbl_count=SHORTCUT_VBLS_BETWEEN_CHECKS;
  }

  //------------- Auto Frameskip Calculation -----------
  if(frameskip==AUTO_FRAMESKIP) 
  { //decide if we are ahead of schedule
    if(fast_forward==0&&slow_motion==0&&VSyncing==0) 
    {
      timer=timeGetTime();
      if(timer<auto_frameskip_target_time) 
      {
        frameskip_count=1;   //we are ahead of target so draw the next frame
        speed_limit_wait_till=auto_frameskip_target_time;
      }
      else
        auto_frameskip_target_time+=(run_speed_ticks_per_second
        +(Glue.video_freq/2))/Glue.video_freq;
    }
    else if(VSyncing) 
    {
      frameskip_count=1;   //disable auto frameskip
      auto_frameskip_target_time=timer;
    }
  }

  int time_for_exact_limit=1;

  // Work out how long to wait until we start next screen
  LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: Getting ready to wait at ")+(timeGetTime()-run_start_time)+" timer="+(timer-run_start_time));
  if(slow_motion) 
  {
    int i=int((cut_slow_motion_speed)?cut_slow_motion_speed:slow_motion_speed);
    if(i&&Glue.video_freq)
      frame_delay_timeout=timer+(1000000/i)/Glue.video_freq;
    auto_frameskip_target_time=timer;
    frameskip_count=1;
  }
  else if((frameskip_count<=1||fast_forward)&&!disable_speed_limiting) 
  {
    frame_delay_timeout=speed_limit_wait_till;
    if(VSyncing)
      // Allow up to a 25% increase in run speed
      time_for_exact_limit=((run_speed_ticks_per_second+(Glue.video_freq/2))/Glue.video_freq)/4;
  }
  else
    frame_delay_timeout=timer;
  LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: Going to wait until ")
    +(frame_delay_timeout-run_start_time)+" timer="+(timer-run_start_time));

  //--------- Look for Windows messages ------------
  // not necessary if emu thread active
  int old_slow_motion=slow_motion;
  int m=0;

  for(;;)
  {
    timer=timeGetTime();

#if defined(SSE_VID_DD_3BUFFER_WIN)
    if(OPTION_3BUFFER_WIN&&!FullScreen)
      Disp.BlitIfVBlank();
#endif

    // Break if used up enough time and processed at least 3 messages
    if(int(frame_delay_timeout-timer)<=time_for_exact_limit && m>=3) break;

    // Get next message from the queue, if none left then go to the Sleep
    // routine in Windows to give other processes more time. Also do that
    // if more than 15 messages have been retrieved.
    // Don't go to Sleep if slow motion is on, that way the message to turn
    // it off can be dealt with instantly.
    LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: Getting message at ")+(timeGetTime()-run_start_time)+" timer="+(timer-run_start_time));
    // emu thread: take 1 max in case main loop is stuck (test)
    if(PeekEvent()==PEEKED_NOTHING||m>(OPTION_EMUTHREAD?0:15))
    {
      // Get more than 15 messages if slow motion is on, otherwise GUI will lock up
      // Should really do something to stop high CPU load when slow_motion is on
      if(slow_motion==0) 
      {
        if(old_slow_motion)
          // Don't sleep if you just turned slow motion off (stops annoying GUI delay)
          frame_delay_timeout=timer;
        break;
      }
    }
    m++;
  }
  if(new_n_cpu_cycles_per_second) 
  {
    if(new_n_cpu_cycles_per_second!=n_cpu_cycles_per_second) 
    {
      n_cpu_cycles_per_second=new_n_cpu_cycles_per_second;
      prepare_cpu_boosted_event_plans();
    }
    new_n_cpu_cycles_per_second=0;
  }
#ifdef ENABLE_LOGGING
  LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: Finished getting messages at ")+(timeGetTime()-run_start_time)+" timer="+(timer-run_start_time));
#endif
  // Eat up remaining time-time_for_exact_limit with Sleep
  int time_to_sleep=(int(frame_delay_timeout)-int(timeGetTime()))-time_for_exact_limit;

//time_to_sleep-=1;

  if(time_to_sleep>0)
  {
#ifdef ENABLE_LOGGING
    LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: Sleeping for ")+time_to_sleep);
#endif
#if defined(SSE_VID_DD_3BUFFER_WIN)
/*  This is the part responsible for high CPU use.
    Maybe check probability of VBLANK but it depends on HZ
*/
    if(OPTION_3BUFFER_WIN&&!FullScreen)
    {
      int limit=(int)(frame_delay_timeout)-(int)(time_for_exact_limit);
      do {
        Disp.BlitIfVBlank();
      } while((int)(timeGetTime())<limit);
    }
    else
#endif
      Sleep(DWORD(time_to_sleep)); // it is very approximate
  }
#if defined(SSE_STATS)
  else if(time_to_sleep<-10 && !fast_forward) // only when way off
  {
    Stats.nSlowdown++;
#if defined(SSE_OSD_DEBUGINFO)
    if(OPTION_OSD_DEBUGINFO)
      TRACE_OSD2("S");
#endif
  }
#endif
  DWORD now=timeGetTime(); // take time before vsync
  if(VSyncing && BlitFrame) 
  {
    Disp.VSync();
    timer=timeGetTime();
    draw_blit();
#ifdef ENABLE_LOGFILE
    if(timer>speed_limit_wait_till+5) {
      LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: !!!!!!!!! SLOW FRAME !!!!!!!!!"));
    }
    LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: Finished Vsynced frame at ")+(timeGetTime()-run_start_time)+" timer="+(timer-run_start_time));
#endif
  }
  else
  {
#ifdef ENABLE_LOGGING
    LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: Doing exact timing at ")+(timeGetTime()-run_start_time)+" timer="+(timer-run_start_time));
#endif
    // Wait until desired time (to nearest 1000th of a second).
    do {
      timer=timeGetTime();
#if defined(SSE_VID_DD_3BUFFER_WIN)
      if(OPTION_3BUFFER_WIN&&!FullScreen)
        Disp.BlitIfVBlank();
#endif
    } while(int(frame_delay_timeout-timer)>0);
#ifdef ENABLE_LOGGING
    LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: Finished speed limiting at ")+(timer-run_start_time));
#endif
    // For some reason when we get here timer can be > frame_delay_timeout, even if
    // we are running very fast. This line makes it so we don't lose a millisecond
    // here and there.
    if(time_to_sleep>0) 
      timer=frame_delay_timeout;
  }
#if defined(SSE_EMU_THREAD)
  if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING && VideoLock.blocked)
    VideoLock.Acknowledge();
#endif

  //-------------- Pause Steem -------------
  if(GUIPauseWhenInactive()) 
  {
    timer=timeGetTime();
    avg_frame_time_timer=timer;
    avg_frame_time_counter=0;
    auto_frameskip_target_time=timer;
  }
  if(floppy_access_ff_counter>0) 
  {
    floppy_access_ff_counter--;
    if(fast_forward==0&&floppy_access_ff_counter>0)
    {
      fast_forward_change(true,0);
      floppy_access_started_ff=true;
    }
    else if(fast_forward && floppy_access_ff_counter==0
      &&floppy_access_started_ff)
      fast_forward_change(0,0);
  }
  //--------- Work out avg_frame_time (for OSD) ----------
  if(avg_frame_time==0)
    avg_frame_time=(timer-avg_frame_time)*12;
  else if(++avg_frame_time_counter>=12) 
  {
    avg_frame_time=timer-avg_frame_time_timer; //take average of frame time over 12 frames, ignoring the time we've skipped
    avg_frame_time_timer=timer;
    avg_frame_time_counter=0;
  }
  JoyGetPoses(); // Get the positions of all the PC joysticks
  if(slow_motion) 
  {
    // Extra screenshot check (so you actually take a picture of what you see)
    frameskip_count=0;
    ShortcutsCheck();
    if(DoSaveScreenShot&1) 
    {
      Disp.SaveScreenShot();
      DoSaveScreenShot&=~1;
    }
    // without this, Steem will make up the slow down as soon as slow_motion is cleared!
    start_of_this_second=timer;
    nframes_this_second=0;
  }
  IKBD_VBL();    // Handle ST joysticks and mouse
#if defined(SSE_HD6301_LL)
  if(OPTION_C1)
    Ikbd.Vbl();
#endif
  RS232_VBL();   // Update all flags, check for the phone ringing
  Sound_VBL();   // Write a VBLs worth + a bit of samples to the sound card
#if defined(SSE_DRIVE_SOUND)
/*  We don't check the option here because we may have to suddenly stop
    motor sound loop.
*/
  FloppyDrive[0].Sound_CheckMotor();
  FloppyDrive[1].Sound_CheckMotor();
#endif
#if defined(SSE_EMU_THREAD)
  if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING && SoundLock.blocked)
    SoundLock.Acknowledge();
#endif
  ste_sound_channel_buf_idx=0;  //need to maintain this even if sound off
  ste_sound_on_this_screen=(Mmu.sound_control&BIT_0)||Shifter.sound_fifo_idx;
#ifdef ENABLE_LOGGING
  LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: Finished event_vbl tasks at ")+(timeGetTime()-run_start_time)+" timer="+(timer-run_start_time));
#endif
  //---------- Frameskip -----------
#undef LOGSECTION 
#define LOGSECTION LOGSECTION_VIDEO_RENDERING
  if((--frameskip_count)<=0) 
  {
    if(runstate==RUNSTATE_RUNNING && bAppMinimized==0) 
    {
      if(extended_monitor) 
      {
        //bad_drawing bits: &1 - bad drawing option selected  &2 - bad-draw next screen
        //                  &4 - temporary bad drawing because of extended monitor.
        bad_drawing|=6;
        //        if(!FullScreen && em_needs_fullscreen)bad_drawing=4;
      }
      else
      {
        bad_drawing&=3;
        if(bad_drawing&1)
          bad_drawing|=3;
        else if(Glue.m_Status.stop_emu!=1||!OPTION_NO_OSD_ON_STOP)
          draw_begin();
      }
    }
    if(fast_forward)
      frameskip_count=20;
    else 
    {
      int fs=frameskip;
      if(fs==AUTO_FRAMESKIP && VSyncing) 
        fs=1;
      frameskip_count=fs;

      if(!Glue.video_freq)
        ;
      else if(fs==AUTO_FRAMESKIP) 
      {
        if(now-start_of_this_second>=1000)
        {
#if defined(SSE_OSD_FPS_INFO)
          if(OPTION_OSD_FPSINFO)
          {
            WORD fps=(WORD)(nframes_this_second-Debug.frame_no_change);
            TRACE_OSD2("%d",fps);
#if defined(SSE_STATS)
            Stats.nFps=(Stats.nFps) ? (Stats.nFps+fps)/2 : fps;
#endif
          }
          Debug.frame_no_change=0;
#endif
          nframes_this_second=0;
          start_of_this_second=now;
        }
        nframes_this_second++;
        auto_frameskip_target_time=timer+((run_speed_ticks_per_second
          +(Glue.video_freq/2))/Glue.video_freq);
        speed_limit_wait_till=auto_frameskip_target_time;
      }
      else 
      {
        if(frameskip==1)
        {
          int lag=0;
          if(nframes_this_second>=Glue.previous_video_freq)
          {
            lag=now-start_of_this_second-run_speed_ticks_per_second;
            if(VSyncing)
            {
#if defined(SSE_OSD_FPS_INFO)
              if(OPTION_OSD_FPSINFO)
              {
                WORD fps=(WORD)(nframes_this_second-Debug.frame_no_change);
                if(lag<-10 || lag>10) // only if significant because our computing is imprecise TODO
                {
                  int pcfps=(Glue.previous_video_freq*run_speed_ticks_per_second)
                    /(lag+run_speed_ticks_per_second);
                  TRACE_OSD2("ST%d PC%d",fps,pcfps);
                }
                else
                  TRACE_OSD2("%d",fps);
#if defined(SSE_STATS)
                Stats.nFps=(Stats.nFps) ? (Stats.nFps+fps)/2 : fps;
#endif
              }
#endif
              //start_of_this_second=timer;
              start_of_this_second=now; //leeway?
            }
            else //!(VSyncing)
            {
#if defined(SSE_OSD_FPS_INFO)
              if(OPTION_OSD_FPSINFO)
              {
                WORD fps=(WORD)(nframes_this_second-Debug.frame_no_change);
                TRACE_OSD2("%d",fps);
#if defined(SSE_STATS)
                Stats.nFps=(Stats.nFps) ? (Stats.nFps+fps)/2 : fps;
#endif
              }
#endif
              // very precise, lose no ms
              start_of_this_second+=run_speed_ticks_per_second; 
              // but give up if >1s (emu probably stopped by external cause)
              if(lag>run_speed_ticks_per_second) 
              {
                start_of_this_second=now;
                lag=0; 
              }
            }
            nframes_this_second=0;
#if defined(SSE_OSD_FPS_INFO)            
            Debug.frame_no_change=0;
#endif          
          }
#ifdef SSE_DEBUG
          else if(now-start_of_this_second>=(DWORD)run_speed_ticks_per_second)
          {
            TRACE_LOG("frame lag %d\n",now-start_of_this_second-(DWORD)run_speed_ticks_per_second);
          }
#endif
          nframes_this_second++;
          if(VSyncing)
          {
            //TRACE_OSD2("%d",time_of_next_event-cpu_time_of_last_vbl);
            speed_limit_wait_till=timer
              +(run_speed_ticks_per_second/Glue.video_freq);
            if(Glue.video_freq!=50 && !OPTION_C3)
              speed_limit_wait_till-=1; // leeway
          }
          else
          {
            DWORD from_start=((fs*run_speed_ticks_per_second
              *(nframes_this_second))/Glue.previous_video_freq)-lag;
            speed_limit_wait_till=start_of_this_second+from_start;
          }
        }
        else
        {
          nframes_this_second++;
          if(now>=start_of_this_second+run_speed_ticks_per_second)
          {
#if defined(SSE_OSD_FPS_INFO)
            if(OPTION_OSD_FPSINFO)
            {
              WORD fps=(WORD)(nframes_this_second-Debug.frame_no_change);
              TRACE_OSD2("%d",fps);
#if defined(SSE_STATS)
              Stats.nFps=(Stats.nFps) ? (Stats.nFps+fps)/2 : fps;
#endif
              Debug.frame_no_change=0;
            }
#endif
            nframes_this_second=0;
            start_of_this_second=now;
          }
          speed_limit_wait_till=timer+((fs*run_speed_ticks_per_second)
            /Glue.video_freq);
        }
#ifdef ENABLE_LOGGING
        LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: Calculating speed_limit_wait_till at ")+(timeGetTime()-run_start_time)+" timer="+(timer-run_start_time));
        LOG_TO(LOGSECTION_SPEEDLIMIT,Str("      frameskip=")+frameskip+" Glue.video_freq="+Glue.video_freq);
#endif
      }
    }
  }
  if(fast_forward)
    speed_limit_wait_till=timer+(fast_forward_max_speed/Glue.video_freq);
#ifdef ENABLE_LOGGING
  LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: speed_limit_wait_till is ")+(speed_limit_wait_till-run_start_time));
#endif
  // The MFP clock aligns with the CPU clock every 8000 CPU cycles
  while(abs_quick(ABSOLUTE_CPU_TIME-cpu_time_of_first_mfp_tick)>160000)
    cpu_time_of_first_mfp_tick+=160000;
  while(abs_quick(ABSOLUTE_CPU_TIME-shifter_cycle_base)>160000)
    shifter_cycle_base+=160000; //SS 60000?
  shifter_pixel=shifter_hscroll;
  left_border=BORDER_SIDE;right_border=BORDER_SIDE;
  scanline_drawn_so_far=0;
  video_first_draw_line=0;
  video_last_draw_line=shifter_y;
  if(emudetect_falcon_mode && emudetect_falcon_extra_height) 
  {
    video_first_draw_line=-20;
    video_last_draw_line=320;
  }
  if((Shifter.m_ShiftMode&2)&&screen_res<2)
  {
    video_last_draw_line*=2; //400 fetching lines
    memset(PCpal,0,sizeof(long)*16); // all colours black
  }
  Glue.Vbl();
  video_freq_at_start_of_vbl=Glue.video_freq;
  scanline_time_in_cpu_cycles_at_start_of_vbl
    =scanline_time_in_cpu_cycles[video_freq_idx];
  if(!OPTION_68901)
  {
    CALC_CYCLES_FROM_HBL_TO_TIMER_B(Glue.video_freq);
  }
  cpu_time_of_last_vbl=time_of_next_event;
  cpu_time_of_start_of_event_plan=cpu_time_of_last_vbl;
  LOG_TO(LOGSECTION_SPEEDLIMIT,"--");
  PasteVBL();
  ONEGAME_ONLY(OGVBL(); )
#ifdef DEBUG_BUILD
  if(debug_run_until==DRU_VBL||(debug_run_until==DRU_SCANLINE && debug_run_until_val==scan_y)) {
    if(runstate==RUNSTATE_RUNNING) 
    {
      runstate=RUNSTATE_STOPPING;
      runstate_why_stop="Run until";
    }
  }
  debug_vbl();
#endif
  Debug.Vbl(); // Debug.Vbl() is different from debug_vbl()!
#if !defined(SSE_X64)
  // call to refresh, overkill in 64bit build
  Cpu.UpdateCyclesForEClock();
#endif
#if defined(SSE_VID_STVL1)
  if(OPTION_C3)
    prepare_next_event();
#endif
}


void prepare_cpu_boosted_event_plans() {
  n_millions_cycles_per_sec=n_cpu_cycles_per_second/1000000;
  int factor=n_millions_cycles_per_sec;
  cpu_cycles_multiplier=(double)factor/8;
  SSEConfig.CpuBoost=(int)cpu_cycles_multiplier;
  SSEConfig.CpuBoosted=(cpu_cycles_multiplier > 1.0);
  for(int idx=0;idx<3;idx++)
  { //3 frequencies
    scanline_time_in_cpu_cycles[idx]=(int)(scanline_time_in_cpu_cycles_8mhz[idx]
    *cpu_cycles_multiplier);
  }
  mfp_init_timers();
  ChangeTimingFunctions();
  if(runstate==RUNSTATE_RUNNING) 
    prepare_next_event();
  CheckResetDisplay();
}


#if USE_PASTI

void event_pasti_update() {
  if(!(hPasti && (pasti_active || FloppyDrive[DRIVE].ImageType.Extension==EXT_STX)))
  {
    pasti_update_time=time_of_next_event+EIGHT_MILLION;
    return;
  }
  struct pastiIOINFO pioi;
  pioi.stPC=pc;
  pioi.cycles=time_of_next_event;
//  LOG_TO(LOGSECTION_PASTI,Str("PASTI: Update pc=$")+HEXSl(pc,6)+" cycles="+pioi.cycles);
  pasti->Io(PASTI_IOUPD,&pioi);
  pasti_handle_return(&pioi);
}

#endif


#ifdef DEBUG_BUILD

void event_debug_stop() {
  if(runstate==RUNSTATE_RUNNING)
  {
    runstate=RUNSTATE_STOPPING;
    runstate_why_stop="Run until";
  }
  debug_run_until=DRU_OFF; // Must be here to prevent freeze up as this event never goes into the future!
}

#endif


// SSE added events

void event_trigger_vbi() { //6X cycles into frame (colour)
#if defined(SSE_OSD_FPS_INFO)
  if(OPTION_OSD_FPSINFO)
    Debug.vbase_at_vbi=vbase; // program can change vbase at end of frame
#endif
  // video_freq_idx and video_freq are incorrect if mode=2 on colour screen
  // at least start the frame with correct video freq
  BYTE idx=(Glue.m_ShiftMode&2) ? 2 : ( (Glue.m_SyncMode&2) ? 0 : 1 );
  Glue.video_freq=Glue.Freq[idx];
  scan_y=-scanlines_above_screen[idx];
  video_freq_idx=idx;
  if(!OPTION_C3) 
  {
    if(Glue.previous_video_freq!=Glue.Freq[idx])
    {
      Glue.previous_video_freq=Glue.Freq[idx];
      init_screen();
      REFRESH_STATUS_BAR; // new frequency in status bar
      OptionBox.UpdateSTVideoPage(); // new frequency in option box
    }
  }
#if defined(SSE_VID_STVL1) 
  if(OPTION_C3)
  {
    // blit the frame, set vbi pending
    // delete rest of 60hz or 71hz screen, because our rendering
    // surface has just too many lines for those frequencies
    if(Stvl.framefreq!=50 && border && draw_mem_line_ptr>draw_mem
      && draw_mem_line_ptr<Disp.VideoMemoryEnd)
      ZeroMemory(draw_mem_line_ptr,Disp.VideoMemoryEnd-draw_mem_line_ptr-1);
    if(Stvl.framefreq && Glue.previous_video_freq!=Stvl.framefreq)
    {
      StvlUpdate();
      Glue.previous_video_freq=Stvl.framefreq;
      REFRESH_STATUS_BAR;
      OptionBox.UpdateSTVideoPage();
    }
    Stvl.render_y=0;
    draw_mem_line_ptr=draw_mem;
    event_vbl_interrupt();
    Glue.vbl_pending=true;
    Glue.vbl_pending_time=ACT+Stvl.tick8;
    update_ipl(Glue.vbl_pending_time);
    return;
  }
#endif
  //ASSERT(!Glue.m_Status.vbi_done);
  // As soon as VSYNC is asserted, the MMU keeps on copying VBASE to VCOUNT
  // We don't emulate the continuous copy but we copy at start and stop
  // (event_start_vbl(),event_trigger_vbi())
  Glue.vsync=false;
  Mmu.VideoCounter=shifter_draw_pointer_at_start_of_line
   =shifter_draw_pointer=vbase;
#if defined(SSE_HARDWARE_OVERSCAN)
  // hack to get correct display
  if(OPTION_HWOVERSCAN && SSEConfig.OverscanOn)
  {
    int off;
    if(COLOUR_MONITOR)
    {
      if(video_freq_at_start_of_vbl==50)
        off=(OPTION_HWOVERSCAN==LACESCAN)?(27*236-8*3):(23*224+2*8); 
      // and 236*24-80+22+8+8+8+8+8 for other "generic" overscan circuit
      else //TODO
        off=(OPTION_HWOVERSCAN==LACESCAN)?(20*234-8*3):(16*224+2*8);
      if(DISPLAY_SIZE>=2)
        off+=8;
    }
    else //monochrome: normal size only
      off=0;
      //off=(OPTION_HWOVERSCAN==LACESCAN)?(100*18+4):(96*18+4);
    shifter_draw_pointer+=off;
    Mmu.VideoCounter=shifter_draw_pointer;
  }
#endif
  Glue.vbl_pending=true;
  Glue.vbl_pending_time=time_of_next_event;
  update_ipl(Glue.vbl_pending_time);
  Glue.m_Status.vbi_done=true;
  // note: now we know that it's not a down counter on real HW but anyways, it works
  if(Glue.m_ShiftMode&2) // 71hz (monochrome)
  {
    Glue.nLines=501; // not 500
    Glue.de_start_line=34;
    Glue.de_end_line=434-1;
  }
  else if(Glue.m_SyncMode&2) // 50hz
  {
    Glue.nLines=313;
    Glue.de_start_line=63;
    Glue.de_end_line=263-1;
  }
  else // 60hz
  {
    Glue.nLines=263;
    Glue.de_start_line=34;
    Glue.de_end_line=234-1;
  }
  scan_y=-scanlines_above_screen[video_freq_idx];
  if(Glue.m_ShiftMode&2)
    scan_y=-scanlines_above_screen[2];
  //TRACE("F%d y%d (%d) vsync off\n",FRAME,scan_y,video_freq_idx);
}


/*  There's an event for floppy (STW etc.) because we want to handle DRQ for each
    byte, and the resolution of HBL is too gross for that:

    6256 bytes/ track , 5 revs /s= 31280 bytes
    1 second= 8021248 CPU cycles in our emu
    8021248/31280 = 256,433 cycles / byte
    8000000/31280 = 255,754 cycles / byte
    One HBL= 512 cycles at 50hz.

    Caps works with HBL because it hold its own cycle count.
    
    Here we should transfer control, or dispatch to handlers
*/

void event_wd1772() {
  Fdc.current_time=time_of_next_event;
  Fdc.OnUpdate();
}


void event_driveA_ip() {
  FloppyDrive[0].IndexPulse();
}


void event_driveB_ip() {
  FloppyDrive[1].IndexPulse();
}


//  ACIA event to handle IO with both 6301 and MIDI
// TODO risk of simultanate events?

COUNTER_VAR time_of_event_acia;

void event_acia() {
  if(OPTION_C1)
  {
    // find ACIA event to run
    // start transmission
//    ASSERT(time_of_event_acia==time_of_next_event);
    if(acia[ACIA_IKBD].LineTxBusy==2 && time_of_event_acia==acia[ACIA_IKBD].time_of_event_outgoing)
      acia[ACIA_IKBD].TransmitTDR();
    else if(acia[ACIA_MIDI].LineTxBusy==2 && time_of_event_acia==acia[ACIA_MIDI].time_of_event_outgoing)
      acia[ACIA_MIDI].TransmitTDR();
    // IKBD
    else if(acia[ACIA_IKBD].LineRxBusy==1 && time_of_event_acia==acia[ACIA_IKBD].time_of_event_incoming)
      agenda_keyboard_replace(0); // from IKBD
    else if(acia[ACIA_IKBD].LineTxBusy && time_of_event_acia==acia[ACIA_IKBD].time_of_event_outgoing)
      agenda_ikbd_process(acia[ACIA_IKBD].tdrs); // to IKBD
    // MIDI
    else if(acia[ACIA_MIDI].LineRxBusy && time_of_event_acia==acia[ACIA_MIDI].time_of_event_incoming)
      agenda_midi_replace(0); // from MIDI
    else if(acia[ACIA_MIDI].LineTxBusy && time_of_event_acia==acia[ACIA_MIDI].time_of_event_outgoing)
    { // to MIDI, do the job here
      acia[ACIA_MIDI].LineTxBusy=false; 
      MIDIPort.OutputByte(acia[ACIA_MIDI].tdrs);
      // send next MIDI note if any
      if(!(acia[ACIA_MIDI].sr&BIT_1))
        acia[ACIA_MIDI].TransmitTDR();
    }
    time_of_event_acia=time_of_next_event+n_cpu_cycles_per_second; // put into future
    // schedule next ACIA event if any (if not, it's still in the future)
    // it's not very smart but I see no better way for now
    if(acia[ACIA_IKBD].LineRxBusy==1 && acia[ACIA_IKBD].time_of_event_incoming-time_of_event_acia<0)
      time_of_event_acia=acia[ACIA_IKBD].time_of_event_incoming;
    if(acia[ACIA_IKBD].LineTxBusy && acia[ACIA_IKBD].time_of_event_outgoing-time_of_event_acia<0)
      time_of_event_acia=acia[ACIA_IKBD].time_of_event_outgoing;
    if(acia[ACIA_MIDI].LineRxBusy && acia[ACIA_MIDI].time_of_event_incoming-time_of_event_acia<0)
      time_of_event_acia=acia[ACIA_MIDI].time_of_event_incoming;
    if(acia[ACIA_MIDI].LineTxBusy && acia[ACIA_MIDI].time_of_event_outgoing-time_of_event_acia<0)
      time_of_event_acia=acia[ACIA_MIDI].time_of_event_outgoing;
  }
}


void event_dummy() { 
  // used when STVL is active
  // if it's actually reached, it's probably a bug
  cpu_timer_at_start_of_hbl=time_of_next_event;
}
