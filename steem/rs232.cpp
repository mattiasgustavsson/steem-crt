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
FILE: rs232.cpp
DESCRIPTION: Serial port emulation.
---------------------------------------------------------------------------*/
#include "pch.h"
#pragma hdrstop

#include <computer.h>


bool UpdateBaud=0;
BYTE rs232_recv_byte=0;
bool rs232_recv_overrun=0;
DWORD rs232_bits_per_word=8,rs232_hbls_per_word=1;

// Send this to modem to test ATDT1471\r

void RS232_VBL(int) {
  DWORD Flags=SerialPort.GetModemFlags();
  // If flag is on bit=0
  mfp_gpip_set_bit(MFP_GPIP_CTS_BIT,((Flags&MS_CTS_ON)==0));
  mfp_gpip_set_bit(MFP_GPIP_DCD_BIT,((Flags&MS_RLSD_ON)==0));
  mfp_gpip_set_bit(MFP_GPIP_RING_BIT,((Flags&MS_RING_ON)==0));
  if(SerialPort.IsPCPort())
  {
    agenda_delete(RS232_VBL);
    agenda_add(RS232_VBL,int((SSEConfig.ColourMonitor)?6:14),0);
  }
}


BYTE RS232_ReadReg(int Reg) {
  switch(Reg) {
  case MFPR_RSR:
    if(!rs232_recv_overrun) 
      Mfp.reg[MFPR_RSR]&=(BYTE)~BIT_6;
    break;
  case MFPR_TSR:
    Mfp.reg[MFPR_RSR]&=(BYTE)~BIT_6; // Clear underrun
    break;
  case MFPR_UDR:
    Mfp.reg[MFPR_RSR]&=(BYTE)~BIT_7; //clear RX buffer full
    if(rs232_recv_overrun)
    {
      Mfp.reg[MFPR_RSR]|=BIT_6;
      rs232_recv_overrun=false;
      mfp_interrupt_pend(((mfp_interrupt_enabled[MFP_INT_RS232_RECEIVE_ERROR])
        ? MFP_INT_RS232_RECEIVE_ERROR : MFP_INT_RS232_RECEIVE_BUFFER_FULL),
        ABSOLUTE_CPU_TIME);
    }
    return rs232_recv_byte;
  }
  return Mfp.reg[Reg];
}


void RS232_CalculateBaud(bool Div16,BYTE cr,bool SetBaudNow) {
  if(cr)
  { //Timer D running
    int hbls_per_second=(int)((SSEConfig.ColourMonitor)
      ? HBLS_PER_SECOND_AVE : HBLS_PER_SECOND_MONO);
    if(Div16)
    {
      rs232_hbls_per_word=(mfp_timer_prescale[cr]
        *BYTE_00_TO_256(Mfp.reg[MFPR_TDDR]))*16*
        rs232_bits_per_word*hbls_per_second/MFP_CLK_EXACT;
    }
    else
    {
      // max works with int, and we made rs232_hbls_per_word unsigned
      rs232_hbls_per_word=(mfp_timer_prescale[cr]
        *BYTE_00_TO_256(Mfp.reg[MFPR_TDDR]))*
        rs232_bits_per_word*hbls_per_second/MFP_CLK_EXACT;
//     ASSERT( rs232_hbls_per_word>=1 ); //asserts
      if(rs232_hbls_per_word<1)
        rs232_hbls_per_word=1;
    }
    //ASSERT( rs232_hbls_per_word>=0 );
    if(SerialPort.IsPCPort())
    {
      if(!SetBaudNow)
      {
        UpdateBaud=true;
        return;
      }
      BYTE UCR=Mfp.reg[MFPR_UCR];
      double Baud=double(19200*4)/(mfp_timer_prescale[cr]
        *BYTE_00_TO_256(Mfp.reg[MFPR_TDDR]));
      if(!Div16) 
        Baud*=16;
      BYTE StopBits=ONESTOPBIT;
      switch(UCR & b00011000) {
      case b00010000:
        StopBits=ONE5STOPBITS;
        break;
      case b00011000:
        StopBits=TWOSTOPBITS;
        break;
      }
      int RTS=(psg_reg[PSGR_PORT_A]&BIT_3)
        ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE;
      int DTR=(psg_reg[PSGR_PORT_A]&BIT_4)
        ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;
      SerialPort.SetupCOM(DWORD(Baud),0,RTS,DTR,
        ((UCR & BIT_2)!=0),BYTE((UCR & BIT_1)?EVENPARITY:ODDPARITY),
        StopBits,BYTE(8-((UCR & b01100000)>>5)));
      UpdateBaud=false;
    }
  }
  else
    rs232_hbls_per_word=80000000; //SS? 
}


