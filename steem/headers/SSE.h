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

DOMAIN: All
FILE: SSE.h
DESCRIPTION: Compilation directives (similar to conditions.h, this one is
specific to Steem SSE)
---------------------------------------------------------------------------*/

// for v4.0.2 + 4.1.0 beta
#pragma once
#ifndef SSE_H
#define SSE_H

/*
ST Enhanced EMulator Sensei Software Edition (Steem SSE)
--------------------------------------------------------

SVN code repository is at:
https://sourceforge.net/projects/steemsse/

This is based on the source code for Steem R63 as released by Steem authors,
Ant & Russ Hayward, at
https://github.com/steem-engine/steem-engine

STEVEN_SEAGAL doesn't need to be defined anymore: the end of an era!

SSE_DEBUG, if needed, should be defined in the project/makefile.
It has an effect on both the Debugger and the Visual Studio _DEBUG builds.
DEBUG_BUILD for the Debugger (Windows-only)
SSE_DD for the DirectDraw build (2 or 7), Direct3D by default (Windows)
SSE_RELEASE for official releases (SSE2 processor requirement in Windows)
BCC_BUILD for the Borland C++ builds (development-only)
MINGW_BUILD for the MinGW32 builds (development-only)
VC_BUILD for all Visual C++ builds
SSE_STDCALL for the __stdcall calling convention (default in v4, VC builds only)

SSE.h is supposed to mainly be a collection of compiling switches (defines).
It should include nothing and can be included everywhere.
It is included in pch (precompiled header), any change in SSE.h triggers
compilation of about everything.
*/

#define SSE_BUILD
#define NO_RARLIB // don't use rarlib (SSE Windows build supports unrar.dll)
#define SSE_NO_INTERNAL_SPEAKER
#define SSE_NO_UPDATE // remove all update code
#define SSE_NO_WINSTON_IMPORT // nuke WinSTon import

#define SSE_NO_FALCONMODE //v402
//#define SSE_NO_INTRO

#if defined(SSE_RELEASE)
#define SSE_VERSION 402
#define SSE_VERSION_R 1
#else
#define SSE_BETA
//#define SSE_VERSION 402
#define SSE_VERSION 410
#define SSE_VERSION_R 0
#endif

#if defined(SSE_BETA)
//#define SSE_PRIVATE_BUILD // my "beta" option
#endif

#define SSE_LEAN_AND_MEAN

#ifdef WIN32
#ifndef BCC_BUILD
#define SSE_WINDOWS_2000 // needed for 2 screens + CPU %
#endif
#if defined(VC_BUILD) && _MSC_VER>=1500 //VS2008+
#define SSE_VC_INTRINSICS
#endif
#define SSE_WINDOWS_XP // staying XP-compatible
#endif

#ifdef UNIX
#define SSE_UNIX // XSteem SSE
#define SSE_RTAUDIO_LARGER_BUFFER //simplistic attempt
#define SSE_UNIX_OPTIONS_SSE_ICON
#define SSE_UNIX_STATIC_VAR_INIT //odd
#endif

#ifdef _WIN64
#define SSE_X64
#define SSE_X64_DEBUG
#if (_MSC_VER>=1900) // VS2015
#define SSE_X64_GUI // horrible problems
#endif
#define SSE_NO_UNZIPD32 // remove code for unzipd32.dll
#endif


// Exception management...
//#define SSE_M68K_EXCEPTION_TRY_CATCH //works but too slow, especially if _DEBUG
#ifndef _DEBUG
#define SSE_MAIN_LOOP2 //2KB on optimised exe but will catch and report everything (?)
#endif
#if _MSC_VER >= 1500 && !defined(_DEBUG)
#define SSE_MAIN_LOOP3 //VC only
#endif


