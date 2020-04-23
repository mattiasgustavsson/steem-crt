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
FILE: emulator.h
DESCRIPTION: Declarations for miscellaneous core emulator functions and
objects.
struct TAgenda, TRp5c15, TStats, TMegaSte
---------------------------------------------------------------------------*/

#pragma once
#ifndef EMULATOR_DECLA_H
#define EMULATOR_DECLA_H

#include <dynamicarray.h>
#include "conditions.h"
#include "blitter.h"
#include "steemh.h"
#include "interface_caps.h"
#include "disk_ghost.h"
#include "disk_stw.h"
#include "disk_scp.h"
#include "disk_hfe.h"
#include "hd_acsi.h"
#if defined(SSE_STATS_CPU)
#include <CpuUsage/CpuUsage.h>
#endif



#define STRS(a) itoa((a),d2_t_buf,10)
#define HEXS(a) itoa((a),d2_t_buf,16)

#define DOT_B 0
#define DOT_W 1
#define DOT_L 2

#define BIT_0 0x1
#define BIT_1 0x2
#define BIT_2 0x4
#define BIT_3 0x8
#define BIT_4 0x10
#define BIT_5 0x20
#define BIT_6 0x40
#define BIT_7 0x80
#define BIT_8 0x100
#define BIT_9 0x200
#define BIT_a 0x400
#define BIT_b 0x800
#define BIT_c 0x1000
#define BIT_d 0x2000
#define BIT_e 0x4000
#define BIT_f 0x8000
#define BIT_10 0x400
#define BIT_11 0x800
#define BIT_12 0x1000
#define BIT_13 0x2000
#define BIT_14 0x4000
#define BIT_15 0x8000

#define BIT_16 0x00010000
#define BIT_17 0x00020000
#define BIT_18 0x00040000
#define BIT_19 0x00080000
#define BIT_20 0x00100000
#define BIT_21 0x00200000
#define BIT_22 0x00400000
#define BIT_23 0x00800000
#define BIT_24 0x01000000
#define BIT_25 0x02000000
#define BIT_26 0x04000000
#define BIT_27 0x08000000
#define BIT_28 0x10000000
#define BIT_29 0x20000000
#define BIT_30 0x40000000
#define BIT_31 0x80000000

#define BITS_ba9 0xe00
#define BITS_876 0x1c0
#define BITS_543 0x038

#define BITS_ba9_000 0x000
#define BITS_ba9_001 0x200
#define BITS_ba9_010 0x400
#define BITS_ba9_011 0x600
#define BITS_ba9_100 0x800
#define BITS_ba9_101 0xa00
#define BITS_ba9_110 0xc00
#define BITS_ba9_111 0xe00

#define BITS_876_000 0x000
#define BITS_876_001 0x040
#define BITS_876_010 0x080
#define BITS_876_011 0x0c0
#define BITS_876_100 0x100
#define BITS_876_101 0x140
#define BITS_876_110 0x180
#define BITS_876_111 0x1c0

#define BITS_543_000 0x00
#define BITS_543_001 0x08
#define BITS_543_010 0x10
#define BITS_543_011 0x18
#define BITS_543_100 0x20
#define BITS_543_101 0x28
#define BITS_543_110 0x30
#define BITS_543_111 0x38

