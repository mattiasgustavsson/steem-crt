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
FILE: cpu.cpp
DESCRIPTION: Core definitions for the Motorola MC68000 emulation: process,
timings, exceptions, e-clock.
m68kProcess() is the central function.
Opcodes are emulated in cpu_op.cpp, effective address is dealt with
in cpu_ea.cpp.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop
#include <debug_framereport.h>
#include <gui.h>
#include <reset.h>
#include <hd_gemdos.h>
#include <computer.h>
#include <interface_stvl.h>
#include <osd.h>
#include <cpu_op.h>

/// pointers to functions
BYTE (*m68k_peek)(MEM_ADDRESS ad);
WORD (*m68k_dpeek)(MEM_ADDRESS ad);
WORD (*m68k_fetch)(MEM_ADDRESS ad); // = dpeek for fetching
void (*pInstructionTimeIdle)(int t);
void (*pInstructionTimeWS)(int t); 
void (*pInstructionTimePrefetchOnly)();
void (*pInstructionTimePrefetchFinal)();
void (*pInstructionTimePrefetchTotal)();
void (*pInstructionTimeReadB)();
void (*pInstructionTimeRead)();
void (*pInstructionTimeWrite)();
void (*pInstructionTimeWriteB)();
void (*pInstructionTimePush)();
void (*pInstructionTimeBltRead)(); // blitter
void (*pInstructionTimeBltWrite)();


#if defined(SSE_VC_INTRINSICS)
int (*count_bits_set_in_word)(unsigned short); // pointer to function
int count_bits_set_in_word2(unsigned short w) { // fast function if available
  return __popcnt16(w);
}
#endif

DWORD CpuNormalHz=CPU_CLOCK_STF_PAL;
#if defined(SSE_CPU_MFP_RATIO_OPTION)
DWORD CpuCustomHz=CPU_CLOCK_STF_PAL;
#endif

WORD &SR=Cpu.sr; // Status Register
WORD &IRC=Cpu.irc; // Instruction Register Capture
WORD &IR=Cpu.ir;  // Instruction Register
WORD &IRD=Cpu.ird;  // Instruction Register Decode
DUS32 *cpureg=(DUS32*)&Cpu.r; // all registers
DU32 *cpuareg=(DU32*)&Cpu.r[8]; // address registers
// hardware does it differently, it's easier so in emulation (we swap)
MEM_ADDRESS &sp=(MEM_ADDRESS&)Cpu.r[15];
MEM_ADDRESS &other_sp=(MEM_ADDRESS&)Cpu.r[16];
MEM_ADDRESS &pc=Cpu.upc.d32; // address of next operand
WORD &pch=Cpu.upc.d16[HI],&pcl=Cpu.upc.d16[LO];
DU32 uiabus; // AOB
MEM_ADDRESS &iabus=uiabus.d32;
WORD &iabush=uiabus.d16[HI];
WORD &iabusl=uiabus.d16[LO];
DU32 ueffective_address;
DWORD &effective_address=ueffective_address.d32;
WORD &effective_address_h=ueffective_address.d16[HI];
WORD &effective_address_l=ueffective_address.d16[LO];
DU32 um68k_old_dest;
DWORD &m68k_old_dest=um68k_old_dest.d32;
WORD &m68k_old_dest_h=um68k_old_dest.d16[HI];
WORD &m68k_old_dest_l=um68k_old_dest.d16[LO];
WORD m68k_ap,m68k_iriwo;
short m68k_src_w;
DUS32 sm68k_src_l;
long &m68k_src_l=sm68k_src_l.d32;
short &m68k_src_lh=sm68k_src_l.d16[HI];
short &m68k_src_ll=sm68k_src_l.d16[LO];
char m68k_dst_b;
short m68k_dst_w;
DUS32 sm68k_dst_l;
long &m68k_dst_l=sm68k_dst_l.d32;
short &m68k_dst_lh=sm68k_dst_l.d16[HI];
short &m68k_dst_ll=sm68k_dst_l.d16[LO];
char m68k_src_b;
long save_r[16];
WORD ry,rx;

signed int compare_buffer;
COUNTER_VAR act; // to record Absolute CPU Time
DUS32 uresult;
long &result=uresult.d32; //32bit internal register 
short &resulth=uresult.d16[HI];
short &resultl=uresult.d16[LO];
signed char &resultb=uresult.d8[LO];

UFlags uflags;
#ifdef BIG_ENDIAN_PROCESSOR
BYTE &pswT=uflags.d8[0];
BYTE &pswS=uflags.d8[1];
BYTE &pswI=uflags.d8[2];
BYTE &pswX=uflags.d8[3];
BYTE &pswN=uflags.d8[4];
BYTE &pswZ=uflags.d8[5];
BYTE &pswV=uflags.d8[6];
BYTE &pswC=uflags.d8[7];
#else
BYTE &pswT=uflags.d8[7];
BYTE &pswS=uflags.d8[6];
BYTE &pswI=uflags.d8[5];
BYTE &pswX=uflags.d8[4];
BYTE &pswN=uflags.d8[3];
BYTE &pswZ=uflags.d8[2];
BYTE &pswV=uflags.d8[1];
BYTE &pswC=uflags.d8[0];
#endif

#ifdef ENABLE_LOGFILE
MEM_ADDRESS debug_mem_write_log_address;
int debug_mem_write_log_bytes;
#endif

extern const char* exception_action_name[4];
m68k_exception ExceptionObject;
jmp_buf *pJmpBuf=NULL;

TIplTiming ipl_timing[256];

BYTE ipl_timing_index=0;

#define LOGSECTION LOGSECTION_CPU


void m68k_trap1() { 
  // on the MC68000, illegal instructions including illegal addressing mode
  // decode to the trap1 exception microcode
  // older CPUs tried to execute them, with undocumented effects
  exception(BOMBS_ILLEGAL_INSTRUCTION,EA_INST,0);
}


void update_sr_from_flags() {
  SR=(pswT<<15)|(pswS<<13)|(pswI<<8)|(pswX<<4)|(pswN<<3)|(pswZ<<2)
    |(pswV<<1)|pswC;
}


void update_flags_from_sr() {
  PSWT=((SR&SR_TRACE)!=0);
  PSWS=((SR&SR_SUPER)!=0);
  PSWI=((SR&SR_IPL)>>8);
  PSWX=((SR&SR_X)!=0);
  PSWN=((SR&SR_N)!=0);
  PSWZ=((SR&SR_Z)!=0);
  PSWV=((SR&SR_V)!=0);
  PSWC=((SR&SR_C)!=0);
}


/*  Adapt function pointers to the current configuration.
    There are functions for timing (cycle counting), for reading the bus, 
    including fetching.
    Timing functions also call R/W functions.
*/

#if defined(SSE_OPTION_FASTBLITTER)

void dummy() {
}

#endif

