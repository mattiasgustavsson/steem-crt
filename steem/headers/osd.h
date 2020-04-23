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

DOMAIN: Rendering
FILE: osd.h
DESCRIPTION: Declarations for OSD (On Screen Display).
struct TOsdControl
---------------------------------------------------------------------------*/

#pragma once
#ifndef OSD_DECLA_H
#define OSD_DECLA_H

#include <easystr.h>
#include <easystringlist.h>
#include <conditions.h>
#include <parameters.h>

#define OSD_ICON_SIZE 24
#define OSD_LOGO_W 124
#define OSD_LOGO_H 11
#if 0
#define CHANCE_OF_SCROLLER 1 //for tests
#else
#define CHANCE_OF_SCROLLER 8
#endif
#define OSD_SHOW_ALWAYS 0xff

extern bool osd_no_draw,osd_disable;
extern BYTE osd_show_plasma,osd_show_speed,osd_show_icons,osd_show_cpu;
extern bool osd_old_pos;
extern bool osd_show_disk_light,osd_show_scrollers;
extern DWORD osd_start_time,osd_scroller_start_time,osd_scroller_finish_time;
extern bool osd_shown_scroller;
extern long col_yellow[2],col_blue,col_red,col_green,col_white;
extern DWORD FDCCantWriteDisplayTimer;
extern long col_fd_red[2],col_fd_green[2];
extern DWORD HDDisplayTimer;

void osd_draw_begin();
void osd_init_run(bool);
void osd_draw();
void osd_hide();
//void osd_start_scroller(char*);
void osd_draw_full_stop();
void osd_draw_end();
void osd_routines_init();
void osd_init_draw_static();
bool osd_is_on(bool);

#ifdef WIN32
void osd_draw_reset_info(HDC);
LRESULT CALLBACK ResetInfoWndProc(HWND,UINT,WPARAM,LPARAM);
extern HWND ResetInfoWin;
#else
void osd_draw_reset_info(int,int,int,int);
#endif

UNIX_ONLY( extern "C" long* Get_charset_blk(); )

void osd_pick_scroller();
extern EasyStr get_osd_scroller_text(int n);
extern EasyStr osd_scroller;
extern EasyStringList osd_scroller_array;

extern DWORD *osd_plasma_pal;
extern BYTE *osd_plasma;

void osd_draw_plasma(int,int,int);
void ASMCALL osd_blueize_line_8(int,int,int);
void ASMCALL osd_blueize_line_dont(int,int,int);
void ASMCALL osd_draw_char_dont(long*,BYTE*,long,long,int,long,long);
void ASMCALL osd_draw_char_clipped_dont(long*,BYTE*,long,long,int,long,long,RECT*);

extern long *osd_font;

typedef void ASMCALL OSDDRAWCHARPROC(long*,BYTE*,long,long,int,long,long);
typedef void ASMCALL OSDDRAWCHARCLIPPEDPROC (long*,BYTE*,long,long,int,long,long,RECT*);
typedef void ASMCALL OSDBLUEIZELINEPROC (int,int,int);
typedef void ASMCALL OSDBLACKRECTPROC (void*,int,int,int,int,long);
typedef OSDDRAWCHARPROC* LPOSDDRAWCHARPROC;
typedef OSDDRAWCHARCLIPPEDPROC* LPOSDDRAWCHARCLIPPEDPROC;
typedef OSDBLUEIZELINEPROC* LPOSDBLUEIZELINEPROC;
typedef OSDBLACKRECTPROC* LPOSDBLACKRECTPROC;

extern LPOSDBLUEIZELINEPROC osd_blueize_line;
extern LPOSDDRAWCHARPROC jump_osd_draw_char[4],osd_draw_char,
  jump_osd_draw_char_transparent[4],osd_draw_char_transparent;
extern LPOSDDRAWCHARCLIPPEDPROC jump_osd_draw_char_clipped[4],osd_draw_char_clipped,
  jump_osd_draw_char_clipped_transparent[4],osd_draw_char_clipped_transparent;
