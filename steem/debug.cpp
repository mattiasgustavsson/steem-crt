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

FILE: debug.cpp
DESCRIPTION: General debugging facilities. This is meant to debug Steem
itself, not ST programs. This is not the Debugger, see debugger.cpp etc.
for that.
TRACE function (file or IDE), OSD message.
Debug facilities are used in the Regular Steem as well as in the Debugger.
---------------------------------------------------------------------------*/

#include "pch.h" 
#pragma hdrstop

#include <debug.h>
#include <stdarg.h>
#ifdef WIN32
#include <time.h>
#endif
#include <gui.h>
#include <harddiskman.h>
#include <run.h>
#include <steemh.h>
#include <stports.h>
#include <sound.h>
#include <draw.h>
#include <computer.h>
#include <display.h>
#include <debug_framereport.h>
#include <osd.h>
#include <infobox.h>

#if defined(SSE_DEBUG)
int debug0,debug1,debug2,debug3,debug4,debug5,debug6,debug7,debug8,debug9;
#if defined(SSE_HD6301_LL)
extern "C" void (*hd6301_trace)(char *fmt,...);
#endif
#endif

TDebug Debug; // singleton, now present in all builds

TDebug::TDebug() {
#if defined(SSE_DEBUG)
  //ZeroMemory(&debug0,10*sizeof(int));  //warning C4789: destination of memory copy is too small
#endif
#if defined(SSE_DEBUG_LOG_OPTIONS)
  //  We must init those variables for the builds without the Debugger
  ZeroMemory(logsection_enabled,100*sizeof(bool)); // 100> our need
  logsection_enabled[ LOGSECTION_ALWAYS ] = 1;
#if defined(_DEBUG) && !defined(DEBUG_BUILD) // VS IDE debug no Debugger
  logsection_enabled[LOGSECTION_FDC]=0;
  logsection_enabled[LOGSECTION_IO]=0;
  logsection_enabled[LOGSECTION_MFP_TIMERS]=0;
  logsection_enabled[LOGSECTION_INIT]=0; //0; by default
  logsection_enabled[LOGSECTION_CRASH]=0;
  logsection_enabled[LOGSECTION_STEMDOS]=0;
  logsection_enabled[LOGSECTION_IKBD]=0;
  logsection_enabled[LOGSECTION_AGENDA]=0;
  logsection_enabled[LOGSECTION_INTERRUPTS]=0;
  logsection_enabled[LOGSECTION_TRAP]=0;
  logsection_enabled[LOGSECTION_SOUND]=0;
  logsection_enabled[LOGSECTION_VIDEO]=0;
  logsection_enabled[LOGSECTION_BLITTER]=0;
  logsection_enabled[LOGSECTION_MIDI]=0;
  logsection_enabled[LOGSECTION_TRACE]=0;
  logsection_enabled[LOGSECTION_SHUTDOWN]=0;
  logsection_enabled[LOGSECTION_SPEEDLIMIT]=0;
  logsection_enabled[LOGSECTION_CPU]=0;
  logsection_enabled[LOGSECTION_INIFILE]=0;
  logsection_enabled[LOGSECTION_GUI]=0;
  logsection_enabled[LOGSECTION_VIDEO_RENDERING]=0;
  logsection_enabled[LOGSECTION_OPTIONS]=0;
  logsection_enabled[LOGSECTION_IMAGE_INFO]=0;
#endif
#endif
#if defined(SSE_DEBUG_ASSERT)
  IgnoreErrors=0; 
#endif
#if defined(SSE_DEBUG)
  nTrace=0; // trace counter
#endif
#ifdef WIN32  
  SetCurrentDirectory(RunDir.Text);
#endif
  trace_file_pointer=NULL;
#if defined(SSE_DEBUG)
#if defined(SSE_HD6301_LL)
  hd6301_trace=&TDebug::TraceLog;
#endif
#if defined(SSE_DEBUGGER_STACK_CHOICE)
  StackDisplayUseOtherSp=0;
#endif
#endif//#if defined(SSE_DEBUG)
}


TDebug::~TDebug() {
  if(trace_file_pointer)
  {
    TRACE("End\n");
    fclose(trace_file_pointer);
  }
}