#define B6_000000 0
#define B6_000001 1
#define B6_000010 2
#define B6_000011 3
#define B6_000100 4
#define B6_000101 5
#define B6_000110 6
#define B6_000111 7
#define B6_001000 8
#define B6_001001 9
#define B6_001010 10
#define B6_001011 11
#define B6_001100 12
#define B6_001101 13
#define B6_001110 14
#define B6_001111 15
#define B6_010000 16
#define B6_010001 17
#define B6_010010 18
#define B6_010011 19
#define B6_010100 20
#define B6_010101 21
#define B6_010110 22
#define B6_010111 23
#define B6_011000 24
#define B6_011001 25
#define B6_011010 26
#define B6_011011 27
#define B6_011100 28
#define B6_011101 29
#define B6_011110 30
#define B6_011111 31
#define B6_100000 32
#define B6_100001 33
#define B6_100010 34
#define B6_100011 35
#define B6_100100 36
#define B6_100101 37
#define B6_100110 38
#define B6_100111 39
#define B6_101000 40
#define B6_101001 41
#define B6_101010 42
#define B6_101011 43
#define B6_101100 44
#define B6_101101 45
#define B6_101110 46
#define B6_101111 47
#define B6_110000 48
#define B6_110001 49
#define B6_110010 50
#define B6_110011 51
#define B6_110100 52
#define B6_110101 53
#define B6_110110 54
#define B6_110111 55
#define B6_111000 56
#define B6_111001 57
#define B6_111010 58
#define B6_111011 59
#define B6_111100 60
#define B6_111101 61
#define B6_111110 62
#define B6_111111 63

#define BTST(n,b) (bool)((n>>b)&1)

#define BTST0(n) ((n&1)!=0)
#define BTST1(n) ((n&2)!=0)
#define BTST2(n) ((n&4)!=0)
#define BTST3(n) ((n&8)!=0)
#define BTST4(n) ((n&16)!=0)
#define BTST5(n) ((n&32)!=0)
#define BTST6(n) ((n&64)!=0)
#define BTST7(n) ((n&128)!=0)
#define BTST8(n) ((n&256)!=0)
#define BTST9(n) ((n&512)!=0)
#define BTSTa(n) ((n&1024)!=0)
#define BTSTb(n) ((n&2048)!=0)
#define BTSTc(n) ((n&4096)!=0)
#define BTSTd(n) ((n&8192)!=0)
#define BTSTe(n) ((n&0x4000)!=0)
#define BTSTf(n) ((n&32768)!=0)


#define MSB_B BYTE(0x80)
#define MSB_W (WORD)0x8000
#define MSB_L (DWORD)0x80000000
#define BYTE_00_TO_256(x) ( (int) ((unsigned char) (( (unsigned char)x )-1))  +1 )

#ifndef DEBUG_BUILD
#define DEBUG_CHECK_WRITE_B(ad) 
#define DEBUG_CHECK_WRITE_W(ad)
#define DEBUG_CHECK_WRITE_L(ad)
#define DEBUG_CHECK_READ_B(ad)
#define DEBUG_CHECK_READ_W(ad)
#define DEBUG_CHECK_READ_L(ad)

#define DEBUG_CHECK_WRITE_IO_B(ad,v)
#define DEBUG_CHECK_WRITE_IO_W(ad,v)
#define DEBUG_CHECK_WRITE_IO_L(ad,v)
#define DEBUG_CHECK_READ_IO_B(ad)
#define DEBUG_CHECK_READ_IO_W(ad)
#define DEBUG_CHECK_READ_IO_L(ad)
#endif

#define MEM_EXTRA_BYTES 320

#define STEM_MODE_CPU 0
#define STEM_MODE_D2 1
#define STEM_MODE_INSPECT 2

#if defined(PEEK_RANGE_TEST) && defined(DEBUG_BUILD)

void RangeError(DWORD &ad,DWORD hi_ad);

// Have to allow pointer to last byte to be returned for lpDPEEK (SET_PC)

#define RANGE_CHECK_MESSAGE(hi,len,hiadd) if (ad<0 || (ad+(len))>=((hi)+(hiadd))) RangeError(ad,hi-len)

