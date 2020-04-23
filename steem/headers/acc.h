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

DOMAIN: Various
FILE: acc.h
DESCRIPTION: Declarations for completely random accessory functions.
---------------------------------------------------------------------------*/

#pragma once
#ifndef ACC_DECLA_H
#define ACC_DECLA_H

#include <conditions.h>
#include <stdio.h>
#include <easystr.h>
#include <dynamicarray.h>
#include "debug.h"

// SWAP_BIG_ENDIAN: swap big endian data if we use a little endian processor (such as Intel, AMD)
// SWAP_LITTLE_ENDIAN: swap little endian data if we use a big endian processor (such as ...?)
#if defined(BIG_ENDIAN_PROCESSOR)
#define SWAP_BIG_ENDIAN_WORD(val)
#define SWAP_BIG_ENDIAN_DWORD(val)
//#define SWAP_LITTLE_ENDIAN_WORD(val) val=_byteswap_ushort(val) // intrinsics
//#define SWAP_LITTLE_ENDIAN_DWORD(val) val=_byteswap_ulong(val)
#define BYTESWAP16(n) (((n&0xFF00)>>8)|((n&0x00FF)<<8))
#define BYTESWAP32(n) ((BYTESWAP16((n&0xFFFF0000)>>16))|((BYTESWAP16(n&0x0000FFFF))<<16))
#define BYTESWAP64(n) ((BYTESWAP32((n&0xFFFFFFFF00000000)>>32))|((BYTESWAP32(n&0x00000000FFFFFFFF))<<32))
#define SWAP_LITTLE_ENDIAN_WORD(val) val=BYTESWAP16(val)
#define SWAP_LITTLE_ENDIAN_DWORD(val) val=BYTESWAP32(val)
#elif defined(SSE_VC_INTRINSICS) // Intel = little endian
#define SWAP_BIG_ENDIAN_WORD(val) val=_byteswap_ushort(val)
#define SWAP_BIG_ENDIAN_DWORD(val) val=_byteswap_ulong(val)
#define SWAP_LITTLE_ENDIAN_WORD(val)
#define SWAP_LITTLE_ENDIAN_DWORD(val)
#else // Intel = little endian
#define BYTESWAP16(n) (((n&0xFF00)>>8)|((n&0x00FF)<<8))
#define BYTESWAP32(n) ((BYTESWAP16((n&0xFFFF0000)>>16))|((BYTESWAP16(n&0x0000FFFF))<<16))
#define BYTESWAP64(n) ((BYTESWAP32((n&0xFFFFFFFF00000000)>>32))|((BYTESWAP32(n&0x00000000FFFFFFFF))<<32))
#define SWAP_BIG_ENDIAN_WORD(val) val=BYTESWAP16(val)
#define SWAP_BIG_ENDIAN_DWORD(val) val=BYTESWAP32(val)
#define SWAP_LITTLE_ENDIAN_WORD(val)
#define SWAP_LITTLE_ENDIAN_DWORD(val)
#endif

#ifdef ENABLE_LOGFILE

#define DBG_LOG(s)  \
   {if(logsection_enabled[LOGSECTION]){ \
      if(!logging_suspended){            \
        log_write(s);                \
      }                               \
   }}

#define log_stack  \
   {if(logsection_enabled[LOGSECTION]){ \
      if(!logging_suspended){            \
        log_write_stack();                 \
      }                               \
   }}

void log_write(EasyStr);
void log_os_call(int trap);

#define log_to_section(section,s) if (logsection_enabled[section] && logging_suspended==0) log_write(s);
#define LOG_TO(section,s)  if (logsection_enabled[section] && logging_suspended==0) log_write(s);
void log_write_stack();

extern bool logging_suspended;
extern bool logsection_enabled[100];
void log_io_write(MEM_ADDRESS,BYTE);

#define CPU_INSTRUCTIONS_TO_LOG 10000
extern int log_cpu_count;
void stop_cpu_log();
Str scanline_cycle_log();

#if defined(SSE_DEBUGGER_FAKE_IO)
#define LOG_CPU \
    if(log_cpu_count) { \
      log_to_section(LOGSECTION_CPU,HEXSl(pc,6)+": "+disa_d2(pc)); \
      if(TRACE_MASK4&TRACE_CONTROL_CPU_LIMIT) \
        if((--log_cpu_count)==0) stop_cpu_log(); \
    }
#elif defined(DEBUG_BUILD)
#define LOG_CPU \
    if (log_cpu_count){ \
      log_to_section(LOGSECTION_CPU,HEXSl(pc,6)+": "+disa_d2(pc)); \
      if ((--log_cpu_count)==0) stop_cpu_log(); \
    }
#else
#define LOG_CPU
#endif

struct struct_logsection {
  char *Name;
  int Index;
};

extern struct_logsection logsections[NUM_LOGSECTIONS+8];
extern const char *name_of_mfp_interrupt[22];
extern const char* gemdos_calls[0x58];
extern const char* bios_calls[12];
extern const char* xbios_calls[40];
extern FILE *logfile;
extern EasyStr LogFileName;

#else

#define DBG_LOG(s)
#define log_stack ;
// s can be char* or EasyStr
#ifdef UNIX
#define log_write(s) printf(s);printf("\n");
#else
#define log_write(s)
#endif
#define log_io_write(a,b)
#define log_to_section(section,s)
#define LOG_TO(section,s)
#define log_write_stack() ;
#define LOG_CPU

#endif

#define log_DELETE_SOON(s) log_write(s);

EasyStr HEXSl(long,int);
#ifdef _WIN64
EasyStr HEXSll(long long n,int ln);
#endif

#if defined(SSE_VC_INTRINSICS)
extern int count_bits_set_in_word1(unsigned short w);
#else
extern int count_bits_set_in_word(unsigned short w);
#endif

EasyStr read_string_from_memory(MEM_ADDRESS,int);
MEM_ADDRESS write_string_to_memory(MEM_ADDRESS,char*);
void acc_parse_search_string(Str,DynamicArray<BYTE>&,bool&);
MEM_ADDRESS acc_find_bytes(DynamicArray<BYTE> &,bool,MEM_ADDRESS,int);
long colour_convert(int,int,int);


#ifdef WIN32
BOOL SetClipboardText(LPCTSTR pszText);
HMODULE SteemLoadLibrary(LPCSTR lpLibFileName);
int get_text_width(char*t);
#endif

#if !defined(SSE_NO_UPDATE)
EasyStr time_or_times(int n);
#endif

EasyStr DirID_to_text(int ID,bool st_key);

bool has_extension_list(char*,char*,...);
bool has_extension(char*,char*);
bool MatchesAnyString(char *,char *,...);
bool MatchesAnyString_I(char *,char *,...);

EasyStr GetEXEDir();
EasyStr GetCurrentDir();
EasyStr GetEXEFileName();


#endif//ACC_DECLA_H
