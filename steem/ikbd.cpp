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
FILE: ikbd.cpp
DESCRIPTION: The code to emulate the ST's Intellegent Keyboard Controller
(Hitachi HD6301 aka IKBD) that encompasses mouse, keyboard and joystick
input.
This file contains a high-level emulation (not OPTION_C1) as well as an 
interface for the low-level emulation (OPTION_C1) that is in 3rdparty/6301.
There are two approaches because one is older, we give the choice essentially
for the mouse pointer although it has been much improved in the low-level one.
Reprogramming is not implemented in the high-level emulation but some quirks
are (Barbarian...).
---------------------------------------------------------------------------*/

#include "pch.h"
#include <easystr.h>
#include <emulator.h>
#include <mymisc.h>
#include <gui.h>
#include <computer.h>
#include <stjoy.h>
#include <macros.h>
#include <draw.h>
#include <stports.h>
#include <shortcutbox.h>
#include <debug.h>
#include <ikbd.h>
#include <display.h>
#include <debug_framereport.h>
#include <key_table.h>
#include <osd.h>
#include <stdarg.h>


BYTE ST_Key_Down[128];
int disable_input_vbl_count=0;
int ikbd_joy_poll_line=0;
BYTE keyboard_buffer[MAX_KEYBOARD_BUFFER_SIZE];
WORD keyboard_buffer_length;
BYTE mouse_speed=10;
int mouse_move_since_last_interrupt_x,mouse_move_since_last_interrupt_y;
bool mouse_change_since_last_interrupt;
int mousek;

#ifndef CYGWIN
BYTE no_set_cursor_pos=0;
#else
BYTE no_set_cursor_pos=true;
#endif

#ifdef WIN32
// When task switching is turned off we have to manually update these keys at the VBL
// TODO does it work with C1?
BYTE TaskSwitchVKList[4]={VK_ESCAPE,VK_TAB,VK_DELETE,0};
bool CutTaskSwitchVKDown[4]={0,0,0,0};
#endif

const int ikbd_clock_days_in_mon[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};
const int ikbd_clock_max_val[6]={99,12,0,23,59,59};

#define LOGSECTION LOGSECTION_IKBD

void ikbd_run_start(bool reset) { // called at the start of run()
  if(reset) 
  {
    keyboard_buffer_length=0;
    keyboard_buffer[0]=0;
  }
  else
    UpdateSTKeys();
  mouse_move_since_last_interrupt_x=0;
  mouse_move_since_last_interrupt_y=0;
  mouse_change_since_last_interrupt=false;
  JoyGetPoses();
}


bool ikbd_keys_disabled() { // called by HandleKeyPress() if !C1
  return Ikbd.joy_mode>=100;
}