BYTE& PEEK; //(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,0,0);return *LPBYTE(Mem_End_minus_1-ad); }
WORD& DPEEK; //(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,1,0);return *LPWORD(Mem_End_minus_2-ad); }
DWORD& LPEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,3,0);return *LPDWORD(Mem_End_minus_4-ad); }
BYTE* lpPEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,0,0);return LPBYTE(Mem_End_minus_1-ad); }
WORD* lpDPEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,1,0);return LPWORD(Mem_End_minus_2-ad); }
DWORD* lpLPEEK(DWORD ad; //){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,3,0);return LPDWORD(Mem_End_minus_4-ad); }

BYTE& ROM_PEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(tos_len,0,0);return *LPBYTE(Rom_End_minus_1-ad); }
WORD& ROM_DPEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(tos_len,1,0);return *LPWORD(Rom_End_minus_2-ad); }
DWORD& ROM_LPEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(tos_len,3,0);return *LPDWORD(Rom_End_minus_4-ad); }
BYTE* lpROM_PEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(tos_len,0,0);return LPBYTE(Rom_End_minus_1-ad); }
WORD* lpROM_DPEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(tos_len,1,2);return LPWORD(Rom_End_minus_2-ad); }
DWORD* lpROM_LPEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(tos_len,3,0);return LPDWORD(Rom_End_minus_4-ad); }

BYTE& CART_PEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(128*1024,0,0);return *LPBYTE(Cart_End_minus_1-ad); }
WORD& CART_DPEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(128*1024,1,0);return *LPWORD(Cart_End_minus_2-ad); }
DWORD& CART_LPEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(128*1024,3,0);return *LPDWORD(Cart_End_minus_4-ad); }
BYTE* lpCART_PEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(128*1024,0,0);return LPBYTE(Cart_End_minus_1-ad); }
WORD* lpCART_DPEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(128*1024,1,2);return LPWORD(Cart_End_minus_2-ad); }
DWORD* lpCART_LPEEK(DWORD ad); //{ RANGE_CHECK_MESSAGE(128*1024,3,0);return LPDWORD(Cart_End_minus_4-ad); }

#define PAL_DPEEK(l)   *(WORD*)(palette_exec_mem+64+PAL_EXTRA_BYTES-2-(l))
#define lpPAL_DPEEK(l) (WORD*)(palette_exec_mem+64+PAL_EXTRA_BYTES-2-(l))

#else

#ifndef BIG_ENDIAN_PROCESSOR
//little endian version (x86,x64...)

#define PEEK(l)    *(BYTE*)(Mem_End_minus_1-(l))
#define DPEEK(l)   *(WORD*)(Mem_End_minus_2-(l))
#define DPEEK(l)   *(WORD*)(Mem_End_minus_2-(l))
#define LPEEK(l)   *(DWORD*)(Mem_End_minus_4-(l))
#define lpPEEK(l)  (BYTE*)(Mem_End_minus_1-(l))
#define lpDPEEK(l) (WORD*)(Mem_End_minus_2-(l))
#define lpLPEEK(l) (DWORD*)(Mem_End_minus_4-(l))

#define ROM_PEEK(l)    *(BYTE*)(Rom_End_minus_1-(l))
#define ROM_DPEEK(l)   *(WORD*)(Rom_End_minus_2-(l))
#define ROM_LPEEK(l)   *(DWORD*)(Rom_End_minus_4-(l))
#define lpROM_PEEK(l)  (BYTE*)(Rom_End_minus_1-(l))
#define lpROM_DPEEK(l) (WORD*)(Rom_End_minus_2-(l))
#define lpROM_LPEEK(l) (DWORD*)(Rom_End_minus_4-(l))

#define CART_PEEK(l)    *(BYTE*)(Cart_End_minus_1-(l))
#define CART_DPEEK(l)   *(WORD*)(Cart_End_minus_2-(l))
#define CART_LPEEK(l)   *(DWORD*)(Cart_End_minus_4-(l))
#define lpCART_PEEK(l)  (BYTE*)(Cart_End_minus_1-(l))
#define lpCART_DPEEK(l) (WORD*)(Cart_End_minus_2-(l))
#define lpCART_LPEEK(l) (DWORD*)(Cart_End_minus_4-(l))

