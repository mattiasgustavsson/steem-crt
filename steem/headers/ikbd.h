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
FILE: ikbd.h
DESCRIPTION: Declarations for the keyboard (IKBD) emulation.
struct THD6301
This file is used by 6301.c.
---------------------------------------------------------------------------*/

#pragma once
#ifndef SSE6301_H
#define SSE6301_H

#ifdef __cplusplus 
#include "steemh.h"
#include <conditions.h>
#include <parameters.h>
extern "C" {
#include "../../3rdparty/6301/6301.h"
}

#define IKBD_HBLS_FROM_COMMAND_WRITE_TO_PROCESS 5
#define IKBD_DEFAULT_MOUSE_MOVE_MAX 15
#define IKBD_RESET_MESSAGE 0xf1
#define MAX_KEYBOARD_BUFFER_SIZE 1024
#define IKBD_MOUSE_MODE_ABSOLUTE 0x9
#define IKBD_MOUSE_MODE_RELATIVE 0x8
#define IKBD_MOUSE_MODE_CURSOR_KEYS 0xa
#define IKBD_MOUSE_MODE_OFF 0x12
#define IKBD_JOY_MODE_OFF 0x1a
#define IKBD_JOY_MODE_ASK 0x15
#define IKBD_JOY_MODE_AUTO_NOTIFY 0x14
#define IKBD_JOY_MODE_CURSOR_KEYS 0x19
#define IKBD_JOY_MODE_DURATION 100
#define IKBD_JOY_MODE_FIRE_BUTTON_DURATION 101
#define BIT_RMB BIT_0
#define BIT_LMB BIT_1
#define IKBD_SCANLINES_FROM_ABS_MOUSE_POLL_TO_SEND int((MONO) ? 50:30)
#define IKBD_SCANLINES_FROM_JOY_POLL_TO_SEND int((MONO) ? 32:20)   // 32:20
#define RMB_DOWN(mk) (mk & 1)
#define LMB_DOWN(mk) (mk & 2)
#define STKEY_PAD_DIVIDE 0x65
#define STKEY_PAD_ENTER 0x72


extern "C" BYTE ST_Key_Down[128]; // used by 6301
extern "C" int mousek; 
extern int disable_input_vbl_count;
extern int ikbd_joy_poll_line;
extern BYTE keyboard_buffer[MAX_KEYBOARD_BUFFER_SIZE];
extern WORD keyboard_buffer_length; // 0-MAX_KEYBOARD_BUFFER_SIZE-1
extern BYTE mouse_speed; // 1-19
extern int mouse_move_since_last_interrupt_x,mouse_move_since_last_interrupt_y;
extern bool mouse_change_since_last_interrupt;
extern const int ikbd_clock_days_in_mon[13];
extern const int ikbd_clock_max_val[6];
extern BYTE no_set_cursor_pos;
#ifdef WIN32
// When task switching is turned off we have to manually update these keys at the VBL
extern BYTE TaskSwitchVKList[4];
extern bool CutTaskSwitchVKDown[4];
#endif
extern "C" void keyboard_buffer_write(BYTE); // called by the 6301 emu
extern "C" void keyboard_buffer_write_n_record(BYTE);
void agenda_keyboard_replace(int);
void keyboard_buffer_write_string(int s1,...);
bool ikbd_keys_disabled();
void ikbd_mouse_move(int,int,int,int =(IKBD_DEFAULT_MOUSE_MOVE_MAX));
void IKBD_VBL();
void agenda_ikbd_process(int);  //intelligent keyboard handle byte
void ikbd_run_start(bool);
void ikbd_reset(bool);
void agenda_keyboard_reset(int);
void ikbd_report_abs_mouse(int);
void ikbd_send_joystick_message(int);
void ikbd_set_clock_to_correct_time();
#endif//#ifdef __cplusplus 

#pragma pack(push, 8)

struct THD6301 {
  // from former ikbd struct so it is compatible with v3.2, unfortunately
  // maybe not with intermediate versions
  // BYTE instead of bool for C compatibility
  int command_read_count,command_parameter_counter;
  BYTE command;
  BYTE command_param[8];
  BYTE mouse_button_press_what_message;
  int mouse_mode;
  int joy_mode;
  int abs_mouse_max_x,abs_mouse_max_y;
  int cursor_key_mouse_pulse_count_x,cursor_key_mouse_pulse_count_y;
  int relative_mouse_threshold_x,relative_mouse_threshold_y;
  int abs_mouse_scale_x,abs_mouse_scale_y;
  int abs_mouse_x,abs_mouse_y;
  BYTE mouse_upside_down;
  BYTE send_nothing;
  int duration;
  BYTE clock[6];
  DWORD cursor_key_joy_time[6];
  DWORD cursor_key_joy_ticks[4];
  BYTE port_0_joy;
  int abs_mousek_flags;
  BYTE resetting;
  int psyg_hack_stage;
  int clock_vbl_count;
  WORD load_memory_address;
  BYTE ram[128]; 
  int reset_121A_hack,reset_0814_hack,reset_1214_hack;
  int joy_packet_pos,mouse_packet_pos;
  // end former ikbd struct
  COUNTER_VAR ChipCycles,MouseNextTickX,MouseNextTickY;
  COUNTER_VAR hd6301_vbl_cycles,time_of_tdr_to_tdrs;
  COUNTER_VAR current_m68_cycle; // will lag or be forward
  int MouseCyclesPerTickX, MouseCyclesPerTickY;
  short MouseVblDeltaX; // must keep separate for true emu
  short MouseVblDeltaY;
  BYTE Initialised; // we do need a rom
  BYTE Crashed; // oops
  BYTE click_x,click_y; // current click
  // lower case because uppercase are constants in 6301 emu itself
  BYTE rdr,rdrs,tdr,tdrs; 
#if defined(SSE_IKBDI) 
  BYTE LastCommand;
  BYTE CurrentParameter; //0-5
  BYTE nParameters; //0-6
  BYTE Parameter[6]; // max 6
  BYTE CustomProgram;
  char CurrentCommand;
#endif
  enum EProgramTag {
    CUSTOM_PROGRAM_NONE,
    CUSTOM_PROGRAM_LOADING,
    CUSTOM_PROGRAM_LOADED,
    CUSTOM_PROGRAM_RUNNING
  }custom_program_tag;
  //DATA 
  //FUNCTIONS
#ifdef __cplusplus //isolate member functions, for C it's just POD
  THD6301();
  ~THD6301();
  void ResetChip(int Cold);
  void ResetProgram();
  void Init();
  void Vbl();
#if defined(SSE_IKBDI) 
  void InterpretCommand(BYTE ByteIn);
#if defined(SSE_DEBUG)
  void ReportCommand();
#endif
#endif
#endif//c++?
};

#pragma pack(pop)

extern
#ifdef __cplusplus
"C"
#endif
struct THD6301 Ikbd; // 6301.c must know about it

#endif//#ifndef SSE6301_H