void TDebug::Vbl() { 
#if defined(SSE_DEBUGGER_FRAME_REPORT)
  FrameEvents.Vbl(); 
#endif
  if(ShifterTricks)
  {
#if defined(SSE_DEBUG)
#if defined(SSE_DEBUGGER_OSD_CONTROL)
    if(OSD_MASK2 & OSD_CONTROL_SHIFTERTRICKS)
      TRACE_OSD("T%X",ShifterTricks);
#endif
#if defined(SSE_OSD_FPS_INFO) // can also give VRAM size
    if(vcount_at_vsync-vbase_at_vbi>32000)
      TRACE_VID("F%d tricks %x %d bytes\n",FRAME,ShifterTricks,vcount_at_vsync-vbase_at_vbi);
    else
#endif
    TRACE_VID("F%d tricks %x\n",FRAME,ShifterTricks);
#endif
#if defined(SSE_OSD_DEBUGINFO) // eg invisible scroller in menu disk
  if(OPTION_OSD_DEBUGINFO && !border && ShifterTricks)
    TRACE_OSD2("O%X",ShifterTricks);
#endif
    ShifterTricks=0;
  }
#if defined(SSE_DEBUGGER_FRAME_INTERRUPTS)
/*  This system so that we only report these once per frame, giving
    convenient info about VBI, HBI, and MFP IRQ.
*/
  if((OSD_MASK1 & OSD_CONTROL_INTERRUPT)&&FrameInterrupts)
  {
    char buf1[40]="",buf2[4];
    if(FrameInterrupts&2)
      strcat(buf1,"V");
    if(FrameInterrupts&1)
      strcat(buf1,"H");
    if(FrameMfpIrqs)
    {
      strcat(buf1," MFP ");
      for(int i=15;i>=0;i--)
      {
        if(FrameMfpIrqs&(1<<i))
        {
          sprintf(buf2,"%d ",i);
          strcat(buf1,buf2);
        }
      }
    }
    TRACE_OSD(buf1);
  }
  FrameInterrupts=0;
  FrameMfpIrqs=0;
#endif  
#if defined(SSE_OSD_FPS_INFO)
/*  cases where it doesn't work
    Pacmania STE: VBASE ignored by game
    Pro Tennis Simulator intro: screen cleaned up after DE
*/
  if(OPTION_OSD_FPSINFO) // will the load kill our fps?
  {
    DWORD checksum=0;
    DWORD max=MIN(vcount_at_vsync,mem_len);
    for(DWORD x=vbase_at_vbi;x<max;x+=4)
      checksum+=LPEEK(x);
    if(checksum==frame_checksum)
      frame_no_change++;
    frame_checksum=checksum;
  }
#endif
#ifdef SSE_DEBUG
  ASSERT(!(cpu_timer&1));
  nHbis=0;
#endif
  Shifter.nVbl++;
#if defined(SSE_STATS)
  Stats.nFrame++;
#endif
#if defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
  if(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_VC_LINES)
    FrameEvents.Add(scan_y,0,"VB",vbase);
  if(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_SHIFTMODE)
    FrameEvents.Add(scan_y,0,"R=",Shifter.m_ShiftMode);
  if(FRAME_REPORT_MASK1 & FRAME_REPORT_MASK_SYNCMODE)
    FrameEvents.Add(scan_y,0,"S=",Glue.m_SyncMode);
#endif
}


void TDebug::TraceInit() {
  // make sure we run in Steem dir (was a bug before)...
  EasyStr trace_file=RunDir+SLASH+SSE_TRACE_FILE_NAME;
  trace_file_pointer=freopen(trace_file,"w",stdout);
#if defined(SSE_DEBUG)
  if(!trace_file_pointer)
  {
    // handle failure
    int Ret;
    do {
      Ret=Alert("Couldn't open TRACE file, maybe it is open in another\
 program or Steem has no writing right in this directory or the file is\
 read-only",trace_file.Text,MB_ABORTRETRYIGNORE);
      trace_file_pointer=freopen(trace_file,"w",stdout);
    } while(Ret==IDRETRY && !trace_file_pointer);
    if(Ret==IDABORT)
      QuitSteem(); // Init goes on before leaving
  }
#endif
  TraceGeneralInfos(INIT0);
}


