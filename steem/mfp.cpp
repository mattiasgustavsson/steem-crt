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
FILE: mfp.cpp
DESCRIPTION: The core of Steem's high-level emulation of the infamous
Motorola MC68901 MFP (Multi Function Processor).
As it is, it is quite compatible (runs many cases) but MFP emulation
in Steem is unsatisfactory, too many hacks without a good explanation.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <computer.h>
#include <osd.h>

BYTE mfp_gpip_no_interrupt=0xf7;
BYTE mfp_gpip_input_buffer=0;
const BYTE mfp_timer_irq[4]={13,8,5,4};
const BYTE mfp_gpip_irq[8]={0,1,2,3,6,7,14,15};
const int mfp_timer_prescale[16]={65535,4,10,16,50,64,100,200,
                            65535,4,10,16,50,64,100,200};
int mfp_timer_counter[4];
COUNTER_VAR mfp_timer_timeout[4];
COUNTER_VAR mfp_time_of_start_of_last_interrupt[16];
COUNTER_VAR cpu_time_of_first_mfp_tick;
bool mfp_timer_enabled[4]={0,0,0,0};
int mfp_timer_period[4]={10000,10000,10000,10000};
int mfp_timer_period_fraction[4];
int mfp_timer_period_current_fraction[4];
bool mfp_timer_period_change[4]={0,0,0,0};
bool mfp_interrupt_enabled[16];
double CpuMfpRatio=(double)CpuNormalHz/(double)MFP_XTAL;

#ifdef SSE_DEBUG
#include <debug_framereport.h>
#define LOGSECTION LOGSECTION_MFP
char* mfp_reg_name[]={"GPIP","AER","DDR","IERA","IERB","IPRA","IPRB","ISRA",
  "ISRB","IMRA","IMRB","VR","TACR","TBCR","TCDCR","TADR","TBDR","TCDR","TDDR",
  "SCR","UCR","RSR","TSR","UDR"};
#endif


void mfp_gpip_set_bit(int bit,bool set) {
  BYTE mask=BYTE(1<<bit);
  BYTE set_mask=BYTE(set?mask:0);
  BYTE cur_val=(Mfp.reg[MFPR_GPIP]&mask);
  if(cur_val==set_mask)
    return; //no change
  bool old_1_to_0_detector_input=(cur_val^(Mfp.reg[MFPR_AER]&mask))==mask;
  Mfp.reg[MFPR_GPIP]&=BYTE(~mask);
  Mfp.reg[MFPR_GPIP]|=set_mask;
  // If the DDR bit is low then the bit from the io line is used,
  // if it is high interrupts then it comes from the input buffer.
  // In that case interrupts are handled in the write to the GPIP.
  if(old_1_to_0_detector_input&&(Mfp.reg[MFPR_DDR]&mask)==0) 
  {
    // Transition the right way! Make the interrupt pend (don't cause an intr
    // straight away in case another more important one has just happened).
      mfp_interrupt_pend(mfp_gpip_irq[bit],ABSOLUTE_CPU_TIME)
  }
}


inline void calc_time_of_next_timer_b() {
  // called only by mfp_set_timer_reg()
  if(OPTION_68901)
  {
    // use our new function instead
    Mfp.ComputeNextTimerB(); 
  }
  else
  {
    int cycles_in=int(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl);
    if(cycles_in<cpu_cycles_from_hbl_to_timer_b) 
    {
      if(scan_y>=video_first_draw_line && scan_y<video_last_draw_line)
      {
        time_of_next_timer_b=cpu_timer_at_start_of_hbl
          +cpu_cycles_from_hbl_to_timer_b+TB_TIME_WOBBLE;
      }
      else
        time_of_next_timer_b=cpu_timer_at_start_of_hbl+160000*SSEConfig.CpuBoost;  //put into future
    }
    else
      time_of_next_timer_b=cpu_timer_at_start_of_hbl+160000*SSEConfig.CpuBoost;  //put into future
  }
}


#ifdef DEBUG_BUILD

bool mfp_set_pending(int irq,COUNTER_VAR when_set) {
  irq&=0x0F;
  bool was_already_pending=
    ((Mfp.reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)]&mfp_interrupt_i_bit(irq))!=0);
  if(OPTION_68901)
  {
    Mfp.reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)]|=mfp_interrupt_i_bit(irq); // Set pending
#if defined(SSE_DEBUG)
#if defined(SSE_DEBUGGER_FAKE_IO) //timers only when checked in mask
    if(!(irq==13&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TA)
      ||irq==8&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TB)
      ||irq==5&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TC)
      ||irq==4&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TD)))