void IKBD_VBL() {
  if(!OPTION_RTC_HACK && (++Ikbd.clock_vbl_count)>=video_freq_at_start_of_vbl)
  {
    Ikbd.clock_vbl_count=0;
    for(int n=5;n>=0;n--)
    {
      int val=(Ikbd.clock[n]>>4)*10+(Ikbd.clock[n]&0xf);
      int max_val=ikbd_clock_max_val[n];
      if(max_val==0) 
      {
        int mon=MIN((Ikbd.clock[1]>>4)*10+(Ikbd.clock[1]&0xf),12);
        max_val=ikbd_clock_days_in_mon[mon];
      }
      bool increase_next=0;
      if((++val)>max_val) 
      {
        val=(n==1)?1:0;
        increase_next=true;
      }
      if(n==0) 
        val%=100;
      Ikbd.clock[n]=BYTE((val%10)|((val/10)<<4));
      if(increase_next==0) 
        break;
    }
  }
  if(macro_start_after_ikbd_read_count) 
    return;
  // randomize polling scanline of joystick (note it's not done for the rest)
  int max_line=scanlines_above_screen[video_freq_idx]+(MONO?400:200);
  ikbd_joy_poll_line+=527;
  ikbd_joy_poll_line%=max_line;
  if(macro_play_has_keys) 
    macro_play_keys();
  static BYTE old_stick[2];
  old_stick[0]=stick[0];
  old_stick[1]=stick[1];
  bool old_joypar1_bit4=(stick[N_JOY_PARALLEL_1]&BIT_4)!=0;
  if(macro_play_has_joys)
    macro_play_joy();
  else
  {
    if(disable_input_vbl_count==0) 
    {
      joy_read_buttons();
      for(int Port=0;Port<8;Port++) 
        stick[Port]=joy_get_pos(Port);
#if defined(SSE_STATS)
      if((old_stick[0]&0x7F)!=(stick[0]&0x7F))
        Stats.nJoy0++;
      if((old_stick[1]&0x7F)!=(stick[1]&0x7F))
        Stats.nJoy1++;
      if((old_stick[0]&0x80)!=(stick[0]&0x80))
        Stats.nClick0++;
      if((old_stick[1]&0x80)!=(stick[1]&0x80))
        Stats.nClick1++;
#endif
#if defined(SSE_DONGLE)
      switch(DONGLE_ID) {
#if defined(SSE_DONGLE_LEADERBOARD) 
      case TDongle::LEADERBOARD:
      case TDongle::TENTHFRAME:
        stick[1]|=3; // up and down
        break;
#endif
#if defined(SSE_DONGLE_CRICKET) 
      case TDongle::CRICKET: // port 0 bit 0 toggles (?)
      case TDongle::SOCCER: // port 0 = $D or $C
      case TDongle::RUGBY: // port 1 = $D or $C (?)
        Dongle.Value=(Dongle.Value==0xC)?0xD:0xC;
        stick[(DONGLE_ID==TDongle::RUGBY)?1:0]|=Dongle.Value;
        break;
#endif
      }//sw
#endif
#if defined(SSE_HD6301_LL)
      if(stick[0]&0xF)
        SSEConfig.Port0Joy=true; // it's joystick or mouse
#endif
    }
    else // disable_input_vbl_count
      for(int Port=0;Port<8;stick[Port++]=0) ;
    if(IsJoyActive(N_JOY_PARALLEL_0)) 
      stick[N_JOY_PARALLEL_0]|=BIT_4;
    if(IsJoyActive(N_JOY_PARALLEL_1)) 
      stick[N_JOY_PARALLEL_1]|=BIT_4;
  }
  // if 6301 is on, the ROM will send the packets
  if(!OPTION_C1) switch(Ikbd.joy_mode) {
  case IKBD_JOY_MODE_DURATION:
    keyboard_buffer_write(BYTE(int((stick[0]&MSB_B)?BIT_1:0)|int((stick[1]&MSB_B)?BIT_0:0)));
    keyboard_buffer_write(BYTE(((stick[0]&0xf)<<4)|(stick[1]&0xf)));
    break;
  case IKBD_JOY_MODE_AUTO_NOTIFY:
    for(BYTE j=!Ikbd.port_0_joy;j<2;j++) 
    {
      BYTE os=old_stick[j],s=stick[j];
      // If mouse active then joystick button never down
      if(!Ikbd.port_0_joy) 
        os&=0x0f,s&=0x0f;
      if(os!=s) 
        agenda_add(ikbd_send_joystick_message,ikbd_joy_poll_line,j);
    }
    break;
  case IKBD_JOY_MODE_FIRE_BUTTON_DURATION:
    if(stick[1]&MSB_B)
      keyboard_buffer_write_string(0xff,0xff,0xff,0xff,0xff,0xff,-1);
    else
      keyboard_buffer_write_string(0,0,0,0,0,0,-1);
    break;
  case IKBD_JOY_MODE_CURSOR_KEYS: {
    if(stick[0]&(~old_stick[0])&0xc) 
    { //new press left/right
      Ikbd.cursor_key_joy_ticks[0]=timeGetTime(); //reset timer left/right
      Ikbd.cursor_key_joy_ticks[2]=timeGetTime(); //last report
      if(stick[0]&4) 
      {
        keyboard_buffer_write(0x4b);
        keyboard_buffer_write(0x4b|MSB_B);
      }
      else 
      {
        keyboard_buffer_write(0x4d);
        keyboard_buffer_write(0x4d|MSB_B);
      }
    }
    else if(stick[0]&(~old_stick[0])&0x3) 
    {
      Ikbd.cursor_key_joy_ticks[1]=timeGetTime(); //reset timer up/down
      Ikbd.cursor_key_joy_ticks[3]=timeGetTime(); //last report
      if(stick[0]&1) 
      {
        keyboard_buffer_write(0x48);
        keyboard_buffer_write(0x48|MSB_B);
      }
      else 
      {
        keyboard_buffer_write(0x50);
        keyboard_buffer_write(0x50|MSB_B);
      }
    }
    else if(stick[0]) 
    {
      for(int xy=0;xy<2;xy++) 
      {
        BYTE s=stick[0]&BYTE(0xc>>(xy*2));
        if(s) 
        { //one of these directions pressed
          DWORD interval=(timeGetTime()-Ikbd.cursor_key_joy_ticks[2+xy])/100;
          DWORD elapsed=(timeGetTime()-Ikbd.cursor_key_joy_ticks[xy])/100;
          bool report=false;
          BYTE key;
          if(elapsed>Ikbd.cursor_key_joy_time[xy]) 
          { //>Rx
            if(interval>Ikbd.cursor_key_joy_time[2+xy])  //Tx
              report=true;
          }
          else if(interval>Ikbd.cursor_key_joy_time[4+xy]) //Vx
            report=true;
          if(report) 
          {
            if(s&8) 
              key=0x4d;
            else if(s&4) 
              key=0x4b;
            else if(s&2) 
              key=0x50;
            else key=0x48;
            keyboard_buffer_write(key);
            keyboard_buffer_write(key|MSB_B);
            Ikbd.cursor_key_joy_ticks[2+xy]=timeGetTime();
          }
        }
      }
    }
    break;
  }//case
  }//sw
  if(macro_record)
  {
    macro_jagpad[0]=GetJagPadDown(N_JOY_STE_A_0,0xffffffff);
    macro_jagpad[1]=GetJagPadDown(N_JOY_STE_B_0,0xffffffff);
    macro_record_joy();
  }
  // Handle io line for parallel port joystick 1 (busy bit cleared if fire is pressed)
  if(stick[N_JOY_PARALLEL_1]&BIT_4)
    mfp_gpip_set_bit(0,((stick[N_JOY_PARALLEL_1]&BIT_7)==0));
  else if(old_joypar1_bit4)
    UpdateCentronicsBusyBit();
  int old_mousek=mousek;
  mousek=0;
  if(stick[0]&128)
    mousek|=BIT_LMB;
  if(stick[1]&128)
    mousek|=BIT_RMB;
  if(stem_mousemode==STEM_MOUSEMODE_WINDOW)
  {
    POINT pt;  GetCursorPos(&pt);
    if(pt.x!=window_mouse_centre_x||pt.y!=window_mouse_centre_y)
    {
#if defined(SSE_STATS)
      if(pt.x!=window_mouse_centre_x)
        Stats.nMousex++;
      if(pt.y!=window_mouse_centre_y)
        Stats.nMousey++;
#endif
      // disable_input_vbl_count is used when you reset to prevent TOS getting IKBD messages
      // before it is ready (causes annoying clicking). It is also used when you change disk
      // in order to prevent you from continuing before the new disk has been fully inserted.
      // In the latter case we do not need to disable mouse movement.
      if(disable_input_vbl_count<=30) 
      {
        mouse_move_since_last_interrupt_x+=(pt.x-window_mouse_centre_x);
        mouse_move_since_last_interrupt_y+=(pt.y-window_mouse_centre_y);
        if(mouse_speed!=10) 
        {
          int x_if_0=0;
          if(mouse_move_since_last_interrupt_x<0) 
            x_if_0=-1;
          if(mouse_move_since_last_interrupt_x>0) 
            x_if_0=1;
          int y_if_0=0;
          if(mouse_move_since_last_interrupt_y<0) 
            y_if_0=-1;
          if(mouse_move_since_last_interrupt_y>0) 
            y_if_0=1;
          mouse_move_since_last_interrupt_x*=mouse_speed;
          mouse_move_since_last_interrupt_y*=mouse_speed;
          mouse_move_since_last_interrupt_x/=10;
          mouse_move_since_last_interrupt_y/=10;
          if(mouse_move_since_last_interrupt_x==0) 
            mouse_move_since_last_interrupt_x=x_if_0;
          if(mouse_move_since_last_interrupt_y==0) 
            mouse_move_since_last_interrupt_y=y_if_0;
        }
        if(Ikbd.mouse_upside_down)
          mouse_move_since_last_interrupt_y=-mouse_move_since_last_interrupt_y;
        mouse_change_since_last_interrupt=true;
      }
      //  SetCursorPos() and ClipCursor() don't work in some VM because the host
      //  keeps control of the mouse cursor. This fixes the drifting mouse issue.
      if(OPTION_VMMOUSE || no_set_cursor_pos) 
      {
        window_mouse_centre_x=pt.x;
        window_mouse_centre_y=pt.y;
      }
      else
        SetCursorPos(window_mouse_centre_x,window_mouse_centre_y);
    }
  }
  if(macro_record)
    macro_record_mouse(mouse_move_since_last_interrupt_x,mouse_move_since_last_interrupt_y);
  if(macro_play_has_mouse) 
  {
    mouse_change_since_last_interrupt=0;
    macro_play_mouse(mouse_move_since_last_interrupt_x,mouse_move_since_last_interrupt_y);
    if(mouse_move_since_last_interrupt_x||mouse_move_since_last_interrupt_y)
      mouse_change_since_last_interrupt=true;
  }
  int report_button_abs=0;
  if(mousek!=old_mousek)
  {
    bool send_change_for_button=true;
    // Handle absolute mouse button flags
    if(RMB_DOWN(mousek)&&RMB_DOWN(old_mousek)==0) 
      report_button_abs|=BIT_0;
    if(RMB_DOWN(mousek)==0&&RMB_DOWN(old_mousek)) 
      report_button_abs|=BIT_1;
    if(LMB_DOWN(mousek)&&LMB_DOWN(old_mousek)==0) 
      report_button_abs|=BIT_2;
    if(LMB_DOWN(mousek)==0&&LMB_DOWN(old_mousek)) 
      report_button_abs|=BIT_3;
    Ikbd.abs_mousek_flags|=report_button_abs;
    if(OPTION_C1)
      ;
      // Handle mouse buttons as keys
    else if(Ikbd.mouse_button_press_what_message & BIT_2) 
    {
      if(2&(mousek^old_mousek)) 
        keyboard_buffer_write(BYTE((mousek&2)?0x74:0xf4)); //if mouse button 1
      if(1&(mousek^old_mousek)) 
        keyboard_buffer_write(BYTE((mousek&1)?0x75:0xf5)); //if mouse button 2
      send_change_for_button=0; // Don't send mouse packet if you haven't moved mouse
      report_button_abs=0; // No ABS reporting
    }
    else if(Ikbd.mouse_mode==IKBD_MOUSE_MODE_ABSOLUTE) 
    {
      if((Ikbd.mouse_button_press_what_message & BIT_0)==0) // Don't report ABS on press
        report_button_abs&=~(BIT_0|BIT_2);
      if((Ikbd.mouse_button_press_what_message & BIT_1)==0)  // Don't report ABS on release
        report_button_abs&=~(BIT_1|BIT_3);
    }
    else
      report_button_abs=0;
    if(!OPTION_C1 && send_change_for_button)
      mouse_change_since_last_interrupt=true;
  }
  if(mouse_change_since_last_interrupt) 
  {
    int max_mouse_move=IKBD_DEFAULT_MOUSE_MOVE_MAX; //15
    if(macro_play_has_mouse) 
      max_mouse_move=macro_play_max_mouse_speed;
    ikbd_mouse_move(mouse_move_since_last_interrupt_x,
      mouse_move_since_last_interrupt_y,mousek,max_mouse_move);
    mouse_change_since_last_interrupt=false;
    mouse_move_since_last_interrupt_x=0;
    mouse_move_since_last_interrupt_y=0;
#if defined(SSE_HD6301_LL)
    SSEConfig.Port0Joy=0;
#endif
  }
#if defined(SSE_HD6301_LL)
  else
    Ikbd.MouseVblDeltaX=Ikbd.MouseVblDeltaY=0;
#endif
  if(!OPTION_C1 && report_button_abs) 
  {
    for(int bit=BIT_0;bit<=BIT_3;bit<<=1) {
      if(report_button_abs & bit) 
        ikbd_report_abs_mouse(report_button_abs & bit);
    }
  }
  if(macro_play_has_keys==0)
  {
    // Check modifier keys, it's simpler to check them like this rather than
    // respond to messages
    TModifierState mss=GetLRModifierStates();
    bool StemWinActive=GUICanGetKeys();
    if(joy_is_key_used(VK_SHIFT)||CutDisableKey[VK_SHIFT]||!StemWinActive) 
      mss.LShift=mss.RShift=false;
    if(joy_is_key_used(VK_LSHIFT)||CutDisableKey[VK_LSHIFT]) 
      mss.LShift=false;
    if(joy_is_key_used(VK_RSHIFT)||CutDisableKey[VK_RSHIFT]) 
      mss.RShift=false;
    if(joy_is_key_used(VK_CONTROL)||CutDisableKey[VK_CONTROL]||!StemWinActive) 
      mss.LCtrl=mss.RCtrl=false;
    if(joy_is_key_used(VK_LCONTROL)||CutDisableKey[VK_LCONTROL]) 
      mss.LCtrl=false;
    if(joy_is_key_used(VK_RCONTROL)||CutDisableKey[VK_RCONTROL]) 
      mss.RCtrl=false;
    if(joy_is_key_used(VK_MENU)||CutDisableKey[VK_MENU]||!StemWinActive) 
      mss.LAlt=mss.RAlt=false;
    if(joy_is_key_used(VK_LMENU)||CutDisableKey[VK_LMENU]) 
      mss.LAlt=false;
    if(joy_is_key_used(VK_RMENU)||CutDisableKey[VK_RMENU]) 
      mss.RAlt=false;
    int ModDown=ExternalModDown|CutModDown;
    if(ModDown & b00000001) 
      mss.LShift=true;
    if(ModDown & b00000010) 
      mss.RShift=true;
    if(ModDown & b00001100) 
      mss.LCtrl=true;
    if(ModDown & b00110000) 
      mss.LAlt=true;
    // what if we're late? should be done before?
    if(ST_Key_Down[key_table[VK_LSHIFT]]!=(BYTE)mss.LShift)
      HandleKeyPress(VK_LSHIFT,!mss.LShift,IGNORE_EXTEND);
    if(ST_Key_Down[key_table[VK_RSHIFT]]!=(BYTE)mss.RShift)
      HandleKeyPress(VK_RSHIFT,!mss.RShift,IGNORE_EXTEND);
    if(ST_Key_Down[key_table[VK_CONTROL]]!=(mss.LCtrl||mss.RCtrl))
      HandleKeyPress(VK_CONTROL,!(mss.LCtrl||mss.RCtrl),IGNORE_EXTEND);
    if(ST_Key_Down[key_table[VK_MENU]]!=(mss.LAlt||mss.RAlt))
      HandleKeyPress(VK_MENU,!(mss.LAlt||mss.RAlt),IGNORE_EXTEND);
#ifdef WIN32
#if !defined(ONEGAME)
    if(TaskSwitchDisabled) 
    {
      BYTE n=0,Key;
      while(TaskSwitchVKList[n]) 
      {
        Key=TaskSwitchVKList[n];
        if(!joy_is_key_used(Key)&&!CutDisableKey[Key]&&!CutTaskSwitchVKDown[n]) 
        {
          if(ST_Key_Down[key_table[Key]]!=(GetAsyncKeyState(Key)<0))
            HandleKeyPress(Key,GetAsyncKeyState(Key)>=0,IGNORE_EXTEND);
        }
        n++;
      }
    }
#endif
#endif
  }
  macro_advance();
  if(disable_input_vbl_count) 
    disable_input_vbl_count--;
}


