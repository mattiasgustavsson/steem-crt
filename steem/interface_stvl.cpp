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
FILE: interface_stvl.cpp
CONDITION: SSE_VID_STVL must be defined
DESCRIPTION: This file contains client code for STVL, a plugin for ST 
emulators that emulates the video logic of an STF or an STE at low level.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#if defined(SSE_VID_STVL)

#include <interface_stvl.h>
#include <emulator.h>
#include <options.h>
#include <run.h>
#include <draw.h>
#include <display.h>
#include <computer.h>
#include <debug_framereport.h>
#include <gui.h>
#include <translate.h>

HMODULE hStvl=NULL;
TStvl Stvl; // We own the full data set

DWORD(STVL_CALLCONV *video_logic_init)(TStvl *pStvl)=NULL;
void(STVL_CALLCONV *video_logic_reset)(TStvl *pStvl,bool Cold)=NULL;
void(STVL_CALLCONV *video_logic_stf_run)(TStvl *pStvl,int cycles)=NULL;
void(STVL_CALLCONV *video_logic_ste_run)(TStvl *pStvl,int cycles)=NULL;
void(STVL_CALLCONV *video_logic_update)(TStvl *pStvl)=NULL;
//void(STVL_CALLCONV *video_logic_update_pal)(TStvl *pStvl, int n, WORD pal)=NULL;

// STVL data includes variables for the ST bus, to spare cycles we
// directly use those instead of Steem's.
MEM_ADDRESS& abus=Stvl.abus;
DU16 &udbus=Stvl.udbus;
WORD& dbus=Stvl.udbus.d16;
BYTE& dbusl=Stvl.udbus.d8[LO];
BYTE& dbush=Stvl.udbus.d8[HI];

BYTE *draw_mem_line_ptr;
WORD render_vstart, render_vend;
WORD render_hstart;
WORD render_scanline_length;


WORD PeekWord(DWORD addr) {
  WORD video_word= (addr<mem_len) ? *(WORD*)(Mem_End_minus_2-addr) : dbus;
  return video_word;
}


void StvlInit() {
  hStvl=SteemLoadLibrary(VIDEO_LOGIC_DLL);
  if(hStvl)
  {
    VERIFY(video_logic_init=(DWORD (STVL_CALLCONV*)(TStvl*))
      GetProcAddress(hStvl,"STVL_init"));
    VERIFY(video_logic_reset=(void (STVL_CALLCONV*)(TStvl*,bool))
      GetProcAddress(hStvl,"STVL_reset"));
    VERIFY(video_logic_stf_run=(void (STVL_CALLCONV*)(TStvl*,int))
      GetProcAddress(hStvl,"STVL_stf_run"));
    VERIFY(video_logic_ste_run=(void (STVL_CALLCONV*)(TStvl*,int))
      GetProcAddress(hStvl,"STVL_ste_run"));
    VERIFY(video_logic_update=(void (STVL_CALLCONV*)(TStvl*))
      GetProcAddress(hStvl,"STVL_update"));
    //video_logic_update_pal = (void(STVL_CALLCONV*)(TStvl*,int,WORD))GetProcAddress(hStvl, ""STVL_update_pal");
   // TRACE2("STVL %d %d %d %d %d\n",video_logic_init,video_logic_reset,video_logic_stf_run,video_logic_ste_run,video_logic_update);
    DWORD version=video_logic_init(&Stvl);
    char tmp[40];
    sprintf(tmp,"%s v%x",VIDEO_LOGIC_DLL,version);
    TRACE_INIT("%s loaded\n",tmp);
    SSEConfig.Stvl=(version&0xFFFF);
#ifndef SSE_X64
#ifdef SSE_STDCALL // 1st version returned nothing, so just alert if suspicious
    if(!(version&0x80000000)) // bit set if dll expects __stdcall
#else
    if(version&0x80000000)
#endif
    {
      Alert(tmp,T("Warning"),MB_OK|MB_ICONWARNING);
      SSEConfig.Stvl=TRUE; // else it could be garbage eg $800
    }
    else
#endif
      SSEConfig.Stvl=(version&0xFFFF);
    if(SSEConfig.Stvl>0x0100) // there was no version variable before v101!
      Stvl.version=SSEConfig.Stvl; // the plugin wants it to be the same as its internal version
    if(!SSEConfig.Stvl)
    {
      FreeLibrary(hStvl);
      hStvl=NULL;
    }
    Stvl.cbHInt=event_scanline; // hbl pending + scanline routines
    Stvl.cbVInt=event_trigger_vbi; // vbl pending + frame routines
    Stvl.cbDe=event_timer_b;
    Stvl.cbPeekWord=PeekWord; 
#if defined(SSE_DEBUGGER_FRAME_REPORT)
    Stvl.cbTraceVideoEvent=FrameEvents_Add;
#endif
#if defined(SSE_VID_STVL_SREQ)
    if(SSEConfig.Stvl>=0x0101)
      Stvl.cbFetchSound=Mmu.sound_fetch;
#endif
    // comment off to test dll rendering (check: doesn't react to colour control)
    Stvl.pPCpal=PCpal;
    Stvl.cbSetPal=&TShifter::SetPal; // must be static now, no big trouble
  }
  else // if snapshot later loaded with STVL enabled
    ZeroMemory(&Stvl,sizeof(Stvl));
}