// Feature switches, still a few, it's nothing compared with before!
#define SSE_ACSI // hard drive
#define SSE_DISK_CAPS // IPF, CTR disk images
//#define SSE_DISK_CAPS_MEMORY // file in memory
#define SSE_DISK_STW // MFM disk image format
#define SSE_DISK_SCP // Supercard Pro disk image format support
#define SSE_DISK_HFE // HxC floppy emulator HFE (v.1) image support
#define SSE_GUI_OPTIONS_MICROWIRE
#define SSE_HD6301_LL // using 3rd party code
#define SSE_IKBDI // command interpreter
#define SSE_IKBD_MAPPINGFILE // v402
#define SSE_INT_MFP_EVENT_IRQ // interrupts in event count mode - to check
//#define SSE_INT_MFP_OPTION // option 68901 (=TRUE if not defined)
#define SSE_MEGASTF_RTC // Ricoh chip //TODO linux
#define SSE_SHIFTER_UNSTABLE
#define SSE_SOUND_16BIT_CENTRED
#define SSE_SOUND_CARTRIDGE // B.A.T etc.
//#define SSE_SOUND_OPTION_DISABLE_DSP // option is disabled!
#define SSE_TOS_KEYBOARD_CLICK // hack to suppress the click
#define SSE_VID_CHECK_VIDEO_RAM
#define SSE_WD1772_LL // low-level elements (3rd party-inspired)
#define SSE_YM2149_LL // low-level emu (3rd party-inspired)

#ifdef WIN32 //todo, some features could be added to XSteem
#define SSE_ARCHIVEACCESS_SUPPORT // 7z + ...
#define SSE_CPU_MFP_RATIO_OPTION // user can fine tune CPU clock
#define SSE_DISK_AUTOSTW
#define SSE_DISK_CREATE_MSA_DIM
#define SSE_DISK_GHOST // save hiscores of STX etc.
#define SSE_DISK_HD // 1.44MB disks
#define SSE_DONGLE     // special adapters (including protection dongles)
#if defined(SSE_DONGLE)
#define SSE_DONGLE_PORT // all dongles grouped in "virtual" port
#define SSE_DONGLE_BAT2
#define SSE_DONGLE_CRICKET
#define SSE_DONGLE_JEANNEDARC
#define SSE_DONGLE_PROSOUND // Wings of Death, Lethal Xcess  STF
#define SSE_DONGLE_LEADERBOARD
#define SSE_DONGLE_MULTIFACE
#define SSE_DONGLE_MUSIC_MASTER
#define SSE_DONGLE_URC
#endif
#define SSE_DRIVE_FREEBOOT // + single-sided
#define SSE_DRIVE_SOUND // poor imitation of a SainT feature
#define SSE_EMU_THREAD // reduces hiccups
#define SSE_FILES_IN_RC
#define SSE_GUI_ALT_MENU // for keyboard control
#define SSE_GUI_CONFIG_FILE // icon in tool bar
#define SSE_GUI_DEFAULT_ST_CONFIG // option Default ST config
#define SSE_GUI_FONT_FIX // not DEFAULT_GUI_FONT if possible (useful?)
#define SSE_GUI_KBD // better keyboard control
#define SSE_GUI_LEGACY_TOOLBAR
#define SSE_GUI_RICHEDIT // for Manual in rtf format
#define SSE_GUI_RICHEDIT2 // links mod
#define SSE_GUI_STATUS_BAR
//#define SSE_GUI_STATUS_BAR_GAME_NAME
#define SSE_HARDWARE_OVERSCAN
#define SSE_IKBD_RTC // battery-powered 6301
#define SSE_JOYSTICK_JUMP_BUTTON
#define SSE_JOYSTICK_NO_MM //circle around unsolved bug (ours or theirs?)
#define SSE_MMU_MONSTER_ALT_RAM // HW hack for ST
#define SSE_OPTION_FASTBLITTER // v402
#define SSE_OSD_DEBUGINFO // in rlz build
//#define SSE_OSD_EXTRACT_GRAPHICS // one-time switch
#define SSE_OSD_FPS_INFO
#define SSE_OSD_SHOW_TIME // measure time you waste
#define SSE_UNRAR_SUPPORT
#define SSE_UNRAR_SUPPORT_WIN // using unrar.dll, up to date
#define SSE_SOUND_ENFORCE_RECOM_OPT // no more exotic options cluttering the page //Unix?
#define SSE_SOUND_MICROWIRE_12DB_HACK
#define SSE_STATS // v401 file + window telling what the ST program is doing
#define SSE_STATS_RTF
#define SSE_TOS_PRG_AUTORUN // Atari PRG + TOS file direct support
#define SSE_VID_3BUFFER_FS // fullscreen Triple Buffering // DD + D3D
#define SSE_VID_FAKE_FULLSCREEN
#define SSE_VID_SCANLINES_INTERPOLATED
#define SSE_VID_VSYNC_WINDOW
#endif//WIN32

