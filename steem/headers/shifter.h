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
FILE: shifter.h
DESCRIPTION: Declarations for the high-level Shifter emulation.
struct TShifter
---------------------------------------------------------------------------*/

#pragma once
#ifndef SSESHIFTER_H
#define SSESHIFTER_H

#include "conditions.h"
#include "steemh.h"
#include "parameters.h"
#include "sound.h"
#include "options.h"


// in all shift modes, all 4 Shifter registers are loaded before picture starts:
#define SHIFTER_RASTER_PREFETCH_TIMING 16
#define SHIFT_SDP(nbytes) shifter_draw_pointer+=(DWORD)nbytes

// ID which part of emulation required video rendering (only a few used now)
enum EShifterDispatchers {DISPATCHER_NONE, DISPATCHER_CPU, DISPATCHER_LINEWIDTH,
  DISPATCHER_WRITE_SDP, DISPATCHER_SET_SHIFT_MODE, DISPATCHER_SET_SYNC,
  DISPATCHER_SET_PAL, DISPATCHER_DSTE};


extern "C" // used by assembly routines => no struct
{
  extern WORD STpal[16]; // the full ST palette is here
  extern MEM_ADDRESS shifter_draw_pointer; // ST pointer to video RAM to be rendered
  extern BYTE shifter_hscroll; // STE-only horizontal scroll
}

#define HSCROLL shifter_hscroll

extern short shifter_pixel;
extern short shifter_x,shifter_y;
extern BYTE shifter_sound_mode; // should be in struct but old memory snapshot issues

#pragma pack(push, 8)

struct TShifter {
  //FUNCTIONS
  TShifter();
  inline void DrawBufferedScanlineToVideo();
  void DrawScanlineToEnd();
  void IncScanline();
  void Render(short cycles_since_hbl, int dispatcher=DISPATCHER_NONE);
  void Reset(bool Cold);
  void Restore();
  void RoundCycles(short &cycles_in);
  static void SetPal(int n, WORD NewPal); // static for STVL
  void sound_get_last_sample(WORD *pw1,WORD *pw2); // v402
  void sound_set_mode(BYTE new_mode);
  void sound_play();
  //DATA
  BYTE *ScanlineBuffer; // unused
  DWORD Scanline[230/4+2];
  int nVbl; // = FRAME for debugger
  BYTE Scanline2[112];
  BYTE HiresRaster; // for a hack
  BYTE HblStartingHscroll; // saving true hscroll in MED RES (no use)
  BYTE m_ShiftMode;
  BYTE Preload; // #words into Shifter's IR (gross approximation)
  char WakeupShift;
  char HblPixelShift; // for 4bit scrolling, other shifts
  WORD sound_fifo[4]; // ref. 4.0.2
  BYTE sound_fifo_idx;
};


#pragma pack(pop)


// just taking some unimportant code out of Render for clarity
#define   AUTO_BORDER_ADJUST  \
          if(!(border)) { \
            if(scanline_drawn_so_far<BORDER_SIDE) { \
              border1-=(BORDER_SIDE-scanline_drawn_so_far); \
              if(border1<0){ \
                picture+=border1; \
                if(!screen_res) {  \
                  hscroll-=border1;  \
                  shifter_draw_pointer+=(hscroll/16)*8; \
                  hscroll&=15; \
                }else if(screen_res==1) { \
                  hscroll-=border1*2;  \
                  shifter_draw_pointer+=(hscroll/16)*4; \
                  hscroll&=15; \
                } \
                border1=0; \
                if(picture<0) picture=0; \
              } \
            } \
            int ta=(border1+picture+border2)-320; \
            if(ta>0) { \
              border2-=ta; \
              if(border2<0)  { \
                picture+=border2; \
                border2=0; \
                if (picture<0)  picture=0; \
              } \
            } \
            border1=border2=0; \
          }


////////////////////////////////////////////
// Digital Sound (GST Shifter of the STE) //
////////////////////////////////////////////


extern WORD ste_sound_last_word,ste_sound_freq;
#define CPU_CYCLES_PER_MW_SHIFT 8
// Max frequency/lowest refresh *2 for stereo
extern WORD *ste_sound_channel_buf;
extern DWORD ste_sound_channel_buf_len;
#define STE_SOUND_BUFFER_LENGTH (ste_sound_channel_buf_len)
extern DWORD ste_sound_channel_buf_idx;
extern const WORD ste_sound_mode_to_freq[4];
extern int ste_sound_output_countdown,ste_sound_samples_countdown;

#endif//define SSESHIFTER_H