void StvlUpdate() {
  //TRACE_OSD("%d",Stvl.framefreq);
  Stvl.VideoOut=COLOUR_MONITOR+1;
  Stvl.mono_col=(STpal[0]&1); // extra register in Shifter
  // timer B tick event
  if(Mfp.reg[MFPR_AER]&8)
  {
    Stvl.ctrDeOn=MFP_TIMER_B_DELAY_STVL;
    Stvl.ctrDeOff=0;
  }
  else // general case
  {
    Stvl.ctrDeOn=0;
    Stvl.ctrDeOff=MFP_TIMER_B_DELAY_STVL;
  }
#if defined(SSE_HIRES_COLOUR) // no, poc-only
  if(COLOUR_MONITOR && HIRES_COLOUR)
  {
    render_hstart=150; // border 32
    render_vstart=(DISPLAY_SIZE==3)?(35-(BIG_BORDER_TOP-ORIGINAL_BORDER_TOP)):35;
    if(Stvl.framefreq==60||Glue.m_ShiftMode!=mode)
      render_vstart-=(63-34);
    render_vend=render_vstart+BORDER_TOP+BottomBorderSize+200;
    if(!DISPLAY_SIZE)
      render_hstart+=32*4;
    else if(DISPLAY_SIZE>=2)
      render_hstart-=(52-32)*4;
   }
  else
#endif  
#if defined(SSE_HARDWARE_OVERSCAN)
  if(SSEConfig.OverscanOn)
  {
    Stvl.hwoverscan=OPTION_HWOVERSCAN;
    if(COLOUR_MONITOR)
    {
      switch(OPTION_HWOVERSCAN) {
      case LACESCAN:
        render_hstart=210;
        render_vstart=(DISPLAY_SIZE==3)?(25-(BIG_BORDER_TOP-ORIGINAL_BORDER_TOP)):25;
        render_vend=500-3;
        if(Stvl.framefreq==60)
          render_vstart-=(63-34),render_vend-=(63-34);
        render_vend=render_vstart+BORDER_TOP+BottomBorderSize+200;
        if(DISPLAY_SIZE>=2)
          render_hstart-=(52-32)*4;
        break;
      case AUTOSWITCH:
        render_hstart=210;
        render_vstart=(DISPLAY_SIZE==3)?(33-(BIG_BORDER_TOP-ORIGINAL_BORDER_TOP)):33;
        render_vstart-=6+2+1;
        if(Stvl.framefreq==60)
          render_vstart-=(63-34);
        render_vend=render_vstart+BORDER_TOP+BottomBorderSize+200;
        if(DISPLAY_SIZE>=2)
          render_hstart-=(52-32)*4;
        break;
      }
    }
    else // monochrome
    {
      switch(OPTION_HWOVERSCAN) {
      case LACESCAN:
        render_vstart=(DISPLAY_SIZE==3)?(35-(BIG_BORDER_TOP-ORIGINAL_BORDER_TOP)):35;
        // afraid of horrible complication if trying to do it a blit time
        if(Stvl.framefreq==MONO_HZ)
          render_vstart-=(63-36);
        render_vend=500;
        render_hstart=106+68; // hides trash
        break;
      case AUTOSWITCH: // GLU wakeup 2 advised for monochrome but we don't enforce...
        render_vstart=(DISPLAY_SIZE==3)?(35-(BIG_BORDER_TOP-ORIGINAL_BORDER_TOP)):35;
        // afraid of horrible complication if trying to do it a blit time
        if(Stvl.framefreq==MONO_HZ)
          render_vstart-=(63-36);
        render_vend=500; //-35?
        render_hstart=110+60;
        break;
      }//sw
    }
  }
  else
#endif//hwo
  if(COLOUR_MONITOR) // standard colour
  {
    render_hstart=236; // border 32
    render_vstart=33;
    if(Stvl.framefreq==60)
    {
      render_hstart-=16;
      render_vstart-=(63-34);
    }
    render_vend=render_vstart+BORDER_TOP+BottomBorderSize+200;
    if(!DISPLAY_SIZE)
    {
      short offset=Mmu.DL[OPTION_WS]*4-20; // must see no WS shift
      render_hstart+=32*4+offset;
      render_vstart=(Stvl.framefreq==50)?63:34;
      render_vend=render_vstart+200;
    }
    else if(DISPLAY_SIZE>=2)
      render_hstart-=(52-32)*4;
    render_hstart+=(OPTION_SHIFTER_WU-1)* (Shifter.m_ShiftMode==1?2:4);
  }
  else // standard monochrome
  {
    render_vstart=28;
    // afraid of horrible complication if trying to do it a blit time
    if(Stvl.framefreq==MONO_HZ)
      render_vstart-=(63-36);
    render_vend=500;
    render_hstart=164+16+2;
    if(!DISPLAY_SIZE)
    {
      short offset=Mmu.DL[OPTION_WS]*8-20-16-4-12;
      render_hstart+=32*4+offset;
      render_vstart=33+2; //?
      if(IS_STE) //? todo
        render_vstart+=1;
      render_vend=render_vstart+400;
    }
    if(OPTION_UNSTABLE_SHIFTER)
      render_hstart+=2+(OPTION_SHIFTER_WU-1)*2;
  }
  render_scanline_length=(640+(border!=0)*SideBorderSizeWin*2*2)*4; // =precomp
  render_hstart*=2;
  Stvl.st_model=(IS_STE+1); 
  Stvl.wakestate=(IS_STE)?1:Mmu.WS[OPTION_WS];
  Stvl.shifter_wakeup=OPTION_SHIFTER_WU-1;
  Stvl.UnstableShifter=(OPTION_UNSTABLE_SHIFTER!=0);
  if(hStvl)
    video_logic_update(&Stvl);
}