#if defined(SSE_WINDOWS_2000)
#ifndef MINGW_BUILD
#define SSE_STATS_CPU // using cool 3rd party function
#endif
#define SSE_VID_2SCREENS
#define SSE_VID_32BIT_ONLY
#endif

#ifdef WIN32
// Windows build is either DirectDraw (DD) or Direct3D (D3D)
// SSE_DD as compile directive (config, makefile) commands DirectDraw build
// D3D9 has smaller footprint and more abilities than DD but 
// has fullscreen GUI issues
#if defined(SSE_DD)
#define SSE_VID_DD
#ifdef VC_BUILD
#define SSE_VID_DD7 // if not defined, DirectDraw2
#endif
#else
#define SSE_VID_D3D
#endif
#endif//WIN32

#if defined(SSE_VID_DD) // DirectDraw
#define SSE_VID_DD_3BUFFER_WIN // window Triple Buffering (DD-only)
#define SSE_VID_DD_MISC // compatibility issues
#ifndef MINGW_BUILD
#define SSE_VID_RECORD_AVI //avifile not so good 
#endif
#endif//dd

#if defined(SSE_VID_D3D) // Direct3D
//#define SSE_VID_NO_FREEIMAGE //saves some KB
#define SSE_VID_D3D_FAKE_FULLSCREEN
//#define SSE_VID_D3D_FLIPEX // works but has constraints and downsides
#define SSE_VID_D3D_MISC
//#define SSE_VID_D3D_SWAPCHAIN // test
#endif//d3d

#if defined(SSE_VID_D3D_FLIPEX)
#define SSE_GUI_MENU // menu replaces icons, for FlipEx mode
#define SSE_GUI_TOOLBAR // togglable floating toolbar
#undef SSE_GUI_LEGACY_TOOLBAR
#endif

#ifdef UNIX
#define SSE_VID_DISABLE_AUTOBORDER //temp
#define SSE_VID_NO_FREEIMAGE
#endif

#ifdef WIN32
#if !defined(SSE_LLVLE)
#define SSE_LLVLE
#endif
#endif

#if defined(SSE_LLVLE)
#define SSE_VID_STVL // use low-level video logic plugin
#if defined(SSE_VID_STVL)
#define SSE_VID_STVL1 // misc
#define SSE_VID_STVL2 // keep STVL regs up-to-date
#define SSE_VID_STVL_SREQ // callback fetch ste sound
#ifdef SSE_DEBUG
#define SSE_VID_STVL3 // use debug facilities
#endif
#endif
#endif//llvle


///////////
// DEBUG //
///////////

// SSE_DEBUG is defined or not by the environment
// debug switches are grouped here

#define SSE_DEBUG_TRACE // all builds now

#if defined(SSE_DEBUG) // Debugger + debug build

#define SSE_DEBUG_ASSERT // 3.8.2

#if defined(DEBUG_BUILD)
#define SSE_DEBUGGER // equivalent to DEBUG_BUILD in Steem SSE
#endif

