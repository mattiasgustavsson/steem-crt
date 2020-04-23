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

DOMAIN: Debug
FILE: debug.h
DESCRIPTION: Declarations for debug facilities, like ASSERT, TRACE...
The Debug object and some debug facilities are used in all builds (release
too).
Debugger: fake IO declarations
struct TDebug
This file is used by 6301.c.
---------------------------------------------------------------------------*/

#pragma once
#ifndef SSEDEBUG_H
#define SSEDEBUG_H

#include "SSE.h"

#ifdef UNIX
#include "../pch.h"
#include <assert.h>
#endif

#if defined(__cplusplus)
#ifdef WIN32
#include <windows.h>
#endif
#include "conditions.h"
#endif

#include "parameters.h"
#include <stdio.h>

#ifdef MINGW_BUILD
#define NULL 0
#endif

#if defined(SSE_DEBUG) // Debugger build or ide debug build
// general use debug variables;
extern 
#ifdef __cplusplus
"C" 
#endif//c++
int debug0,debug1,debug2,debug3,debug4,debug5,debug6,debug7,debug8,debug9;
#endif


#define MAX_TRACE_CHARS 512//256

#if defined(SSE_DEBUGGER_PSEUDO_STACK)
#define PSEUDO_STACK_ELEMENTS 64
#endif

// a structure that may be used by C++ and C objects
#pragma pack(push, 8)

struct TDebug {
#ifdef __cplusplus 
  // ENUM 
  enum EDebug {INIT0,INIT,RESET,START,STOP,EXIT};
#endif
  // FUNCTIONS
#ifdef __cplusplus 
  TDebug();
  ~TDebug();
#if defined(SSE_DEBUG_TRACE)
  void FlushTrace();
  void TraceInit();
  void Trace(char *fmt,...); // one function for both IDE & file
  // if logsection enabled, static for function pointer, used in '6301':
  static void TraceLog(char *fmt,...); 
#endif
  void TraceGeneralInfos(int when);
  void Vbl();
  void Reset(bool Cold);
#if defined(SSE_DEBUGGER_TRACE_EVENTS)
  void TraceEvent( void* pointer);
#endif
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  void PseudoStackCheck(DWORD return_address);
  void PseudoStackPop(DWORD return_address);
  void PseudoStackPush(DWORD return_address);
#endif
#endif//C++
  // DATA
#if defined(SSE_DEBUGGER_TIMERS_ACTIVE)
  HWND boiler_timer_hwnd[4]; //to record WIN handles
#endif
  FILE *trace_file_pointer; 
#if defined(SSE_DEBUG)
  int nTrace;
#endif
  int LogSection;
  int ShifterTricks;
#if defined(SSE_OSD_FPS_INFO)
  DWORD frame_checksum;
  DWORD vbase_at_vbi,vcount_at_vsync;
  BYTE frame_no_change;
#endif
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  DWORD PseudoStack[PSEUDO_STACK_ELEMENTS]; // rotating stack
#endif
#if defined(SSE_DEBUGGER_FAKE_IO)
/*  Hack. A free zone in IO is mapped to an array of masks to control 
    a lot of debug options using the Debugger's built-in features.
    Memory browsers display words so we use words even if bytes are
    handier (less GUI clutter).
*/
  WORD ControlMask[FAKE_IO_LENGTH];
#endif
#if defined(SSE_DEBUGGER_MONITOR_VALUE)
  WORD MonitorValue;
#endif
#if defined(SSE_DEBUGGER_FRAME_INTERRUPTS)
  WORD FrameMfpIrqs; // for OSD report
#endif
#if defined(SSE_DEBUG)
  WORD nHbis; // counter for each frame
#endif
#if defined(SSE_DEBUG)
  BYTE logsection_enabled[100];
#endif
#if defined(SSE_DEBUG_ASSERT)
  BYTE IgnoreErrors;
#endif
#if defined(SSE_DEBUGGER_MONITOR_VALUE)
  BYTE MonitorValueSpecified; // Debugger SSE option
  BYTE MonitorComparison; // as is, none found = 0 means no value to look for
#endif
#if defined(SSE_DEBUGGER_MONITOR_RANGE)
  BYTE MonitorRange; //check from ad1 to ad2
#endif
#if defined(SSE_DEBUGGER_BROWSER_6301)
  BYTE HD6301RamBuffer[256+8];
#endif
#if defined(SSE_DEBUGGER_STACK_CHOICE)
  BYTE StackDisplayUseOtherSp; //flag
#endif
#if defined(SSE_DEBUGGER_FRAME_INTERRUPTS)
  BYTE FrameInterrupts; //bit0 VBI 1 HBI 2 MFP
#endif
#if defined(SSE_DEBUG_TRACE)
  char trace_buffer[MAX_TRACE_CHARS];
#endif
#if defined(SSE_DEBUGGER_ALERTS_IN_STATUS_BAR)
  BYTE PromptOnBreakpoint;
#endif
  BYTE log_in_trace,trace_in_log,tracing_suspended;
};