void TDebug::Reset(bool Cold) {
  TRACE_INIT("%s reset\n",(Cold?"Cold":"Warm"));
  if(Cold)
  {
#if defined(SSE_DEBUG_ASSERT)
    IgnoreErrors=0;
#endif
#if defined(SSE_OSD_SHOW_TIME)
    OsdControl.StartingTime=timeGetTime();
    OsdControl.StoppingTime=0;
#endif
  }
  else if(runstate==RUNSTATE_RUNNING)
  {
#if defined(SSE_OSD_DEBUGINFO)
    if(OPTION_OSD_DEBUGINFO)
    {
      if(IS_STF)
        TRACE_OSD2("RESET (WU%d)",Mmu.WS[OPTION_WS]);
      else
        TRACE_OSD2("RESET"); //STE covered by 2B
    }
#endif
  }
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  for(int i=0;i<PSEUDO_STACK_ELEMENTS;Debug.PseudoStack[i++]=0);
#endif
#ifdef SSE_DEBUG
  ShifterTricks=0;
#endif
}


#if defined(SSE_DEBUG_TRACE)

void TDebug::Trace(char *fmt,...) {
  // Our TRACE facility has no MFC dependency.
  if(tracing_suspended)
    return;
  va_list body;	
  va_start(body, fmt);
  //ASSERT(trace_buffer);
#if defined(SSE_UNIX)
  int nchars=vsnprintf(trace_buffer,MAX_TRACE_CHARS,fmt,body); // check for overrun 
#else
#if !defined(SSE_DEBUG)
  _vsnprintf(trace_buffer,MAX_TRACE_CHARS,fmt,body); // check for overrun 
#else
  int nchars=_vsnprintf(trace_buffer,MAX_TRACE_CHARS,fmt,body); // check for overrun 
#endif
#endif
  va_end(body);	
#ifdef SSE_DEBUG
  if(nchars==-1)
    strcpy(trace_buffer,"TRACE buffer overrun\n");
#endif
  if(trace_in_log && !log_in_trace)
  {
    log_write(trace_buffer);
    return;
  }
#ifdef WIN32
#if defined(SSE_DEBUG_TRACE_IDE)
  OutputDebugString(trace_buffer);
#endif
#endif
#if defined(SSE_UNIX_TRACE)
  if(!SSEConfig.TraceFile)  
    fprintf(stderr,trace_buffer);
#endif 
  if(trace_file_pointer && trace_buffer)
  {
    //printf(trace_buffer); // warning
    printf("%s",trace_buffer); // better?
#if defined(SSE_DEBUG)
    nTrace++; 
#else
//    if(Debug.trace_file_pointer)
  //    fflush(Debug.trace_file_pointer);
#endif
  }
#if defined(SSE_DEBUGGER)
  if(TRACE_FILE_REWIND && nTrace>=TRACE_MAX_WRITES && trace_file_pointer)
  {
    nTrace=0;
    rewind(trace_file_pointer); // it doesn't erase
    TRACE("\n============\nREWIND TRACE\n============\n");
  }
#endif
}


// A series of TRACE giving precious info at the start & end of emulation