#endif
      if(was_already_pending)
        TRACE_MFP(PRICV " MFP irq %d pending again at %d\n",ACT,irq,(int)when_set);
      else
        TRACE_MFP(PRICV " MFP irq %d pending at %d\n",ACT,irq,(int)when_set);
#endif
    if(!was_already_pending)
    {
      //Mfp.PendingTime[irq]=when_set; // record timing
      Mfp.UpdateNextIrq(when_set);
    }
  }
  else if((abs_quick(when_set-mfp_time_of_start_of_last_interrupt[irq])
    >=CYCLES_FROM_START_OF_MFP_IRQ_TO_WHEN_PEND_IS_CLEARED))
  {
    Mfp.reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)]|=mfp_interrupt_i_bit(irq); // Set pending
    return true;
  }
  return (Mfp.reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)] & mfp_interrupt_i_bit(irq))!=0;
}

#else

void mfp_set_pending(int irq,COUNTER_VAR when_set) {
  irq&=0x0F;
  BOOL was_already_pending=(Mfp.reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)]&mfp_interrupt_i_bit(irq));
  if(OPTION_68901)
  {
    Mfp.reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)]|=mfp_interrupt_i_bit(irq); // Set pending
    if(!was_already_pending)
    {
      //Mfp.PendingTime[irq]=when_set; // record timing
      Mfp.UpdateNextIrq(when_set);
    }
  }
  else if((abs_quick(when_set-mfp_time_of_start_of_last_interrupt[irq])
    >=CYCLES_FROM_START_OF_MFP_IRQ_TO_WHEN_PEND_IS_CLEARED))
  {
    Mfp.reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)]|=mfp_interrupt_i_bit(irq); // Set pending
  }
}

#endif