extern 
#ifdef __cplusplus
"C" 
#endif
struct TDebug Debug;

#pragma pack(pop)



#ifdef __cplusplus

#if defined(DEBUG_BUILD)
// to place Steem Debugger breakpoints in the code
extern void debug_set_bk(unsigned long ad,bool set); //bool set
#define BREAK(ad) debug_set_bk(ad,true)
#else
#define BREAK(ad)
#endif
#endif//#ifdef __cplusplus

#if defined(SSE_DEBUG_LOG_OPTIONS)
/* We replace defines (that were in acc.h but also other places!) with an enum
   and we change some of the sections to better suit our needs.
   We use the same sections to control our traces in the Debugger, menu log.
   So, Steem has a double log system!
   It came to be because:
   - unfamiliar with the log system
   - familiar with a TRACE system (like in MFC) that can output in the IDE
   - the log system would choke the computer (each Shifter trick written down!)
*/

#undef LOGSECTION_INIFILE
#undef LOGSECTION_GUI

enum logsection_enum_tag {
 LOGSECTION_ALWAYS,
 LOGSECTION_FDC,
 LOGSECTION_DMA,
 LOGSECTION_PASTI,
 LOGSECTION_IMAGE_INFO,
 LOGSECTION_IO ,
 LOGSECTION_INTERRUPTS ,
 LOGSECTION_TRAP ,
 LOGSECTION_MMU,
 LOGSECTION_MFP_TIMERS ,
 LOGSECTION_MFP=LOGSECTION_MFP_TIMERS,
 
 LOGSECTION_CRASH ,
 LOGSECTION_STEMDOS ,
 LOGSECTION_ACIA,
 LOGSECTION_IKBD ,
 LOGSECTION_MIDI ,
 LOGSECTION_VIDEO ,
 LOGSECTION_BLITTER ,
 LOGSECTION_TRACE ,
 LOGSECTION_CPU ,
 LOGSECTION_CARTRIDGE,
 LOGSECTION_INIT ,
 LOGSECTION_INIFILE ,
 LOGSECTION_SHUTDOWN ,
 LOGSECTION_SPEEDLIMIT ,
 LOGSECTION_GUI ,
 LOGSECTION_AGENDA ,
 LOGSECTION_OPTIONS, 
 LOGSECTION_VIDEO_RENDERING,
 LOGSECTION_SOUND ,
 NUM_LOGSECTIONS,
 };
#endif

#if defined(SSE_DEBUGGER_FAKE_IO)

#define OSD_MASK1 (Debug.ControlMask[2])
#define OSD_CONTROL_INTERRUPT               (1<<15)
#define OSD_CONTROL_IKBD                  (1<<14)
#define OSD_CONTROL_FDC              (1<<13)

