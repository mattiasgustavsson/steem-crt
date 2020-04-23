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
FILE: glue.h
DESCRIPTION: Declarations for the high-level GLU (General Logic Unit, often
called Glue) emulation.
struct TGlue, which includes TScanline and TGlueStatus
---------------------------------------------------------------------------*/

#pragma once
#ifndef SSEGLUE_H
#define SSEGLUE_H

#include <run.h>

#define MEM_FIRST_WRITEABLE 8 // bus error if writing on 0-7!
#define MEM_IO_BASE 0xff8000
#define MEM_EXPANSION_CARTRIDGE 0xfa0000 // used in d2
#define MEM_START_OF_USER_AREA 0x800

void update_ipl(COUNTER_VAR when);
void HBLInterrupt();
void VBLInterrupt();

#pragma pack(push, 8)

struct TScanline {
  DWORD Tricks; // see mask description in display.h
  short StartCycle; // eg 56
  short EndCycle; // eg 376
  int Cycles; // eg 512  // int because of cpu multiplier
  BYTE Bytes; // eg 160
};


struct TGlueStatus { 
  // necessary because GetNextVideoEvent() can be called an arbitrary
  // number of times at the same timing
  bool scanline_done;
  bool vc_reload_done;
  bool vbl_done,vbi_done,hbi_done; // vbl = draw frame, vbi = interrupt
  bool timerb_start, timerb_end; // here by convenience
  // 2: draw a frame without OSD first, then 1
  // 1: set runstate to RUNSTATE_STOPPING at VBL
  BYTE stop_emu; // it's here because it's controlled in Glue.Vbl()
};


struct TGlue {
  // ENUM
  enum EGlue {FREQ_50,FREQ_60,FREQ_71,NFREQS,
    HBLANK_OFF=0,DE_OFF,HBLANK_ON,HSYNC_ON,HSYNC_ON1,HSYNC_ON2,HSYNC_OFF,
    HSYNC_OFF2,RELOAD_SDP,ENABLE_VBI,VERT_OVSCN_LIMIT,
    DE_ON,DE_ON_HSCROLL, // !!! must be DE_ON+1
    LINE_START_LIMIT,LINE_START_LIMIT_PLUS2,LINE_START_LIMIT_PLUS26,
    LINE_START_LIMIT_MINUS12,LINE_STOP_LIMIT,LINE_STOP_LIMIT_MINUS2,
    LINE_PLUS_44_R, LINE_PLUS_20A, LINE_PLUS_20B, LINE_PLUS_20C, LINE_PLUS_20D,
    LINE_PLUS_26A, LINE_PLUS_26B, LINE_PLUS_26C,
    MEDRES_OA, MEDRES_OB, MEDRES_OC,
    DESTAB_A, DESTAB_B, DESTAB0, STAB_A, STAB_B, STAB_C, STAB_D,
    RENDER_CYCLE, NTIMINGS};
  // FUNCTIONS
  TGlue();
  void AdaptScanlineValues(int CyclesIn); // on set sync of shift mode
  void CheckSideOverscan(); // left & right border effects
  void CheckVerticalOverscan(); // top & bottom borders
  void EndHBL();
  bool FetchingLine();
  void GetNextVideoEvent();
  void IncScanline();
  void Reset(bool Cold);
  void Restore();
  void SetShiftMode(BYTE NewRes);
  void SetSyncMode(BYTE NewSync);
  void Update();
  void Vbl();
  // for high level Shifter trick analysis
  void AddFreqChange(BYTE f);
  void AddShiftModeChange(BYTE mode);  
  int FreqChangeAtCycle(int cycle);
  int FreqAtCycle(int cycle);
  int ShiftModeAtCycle(int cycle);
  int ShiftModeChangeAtCycle(int cycle);
#ifdef SSE_DEBUG
  int NextFreqChange(int cycle,int value=-1);
  int PreviousFreqChange(int cycle);
#endif
  short NextShiftModeChange(int cycle,int value=-1);
  short NextChangeToHi(int cycle);
  short NextChangeToLo(int cycle); // Lo = not HI for GLU
  short PreviousChangeToHi(int cycle);
  short PreviousChangeToLo(int cycle); // Lo = not HI for GLU
  int NextShiftModeChangeIdx(int cycle);
  short PreviousShiftModeChange(int cycle);
  short CycleOfLastChangeToShiftMode(int value);
  // DATA
  COUNTER_VAR hbl_pending_time, vbl_pending_time;
  TEvent video_event;
  int TrickExecuted; //make sure that each trick will only be applied once
  // we need keep info for only 3 scanlines:
  TGlueStatus m_Status;
  BYTE m_ShiftMode; // both bits of shift mode are shadowed in GLU (ijor)
  BYTE m_SyncMode; // sync mode is a GLU register
  BYTE Freq[NFREQS];
  BYTE cycle_of_scanline_length_decision; 
  BYTE video_freq,previous_video_freq;
  bool de_v_on;
  bool vsync; // state of VSync line
  bool gamecart;
  bool hbl_pending, vbl_pending;
  bool hscroll;
  WORD DE_cycles[NFREQS];
  short nLines; // 313, 263, 501
  short de_start_line,de_end_line,VCount;
  TScanline PreviousScanline, CurrentScanline, NextScanline;
  short ScanlineTiming[NTIMINGS][NFREQS];
  MEM_ADDRESS cartbase,cartend;
};

#pragma pack(pop)

#endif//#ifndef SSEGLUE_H