void ikbd_inc_hack(int &hack_val,int inc_val) {
  if(!Ikbd.resetting)
    return;
  if(hack_val==inc_val)
    hack_val=inc_val+1;
  else
    hack_val=-1;
}


void agenda_ikbd_process(int src) {   //intelligent keyboard handle byte
/*  All bytes that are written by the program or the OS on address $fffc02 
    end up here, after a transmission delay.
*/
  DBG_LOG(EasyStr("IKBD: At ")+hbl_count+" receives $"+HEXSl(src,2));
#ifdef SSE_DEBUG
  TRACE_LOG("%d %d %d IKBD RDRS %X\n",TIMING_INFO,src);
#endif
#if defined(SSE_IKBDI)
  // our powerful 6301 command interpreter, working for both emulations
  Ikbd.InterpretCommand((BYTE)src); 
#endif
#if defined(SSE_HD6301_LL)
  if(OPTION_C1)
  {
    //ASSERT(acia[ACIA_IKBD].tdrs==src);
    Ikbd.rdrs=acia[ACIA_IKBD].tdrs;
    hd6301_run_cycles(ACT); // run up to IO time
    hd6301_receive_byte((BYTE)src);
    acia[ACIA_IKBD].LineTxBusy=false;
    // If there's another byte in TDR waiting to be shifted, do it now.
    if(!(acia[ACIA_IKBD].sr&BIT_1))
      acia[ACIA_IKBD].TransmitTDR();
    // That's it for Steem, the rest is handled by the program in ROM!
    return; 
  }//if
#endif//6301
  Ikbd.send_nothing=0;  // This should only happen if valid command is received!
  if(Ikbd.command_read_count)
  {
    if(Ikbd.command!=0x50) //load memory rubbish
      Ikbd.command_param[Ikbd.command_parameter_counter++]=(BYTE)src;
    else
    { // Save into IKBD RAM (this won't be used by this high-level emu)
      if(Ikbd.load_memory_address>=0x80&&Ikbd.load_memory_address<=0xff)
        Ikbd.ram[Ikbd.load_memory_address-0x80]=(BYTE)src;
      Ikbd.load_memory_address++;
    }
    Ikbd.command_read_count--;
    if(Ikbd.command_read_count<=0)
    {
      //ASSERT(Ikbd.command_read_count==0); // we've command & all parameters
      switch(Ikbd.command) {
      case 0x7: // Set what package is returned when mouse buttons are pressed
        Ikbd.mouse_button_press_what_message=Ikbd.command_param[0];
        Ikbd.port_0_joy=0;
        break;
      case 0x9: // Absolute mouse mode
        Ikbd.mouse_mode=IKBD_MOUSE_MODE_ABSOLUTE;
        Ikbd.abs_mouse_max_x=MAKEWORD(Ikbd.command_param[1],Ikbd.command_param[0]);
        Ikbd.abs_mouse_max_y=MAKEWORD(Ikbd.command_param[3],Ikbd.command_param[2]);
        Ikbd.port_0_joy=0;
#if 0   // don't need all this, eg Manchester United
        Ikbd.abs_mouse_x=Ikbd.abs_mouse_max_x/2;
        Ikbd.abs_mouse_y=Ikbd.abs_mouse_max_y/2;
        Ikbd.abs_mousek_flags=0; 
        if(RMB_DOWN(mousek))
          Ikbd.abs_mousek_flags|=BIT_0;
        if(LMB_DOWN(mousek))
          Ikbd.abs_mousek_flags|=BIT_2;
#endif
        break;
      case 0xa: // Return mouse movements as cursor keys
        Ikbd.mouse_mode=IKBD_MOUSE_MODE_CURSOR_KEYS;
        Ikbd.cursor_key_mouse_pulse_count_x=MAX(int(Ikbd.command_param[0]),1);
        Ikbd.cursor_key_mouse_pulse_count_y=MAX(int(Ikbd.command_param[1]),1);
        Ikbd.port_0_joy=0;
        agenda_delete(ikbd_report_abs_mouse);
        break;
      case 0xb: // Set relative mouse threshold
        Ikbd.port_0_joy=0;
        Ikbd.relative_mouse_threshold_x=Ikbd.command_param[0];
        Ikbd.relative_mouse_threshold_y=Ikbd.command_param[1];
        ikbd_inc_hack(Ikbd.psyg_hack_stage,1);
        break;
      case 0xc://set absolute mouse threshold
        Ikbd.port_0_joy=0;
        Ikbd.abs_mouse_scale_x=Ikbd.command_param[0];
        Ikbd.abs_mouse_scale_y=Ikbd.command_param[1];
        break;
      case 0xe://set mouse position in IKBD
        Ikbd.port_0_joy=0;
        Ikbd.abs_mouse_x=MAKEWORD(Ikbd.command_param[2],Ikbd.command_param[1]);
        Ikbd.abs_mouse_y=MAKEWORD(Ikbd.command_param[4],Ikbd.command_param[3]);
        break;
      case 0x17://joystick duration
        DBG_LOG("IKBD: Joysticks set to duration mode");
        Ikbd.joy_mode=IKBD_JOY_MODE_DURATION;
        Ikbd.duration=Ikbd.command_param[0]*10; //in 1000ths of a second
        Ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;  //disable mouse
        Ikbd.port_0_joy=1;
        agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
        agenda_delete(ikbd_report_abs_mouse);
        break;
      case 0x19://cursor key simulation mode for joystick 0
        Ikbd.joy_mode=IKBD_JOY_MODE_CURSOR_KEYS;
        for(int n=0;n<6;n++)
          Ikbd.cursor_key_joy_time[n]=Ikbd.command_param[n];
        Ikbd.cursor_key_joy_ticks[0]=timeGetTime();
        Ikbd.cursor_key_joy_ticks[1]=Ikbd.cursor_key_joy_ticks[0];
        Ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;  //disable mouse
        Ikbd.port_0_joy=1;
        agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
        agenda_delete(ikbd_report_abs_mouse);
        break;
      case 0x1b://set clock time
        DBG_LOG("IKBD: Set clock to... ");
        for(int n=0;n<6;n++) 
        {
          int newval=Ikbd.command_param[n];
#if defined(SSE_IKBD_RTC)
          if(OPTION_BATTERY6301==2&&(!n))
            newval-=0xA0;
#endif
          if((newval&0xf0)>=0xa0) 
          { // Invalid high nibble
            newval&=0x0f;
            newval|=Ikbd.clock[n]&0xf0;
          }
          if((newval&0xf)>=0xa) 
          { // Invalid low nibble
            newval&=0xf0;
            newval|=Ikbd.clock[n]&0x0f;
          }
          int val=(newval>>4)*10+(newval&0xf);
          int max_val=ikbd_clock_max_val[n];
          if(max_val==0) 
          {
            int mon=MIN((Ikbd.clock[1]>>4)*10+(Ikbd.clock[1]&0xf),12);
            max_val=ikbd_clock_days_in_mon[mon];
          }
          if(val>max_val)
          {
            val=0;
            if(n==1)
              val=1;
          }
          Ikbd.clock[n]=BYTE((val%10)|((val/10)<<4));
          DBG_LOG(HEXSl(Ikbd.clock[n],2));
        }
        Ikbd.clock_vbl_count=0;
        break;
      case 0x20:  //load memory
        Ikbd.command=0x50; // Ant's command about loading memory
        Ikbd.load_memory_address=MAKEWORD(Ikbd.command_param[1],Ikbd.command_param[0]);
        Ikbd.command_read_count=Ikbd.command_param[2]; //how many bytes to load
        DBG_LOG(Str("IKBD: Loading next ")+Ikbd.command_read_count+" bytes into IKBD memory address "+
              HEXSl(Ikbd.load_memory_address,4));
        break;
      case 0x50:
        DBG_LOG("IKBD: Finished loading memory");
        break;    //but instead just throw it away!
      case 0x21: { //read memory
        WORD adr=MAKEWORD(Ikbd.command_param[1],Ikbd.command_param[0]);
        DBG_LOG(Str("IKBD: Reading 6 bytes of IKBD memory, address ")+HEXSl(adr,4));
        keyboard_buffer_write_string(0xf6,0x20,(-1));
        for(int n=0;n<6;n++)
        {
          BYTE b=0;
          if(adr>=0x80&&adr<=0xff) 
            b=Ikbd.ram[adr-0x80];
          keyboard_buffer_write(b);
        }
        break;
      }
      case 0x22:  //execute routine
        //TRACE2("IKBD $22\n");
        DBG_LOG(Str("IKBD: Blimey! Executing IKBD routine at ")+
              HEXSl(MAKEWORD(Ikbd.command_param[1],Ikbd.command_param[0]),4));
        break;   
      case 0x80:  
        if(src==0x01) 
          ikbd_reset(0);
        break;
      }//sw
    }
  }
  else
  { //new command, some commands are executed at once, others require parameters
/*
After any joystick command, the ikbd assumes that joysticks are connected to both Joystick0
and Joystick1. Any mouse command (except MOUSE DISABLE) then causes port 0 to again
be scanned as if it were a mouse, and both buttons are logically connected to it. If a mouse
disable command is received while port 0 is presumed to be a mouse, the button is logically
assigned to Joystick1 ( until the mouse is reenabled by another mouse command).
*/
    if(Ikbd.joy_mode==IKBD_JOY_MODE_FIRE_BUTTON_DURATION) 
      Ikbd.joy_mode=IKBD_JOY_MODE_OFF;
    if(Ikbd.resetting && src!=0x08&&src!=0x14) 
      Ikbd.reset_0814_hack=-1;
    if(Ikbd.resetting && src!=0x12&&src!=0x14) 
      Ikbd.reset_1214_hack=-1;
    if(Ikbd.resetting && src!=0x08&&src!=0x0B&&src!=0x14) 
      Ikbd.psyg_hack_stage=-1;
    if(Ikbd.resetting && src!=0x12&&src!=0x1A) 
      Ikbd.reset_121A_hack=-1;
    Ikbd.command=(BYTE)src;
    switch(src) {  //how many bytes of parameters do we want?
    case 0x7:case 0x17:case 0x80:Ikbd.command_read_count=1;
      break;
    case 0x8: //return relative mouse position from now on
      Ikbd.mouse_mode=IKBD_MOUSE_MODE_RELATIVE;
      Ikbd.port_0_joy=0;
      ikbd_inc_hack(Ikbd.psyg_hack_stage,0);
      ikbd_inc_hack(Ikbd.reset_0814_hack,0);
      agenda_delete(ikbd_report_abs_mouse);
      break;
    case 0x9:Ikbd.command_read_count=4;
      break;
    case 0xa:case 0xb:case 0xc:case 0x21:case 0x22:Ikbd.command_read_count=2;
      break;
    case 0xd: //read absolute mouse position
      // This should be ignored if you aren't in absolute mode!
      if(Ikbd.mouse_mode!=IKBD_MOUSE_MODE_ABSOLUTE) 
        break;
      Ikbd.port_0_joy=0;
      // Ignore command if already calcing and sending packet
      if(agenda_get_queue_pos(ikbd_report_abs_mouse)>=0) 
        break;
      agenda_add(ikbd_report_abs_mouse,IKBD_SCANLINES_FROM_ABS_MOUSE_POLL_TO_SEND,-1);
      break;
    case 0xe:Ikbd.command_read_count=5;
      break;
    case 0xf: //mouse goes upside down
      Ikbd.port_0_joy=0;
      Ikbd.mouse_upside_down=1;
      break;
    case 0x10: //mouse goes right way up
      Ikbd.port_0_joy=Ikbd.mouse_upside_down=0;
      break;
    case 0x11: //okay to send!
      DBG_LOG("IKBD turned on");
      Ikbd.send_nothing=0;
      break;
    case 0x12: //turn mouse off
      DBG_LOG("IKBD: Mouse turned off");
      Ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;
      //Ikbd.port_0_joy=1;
      ikbd_inc_hack(Ikbd.reset_1214_hack,0);
      ikbd_inc_hack(Ikbd.reset_121A_hack,0);
      agenda_delete(ikbd_report_abs_mouse);
      break;
    case 0x13: //stop data transfer to main processor
      DBG_LOG("IKBD turned off");
      Ikbd.send_nothing=1;
      break;
    case 0x14: //return joystick movements
      DBG_LOG("IKBD: Changed joystick mode to change notification");
      Ikbd.port_0_joy=1;
      Ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;  //disable mouse
      agenda_delete(ikbd_report_abs_mouse);
      if(Ikbd.joy_mode!=IKBD_JOY_MODE_AUTO_NOTIFY) 
      {
        agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
        Ikbd.joy_mode=IKBD_JOY_MODE_AUTO_NOTIFY;
      }
      // In the IKBD this resets old_stick to 0
      for(int j=0;j<2;j++)
        if(stick[j])
          ikbd_send_joystick_message(j);
      ikbd_inc_hack(Ikbd.psyg_hack_stage,2);
      ikbd_inc_hack(Ikbd.reset_0814_hack,1);
      ikbd_inc_hack(Ikbd.reset_1214_hack,1);
      break;
    case 0x15: //don't return joystick movements
      DBG_LOG("IKBD: Joysticks set to only report when asked");
      Ikbd.port_0_joy=1;
      Ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;  //disable mouse
      agenda_delete(ikbd_report_abs_mouse);
      if(Ikbd.joy_mode!=IKBD_JOY_MODE_ASK) 
      {
        Ikbd.joy_mode=IKBD_JOY_MODE_ASK;
        agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
      }
      break;
    case 0x16: //read joystick
      if(Ikbd.joy_mode!=IKBD_JOY_MODE_OFF) 
      {
        // Ignore command if already calcing and sending packet
        if(agenda_get_queue_pos(ikbd_send_joystick_message)>=0)
          break;
        agenda_add(ikbd_send_joystick_message,IKBD_SCANLINES_FROM_JOY_POLL_TO_SEND,-1);
      }
      Ikbd.port_0_joy=1;
      break;
    case 0x18: //fire button duration, constant high speed joystick button test
      DBG_LOG("IKBD: Joysticks set to fire button duration mode!");
      Ikbd.joy_mode=IKBD_JOY_MODE_FIRE_BUTTON_DURATION;
      Ikbd.mouse_mode=IKBD_MOUSE_MODE_OFF;  //disable mouse
      agenda_delete(ikbd_report_abs_mouse);
      Ikbd.port_0_joy=1;
      agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
      break;
    case 0x1a: //turn off joysticks
      DBG_LOG("IKBD: Joysticks turned off");
      Ikbd.port_0_joy=0;
      Ikbd.joy_mode=IKBD_JOY_MODE_OFF;
      stick[0]=0;stick[1]=0;
      ikbd_inc_hack(Ikbd.reset_121A_hack,1);
      agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
      break;
    case 0x1b:case 0x19:Ikbd.command_read_count=6;
      break;
    case 0x1c: //read clock time
      keyboard_buffer_write(0xfc);
      if(OPTION_RTC_HACK)
        ikbd_set_clock_to_correct_time();
      for(int n=0;n<6;n++) 
      {
#if defined(SSE_IKBD_RTC)
        BYTE val=Ikbd.clock[n];
        if(OPTION_BATTERY6301==2&&(!n))
          val+=0xA0;
        keyboard_buffer_write(val);
#else
        keyboard_buffer_write(Ikbd.clock[n]);
#endif
      }
      break;
    case 0x20:Ikbd.command_read_count=3;
      break;
    case 0x87: //return what happens when mouse buttons are pressed
      keyboard_buffer_write_string(0xf6,0x7,
        Ikbd.mouse_button_press_what_message,0,0,0,0,0,(-1));
      break;
    case 0x88:case 0x89:case 0x8a:
      keyboard_buffer_write(0xf6);
      keyboard_buffer_write(BYTE(Ikbd.mouse_mode));
      if(Ikbd.mouse_mode==0x9) 
      {
        keyboard_buffer_write_string(HIBYTE(Ikbd.abs_mouse_max_x),
          LOBYTE(Ikbd.abs_mouse_max_x),HIBYTE(Ikbd.abs_mouse_max_y),
          LOBYTE(Ikbd.abs_mouse_max_y),0,0,(-1));
      }
      else if(Ikbd.mouse_mode==0xa) 
      {
        keyboard_buffer_write_string(Ikbd.cursor_key_mouse_pulse_count_x,
          Ikbd.cursor_key_mouse_pulse_count_y,
          0,0,0,0,(-1));
      }
      else
        keyboard_buffer_write_string(0,0,0,0,0,0,(-1));
      break;
    case 0x8b: //x, y threshhold for relative mouse movement messages
      keyboard_buffer_write_string(0xf6,0xb,Ikbd.relative_mouse_threshold_x,
        Ikbd.relative_mouse_threshold_y,0,0,0,0,(-1));
      break;
    case 0x8c: //x,y scaling of mouse for absolute mouse
      keyboard_buffer_write_string(0xf6,0xc,Ikbd.abs_mouse_scale_x,
        Ikbd.abs_mouse_scale_y,0,0,0,0,(-1));
      break;
    case 0x8f:case 0x90: //return 0xf if mouse is upside down, 0x10 otherwise
      keyboard_buffer_write(0xf6);
      if(Ikbd.mouse_upside_down)
        keyboard_buffer_write(0xf);
      else 
        keyboard_buffer_write(0x10);
      keyboard_buffer_write_string(0,0,0,0,0,0,(-1));
      break;
    case 0x92:  //is mouse off?
      keyboard_buffer_write(0xf6);
      if(Ikbd.mouse_mode==IKBD_MOUSE_MODE_OFF)
        keyboard_buffer_write(0x12);
      else
        keyboard_buffer_write(0);
      keyboard_buffer_write_string(0,0,0,0,0,0,(-1));
      break;
    case 0x94:case 0x95:case 0x99: {
      keyboard_buffer_write(0xf6);
      // if joysticks are disabled then return previous state. We don't store that.
      BYTE jmode=BYTE(Ikbd.joy_mode);
      if(jmode==IKBD_JOY_MODE_OFF) 
        jmode=IKBD_JOY_MODE_AUTO_NOTIFY;
      keyboard_buffer_write(jmode);
      if(Ikbd.joy_mode==0x19)
        for(int n=0;n<6;n++) 
          keyboard_buffer_write(BYTE(Ikbd.cursor_key_joy_time[n]));
      else
        keyboard_buffer_write_string(0,0,0,0,0,0,(-1));
      break;
    }
    case 0x9a:  //is joystick off?
      keyboard_buffer_write(0xf6);
      if(Ikbd.joy_mode==IKBD_JOY_MODE_OFF)
        keyboard_buffer_write(0x1a);
      else
        keyboard_buffer_write(0);
      keyboard_buffer_write_string(0,0,0,0,0,0,(-1));
      break;
      // > 0x9a all DEAD (tested up to 0xac)
    default:
#if defined(SSE_DEBUG)
      if(src) 
        TRACE_LOG("Byte ignored",src);
#endif
      break;
    }//sw
    Ikbd.command_parameter_counter=0;
  }
}