#define OSD_MASK_CPU (Debug.ControlMask[3])
#define OSD_CONTROL_CPUTRACE           (1<<15)
#define OSD_CONTROL_CPUBOMBS  (1<<14)

#define OSD_MASK2 (Debug.ControlMask[4])
#define OSD_CONTROL_SHIFTERTRICKS           (1<<15)
#define OSD_CONTROL_MODES (1<<14)

#define TRACE_MASK1 (Debug.ControlMask[6]) //Glue
#define TRACE_CONTROL_VERTOVSC (1<<15)

#define TRACE_MASK2 (Debug.ControlMask[7])
#define TRACE_CONTROL_IRQ_TA (1<<15) //timer A
#define TRACE_CONTROL_IRQ_TB (1<<14) //timer B
#define TRACE_CONTROL_IRQ_TC (1<<13) //timer C
#define TRACE_CONTROL_IRQ_TD (1<<12) //timer D
#define TRACE_CONTROL_ECLOCK (1<<11)
#define TRACE_CONTROL_IRQ_SYNC (1<<10) //vbi, hbi
#define TRACE_CONTROL_RTE (1<<9)
#define TRACE_CONTROL_EVENT (1<<8)

#define TRACE_MASK3 (Debug.ControlMask[8])
#define TRACE_CONTROL_FDCSTR (1<<15)
#define TRACE_CONTROL_FDCBYTES (1<<14)//no logsection needed
#define TRACE_CONTROL_FDCPSG (1<<13)//drive/side
#define TRACE_CONTROL_FDCREGS (1<<12)// writes to registers CR,TR,SR,DR
#define TRACE_CONTROL_FDCWD (1<<11) // for Steem's 2nd wd1772 emu, more details
#define TRACE_CONTROL_FDCMFM (1<<10)
#define TRACE_CONTROL_DISKBOOT (1<<9)

#define DEBUGGER_CONTROL_MASK1 (Debug.ControlMask[9])
#define DEBUGGER_CONTROL_LARGE_HISTORY (1<<15)
#define DEBUGGER_CONTROL_HISTORY_TMG (1<<14)

#define SOUND_CONTROL_MASK (Debug.ControlMask[10])
#define SOUND_CONTROL_OSD (1<<9)//first entries other variables

#define DEBUGGER_CONTROL_MASK2 (Debug.ControlMask[11])
#define DEBUGGER_CONTROL_NEXT_PRG_RUN (1<<15)
#define DEBUGGER_CONTROL_TOPOFF (1<<14)
#define DEBUGGER_CONTROL_BOTTOMOFF (1<<13)
#define DEBUGGER_CONTROL_6301 (1<<12) // custom prg run

#define TRACE_MASK_IO (Debug.ControlMask[12])
#define TRACE_CONTROL_IO_W (1<<15)
#define TRACE_CONTROL_IO_R (1<<14)

#define TRACE_MASK4 (Debug.ControlMask[13]) //cpu
#define TRACE_CONTROL_CPU_REGISTERS (1<<15) 
#define TRACE_CONTROL_CPU_CYCLES (1<<14) 
#define TRACE_CONTROL_CPU_LIMIT (1<<13)
#define TRACE_CONTROL_CPU_VALUES (1<<12)

#define TRACE_MASK_14 (Debug.ControlMask[14]) //Glue 2

#endif//#if defined(SSE_DEBUGGER_FAKE_IO)

// debug macros
extern BYTE FullScreen; // to avoid asserts in fullscreen