void TDebug::TraceGeneralInfos(int when) {

  // get run date and time in strings
  char sdate[9],stime[9];
#ifdef WIN32
  _strdate( sdate );
  _strtime( stime );
#endif
#ifdef UNIX
  time_t rawtime;
  time (&rawtime);
  tm *timeinfo=localtime(&rawtime);
  strftime(sdate,9,"%D",timeinfo);
  strftime(stime,9,"%T",timeinfo);
#endif

  switch(when) {
  case INIT0: // opening of trace
    TRACE2("Steem TRACE %s %s\n",sdate,stime);
    TRACE2("v%s R%d (built %s)\n",stem_version_text,SSE_VERSION_R,
      stem_version_date_text);
#ifdef SSE_DEBUGGER
    TRACE2("Debugger ");
#endif
#ifdef SSE_BETA
    TRACE2("Beta ");
#endif
#ifdef SSE_VID_DD
    TRACE2("DD%x ",DIRECTDRAW_VERSION>>8);
#endif
#ifdef SSE_VID_D3D
    TRACE2("D3D%x ",DIRECT3D_VERSION>>8);
#endif
#ifdef BCC_BUILD
    TRACE2("BCC");
#endif
#ifdef VC_BUILD
    TRACE2("VC%d",_MSC_VER);
#endif
#ifdef MINGW_BUILD
    TRACE2("MinGW");
#endif
#ifdef UNIX
    TRACE2("GCC");
#endif
#ifdef SSE_X64
    TRACE2(" 64bit\n");
#else
    TRACE2(" 32bit\n");
#endif
    break;
  case INIT: // when init done
  {
    TRACE2("%s %d %s %d %s %d %s %d %s %d %s %d %s %X\n",
      UNRAR_DLL,SSEConfig.UnrarDll,
      UNZIP_DLL,SSEConfig.unzipd32Dll,
      ARCHIVEACCESS_DLL,SSEConfig.ArchiveAccess,
      SSE_DISK_CAPS_PLUGIN_FILE,SSEConfig.CapsImgDll,
      PASTI_DLL,SSEConfig.PastiDll,
      FREE_IMAGE_DLL,SSEConfig.FreeImageDll,
      VIDEO_LOGIC_DLL,SSEConfig.Stvl);
    TConfigStoreFile CSF(globalINIFile);
    TRACE2(
#ifdef WIN32
      "startup %d %d %d %d %d %d %d\n",
#endif
#ifdef UNIX
      "startup %d %d %d %d %d\n",
#endif
      CSF.GetInt("Options","AutoLoadSnapShot",0),
      CSF.GetInt("Options","StartFullscreen",0),
#ifdef WIN32
      Disp.DrawToVidMem,
      Disp.BlitHideMouse,
#endif
      CSF.GetInt("Options","NoDirectDraw",0),
      CSF.GetInt("Options","NoDirectSound",0),
      CSF.GetInt("Options","RunOnStart",0),
      SSEConfig.TraceFile);
    break;
  }
  case RESET: // ST reset + load snaphot
    TRACE2("%s%d ~%d %dK T%X(%d) C%d C%d C%d #%d D%d B%d dwm %d %d V%d E%d F%d H%d A%d\n",
    st_model_name[ST_MODEL],Mmu.WS[OPTION_WS],n_cpu_cycles_per_second,
    mem_len/1024,tos_version,SSEConfig.TosLanguage,OPTION_C1,OPTION_C2*2,OPTION_C3*3,
    OPTION_HACKS,STPort[3].Type,border,draw_win_mode[0],draw_win_mode[1],OPTION_WIN_VSYNC,extended_monitor,
    floppy_instant_sector_access,!HardDiskMan.DisableHardDrives,SSEOptions.Acsi);
    break;
  case START:
#if defined(SSE_OSD_SHOW_TIME)
    if(OsdControl.StoppingTime)
      OsdControl.StartingTime+=timeGetTime()-OsdControl.StoppingTime;
#endif
#if defined(SSE_EMU_THREAD)
    if(OPTION_EMUTHREAD)
      TRACE2("%s Run thread $%x\n",stime,EmuThreadId);
    else
#endif
      TRACE2("%s Run\n",stime);
    break;

  case STOP:
#if defined(SSE_OSD_SHOW_TIME)
    OsdControl.StoppingTime=timeGetTime();
#endif
#if defined(SSE_EMU_THREAD)
    if(OPTION_EMUTHREAD)
      TRACE2("%s Stop thread $%x\n",stime,EmuThreadId);
    else
#endif
      TRACE2("%s Stop\n",stime);
    break;

  case EXIT:
    TRACE2("%s Leaving Steem\n",stime);
    break;

  }//sw
  FlushTrace();
}


void TDebug::FlushTrace() {
  if(trace_file_pointer)
  {
    fflush(trace_file_pointer);
    if(InfoBox.IsVisible()) // show?
    {
      if(InfoBox.Page==INFOPAGE_TRACE)
        InfoBox.CreatePage(InfoBox.Page);
    }
  }
#ifdef ENABLE_LOGFILE
  if(logfile)
    fflush(logfile);
#endif
}


#if defined(SSE_DEBUG)