#define PAL_DPEEK(l)   *(WORD*)(palette_exec_mem+64+PAL_EXTRA_BYTES-2-(l))
#define lpPAL_DPEEK(l) (WORD*)(palette_exec_mem+64+PAL_EXTRA_BYTES-2-(l))

#else

#define PEEK(l)    *(BYTE*)(STMem+(l))
#define DPEEK(l)   *(WORD*)(STMem+(l))
#define LPEEK(l)   *(DWORD*)(STMem+(l))
#define lpPEEK(l)  (BYTE*)(STMem+(l))
#define lpDPEEK(l) (WORD*)(STMem+(l))
#define lpLPEEK(l) (DWORD*)(STMem+(l))

#define ROM_PEEK(l)    *(BYTE*)(STRom+(l))
#define ROM_DPEEK(l)   *(WORD*)(STRom+(l))
#define ROM_LPEEK(l)   *(DWORD*)(STRom+(l))
#define lpROM_PEEK(l)  (BYTE*)(STRom+(l))
#define lpROM_DPEEK(l) (WORD*)(STRom+(l))
#define lpROM_LPEEK(l) (DWORD*)(STRom+(l))

#define CART_PEEK(l)    *(BYTE*)(cart+(l))
#define CART_DPEEK(l)   *(WORD*)(cart+(l))
#define CART_LPEEK(l)   *(DWORD*)(cart+(l))
#define lpCART_PEEK(l)  (BYTE*)(cart+(l))
#define lpCART_DPEEK(l) (WORD*)(cart+(l))
#define lpCART_LPEEK(l) (DWORD*)(cart+(l))

#define PAL_DPEEK(l)   *(WORD*)(palette_exec_mem+(l))
#define lpPAL_DPEEK(l) (WORD*)(palette_exec_mem+(l))

#endif

#endif

#ifdef WIN32
#if !defined(VC_BUILD) && !defined(MINGW_BUILD)
extern void _RTLENTRY __int__(int);
#define INTR(i) __int__(i)
#endif
#endif


/////////
// Bus //
/////////

// this is Steem SSE arbitrary internal coding for bus access
#define BUS_MASK_HIBYTE BIT_4
#define BUS_MASK_LOBYTE BIT_3
#define BUS_MASK_WORD (BUS_MASK_HIBYTE|BUS_MASK_LOBYTE)
#define BUS_MASK_FETCH BIT_2
#define BUS_MASK_WRITE BIT_1
#define BUS_MASK_ACCESS BIT_0
#define BUS_MASK bus_mask
// fetch:
//  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_FETCH|BUS_MASK_WORD;
// read byte:
// BUS_MASK=(iabus&1)? (BUS_MASK_ACCESS|BUS_MASK_LOBYTE) : (BUS_MASK_ACCESS|BUS_MASK_HIBYTE);
// read word
//  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WORD;
// write byte
//  BUS_MASK=(iabus&1)? (BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_LOBYTE) : (BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_HIBYTE);
// write word
//  BUS_MASK=BUS_MASK_ACCESS|BUS_MASK_WRITE|BUS_MASK_WORD;

#if defined(SSE_VID_STVL1)
extern DU16 &udbus;
extern WORD &dbus;
extern BYTE &dbush;
extern BYTE &dbusl;
extern MEM_ADDRESS& abus;
#else
extern DU16 udbus;
extern MEM_ADDRESS abus;
extern WORD &dbus;
extern BYTE &dbusl, &dbush;
#endif
#if defined(SSE_VID_STVL1)
extern BYTE &bus_mask;
#else
extern BYTE bus_mask;
#endif

int milliseconds_to_hbl(int);

extern COUNTER_VAR cpu_timer;
extern int cpu_cycles;
extern int ioaccess;
extern unsigned long hbl_count;


#define COLOUR_MONITOR (SSEConfig.ColourMonitor)
#define MONO (!(SSEConfig.ColourMonitor))