/*  We use CPU timings to drive the video logic emulation, this way we can be 
    cycle accurate without rewriting the whole emulation.
    It's a downside of our method that many timing functions are necessary
    (the rest of this file).
    This is in part because we also provide functions for an accelerated ST, even
    if precise video logic emu makes little sense then.
    The load is extraordinary. An older PC (Pentium D?) can't run it.
*/

#define CHECK_BLIT_REQUEST\
  if(Blitter.Request)\
    Blitter_CheckRequest()
#define BLIT_CYCLES Blitter.BlitCycles
#define BLIT_BUS_ACCESS_CTR Blitter.BusAccessCounter
#define THINKING_CYCLES Cpu.BusIdleCycles


// 1 low-level video, no acceleration
void InstructionTimeStf1Idle(int t) {
  BUS_MASK=0;
  video_logic_stf_run(&Stvl,t*4); // call STVL before changing cycles
  cpu_cycles-=(t);
}


void InstructionTimeSte1Idle(int t) {
  BUS_MASK=0;
  while(BLIT_CYCLES>t && t>0)
  {
    BLIT_CYCLES--;
    t--;
  }
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=(t);
  THINKING_CYCLES+=t;
  CHECK_BLIT_REQUEST;
}


#if defined(SSE_MEGASTE)

void InstructionTimeMegaSte1Idle(int t) {
  BUS_MASK=0;
  while(BLIT_CYCLES>t && t>0)
  {
    BLIT_CYCLES--;
    t--;
  }
  THINKING_CYCLES+=t;
  if(MegaSte.MemCache.ScuReg&1)
    t/=2;
  video_logic_ste_run(&Stvl,t*4); // Mega STE uses STE video logic
  cpu_cycles-=(t);
  CHECK_BLIT_REQUEST;
}

#endif


void InstructionTimeMegaSt1Idle(int t) {
  BUS_MASK=0;
  while(BLIT_CYCLES>t && t>0)
  {
    BLIT_CYCLES--;
    t--;
  }
  video_logic_stf_run(&Stvl,t*4); // Mega ST uses STF video logic
  cpu_cycles-=(t);
  THINKING_CYCLES+=t;
  CHECK_BLIT_REQUEST;
}


