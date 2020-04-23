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
FILE: mfp.h
DESCRIPTION: Declarations for Steem's high-level emulation of the infamous
Motorola MC68901 MFP (Multi Function Processor).
TODO: move some variables to the Mfp object
struct TMC68901IrqInfo, TMC68901
---------------------------------------------------------------------------*/

#pragma once
#ifndef MFP_DECLA_H
#define MFP_DECLA_H

#include "emulator.h"
#include "steemh.h"
#include "binary.h"
#include "options.h"
#include "debug.h"


// bad imitation of a normal distribution
#define TB_TIME_WOBBLE ( ((rand()&6)+(rand()&6)+(rand()&6)+(rand()&6))/4  )

enum EMfpRegs {
  MFPR_GPIP=0, // ff fa01 MFP General Purpose I/O
  MFPR_AER=1, // ff fa03 MFP Active Edge
  MFPR_DDR=2, // ff fa05 MFP Data Direction
  MFPR_IERA=3, // ff fa07 MFP Interrupt Enable A
  MFPR_IERB=4, // ff fa09 MFP Interrupt Enable B
  MFPR_IPRA=5, // ff fa0b MFP Interrupt Pending A
  MFPR_IPRB=6, // ff fa0d MFP Interrupt Pending B
  MFPR_ISRA=7, // ff fa0f MFP Interrupt In-Service A
  MFPR_ISRB=8, // ff fa11 MFP Interrupt In-Service B
  MFPR_IMRA=9, // ff fa13 MFP Interrupt Mask A
  MFPR_IMRB=10, // ff fa15 MFP Interrupt Mask B
  MFPR_VR=11, // ff fa17 MFP Vector
  MFPR_TACR=12, // ff fa19 MFP Timer A Control
  MFPR_TBCR=13, // ff fa1b MFP Timer B Control
  MFPR_TCDCR=14, // ff fa1d MFP Timers C and D Control
  MFPR_TADR=15, // ff fa1f  MFP Timer A Data
  MFPR_TBDR=16, // ff fa21  MFP Timer B Data
  MFPR_TCDR=17, // ff fa23  MFP Timer C Data
  MFPR_TDDR=18, // ff fa25  MFP Timer D Data
  // RS232
  MFPR_SCR=19, // ff fa27 MFP Sync Character
  MFPR_UCR=20, // ff fa29 MFP USART Control
  MFPR_RSR=21, // ff fa2b MFP Receiver Status
  MFPR_TSR=22, // ff fa2d MFP Transmitter Status
  MFPR_UDR=23 // ff fa2f MFP USART Data
};

#if defined(SSE_DEBUG) || defined(BCC_BUILD)
extern char* mfp_reg_name[]; //3.8.2
#endif

extern BYTE mfp_gpip_no_interrupt;

enum EMfpInterrupts {
  MFP_INT_MONOCHROME_MONITOR_DETECT=15,
  MFP_INT_RS232_RING_INDICATOR=14,
  MFP_INT_TIMER_A=13,
  MFP_INT_RS232_RECEIVE_BUFFER_FULL=12,
  MFP_INT_RS232_RECEIVE_ERROR=11,
  MFP_INT_RS232_TRANSMIT_BUFFER_EMPTY=10,
  MFP_INT_RS232_TRANSMIT_ERROR=9,
  MFP_INT_TIMER_B=8,
  MFP_INT_FDC_AND_DMA=7,
  MFP_INT_ACIA=6,  // Vector at $118
  MFP_INT_TIMER_C=5,
  MFP_INT_TIMER_D=4,
  MFP_INT_BLITTER=3,
  MFP_INT_RS232_CTS=2,
  MFP_INT_RS232_DCD=1,
  MFP_INT_CENTRONICS_BUSY=0
};

#define MFP_GPIP_COLOUR BYTE(0x80)
#define MFP_GPIP_NOT_COLOUR BYTE(~0x80)
#define MFP_GPIP_CTS BYTE(BIT_2)
#define MFP_GPIP_DCD BYTE(BIT_1)
#define MFP_GPIP_RING BYTE(BIT_6)


enum EGpipBits {
  MFP_GPIP_CENTRONICS_BIT=0,
  MFP_GPIP_DCD_BIT=1,
  MFP_GPIP_CTS_BIT=2,
  MFP_GPIP_BLITTER_BIT=3,
  MFP_GPIP_ACIA_BIT=4,
  MFP_GPIP_FDC_BIT=5,
  MFP_GPIP_RING_BIT=6,
  MFP_GPIP_MONO_BIT=7
};

extern BYTE mfp_gpip_input_buffer;
#define MFP_CLK 2451
#define MFP_CLK_EXACT ((OPTION_68901)?(MFP_XTAL):(2451134)) // (2457600) it is used in rs232
#define CYCLES_FROM_START_OF_MFP_IRQ_TO_WHEN_PEND_IS_CLEARED 20
#define MFP_S_BIT (Mfp.reg[MFPR_VR] & BIT_3)

extern double CpuMfpRatio;
#if defined(SSE_CPU_MFP_RATIO_OPTION)
extern DWORD CpuCustomHz;
#endif

#pragma pack(push, 8)

struct TMC68901IrqInfo {
  unsigned int IsGpip:1;
  unsigned int IsTimer:1;
  unsigned int Timer:4;
};