#define ON_RTE_RTE 0
#define ON_RTE_STEMDOS 1
#define ON_RTE_LINE_A 2
#define ON_RTE_EMHACK 3
#define ON_RTE_DONE_MALLOC_FOR_EM 4
#define ON_RTE_STOP 400
#define ON_RTS_STOP 401


#define HBLS_PER_SECOND_AVE 15700 // Average between 50 and 60hz
#define HBLS_PER_SECOND_MONO (501.0*71.42857)

extern int interrupt_depth;

extern DWORD em_width,em_height;
extern BYTE em_planes;
#ifdef NO_CRAZY_MONITOR
extern const bool extended_monitor;
#else
extern BYTE extended_monitor;
#endif
extern DWORD n_cpu_cycles_per_second,new_n_cpu_cycles_per_second;
extern DWORD n_millions_cycles_per_sec;
extern "C" double cpu_cycles_multiplier; // used by 6301
extern int on_rte;
extern int on_rte_interrupt_depth;

extern short cpu_cycles_from_hbl_to_timer_b;
extern BYTE video_freq_idx;
extern int video_first_draw_line;
extern int video_last_draw_line;
extern BYTE screen_res;
extern short scan_y;
extern BYTE video_mixed_output;
extern MEM_ADDRESS old_pc;
extern MEM_ADDRESS pc_high_byte;
extern BYTE screen_res_at_start_of_vbl;
extern BYTE video_freq_at_start_of_vbl;
extern COUNTER_VAR cpu_time_of_last_vbl,shifter_cycle_base;
extern COUNTER_VAR cpu_timer_at_start_of_hbl;

#define OS_CALL 0
#define OS_NO_CALL true

void init_timings();

void call_a000();
WIN_ONLY( extern CRITICAL_SECTION agenda_cs; )
void ASMCALL emudetect_falcon_draw_scanline(int,int,int,int);
void emudetect_falcon_palette_convert(int);
void emudetect_init();
void emudetect_reset();
extern bool emudetect_called,emudetect_write_logs_to_printer,
  emudetect_overscans_fixed;
#define EMUD_FALC_MODE_OFF 0
#define EMUD_FALC_MODE_8BIT 1
#define EMUD_FALC_MODE_16BIT 2
#if defined(SSE_NO_FALCONMODE)
extern const BYTE emudetect_falcon_mode;
#else
extern BYTE emudetect_falcon_mode;
#endif
extern BYTE emudetect_falcon_mode_size;
extern bool emudetect_falcon_extra_height;

extern DynamicArray<DWORD> emudetect_falcon_stpal;
extern DynamicArray<DWORD> emudetect_falcon_pcpal;


#ifndef NO_CRAZY_MONITOR
void extended_monitor_hack();
#endif

#define MAX_AGENDA_LENGTH 32

typedef void AGENDAPROC(int);
typedef AGENDAPROC* LPAGENDAPROC;

struct TAgenda{
  LPAGENDAPROC perform;
  unsigned long time;
  int param;
};

extern TAgenda agenda[MAX_AGENDA_LENGTH];
extern int agenda_length;
extern unsigned long agenda_next_time;

void agenda_add(LPAGENDAPROC action,int pause,int param);
void agenda_delete(LPAGENDAPROC job);
int agenda_get_queue_pos(LPAGENDAPROC job);

extern MEM_ADDRESS on_rte_return_address;

// This list is used to reinit the agendas after loading a snapshot
// add any new agendas to the end of the list, replace old agendas
// with NULL.
//extern LPAGENDAPROC agenda_list[]; // BCC don't like that
extern LPAGENDAPROC agenda_list[15+4+1+1+1+1];


#ifndef NO_CRAZY_MONITOR
extern int aes_calls_since_reset; // unused but saved in snapshot
extern long save_r[16];
extern MEM_ADDRESS line_a_base;
extern MEM_ADDRESS vdi_intout;
#endif