void mfp_set_timer_reg(int reg,BYTE old_val,BYTE new_val) {
  int ti=0; // 0=Timer A 1=Timer B 2=Timer C 3=Timer D
  BYTE new_control;
  if(reg>=MFPR_TACR && reg<=MFPR_TCDCR)
  { //control reg change
    new_control=BYTE(new_val & 15);
    switch(reg) {
    case MFPR_TACR: ti=0; break;
    case MFPR_TBCR: ti=1; break;
    case MFPR_TCDCR:
      ti=2; //we'll do D too
      new_control=BYTE((new_val>>4)&7);
      break;
    }
    do { // this do to do D
      if(Mfp.get_timer_control_register(ti)!=new_control)
      {
        new_control&=7;
#ifdef ENABLE_LOGGING
        DBG_LOG( EasyStr("MFP: ")+HEXSl(old_pc,6)+" - Changing timer "+char('A'+ti)+" control; current time="+
              ABSOLUTE_CPU_TIME+"; old timeout="+mfp_timer_timeout[ti]+";"
              "\r\n           ("+(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl)+
              " cycles into scanline #"+scan_y+")" );
#endif
        // This ensures that mfp_timer_counter is set to the correct value just
        // in case the data register is read while timer is stopped or the timer
        // is restarted before a write to the data register.
        // prescale_count is the number of MFP_CLKs there has been since the
        // counter last decreased.
        //MFP_CALC_TIMER_PERIOD(ti);//?
        int prescale_count=mfp_calc_timer_counter(ti,ACT
          +(OPTION_68901?MFP_TIMER_SET_DELAY:12));
        TRACE_LOG("Timer %c main %d prescale %d\n",
          'A'+ti,mfp_timer_counter[ti],prescale_count);
        if(new_control)
        { // Timer running in delay mode (or pulse, but it's very unlikely, not emulated)
#if defined(SSE_DEBUG)
          Mfp.nTimeouts[ti]=0;
#endif
          mfp_timer_timeout[ti]=ABSOLUTE_CPU_TIME;
          mfp_timer_timeout[ti]+=(OPTION_68901?MFP_TIMER_SET_DELAY:12);
          // compute next timeout, which depends on current counter + prescale
          double precise_cycles0=mfp_timer_prescale[new_control]
            *mfp_timer_counter[ti]/64;
          if(OPTION_68901)
            precise_cycles0-=prescale_count; // we count it here now
          if(precise_cycles0<0) 
            precise_cycles0=0;
          precise_cycles0*=CpuMfpRatio;
          precise_cycles0*=cpu_cycles_multiplier;
          mfp_timer_timeout[ti]+=(int)precise_cycles0;
          if(OPTION_68901 && (precise_cycles0-(int)precise_cycles0)) //generally
            mfp_timer_timeout[ti]++; 
          mfp_timer_enabled[ti]=mfp_interrupt_enabled[mfp_timer_irq[ti]];
          double precise_cycles= mfp_timer_prescale[new_control]
            *(int)(BYTE_00_TO_256(Mfp.reg[MFPR_TADR+ti]))
            *cpu_cycles_multiplier
            *CpuMfpRatio;
          mfp_timer_period[ti]=(int)precise_cycles;
          if(OPTION_68901)
          {
            mfp_timer_period_fraction[ti]=(int)((precise_cycles
              -mfp_timer_period[ti])*1000);
            mfp_timer_period_current_fraction[ti]=0;
          }
#if defined(SSE_DEBUG)
#if defined(SSE_DEBUGGER_FAKE_IO) //timers only when checked in mask
          if(! ( ti==0&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TA)
            || ti==1&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TB)
            || ti==2&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TC)
            || ti==3&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TD)))
#endif
          {
            TRACE_LOG(PRICV " (%d %d %d) PC %X set timer %c control %d data %d counter %d prescale %d\n",
              ACT,TIMING_INFO,old_pc,'A'+ti,new_control,
              Mfp.reg[MFPR_TADR+ti], mfp_timer_counter[ti]/64,prescale_count);
            if(new_val==8)
              TRACE_LOG(" (%d)\n",Mfp.reg[MFPR_TADR+ti]);
            else
            {
              int freq=MFP_XTAL
                /(mfp_timer_prescale[new_control]
              *BYTE_00_TO_256(Mfp.reg[MFPR_TADR+ti]));
              double cpu_ticks=mfp_timer_prescale[new_control]*CpuMfpRatio;
              TRACE_LOG(" MFP cycles pulse %d %dHz total %d CPU cycles pulse %f\
 total %d.%03d next timeout " PRICV "\n",
              mfp_timer_prescale[new_control],freq,
              mfp_timer_prescale[new_control]*int(BYTE_00_TO_256(Mfp.reg[MFPR_TADR+ti])),
              cpu_ticks,
              mfp_timer_period[ti],mfp_timer_period_fraction[ti],mfp_timer_timeout[ti]);
            }
          }
#endif//#if defined(SSE_DEBUG)
          if(!OPTION_68901) 
          {
            prescale_count=MIN(prescale_count,mfp_timer_prescale[new_control]);
            mfp_timer_timeout[ti]-=cpu_time_of_first_mfp_tick;
            // Convert to MFP cycles
            mfp_timer_timeout[ti]*=MFP_CLK; //SS =2451
            mfp_timer_timeout[ti]/=8000;
            // Take off number of cycles already counted
            mfp_timer_timeout[ti]-=prescale_count;
            // Convert back to CPU time
            mfp_timer_timeout[ti]*=8000;
            mfp_timer_timeout[ti]/=MFP_CLK;
            // Make absolute time again
            mfp_timer_timeout[ti]+=cpu_time_of_first_mfp_tick;
          } //if(!OPTION_68901)
          else
          {
/*  This should be confirmed (or not...), we consider that delay timers
    have some wobble (due to timer crystal)
    We add some time to delay, which we'll correct at timeout for
    next timer.*/
            Mfp.Wobble[ti]=(rand() % MFP_TIMERS_WOBBLE); //1-4
            mfp_timer_timeout[ti]+=Mfp.Wobble[ti];
          }
#ifdef ENABLE_LOGGING
          DBG_LOG(EasyStr("    Set control to ")+new_control+
                " (reg=$"+HEXSl(new_val,2)+"); data="+Mfp.reg[MFPR_TADR+ti]+
                "; counter="+mfp_timer_counter[ti]/64+
                "; period="+mfp_timer_period[ti]+
                "; new timeout="+mfp_timer_timeout[ti]);
#endif
        }
        else //if(new_control)
        {  //timer stopped, or in event count mode
          if(OPTION_68901 && !new_val && old_val!=8 
            && mfp_timer_enabled[ti] && ACT-mfp_timer_timeout[ti]>=0)
          {
            BYTE i_ab=mfp_interrupt_i_ab(mfp_timer_irq[ti]);
            BYTE i_bit=mfp_interrupt_i_bit(mfp_timer_irq[ti]);
            if(!(Mfp.reg[MFPR_ISRA+i_ab]&i_bit) &&
              (Mfp.reg[MFPR_IERA+i_ab]&Mfp.reg[MFPR_IMRA+i_ab]&i_bit))
            {
              TRACE_MFP(PRICV " timer %c pending on stop\n",ACT,'A'+ti);
              mfp_interrupt_pend(mfp_timer_irq[ti],mfp_timer_timeout[ti])
            }
          }
          //Mfp.Prescale[ti]=0; //according to doc but breaks Froggies
#ifdef SSE_DEBUG
#if defined(SSE_DEBUGGER_FAKE_IO) //only for timers checked in mask
          if(! ( ti==0&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TA)
            || ti==1&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TB)
            || ti==2&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TC)
            || ti==3&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TD)))
