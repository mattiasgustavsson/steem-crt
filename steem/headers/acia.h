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
FILE: acia.h
DESCRIPTION: Declarations for high-level ACIA emulation.
struct TMC6850
acia.h must be compatible with C and with C++, it is included by 6310.c.
---------------------------------------------------------------------------*/

#pragma once
#ifndef ACIA_H
#define ACIA_H

#include <binary.h>

#ifdef __cplusplus
#include "options.h"
#include "emulator.h"
#include "steemh.h"
#include "debug.h"
#include "options.h"
#endif

enum EAcia {  ACIA_OVERRUN_NO=0,ACIA_OVERRUN_COMING,ACIA_OVERRUN_YES,
              ACIA_IKBD=0,ACIA_MIDI };

#pragma pack(push, 8)

#ifdef __cplusplus
extern "C" {
#endif

struct TMC6850 {
  // DATA
  // we keep the start of the old structure for snapshot compatibility
  int clock_divide;
  int rx_delay__unused;
  BYTE rx_irq_enabled;
  BYTE rx_not_read;
  int overrun;
  int tx_flag;
  BYTE tx_irq_enabled;
  BYTE data;
  BYTE irq;
  COUNTER_VAR last_tx_write_time;
  COUNTER_VAR last_rx_read_time;
  BYTE LineRxBusy;// receiveing from 6301 or MIDI
  BYTE ByteWaitingRx;
  BYTE ByteWaitingTx;
  BYTE LineTxBusy;// transmitting to 6301 or MIDI
  BYTE cr;    // control 
  BYTE sr;    // status
  BYTE rdr;   // receive data 
  BYTE tdr;   // transmit data
  BYTE rdrs;  // receive data shift
  BYTE tdrs;  // transmit data shift
  BYTE Id;
  COUNTER_VAR time_of_event_incoming, time_of_event_outgoing;
#ifdef __cplusplus
  // FUNCTIONS
  inline bool IrqForTx() { return ((cr&BIT_5)&&!(cr&BIT_6)); }
  static void BusJam();
  bool CheckIrq();
  void TransmitTDR();
  inline int TransmissionTime() {
/*  
Compute cycles according to bits 0-1 of cr
SS1 SS0                  Speed (bit/s)
 0   0    Normal            500000
 0   1    Div by 16          31250 (ST: MIDI)   cr&1=1
 1   0    Div by 64         7812.5 (ST: IKBD)   cr&1=0
MIDI transmission is 4 times faster than IKBD
*/
    int cycles=(cr&1) ? (10*16*16) : (10*16*64); //2560 for MIDI, 10240 for 6301
    if(SSEConfig.CpuBoosted)
    {
      double m=cycles*cpu_cycles_multiplier;
      cycles=(int)m;
    }
    return cycles;
  }
#endif
};

extern struct TMC6850 acia[2]; // 6301 needs the declaration

#pragma pack(pop)

#ifdef __cplusplus
}//extern "C"

#define ACIA_CHECK_IRQ(i) {acia[i].CheckIrq(); \
  mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!( (acia[ACIA_IKBD].sr&BIT_7) \
      || (acia[ACIA_MIDI].sr&BIT_7))); \
  }

int ACIAClockToHBLS(int ClockDivide,bool MIDI_In=false);
void ACIA_Reset(int nACIA,bool Cold);
void ACIA_SetControl(int nACIA,BYTE Val);
void agenda_acia_tx_delay_IKBD(int),agenda_acia_tx_delay_MIDI(int);

#endif

#endif//#ifndef ACIA_H