extern const BYTE scanlines_above_screen[4];
extern const WORD scanline_time_in_cpu_cycles_8mhz[4];
extern int scanline_time_in_cpu_cycles[4];
extern const WORD hbl_per_second[4]; 

// Steem can still be confused if frequency changes right after a disk command
// TODO: use events for disk, or have optional auto conversion from/to STW for ADAT
#define HBL_PER_SECOND (hbl_per_second[video_freq_idx])
#define SCANLINE_TIME_IN_CPU_CYCLES_50HZ 512
#define SCANLINE_TIME_IN_CPU_CYCLES_60HZ 508
#define SCANLINE_TIME_IN_CPU_CYCLES_70HZ 224
#define CYCLES_FOR_VERTICAL_RETURN_IN_50HZ 444
#define CYCLES_FOR_VERTICAL_RETURN_IN_60HZ 444
#define CYCLES_FOR_VERTICAL_RETURN_IN_70HZ 200
#define CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN (84) //SS 84 = 56+28

#define CALC_CYCLES_FROM_HBL_TO_TIMER_B(freq) \
  switch (freq){ \
    case MONO_HZ: cpu_cycles_from_hbl_to_timer_b=192;break; \
    case 60: cpu_cycles_from_hbl_to_timer_b\
      =(CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN+320-4);break; \
    default: cpu_cycles_from_hbl_to_timer_b\
      =(CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN+320); \
}

 // RTC of the Mega ST
struct TRp5c15 {
  BYTE reg[2][16]; // 2 banks, really 4bit each (BCD nibbles)
  WORD Read(MEM_ADDRESS addr);
  void Write(MEM_ADDRESS addr, BYTE io_src_b);
};
extern TRp5c15 MegaRtc;

#if defined(SSE_DISK_CAPS)
extern TCaps Caps;
#endif

#if defined(SSE_DISK_GHOST)
extern TGhostDisk GhostDisk[2];
#endif

extern TImageSTW ImageSTW[3];
#if defined(SSE_DISK_STW)
#define IMAGE_STW (FloppyDrive[DRIVE].ImageType.Extension==EXT_STW)
#else
#define IMAGE_STW 0
#endif

extern TImageSCP ImageSCP[3];
#if defined(SSE_DISK_SCP)
#define IMAGE_SCP (FloppyDrive[DRIVE].ImageType.Extension==EXT_SCP)
#else
#define IMAGE_SCP 0
#endif

extern TImageHFE ImageHFE[3];
#if defined(SSE_DISK_HFE)
#define IMAGE_HFE (FloppyDrive[DRIVE].ImageType.Extension==EXT_HFE)
#else
#define IMAGE_HFE 0
#endif

extern TAcsiHdc AcsiHdc[TAcsiHdc::MAX_ACSI_DEVICES];

/*  The overscan mask is an imitation of Hatari's BorderMask.
    Each trick has a dedicated bit, to set it we '|' it, to check it
    we '&' it. Each value is the previous one x2.
    Note max 32 bit = $80 000 000
*/
enum EOverscanMask {
 TRICK_LINE_PLUS_26=0x01, 
 TRICK_LINE_PLUS_2=0x02, 
 TRICK_HIRES_OVERSCAN=2, // used for software and hw overscan in monochrome
 TRICK_LINE_MINUS_106=0x04,
 TRICK_LINE_MINUS_2=0x08,
 TRICK_LINE_PLUS_44=0x10,
 TRICK_4BIT_SCROLL=0x20,
 TRICK_OVERSCAN_MED_RES=0x40,
 TRICK_BLACK_LINE=0x80,
 TRICK_TOP_OVERSCAN=0x100,
 TRICK_BOTTOM_OVERSCAN=0x200,
 TRICK_BOTTOM_OVERSCAN_60HZ=0x400,
 TRICK_LINE_PLUS_20=0x800,
 TRICK_0BYTE_LINE=0x1000,
 TRICK_STABILISER=0x2000,
 TRICK_80BYTE_LINE=0x4000, // don't assume a "no trick" colour line = 160byte
 TRICK_UNSTABLE=0x08000,
 TRICK_LINE_PLUS_24=0x10000,
 TRICK_LINE_PLUS_4=0x20000,
 TRICK_LINE_PLUS_6=0x40000,
 TRICK_NEO=0x80000//tests
};

