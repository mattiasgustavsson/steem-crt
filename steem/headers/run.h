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
FILE: run.h
DESCRIPTION: Declarations for Steem's central run() function.
struct TEvent
---------------------------------------------------------------------------*/

#pragma once
#ifndef RUN_DECLA_H
#define RUN_DECLA_H

#include "conditions.h"

#define RUNSTATE_RUNNING 0
#define RUNSTATE_STOPPING 1
#define RUNSTATE_STOPPED 2
#define AUTO_FRAMESKIP 8
#define ABSOLUTE_CPU_TIME (cpu_timer-cpu_cycles)
#define ACT ABSOLUTE_CPU_TIME
#define CYCLES_FROM_START_OF_HBL_IRQ_TO_WHEN_PEND_IS_CLEARED 28

#define INTERRUPT_START_TIME_WOBBLE  \
          cpu_cycles&=-4; \
          CPU_BUS_IDLE((EIGHT_MILLION-(ABSOLUTE_CPU_TIME-shifter_cycle_base)) % 10);

#define CHECK_AGENDA                                              \
  if ((hbl_count++)==agenda_next_time){                           \
    if (agenda_length){                                           \
      AGENDA_CS( EnterCriticalSection(&agenda_cs); )               \
      DBG_LOG(EasyStr("TASKS: Executing agenda action at ")+hbl_count);      \
      if (agenda_length){                                         \
        while ((signed int)(hbl_count-agenda[agenda_length-1].time)>=0){ \
          agenda_length--;                                        \
          TRACE_LOG("agenda execute #%d %p(%d)\n",agenda_length,agenda[agenda_length].perform,agenda[agenda_length].param);\
          if (agenda[agenda_length].perform!=NULL) agenda[agenda_length].perform(agenda[agenda_length].param); \
          if (agenda_length){                                     \
            agenda_next_time=agenda[agenda_length-1].time;        \
          }else{                                                  \
            agenda_next_time=hbl_count-1; /*wait 42 hours*/       \
            break;                                                \
          }                                                       \
        }                                                         \
      }                                                           \
      /*ASSERT(agenda_cs.RecursionCount==1);*/\
      AGENDA_CS( LeaveCriticalSection(&agenda_cs); )               \
    }                                                             \
  }

#define CALC_VIDEO_FREQ_IDX           \
            switch(Glue.video_freq){        \
              case 50:      video_freq_idx=0; break;  \
              case 60:      video_freq_idx=1; break;   \
              default:      video_freq_idx=2;   \
            }

// could we inline?

#define PREPARE_EVENT_CHECK_FOR_TIMER_TIMEOUTS(tn)      \
    if (mfp_timer_enabled[tn] || mfp_timer_period_change[tn]){                           \
      if ((time_of_next_event-mfp_timer_timeout[tn]) >= 0){  \
        time_of_next_event=mfp_timer_timeout[tn];          \
        event_vector=event_mfp_timer_timeout[tn];    \
      }                                                     \
    }


#define PREPARE_EVENT_CHECK_FOR_TIMER_B       \
  if (Mfp.reg[MFPR_TBCR]==8){  \
    if ((time_of_next_event-time_of_next_timer_b) >= 0){                 \
      time_of_next_event=time_of_next_timer_b;     \
      event_vector=event_timer_b;                    \
    }                                    \
  }


#ifdef DEBUG_BUILD

void event_debug_stop();

#define PREPARE_EVENT_CHECK_FOR_DEBUG       \
  if (debug_run_until==DRU_CYCLE){    \
    if ((time_of_next_event-debug_run_until_val) >= 0){                 \
      time_of_next_event=debug_run_until_val;  \
      event_vector=event_debug_stop;                    \
    }    \
  }

#define CHECK_BREAKPOINT                     \
        if (debug_num_bk){ \
          if (debug_first_instruction==0) breakpoint_check();     \
        }   \
        if (LITTLE_PC==trace_over_breakpoint){ \
          if (runstate==RUNSTATE_RUNNING) runstate=RUNSTATE_STOPPING;                 \
        } 

