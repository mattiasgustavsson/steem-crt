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
FILE: draw.h
DESCRIPTION: Declarations for draw routines.
---------------------------------------------------------------------------*/

#pragma once
#ifndef DRAW_DECLA_H
#define DRAW_DECLA_H

#include "conditions.h"

enum EDraw {DWM_STRETCH=0,DWM_NOSTRETCH,DWM_GRILLE,
            SCANLINES_ABOVE_SCREEN_50HZ=63,SCANLINES_ABOVE_SCREEN_60HZ=34,
            SCANLINES_ABOVE_SCREEN_70HZ=34};

bool draw_routines_init();
void init_screen();
void draw_begin();
void draw_end();
bool draw_blit();
void draw_set_jumps_and_source();
void draw(bool);
HRESULT change_fullscreen_display_mode(bool resizeclippingwindow);
void res_change();
void draw_double_lines(); // win32?
//ASMCALL is __cdecl
void ASMCALL draw_scanline_dont(int,int,int,int);
extern "C" 
{
  long* ASMCALL Get_PCpal();
  void ASMCALL draw_scanline_8_lowres_pixelwise(int,int,int,int);
  void ASMCALL draw_scanline_16_lowres_pixelwise(int,int,int,int);
  void ASMCALL draw_scanline_24_lowres_pixelwise(int,int,int,int);
  void ASMCALL draw_scanline_32_lowres_pixelwise(int,int,int,int);
  void ASMCALL draw_scanline_8_lowres_pixelwise_dw(int,int,int,int);
  void ASMCALL draw_scanline_16_lowres_pixelwise_dw(int,int,int,int);
  void ASMCALL draw_scanline_24_lowres_pixelwise_dw(int,int,int,int);
  void ASMCALL draw_scanline_32_lowres_pixelwise_dw(int,int,int,int);
  void ASMCALL draw_scanline_8_lowres_pixelwise_400(int,int,int,int);
  void ASMCALL draw_scanline_16_lowres_pixelwise_400(int,int,int,int);
  void ASMCALL draw_scanline_24_lowres_pixelwise_400(int,int,int,int);
  void ASMCALL draw_scanline_32_lowres_pixelwise_400(int,int,int,int);
  void ASMCALL draw_scanline_8_medres_pixelwise(int,int,int,int);
  void ASMCALL draw_scanline_16_medres_pixelwise(int,int,int,int);
  void ASMCALL draw_scanline_24_medres_pixelwise(int,int,int,int);
  void ASMCALL draw_scanline_32_medres_pixelwise(int,int,int,int);
  void ASMCALL draw_scanline_8_medres_pixelwise_400(int,int,int,int);
  void ASMCALL draw_scanline_16_medres_pixelwise_400(int,int,int,int);
  void ASMCALL draw_scanline_24_medres_pixelwise_400(int,int,int,int);
  void ASMCALL draw_scanline_32_medres_pixelwise_400(int,int,int,int);
  void ASMCALL draw_scanline_8_hires(int,int,int,int);
  void ASMCALL draw_scanline_16_hires(int,int,int,int);
  void ASMCALL draw_scanline_24_hires(int,int,int,int);
  void ASMCALL draw_scanline_32_hires(int,int,int,int);
}

typedef void ASMCALL PIXELWISESCANPROC(int,int,int,int);
typedef PIXELWISESCANPROC* LPPIXELWISESCANPROC;
#if defined(SSE_VID_32BIT_ONLY)
extern LPPIXELWISESCANPROC jump_draw_scanline[3][3];
#else
extern LPPIXELWISESCANPROC jump_draw_scanline[3][4][3];
#endif
extern LPPIXELWISESCANPROC draw_scanline,draw_scanline_lowres,draw_scanline_medres;

extern "C" // used in assembly routines
{
  //extern BYTE FullScreen;
  extern BYTE *draw_mem;
  extern int draw_line_length;
  extern long *PCpal;
  extern BYTE *draw_dest_ad,*draw_dest_next_scanline;
}

extern RECT draw_blit_source_rect;
extern int draw_dest_increase_y;
extern int draw_win_mode[2];
extern short res_vertical_scale;
//calculated from BORDER_TOP, BORDER_BOTTOM and res_vertical_scale
extern short draw_first_scanline_for_border,draw_last_scanline_for_border; 
extern short draw_first_scanline_for_border60,draw_last_scanline_for_border60; 
extern short draw_first_possible_line,draw_last_possible_line;
extern char overscan;
extern bool draw_lock;
extern bool draw_med_low_double_height;
extern BYTE bad_drawing;
extern BYTE draw_grille_black;
extern BYTE border,border_last_chosen; //0: no border 1: normal 2: large 3: max
extern int scanline_drawn_so_far;
extern int left_border,right_border;
// for Shifter trick analysis
extern COUNTER_VAR shifter_freq_change_time[32];
extern BYTE shifter_freq_change[32];
extern BYTE shifter_freq_change_idx;
extern COUNTER_VAR shifter_mode_change_time[32];
extern BYTE shifter_mode_change[32];
extern BYTE shifter_mode_change_idx;
extern bool draw_line_off;
extern bool freq_change_this_scanline;

#if defined(SSE_VID_2SCREENS)
void get_fullscreen_totalrect(RECT* rc);
#endif

#ifdef WIN32

// This is for the new scanline buffering (v2.6). If you write a lot direct
// to video memory it can be very slow due to recasching, so if the surface is
// in vid mem we set draw_buffer_complex_scanlines. This means that in
// draw_scanline_to we change draw_dest_ad to draw_temp_line_buf and
// set draw_scanline to draw_scanline_1_line. In draw_scanline_to_end
// we then copy from draw_temp_line_buf to the old draw_dest_ad and
// restore draw_scanline.
#define DRAW_TEMP_LINE_BUF_LEN (sizeof(int)*1024)
extern BYTE draw_temp_line_buf[DRAW_TEMP_LINE_BUF_LEN]; // overkill
extern BYTE* draw_temp_line_buf_lim;
extern BYTE *draw_store_dest_ad;
extern LPPIXELWISESCANPROC draw_scanline_1_line[2],draw_store_draw_scanline;
extern bool draw_buffer_complex_scanlines;

#if defined(SSE_VID_DD)
extern void get_fullscreen_rect(RECT *);
#if !defined(SSE_VID_DD_MISC)
extern HWND ClipWin; 
#endif
#endif

#endif//WIN32

#ifdef UNIX
extern int x_draw_surround_count;
#endif

#if defined(SSE_VID_D3D)
extern const BYTE draw_fs_fx;
#else // Linux too
extern BYTE draw_fs_fx;
extern BYTE draw_fs_blit_mode;
extern bool prefer_res_640_400,using_res_640_400;
#endif

#endif//DRAW_DECLA_H