#endif
          if(new_val & BIT_3)
            TRACE_LOG(PRICV " (%d %d %d) PC %X set Timer %c data %d\n",
              ACT,TIMING_INFO,old_pc,'A'+ti,Mfp.reg[MFPR_TADR+ti]);
          else
            TRACE_LOG(PRICV " PC %X stop Timer %c counter %d prescale %d\n",
              ACT,old_pc,'A'+ti,Mfp.Counter[ti]/64,prescale_count);
#endif
          mfp_timer_enabled[ti]=false;
          mfp_timer_period_change[ti]=0;
#ifdef ENABLE_LOGGING
          DBG_LOG(EasyStr("  Set control to ")+new_control+" (reg=$"+HEXSl(new_val,2)+")"+
                "; counter="+mfp_timer_counter[ti]/64+" ;"+
                LPSTR((ti<2 && (new_val & BIT_3)) ? "event count mode.":"stopped.") );
#endif
        }//if(new_control)
        if(ti==3) 
          RS232_CalculateBaud(((Mfp.reg[MFPR_UCR] & BIT_7)!=0),new_control,0);
      }
      ti++;
      new_control=BYTE(new_val & 7); // Timer D control
    } while (ti==3); 
    if(reg==MFPR_TBCR && new_val==8)
    {
  #if defined(SSE_DEBUG)
      Mfp.nTimeouts[1]=0;
  #endif
      if(!OPTION_C3)
        calc_time_of_next_timer_b();
    }
#ifdef ENABLE_LOGGING
    if(reg<=MFPR_TBCR && new_val>8)
    {
      DBG_LOG("MFP: --------------- PULSE EXTENSION MODE!! -----------------");
    }
#endif
#ifdef SSE_DEBUG
    if(reg<=MFPR_TBCR && new_val>8)
      TRACE_LOG("MFP: --------------- PULSE EXTENSION MODE!! -----------------\n");
#endif
#if defined(SSE_INT_MFP_PULSE)
/*  and what should we do?  
    2 timer B events/line is easy
    actual counting is annoying
    since no program uses this feature, we may save the trouble as long as we
    don't have a low-level MFP emu
*/
    if(reg<=MFPR_TBCR && new_val>8)
    {
    }
#endif
    prepare_next_event();
  }
  else if(reg>=MFPR_TADR && reg<=MFPR_TDDR)
  { //data reg change
    ti=reg-MFPR_TADR;
#ifdef ENABLE_LOGGING
    DBG_LOG(Str("MFP: ")+HEXSl(old_pc,6)+" - Changing timer "+char(('A')+ti)+" data reg to "+new_val+" ($"+HEXSl(new_val,2)+") "+
          " at time="+ABSOLUTE_CPU_TIME+" ("+(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl)+
          " cycles into scanline #"+scan_y+"); timeout is "+mfp_timer_timeout[ti]);
#endif
    BYTE control=BYTE(Mfp.get_timer_control_register(ti));
    if(control==0) 
    {  // timer stopped
      mfp_timer_counter[ti]=((int)BYTE_00_TO_256(new_val))*64;
      Mfp.Counter[ti]=new_val;
      Mfp.Prescale[ti]=0;
      mfp_timer_period[ti]=int(double(mfp_timer_prescale[control]
        *int(BYTE_00_TO_256(new_val)))*CpuMfpRatio);
    }
    else if(control&7)
    {
/*
If the Timer Data Register is written while the timer is running, the new word
is not loaded into the timer until it counts through H01.      
*/      
      // Need to calculate the period next time the timer times out
      mfp_timer_period_change[ti]=true;
      if(mfp_timer_enabled[ti]==0)
      {
        // If it is disabled it could be in the past, causing instant
        // event_timer_?_timeout, so realign it
        COUNTER_VAR stage=mfp_timer_timeout[ti]-ABSOLUTE_CPU_TIME;
        if(stage<0) 
          stage+=((-stage/mfp_timer_period[ti])+1)*mfp_timer_period[ti];
        stage%=mfp_timer_period[ti];
        mfp_timer_timeout[ti]=ABSOLUTE_CPU_TIME+stage; //realign
      }
    }
#ifdef ENABLE_LOGGING
    DBG_LOG(EasyStr("     Period is ")+mfp_timer_period[ti]+" cpu cycles");
#endif
    if(reg==MFPR_TDDR && new_val!=old_val)
      RS232_CalculateBaud(((Mfp.reg[MFPR_UCR] & BIT_7)!=0),control,0);
  }
}