#define SET_WHY_STOP(s) runstate_why_stop=s;
extern int stem_runmode;

#else

#define PREPARE_EVENT_CHECK_FOR_DEBUG
#define CHECK_BREAKPOINT
#define SET_WHY_STOP(s)

#endif


#if USE_PASTI

#define PREPARE_EVENT_CHECK_FOR_PASTI       \
  if ((time_of_next_event-pasti_update_time) >= 0){                 \
    time_of_next_event=pasti_update_time;  \
    event_vector=event_pasti_update;                    \
  }

#else

#define PREPARE_EVENT_CHECK_FOR_PASTI

#endif


#define PREPARE_EVENT_CHECK_FOR_FLOPPY       \
  if ((time_of_next_event-Fdc.update_time) >= 0){                 \
    time_of_next_event=Fdc.update_time;  \
    event_vector=event_wd1772;                    \
  }\
  else if ((time_of_next_event-FloppyDrive[0].time_of_next_ip) >= 0){                 \
    time_of_next_event=FloppyDrive[0].time_of_next_ip;  \
    event_vector=event_driveA_ip;                    \
  }\
  else if ((time_of_next_event-FloppyDrive[1].time_of_next_ip) >= 0){                 \
    time_of_next_event=FloppyDrive[1].time_of_next_ip;  \
    event_vector=event_driveB_ip;                    \
  }


#define PREPARE_EVENT_CHECK_FOR_ACIA \
  if(OPTION_C1 && time_of_next_event-time_of_event_acia>=0)\
  {\
    time_of_next_event=time_of_event_acia;\
    event_vector=event_acia;\
  }


typedef void(*EVENTPROC)();

#pragma pack(push, 8)

struct TEvent {
  EVENTPROC event;
  COUNTER_VAR time;
};

#pragma pack(pop)

void run(); // can be called in an apart thread
void prepare_cpu_boosted_event_plans();
void prepare_next_event();

void event_timer_a_timeout();
void event_timer_b_timeout();
void event_timer_c_timeout();
void event_timer_d_timeout();
void event_scanline();
void event_timer_b();
void event_start_vbl();
void event_vbl_interrupt();
void event_hbl(); //just HBL, don't draw yet, don't increase scan_y
#if USE_PASTI
void event_pasti_update();
#endif
// SSE
void event_trigger_vbi();
void event_wd1772(); //1 event for FDC: various parts of its program
void event_driveA_ip(); // 1 event for each drive: IP
void event_driveB_ip();
void event_acia();
void event_dummy();

extern int runstate;

// fast_forward_max_speed=(1000 / (max %/100)); 0 for unlimited
extern int fast_forward,fast_forward_max_speed;
extern bool fast_forward_stuck_down;
extern int slow_motion,slow_motion_speed;
extern int run_speed_ticks_per_second;
extern bool disable_speed_limiting;
extern int run_start_time;
UNIX_ONLY(extern bool RunWhenStop; )
extern DWORD avg_frame_time,avg_frame_time_timer,frame_delay_timeout,timer;
extern DWORD speed_limit_wait_till;
extern int avg_frame_time_counter;
extern DWORD auto_frameskip_target_time;
extern int frameskip, frameskip_count;
extern bool flashlight_flag;
extern COUNTER_VAR time_of_event_acia;
extern COUNTER_VAR time_of_next_event;
extern COUNTER_VAR cpu_time_of_start_of_event_plan;
extern COUNTER_VAR time_of_next_timer_b;
extern COUNTER_VAR time_of_last_hbl_interrupt,time_of_last_vbl_interrupt;
extern COUNTER_VAR cpu_timer_at_res_change;
extern EVENTPROC event_vector;
extern EVENTPROC event_mfp_timer_timeout[4];
extern int scanline_time_in_cpu_cycles_at_start_of_vbl;


#endif//RUN_DECLA_H
