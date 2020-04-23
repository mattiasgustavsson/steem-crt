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
FILE: parameters.h
DESCRIPTION: Some parameters for emulation and the rest.
---------------------------------------------------------------------------*/

#pragma once
#ifndef SSEPARAMETERS_H
#define SSEPARAMETERS_H

#if defined(__cplusplus)
#include <conditions.h>
#endif


/////////////
// BLITTER //
/////////////

// possibly effect of LINEW, which delays Blitter start until end of Write bus
// cycle - could be different if no CPU bus access at once?
#define BLITTER_LATCH_LATENCY (4) 
// we can see the delay on schematics but it's hard to follow - 4 clocks seems 
// to be most compatible
#define BLITTER_START_WAIT (4)
#define BLITTER_END_WAIT (4)


/////////
// CPU //
/////////

extern const int EIGHT_MILLION; //8000000
#define CPU_MAX_HERTZ (2000000000) // 2ghz

/*
The master clock crystal and derived CPU clock table is:
PAL (all variants)       32.084988   8.021247
STE same as STF, yet it sometimes looks like the CPU/MFP crystal ratio is
different!
NTSC (pre-STE)           32.0424     8.0106
Mega ST                  32.04245    8.0106125
NTSC (STE)               32.215905   8.053976
Peritel (STE) (as PAL)   32.084988   8.021247
Some STFs                32.02480    8.0071
*/

#define CPU_CLOCK_STF_NTSC   (8010600)
#define CPU_CLOCK_STF_PAL    (8021247) // precise
#define CPU_CLOCK_MEGA_ST    (8010613) // rounded
#define CPU_CLOCK_STE_PAL    (CPU_CLOCK_STF_PAL)
#define CPU_CLOCK_STE_NTSC   (8053976)

#define DBI_DELAY (1) // ~8mhz; notice we use < for comparison, not <=


///////////
// DEBUG //
///////////

#if defined(SSE_UNIX)
#define SSE_TRACE_FILE_NAME "./TRACE.txt"
#else
#define SSE_TRACE_FILE_NAME "TRACE.txt"
#endif
#define TRACE_MAX_WRITES 200000 // to avoid too big file (debugger-only)


#if defined(SSE_DEBUGGER_FAKE_IO)
#define FAKE_IO_START 0xfffb00
#define FAKE_IO_LENGTH 64*2 // in bytes
#define FAKE_IO_END (FAKE_IO_START+FAKE_IO_LENGTH-2) // starting address of last one
#define STR_FAKE_IO_CONTROL "Control mask browser"
#endif

#if defined(SSE_DEBUGGER_FRAME_REPORT)
#if defined(SSE_UNIX)
#define FRAME_REPORT_FILENAME "./FrameReport.txt"
#else
#define FRAME_REPORT_FILENAME "FrameReport.txt"
#endif
#endif


//////////
// DISK //
//////////

#define DISK_BYTES_PER_TRACK (6256)
/*  #bytes/track
    The value generally seen is 6250.
    The value for 11 sectors is 6256. It's possible if the clock is higher than
    8mhz, which is the case on the ST.
*/

#define DISK_11SEC_INTERLEAVE 6


///////////
// DRIVE //
///////////

#define DRIVE_RPM 300
#define DRIVE_MAX_CYL 83
#define DRIVE_FAST_CYCLES_PER_BYTE 4
#define DRIVE_FAST_IP_MULTIPLIER 8
#define DRIVE_SOUND_BUZZ_THRESHOLD 2 // 2 min


///////////
// FILES //
///////////

#define ACSI_HD_DIR "ACSI"
#define SSE_VID_RECORD_AVI_FILENAME "SteemVideo.avi"
#define DISK_HFE_BOOT_FILENAME "HFE_boot.bin"
#define DISK_IMAGE_DB "disk image list.txt"
#define HD6301_ROM_FILENAME "HD6301V1ST.img"
#define DRIVE_SOUND_DIRECTORY "DriveSound" // default
#define YM2149_FIXED_VOL_FILENAME "ym2149_fixed_vol.bin"
#define PASTI_DLL "pasti"
#define SSE_DISK_CAPS_PLUGIN_FILE "CAPSImg"
#define ARCHIVEACCESS_DLL "ArchiveAccess"
#define UNZIP_DLL "unzipd32" 
#define SSE_PLUGIN_DIR2 "plugins"
#ifdef SSE_X64
#define SSE_PLUGIN_DIR1 "plugins64"
#define UNRAR_DLL "unrar64"
#if defined(SSE_DEBUG)
#define VIDEO_LOGIC_DLL "stvl64d"
#else
#define VIDEO_LOGIC_DLL "stvl64"
#endif
#else
#define SSE_PLUGIN_DIR1 "plugins32"
#define UNRAR_DLL "unrar" 
#if defined(SSE_DEBUG)
#define VIDEO_LOGIC_DLL "stvl32d"
#else
#define VIDEO_LOGIC_DLL "stvl32"
#endif
#endif
#define STEEM_SSE_FAQ "FAQ (SSE)"
#define STEEM_HINTS "Hints"
#define STEEM_MANUAL_SSE "Steem Manual" // file
#define FREE_IMAGE_DLL "FreeImage"
#if defined(SSE_STATS_RTF)
#define STEEM_STATS_FILENAME "stats.rtf"
#else
#define STEEM_STATS_FILENAME "stats.txt"
#endif