void agenda_keyboard_replace(int) {
  // bytes sent by the IKBD to the ACIA - also option C1
  DBG_LOG(EasyStr("IKBD: agenda_keyboard_replace at time=")+hbl_count
    +" with keyboard_buffer_length="+keyboard_buffer_length);
#if defined(SSE_HD6301_LL)
  if(OPTION_C1) 
  {
    if(keyboard_buffer_length) 
    {
      keyboard_buffer_length--;
#ifdef DEBUG_BUILD
      //ASSERT(mode==STEM_MODE_CPU);
      if(stem_runmode==STEM_MODE_CPU)
#endif
        hd6301_run_cycles(ACT); // run up to IO time
      acia[ACIA_IKBD].rdrs=Ikbd.tdrs;
      /*  Check overrun.
      "The Overrun does not occur in the Status Register until the valid character
      prior to Overrun has been read."
      => 1. SR is updated at next read of RDR
      2. The old byte will be read, the new byte is lost
      */
      if(acia[ACIA_IKBD].sr&BIT_0) // RDR full
      {
        TRACE_LOG("%d %d %d ACIA 0 OVR (RDRS %X)\n",TIMING_INFO,acia[ACIA_IKBD].rdrs);
        DBG_LOG("IKBD: Overrun on keyboard ACIA");
        if(acia[ACIA_IKBD].overrun!=ACIA_OVERRUN_YES) 
          acia[ACIA_IKBD].overrun=ACIA_OVERRUN_COMING; // keep original system
      }
      else // not overrun, OK
      {
        //TRACE_LOG("%d %d %d ACIA RDRS->RDR %X\n",TIMING_INFO,acia[ACIA_IKBD].rdrs);
        acia[ACIA_IKBD].rdr=acia[ACIA_IKBD].rdrs; // transfer shifted byte
        TRACE_LOG("%d %d %d ACIA RDR %X\n",TIMING_INFO,acia[ACIA_IKBD].rdr);
        acia[ACIA_IKBD].sr|=BIT_0; // set RDR full
      }
      // Check if we must activate IRQ (overrun or normal)
      ACIA_CHECK_IRQ(ACIA_IKBD);
    }//if(keyboard_buffer_length)
    acia[ACIA_IKBD].LineRxBusy=false;
    // schedule next event if characters are not coming from 6301
    if(keyboard_buffer_length)
    {
      Ikbd.tdrs=keyboard_buffer[keyboard_buffer_length-1];
      TRACE_LOG("fake IKBD TDRS %X\n",Ikbd.tdrs);
      acia[ACIA_IKBD].LineRxBusy=true;
      acia[ACIA_IKBD].time_of_event_incoming=ACT + acia[ACIA_IKBD].TransmissionTime();
      if(acia[ACIA_IKBD].time_of_event_incoming-time_of_event_acia<=0)
        time_of_event_acia=acia[ACIA_IKBD].time_of_event_incoming; 
    }
    if(macro_start_after_ikbd_read_count)
      macro_start_after_ikbd_read_count--;
    return;
  }
#endif
  if(keyboard_buffer_length)
  {
    if(!Ikbd.send_nothing) 
    {
      keyboard_buffer_length--;
      if(Ikbd.joy_packet_pos>=keyboard_buffer_length) 
        Ikbd.joy_packet_pos=-1;
      if(Ikbd.mouse_packet_pos>=keyboard_buffer_length) 
        Ikbd.mouse_packet_pos=-1;
      if(acia[ACIA_IKBD].rx_not_read) 
      {
        DBG_LOG("IKBD: Overrun on keyboard ACIA");
        // discard data and set overrun
        if(acia[ACIA_IKBD].overrun!=ACIA_OVERRUN_YES) 
          acia[ACIA_IKBD].overrun=ACIA_OVERRUN_COMING;
      }
      else 
      {
        acia[ACIA_IKBD].data=keyboard_buffer[keyboard_buffer_length]; 
        acia[ACIA_IKBD].rx_not_read=true;
      }
      if(acia[ACIA_IKBD].rx_irq_enabled) 
      {
        DBG_LOG(EasyStr("IKBD: Changing ACIA IRQ bit from ")+acia[ACIA_IKBD].irq+" to 1");
        acia[ACIA_IKBD].irq=true;
      }
      mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(acia[ACIA_IKBD].irq||acia[ACIA_MIDI].irq));
    }
    if(keyboard_buffer_length) 
      agenda_add(agenda_keyboard_replace,ACIAClockToHBLS(acia[ACIA_IKBD].clock_divide),0);
  }
  if(macro_start_after_ikbd_read_count) 
    macro_start_after_ikbd_read_count--;
}