void ChangeTimingFunctions() {
  if(IS_STE)
  {
    m68k_peek=m68k_peek_ste;
    m68k_dpeek=m68k_dpeek_ste;
    m68k_fetch=m68k_fetch_ste;
#if defined(SSE_VID_STVL1) // those are defined in interface_stvl.cpp
    if(OPTION_C3 && hStvl)
    {
      if(SSEConfig.CpuBoosted)
      {
        // 2 low-level video, acceleration
        pInstructionTimeIdle=InstructionTimeSte2Idle;
        pInstructionTimeWS=InstructionTimeSte2WS;
        pInstructionTimePrefetchOnly=InstructionTimeSte2PrefetchOnly;
        pInstructionTimePrefetchFinal=InstructionTimeSte2PrefetchFinal;
        pInstructionTimePrefetchTotal=InstructionTimeSte2PrefetchTotal;
        pInstructionTimeReadB=InstructionTimeSte2ReadB;
        pInstructionTimeRead=InstructionTimeSte2Read;
        pInstructionTimeWrite=InstructionTimeSte2Write;
        pInstructionTimeWriteB=InstructionTimeSte2WriteB;
        pInstructionTimeBltRead=InstructionTimeSte2BltRead;
        pInstructionTimeBltWrite=InstructionTimeSte2BltWrite;
      }
      else
      {
        // 1 low-level video, no acceleration
#if defined(SSE_MEGASTE)
        if(IS_MEGASTE)
        {
          pInstructionTimeIdle=InstructionTimeMegaSte1Idle;
          pInstructionTimeWS=InstructionTimeMegaSte1WS;
          pInstructionTimePrefetchOnly=InstructionTimeMegaSte1PrefetchOnly;
          pInstructionTimePrefetchFinal=InstructionTimeMegaSte1PrefetchFinal;
          pInstructionTimePrefetchTotal=InstructionTimeMegaSte1PrefetchTotal;
          pInstructionTimeReadB=InstructionTimeMegaSte1ReadB;
          pInstructionTimeRead=InstructionTimeMegaSte1Read;
          pInstructionTimeWrite=InstructionTimeMegaSte1Write;
          pInstructionTimeWriteB=InstructionTimeMegaSte1WriteB;
          pInstructionTimeBltRead=InstructionTimeMegaSte1BltRead;
          pInstructionTimeBltWrite=InstructionTimeMegaSte1BltWrite;
        }
        else
#endif
        {
          pInstructionTimeIdle=InstructionTimeSte1Idle;
          pInstructionTimeWS=InstructionTimeSte1WS;
          pInstructionTimePrefetchOnly=InstructionTimeSte1PrefetchOnly;
          pInstructionTimePrefetchFinal=InstructionTimeSte1PrefetchFinal;
          pInstructionTimePrefetchTotal=InstructionTimeSte1PrefetchTotal;
          pInstructionTimeReadB=InstructionTimeSte1ReadB;
          pInstructionTimeRead=InstructionTimeSte1Read;
          pInstructionTimeWrite=InstructionTimeSte1Write;
          pInstructionTimeWriteB=InstructionTimeSte1WriteB;
          pInstructionTimeBltRead=InstructionTimeSte1BltRead;
          pInstructionTimeBltWrite=InstructionTimeSte1BltWrite;
        }
      }
    }
    else
#endif
    {
      // 0 no low-level video, no acceleration
#if defined(SSE_MEGASTE)
      if(IS_MEGASTE)
      {
        pInstructionTimeIdle=InstructionTimeMegaSteIdle;
        pInstructionTimeWS=InstructionTimeMegaSteWS;
        pInstructionTimePrefetchOnly=InstructionTimeMegaStePrefetchOnly;
        pInstructionTimePrefetchFinal=InstructionTimeMegaStePrefetchFinal;
        pInstructionTimePrefetchTotal=InstructionTimeMegaStePrefetchTotal;
        pInstructionTimeReadB=InstructionTimeMegaSteReadB;
        pInstructionTimeRead=InstructionTimeMegaSteRead;
        pInstructionTimeWrite=InstructionTimeMegaSteWriteW;
        pInstructionTimeWriteB=InstructionTimeMegaSteWriteB;
        pInstructionTimeBltRead=InstructionTimeMegaSteBltRead;
        pInstructionTimeBltWrite=InstructionTimeMegaSteBltWrite;
      }
      else
#endif
      {
        pInstructionTimeIdle=InstructionTimeSteIdle;
        pInstructionTimeWS=InstructionTimeSteWS;
        pInstructionTimePrefetchOnly=InstructionTimeStePrefetchOnly;
        pInstructionTimePrefetchFinal=InstructionTimeStePrefetchFinal;
        pInstructionTimePrefetchTotal=InstructionTimeStePrefetchTotal;
        pInstructionTimeReadB=InstructionTimeSteReadB;
        pInstructionTimeRead=InstructionTimeSteRead;
        pInstructionTimeWrite=InstructionTimeSteWriteW;
        pInstructionTimeWriteB=InstructionTimeSteWriteB;
        pInstructionTimeBltRead=InstructionTimeSteBltRead;
        pInstructionTimeBltWrite=InstructionTimeSteBltWrite;
      }
    }
  }
  else if(ST_MODEL==STF||ST_MODEL==STFM)
  {
    m68k_peek=m68k_peek_stf;
    m68k_dpeek=m68k_dpeek_stf;
    m68k_fetch=m68k_fetch_stf;
#if defined(SSE_VID_STVL1)
    if(OPTION_C3 && hStvl)
    {
      if(SSEConfig.CpuBoosted)
      {
        // 2 low-level video, acceleration
        pInstructionTimeIdle=InstructionTimeStf2Idle;
        pInstructionTimeWS=InstructionTimeStf2WS;
        pInstructionTimePrefetchOnly=InstructionTimeStf2PrefetchOnly;
        pInstructionTimePrefetchFinal=InstructionTimeStf2PrefetchFinal;
        pInstructionTimePrefetchTotal=InstructionTimeStf2PrefetchTotal;
        pInstructionTimeReadB=InstructionTimeStf2ReadB;
        pInstructionTimeRead=InstructionTimeStf2Read;
        pInstructionTimeWrite=InstructionTimeStf2Write;
        pInstructionTimeWriteB=InstructionTimeStf2WriteB;
      }
      else
      {
        // 1 low-level video, no acceleration
        pInstructionTimeIdle=InstructionTimeStf1Idle;
        pInstructionTimeWS=InstructionTimeStf1WS;
        pInstructionTimePrefetchOnly=InstructionTimeStf1PrefetchOnly;
        pInstructionTimePrefetchFinal=InstructionTimeStf1PrefetchFinal;
        pInstructionTimePrefetchTotal=InstructionTimeStf1PrefetchTotal;
        pInstructionTimeReadB=InstructionTimeStf1ReadB;
        pInstructionTimeRead=InstructionTimeStf1Read;
        pInstructionTimeWrite=InstructionTimeStf1Write;
        pInstructionTimeWriteB=InstructionTimeStf1WriteB;
      }
    }
    else 
#endif
    {
      // 0 no low-level video
      pInstructionTimeIdle=InstructionTimeStfIdle;
      pInstructionTimeWS=InstructionTimeStfWS;
      pInstructionTimePrefetchOnly=InstructionTimeStfPrefetchOnly;
      pInstructionTimePrefetchFinal=InstructionTimeStfPrefetchFinal;
      pInstructionTimePrefetchTotal=InstructionTimeStfPrefetchTotal;
      pInstructionTimeReadB=InstructionTimeStfReadB;
      pInstructionTimeRead=InstructionTimeStfRead;
      pInstructionTimeWrite=InstructionTimeStfWriteW;
      pInstructionTimeWriteB=InstructionTimeStfWriteB;
    }
  }
  else if(ST_MODEL==MEGA_ST)
  {
    m68k_peek=m68k_peek_stf;
    m68k_dpeek=m68k_dpeek_stf;
    m68k_fetch=m68k_fetch_stf;
#if defined(SSE_VID_STVL1)
    if(OPTION_C3 && hStvl)
    {
      if(SSEConfig.CpuBoosted)
      {
        // 2 low-level video, acceleration
        pInstructionTimeIdle=InstructionTimeMegaSt2Idle;
        pInstructionTimeWS=InstructionTimeMegaSt2WS;
        pInstructionTimePrefetchOnly=InstructionTimeMegaSt2PrefetchOnly;
        pInstructionTimePrefetchFinal=InstructionTimeMegaSt2PrefetchFinal;
        pInstructionTimePrefetchTotal=InstructionTimeMegaSt2PrefetchTotal;
        pInstructionTimeReadB=InstructionTimeMegaSt2ReadB;
        pInstructionTimeRead=InstructionTimeMegaSt2Read;
        pInstructionTimeWrite=InstructionTimeMegaSt2Write;
        pInstructionTimeWriteB=InstructionTimeMegaSt2WriteB;
        pInstructionTimeBltRead=InstructionTimeMegaSt2BltRead;
        pInstructionTimeBltWrite=InstructionTimeMegaSt2BltWrite;
      }
      else
      {
        // 1 low-level video, no acceleration
        pInstructionTimeIdle=InstructionTimeMegaSt1Idle;
        pInstructionTimeWS=InstructionTimeMegaSt1WS;
        pInstructionTimePrefetchOnly=InstructionTimeMegaSt1PrefetchOnly;
        pInstructionTimePrefetchFinal=InstructionTimeMegaSt1PrefetchFinal;
        pInstructionTimePrefetchTotal=InstructionTimeMegaSt1PrefetchTotal;
        pInstructionTimeReadB=InstructionTimeMegaSt1ReadB;
        pInstructionTimeRead=InstructionTimeMegaSt1Read;
        pInstructionTimeWrite=InstructionTimeMegaSt1Write;
        pInstructionTimeWriteB=InstructionTimeMegaSt1WriteB;
        pInstructionTimeBltRead=InstructionTimeMegaSt1BltRead;
        pInstructionTimeBltWrite=InstructionTimeMegaSt1BltWrite;
      }
    }
    else
#endif
    {
      // 0 no low-level video
      pInstructionTimeIdle=InstructionTimeSteIdle;
      pInstructionTimeWS=InstructionTimeSteWS;
      pInstructionTimePrefetchOnly=InstructionTimeStePrefetchOnly;
      pInstructionTimePrefetchFinal=InstructionTimeStePrefetchFinal;
      pInstructionTimePrefetchTotal=InstructionTimeStePrefetchTotal;
      pInstructionTimeReadB=InstructionTimeSteReadB;
      pInstructionTimeRead=InstructionTimeSteRead;
      pInstructionTimeWrite=InstructionTimeSteWriteW;
      pInstructionTimeWriteB=InstructionTimeSteWriteB;
      pInstructionTimeBltRead=InstructionTimeSteBltRead;
      pInstructionTimeBltWrite=InstructionTimeSteBltWrite;
    }
  }
#if defined(SSE_OPTION_FASTBLITTER)
  if(OPTION_FASTBLITTER)
    pInstructionTimeBltRead=pInstructionTimeBltWrite=dummy;
#endif
}