#if defined(SSE_STATS)

struct TStats { 
  // used to record and display information about what the program is doing
  enum EStats {IKBD_22=1,EMU_DETECT=2,WRITE_DISK=4}; 
  // FUNCTIONS
  TStats() {
    Restore();
  }
  void Restore() {
    ZeroMemory(this,sizeof(TStats)); // simple
  }
  void Report();
  // DATA
#if defined(SSE_STATS_CPU)
  CpuUsage myCpuUsage; // Windows 2000 dependency
#endif
  COUNTER_VAR nFrame,nTimerb,nSlowdown;
  COUNTER_VAR nException[12],nStop;
  COUNTER_VAR nHbi,nVbi,nMfpIrq[16],nTrap[16],nSpurious;
  COUNTER_VAR nBlit,nBlitT,nBlith,nExtendedPal,nExtendedPalT,nHscroll,nVscroll;
  COUNTER_VAR nScreensplit;
  COUNTER_VAR nReadvc;
  COUNTER_VAR nPsgSound,nMicrowire,nMicrowireT;
  COUNTER_VAR nKeyIn,nJoy0,nJoy1,nClick0,nClick1,nMousex,nMousey;
  COUNTER_VAR nHdsector,nPrg;
  COUNTER_VAR nGemdos,nBios,nXbios,nVdi,nAes;
  COUNTER_VAR nGemdosi,nBiosi,nXbiosi,nVdii;
  COUNTER_VAR nPorti[3],nPorto[3],nMfpTimeout[4];
  DWORD nPal,nTimerbtick,nBlit1,nHbi1,nReadvc1,nScreensplit1; // per frame
  DWORD nLinePlus16; 
  DWORD mskSpecial,mskDigitalSound,mskOverscan,mskOverscan1;
  DWORD fTimer[4];
  int run_time;
  WORD nFps,tCpuUsage;
  WORD boot_checksum[2][2];
  BYTE nSide[2],nTrack[2],nSector[2],nSector2[2];
};

extern TStats Stats;

#endif

#if defined(SSE_MEGASTE)

struct TMemCache {
  TMemCache();
  ~TMemCache();
  void Ready(bool enabling);
  bool Check(MEM_ADDRESS ad,WORD &data);
  void Add(MEM_ADDRESS ad);
  void Reset();
  MEM_ADDRESS *pAdlist;
  bool *pIsCached; // big table
  int iPos;
/*
-------+-----+-----------------------------------------------------+----------
##############Mega STe Cache/Processor Control                     ###########
-------+-----+-----------------------------------------------------+----------
$FF8E21|byte |Mega STe Cache/Processor Control           BIT 15-1 0|R/W (MSTe)
       |     |Cache enable lines (set all to 1 to enable) -----'  ||
       |     |CPU Speed (0 - 8mhz, 1 - 16mhz) --------------------'|
-------+-----+-----------------------------------------------------+----------
*/
  BYTE ScuReg;
};