// ASSERT
#if defined(SSE_DEBUG)
#if defined(_DEBUG) && defined(VC_BUILD)
// Our ASSERT facility has no MFC dependency.
#if defined(SSE_X64_DEBUG)
#define ASSERT(x) {if(!(x) && !FullScreen) DebugBreak();}
#else
#define ASSERT(x) {if(!((x)) && !FullScreen) _asm{int 0x03}}
#endif
#elif defined(SSE_UNIX_TRACE)
//#define ASSERT(x) if (!(x)) {TRACE("Assert failed: %s\n",#x);} 
#define ASSERT(x) assert(x)
#elif defined(DEBUG_BUILD) // for Debugger
#ifdef __cplusplus
#define ASSERT(x) {if (!((x))) {TRACE("Assert failed: %s\n",#x); \
  if(!Debug.IgnoreErrors) { \
  debug9=MessageBox(0,#x,"ASSERT",MB_ICONWARNING|MB_ABORTRETRYIGNORE);   \
  if(debug9==IDABORT) exit(EXIT_FAILURE);\
  Debug.IgnoreErrors=(debug9==IDIGNORE);}}}
#endif//c++
#endif//vc
#else //!SSE_DEBUG
#define ASSERT(x)
#endif

// BREAKPOINT 
#if defined(SSE_DEBUG)
#if defined(_DEBUG) && defined(VC_BUILD)
#if defined(SSE_X64_DEBUG)
#define BREAKPOINT {DebugBreak();}
#else
#define BREAKPOINT _asm { int 3 }
#endif
#elif defined(SSE_UNIX_TRACE)
#define BREAKPOINT TRACE("BREAKPOINT\n"); // extremely silly, I know
#elif defined(DEBUG_BUILD) // for Debugger
#ifdef __cplusplus
#define BREAKPOINT {if(!Debug.IgnoreErrors) { \
  TRACE("Breakpoint\n"); \
  Debug.IgnoreErrors=!(MessageBox(0,"no message","Breakpoint",MB_ICONWARNING|MB_OKCANCEL)==IDOK);}}
#endif//c++
#endif
#else //!SSE_DEBUG
#define BREAKPOINT {}
#endif

// BRK(x) 
#if defined(SSE_DEBUG)

#if defined(DEBUG_BUILD)