/*  Specialised timing functions
    Simpler for STF, but to be strict there should be bus arbitration too
    however disk dma timings are less important than for the blitter so
    we can get away with it... TODO maybe, but there's a trade-off with load
*/


#define CHECK_BLIT_REQUEST if(Blitter.Request) Blitter_CheckRequest()
#define BLIT_CYCLES Blitter.BlitCycles
#define BLIT_BUS_ACCESS_CTR Blitter.BusAccessCounter
#define THINKING_CYCLES Cpu.BusIdleCycles

void InstructionTimeStfIdle(int t) {
  BUS_MASK=0; // no bus access, so no wait states possible
  cpu_cycles-=(t);
}


void InstructionTimeStfWS(int t) {
  // wait states: don't change BUS_MASK
  cpu_cycles-=(t);
}


void InstructionTimeStfPrefetchOnly() { // PREFETCH_ONLY
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  cpu_cycles-=4;
  abus=(pc&0xfffffe);
  // rounding up to 4 for RAM access - Shifter is handled in IO
  if(abus<himem) // COMPROMISE: wrong for first 8 bytes but who's gonna fetch there?
    cpu_cycles&=-4;
  IRC=m68k_fetch(pc);
}


void InstructionTimeStfPrefetchFinal() { // PREFETCH_FINAL
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  IR=IRC;
  MEM_ADDRESS au=pc+2;
  abus=(au&0xfffffe);
  cpu_cycles-=4;
  if(abus<himem)
    cpu_cycles&=-4;
  IRC=m68k_fetch(au);
}


void InstructionTimeStfPrefetchTotal() { // PREFETCH
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  pc+=2;
  cpu_cycles-=4;
  abus=(pc&0xfffffe);
  if(abus<himem)
    cpu_cycles&=-4;
  IRC=m68k_fetch(pc);
}


void InstructionTimeStfReadB() {
  abus=(iabus&0x00FFFFFE);
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_HIBYTE);
  cpu_cycles-=4;
  if(abus<himem && abus>8) // those 8 shadow RAM bytes are a drag on emulation performance
    cpu_cycles&=-4;
  m68k_peek(iabus);
}


void InstructionTimeStfRead() {
  abus=(iabus&0x00FFFFFE);
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  cpu_cycles-=4;
  if(abus<himem && abus>8)
    cpu_cycles&=-4;
  dbus=m68k_dpeek(iabus);
}


void InstructionTimeStfWriteB() {
  abus=(iabus&0x00FFFFFE);
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_HIBYTE);
  cpu_cycles-=4;
  if(abus<himem)
    cpu_cycles&=-4;
  dbush=dbusl; // motorola quirk, assume correct byte is low order
  m68k_poke_abus(dbusl);
}


void InstructionTimeStfWriteW() {
  abus=(iabus&0x00FFFFFE);
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  cpu_cycles-=4;
  if(abus<himem)
    cpu_cycles&=-4;
  m68k_dpoke_abus(dbus);
}


