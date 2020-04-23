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

FILE: debug_framereport.cpp
CONDITION: DEBUG_BUILD and SSE_DEBUGGER_FRAME_REPORT must be defined
DESCRIPTION: Frame video events may be recorded at will (using the control
mask browser), and a frame report is written to disk on each emulation
stop or on demand.
Those reports have been invaluable for the development of Steem SSE.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#if defined(SSE_DEBUGGER_FRAME_REPORT)

#include <conditions.h>
#include <easystr.h>
#include <run.h>
#include <cpu.h>
#include <debug_framereport.h>
#include <options.h>
#include <interface_stvl.h>
#include <computer.h>
#include <osd.h>

TFrameEvents FrameEvents;  // singleton

#if defined(SSE_VID_STVL3) && defined(SSE_DEBUGGER_FRAME_REPORT_MASK)
                           
bool shifter_loading=false; // to limit reports
bool shifter_reloading=false;


void FrameEvents_Add(int event_type,int event_value) {
  // new static function for STVL traces
  event_value<<=12;
  switch(event_type) {
  case TStvl::EVENT_BLANK:
    if(FRAME_REPORT_MASK3&FRAME_REPORT_MASK_BLANK)
      FrameEvents.Add(scan_y,Stvl.video_linecycles,"BK",event_value);
    break;
  case TStvl::EVENT_DE:
    if(FRAME_REPORT_MASK3&FRAME_REPORT_MASK_DE)
      FrameEvents.Add(scan_y,Stvl.video_linecycles,"DE",event_value+Stvl.load_ctr);
    break;
  case TStvl::EVENT_HS:
    if(FRAME_REPORT_MASK3&FRAME_REPORT_MASK_SYNC)
      FrameEvents.Add(scan_y,Stvl.video_linecycles,"HS",event_value);
    break;
  case TStvl::EVENT_VS:
    if(FRAME_REPORT_MASK3&FRAME_REPORT_MASK_SYNC)
      FrameEvents.Add(scan_y,Stvl.video_linecycles,"VS",event_value);
    break;
  case TStvl::EVENT_PX:
    if(FRAME_REPORT_MASK4&FRAME_REPORT_MASK_PX)
      FrameEvents.Add(scan_y,Stvl.video_linecycles,"PX",event_value+Stvl.pixel_ctr);
    break;
  case TStvl::EVENT_LD:
    if(FRAME_REPORT_MASK3&FRAME_REPORT_MASK_LD)
    {
      if((FRAME_REPORT_MASK4&FRAME_REPORT_MASK_LIM)==0 
        || !shifter_loading || !Stvl.de || Stvl.load_ctr>4)
      {
        FrameEvents.Add(scan_y,Stvl.video_linecycles,"LD",event_value);
      }
      if(!shifter_loading)
        shifter_loading=true;
      else if(!Stvl.de)
        shifter_loading=false;
    }
    break;
  case TStvl::EVENT_RL:
    if(FRAME_REPORT_MASK4&FRAME_REPORT_MASK_RL)
    {
      if((FRAME_REPORT_MASK4&FRAME_REPORT_MASK_LIM)==0 
        || !shifter_reloading || !Stvl.de || Stvl.load_ctr>4)
        FrameEvents.Add(scan_y,Stvl.video_linecycles,"RL",event_value);
      if(!shifter_reloading)
        shifter_reloading=true;
      else if(!Stvl.de)
        shifter_reloading=false;
    }
    break;
  case TStvl::EVENT_GR:
    if(FRAME_REPORT_MASK1&FRAME_REPORT_MASK_SHIFTMODE)
      FrameEvents.Add(scan_y,Stvl.video_linecycles,"GR",event_value);
    break;
  case TStvl::EVENT_GS:
    if(FRAME_REPORT_MASK1&FRAME_REPORT_MASK_SYNCMODE)
      FrameEvents.Add(scan_y,Stvl.video_linecycles,"GS",event_value);
    break;
  case TStvl::EVENT_SM:
    if(FRAME_REPORT_MASK4&FRAME_REPORT_MASK_SM)
      FrameEvents.Add(scan_y,Stvl.video_linecycles,"SM",event_value);
    break;
  default:
    FrameEvents.Add(scan_y,Stvl.video_linecycles,"??",event_value);
  }//sw
}

#endif


TFrameEvents::TFrameEvents() {
  Init();
}


void TFrameEvents::Add(short scanline,short cycle,char type,int value) {
  if(m_nEvents<MAX_EVENTS-1)
  {
    m_nEvents++;  // starting from 0 each VBL, event 0 is dummy 
    m_FrameEvent[m_nEvents].Add(scanline,cycle,type,value);
  }
}


void TFrameEvents::Add(short scanline,short cycle,char *type,int value) {
  // This overload records a 2 char string as type.
  //ASSERT(m_nEvents>0&&m_nEvents<MAX_EVENTS);
  if(m_nEvents<MAX_EVENTS-1)
  {
    m_nEvents++;  // starting from 0 each VBL, event 0 is dummy 
    m_FrameEvent[m_nEvents].Add(scanline,cycle,type,value);
  }
}


#if defined(SSE_DEBUGGER_REPORT_SDP_ON_CLICK)
// maybe it duplicates an existing feature?