void mfp_init_timers() {// For load state and CPU speed change
  MFP_CALC_INTERRUPTS_ENABLED;
  MFP_CALC_TIMERS_ENABLED;
  for(int ti=0;ti<4;ti++)
  {
    BYTE cr=Mfp.get_timer_control_register(ti);
    if(cr&7)
    { // Not stopped or in event count mode
      // This must allow for counter not being a multiple of 64
      mfp_timer_timeout[ti]=ABSOLUTE_CPU_TIME+int((double(mfp_timer_prescale[cr])*
        double(mfp_timer_counter[ti])/64.0)*CpuMfpRatio);
      mfp_timer_period_change[ti]=true;
    }
  }
  RS232_CalculateBaud(((Mfp.reg[MFPR_UCR]&BIT_7)!=0),
    Mfp.get_timer_control_register(3),true);
}


int mfp_calc_timer_counter(int ti,COUNTER_VAR t) {
  BYTE cr=Mfp.get_timer_control_register(ti);
  if(cr&7) 
  { // not event count mode
    COUNTER_VAR stage=mfp_timer_timeout[ti]-t;
    if(stage<0) 
    { // has timed out? - no high precision here...
      stage-=Mfp.Wobble[ti];
      //MFP_CALC_TIMER_PERIOD(ti);
      if(OPTION_68901) 
      {
        // assume: If the counter wrapped not long ago, its visible value
        // is 0, not TXDR - Froggies back to menu
        if(stage>=-2) // depends on wobble
        {
          mfp_timer_counter[ti]=0;
          Mfp.Counter[ti]=0;
          Mfp.Prescale[ti]=0;
          return Mfp.Prescale[ti];
        }
        while(stage<0 && mfp_timer_period[ti]>0)
          stage+=mfp_timer_period[ti];
      }
      else
        stage+=((-stage/mfp_timer_period[ti])+1)*mfp_timer_period[ti];
    }
    stage%=mfp_timer_period[ti];
    int ticks_per_count=mfp_timer_prescale[cr & 7];
    // Convert to number of MFP cycles until timeout
    if(CpuMfpRatio)
      stage=(COUNTER_VAR)(double(stage)/CpuMfpRatio);
    mfp_timer_counter[ti]=((int)stage/ticks_per_count)*64 + 64;
    Mfp.Counter[ti]=(BYTE)(mfp_timer_counter[ti]/64);
    Mfp.Prescale[ti]=(BYTE)(ticks_per_count-((stage % ticks_per_count)+1));
  }
  return Mfp.Prescale[ti];
}