#if defined(SSE_DEBUGGER)
#define SSE_DEBUGGER_FRAME_REPORT //3.3, now Debugger-only because of mask control
#define SSE_DEBUGGER_REPORT_SCAN_Y_ON_CLICK
#define SSE_DEBUGGER_REPORT_SDP_ON_CLICK // yeah!
#ifdef SSE_HD6301_LL
#define SSE_DEBUGGER_BROWSER_6301 // there's some more code for this one
#define SSE_DEBUGGER_DUMP_6301_RAM
#endif
#define SSE_DEBUGGER_BROWSERS_VECS // in 'reg' columns, eg TB for timer B
#define SSE_DEBUGGER_DECRYPT_TIMERS
#define SSE_DEBUGGER_EXCEPTION_NOT_TOS
#define SSE_DEBUGGER_FAKE_IO //to control some debug options
#if defined(SSE_DEBUGGER_FAKE_IO)
#define SSE_DEBUGGER_BOOT_CHECKSUM
#define SSE_DEBUGGER_OSD_CONTROL
#define SSE_DEBUGGER_FRAME_INTERRUPTS//OSD, handy
#define SSE_DEBUGGER_MUTE_SOUNDCHANNELS //fake io
#define SSE_DEBUGGER_TRACE_CONTROL //beyond log options
#define SSE_DEBUGGER_FRAME_REPORT_MASK // for interactive control in Debugger
#define SSE_DEBUGGER_BIG_HISTORY
#define SSE_DEBUGGER_HISTORY_DUMP
#define SSE_DEBUGGER_TOPOFF
#endif//fake io
#define SSE_DEBUGGER_MONITOR_RANGE // will stop for every address between 2 stops
#define SSE_DEBUGGER_MONITOR_VALUE // specify value (RW) that triggers stop
#define SSE_DEBUGGER_MOUSE_WHEEL // yeah!
#define SSE_DEBUGGER_PSEUDO_STACK
#define SSE_DEBUGGER_STACK_CHOICE
#define SSE_DEBUGGER_STATUS_BAR
#define SSE_DEBUGGER_ALERTS_IN_STATUS_BAR
#define SSE_DEBUGGER_TIMERS_ACTIVE // (in reverse video) yeah!
#define SSE_DEBUGGER_TOGGLE
#define SSE_DEBUGGER_TRACE_EVENTS
#define SSE_DEBUGGER_HIRES_DONT_REDRAW
//#define SSE_IKBD_6301_DISASSEMBLE_ROM // once is enough
#endif//Debugger
#define SSE_DEBUG_LOG_OPTIONS // mine, Debugger or _DEBUG
#ifdef SSE_DEBUG_TRACE
#ifdef _DEBUG // VC
#define SSE_DEBUG_TRACE_IDE
#endif
#endif//trace

#if defined(SSE_UNIX)
#define SSE_UNIX_TRACE // TRACE into the terminal (if it's open?)
#endif

#endif//SSE_DEBUG


// BUGFIXES v4.0.2
#define SSE402R1



///////////////
// DEV BUILD //
///////////////

#if defined(SSE_BETA)
#define TEST01//quick switch
#define TEST02//track bug
//#define TEST03
//#define TEST04
//#define TEST05
//#define TEST06 
//#define TEST07
//#define TEST08
//#define TEST09
#endif

#if defined(SSE_PRIVATE_BUILD)
#define SSE_GUI_OPTION_FOR_TESTS
#endif

#ifdef SSE_BETA // future version
#if SSE_VERSION>=410
/*  The big feature of v4.1 is ready, but it would be nice to beta test
    it for a while, add features (68881?)
    Besides releasing v4.0.2 allows us to use the spaceman demo once more,
    and give us time for another demo (I already have the idea but...)
*/
#define SSE_MEGASTE
#endif
#ifdef UNIX // Unix build is beta-only for a while
#define SSE_UNRAR_SUPPORT
#define SSE_UNRAR_SUPPORT_UNIX
#define SSE_7Z_SUPPORT_UNIX
#endif
#endif

#ifdef SSE_BETA // long term, tests
//#define SSE_HIRES_COLOUR // as POC
//#define SSE_INT_MFP_TIMER_B_PULSE
//#define TEST_STEEM_INTRO
#endif

#endif//#ifndef SSE_H 