MEM_ADDRESS TFrameEvents::GetSDP(int x,int guessed_scan_y) {
  MEM_ADDRESS sdp=0;
  int i;
  for(i=1; i<=MAX_EVENTS;i++)
  {
    if(m_FrameEvent[i].Scanline==guessed_scan_y)
    {
      int first_letter=m_FrameEvent[i].Type>>8;
      if(first_letter=='V' && (m_FrameEvent[i].Type&0xFF)=='C' &&!sdp)
      {
        sdp=m_FrameEvent[i].Value;
        // look for tricks
        int trick=0;
        while(i<=MAX_EVENTS && m_FrameEvent[i].Scanline==guessed_scan_y
          && m_FrameEvent[i].Type!='T')
          i++;
        if(m_FrameEvent[i].Type=='T')
          trick=m_FrameEvent[i].Value;
        // this could already help the precision but
        // it isn't meant to be complete nor accurate!
        if(trick&TRICK_LINE_PLUS_26)
          x+=52;
        else if(trick&TRICK_LINE_PLUS_24)
          x+=48;
        else if(trick&TRICK_0BYTE_LINE)
          x=-1;
        if(x>0)
          sdp+=x/2;
        sdp&=~1; // looks more serious...
        break;
      }
    }
  }//nxt
  return sdp;
}

#endif


DWORD TFrameEvents::GetShifterTricks(short y) {
  DWORD tricks=0;
  for(int i=1; i<=MAX_EVENTS && m_FrameEvent[i].Scanline<=y;i++)
  {
    if(m_FrameEvent[i].Scanline==y)
    {
      if(m_FrameEvent[i].Type=='T')
      {
        tricks=m_FrameEvent[i].Value;
        break;
      }
    }
  }//nxt
  return tricks;
}


void TFrameEvents::Init() {
  m_nEvents=m_nReports=TriggerReport=nVbl=0;
}


int TFrameEvents::Report() {
  FILE* fp;
  fp=fopen(FRAME_REPORT_FILENAME,"w"); // unique file name
  //ASSERT(fp);
  if(fp)
  {
#ifdef WIN32
    char sdate[9];
    char stime[9];
    _strdate(sdate);
    _strtime(stime);
    fprintf(fp,"Steem frame report - %s -%s \n%s WS%d C%d C%d C%d\n",
      sdate,stime,st_model_name[ST_MODEL],Mmu.WS[OPTION_WS],
      OPTION_C1,OPTION_C2*2,OPTION_C3*3);
#endif
    if(FloppyDrive[0].DiskInDrive())
      fprintf(fp,"Disk A: %s",FloppyDisk[0].DiskName.c_str());
    int i,j;
    for(i=1,j=-1;i<=m_nEvents;i++)
    {
      if(m_FrameEvent[i].Scanline!=j)
      {
        j=m_FrameEvent[i].Scanline;
        fprintf(fp,"\n%03d -",j);
      }
      int first_letter=m_FrameEvent[i].Type>>8;
      if(first_letter) // eg "TB" for Timer B
        fprintf(fp," %03d:%c%c%04X",m_FrameEvent[i].Cycle,first_letter,m_FrameEvent[i].Type,m_FrameEvent[i].Value);
      else // eg 'S' for Sync
      {
        if(m_FrameEvent[i].Type=='#') // decimal
          fprintf(fp," %03d:%c%04d",m_FrameEvent[i].Cycle,m_FrameEvent[i].Type,m_FrameEvent[i].Value);
        else  // hexa
          fprintf(fp," %03d:%c%04X",m_FrameEvent[i].Cycle,m_FrameEvent[i].Type,m_FrameEvent[i].Value);
      }
    }//nxt

    fprintf(fp,"\n--"); // so we know it was OK
    fclose(fp);
  }
  m_nReports++;
  return m_nReports;
}


void TFrameEvents::ReportLine() {
  // current line
  // look back
  int i=m_nEvents;
  while(i>1&&m_FrameEvent[i].Scanline==scan_y)
    i--;
  if(m_FrameEvent[i].Cycle>=508)
    i++; // former line
  TRACE("Y%d C%d ",scan_y,(short)LINECYCLES);
  for(;i<=m_nEvents;i++)
  {

    int first_letter=m_FrameEvent[i].Type>>8;
    if(first_letter) // eg "TB" for Timer B
      TRACE(" %03d:%c%c%04X",m_FrameEvent[i].Cycle,first_letter,m_FrameEvent[i].Type,m_FrameEvent[i].Value);
    else // eg 'S' for Sync
    {
      if(m_FrameEvent[i].Type=='#') // decimal
        TRACE(" %03d:%c%04d",m_FrameEvent[i].Cycle,m_FrameEvent[i].Type,m_FrameEvent[i].Value);
      else  // hexa
        TRACE(" %03d:%c%04X",m_FrameEvent[i].Cycle,m_FrameEvent[i].Type,m_FrameEvent[i].Value);
    }

/*
    if(m_FrameEvent[i].Type=='L' //|| m_FrameEvent[i].Type=='C' 
      ||m_FrameEvent[i].Type=='#') // decimal
      TRACE(" %d %03d:%c%04d",m_FrameEvent[i].Scanline,m_FrameEvent[i].Cycle,m_FrameEvent[i].Type,m_FrameEvent[i].Value);
    else  // hexa
      TRACE(" %d %03d:%c%04X",m_FrameEvent[i].Scanline,m_FrameEvent[i].Cycle,m_FrameEvent[i].Type,m_FrameEvent[i].Value);
      */
  }
  TRACE("\n");
}


int TFrameEvents::Vbl() {
  int rv=TriggerReport;
  if(TriggerReport==2&&m_nEvents)
    TriggerReport--;
  else if(TriggerReport==1&&m_nEvents)
  {
    Report();
    TriggerReport=FALSE;
  }
  nVbl++;
  m_nEvents=0;
  return rv;
}

#endif//#if defined(SSE_DEBUGGER_FRAME_REPORT)