void InstructionTimeSteIdle(int t) {
  BUS_MASK=0;
  while(Blitter.BlitCycles>t && t>0)
  {
    Blitter.BlitCycles--;
    t--; // CPU running during a blit (rare)
  }
  cpu_cycles-=(t);
  Cpu.BusIdleCycles+=t;
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSteWS(int t) {
  cpu_cycles-=(t);
}


void InstructionTimeStePrefetchOnly() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  cpu_cycles-=4;
  Blitter.BlitCycles=0;
  Cpu.BusIdleCycles=0;
  abus=(pc&0xfffffe);
  if(abus<himem)
    cpu_cycles&=-4;
  BLIT_BUS_ACCESS_CTR++; // 7bit + only if busy on HW but we must keep code lean
  IRC=m68k_fetch(pc);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeStePrefetchFinal() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  IR=IRC;
  MEM_ADDRESS au=pc+2;
  cpu_cycles-=4;
  Blitter.BlitCycles=0;
  Cpu.BusIdleCycles=0;
  abus=(au&0xfffffe);
  if(abus<himem)
    cpu_cycles&=-4;
  BLIT_BUS_ACCESS_CTR++;
  IRC=m68k_fetch(au);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeStePrefetchTotal() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  pc+=2;
  cpu_cycles-=4;
  Blitter.BlitCycles=0;
  Cpu.BusIdleCycles=0;
  abus=(pc&0xfffffe);
  if(abus<himem)
    cpu_cycles&=-4;
  BLIT_BUS_ACCESS_CTR++;
  IRC=m68k_fetch(pc);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSteReadB() {
  abus=(iabus&0x00FFFFFE);
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_HIBYTE);
  cpu_cycles-=4;
  Blitter.BlitCycles=0;
  Cpu.BusIdleCycles=0;
  if(abus<himem && abus>8)
    cpu_cycles&=-4;
  BLIT_BUS_ACCESS_CTR++;
  m68k_peek(iabus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSteRead() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  abus=(iabus&0x00FFFFFE);
  cpu_cycles-=4;
  Blitter.BlitCycles=0;
  Cpu.BusIdleCycles=0;
  if(abus<himem && abus>8)
    cpu_cycles&=-4;
  BLIT_BUS_ACCESS_CTR++;
  dbus=m68k_dpeek(iabus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSteWriteB() {
  abus=(iabus&0x00FFFFFE);
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_HIBYTE);
  cpu_cycles-=4;
  Blitter.BlitCycles=0;
  Cpu.BusIdleCycles=0;
  if(abus<himem)
    cpu_cycles&=-4;
  BLIT_BUS_ACCESS_CTR++;
  dbush=dbusl;
  m68k_poke_abus(dbusl);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSteWriteW() {
  abus=(iabus&0x00FFFFFE);
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  cpu_cycles-=4;
  Blitter.BlitCycles=0;
  Cpu.BusIdleCycles=0;
  if(abus<himem)
    cpu_cycles&=-4;
  BLIT_BUS_ACCESS_CTR++;
  m68k_dpoke_abus(dbus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeSteBltRead() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  cpu_cycles-=4;
  if(abus<himem && abus>8)
    cpu_cycles&=-4;
  // peek is done by blitter
  BLIT_BUS_ACCESS_CTR++;
}


void InstructionTimeSteBltWrite() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  cpu_cycles-=4;
  if(abus<himem)
    cpu_cycles&=-4;
  // poke is done by blitter
  BLIT_BUS_ACCESS_CTR++;
}


#if defined(SSE_MEGASTE)

void InstructionTimeMegaSteIdle(int t) {
  BUS_MASK=0;
  while(BLIT_CYCLES>t && t>0)
  {
    BLIT_CYCLES--;
    t--;
  }
  THINKING_CYCLES+=t;
  ASSERT(!(t&1));
  if(MegaSte.MemCache.ScuReg&1) // bit 0 = CPU clock
    t/=2; // to emulate 16MHz we just count half the cycles
  cpu_cycles-=(t);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSteWS(int t) { // same
  cpu_cycles-=(t);
}


void InstructionTimeMegaStePrefetchOnly() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  abus=(pc&0xfffffe);
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  BLIT_BUS_ACCESS_CTR++;
  if(MegaSte.MemCache.Check(abus,IRC)) // IRC is updated by the function
    cpu_cycles-=2; // only possible if cached + 16MHz
  else
  {
    cpu_cycles-=4;
    if(abus<himem)
      cpu_cycles&=-4;
    IRC=m68k_fetch(pc);
    MegaSte.MemCache.Add(abus);
  }
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaStePrefetchFinal() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  IR=IRC;
  MEM_ADDRESS au=pc+2;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  abus=(au&0xfffffe);
  BLIT_BUS_ACCESS_CTR++;
  if(MegaSte.MemCache.Check(abus,IRC))
    cpu_cycles-=2;
  else
  {
    cpu_cycles-=4;
    if(abus<himem)
      cpu_cycles&=-4;
    IRC=m68k_fetch(au);
    MegaSte.MemCache.Add(abus);
  }
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaStePrefetchTotal() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
  pc+=2;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  abus=(pc&0xfffffe);
  BLIT_BUS_ACCESS_CTR++;
  if(MegaSte.MemCache.Check(abus,IRC))
    cpu_cycles-=2;
  else
  {
    cpu_cycles-=4;
    if(abus<himem)
      cpu_cycles&=-4;
    IRC=m68k_fetch(pc);
    MegaSte.MemCache.Add(abus);
  }
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSteReadB() {
  abus=(iabus&0x00FFFFFE);
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_HIBYTE);
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  BLIT_BUS_ACCESS_CTR++;
  if(MegaSte.MemCache.Check(abus,dbus))
  {
    cpu_cycles-=2;
    d8=(iabus&1) ? dbusl : dbush;
  }
  else
  {
    cpu_cycles-=4;
    if(abus<himem && abus>8)
      cpu_cycles&=-4;
    m68k_peek(iabus);
    MegaSte.MemCache.Add(abus);
  }
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSteRead() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  abus=(iabus&0x00FFFFFE);
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  BLIT_BUS_ACCESS_CTR++;
  if(MegaSte.MemCache.Check(abus,dbus))
    cpu_cycles-=2;
  else
  {
    cpu_cycles-=4;
    if(abus<himem && abus>8)
      cpu_cycles&=-4;
    dbus=m68k_dpeek(iabus);  
  }
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSteWriteB() {
  abus=(iabus&0x00FFFFFE);
  BUS_MASK=(iabus&1)?(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_LOBYTE)
    :(BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_HIBYTE);
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  BLIT_BUS_ACCESS_CTR++;
  dbush=dbusl;
  cpu_cycles-=4; // writes are not cached
  if(abus<himem)
    cpu_cycles&=-4;
  m68k_poke_abus(dbusl);
  MegaSte.MemCache.Add(abus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSteWriteW() {
  abus=(iabus&0x00FFFFFE);
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  BLIT_CYCLES=0;
  THINKING_CYCLES=0;
  BLIT_BUS_ACCESS_CTR++;
  cpu_cycles-=4;
  if(abus<himem)
    cpu_cycles&=-4;
  m68k_dpoke_abus(dbus);
  MegaSte.MemCache.Add(abus);
  CHECK_BLIT_REQUEST;
}


void InstructionTimeMegaSteBltRead() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
  if(MegaSte.MemCache.Check(abus,dbus))
    cpu_cycles-=2; // The blitter uses the cache too
  else
  {
    cpu_cycles-=4;
    if(abus<himem && abus>8)
      cpu_cycles&=-4;
    if(abus<himem)
      MegaSte.MemCache.Add(abus);
  }
  BLIT_BUS_ACCESS_CTR++;
}


void InstructionTimeMegaSteBltWrite() {
  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;
  cpu_cycles-=4;
  if(abus<himem)
    cpu_cycles&=-4;
  MegaSte.MemCache.Add(abus);
  BLIT_BUS_ACCESS_CTR++;
}

#endif//#if defined(SSE_MEGASTE)

// pointers to functions
void (*m68k_call_table[0xffff+1])(); // opcode entry
void (*m68k_jsr_get_source_b[8])();
void (*m68k_jsr_get_source_w[8])();
void (*m68k_jsr_get_source_l[8])();
void (*m68k_jsr_get_source_b_not_a[8])();
void (*m68k_jsr_get_source_w_not_a[8])();
void (*m68k_jsr_get_source_l_not_a[8])();
void (*m68k_jsr_get_dest_b[8])();
void (*m68k_jsr_get_dest_w[8])();
void (*m68k_jsr_get_dest_l[8])();
void (*m68k_jsr_get_dest_b_not_a[8])();
void (*m68k_jsr_get_dest_w_not_a[8])();
void (*m68k_jsr_get_dest_l_not_a[8])();
void (*m68k_jsr_get_dest_b_not_a_or_d[8])();
void (*m68k_jsr_get_dest_w_not_a_or_d[8])();
void (*m68k_jsr_get_dest_l_not_a_or_d[8])();


void m68k_lpoke_abus(LONG x) {
  // lpoke now used only by utility functions, not CPU emulation proper
  abus=iabus&0xffffff;
  m68k_dpoke_abus(x>>16);
  iabus+=2;
  m68k_dpoke_abus(x&0xffff);
}


void m68k_push_l_without_timing(DWORD x) {
  AREG(7)-=4;
  iabus=AREG(7);
  m68k_dpoke_abus(x>>16);
  iabus+=2;
  m68k_dpoke_abus(x&0xFFFF);
}


void m68k_finish_exception(int vector) { //This is to avoid code duplication
/*
  Trace               | 36(4/3)  |              nn    ns nS ns nV nv np n np
  CHK Instruction     | 42(4/3)+ |   np (n-)    nn    ns nS ns nV nv np n np      
  Divide by Zero      | 40(4/3)+ |           nn nn    ns nS ns nV nv np n np      
  TRAP instruction    | 36(4/3)  |              nn    ns nS ns nV nv np n np      
  TRAPV instruction   | 36(5/3)  |   np               ns nS ns nV nv np n np   
Here we do:                                           ns nS ns nV nv np n np   
  vector was multiplied by 4 (during some n timing)
*/
  UPDATE_SR;
  WORD saved_sr=SR;
  if(!SUPERFLAG)
    change_to_supervisor_mode();
  CLEAR_T;
  iabus=AREG(7)-2;
  dbus=pcl; // stack PC low word
  CPU_BUS_ACCESS_WRITE; // ns
  iabus-=4;
  dbus=saved_sr; // stack SR
  CPU_BUS_ACCESS_WRITE; // ns
  AREG(7)=iabus;
  iabus+=2;
  dbus=pch; // PC high word 
  CPU_BUS_ACCESS_WRITE; // nS
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  Debug.PseudoStackPush(pc);
#endif
  iabus=vector;
  CPU_BUS_ACCESS_READ; // nV
  effective_address_h=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; // nv
  effective_address_l=dbus;
  if(Cpu.ProcessingState!=TMC68000::HALTED) // normally not, just in case
    Cpu.ProcessingState=TMC68000::NORMAL; // before fetching PC
  m68kSetPC(effective_address,2);
  interrupt_depth++;
}


void m68k_interrupt(MEM_ADDRESS ad) { 
/*  Only called by GEMDOS hack (os_gemdos_vector), no need for precise 
    CPU behaviour here
*/
  M68K_UNSTOP;
  if(!SUPERFLAG)
    change_to_supervisor_mode();
  CLEAR_T;
  m68k_PUSH_L(pc);
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  Debug.PseudoStackPush(pc);
#endif
  UPDATE_SR;
  m68k_PUSH_W(SR);
  SET_PC(ad);
  interrupt_depth++;
}


#undef LOGSECTION
#define LOGSECTION LOGSECTION_CRASH

void exception(int en,int ea,MEM_ADDRESS a) {
  ioaccess=0;
  ExceptionObject.init(en,ea,a);
#if defined(SSE_M68K_EXCEPTION_TRY_CATCH)
  throw &ExceptionObject;
#else
  if(pJmpBuf==NULL)
  {
    if(en)
    {
      log_write(Str("Unhandled exception! pc=")+HEXSl(old_pc,6)+" action="+(int)ea+" address involved="+HEXSl(a,6));
      //BRK(Unhandled exception!); //emulator crash on bad snapshot etc.
    }
    return;
  }
  longjmp(*pJmpBuf,1);
#endif
}

void m68k_exception::init(int en,int ea,MEM_ADDRESS a) {
  bombs=en;
  u_pc.d32=pc;
  ucrash_address.d32=old_pc; //this is for group 1+2
  if((bombs==2||bombs==3) && ea!=EA_FETCH)
    uaddress.d32=iabus;
  else
    uaddress.d32=a;
  UPDATE_SR;
  crash_sr=SR;
  crash_ird=IRD;
  // we use bus mask instead of ea to assign action
  if((BUS_MASK&BUS_MASK_FETCH)==BUS_MASK_FETCH)
    action=EA_FETCH;
  else if((BUS_MASK&BUS_MASK_WRITE)==BUS_MASK_WRITE)
    action=EA_WRITE;
  else
    action=EA_READ;
}


void m68k_halt() {
  Cpu.ProcessingState=TMC68000::HALTED;
   // contrary to real ST we indicate the CPU's halted
  StatusInfo.MessageIndex=TStatusInfo::MOTOROLA_CRASH;
  REFRESH_STATUS_BAR;
  runstate=RUNSTATE_STOPPING;
}


#ifdef DEBUG_BUILD

void log_registers() {
  log_write("        Register dump:");
  log_write(EasyStr("        pc = ")+HEXSl(pc,8));
  log_write(EasyStr("        sr = ")+HEXSl(SR,4));
  for(int n=0;n<16;n++)
  {
    log_write(EasyStr("        ")+("d\0a\0"+(n/8)*2)+(n&7)+" = "+HEXSl(Cpu.r[n],8));
  }
}


void log_history(int bombs,MEM_ADDRESS crash_address) {
  if(logsection_enabled[LOGSECTION]&&logging_suspended==0)
  {
    DBG_LOG("");
    DBG_LOG("****************************************");
    if(bombs)
    {
      DBG_LOG(EasyStr(bombs)+" bombs");
    }
    else
    {
      DBG_LOG("Exception/interrupt");
    }
    DBG_LOG(EasyStr("Crash at ")+HEXSl(crash_address,6));
    int n=pc_history_idx-HIST_MENU_SIZE;
    if(n<0)
      n=HISTORY_SIZE+n;
    EasyStr Disassembly;
    do
    {
      if(pc_history[n]!=0xffffff71)
      {
        Disassembly=disa_d2(pc_history[n]);
        DBG_LOG(EasyStr(HEXSl(pc_history[n],6))+" - "+Disassembly);
      }
      n++; if(n>=HISTORY_SIZE) n=0;
    } while(n!=pc_history_idx);
    DBG_LOG("^^ Crash!");
    DBG_LOG("****************************************");
    DBG_LOG("");
  }
}

#endif


void m68k_exception::crash() {
  ASSERT(bombs<12);
#if defined(SSE_STATS)
  Stats.nException[bombs]++;
#endif
  if(bombs==0) // exception 0 is reset
  {
    reset_st(uaddress.d32|RESET_STAGE2);  // address = flags of reset_st (trick)
    return;
  }
  M68K_UNSTOP;
#if defined(SSE_DEBUG)   
  if(Cpu.nExceptions!=-1)
  {
    Cpu.nExceptions++;  
    TRACE_LOG("\nException #%d, %d bombs (",Cpu.nExceptions,bombs);
#if defined(SSE_DEBUGGER_OSD_CONTROL)
    if(OSD_MASK_CPU & OSD_CONTROL_CPUBOMBS) 
#endif
      TRACE_OSD("%d-%d BOMBS",Cpu.nExceptions,bombs);
    switch(bombs) {  
    case 2:
      TRACE_LOG("BOMBS_BUS_ERROR"); 
      break;
    case 3:
      TRACE_LOG("BOMBS_ADDRESS_ERROR"); 
      break;
    case 4:
      TRACE_LOG("BOMBS_ILLEGAL_INSTRUCTION"); 
      break;
    case 5:
      TRACE_LOG("BOMBS_DIVISION_BY_ZERO"); 
      break;
    case 6:
      TRACE_LOG("BOMBS_CHK"); 
      break;
    case 7:
      TRACE_LOG("BOMBS_TRAPV"); 
      break;
    case 8:
      TRACE_LOG("BOMBS_PRIVILEGE_VIOLATION"); 
      break;
    case 9:
      TRACE_LOG("BOMBS_TRACE_EXCEPTION"); 
      break;
    case 10:
      TRACE_LOG("BOMBS_LINE_A"); 
      break;
    case 11:
      TRACE_LOG("BOMBS_LINE_F"); 
      break;
    }//sw
#if defined(DEBUG_BUILD)
    TRACE_LOG(") during \"%s\"\n",exception_action_name[action]);
#else
    TRACE_LOG(") action %d\n",action);
#endif
#ifdef DEBUG_BUILD 
    // take advantage of the disassembler
    // disassembly can be wrong if word at old_pc has changed!
    EasyStr instr=disa_d2(old_pc);
    TRACE_LOG("PC=%X-IRD=%04X-Ins: %s -SR=%04X-Bus=%06X",
      old_pc,crash_ird,instr.Text,crash_sr,abus);
#else
    TRACE_LOG("PC=%X-IRD=%04X-SR=%04X-Bus=%06X",old_pc,crash_ird,crash_sr,iabus);
#endif
    TRACE_LOG("-Vector $%X=%08X\n",bombs*4,LPEEK(bombs*4));
    // dump registers
    TRACE_LOG("D0=%X D1=%X D2=%X D3=%X D4=%X D5=%X D6=%X D7=%X\n",
      Cpu.r[0],Cpu.r[1],Cpu.r[2],Cpu.r[3],Cpu.r[4],Cpu.r[5],Cpu.r[6],Cpu.r[7]);
    TRACE_LOG("A0=%X A1=%X A2=%X A3=%X A4=%X A5=%X A6=%X A7=%X\n",Cpu.r[8],
      Cpu.r[9],Cpu.r[10],Cpu.r[11],Cpu.r[12],Cpu.r[13],Cpu.r[14],Cpu.r[15]);
  }
#elif defined(SSE_OSD_DEBUGINFO)
  if(OPTION_OSD_DEBUGINFO)
      TRACE_OSD2("%dB",bombs);
#endif
  bool inExcept01=(Cpu.ProcessingState==TMC68000::EXCEPTION); // but which exception?
  Cpu.ProcessingState=TMC68000::EXCEPTION;
  Cpu.tpend=false;
  if(bombs==BOMBS_ILLEGAL_INSTRUCTION||bombs==BOMBS_PRIVILEGE_VIOLATION)
  {
    // Illegal Instruction | 34(4/3)  |              nn    ns nS ns nV nv np n np
    CPU_BUS_IDLE(4); //nn
    if(!SUPERFLAG) //set S
      change_to_supervisor_mode();
    CLEAR_T;
    UPDATE_SR;
    TRACE_LOG("Push PC %X on %X, SR %04X on %X\n",ucrash_address.d32,sp-4,crash_sr,sp-6);
    iabus=AREG(7)-2;
    dbus=ucrash_address.d16[LO]; // stack PC low word
    CPU_BUS_ACCESS_WRITE; // ns 
    iabus-=4;
    dbus=crash_sr; // SR written between two parts of PC
    AREG(7)=iabus;
    CPU_BUS_ACCESS_WRITE; // ns
    iabus+=2;
    dbus=ucrash_address.d16[HI]; // PC high word 
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
    Debug.PseudoStackPush(ucrash_address.d32);
#endif
    CPU_BUS_ACCESS_WRITE; // nS      
    iabus=bombs*4;
    CPU_BUS_ACCESS_READ; // nV
    effective_address_h=dbus;
    iabus+=2;
    CPU_BUS_ACCESS_READ; // nv
    effective_address_l=dbus;
    MEM_ADDRESS ad=effective_address;
    if(ad&1) // bad vector!
    {
      // Very rare, generally indicates emulation/snapshot bug, but there are cases
      bombs=BOMBS_ADDRESS_ERROR;
#if defined(SSE_STATS)
      Stats.nException[bombs]++;
#endif
#if defined(SSE_DEBUG)
      Cpu.nExceptions++;
      TRACE_LOG("->%d bombs\n",bombs);
#endif
      uaddress.d32=ad;
      action=EA_FETCH;
    }
    else
    {
      TRACE_LOG("PC = %X\n\n",ad);
      Cpu.ProcessingState=TMC68000::NORMAL;
      m68kSetPC(ad,2);
      interrupt_depth++;
    }
  }
  if(bombs==BOMBS_BUS_ERROR||bombs==BOMBS_ADDRESS_ERROR)
  {
/*
Address error       | 50(4/7)  |     nn ns nS ns ns ns nS ns nV nv np n np
Bus error           | 50(4/7)  |     nn ns nS ns ns ns nS ns nV nv np n np
*/
    // MC68000 bug documented by ijor, for the explanation see:
    // http://www.atari-forum.com/viewtopic.php?f=68&t=37890#p387686
    // it's not fancy to support it because the opcode and the ssw are changed
    if(crash_ird!=IR) // simplification, no overhead cost
    {
      crash_ird=IR;
      TRACE_LOG("TVN latched IR %04X I/N %d\n",crash_ird,inExcept01);
      // test is heavy but it's rarely necessary
      if(Cpu.tpend || check_ipl() || m68k_call_table[crash_ird]==m68k_trap1
        || (crash_sr&0x2000)==0 && Cpu.IsPriv(crash_ird)
        || m68k_call_table[crash_ird]==m68k_lineA
        || m68k_call_table[crash_ird]==m68k_lineF)
        inExcept01=true; // I/N bit affected
    }
    // The GLUE contains a 6bit counter that asserts BERR if AS stays asserted 
    // for more than 64 cycles. 64 cycles + 4 see error + 2 internal CPU propagation?
    // We don't emulate this at low level so we count cycles here.
    int ncycles=(bombs==BOMBS_BUS_ERROR) ? (64+4+2) : (4); // timing on STE (BUSERRT1.TOS)
    for(int i=0;i<ncycles;i+=2)
    {
      BUS_JAM_TIME(2); // just in case, avoid too long CPU timings
    }
    BUS_JAM_TIME(4); //nn
    if(!SUPERFLAG)
      change_to_supervisor_mode();
    CLEAR_T;
    UPDATE_SR;
    TRY_M68K_EXCEPTION
      if(u_pc.d32!=Cpu.Pc)
      {
        TRACE_LOG("pc %X true PC %X\n",u_pc.d32,Cpu.Pc);
        u_pc.d32=Cpu.Pc; // guaranteed exact...
      }
      TRACE_LOG("Push PC %X on %X, SR %04X on %X\n",u_pc.d32,sp-4,crash_sr,sp-6);
      iabus=AREG(7)-2;
      dbus=u_pc.d16[LO]; // stack PC low word
      CPU_BUS_ACCESS_WRITE; // ns 
      iabus-=4;
      dbus=crash_sr; // SR written between two parts of PC
      CPU_BUS_ACCESS_WRITE; // ns
      AREG(7)=iabus;
      iabus+=2;
      dbus=u_pc.d16[HI]; // PC high word 
      CPU_BUS_ACCESS_WRITE; // nS      
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
      Debug.PseudoStackPush(u_pc.d32);
#endif
      TRACE_LOG("Push IR %X on %X\n",crash_ird,sp-2);
      iabus=AREG(7)-2;
      dbus=crash_ird;
      AREG(7)=iabus;
      CPU_BUS_ACCESS_WRITE; // ns
      // special status word: 5 bits only, the rest is previous ird
      // (because ftu wasn't reset between the two writes - ijor)
      WORD ssw=(crash_ird&0xffe0);
      // ssw <= { ~bciWrite, inExcept01, rFC};
      if(action!=EA_WRITE)
        ssw|=B6_010000;
      if(inExcept01)
        ssw|=B6_001000;
      if(crash_sr & SR_SUPER)
        ssw|=B6_000100;
      if(action==EA_FETCH)
        ssw|=B6_000010;
      else
        ssw|=B6_000001;
      TRACE_LOG("Push crash address %X on %X, ssw %04X on %X\n",
        uaddress.d32,sp-4,ssw,sp-6);
      iabus=AREG(7)-2;
      dbus=uaddress.d16[LO]; // stack crash address low word
      CPU_BUS_ACCESS_WRITE; // ns 
      iabus-=4;
      dbus=ssw;
      CPU_BUS_ACCESS_WRITE; // ns
      AREG(7)=iabus;
      iabus+=2;
      dbus=uaddress.d16[HI]; // crash address high word 
      CPU_BUS_ACCESS_WRITE; // nS      
      iabus=bombs*4;
      CPU_BUS_ACCESS_READ; // nV
      effective_address_h=dbus;
      iabus+=2;
      CPU_BUS_ACCESS_READ; // nv
      effective_address_l=dbus;
      iabus=effective_address;
      TRACE_LOG("PC = %X\n",iabus);
      if(Cpu.ProcessingState!=TMC68000::HALTED)
        Cpu.ProcessingState=TMC68000::NORMAL;
      m68kSetPC(iabus,2);
    CATCH_M68K_EXCEPTION
      TRACE2("HALT PC %X SR %X address %X Exception %d dbus %X abus %X\n",u_pc.d32,crash_sr,uaddress.d32,bombs,dbus,iabus); 
      m68k_halt(); // bus/address error in group 0 exception
      return;
    END_M68K_EXCEPTION
    interrupt_depth++;
  }
#ifdef ENABLE_LOGFILE
  log_history(bombs,ucrash_address.d32);
#endif
  if(!OPTION_EMUTHREAD)
    PeekEvent(); // Stop exception freeze
}

#undef LOGSECTION


#if defined(DEBUG_BUILD)

void DebugCheckIOAccess() {
  if(ioaccess & IOACCESS_DEBUG_MEM_WRITE_LOG)
  {
      int val=int((debug_mem_write_log_bytes==1) 
        ? int(d2_peek(debug_mem_write_log_address)) 
        : int(d2_dpeek(debug_mem_write_log_address))); 
      log_write(HEXSl(old_pc,6)+": Write to address $"+HEXSl(debug_mem_write_log_address,6)+
        ", new value is "+val+" ($"+HEXSl(val,debug_mem_write_log_bytes*2)+")"); 
      val=d2_peek(debug_mem_write_log_address);
      int val2=(debug_mem_write_log_address&1) 
        ? 0 : d2_dpeek(debug_mem_write_log_address);
      int val3=(debug_mem_write_log_address&1)
        ? 0 : d2_lpeek(debug_mem_write_log_address);
      TRACE("PC %X %s write %X|%X|%X to %Xn",old_pc,disa_d2(old_pc).Text,
        val,val2,val3,debug_mem_write_log_address);
  }
  if(ioaccess & IOACCESS_DEBUG_MEM_READ_LOG)
  {
      int val=int((debug_mem_write_log_bytes==1)
        ? int(d2_peek(debug_mem_write_log_address))
        : int(d2_dpeek(debug_mem_write_log_address))); 
      log_write(HEXSl(old_pc,6)+": Read from address $"+HEXSl(debug_mem_write_log_address,6)+
        ", = "+val+" ($"+HEXSl(val,debug_mem_write_log_bytes*2)+")"); 
      val=d2_peek(debug_mem_write_log_address);
      int val2=(debug_mem_write_log_address&1)
        ? 0 : d2_dpeek(debug_mem_write_log_address);
      int val3=(debug_mem_write_log_address&1)
        ? 0 : d2_lpeek(debug_mem_write_log_address);
      TRACE("PC %X %s read %X|%X|%X from %Xn",old_pc,disa_d2(old_pc).Text,
        val,val2,val3,debug_mem_write_log_address);
  }
  ioaccess&=~(IOACCESS_DEBUG_MEM_WRITE_LOG|IOACCESS_DEBUG_MEM_READ_LOG);
}

#endif


bool TMC68000::IsPriv(WORD op) { // it's less heavy than a flag in a big opcode table
  bool is_priv=(op==0x4E70 || op==0x4E72 || op==0x4E73 // reset||stop||rte
   || m68k_call_table[op]==m68k_ori_w_to_sr
   || m68k_call_table[op]==m68k_andi_w_to_sr
   || m68k_call_table[op]==m68k_eori_w_to_sr
   || m68k_call_table[op]==m68k_move_to_sr
   || m68k_call_table[op]==m68k_move_to_usp
   || m68k_call_table[op]==m68k_move_from_usp);
  return is_priv;
}


#define LOGSECTION LOGSECTION_INTERRUPTS


COUNTER_VAR tvn_latch_time=0; // updated by CPU opcode emulation

/*  check if tvn was set for interrupts this instruction
    we look back because event may trigger right after the instruction
*/

#ifdef VC_BUILD
//__forceinline 
#endif
inline // it doesn't get inlined
BYTE check_ipl() {
#if 0
#ifdef DEBUG_BUILD
  if(stem_runmode==STEM_MODE_CPU)
#endif
  // if() should be enough, while better but risk of hanging at 0
  if(cpu_cycles<=0) 
  {
    event_vector();
    prepare_next_event();
  }
#else // more complex and safe
  for(int i=0;cpu_cycles<=0&&i<10;i++)
  {
#ifdef DEBUG_BUILD
    if(stem_runmode!=STEM_MODE_CPU)
      break;
#endif
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
    if(TRACE_MASK2&TRACE_CONTROL_EVENT)
      TRACE_EVENT(event_vector);
#endif
    event_vector();
    prepare_next_event();
  }
#endif
  // get correct ipl
  BYTE ipl;
  BYTE look_back_index=ipl_timing_index;
  // tvn_latch_time = 1 cycle before rInterrupt latch
  //                  0.5 cycle after ipl should be lastly updated to be stable 
  while(tvn_latch_time-ipl_timing[look_back_index].time<DBI_DELAY)
  {
    look_back_index--;
    // necessary test, happens on reset
    if(look_back_index==ipl_timing_index)
    {
      TRACE_LOG("ipl overflow latch " PRICV " time " PRICV " ipl %d\n",tvn_latch_time,ipl_timing[look_back_index].time,ipl_timing[look_back_index].ipl);
      TRACE_LOG("MFP %d (%x) VBI %d HBI %d\n",Mfp.Irq,MFP_IRQ,Glue.vbl_pending,Glue.hbl_pending);
      update_ipl(ACT); // get current ipl regardless of timing
      look_back_index=ipl_timing_index;
      break;
    }
  }
  ipl=ipl_timing[look_back_index].ipl;
  // return ipl level IF above mask, 0 otherwise
  return ((ipl>pswI)?ipl:0);
}


#undef LOGSECTION
#define LOGSECTION LOGSECTION_CPU

void m68kProcess() { // process one instruction
#if defined(SSE_DEBUG)
#if defined(SSE_DEBUGGER)
  LOG_CPU  
/*  Very powerful but demanding traces.
    You may have, beside the disassembly, cycles (absolute, frame, line) and
    registers.
*/
  if(TRACE_ENABLED(LOGSECTION_CPU))
  {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
    if(TRACE_MASK4 & TRACE_CONTROL_CPU_CYCLES)
    {
      TRACE_LOG("Cycles " PRICV " %d %d (%d)\n",ACT,FRAMECYCLES,(short)LINECYCLES,scan_y);
    }
    if(TRACE_MASK4 & TRACE_CONTROL_CPU_REGISTERS)
    {
      UPDATE_SR;
      TRACE_LOG("SR=%04X D0=%X D1=%X D2=%X D3=%X D4=%X D5=%X D6=%X D7=%X\n",
        SR,Cpu.r[0],Cpu.r[1],Cpu.r[2],Cpu.r[3],Cpu.r[4],Cpu.r[5],Cpu.r[6],Cpu.r[7]);
      TRACE_LOG("PC=%X A0=%X A1=%X A2=%X A3=%X A4=%X A5=%X A6=%X A7=%X\n",
        pc,Cpu.r[8],Cpu.r[9],Cpu.r[10],Cpu.r[11],Cpu.r[12],Cpu.r[13],Cpu.r[14],sp);
    }
#endif
    // makes a difference when there's a prefetch trick (only IR is correct) TODO
    if(PSWT)
      TRACE_LOG("(T) %X: %04X %04X %s\n",pc,IR,IRC,disa_d2(pc,IR).Text); // IRD not valid yet
    else
      TRACE_LOG("%X: %04X %04X %s\n",pc,IR,IRC,disa_d2(pc,IR).Text);
  }
#undef LOGSECTION
#define LOGSECTION LOGSECTION_TRACE
#ifdef DEBUG_BUILD
  if((PSWT) && !Debug.logsection_enabled[LOGSECTION_CPU] 
    && !logsection_enabled[LOGSECTION_CPU])
  {
    TRACE_LOG("(T) PC %X SR %04X VEC %X IRD %04X: %s\n",
      pc,SR,LPEEK(0x24),IR,disa_d2(pc,IR).Text);
  }
#endif
#undef LOGSECTION
#define LOGSECTION LOGSECTION_CPU
#endif//SSE_DEBUGGER
#endif//SSE_DEBUG
  if(PSWT)
  {
    Cpu.tpend=true; // hardware latch (=flag), used in cpu_op too
#if defined(SSE_DEBUGGER_OSD_CONTROL)
    if(OSD_MASK_CPU & OSD_CONTROL_CPUTRACE) 
      TRACE_OSD("TRACE %X",pc);
#elif defined(SSE_OSD_DEBUGINFO)
    if(OPTION_OSD_DEBUGINFO)
      TRACE_OSD2("T");
#endif
  }
  old_pc=pc;  
  IRD=IR; // like for pc, timing isn't 100% correct
  // generally but not always useful, could use macros to compute only when needed
  // at the start of the instruction TODO
  ry=(IRD&0x7);           // (PARAM_M) (EA)
  rx=((IRD&BITS_ba9)>>9); // (PARAM_N)
  pc+=2; // we do it here by convenience + the Debugger
  TRUE_PC=pc; // anyway
  /////////// CALL CPU EMU FUNCTION ///////////////
  m68k_call_table[IRD](); // big opcode function table
  //if(Blitter.Request)   Blitter_CheckRequest();
  // trace, before interrupt, cancelled by crash
  if(Cpu.tpend) 
  {
#if defined(SSE_STATS)
    Stats.nException[BOMBS_TRACE_EXCEPTION]++;
#endif
    M68K_UNSTOP; // STOP cancelled at once by trace
    Cpu.ProcessingState=TMC68000::EXCEPTION;
    CPU_BUS_IDLE(4); //  nn 4
    m68k_finish_exception(BOMBS_TRACE_EXCEPTION*4); //ns nS ns nV nv np n np 
    Cpu.tpend=false; // timing?
  }
  // check interrupt, looking back
  switch(check_ipl()) {
  case 0:
    break;
  case 6:
    mfp_interrupt(Mfp.NextIrq);
    break;
  case 4:
    VBLInterrupt();
    if(check_ipl()==6) // see below
      mfp_interrupt(Mfp.NextIrq);
    break;
  case 2:
    HBLInterrupt();
    // do it twice, a higher interrupt can trigger before the first instruction 
    // of a lower interrupt's handler: Suretrip fullscreen
    switch(check_ipl()) {
    case 0:
      break;
    case 6:
      mfp_interrupt(Mfp.NextIrq);
      break;
    case 4:
      VBLInterrupt();
      if(check_ipl()==6) // see above
        mfp_interrupt(Mfp.NextIrq);
      break;
    }
    break;
  }//sw
#ifdef DEBUG_BUILD
  if(ioaccess&(IOACCESS_DEBUG_MEM_WRITE_LOG|IOACCESS_DEBUG_MEM_READ_LOG))
  {
    DEBUG_CHECK_IOACCESS
  }
  CHECK_STOP_USER_MODE_NO_INTR // quite heavy!
  debug_first_instruction=0;
#endif
}


void m68kSetPC(MEM_ADDRESS ad,int count_timing) {
  pc=ad;             
#if defined(SSE_CPU_CHECK_PC)
  PC=ad;
#endif    
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  Debug.PseudoStackCheck(ad) ;
#endif
  if(count_timing)
  {
    PREFETCH_ONLY;
    if(count_timing==2) // exception
      CPU_BUS_IDLE(2); // two nonbus clock periods (dead cycles)
    CHECK_IPL;
    PREFETCH_FINAL;
  }
  else
  {
    BUS_MASK=(BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD);
    abus=(pc&0xfffffe);
    IR=m68k_fetch(pc);
    MEM_ADDRESS au=pc+2;
    abus=(au&0xfffffe);
    IRC=m68k_fetch(au);
  }
}


void m68kPerformRte() {
  MEM_ADDRESS pushed_return_address=m68k_lpeek(sp+2);
  // An Illegal routine could manipulate this value.
  SET_PC(pushed_return_address);
  SR=m68k_dpeek(sp);
  sp+=6;    
#ifndef SSE_LEAN_AND_MEAN
  SR&=SR_VALID_BITMASK;
#endif
  UPDATE_FLAGS;
  DETECT_CHANGE_TO_USER_MODE;         
}


#undef LOGSECTION


TMC68000::TMC68000() {
  Reset(true);
}


void TMC68000::Reset(bool Cold) {
  tpend=false; //guess so
  if(Cold)
  {
#if defined(SSE_DEBUG)    
    nExceptions=0;
#endif
    cycles_for_eclock=0;
    cycles0=0;
    eclock_sync_cycle=0;
  }
  ZeroMemory(&ipl_timing,sizeof(TIplTiming)*256);
  tvn_latch_time=0;
  ProcessingState=EXCEPTION;
}


/*  
"Enable (E)
This signal is the standard enable signal common to all M6800 Family peripheral
devices. A single period of clock E consists of 10 Cpu clock periods (six clocks
low, four clocks high). This signal is generated by an internal ring counter that may
come up in any state. (At power-on, it is impossible to guarantee phase relationship of E
to CLK.) The E signal is a free-running clock that runs regardless of the state of the
MPU bus."

  The ambition of this routine is to get correct timings
  for ACIA, HBL and VBL interrupts..
  It is based on this table by Nyh:
CPU-Clock  E-clock Keyboard read
  000000   00000.0     28
  160256   16025.6     20
  320512   32051.2     24
  480768   48076.8     20
  641024   64102.4     24
  801280   80128.0     28
  961536   96153.6     20
 1121792  112179.2     24
 1282048  128204.8     20
 1442304  144230.4     24

*/

#undef LOGSECTION
#define LOGSECTION LOGSECTION_INTERRUPTS

/*  No reason to have blocks of 4 cycles (0, 4 or 8), the real value can be 0,
    2, 4, 6 or 8.
    But never 9 apparently, so we have blocks of 2 cycles instead of 4 now ;)
    Notice for hbl, vbl interrupts, there's a bus access right before we come
    here.
    We could finally remove the previous WS1 hack (see v394) thanks to other 
    improvements.
*/

BYTE TMC68000::SyncEClock() {
  //ASSERT(eclock_sync_cycle<10);
  UpdateCyclesForEClock();
  BYTE cycles=(cycles_for_eclock+eclock_sync_cycle)%10;
  cycles&=0xFE;
  BYTE wait_states=8-cycles; //0 2 4 6 8
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
  if((TRACE_MASK2&TRACE_CONTROL_ECLOCK)) // sick of those E everywhere 
    FrameEvents.Add(scan_y,(short)LINECYCLES,'E',wait_states);
#endif
  //ASSERT(wait_states==0||wait_states==2||wait_states==4||wait_states==6||wait_states==8);
  return wait_states;
}


/*  We come here at each VBL and each time the e-clock is read,
    so cycles_for_eclock should never overflow or go negative.
*/

void TMC68000::UpdateCyclesForEClock() {
  COUNTER_VAR cycles1=ACT; // current CPU cycles (can be negative)
  COUNTER_VAR ncycles=cycles1-cycles0; // elapsed CPU cycles since last refresh
  cycles_for_eclock+=ncycles; // update counter for E-clock
  cycles_for_eclock%=(10*16); // remove high bits
  cycles0=cycles1; // record current CPU cycles
}