void mfp_interrupt(int irq) {
  //n nn ns ni n-  n nS ns nV nv np n np 
  int irq0=irq;
  if(irq==16&&(!OPTION_SPURIOUS||STOP_INTS_BECAUSE_INTERCEPT_OS))
    return;
  M68K_UNSTOP;
  Cpu.ProcessingState=TMC68000::EXCEPTION;
  // we redo some tests for the RS232 part that comes directly here, and also
  // when option C2 isn't checked!
  if(irq>=0&&irq<16)
    if(!(mfp_interrupt_enabled[irq]) || (SR & SR_IPL) >= SR_IPL_6
    || ((Mfp.reg[MFPR_IMRA+mfp_interrupt_i_ab(irq)] & mfp_interrupt_i_bit(irq))==0)
    || ((Mfp.reg[MFPR_ISRA+mfp_interrupt_i_ab(irq)] & (-mfp_interrupt_i_bit(irq))) 
    || (mfp_interrupt_i_ab(irq)&&Mfp.reg[MFPR_ISRA])))
  {
    TRACE_OSD("MFP irq denied"); // funny message
    TRACE_LOG("MFP irq denied\n"); // funny message
    return;
  }
  CPU_BUS_IDLE(6); //n nn
  UPDATE_SR;
  WORD saved_sr=SR; //copy sr
  if(!SUPERFLAG) //set S
    change_to_supervisor_mode();
  CLEAR_T;
  PSWI=6; // update ipl mask
  iabus=areg[7]-2;
  dbus=pcl; // stack PC low word;
  CPU_BUS_ACCESS_WRITE; // ns 12
  iabus-=4;
  areg[7]=iabus;
  // start IACK bus cycle - the MFP is slow
  BUS_JAM_TIME(12); // ni 24
  if(OPTION_68901)
  {
    // If no irq is pending at the start of IACK, the MFP won't respond (spurious).
    // Otherwise, another higher irq could trigger during IACK (Anomaly Menu)
    // or the same irq could trigger again (Final Conflict)
    if(irq<=15)
    {
      while(cpu_cycles<=0)
      {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
        if(TRACE_MASK2&TRACE_CONTROL_EVENT)
          TRACE_EVENT(event_vector);
#endif
        event_vector(); // event_scanline etc. also taken! TODO
        prepare_next_event();
      }
      irq=Mfp.UpdateNextIrq(ACT);
    }
  }
  MEM_ADDRESS vector;
  // The dangerous spurious interrupt test. SPURIOUS.TOS, and triggers in
  // some demos (have a convenient RTE).
  if(irq==16&&(!OPTION_SPURIOUS||(STOP_INTS_BECAUSE_INTERCEPT_OS)))
    irq=irq0;
  if(OPTION_SPURIOUS&&irq==16)
  {
#if defined(SSE_STATS)
    Stats.nSpurious++;
#endif
#if defined(SSE_OSD_DEBUGINFO)
    if(OPTION_OSD_DEBUGINFO)
      TRACE_OSD2("24B");
#ifdef SSE_DEBUG
    else TRACE_OSD("Spurious!");
#endif
#endif
    TRACE2("Spurious\n");
#ifdef SSE_DEBUG
    TRACE_MFP(PRICV " PC %X Spurious! %X\n",ACT,old_pc);
    TRACE_MFP("IRQ %d (%d->%d) IERA %X IPRA %X IMRA %X ISRA %X IERB %X IPRB %X IMRB %X ISRB %X\n",
      Mfp.Irq,irq0,Mfp.NextIrq,Mfp.reg[MFPR_IERA],Mfp.reg[MFPR_IPRA],Mfp.reg[MFPR_IMRA],Mfp.reg[MFPR_ISRA],Mfp.reg[MFPR_IERB],Mfp.reg[MFPR_IPRB],Mfp.reg[MFPR_IMRB],Mfp.reg[MFPR_ISRB]);
#endif
    int ncycles=(64+4+2-12); //-ni? This hasn't been tested
    for(int i=0;i<ncycles;i+=2)
    {
      BUS_JAM_TIME(2); // just in case, avoid too long CPU timings
    }
    BUS_JAM_TIME(4);
    vector=0x18;
  }
  else
  {
    vector=(Mfp.reg[MFPR_VR]&0xf0)+(irq);
    Mfp.reg[MFPR_IPRA+mfp_interrupt_i_ab(irq)]&=BYTE(~mfp_interrupt_i_bit(irq));
    if(MFP_S_BIT) // software mode, set when vector is passed, cleared by program
      Mfp.reg[MFPR_ISRA+mfp_interrupt_i_ab(irq)]|=mfp_interrupt_i_bit(irq);
    else //automatic
      Mfp.reg[MFPR_ISRA+mfp_interrupt_i_ab(irq)]&=BYTE(~mfp_interrupt_i_bit(irq));
    //timing is recorded after IACK
    mfp_time_of_start_of_last_interrupt[irq]=ABSOLUTE_CPU_TIME;
#if defined(SSE_STATS)
    Stats.nMfpIrq[irq]++;
#endif
  }
  vector<<=2;

#if defined(SSE_DEBUG)
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
  if(FRAME_REPORT_MASK2 & FRAME_REPORT_MASK_INT)
    FrameEvents.Add(scan_y,(short)(LINECYCLES-24),'I',0x60+irq);
#endif
#if defined(SSE_DEBUGGER_FAKE_IO) //timers only when checked in mask
  if(! ( irq==13&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TA)
      || irq==8&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TB)
      || irq==5&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TC)
      || irq==4&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TD)))