struct TMegaSte {
  TMemCache MemCache;
  // For now we regroup other Mega STE specific hardware registers in one struct
  BYTE FdHd; // $ffff860e.w (0 = 8MHz = DD/ 3 = 16MHz = HD)
/*
-------+-----+-----------------------------------------------------+----------
##############Zilog 8530 SCC (MSTe/TT/F030)                        ###########
-------+-----+-----------------------------------------------------+----------
$FF8C81|byte |Channel A - Control Register                         |R/W  (SCC)
$FF8C83|byte |Channel A - Data Register                            |R/W  (SCC)
$FF8C85|byte |Channel B - Control Register                         |R/W  (SCC)
$FF8C87|byte |Channel B - Data Register                            |R/W  (SCC)
*/
  BYTE Scc[4]; //TODO
/*
-------+-----+-----------------------------------------------------+----------
##############VME Bus System Control Unit (MSTe/TT)                ###########
-------+-----+-----------------------------------------------------+----------
$FF8E01|byte |VME sys_mask                      BIT 7 6 5 4 . 2 1 .|R/W  (VME)
$FF8E03|byte |VME sys_stat                      BIT 7 6 5 4 . 2 1 .|R    (VME)
       |     |_SYSFAIL in VMEBUS -------------------' | | |   | |  |program
       |     |MFP ------------------------------------' | |   | |  |autovec
       |     |SCC --------------------------------------' |   | |  |autovec
       |     |VSYNC --------------------------------------'   | |  |program
       |     |HSYNC ------------------------------------------' |  |program
       |     |System software INT ------------------------------'  |program
       |     +-----------------------------------------------------+
       |     |Reading sys_mask resets pending int-bits in sys_stat,|
       |     |so read sys_stat first.                              |
-------+-----+-----------------------------------------------------+----------
$FF8E05|byte |VME sys_int                                     BIT 0|R/W  (VME)
       |     |Setting bit 0 to 1 forces an INT of level 1. INT must|Vector $64
       |     |be enabled in sys_mask to use it.                    |
-------+-----+-----------------------------------------------------+----------
$FF8E0D|byte |VME vme_mask                      BIT 7 6 5 4 3 2 1 .|R/W  (VME)
$FF8E0F|byte |VME vme_stat                      BIT 7 6 5 4 3 2 1 .|R    (VME)
       |     |_IRQ7 from VMEBUS --------------------' | | | | | |  |program
       |     |_IRQ6 from VMEBUS/MFP ------------------' | | | | |  |program
       |     |_IRQ5 from VMEBUS/SCC --------------------' | | | |  |program
       |     |_IRQ4 from VMEBUS --------------------------' | | |  |program
       |     |_IRQ3 from VMEBUS/soft -----------------------' | |  |prog/autov
       |     |_IRQ2 from VMEBUS ------------------------------' |  |program
       |     |_IRQ1 from VMEBUS --------------------------------'  |program
       |     +-----------------------------------------------------+
       |     |MFP-int and SCC-int are hardwired to the VME-BUS-ints|
       |     |(or'ed). Reading vme_mask resets pending int-bits in |
       |     |vme_stat, so read vme_stat first.                    |
*/
  BYTE VmeSysMask; //TODO
  BYTE VmeSysStat;
  BYTE VmeSysInt;
  BYTE VmeMask;
  BYTE VmeStat;

/*
-------+-----+-----------------------------------------------------+----------
##############Floating Point Coprocessor (CIR Interface in MSTe)   ###########
-------+-----+-----------------------------------------------------+----------
$FFFA40|word |FP_Stat    Response-Register                         |??? (MSTe)
$FFFA42|word |FP_Ctl     Control-Register                          |??? (MSTe)
$FFFA44|word |FP_Save    Save-Register                             |??? (MSTe)
$FFFA46|word |FP_Restor  Restore-Register                          |??? (MSTe)
$FFFA48|word |                                                     |??? (MSTe)
$FFFA4A|word |FP_Cmd     Command-Register                          |??? (MSTe)
$FFFA4E|word |FP_Ccr     Condition-Code-Register                   |??? (MSTe)
$FFFA50|long |FP_Op      Operand-Register                          |??? (MSTe)
$FFFA54|word |FP_Selct   Register Select                           |??? (MSTe)
$FFFA58|long |FP_Iadr    Instruction Address                       |??? (MSTe)
$FFFA5C|long |           Operand Address                           |??? (MSTe)
*/
   //TODO
};

#endif//#if defined(SSE_MEGASTE)

#endif// EMULATOR_DECLA_H