void keyboard_buffer_write_n_record(BYTE src) {
  keyboard_buffer_write(src);
  if(macro_record) 
    macro_record_key(src);
}


void keyboard_buffer_write(BYTE src) {
/*  Function keyboard_buffer_write() receive bytes from the fake or the true
    6301 emu, or from shortcuts or macros, and sets then in an agenda or sets up
    an event to take care of the 6301->ACIA delay.
*/
#if defined(SSE_HD6301_LL)
  if(OPTION_C1)
  {
    if(!acia[ACIA_IKBD].LineRxBusy)
      Ikbd.tdrs=src;
    acia[ACIA_IKBD].LineRxBusy=true;
/*  Normally the buffer should be 2 bytes, one being shifted, one in the 
    register. In fake mode, it's much more.
    If this is first byte, we prepare an event to emulate the ACIA transmission
    delay.
    If there's already info in the buffer, an event was already prepared,
    a new event will be prepared when this one is executed. We only need to
    shift the buffer and insert the new value.
    For the moment we keep this system of keyboard buffer even for 6301 true
    emu because it is flexible.
*/
    if(keyboard_buffer_length<MAX_KEYBOARD_BUFFER_SIZE)
    {
      if(keyboard_buffer_length)
        memmove(keyboard_buffer+1,keyboard_buffer,keyboard_buffer_length); // shift
      else
      {
        TRACE_LOG("IKBD TDRS %X\n",src);
        acia[ACIA_IKBD].time_of_event_incoming
          =cpu_timer_at_start_of_hbl + cycles_run*HD6301_CYCLE_DIVISOR 
          + acia[ACIA_IKBD].TransmissionTime();
        if(acia[ACIA_IKBD].time_of_event_incoming-time_of_event_acia<=0)
          time_of_event_acia=acia[ACIA_IKBD].time_of_event_incoming;        
      }
      keyboard_buffer_length++;
      keyboard_buffer[0]=src;
      //TRACE_LOG("IKBD +$%X (%d)\n",src,keyboard_buffer_length);
    }
#ifdef SSE_DEBUG
    else
      TRACE_LOG("IKBD: Keyboard buffer overflow\n");
#endif
    return;
  }//C1
#endif
  if(keyboard_buffer_length<MAX_KEYBOARD_BUFFER_SIZE)
  {
    if(keyboard_buffer_length)
      memmove(keyboard_buffer+1,keyboard_buffer,keyboard_buffer_length);
    else
      // new chars in keyboard so time them out, +1 for middle of scanline
      agenda_add(agenda_keyboard_replace,ACIAClockToHBLS(acia[ACIA_IKBD].clock_divide)+1,0);
    keyboard_buffer_length++;
    keyboard_buffer[0]=src;
    DBG_LOG(EasyStr("IKBD: Wrote $")+HEXSl(src,2)+" keyboard buffer length="+keyboard_buffer_length);
    if(Ikbd.joy_packet_pos>=0) 
      Ikbd.joy_packet_pos++;
    if(Ikbd.mouse_packet_pos>=0) 
      Ikbd.mouse_packet_pos++;
  }
  else
    DBG_LOG("IKBD: Keyboard buffer overflow");
}