void InstructionTimeStf1WS(int t) {
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=(t);
}


void InstructionTimeSte1WS(int t) {
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=(t);
}


#if defined(SSE_MEGASTE)

void InstructionTimeMegaSte1WS(int t) { // same
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=(t);
}

#endif


void InstructionTimeMegaSt1WS(int t) {
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=(t);
}


void InstructionTimeStf1PrefetchOnly() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  int t=4;
  abus=(pc&0xfffffe);
  // rounding is computed before the call to STVL and applied after
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  IRC=m68k_fetch(pc);
}


void InstructionTimeStf1PrefetchFinal() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  IR=IRC;
  MEM_ADDRESS au=pc+2;
  int t=4;
  abus=(au&0xfffffe);
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  IRC=m68k_fetch(au);
}


void InstructionTimeStf1PrefetchTotal() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  pc+=2;
  int t=4;
  abus=(pc&0xfffffe);
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  IRC=m68k_fetch(pc);
}


void InstructionTimeSte1PrefetchOnly() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  int t=4;
  abus=(pc&0xfffffe);
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(pc);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSte1PrefetchFinal() {
  int t=4;
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  IR=IRC;
  MEM_ADDRESS au=pc+2;
  abus=(au&0xfffffe);
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(au);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSte1PrefetchTotal() {
  int t=4;
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  pc+=2;
  abus=(pc&0xfffffe);
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(pc);
  CHECK_BLIT_REQUEST;
}


#if defined(SSE_MEGASTE)

void InstructionTimeMegaSte1PrefetchOnly() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  int t=4;
  abus=(pc&0xfffffe);
  bool cached=MegaSte.MemCache.Check(abus,IRC);
  if(!cached && abus<himem)
    t+=(cpu_cycles&3);
  else if(cached)
    t=2;
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(pc);
  if(!cached)
    MegaSte.MemCache.Add(abus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSte1PrefetchFinal() {
  int t=4;
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  IR=IRC;
  MEM_ADDRESS au=pc+2;
  abus=(au&0xfffffe);
  bool cached=MegaSte.MemCache.Check(abus,IRC);
  if(!cached && abus<himem)
    t+=(cpu_cycles&3);
  else if(cached)
    t=2;
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(au);
  if(!cached)
    MegaSte.MemCache.Add(abus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSte1PrefetchTotal() {
  int t=4;
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  pc+=2;
  abus=(pc&0xfffffe);
  bool cached=MegaSte.MemCache.Check(abus,IRC);
  if(!cached && abus<himem)
    t+=(cpu_cycles&3);
  else if(cached)
    t=2;
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(pc);
  if(!cached)
    MegaSte.MemCache.Add(abus);
  CHECK_BLIT_REQUEST;
}

#endif


void InstructionTimeMegaSt1PrefetchOnly() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  int t=4;
  abus=(pc&0xfffffe);
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(pc);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSt1PrefetchFinal() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  IR=IRC;
  MEM_ADDRESS au=pc+2;
  int t=4;
  abus=(au&0xfffffe);
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(au);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSt1PrefetchTotal() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  pc+=2;
  int t=4;
  abus=(pc&0xfffffe);
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(pc);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeStf1ReadB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  if(abus<himem && abus>8)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  m68k_peek(iabus);
}


void InstructionTimeSte1ReadB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  if(abus<himem && abus>8)
    t+=(cpu_cycles&3);
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_peek(iabus);
  CHECK_BLIT_REQUEST;
}


#if defined(SSE_MEGASTE)

void InstructionTimeMegaSte1ReadB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  bool cached=MegaSte.MemCache.Check(abus,dbus);
  if(!cached && abus<himem && abus>8)
    t+=(cpu_cycles&3);
  else if(cached)
    t=2;
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_peek(iabus);
  if(!cached)
    MegaSte.MemCache.Add(abus);
  CHECK_BLIT_REQUEST;
}

#endif


void InstructionTimeMegaSt1ReadB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  if(abus<himem && abus>8)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_peek(iabus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeStf1Read() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  if(abus<himem && abus>8)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  dbus=m68k_dpeek(iabus);
}


void InstructionTimeSte1Read() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  if(abus<himem && abus>8)
    t+=(cpu_cycles&3);
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  dbus=m68k_dpeek(iabus);
  CHECK_BLIT_REQUEST;
}