void TDebug::TraceLog(char *fmt,...) { // static
  //ASSERT(Debug.LogSection>=0 && Debug.LogSection<100);
  if(Debug.tracing_suspended)
    return;
  if(Debug.LogSection<NUM_LOGSECTIONS 
    && Debug.logsection_enabled[Debug.LogSection]
#if defined(DEBUG_BUILD)
    ||::logsection_enabled[Debug.LogSection] 
#endif
    )
  {
    va_list body;	
    va_start(body, fmt);
    //ASSERT(Debug.trace_buffer);
#if defined(SSE_UNIX)
    int nchars=vsnprintf(Debug.trace_buffer,MAX_TRACE_CHARS,fmt,body); // check for overrun 
#else
    int nchars=_vsnprintf(Debug.trace_buffer,MAX_TRACE_CHARS,fmt,body); // check for overrun 
#endif
    va_end(body);	
    if(nchars==-1)
      strcpy(Debug.trace_buffer,"TRACE buffer overrun\n");
    if(Debug.trace_in_log && !Debug.log_in_trace)
    {
      log_write(Debug.trace_buffer);
      return;
    }
#ifdef WIN32
#if defined(SSE_DEBUG_TRACE_IDE)
    if(Debug.LogSection!=LOGSECTION_CPU) //there are limits!
      OutputDebugString(Debug.trace_buffer);
#endif
#endif
#if defined(SSE_UNIX_TRACE)
    if(!SSEConfig.TraceFile)
      fprintf(stderr,Debug.trace_buffer);
#endif 
    if(Debug.trace_file_pointer && Debug.trace_buffer)
      printf(Debug.trace_buffer),Debug.nTrace++; 
    if(TRACE_FILE_REWIND && Debug.nTrace>=TRACE_MAX_WRITES && Debug.trace_file_pointer)
    {
      Debug.nTrace=0;
      rewind(Debug.trace_file_pointer); // it doesn't erase
      TRACE("\n============\nREWIND TRACE\n============\n");
      Debug.TraceGeneralInfos(INIT);
    }
  }
}

#endif//#if defined(SSE_DEBUG)

#endif


#if defined(SSE_DEBUGGER_PSEUDO_STACK)

void TDebug::PseudoStackCheck(DWORD return_address) {
  for(int i=0;i<PSEUDO_STACK_ELEMENTS;i++)
    if(PseudoStack[i]==return_address)
      for(int j=i;j<PSEUDO_STACK_ELEMENTS-1;j++)
        PseudoStack[j]=PseudoStack[j+1];
}


void TDebug::PseudoStackPop(DWORD return_address) {
  int address_index=-1;
  for(int i=0;i<PSEUDO_STACK_ELEMENTS-1;i++)
  {
    if(PseudoStack[i]==return_address)
    {
      address_index=i;
      break;
    }
  }
  if(address_index>-1)
  {
    for(int i=address_index;i<PSEUDO_STACK_ELEMENTS-1;i++)
      PseudoStack[i]=PseudoStack[i+1];
  }
}


void TDebug::PseudoStackPush(DWORD return_address) {
  for(int i=PSEUDO_STACK_ELEMENTS-1;i>0;i--)
    PseudoStack[i]=PseudoStack[i-1];
  PseudoStack[0]=return_address;
}

#endif


#if defined(SSE_DEBUGGER_TRACE_EVENTS)
  // not very smart...
void TDebug::TraceEvent(void* pointer) {
  TRACE(PRICV " ",ACT);
  if(pointer==event_scanline)
    TRACE("event_scanline");
  else if(pointer==event_timer_a_timeout)
    TRACE("event_timer_a_timeout");
  else if(pointer==event_timer_b_timeout)
    TRACE("event_timer_b_timeout");
  else if(pointer==event_timer_c_timeout)
    TRACE("event_timer_c_timeout");
  else if(pointer==event_timer_d_timeout)
    TRACE("event_timer_d_timeout");
  else if(pointer==event_timer_b)
    TRACE("event_timer_b");
  else if(pointer==event_start_vbl)
    TRACE("event_start_vbl");
  else if(pointer==event_vbl_interrupt)
    TRACE("event_vbl_interrupt");
  else if(pointer==event_trigger_vbi)
    TRACE("event_trigger_vbi");
  else if(pointer==event_wd1772)
    TRACE("event_wd1772");
  else if(pointer==event_driveA_ip)
    TRACE("event_driveA_ip");
  else if(pointer==event_driveB_ip)
    TRACE("event_driveB_ip");
  else if(pointer==event_acia)
    TRACE("event_acia");
#if USE_PASTI
  else if(pointer==event_pasti_update)
    TRACE("event_pasti_update");
#endif
    TRACE(" (" PRICV ")\n",ACT-time_of_next_event);
}

#endif