void keyboard_buffer_write_string(int s1,...) {
  va_list vl;
  int arg=s1;
  va_start(vl,s1);
  while(arg!=-1)
  {
    keyboard_buffer_write(LOBYTE(arg));
    arg=va_arg(vl,int);
  }
  va_end(vl);
}


void ikbd_mouse_move(int x,int y,int mouseb,int max_mouse_move) {
  DBG_LOG(EasyStr("Mouse moves ")+x+","+y);
#if defined(SSE_HD6301_LL)
  if(OPTION_C1)
  {
    Ikbd.MouseVblDeltaX=(short)x,Ikbd.MouseVblDeltaY=(short)y;
    return;
  }
#endif
  if(Ikbd.joy_mode<100||!Ikbd.port_0_joy) 
  {  //not in duration mode or joystick mode
    if(Ikbd.mouse_mode==IKBD_MOUSE_MODE_ABSOLUTE) 
    {
      // use the scale! eg Sentinel
      x*=Ikbd.abs_mouse_scale_x;
      y*=Ikbd.abs_mouse_scale_y;
      Ikbd.abs_mouse_x+=x;
      if(Ikbd.abs_mouse_x<0)
        Ikbd.abs_mouse_x=0;
      else if(Ikbd.abs_mouse_x>Ikbd.abs_mouse_max_x)
        Ikbd.abs_mouse_x=Ikbd.abs_mouse_max_x;
      Ikbd.abs_mouse_y+=y;
      if(Ikbd.abs_mouse_y<0)
        Ikbd.abs_mouse_y=0;
      else if(Ikbd.abs_mouse_y>Ikbd.abs_mouse_max_y)
        Ikbd.abs_mouse_y=Ikbd.abs_mouse_max_y;
    }
    else if(Ikbd.mouse_mode==IKBD_MOUSE_MODE_RELATIVE) 
    {
      int x1=0;int y1=0;
      while(abs_quick(x-x1)>max_mouse_move||abs_quick(y-y1)>max_mouse_move) 
      {
        int x2=MIN(max_mouse_move,MAX(-max_mouse_move,x-x1));
        int y2=MIN(max_mouse_move,MAX(-max_mouse_move,y-y1));
        keyboard_buffer_write(BYTE(0xf8+(mouseb & 3)));
        keyboard_buffer_write(LOBYTE(x2));
        keyboard_buffer_write(LOBYTE(y2));
        x1+=x2;
        y1+=y2;
      }
      keyboard_buffer_write(BYTE(0xf8+(mouseb & 3)));
      keyboard_buffer_write(LOBYTE(x-x1));
      keyboard_buffer_write(LOBYTE(y-y1));
    }
    else if(Ikbd.mouse_mode==IKBD_MOUSE_MODE_CURSOR_KEYS)
    {
      while(abs_quick(x)>Ikbd.cursor_key_mouse_pulse_count_x 
        || abs_quick(y)>Ikbd.cursor_key_mouse_pulse_count_y)
      {
        if(x>Ikbd.cursor_key_mouse_pulse_count_x)
        {
          keyboard_buffer_write(0x4d);
          keyboard_buffer_write(0x4d|MSB_B);
          x-=Ikbd.cursor_key_mouse_pulse_count_x;
        }
        else if(x<-Ikbd.cursor_key_mouse_pulse_count_x)
        {
          keyboard_buffer_write(0x4b);
          keyboard_buffer_write(0x4b|MSB_B);
          x+=Ikbd.cursor_key_mouse_pulse_count_x;
        }
        if(y>Ikbd.cursor_key_mouse_pulse_count_y)
        {
          keyboard_buffer_write(0x50);
          keyboard_buffer_write(0x50|MSB_B);
          y-=Ikbd.cursor_key_mouse_pulse_count_y;
        }
        else if(y<-Ikbd.cursor_key_mouse_pulse_count_y)
        {
          keyboard_buffer_write(0x48);
          keyboard_buffer_write(0x48|MSB_B);
          y+=Ikbd.cursor_key_mouse_pulse_count_y;
        }
      }
      if(mouseb&2)
        keyboard_buffer_write(0x74);
      else keyboard_buffer_write(0x74|MSB_B);
      if(mouseb&1)
        keyboard_buffer_write(0x75);
      else keyboard_buffer_write(0x75|MSB_B);
    }
  }
}


void ikbd_set_clock_to_correct_time() {
  time_t t=time(NULL);
  struct tm *lpTime=localtime(&t);
  Ikbd.clock[5]=BYTE((lpTime->tm_sec % 10) | ((lpTime->tm_sec/10) << 4));
  Ikbd.clock[4]=BYTE((lpTime->tm_min % 10) | ((lpTime->tm_min/10) << 4));
  Ikbd.clock[3]=BYTE((lpTime->tm_hour % 10) | ((lpTime->tm_hour/10) << 4));
  Ikbd.clock[2]=BYTE((lpTime->tm_mday % 10) | ((lpTime->tm_mday/10) << 4));
  int m=lpTime->tm_mon +1; //month is 0-based in C RTL
  Ikbd.clock[1]=BYTE((m % 10) | ((m/10) << 4));
  int y= (lpTime->tm_year);
  y %= 100;
//  lpTime->tm_year %= 100;
//  Ikbd.clock[0]=BYTE((lpTime->tm_year % 10) | ((lpTime->tm_year/10) << 4));
  Ikbd.clock[0]=BYTE((y % 10) | ((y/10) << 4));
  Ikbd.clock_vbl_count=0;
#if defined(SSE_HD6301_LL) && defined(SSE_IKBD_RTC)
  for(int i=0;i<6;i++)
    hd6301_poke(0x82+i,Ikbd.clock[i]);
#endif
}


void ikbd_reset(bool hardware_reset) {
  //ASSERT(Cold||!OPTION_C1);
  agenda_delete(agenda_keyboard_reset);
#if defined(SSE_HD6301_LL)
  if(OPTION_C1)
  {
    if(HD6301_OK) 
    {
      TRACE_LOG("6301 reset Ikbd.cpp part\n");
      Ikbd.mouse_upside_down=0;
      return;
    }
    else
      OPTION_C1=0; // and no return
  }
#endif
  if(hardware_reset)
  {
#if defined(SSE_IKBD_RTC)
    if(!OPTION_BATTERY6301)
      for(int n=0;n<6;Ikbd.clock[n++]=0); // ST without battery
    else
#endif
      ikbd_set_clock_to_correct_time();
    Ikbd.command_read_count=0;
    agenda_delete(agenda_keyboard_replace);
    keyboard_buffer_length=0;
    keyboard_buffer[0]=0;
    Ikbd.joy_packet_pos=-1;
    Ikbd.mouse_packet_pos=-1;
    agenda_keyboard_reset(0);
    ZeroMemory(ST_Key_Down,sizeof(ST_Key_Down));
  }
  else
  { // software reset: we've received a 0x80 0x01 command
    agenda_keyboard_reset(0);
    Ikbd.resetting=1;
    agenda_add(agenda_keyboard_reset,milliseconds_to_hbl(50),TRUE);
  }
}


