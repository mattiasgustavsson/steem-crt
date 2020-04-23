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
FILE: acia.cpp
DESCRIPTION: High level emulation of the MC6850.
Two Motorola MC6850 ACIA (Asynchronous Communications Interface
Adapter) chips are used in the ST to handle communication between the CPU and 
1) the keyboard chip, 2) the MIDI ports.
Serial communication happens bit by bit on a single line per direction.
There are two emulations, one based on agendas (not OPTION_C1, scanline
precision) and one based on events (OPTION_C1, cycle precision). Both
emulations directly handle bytes, not bits.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop
#include <computer.h>
#include <debug.h>
#if defined(SSE_VID_STVL1)
#include <interface_stvl.h>
#endif


#define LOGSECTION LOGSECTION_ACIA

int ACIAClockToHBLS(int ClockDivide,bool MIDI_In/*=false*/) {
  int HBLs=1;
  // We assume the default setting of 9 bits per byte sent, I'm not sure
  // if the STs ACIAs worked in any other mode. The ACIA master clock is 500kHz.
  //ASSERT(ClockDivide==1||ClockDivide==2);
  if(Glue.video_freq==MONO_HZ) 
  {
    if(ClockDivide==1) // Divide by 16 (31250 bits per second)
      HBLs=int(HBLS_PER_SECOND_MONO/(500000.0/16.0/9.0)+1);
    else if(ClockDivide==2) // Divide by 64 (7812.5 bits per second)
      HBLs=int(HBLS_PER_SECOND_MONO/(500000.0/64.0/9.0)+1);
  }
  else 
  {
    if(ClockDivide==1)  // Divide by 16 (31250 bits per second)
      HBLs=(int)(HBLS_PER_SECOND_AVE/(500000.0/16.0/9.0)+1);
    else if(ClockDivide==2)  // Divide by 64 (7812.5 bits per second)
      HBLs=(int)(HBLS_PER_SECOND_AVE/(500000.0/64.0/9.0)+1);
  }
  if(MIDI_In 
#if !defined(SSE_LEAN_AND_MEAN)
    && MIDI_in_speed
#endif
    && MIDI_in_speed!=100) 
  {
    HBLs*=100;
    HBLs/=MIDI_in_speed;
  }
  return HBLs;
}


void ACIA_Reset(int nACIA,bool Cold) {
  TRACE_LOG("ACIA %d Reset (cold %d)\n",nACIA,Cold);
  acia[nACIA].tx_flag=0;
  if(nACIA==ACIA_IKBD) 
    agenda_delete(agenda_acia_tx_delay_IKBD);
  if(nACIA==ACIA_MIDI) 
    agenda_delete(agenda_acia_tx_delay_MIDI);
  acia[nACIA].rx_not_read=0;
  acia[nACIA].overrun=0;
  acia[nACIA].clock_divide=(nACIA==ACIA_MIDI) ? 1 : 2;
  acia[nACIA].tx_irq_enabled=0;
  acia[nACIA].rx_irq_enabled=true;
  acia[nACIA].data=0; //SS ?
  acia[nACIA].last_tx_write_time=0;
  LOG_ONLY(if(nACIA==0&&acia[nACIA].irq) log_to_section(LOGSECTION_IKBD,EasyStr("IKBD: ACIA reset - Changing ACIA IRQ bit from ")+acia[ACIA_IKBD].irq+" to 0"); )
  acia[nACIA].irq=false;
  acia[nACIA].Id=(BYTE)nACIA;
  if(OPTION_C1)
  {
/*  "Master Reset clears the status register (except for external conditions
    on CTS, DCD) and initializes both the receiver and the transmitter."
*/
    acia[nACIA].sr=2;
    acia[nACIA].cr=0x80; //?
    acia[nACIA].rdrs=0;
    acia[nACIA].tdrs=0;
    acia[nACIA].LineRxBusy=0;
    acia[nACIA].LineTxBusy=0;
  }
  if(Cold==0)
    mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,
    !(acia[ACIA_IKBD].irq||acia[ACIA_MIDI].irq));
}


void ACIA_SetControl(int nACIA,BYTE Val) {
  acia[nACIA].clock_divide=(Val & b00000011);
  acia[nACIA].tx_irq_enabled=((Val & b01100000)==BIT_5);
  acia[nACIA].rx_irq_enabled=((Val & b10000000)!=0);
  LOG_ONLY(if(nACIA==0) LOG_TO(LOGSECTION_IKBD,EasyStr("IKBD: ACIA control set to ")+itoa(Val,d2_t_buf,2)); )
#if defined(SSE_HARDWARE_OVERSCAN)
  // The overscan circuit is activated by using the free ACIA RTS pin (output)
  if(OPTION_HWOVERSCAN && nACIA==ACIA_IKBD)
  {
    SSEConfig.OverscanOn=((Val&BIT_6)!=0); // this is saved with the snapshot
#if defined(SSE_VID_STVL1)
    StvlUpdate();
#endif
  }
#endif
  if(OPTION_C1)
  {
    ACIA_CHECK_IRQ(nACIA);
    return;
  }
  if(acia[nACIA].tx_irq_enabled)
    acia[nACIA].irq=true;
  else 
  {
    LOG_ONLY(if(nACIA==0&&acia[ACIA_IKBD].irq) LOG_TO(LOGSECTION_IKBD,EasyStr("IKBD: ACIA set control - Changing ACIA IRQ bit from ")+acia[ACIA_IKBD].irq+" to 0"); )
    acia[nACIA].irq=false;
  }
  mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,
    !(acia[ACIA_IKBD].irq||acia[ACIA_MIDI].irq));
}


void TMC6850::BusJam() {
  // e-clock synchronisation on read/write
  // see 68000 UM -  we already counted 4 cycles for the R/W so we
  // do -4 here, important for good sync with video interrupts: Mental Hangover
  BYTE wait_states=6-4; 
  if(OPTION_C1)
  {
    BUS_JAM_TIME(wait_states);
    wait_states=Cpu.SyncEClock();
    BUS_JAM_TIME(wait_states+4); // +... +4
  }
  else
  {
    wait_states+=(EIGHT_MILLION-(ACT-shifter_cycle_base))%10;
    BUS_JAM_TIME(wait_states);
  }
}


bool TMC6850::CheckIrq() {
//  ASSERT(Id!=1);
  bool newirq=IrqForTx()&&(sr&BIT_1) // TX
    ||(cr&BIT_7)&&(sr&(BIT_0|BIT_5)); //RX/OVR
  if(newirq)
  {
    sr|=BIT_7;
    TRACE_LOG("ACIA %d IRQ, sr=%X\n",Id,sr);
  }
  else
    sr&=~BIT_7;
  return newirq;
}


void TMC6850::TransmitTDR() {
  // The byte in TDR is moved into TDRS and transmission begins.
  // TDRE is set, and IRQ is asserted if appropriate.
  tdrs=tdr; // there could be timing difference for TDRS and TDRE... 
  sr|=BIT_1; // TDRE free
  if(CheckIrq())
    mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,0); //trigger
  LineTxBusy=true;
  time_of_event_outgoing=time_of_next_event+TransmissionTime();
  if(time_of_event_outgoing-time_of_event_acia<=0)
    time_of_event_acia=time_of_event_outgoing;
}