extern LPOSDBLACKRECTPROC jump_osd_black_box[4],osd_black_box;

extern "C"{
#if !defined(SSE_VID_32BIT_ONLY)
void ASMCALL osd_draw_char_clipped_8(long*,BYTE*,long,long,int,long,long,RECT*);
void ASMCALL osd_draw_char_clipped_16(long*,BYTE*,long,long,int,long,long,RECT*);
void ASMCALL osd_draw_char_clipped_24(long*,BYTE*,long,long,int,long,long,RECT*);
#endif
void ASMCALL osd_draw_char_clipped_32(long*,BYTE*,long,long,int,long,long,RECT*);
#if !defined(SSE_VID_32BIT_ONLY)
void ASMCALL osd_draw_char_8(long*,BYTE*,long,long,int,long,long);
void ASMCALL osd_draw_char_16(long*,BYTE*,long,long,int,long,long);
void ASMCALL osd_draw_char_24(long*,BYTE*,long,long,int,long,long);
#endif
void ASMCALL osd_draw_char_32(long*,BYTE*,long,long,int,long,long);
#if !defined(SSE_VID_32BIT_ONLY)
void ASMCALL osd_draw_char_transparent_8(long*,BYTE*,long,long,int,long,long);
void ASMCALL osd_draw_char_transparent_16(long*,BYTE*,long,long,int,long,long);
void ASMCALL osd_draw_char_transparent_24(long*,BYTE*,long,long,int,long,long);
#endif
void ASMCALL osd_draw_char_transparent_32(long*,BYTE*,long,long,int,long,long);
#if !defined(SSE_VID_32BIT_ONLY)
void ASMCALL osd_draw_char_clipped_transparent_8(long*,BYTE*,long,long,int,long,long,RECT*);
void ASMCALL osd_draw_char_clipped_transparent_16(long*,BYTE*,long,long,int,long,long,RECT*);
void ASMCALL osd_draw_char_clipped_transparent_24(long*,BYTE*,long,long,int,long,long,RECT*);
#endif
void ASMCALL osd_draw_char_clipped_transparent_32(long*,BYTE*,long,long,int,long,long,RECT*);
#if !defined(SSE_VID_32BIT_ONLY)
void ASMCALL osd_blueize_line_16_555(int,int,int),ASMCALL osd_blueize_line_16_565(int,int,int);
void ASMCALL osd_blueize_line_24(int,int,int);
#endif
void ASMCALL osd_blueize_line_32(int,int,int);
#if !defined(SSE_VID_32BIT_ONLY)
void palette_convert_16_555(int),palette_convert_line_16_565(int);
void palette_convert_line_24(int);
#endif
void palette_convert_line_32(int);
#if !defined(SSE_VID_32BIT_ONLY)
void ASMCALL osd_black_box_8(void*,int,int,int,int,long);
void ASMCALL osd_black_box_16(void*,int,int,int,int,long);
void ASMCALL osd_black_box_24(void*,int,int,int,int,long);
#endif
void ASMCALL osd_black_box_32(void*,int,int,int,int,long);
}


#pragma pack(push, 8)

struct TOsdControl {
  enum EOsdControl {NO_SCROLLER,WANT_SCROLLER,SCROLLING};
  TOsdControl() { ScrollerPhase=NO_SCROLLER; };
  //void StartScroller(EasyStr text);
  void Trace(char *fmt,...); // yellow message in top left
#if defined(SSE_OSD_SHOW_TIME)
  DWORD StartingTime; // record time on cold reset
  DWORD StoppingTime; // to adjust when stopping/restarting
#endif
  DWORD MessageTimer;
  long ScrollerPosition;
  //long ScrollerColour;
  BYTE ScrollerPhase;
  EasyStr ScrollText;
  char m_OsdMessage[OSD_MESSAGE_LENGTH+1]; // +null as usual
};

extern TOsdControl OsdControl;

#pragma pack(pop)

#define TRACE_OSD2 OsdControl.Trace

#endif//OSD_DECLA_H