struct TMC68901 {
  // FUNCTIONS
  TMC68901();
  void Restore();
  void Reset(bool Cold);
  int UpdateNextIrq(COUNTER_VAR at_time);
  void ComputeNextTimerB();
  inline void mfp_calc_timer_period(int t);
  inline BYTE get_timer_control_register(int n);
  // DATA
  COUNTER_VAR IrqSetTime;//unused - kept for snapshot compatibility
  COUNTER_VAR IackTiming;//unused
  COUNTER_VAR WriteTiming;//unused
  double Period[4]; // to record the period as double (debug use only for now)
  WORD IPR;//unused
  char Wobble[4]; // can be negative
  BYTE Counter[4],Prescale[4]; // to hold real values
  BYTE LastRegisterWritten;
  BYTE LastRegisterFormerValue;
  BYTE LastRegisterWrittenValue;
  BYTE Vector;//unused
  char NextIrq; //-1 reset
  char LastIrq;
  char IrqInService;//unused
  bool Irq;  // asserted or not, problem, we're not in real-time
  bool WritePending;
  TMC68901IrqInfo IrqInfo[16];
  //400
  BYTE reg[24]; // 24 directly addressable internal registers, each 8bit
  WORD *ier,*ipr,*isr,*imr; // for word access
  COUNTER_VAR time_of_last_tb_tick;
  int nTimeouts[4];//debug
  BYTE tbctr_old;//another hack
};

#pragma pack(pop)



extern const BYTE mfp_timer_irq[4];
extern const BYTE mfp_gpip_irq[8];
extern const int mfp_timer_prescale[16];
extern int mfp_timer_counter[4];
extern COUNTER_VAR mfp_timer_timeout[4];
#ifdef DEBUG_BUILD
bool mfp_set_pending(int,COUNTER_VAR);
#else
void mfp_set_pending(int,COUNTER_VAR);
#endif
extern COUNTER_VAR mfp_time_of_start_of_last_interrupt[16];
extern COUNTER_VAR cpu_time_of_first_mfp_tick;
extern bool mfp_timer_enabled[4];
extern int mfp_timer_period[4];
extern int mfp_timer_period_fraction[4];
extern int mfp_timer_period_current_fraction[4];
extern bool mfp_timer_period_change[4];

void mfp_set_timer_reg(int,BYTE,BYTE);
int mfp_calc_timer_counter(int,COUNTER_VAR t);
void mfp_init_timers();

void mfp_gpip_set_bit(int,bool);
extern bool mfp_interrupt_enabled[16];

//TODO use our 16bit vars?
#define MFP_CALC_INTERRUPTS_ENABLED	\
{	\
  int mask=1;	\
  for (int i=0;i<8;i++){	\
    mfp_interrupt_enabled[i]=(Mfp.reg[MFPR_IERB] & mask)!=0; mask<<=1;	\
  }	\
  mask=1;	\
  for (int i=0;i<8;i++){	\
    mfp_interrupt_enabled[i+8]=(Mfp.reg[MFPR_IERA] & mask)!=0; mask<<=1;	\
  }	\
}

#define MFP_CALC_TIMERS_ENABLED	\
	for (int t=0;t<4;t++){	\
		mfp_timer_enabled[t]=mfp_interrupt_enabled[mfp_timer_irq[t]]\
      && (Mfp.get_timer_control_register(t) & 7);	\
	}


void mfp_interrupt(int);


inline void TMC68901::mfp_calc_timer_period(int t) {
  double precise_cycles=
    double(mfp_timer_prescale[get_timer_control_register(t)]
      *(int)(BYTE_00_TO_256(reg[MFPR_TADR+t])))
    *CpuMfpRatio;
  precise_cycles*=cpu_cycles_multiplier; //cpu_cycles_multiplier is a double
  mfp_timer_period[t]=(int)precise_cycles;
  mfp_timer_period_fraction[t]=(int)(1000*precise_cycles
    -(double)mfp_timer_period[t]);
  //mfp_timer_period_current_fraction[t]=0;
}

#define MFP_CALC_TIMER_PERIOD(t) Mfp.mfp_calc_timer_period(t)


#define mfp_interrupt_i_bit(irq) (BYTE(1 << (irq & 7)))
#define mfp_interrupt_i_ab(irq) (1-((irq & 8) >> 3))

#define MFP_IRQ ( *Mfp.ier & *Mfp.ipr & *Mfp.imr & (~*Mfp.isr) )

//TODO check the condition
#define mfp_interrupt_pend(irq,when_fired)   {     \
  if(mfp_interrupt_enabled[irq]){                 \
    LOG_ONLY( bool done= ) mfp_set_pending(irq,when_fired);   \
    LOG_ONLY( if (done==0) LOG_TO(LOGSECTION_MFP_TIMERS,EasyStr("INTERRUPT: MFP IRQ #")+irq+" ("+    \
                                (char*)name_of_mfp_interrupt[irq]+") - can't set pending as MFP cleared "  \
                                "pending after timeout"); )             \
  }\
}

inline BYTE TMC68901::get_timer_control_register(int n)
{
  if(n==0)
    return reg[MFPR_TACR];
  else if(n==1)
    return reg[MFPR_TBCR];
  else if(n==2)
    return BYTE((reg[MFPR_TCDCR]&b01110000)>>4);
  else
    return BYTE(reg[MFPR_TCDCR]&b00000111);
}

#endif//MFP_DECLA_H