#if defined(SSE_MEGASTE)

void InstructionTimeMegaSte1Read() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  bool cached=MegaSte.MemCache.Check(abus,dbus);
  if(!cached && abus<himem && abus>8)
    t+=(cpu_cycles&3);
  else if(cached)
    t=2;
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  dbus=m68k_dpeek(iabus);
  if(!cached)
    MegaSte.MemCache.Add(abus);
  CHECK_BLIT_REQUEST;
}

#endif


void InstructionTimeMegaSt1Read() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  if(abus<himem && abus>8)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  dbus=m68k_dpeek(iabus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSte1BltRead() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  int t=4;
  if(abus<himem && abus>8)
    t+=(cpu_cycles&3);
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
}


#if defined(SSE_MEGASTE)

void InstructionTimeMegaSte1BltRead() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  int t=4;
  bool cached=MegaSte.MemCache.Check(abus,dbus);
  if(!cached && abus<himem && abus>8)
    t+=(cpu_cycles&3);
  else if(cached)
    t=2;
  video_logic_ste_run(&Stvl,t*4);
  if(abus<himem)
    MegaSte.MemCache.Add(abus);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
}

#endif


void InstructionTimeMegaSt1BltRead() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  int t=4;
  if(abus<himem && abus>8)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
}


void InstructionTimeStf1Write() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  m68k_dpoke_abus(dbus);
}


void InstructionTimeSte1Write() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_dpoke_abus(dbus);
  CHECK_BLIT_REQUEST;
}


#if defined(SSE_MEGASTE)

void InstructionTimeMegaSte1Write() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_dpoke_abus(dbus);
  MegaSte.MemCache.Add(abus);
  CHECK_BLIT_REQUEST;
}

#endif


void InstructionTimeMegaSt1Write() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_dpoke_abus(dbus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeStf1WriteB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  dbush=dbusl; // STVL expects doubled bytes
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  m68k_poke_abus(dbusl);
}


void InstructionTimeSte1WriteB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  dbush=dbusl;
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_poke_abus(dbusl);
  CHECK_BLIT_REQUEST;
}


#if defined(SSE_MEGASTE)

void InstructionTimeMegaSte1WriteB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  dbush=dbusl;
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_poke_abus(dbusl);
  MegaSte.MemCache.Add(abus);
  CHECK_BLIT_REQUEST;
}

#endif


void InstructionTimeMegaSt1WriteB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  dbush=dbusl;
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_poke_abus(dbusl);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSte1BltWrite() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  int t=4;
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_ste_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
}


#if defined(SSE_MEGASTE)

void InstructionTimeMegaSte1BltWrite() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  int t=4;
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_ste_run(&Stvl,t*4);
  MegaSte.MemCache.Add(abus);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
}

#endif


void InstructionTimeMegaSt1BltWrite() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  int t=4;
  if(abus<himem)
    t+=(cpu_cycles&3);
  video_logic_stf_run(&Stvl,t*4);
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
}


// 2 low-level video, acceleration
// we call the video logic when enough cycles have been accumulated
// we don't waste time with rounding

COUNTER_VAR acc_cycles=0;

void InstructionTimeStf2Idle(int t) {
  if(t>0)
  {
    BUS_MASK=0;
    acc_cycles+=t;
    if(acc_cycles>=cpu_cycles_multiplier*12)
    {
      acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
      video_logic_stf_run(&Stvl,12*4);
    }
  }
  cpu_cycles-=(t);
}