void agenda_keyboard_reset(int SendF1) { // scheduled by ikbd_reset()
  TRACE_LOG("IKBD: TM " PRICV " Execute reset\n",ACT);
  if(SendF1==0)
  {
    Ikbd.mouse_button_press_what_message=0;
    Ikbd.mouse_mode=IKBD_MOUSE_MODE_RELATIVE;
    Ikbd.joy_mode=IKBD_JOY_MODE_AUTO_NOTIFY;
    Ikbd.cursor_key_mouse_pulse_count_x=3;
    Ikbd.cursor_key_mouse_pulse_count_y=3;
    Ikbd.relative_mouse_threshold_x=1;
    Ikbd.relative_mouse_threshold_y=1;
    Ikbd.abs_mouse_scale_x=1;
    Ikbd.abs_mouse_scale_y=1;
    Ikbd.abs_mouse_x=shifter_x/2;
    Ikbd.abs_mouse_y=shifter_y/2;
    Ikbd.abs_mouse_max_x=shifter_x;
    Ikbd.abs_mouse_max_y=shifter_y;
    Ikbd.mouse_upside_down=0;
    Ikbd.send_nothing=0;
    Ikbd.port_0_joy=0;
    Ikbd.abs_mousek_flags=0;
    Ikbd.psyg_hack_stage=0;
    Ikbd.reset_0814_hack=Ikbd.reset_1214_hack=Ikbd.reset_121A_hack=0;
    ZeroMemory(Ikbd.ram,sizeof(Ikbd.ram));
    agenda_delete(ikbd_send_joystick_message); // just in case sending other type of packet
    agenda_delete(ikbd_report_abs_mouse); // just in case sending other type of packet
    stick[0]=stick[1]=0;
  }
  else
  {
    DBG_LOG(EasyStr("IKBD: Finished reset at ")+hbl_count);
#if defined(SSE_IKBDI)
    if(OPTION_C1) // shouldn't we leave at once?
      Ikbd.ResetProgram();
     else
#endif
      keyboard_buffer_write(IKBD_RESET_MESSAGE); // 0xF1
    if(Ikbd.psyg_hack_stage==3||Ikbd.reset_0814_hack==2||Ikbd.reset_1214_hack==2) 
    {
      DBG_LOG("IKBD: HACK ACTIVATED - turning mouse on.");
      TRACE_LOG("IKBD: HACK ACTIVATED - turning mouse on\n"); // Barbarian
      Ikbd.mouse_mode=IKBD_MOUSE_MODE_RELATIVE;
      Ikbd.port_0_joy=0;
    }
    if(Ikbd.reset_121A_hack==2) 
    { // Turned both mouse and joystick off, but they should be on.
      DBG_LOG("IKBD: HACK ACTIVATED - turning mouse and joystick on.");
      TRACE_LOG("IKBD: HACK ACTIVATED - turning mouse and joystick on.\n");
      Ikbd.mouse_mode=IKBD_MOUSE_MODE_RELATIVE;
      Ikbd.joy_mode=IKBD_JOY_MODE_AUTO_NOTIFY;
      Ikbd.port_0_joy=0;
    }
    Ikbd.mouse_button_press_what_message=0; // Hack to fix No Second Prize
    Ikbd.send_nothing=0; // Fix Just Bugging (probably correct though)
    if(!OPTION_C1)
    {
      // Send break codes for "stuck" keys
      // The break code for each key is obtained by ORing 0x80 with the make code.
      for(int n=1;n<118;n++)
        if(ST_Key_Down[n]) 
          keyboard_buffer_write(BYTE(0x80|n));
    }
  }
  Ikbd.resetting=0;
}


void ikbd_report_abs_mouse(int abs_mousek_flags) {
  bool use_current_mousek=(abs_mousek_flags==-1);
  if(use_current_mousek) 
    abs_mousek_flags=Ikbd.abs_mousek_flags;
  if(Ikbd.mouse_packet_pos>=0) 
  {
    keyboard_buffer[Ikbd.mouse_packet_pos-1]|=LOBYTE(abs_mousek_flags); // Must |= this or could lose button presses
    keyboard_buffer[Ikbd.mouse_packet_pos-2]=HIBYTE(Ikbd.abs_mouse_x);
    keyboard_buffer[Ikbd.mouse_packet_pos-3]=LOBYTE(Ikbd.abs_mouse_x);
    keyboard_buffer[Ikbd.mouse_packet_pos-4]=HIBYTE(Ikbd.abs_mouse_y);
    keyboard_buffer[Ikbd.mouse_packet_pos-5]=LOBYTE(Ikbd.abs_mouse_y);
  }
  else 
  {
    // 0xF7 | absolute mouse position record
    keyboard_buffer_write_string(0xf7,LOBYTE(abs_mousek_flags),
      HIBYTE(Ikbd.abs_mouse_x),LOBYTE(Ikbd.abs_mouse_x),
      HIBYTE(Ikbd.abs_mouse_y),LOBYTE(Ikbd.abs_mouse_y),-1);
    Ikbd.mouse_packet_pos=5;
  }
  if(use_current_mousek) 
    Ikbd.abs_mousek_flags=0;
}


void ikbd_send_joystick_message(int jn) {
  BYTE s[2]={stick[0],stick[1]};
  // If mouse active then joystick never sends button down
  if(Ikbd.port_0_joy==0) 
    s[0]&=0x0f,s[1]&=0x0f;
  if(jn==-1) 
  { // requested packet
    if(Ikbd.joy_packet_pos>=0)
    {
      keyboard_buffer[Ikbd.joy_packet_pos-1]=s[0];
      keyboard_buffer[Ikbd.joy_packet_pos-2]=s[1];
    }
    else 
    {
      keyboard_buffer_write_string(0xfd,s[0],s[1],-1);
      Ikbd.joy_packet_pos=2; //0=stick 1, 1=stick 0, 2=header
    }
  }
  else
  {
    keyboard_buffer_write_string((BYTE)(0xfe + jn),s[jn],-1);
    DBG_LOG(EasyStr("IKBD: Notified joystick movement, stick[")+jn+"]="+s[jn]);
  }
}


#undef LOGSECTION





THD6301::THD6301() {
}


THD6301::~THD6301() {
#if defined(SSE_HD6301_LL)
  hd6301_destroy(); // calling the C 6301 function
#endif
}

#define LOGSECTION LOGSECTION_IKBD


#if defined(SSE_IKBDI)

void THD6301::InterpretCommand(BYTE ByteIn) {
/*  This interpreter should work for both high and low-level 6301 emulation.
    We know command codes & parameters, we report this info through trace.
    when the command is complete.
    4.0.0: used for RTC Y2K fix
*/
  // custom program running?
  if(CustomProgram==CUSTOM_PROGRAM_RUNNING) 
  {
  }
  // custom program (boot) loading?
  else if(CustomProgram==CUSTOM_PROGRAM_LOADING)
  {
    Parameter[CurrentParameter-1]--;
    if(!Parameter[CurrentParameter-1])
      CustomProgram=CUSTOM_PROGRAM_LOADED; // we don't try and ID the program
  }
  // new command?
  else if(CurrentCommand==-1 && ByteIn) 
  {
    switch(ByteIn) {
    case 0x80: // RESET
    case 0x07: // SET MOUSE BUTTON ACTION
    case 0x17: // SET JOYSTICK MONITORING
      nParameters=1;
      break;
    case 0x09: // SET ABSOLUTE MOUSE POSITIONING
      nParameters=4;
      break;
    case 0x22: // CONTROLLER EXECUTE
#if defined(SSE_STATS)
      Stats.mskSpecial|=Stats.IKBD_22;
#endif
    case 0x0A: // SET MOUSE KEYCODE MOUSE
    case 0x0B: // SET MOUSE THRESHOLD
    case 0x0C: // SET MOUSE SCALE
    case 0x21: // MEMORY READ
      nParameters=2;
      break;
    case 0x0E: // LOAD MOUSE POSITION
      nParameters=5;
      break;
    case 0x19: // SET JOYSTICK KEYCODE MODE
    case 0x1B: // TIME-OF-DAY CLOCK SET
      nParameters=6;
      break;
    case 0x20: // MEMORY LOAD
      nParameters=3;
      break;
    default:
      // 8 SET RELATIVE MOUSE POSITION REPORTING
      // D INTERROGATE MOUSE POSITION
      // F SET Y=0 AT BOTTOM
      // 10 SET Y=0 AT TOP
      // 11 RESUME
      // 12 DISABLE MOUSE
      // 13 PAUSE OUTPUT
      // 14 SET JOYSTICK EVENT REPORTING
      // 15 SET JOYSTICK INTERROGATION MODE
      // 16 JOYSTICK INTERROGATE
      // 18 SET FIRE BUTTON MONITORING
      // 1A DISABLE JOYSTICKS
      // 1C INTERROGATE TIME-OF-DAT CLOCK
      // 87... STATUS INQUIRIES
      nParameters=0;
    }
    CurrentCommand=ByteIn;
    CurrentParameter=0;
  }
  else if(nParameters>12) //snapshot trouble
    CurrentCommand=-1; 
  // taking parameters of a command?
  else if(CurrentCommand!=-1)
  {
    if(CurrentParameter<nParameters)
      Parameter[CurrentParameter++]=ByteIn;
  }
  else ; // could be junk?
  // report?
  if(CurrentCommand!=-1 && nParameters==CurrentParameter)
  {
#if defined(SSE_IKBDI)
#if defined(SSE_DEBUG)
    ReportCommand();
#endif
#endif
    // how to treat further bytes?
    switch(CurrentCommand) {
    case 0x20:
      TRACE_LOG("Loading %d bytes\n",Parameter[CurrentParameter-1]);
      CustomProgram=CUSTOM_PROGRAM_LOADING;
      break;
    case 0x22:
      //ASSERT( CustomProgram==CUSTOM_PROGRAM_LOADED );
      CustomProgram=CUSTOM_PROGRAM_RUNNING;
#if defined(SSE_DEBUGGER_FAKE_IO)
      if((DEBUGGER_CONTROL_MASK2&DEBUGGER_CONTROL_6301))
      {
        runstate=RUNSTATE_STOPPING;
        SET_WHY_STOP("6301 reprogramming");
      }
#endif
      break;
    }
    LastCommand=CurrentCommand;
    CurrentCommand=-1;
  }
}

#define LOGSECTION LOGSECTION_IKBD

#ifdef SSE_DEBUG