#endif
  {
    TRACE_INT(PRICV " %d %d %d PC %X ird %X IRQ %d VEC %X ",ACT-24,FRAME,
      scan_y,(short)LINECYCLES-24,old_pc,IRD,irq,LPEEK(vector));
    switch (irq) {
    case 0:TRACE_INT("Centronics busy\n");          break;
    case 1:TRACE_INT("RS-232 DCD\n");               break;
    case 2:TRACE_INT("RS-232 CTS\n");               break;
    case 3:TRACE_INT("Blitter done\n");             break;
    case 4:TRACE_INT("Timer D #%d\n", ++Mfp.nTimeouts[3]); break;
    case 5:TRACE_INT("Timer C #%d\n", ++Mfp.nTimeouts[2]); break;
    case 6:TRACE_INT("ACIA\n");                     break;
    case 7:TRACE_INT("FDC/HDC\n");                  break;
    case 8:TRACE_INT("Timer B #%d\n", ++Mfp.nTimeouts[1]); break;
    case 9:TRACE_INT("Send Error\n");               break;
    case 10:TRACE_INT("Send buffer empty\n");       break;
    case 11:TRACE_INT("Receive error\n");           break;
    case 12:TRACE_INT("Receive buffer full\n");     break;
    case 13:TRACE_INT("Timer A #%d\n", ++Mfp.nTimeouts[0]); break;
    case 14:TRACE_INT("RS-232 Ring detect\n");      break;
    case 15:TRACE_INT("Monochrome Detect\n");       break;
    case 16:TRACE_INT("Spurious interrupt\n");       break;
    }//sw
  }
#if defined(SSE_DEBUGGER_FRAME_INTERRUPTS)
  Debug.FrameInterrupts|=4;
  Debug.FrameMfpIrqs|= 1<<irq;
#endif
#if defined(SSE_DEBUGGER_IRQ) 
  pc_history_y[pc_history_idx]=scan_y;
  pc_history_c[pc_history_idx]=LINECYCLES;
  pc_history[pc_history_idx++]=0x99000001+(6<<16)+(WORD)(irq<<8); 
  if(pc_history_idx>=HISTORY_SIZE) 
    pc_history_idx=0;
#endif
#endif//dbg

  CPU_BUS_IDLE(4); //nn
  if(OPTION_68901)
  {
    //Mfp.LastIrq=irq; //unused
    Mfp.UpdateNextIrq(ACT); // necessary
  }
  dbus=saved_sr; // SR written between two parts of PC
  CPU_BUS_ACCESS_WRITE; // ns
  iabus+=2;
  dbus=pch; // PC high word 
  CPU_BUS_ACCESS_WRITE; // nS
  iabus=vector;
  CPU_BUS_ACCESS_READ; // nV
  effective_address_h=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; // nv
  effective_address_l=dbus;
  iabus=effective_address;
  Cpu.ProcessingState=TMC68000::NORMAL;
  m68kSetPC(iabus,2);
  interrupt_depth++;
  log_to_section(LOGSECTION_INTERRUPTS,EasyStr("  IRQ fired - vector=")+HEXSl(LPEEK(vector),6));
  debug_check_break_on_irq(irq);
}


TMC68901::TMC68901() {
  ZeroMemory(this,sizeof(TMC68901));
  Restore();
}


void TMC68901::Restore() {
  ZeroMemory(IrqInfo,sizeof(IrqInfo));
  // init IrqInfo structure
  IrqInfo[0].IsGpip=IrqInfo[1].IsGpip=IrqInfo[2].IsGpip=IrqInfo[3].IsGpip
    =IrqInfo[6].IsGpip=IrqInfo[7].IsGpip=IrqInfo[14].IsGpip=IrqInfo[15].IsGpip
    =true;
  IrqInfo[4].IsTimer=IrqInfo[5].IsTimer=IrqInfo[8].IsTimer=IrqInfo[13].IsTimer
    =true;
  IrqInfo[4].Timer=3;  // timer D
  IrqInfo[5].Timer=2;  // timer C
  IrqInfo[8].Timer=1;  // timer B
  IrqInfo[13].Timer=0;  // timer A
  ier=(WORD*)&reg[MFPR_IERA];
  ipr=(WORD*)&reg[MFPR_IPRA];
  isr=(WORD*)&reg[MFPR_ISRA];
  imr=(WORD*)&reg[MFPR_IMRA];
}