void InstructionTimeSte2Idle(int t) {
  if(t>0)
  {
    BUS_MASK=0;
    acc_cycles+=t;
    if(acc_cycles>=cpu_cycles_multiplier*12)
    {
      acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
      video_logic_ste_run(&Stvl,12*4);
    }
  }
  while(BLIT_CYCLES>t && t>0)
  {
    BLIT_CYCLES--;
    t--;
  }
  cpu_cycles-=(t);
  THINKING_CYCLES+=t;
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSt2Idle(int t) {
  if(t>0)
  {
    BUS_MASK=0;
    acc_cycles+=t;
    if(acc_cycles>=cpu_cycles_multiplier*12)
    {
      acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
      video_logic_stf_run(&Stvl,12*4);
    }
  }
  while(BLIT_CYCLES>t && t>0)
  {
    BLIT_CYCLES--;
    t--;
  }
  cpu_cycles-=(t);
  THINKING_CYCLES+=t;
  CHECK_BLIT_REQUEST;
}


void InstructionTimeStf2WS(int t) {
  if(t>0)
  {
    acc_cycles+=t;
    if(acc_cycles>=cpu_cycles_multiplier*12)
    {
      acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
      video_logic_stf_run(&Stvl,12*4);
    }
  }
  cpu_cycles-=(t);
}


void InstructionTimeSte2WS(int t) {
  if(t>0)
  {
    acc_cycles+=t;
    if(acc_cycles>=cpu_cycles_multiplier*12)
    {
      acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
      video_logic_ste_run(&Stvl,12*4);
    }
  }
  cpu_cycles-=(t);
}


void InstructionTimeMegaSt2WS(int t) {
  if(t>0)
  {
    acc_cycles+=t;
    if(acc_cycles>=cpu_cycles_multiplier*12)
    {
      acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
      video_logic_stf_run(&Stvl,12*4);
    }
  }
  cpu_cycles-=(t);
}


void InstructionTimeStf2PrefetchOnly() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  int t=4;
  abus=(pc&0xfffffe);
  // we don't round up in accelerated mode
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  IRC=m68k_fetch(pc);
}


void InstructionTimeStf2PrefetchFinal() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  IR=IRC;
  MEM_ADDRESS au=pc+2;
  int t=4;
  abus=(au&0xfffffe);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  IRC=m68k_fetch(au);
}


void InstructionTimeStf2PrefetchTotal() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  pc+=2;
  int t=4;
  abus=(pc&0xfffffe);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  IRC=m68k_fetch(pc);
}


void InstructionTimeSte2PrefetchOnly() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  int t=4;
  abus=(pc&0xfffffe);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_ste_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  IRC=m68k_fetch(pc);
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSte2PrefetchFinal() {
  int t=4;
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  IR=IRC;
  MEM_ADDRESS au=pc+2;
  abus=(au&0xfffffe);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_ste_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(au);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSte2PrefetchTotal() {
  int t=4;
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  pc+=2;
  abus=(pc&0xfffffe);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_ste_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(pc);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSt2PrefetchOnly() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  int t=4;
  abus=(pc&0xfffffe);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(pc);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSt2PrefetchFinal() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  IR=IRC;
  MEM_ADDRESS au=pc+2;
  int t=4;
  abus=(au&0xfffffe);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(au);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSt2PrefetchTotal() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  pc+=2;
  int t=4;
  abus=(pc&0xfffffe);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  IRC=m68k_fetch(pc);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeStf2ReadB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  m68k_peek(iabus);
}


void InstructionTimeSte2ReadB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_ste_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_peek(iabus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSt2ReadB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_peek(iabus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeStf2Read() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  dbus=m68k_dpeek(iabus);
}


void InstructionTimeSte2Read() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_ste_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  dbus=m68k_dpeek(iabus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSt2Read() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  dbus=m68k_dpeek(iabus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSte2BltRead() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  int t=4;
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_ste_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
}



void InstructionTimeMegaSt2BltRead() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  int t=4;
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
}


void InstructionTimeStf2Write() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  m68k_dpoke_abus(dbus);
}


void InstructionTimeSte2Write() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_ste_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_dpoke_abus(dbus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSt2Write() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  int t=4;
  abus=(iabus&0x00FFFFFE);
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_dpoke_abus(dbus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeStf2WriteB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  dbush=dbusl;
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  m68k_poke_abus(dbusl);
}


void InstructionTimeSte2WriteB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  dbush=dbusl;
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_ste_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_poke_abus(dbusl);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSt2WriteB() {
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_HIBYTE);
  int t=4;
  abus=(iabus&0x00FFFFFE);
  dbush=dbusl;
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  m68k_poke_abus(dbusl);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSte2BltWrite() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  int t=4;
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_ste_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
}


void InstructionTimeMegaSt2BltWrite() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  int t=4;
  acc_cycles+=t;
  if(acc_cycles>=cpu_cycles_multiplier*12)
  {
    acc_cycles-=(COUNTER_VAR)(cpu_cycles_multiplier*12);
    video_logic_stf_run(&Stvl,12*4);
  }
  cpu_cycles-=t;
  BLIT_BUS_ACCESS_CTR++;
}

#endif//#if defined(SSE_VID_STVL)