void RS232_WriteReg(int Reg,BYTE NewVal) {
  switch(Reg) {
  case MFPR_UCR:
  {
    DWORD old_bpw=rs232_bits_per_word;
    rs232_bits_per_word=1+BYTE(8-((NewVal & b01100000)>>5))+1;
    switch(NewVal & b00011000) {
    case b00010000: // 1.5
    case b00011000: // 2
      rs232_bits_per_word++;
      break;
    }
    //ASSERT(rs232_bits_per_word>0);
    NewVal&=b11111110;
    if((Mfp.reg[MFPR_UCR]&BIT_7)!=(NewVal & BIT_7)
      ||old_bpw!=rs232_bits_per_word)
    {
      Mfp.reg[MFPR_UCR]=NewVal;
      RS232_CalculateBaud(((NewVal & BIT_7)!=0),
        Mfp.get_timer_control_register(3),0);
    }
    break;
  }
  case MFPR_RSR:
    if((NewVal & BIT_0)==0&&(Mfp.reg[MFPR_RSR]&BIT_0))
      //disable receiver
      NewVal=0;
    NewVal&=BYTE(~BIT_7);
    NewVal|=BYTE(Mfp.reg[MFPR_RSR]&BIT_7);
    break;
  case MFPR_TSR:
    if((NewVal & BIT_0)&&(Mfp.reg[MFPR_TSR]&BIT_0)==0)
      //enable transmitter
      NewVal&=BYTE(~BIT_4); //Clear END
    NewVal&=BYTE(~BIT_7);
    NewVal|=BYTE(Mfp.reg[MFPR_TSR]&BIT_7);
    if((NewVal & BIT_3)!=(Mfp.reg[MFPR_TSR]&BIT_3))
    {
      if(NewVal & BIT_3)
      {
        SerialPort.StartBreak();
        agenda_delete(agenda_serial_sent_byte);
        agenda_add(agenda_serial_break_boundary,rs232_hbls_per_word,0);
      }
      else
      {
        SerialPort.EndBreak();
        agenda_delete(agenda_serial_break_boundary);
        if((Mfp.reg[MFPR_TSR]&BIT_7)==0)
        { // tx buffer not empty
          agenda_add(agenda_serial_sent_byte,2,0);
          //SS agenda_serial_sent_byte(0);
        }
      }
    }
    break;
  case MFPR_UDR:
    if((Mfp.reg[MFPR_TSR]&BIT_0)&&(Mfp.reg[MFPR_TSR]&BIT_3)==0)
    {
      // Transmitter enabled and no break
      if(UpdateBaud) 
        RS232_CalculateBaud(((Mfp.reg[MFPR_UCR]&BIT_7)!=0),
          Mfp.get_timer_control_register(3),true);
      Mfp.reg[MFPR_TSR]&=BYTE(~BIT_7);
      agenda_add(agenda_serial_sent_byte,rs232_hbls_per_word,0);
      if((Mfp.reg[MFPR_TSR]&b00000110)==b00000110)
        //loopback
        agenda_add(agenda_serial_loopback_byte,rs232_hbls_per_word+1,NewVal);
      else
        SerialPort.OutputByte(BYTE(NewVal 
          & (0xff>>((Mfp.reg[MFPR_UCR]&b01100000)>>5))));
    }
    return;
  }
  Mfp.reg[Reg]=NewVal;
}


void agenda_serial_sent_byte(int) {
  Mfp.reg[MFPR_TSR]|=BYTE(BIT_7); //buffer empty
  mfp_interrupt_pend(MFP_INT_RS232_TRANSMIT_BUFFER_EMPTY,ABSOLUTE_CPU_TIME)
  if((Mfp.reg[MFPR_TSR]&BIT_0)==0)
  { // transmitter disabled
    Mfp.reg[MFPR_TSR]|=BYTE(BIT_4); //End
    mfp_interrupt_pend(MFP_INT_RS232_TRANSMIT_ERROR,ABSOLUTE_CPU_TIME)
    if(Mfp.reg[MFPR_TSR]&BIT_5) 
      Mfp.reg[MFPR_RSR]|=BIT_0; //Auto turnaround! 
  }
}


void agenda_serial_break_boundary(int) {
  if((Mfp.reg[MFPR_TSR]&BIT_6)==0) 
    mfp_interrupt_pend(MFP_INT_RS232_TRANSMIT_ERROR,ABSOLUTE_CPU_TIME)
  agenda_add(agenda_serial_break_boundary,rs232_hbls_per_word,0);
}


void agenda_serial_loopback_byte(int NewVal) {
  if(Mfp.reg[MFPR_RSR]&BIT_0)
  {
    if((Mfp.reg[MFPR_RSR]&BIT_7 /*Buffer Full*/)==0)
    {
      rs232_recv_byte=BYTE(NewVal);
      rs232_recv_overrun=0;
    }
    else
      rs232_recv_overrun=true;
    Mfp.reg[MFPR_RSR]&=BYTE(~(BIT_2 /*Char in progress*/|BIT_3 /*Break*/|
      BIT_4 /*Frame Error*/|BIT_5 /*Parity Error*/));
    Mfp.reg[MFPR_RSR]|=BIT_7 /*Buffer Full*/;
    mfp_interrupt_pend(MFP_INT_RS232_RECEIVE_BUFFER_FULL,ABSOLUTE_CPU_TIME)
  }
}