/////////
// GUI //
/////////

#define README_FONT_NAME "Courier New"
#define README_FONT_HEIGHT 16
#define EXT_TXT ".txt"
#define EXT_RTF ".rtf"
#define CONFIG_FILE_EXT "ini" // ini, cfg?


//////////
// IKBD //
//////////

#if defined(SSE_HD6301_LL)
#define HD6301_CYCLE_DIVISOR 8 // the 6301 runs at 1MHz (verified by Stefan jL)
#define HD6301_CLOCK (1000000) //used in 6301/ireg.c for mouse speed
#define HD6301_ROM_CHECKSUM 0x6E267 // BTW this rom sends $F1 on reset
#endif


/////////
// MFP //
/////////

#define MFP_XTAL 2457600 // for timers - the MC68901 itself runs at 4mhz on the ST
// MFP emu is not low-level enough for direct handling of timer B, so we still use
// a delay hack
// Or we should set up an event (advantage: handle digital sound fetching more precisely)
// There should be wobble
#define MFP_TIMER_B_DELAY (20)  // TIMERB01.TOS, Krig
#define MFP_TIMER_B_DELAY_STVL (18) // 2 less because we start from DE, not HDE, still stinky!
#define MFP_TIMER_SET_DELAY (2) // MFPTA001.TOS
#define MFP_TIMERS_WOBBLE (2) // >2 -> Audio Sculpture trouble!


/////////
// MMU //
/////////

// delay between GLUE 'DE' decision and first LOAD signal emitted by the MMU
// without waitstates
#define MMU_PREFETCH_LATENCY (8)


/////////
// OSD //
/////////

#define OSD_MESSAGE_LENGTH 40 // in bytes excluding /0
#define OSD_MESSAGE_TIME 1 // in seconds
#define HD_TIMER 100 // Yellow hard disk led (imperfect timing)


///////////
// SOUND //
///////////

#define SOUND_DESIRED_LQ_FREQ (50066/2)
#define YM_LOW_PASS_FREQ (10500) //in Hz, default
#define YM_LOW_PASS_MAX (22000)
#define MW_LATENCY_CYCLES (128+16) // quite the hack, TODO
#define MW_LOW_SHELF_FREQ 80 // officially 50 Hz
#define MW_HIGH_SHELF_FREQ (MIN(10000,(int)sound_freq/2)) // officially  15 kHz


/////////
// TOS //
/////////

#if defined(SSE_TOS_PRG_AUTORUN)
#define AUTORUN_HD (2+'Z'-'C')//2=C, Z: is used for PRG support (can't be a valid GEMDOS drive)
#endif


///////////
// VIDEO //
///////////

#define ORIGINAL_BORDER_SIDE 32
#define VERY_LARGE_BORDER_SIDE 50 // 52+48+320=420
#define LARGE_BORDER_SIDE 46 // 48+44+320=412
#define ORIGINAL_BORDER_BOTTOM 40
#define LARGE_BORDER_BOTTOM 45
#define VERY_LARGE_BORDER_BOTTOM 45
#define ORIGINAL_BORDER_TOP 30
#define BIG_BORDER_TOP 38 // 50hz
#define BIGGEST_DISPLAY 3 //420
#define ST_ASPECT_RATIO_DISTORTION 1.10f // multiplier for Y axis
#define ST_ASPECT_RATIO_DISTORTION_60HZ 1.25f // 60hz on my TV very stretched, here too! (no border)
#define SSE_VID_RECORD_AVI_CODEC "MPG4" //DD-only, poor result
// Shifter wakeup
// 0: Spectrum512 compatible
// 1: Petari's little games 
#define SHIFTER_DEFAULT_WAKEUP (0) 
#define SHIFTER_MAX_WU_SHIFT (3) // - or +


#endif//#ifndef SSEPARAMETERS_H