int TMC68901::UpdateNextIrq(COUNTER_VAR at_time) {
  if(MFP_IRQ) // global test before we check each irq
  {
    for(char irq=15;irq>=0;irq--) { //need to check all
      BYTE i_ab=mfp_interrupt_i_ab(irq);
      BYTE i_bit=mfp_interrupt_i_bit(irq);
      if((i_bit
        & reg[MFPR_IERA+i_ab]&reg[MFPR_IPRA+i_ab]&reg[MFPR_IMRA+i_ab])
        && !(i_bit&reg[MFPR_ISRA+i_ab]))
      {
        Irq=true; // line is asserted by the MFP
        NextIrq=irq;
#if defined(SSE_DEBUG)
#if defined(SSE_DEBUGGER_FAKE_IO) //timers only when checked in mask
  if(! ( NextIrq==13&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TA)
      || NextIrq==8&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TB)
      || NextIrq==5&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TC)
      || NextIrq==4&&!(TRACE_MASK2&TRACE_CONTROL_IRQ_TD)))
#endif
        TRACE_MFP("%d MFP IRQ (%d)\n",at_time,NextIrq);
#endif
        break;
      }
      else if((i_bit&reg[MFPR_ISRA+i_ab]))
      {
        Irq=false;
        break; // no IRQ possible then
      }
    }//nxt irq
  }
  else
  {
    Irq=false;
  }
  if(!Irq)
    NextIrq=16; // pseudo irq 16 means no IRQ, internally in Steem
  update_ipl(at_time);
  return NextIrq;
}


/*  Unique function (option C2) that sets up the timing of next
    "timer B event", as far as the MFP input is concerned.
    It should be correct also when timing of DE is changed by
    a "Shifter trick".
*/

void TMC68901::ComputeNextTimerB() {
  //ASSERT(OPTION_68901);
  //ASSERT(!OPTION_C3)
  COUNTER_VAR tontb=0;
#if defined(SSE_HARDWARE_OVERSCAN)
  // mfp receives the normal DE signal, not the tricked overscan DE
  // in this emulation, it's a bit off compared with HW
  if(SSEConfig.OverscanOn 
    &&( (COLOUR_MONITOR && (scan_y<0 || scan_y>=200))
    || (!COLOUR_MONITOR && (scan_y<0 || scan_y>=400))))
    ; // skip - notice TOS expects groups of lines with no timer B tick at reset
  else
#endif
  if(Glue.de_v_on && !(Glue.CurrentScanline.Tricks&TRICK_0BYTE_LINE))
  {
    // time of DE transition this scanline
    cpu_cycles_from_hbl_to_timer_b=(reg[MFPR_AER]&8)?
      Glue.CurrentScanline.StartCycle:Glue.CurrentScanline.EndCycle; // from Hatari
    cpu_cycles_from_hbl_to_timer_b+=
      MFP_TIMER_B_DELAY // add MFP delays (read + irq)
      +Mmu.FreqMod[IS_STE?(3-1):OPTION_WS]; // add wakeup DE shift (Super Neo)
    // already ticked? / no tb?
    if( 
      ((reg[MFPR_AER]&8) && Glue.m_Status.timerb_start)
      || (!(reg[MFPR_AER]&8) && Glue.m_Status.timerb_end)
      || (reg[MFPR_TBCR]!=8 && LINECYCLES>cpu_cycles_from_hbl_to_timer_b) // Oh no more froggies
      )
    {
      cpu_cycles_from_hbl_to_timer_b+=(short)Glue.CurrentScanline.Cycles; //next line?
    }
    // absolute
    tontb=cpu_timer_at_start_of_hbl+cpu_cycles_from_hbl_to_timer_b;
    // add jitter
    tontb+=TB_TIME_WOBBLE;
  }
  else
    tontb=cpu_timer_at_start_of_hbl+160000*SSEConfig.CpuBoost;
  time_of_next_timer_b=tontb;
  prepare_next_event();
}


void TMC68901::Reset(bool Cold) {
  DWORD tmp;
  memcpy(&tmp,&reg[MFPR_TADR],4);
  ZeroMemory(reg,sizeof(Mfp.reg));
  if(!Cold) // power up: 0
    memcpy(&reg[MFPR_TADR],&tmp,4);
  reg[MFPR_GPIP]=mfp_gpip_no_interrupt;
  reg[MFPR_AER]=0x4;   // CTS goes the other way
  reg[MFPR_TSR]=BIT_7|BIT_4;  //buffer empty | END
  for(int t=0;t<4;t++)
    mfp_timer_counter[t]=256*64;
  MFP_CALC_INTERRUPTS_ENABLED;
  MFP_CALC_TIMERS_ENABLED;
  Irq=false;
  for(int t=0;t<4;t++)
  {
    Counter[t]=0;
    Prescale[t]=0;
  }
}