void THD6301::ReportCommand() {
//  ASSERT( CurrentCommand!=-1 );
#if defined(SSE_DEBUGGER_OSD_CONTROL)
  if(OSD_MASK1 & OSD_CONTROL_IKBD)
    TRACE_OSD("IKBD $%02X ",CurrentCommand); 
#endif
  // give command code
  TRACE_LOG("IKBDi $%02X ",CurrentCommand); //i for interpreter
  // spell out command (as in Atari manual)
  switch(CurrentCommand) {
    case 0x80: TRACE_LOG("RESET"); break;
    case 0x07: TRACE_LOG("SET MOUSE BUTTON ACTION"); break;
    case 0x17: TRACE_LOG("SET JOYSTICK MONITORING"); break;
    case 0x09: TRACE_LOG("SET ABSOLUTE MOUSE POSITIONING"); break;
    case 0x0A: TRACE_LOG("SET MOUSE KEYCODE MOUSE"); break;
    case 0x0B: TRACE_LOG("SET MOUSE THRESHOLD"); break;
    case 0x0C: TRACE_LOG("SET MOUSE SCALE"); break;
    case 0x21: TRACE_LOG("MEMORY READ"); break;
    case 0x22: TRACE_LOG("CONTROLLER EXECUTE"); break;
    case 0x0E: TRACE_LOG("LOAD MOUSE POSITION"); break;
    case 0x19: TRACE_LOG("SET JOYSTICK KEYCODE MODE"); break;
    case 0x1B: TRACE_LOG("TIME-OF-DAY CLOCK SET"); break;
    case 0x20: TRACE_LOG("MEMORY LOAD"); break;
    case 0x08: TRACE_LOG("SET RELATIVE MOUSE POSITION REPORTING"); break;
    case 0x0D: TRACE_LOG("INTERROGATE MOUSE POSITION"); break;
    case 0x0F: TRACE_LOG("SET Y=0 AT BOTTOM"); break;
    case 0x10: TRACE_LOG("SET Y=0 AT TOP"); break;
    case 0x11: TRACE_LOG("RESUME"); break;
    case 0x12: TRACE_LOG("DISABLE MOUSE"); break;
    case 0x13: TRACE_LOG("PAUSE OUTPUT"); break;
    case 0x14: TRACE_LOG("SET JOYSTICK EVENT REPORTING"); break;
    case 0x15: TRACE_LOG("SET JOYSTICK INTERROGATION MODE"); break;
    case 0x16: TRACE_LOG("JOYSTICK INTERROGATE"); break;
    case 0x18: TRACE_LOG("SET FIRE BUTTON MONITORING"); break;
    case 0x1A: TRACE_LOG("DISABLE JOYSTICKS"); break;
    case 0x1C: TRACE_LOG("INTERROGATE TIME-OF-DAT CLOCK"); break;
    case 0x87: TRACE_LOG("STATUS INQUIRY mouse button action"); break;
    case 0x88: TRACE_LOG("STATUS INQUIRY mouse mode"); break;
    case 0x8B: TRACE_LOG("STATUS INQUIRY mnouse threshold"); break;
    case 0x8C: TRACE_LOG("STATUS INQUIRY mouse scale"); break;
    case 0x8F: TRACE_LOG("STATUS INQUIRY mouse vertical coordinates"); break;
    case 0x90: TRACE_LOG("STATUS INQUIRY Y=0 at top"); break;
    case 0x92: TRACE_LOG("STATUS INQUIRY mouse enable/disable"); break;
    case 0x94: TRACE_LOG("STATUS INQUIRY joystick mode"); break;
    case 0x9A: TRACE_LOG("STATUS INQUIRY joystick enable/disable"); break;
    default:   TRACE_LOG("Unknown command %X",CurrentCommand);
  }
  // list parameters if any
  if(nParameters)
  {
    TRACE_LOG(" (");
    for(int i=0;i<nParameters;i++)
      TRACE_LOG("%d=$%X ",i,Parameter[i]); // v3.8
    TRACE_LOG(")");
  }
  TRACE_LOG("\n");
}

#endif

#endif//#if defined(SSE_IKBDI)

//#pragma warning(disable: 4701)//potentially uninitialized local variable 'fp' used


#if defined(SSE_IKBDI) 

void THD6301::ResetProgram() {
  TRACE_LOG("6301 Reset ST program\n");
  LastCommand=CurrentCommand=-1;
  CurrentParameter=0;
  nParameters=0;
}

#endif


#if defined(SSE_HD6301_LL)
/*  Note most useful 6301 emulation code is in 3rdparty folder '6301', in C.
    Object HD6301 is more for some support, of both true and fake IKBD
    emulation.
*/

void THD6301::Init() { // called in 'main'
  Initialised=Crashed=0;
  BYTE* pram=hd6301_init();
  int checksum=0;
  if(!pram)
    return;
  const int romlen=4096;
  int rombase=256; // miniram
  // load ROM, first look for the file, if it isn't there, use internal resource
  EasyStr romfile=RunDir+SLASH+SSE_PLUGIN_DIR1+SLASH+HD6301_ROM_FILENAME;
  FILE *fp=fopen(romfile.Text,"r+b");
  if(!fp)
  {
    romfile=RunDir+SLASH+SSE_PLUGIN_DIR2+SLASH+HD6301_ROM_FILENAME;
    fp=fopen(romfile.Text,"r+b");
  }
  if(!fp)
  {
    romfile=RunDir+SLASH+HD6301_ROM_FILENAME;
    fp=fopen(romfile.Text,"r+b");
  }
  if(fp!=NULL)
  {
    fread(pram+rombase,1,romlen,fp);
    for(size_t i=0;i<romlen;i++)
      checksum+=pram[rombase+i];
    //ASSERT(checksum==HD6301_ROM_CHECKSUM);
    TRACE2("Load %s checksum %X\n",romfile.Text,checksum);
    fclose(fp);
    Initialised=1;
  }
  else
  {
#if defined(SSE_FILES_IN_RC)
    HRSRC rc=FindResource(NULL,MAKEINTRESOURCE(IDR_HD6301),RT_RCDATA);
    ASSERT(rc);
    if(rc)
    {
      HGLOBAL hglob=LoadResource(NULL,rc);
      if(hglob)
      {
        size_t n=SizeofResource(NULL,rc);
        BYTE *pdata=(BYTE*)LockResource(hglob);
        memcpy(pram+rombase,pdata,n);
        Initialised=1;
      }
    }//if(rc)
#endif
  }
  HD6301_OK=Initialised;  
}

//#pragma warning (default: 4701)

void THD6301::ResetChip(int Cold) {
  TRACE_LOG("6301 Reset chip %d\n",Cold);
#if defined(SSE_IKBDI)
  CustomProgram=CUSTOM_PROGRAM_NONE;
  ResetProgram();
#endif
#if defined(SSE_HD6301_LL)
  if(HD6301_OK && OPTION_C1)
  {
    //Ikbd.Crashed=(BYTE)mousek; // for fun, but our emu is limited to monochip //that's enough "fun"
    hd6301_reset(Cold);
  }
#endif
  if(Cold)  // real cold
  {
    ZeroMemory(ST_Key_Down,sizeof(ST_Key_Down));
    keyboard_buffer_length=0;
  }
}


void THD6301::Vbl() { // this is called in run.cpp right after IKBD_VBL()
  hd6301_vbl_cycles=0;
  click_x=click_y=0;
  // the following avoids the mouse going backward at high speed, but it can
  // do it on a real ST too - it is very ad hoc, people may prefer the mouse
  // feel of Steem 3.2, this is the main reason why C1 is still optional
  BYTE max_pix_h=30;
  BYTE max_pix_v=30;
  switch(screen_res) {
  case 0:
    break;
  case 1:
    max_pix_h=35;
    max_pix_v=12;
    break;
  default:
    max_pix_h=20;
    max_pix_v=20;
    break;
  }
  if(MouseVblDeltaX>max_pix_h)
    MouseVblDeltaX=max_pix_h;
  else if(MouseVblDeltaX<-max_pix_h)
    MouseVblDeltaX=-max_pix_h;
  if(MouseVblDeltaY>max_pix_v)
    MouseVblDeltaY=max_pix_v;
  else if(MouseVblDeltaY<-max_pix_v)
    MouseVblDeltaY=-max_pix_v;
#ifdef SSE_DEBUG
  if(MouseVblDeltaX||MouseVblDeltaY)
    TRACE_LOG("F%d 6301 mouse move %d,%d\n",FRAME,MouseVblDeltaX,MouseVblDeltaY);
#endif
  // do some computing only once per frame
  //ASSERT(video_freq_at_start_of_vbl);
  if(!video_freq_at_start_of_vbl)
    return; // anti-crash
  int cycles_per_frame=HD6301_CLOCK/video_freq_at_start_of_vbl; //TODO
  MouseCyclesPerTickX=(MouseVblDeltaX)
    ? (cycles_per_frame/abs_quicki(MouseVblDeltaX)) : 0;
  MouseCyclesPerTickY=(MouseVblDeltaY)
    ? (cycles_per_frame/abs_quicki(MouseVblDeltaY)) : 0;
  MouseNextTickX=MouseNextTickY=ChipCycles; // at once if it moves
#ifdef SSE_DEBUG
  if(MouseVblDeltaX||MouseVblDeltaY)
    TRACE_LOG("ticks x %d y %d\n", MouseCyclesPerTickX,MouseCyclesPerTickY);
#endif
}

#endif//SSE_HD6301_LL