#ifdef __cplusplus
#define BRK(x){if(!Debug.IgnoreErrors) { \
  TRACE("Breakpoint: %s\n",#x); \
  Debug.IgnoreErrors=!(MessageBox(0,#x,"Breakpoint",MB_ICONWARNING|MB_OKCANCEL)==IDOK);}}
#endif//c++

#elif defined(SSE_UNIX_TRACE)

#define BRK(x) TRACE("BRK %s\n",#x);

#elif defined(VC_BUILD)

#if defined(SSE_X64_DEBUG)
#define BRK(x) {DebugBreak();}
#else
#define BRK(x) {TRACE("BRK %s\n",#x); _asm { int 3 } }
#endif

#endif

#else //!SSE_DEBUG

#define BRK(x)

#endif

// TRACE
#if defined(SSE_DEBUG_TRACE) && defined(SSE_DEBUG) //395 no TRACE for release mode
#ifdef __cplusplus
#define TRACE Debug.Trace
#endif//c++
#else
#if defined(VC_BUILD)
#define TRACE(x,...)
#else
#define TRACE
#endif
#endif//#if defined(SSE_DEBUG_TRACE) 

// TRACE_ENABLED
#if defined(DEBUG_BUILD) // Debugger
#if defined(_DEBUG) // IDE
//#define TRACE_ENABLED (Debug.logsection_enabled[LOGSECTION] || LOGSECTION<NUM_LOGSECTIONS && logsection_enabled[LOGSECTION])
#define TRACE_ENABLED(section) (Debug.logsection_enabled[section] || section<NUM_LOGSECTIONS && logsection_enabled[section])
#else // no IDE but Debugger
//#define TRACE_ENABLED (Debug.logsection_enabled[LOGSECTION] || LOGSECTION<NUM_LOGSECTIONS && logsection_enabled[LOGSECTION])
#define TRACE_ENABLED(section) (Debug.logsection_enabled[section] || section<NUM_LOGSECTIONS && logsection_enabled[section])
#endif
#else // no Debugger 
#if defined(_DEBUG) // IDE
//#define TRACE_ENABLED (Debug.logsection_enabled[LOGSECTION])
#define TRACE_ENABLED(section) (Debug.logsection_enabled[section])
#else // 3rd party objects
#define TRACE_ENABLED (0)
#endif
#endif

// TRACE_LOG
#if defined(SSE_DEBUG)
#ifdef __cplusplus
#define TRACE_LOG Debug.LogSection=LOGSECTION, Debug.TraceLog //!
#endif//C++
#else
#if defined(VC_BUILD)
#define TRACE_LOG(x,...)
#else
#define TRACE_LOG
#endif
#endif

// v3.6.3 introducing more traces,  verbose here, short in code
// TRACE_FDC
#if defined(SSE_DEBUG)
#ifdef __cplusplus
#define TRACE_FDC Debug.LogSection=LOGSECTION_FDC, Debug.TraceLog //!
#endif//C++
#else
#if defined(VC_BUILD)
#define TRACE_FDC(x,...)
#else
#define TRACE_FDC
#endif
#endif


#define TRACE_HDC TRACE_FDC //3.7.2


// so we get extended trace only if 'wd' checked
#if defined(DEBUG_BUILD)
#ifdef __cplusplus
//#define TRACE_WD if((TRACE_MASK3 & TRACE_CONTROL_FDCWD) && (LOGSECTION==LOGSECTION_FDC)) Debug.LogSection=LOGSECTION,Debug.TraceLog
#define TRACE_WD if((TRACE_MASK3 & TRACE_CONTROL_FDCWD)) Debug.LogSection=LOGSECTION,Debug.TraceLog
#endif//C++
#else
#if defined(VC_BUILD)
#define TRACE_WD(x,...)
#else
#define TRACE_WD
#endif
#endif

// TRACE_INIT 3.7.0
#if defined(SSE_DEBUG)
#ifdef __cplusplus
#define TRACE_INIT Debug.LogSection=LOGSECTION_INIT, Debug.TraceLog //!
#endif//C++
#else
#if defined(VC_BUILD)
#define TRACE_INIT(x,...)
#else
#define TRACE_INIT
#endif
#endif

// TRACE_INT 3.7.0
#if defined(SSE_DEBUG)
#ifdef __cplusplus
#define TRACE_INT Debug.LogSection=LOGSECTION_INTERRUPTS, Debug.TraceLog //!
#endif//C++
#else
#if defined(VC_BUILD)
#define TRACE_INT(x,...)
#else
#define TRACE_INT
#endif
#endif

// TRACE_MFP 3.7.0
#if defined(SSE_DEBUG)
#ifdef __cplusplus
#define TRACE_MFP Debug.LogSection=LOGSECTION_MFP, Debug.TraceLog //!
#endif//C++
#else
#if defined(VC_BUILD)
#define TRACE_MFP(x,...)
#else
#define TRACE_MFP
#endif
#endif

// TRACE_MFM 3.7.1
#if defined(SSE_DEBUGGER_TRACE_CONTROL) 
#ifdef __cplusplus
#define TRACE_MFM if(TRACE_MASK3&TRACE_CONTROL_FDCMFM) Debug.LogSection=LOGSECTION_FDC, Debug.TraceLog //!
#endif//C++
#else
#if defined(VC_BUILD)
#define TRACE_MFM(x,...)
#else
#define TRACE_MFM
#endif
#endif

// TRACE_TOS 3.7.1
#if defined(SSE_DEBUG)
#ifdef __cplusplus
#define TRACE_TOS Debug.LogSection=LOGSECTION_STEMDOS, Debug.TraceLog //!
#endif//C++
#else
#if defined(VC_BUILD)
#define TRACE_TOS(x,...)
#else
#define TRACE_TOS
#endif
#endif

// TRACE_VID 3.7.3
#if defined(SSE_DEBUG)
#ifdef __cplusplus
#define TRACE_VID Debug.LogSection=LOGSECTION_VIDEO, Debug.TraceLog //!
#endif//C++
#else
#if defined(VC_BUILD)
#define TRACE_VID(x,...)
#else
#define TRACE_VID
#endif
#endif

// TRACE_VID_R 3.9.3
#if defined(SSE_DEBUG)
#ifdef __cplusplus
#define TRACE_VID_R Debug.LogSection=LOGSECTION_VIDEO_RENDERING, Debug.TraceLog //!
#define TRACE_VID_RECT(rect) Debug.LogSection=LOGSECTION_VIDEO_RENDERING,Debug.TraceLog("%d %d %d %d\n",rect.left,rect.top,rect.right,rect.bottom)
#endif//C++
#else
#if defined(VC_BUILD)
#define TRACE_VID_R(x,...)
#define TRACE_VID_RECT(rect)
#else
#define TRACE_VID_R
#define TRACE_VID_RECT
#endif
#endif

#if defined(SSE_DEBUGGER_TRACE_EVENTS) //3.8.0
#define TRACE_EVENT(x) Debug.TraceEvent(x)
#else
#define TRACE_EVENT(x) 
#endif

// OSD
#if defined(SSE_DEBUG)
#define TRACE_OSD OsdControl.Trace // (...)
#elif defined(VC_BUILD)
#define TRACE_OSD(x,...)
#else
#define TRACE_OSD //?
#endif

// TRACE_RECT 3.9.2
#define TRACE_RECT(rect) TRACE("%d %d %d %d\n",rect.left,rect.top,rect.right,rect.bottom)
#define TRACE_OSD_RECT(rect) TRACE_OSD("%d %d %d %d",rect.left,rect.top,rect.right,rect.bottom)

// VERIFY
#if defined(SSE_DEBUG)
#if defined(_DEBUG) && defined(VC_BUILD)
// Our VERIFY facility has no MFC dependency.
#if defined(SSE_X64_DEBUG)
#define VERIFY(x) {if(!(x) && !FullScreen) DebugBreak();}
#else
#define VERIFY(x) {if(!((x)) && !FullScreen) _asm{int 0x03}}
#endif
#elif defined(SSE_UNIX_TRACE)
#define VERIFY(x) if (!(x)) {TRACE("Verify failed: %s\n",#x);} 
#elif defined(DEBUG_BUILD) // for Debugger
#ifdef __cplusplus
#define VERIFY(x) {if (!(x)) {TRACE("Verify failed: %s\n",#x); \
  if(!Debug.IgnoreErrors) { \
  debug9=MessageBox(0,#x,"VERIFY",MB_ICONWARNING|MB_ABORTRETRYIGNORE);   \
  if(debug9==IDABORT) exit(EXIT_FAILURE);\
  Debug.IgnoreErrors=(debug9==IDIGNORE);}}}
#endif//C++
#endif
#else //!SSE_DEBUG
#define VERIFY(x) ((void)(x))
#endif


#if !defined(SSE_DEBUG) 
enum { // to pass compilation
 LOGSECTION_FDC_BYTES, // was DIV
 LOGSECTION_IMAGE_INFO, //was Pasti
 LOGSECTION_OPTIONS,
 };
#endif

#if defined(SSE_DEBUGGER_FRAME_REPORT)
#define REPORT_LINE FrameEvents.ReportLine()
#else
#define REPORT_LINE
#endif

#if defined(SSE_DEBUG_TRACE)
#define TRACE2 Debug.Trace
#define FLUSH_TRACE Debug.FlushTrace()
#else
#if defined(VC_BUILD)
#define TRACE2(x,...)
#else
#define TRACE2
#endif
#define FLUSH_TRACE
#endif



#endif//#ifndef SSEDEBUG_H
