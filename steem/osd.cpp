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

DOMAIN: Rendering
FILE: osd.cpp
DESCRIPTION: Functions to create and draw Steem's on screen display that
appears when the emulator begins to run to give useful information.
Also disk track info, scrollers, debug info, FPS.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <osd.h>
#include <computer.h>
#include <translate.h>
#include <gui.h>
#include <draw.h>
#include <mymisc.h>
#include <stdarg.h>


bool osd_no_draw=false,osd_disable=false;
BYTE osd_show_plasma=4,osd_show_speed=4,osd_show_icons=4,osd_show_cpu=4;
bool osd_old_pos=false;
bool osd_show_disk_light=true,osd_show_scrollers=false;
DWORD osd_start_time,osd_scroller_start_time,osd_scroller_finish_time;
bool osd_shown_scroller=false;
long col_yellow[2],col_blue,col_red,col_green,col_white;
DWORD FDCCantWriteDisplayTimer=(0);
long col_fd_red[2],col_fd_green[2];
DWORD HDDisplayTimer=(0);
WIN_ONLY(HWND ResetInfoWin=(NULL); )
EasyStr osd_scroller;
EasyStringList osd_scroller_array;
DWORD *osd_plasma_pal=NULL;
BYTE *osd_plasma=NULL;
long *osd_font=NULL;
LPOSDBLUEIZELINEPROC osd_blueize_line;
LPOSDDRAWCHARPROC jump_osd_draw_char[4]={NULL,NULL,NULL,NULL},osd_draw_char,
jump_osd_draw_char_transparent[4]={NULL,NULL,NULL,NULL},osd_draw_char_transparent;
LPOSDDRAWCHARCLIPPEDPROC jump_osd_draw_char_clipped[4]={NULL,NULL,NULL,NULL},osd_draw_char_clipped,
jump_osd_draw_char_clipped_transparent[4]={NULL,NULL,NULL,NULL},osd_draw_char_clipped_transparent;
LPOSDBLACKRECTPROC jump_osd_black_box[4],osd_black_box;


void ASMCALL osd_draw_char_dont(long*,BYTE*,long,long,int,long,long) {}
void ASMCALL osd_draw_char_clipped_dont(long*,BYTE*,long,long,int,long,long,RECT*){}
void ASMCALL osd_blueize_line_dont(int,int,int) {}
void ASMCALL osd_black_box_dont(void*,int,int,int,int,long) {};



void osd_draw_begin() {
  switch(BytesPerPixel) {
#if !defined(SSE_VID_32BIT_ONLY)
  case 2:
    osd_blueize_line=(rgb555)?osd_blueize_line_16_555:osd_blueize_line_16_565;
    break;
  case 3:
    osd_blueize_line=osd_blueize_line_24; break;
#endif
  case 4:
    osd_blueize_line=osd_blueize_line_32; break;
#if !defined(SSE_VID_32BIT_ONLY)
  default: // 1!
    osd_blueize_line=osd_blueize_line_8; break;
#endif
  }
  osd_draw_char=jump_osd_draw_char[BytesPerPixel-1];
  osd_draw_char_clipped=jump_osd_draw_char_clipped[BytesPerPixel-1];
  osd_draw_char_transparent=jump_osd_draw_char_transparent[BytesPerPixel-1];
  osd_draw_char_clipped_transparent=
    jump_osd_draw_char_clipped_transparent[BytesPerPixel-1];
  osd_black_box=jump_osd_black_box[BytesPerPixel-1];
  col_yellow[0]=colour_convert(255,215,0);
  col_yellow[1]=colour_convert(200,170,0);
  col_red=colour_convert(255,0,0);
  col_blue=colour_convert(0,0,255);
  col_green=colour_convert(0,255,0);
  col_white=colour_convert(255,255,255);
  col_fd_green[0]=colour_convert(0,255,0);
  col_fd_green[1]=colour_convert(0,200,0);
  col_fd_red[0]=colour_convert(255,0,0);
  col_fd_red[1]=colour_convert(200,0,0);
}


void osd_draw_end() {
  osd_draw_char=osd_draw_char_dont;
  osd_draw_char_clipped=osd_draw_char_clipped_dont;
  osd_draw_char_transparent=osd_draw_char_dont;
  osd_draw_char_clipped_transparent=osd_draw_char_clipped_dont;
  osd_black_box=osd_black_box_dont;
  osd_blueize_line=osd_blueize_line_dont;
}


void osd_init_run(bool allow_scroller) {
  osd_start_time=timeGetTime();
  if(allow_scroller)
    osd_shown_scroller=0;
  else
  {
    osd_shown_scroller=true;
    osd_scroller_finish_time=0;
  }
}


void osd_init_draw_static() {
  osd_start_time=timer;
  osd_shown_scroller=true;
  osd_scroller_finish_time=0;
}


EasyStr get_osd_scroller_text(int n) {
  EasyStr ret=osd_scroller_array[n].String;
  char *p=strchr(ret.Text,'§'),c;	// look for §
  //TODO where is the key? + multi in Linux
  int ic;
  while(p)
  {	// found
    ic=(int)(p-ret.Text);
    c=ret[ic+1];
    switch(c) {	// translate special code that follows
    case 'V':case 'v':
      //ret=ret.Lefts(ic)+"Steem Engine v"+(char*)stem_version_text+(ret.Text+ic+2);
      ret=ret.Lefts(ic)+"Steem SSE v"+(char*)stem_version_text+(ret.Text+ic+2);
      break;
    case 'B':case 'b':
      ret=ret.Lefts(ic)+(char*)stem_version_date_text+(ret.Text+ic+2);
      break;
    case 'D':case 'd':
      if(FloppyDrive[0].Empty())
        ret=ret.Lefts(ic)+"NO DISK"+(ret.Text+ic+2);
      else
        ret=ret.Lefts(ic)+FloppyDrive[0].GetDisk()+(ret.Text+ic+2);
      break;
    default:
      ret=ret.Lefts(ic)+(ret.Text+ic+2);
      break;
    }
    p=strchr(ret.Text+ic+1,'§');
  }
  return ret;
}


void osd_pick_scroller() {
  if(osd_show_scrollers==0) 
    return;
  if(osd_scroller_array.NumStrings==0) 
    return;
  srand(osd_start_time+osd_scroller_start_time); // the best is 'Steem is the best'
  if((rand()%CHANCE_OF_SCROLLER)!=0) 
    return;
  int n=(rand()%osd_scroller_array.NumStrings);
  osd_scroller=get_osd_scroller_text(n);
  strupr(osd_scroller.Text);
  osd_shown_scroller=true;
  osd_scroller_start_time=timer+100;
  osd_scroller_finish_time=osd_scroller_start_time+20*20
    +(int)osd_scroller.Length()*4*20+(1280/4*20);
  OsdControl.ScrollerPosition=0;
}

/*
void osd_start_scroller(char *t) {
  osd_scroller=t;
  strupr(osd_scroller.Text);
  osd_start_time=0;
  osd_shown_scroller=true;
  osd_scroller_start_time=timer+100;
  osd_scroller_finish_time=osd_scroller_start_time+20*20
    +(int)osd_scroller.Length()*4*20+(1280/4*20);
}
*/

bool osd_is_on(bool timed_only) {
  if(timed_only)
  {
    int seconds=MAX(MIN((timeGetTime()-osd_start_time)/1000,DWORD(30)),DWORD(0));
    if(seconds>=30) 
      return 0;
    if(osd_show_plasma<OSD_SHOW_ALWAYS) 
      if(seconds<osd_show_plasma) return true;
    if(osd_show_speed<OSD_SHOW_ALWAYS) 
      if(seconds<osd_show_speed) return true;
    if(osd_show_icons<OSD_SHOW_ALWAYS) 
      if(seconds<osd_show_icons) return true;
    if(osd_show_cpu<OSD_SHOW_ALWAYS) 
      if(seconds<osd_show_cpu) return true;
    return 0;
  }
  else
  {
    return bool(osd_show_plasma||osd_show_speed||osd_show_icons||
      osd_show_cpu||osd_show_disk_light ||osd_show_scrollers);
  }
}


void osd_draw() {
#ifndef ONEGAME
  if(osd_no_draw||osd_disable) 
    return;
#if defined(SSE_OSD_SHOW_TIME)
  if(OPTION_OSD_TIME && OsdControl.StartingTime)
  {
    DWORD ms=timer-OsdControl.StartingTime;
    DWORD s=ms/1000;
    DWORD h=s/(60*60);
    s=s%(60*60);
    DWORD m=s/60;
    s=s%60;
    TRACE_OSD2("%02d:%02d:%02d",h,m,s);
  }
#endif
  int x1,y1;
  x1=draw_blit_source_rect.right-draw_blit_source_rect.left;
  if(FullScreen&&!screen_res && SCANLINES_OK
#ifdef WIN32
    &&(!OPTION_C3||draw_win_mode[screen_res]==DWM_STRETCH)
#endif    
    )
    x1/=2;
  y1=draw_blit_source_rect.bottom-draw_blit_source_rect.top;
  bool can_have_scroller=true;
  int seconds=MAX(MIN((timer-osd_start_time)/1000,DWORD(30)),DWORD(0));
  int icon_x=x1-5-OSD_ICON_SIZE;
  int icon_y=y1-5-OSD_ICON_SIZE;
  if(osd_old_pos) 
    icon_x=x1-18-OSD_ICON_SIZE,icon_y=y1-18-OSD_ICON_SIZE;
#ifdef WIN32 //TODO   
  if(!SSEConfig.IsInit)
  {
    const BYTE nlines=4;
    EasyStr advice[nlines];
    int line=0;
    advice[line++]=T("WELCOME TO STEEM 4");
    advice[line++]=T("F1: HELP");
    advice[line++]=T("F11: MOUSE ON/OFF");
    advice[line++]=T("F12: START/STOP");
    DWORD col=col_yellow[0];
    RECT cliprect;
    GetClientRect(StemWin,&cliprect);
    int start_y=0;
    for(line=0;line<nlines;line++)
    {
      int x=0;
      for(int i=0;i<advice[line].Length();i++)
      {
        int n=(int)(advice[line].Text[i])+(60-33);
        if(n>=60&&n<120)
          osd_draw_char_clipped(osd_font+(n*64),draw_mem,x,start_y+26/2
            -OSD_LOGO_H/2+line*30,draw_line_length,col,32,&cliprect);
        x+=16;
      }
    }
  }
#endif//WIN32  
  if(icon_x<0||icon_y<0) // if source rect = 0,0,0,0 
    return;
  if(osd_show_icons)
  {
    if(fast_forward)
    {
      if(draw_grille_black<4) 
        draw_grille_black=4;
      osd_draw_char(osd_font+(34*64),draw_mem,icon_x,icon_y,
        draw_line_length,col_green,OSD_ICON_SIZE);
    }
    else if(sound_record)
    {
      if(draw_grille_black<4) 
        draw_grille_black=4;
      osd_draw_char(osd_font+(36*64),draw_mem,icon_x,icon_y,
        draw_line_length,col_red,OSD_ICON_SIZE);
    }
  }
  if(seconds<osd_show_plasma)
  {
    //TRACE2("F%d plasma\n",FRAME);
    //TRACE("seconds %d osd_show_plasma %d\n",seconds,osd_show_plasma);
#define PLASMA_W ((5+1)*32)
#define PLASMA_H 26
#define PLASMA_MAX 32
    int x=x1/2-PLASMA_W/2;
    int start_y=(osd_old_pos) ? 25 : 4;
    int frame=14;
#if !defined(SSE_VID_32BIT_ONLY)
    if(BytesPerPixel==1)
    {
      osd_black_box(draw_mem,x-1,start_y,PLASMA_W+2,PLASMA_H+2,draw_line_length);
      for(int y=start_y+1;y<start_y+1+PLASMA_H;y++) 
        osd_blueize_line(x,y,PLASMA_W);
    }
    else
#endif
    {
      if(osd_plasma_pal==NULL)
      {
        osd_plasma_pal=new DWORD[PLASMA_MAX*2];
        osd_plasma=new BYTE[PLASMA_W*PLASMA_H];
        BYTE *p=osd_plasma;
        for(int py=0;py<PLASMA_H;py++)
        {
          for(int px=0;px<PLASMA_W;px++)
          {
            *(p++)=(BYTE)(PLASMA_MAX/2+double(PLASMA_MAX/2-1)
              *sin(hypot(px+PLASMA_W/8,(PLASMA_H/2-py)*4)/16));
          }//nxt
        }//nxt
      }//if
      DWORD end_time=osd_start_time+osd_show_plasma*1000-500;
      if(osd_show_plasma==OSD_SHOW_ALWAYS) 
        end_time=timer+20;
      if(timer>=end_time)
        frame=MIN(15-int(timer-end_time)/20,14);
      else
        frame=MIN(int(timer-(osd_start_time+200))/20,14);
      if(frame>=0)
        osd_draw_plasma(x,start_y,frame);
    }
    if(frame==14)
    {
      x=x1/2-OSD_LOGO_W/2;
#if defined(SSE_BUILD)
/*  We write 'STEEM X.X.X', XXX being the version, in the nice plasma with
    scroller letters and digits instead of the 'Steem 3.2' graphics.
    The result isn't too bad.
    Now we can access the graphics in charset.blk, but not sure drawing the
    digits ourselve would be better? TODO
*/
#define BUFFER_LENGTH sizeof("STEEM 4.0.0")
      char tmp_buffer[BUFFER_LENGTH];
      sprintf(tmp_buffer,"STEEM %s",(char*)stem_version_text);
      for(unsigned int i=0;i<BUFFER_LENGTH-1;i++)
      {
        int n=(int)(tmp_buffer[i])+(60-33);	// need macro?
        if(tmp_buffer[i]=='.')
          x-=4; // closer when dot
        if(n>=60&&n<120)
          osd_draw_char(osd_font+(n*64),draw_mem,x-11-8,
            start_y+PLASMA_H/2-OSD_LOGO_H-1,draw_line_length,col_white,32);
        x+=16;
        if(tmp_buffer[i]=='.')
          x-=4;
      }//nxt i
#undef BUFFER_LENGTH
#else // this is a graphic included in the font
      for(int c=0;c<OSD_LOGO_W/32+1;c++)
      {
        osd_draw_char(osd_font+((50+c)*64),draw_mem,x,start_y+PLASMA_H/2
          -OSD_LOGO_H/2,draw_line_length,col_white,OSD_LOGO_H);
        x+=32;
      }
#endif//logo
    }
    if(draw_grille_black<4) 
      draw_grille_black=4;
  }
  else if(osd_plasma_pal)
  {
    delete[] osd_plasma_pal; osd_plasma_pal=NULL;
    delete[] osd_plasma;     osd_plasma=NULL;
  }
  if(seconds<osd_show_speed)
  {
    if(avg_frame_time && runstate==RUNSTATE_RUNNING)
    {
      can_have_scroller=0;
      int real_bar_h=(osd_old_pos?18:12),bar_h;
      DWORD end_time=osd_start_time+osd_show_speed*1000-500;
      if(osd_show_speed==OSD_SHOW_ALWAYS) end_time=timer+20;
      if(timer>=end_time)
        bar_h=real_bar_h-int(timer-end_time)/20;
      else
        bar_h=MIN(int(timer-(osd_start_time+100))/20,real_bar_h);
      if((bar_h+1+1)>0)
      {
        int bar_w=120,bar_x=6,bar_y=y1-5-12-bar_h/2;
        if(osd_old_pos) 
          bar_w=100,bar_x=20,bar_y=y1-40;
        double speed=((1000*12)/Glue.video_freq)/double(avg_frame_time);
        int w=MIN(MAX(int(double(bar_w-1)*speed),2),x1-bar_x);
        osd_black_box(draw_mem,bar_x-1,bar_y-1,1+bar_w+1,1+bar_h+1,
          draw_line_length);
        for(int y=bar_y;y<bar_y+bar_h;y++) 
          osd_blueize_line(bar_x,y,w);
        if(draw_grille_black<4) 
          draw_grille_black=4;
      }
    }
  }
  if(seconds<osd_show_cpu)
  {
    if(n_cpu_cycles_per_second>CpuNormalHz)
    {
      can_have_scroller=0;
      int bar_w=120,bar_x=5,cpu_y=y1-5-12+6-15;
      if(osd_old_pos) 
        bar_w=100,bar_x=20,cpu_y=y1-18-32;
      int x=n_cpu_cycles_per_second/CpuNormalHz;
      x=bar_x+bar_w+10-x+((timer&15)*x)/int(osd_old_pos?8:16);
      osd_draw_char(osd_font+(int(osd_old_pos?40:41)*64),draw_mem,(x),cpu_y,
        draw_line_length,col_red,20);
      if(draw_grille_black<4) 
        draw_grille_black=4;
    }
  }
  if(seconds<osd_show_icons)
  {
    can_have_scroller=0;
    if(runstate==RUNSTATE_RUNNING)
    {
      if(fast_forward==0&&sound_record==0)
      {
        // Play icon
        int full_h=OSD_ICON_SIZE,h;
        DWORD end_time=osd_start_time+osd_show_icons*1000-500;
        if(osd_show_icons==OSD_SHOW_ALWAYS) end_time=timer+20;
        if(timer>=end_time)
          h=MIN(full_h+1-int(timer-end_time)/20,full_h);
        else
          h=MIN(int(timer-(osd_start_time+100))/20,full_h);
        if(h==full_h)
        {
          osd_draw_char(osd_font+(33*64)-2,draw_mem,icon_x,icon_y-1,
            draw_line_length,colour_convert(255,255,0),OSD_ICON_SIZE);
          if(draw_grille_black<4) 
            draw_grille_black=4;
        }
        else if(h>0)
        {
          osd_draw_char(osd_font+(33*64)-2,draw_mem,icon_x,
            icon_y-1+full_h/2-(h+1)/2,
            draw_line_length,colour_convert(255,255,0),(h+1)/2);
          if(h>1)
          {
            osd_draw_char(osd_font+(33*64)-2+full_h*2-(h & ~1),draw_mem,
              icon_x,icon_y-1+full_h/2,
              draw_line_length,colour_convert(255,255,0),h/2);
          }
          if(draw_grille_black<4) 
            draw_grille_black=4;
        }
      }
    }
#if 0
    else
    {
      if(!fast_forward)
      { // this draws a blue square
        osd_draw_char(osd_font+(35*64),draw_mem,icon_x,icon_y,draw_line_length,
          col_blue,OSD_ICON_SIZE);
        if(draw_grille_black<4) 
          draw_grille_black=4;
      }
    }
#endif
  }
  if(OsdControl.MessageTimer>timer)// || seconds<osd_show_speed)
  {
    DWORD col=col_yellow[0];
#define THE_LEFT 0//(x1/2)
#define THE_RIGHT x1//(x1/2)//((x1))
    // TODO refactor in basic function?
    RECT cliprect={THE_LEFT,0,THE_RIGHT,y1};
    int x=0;
    int start_y=0+8;
    for(unsigned int i=0;i<strlen(OsdControl.m_OsdMessage);i++)
    {
      int n=(int)(OsdControl.m_OsdMessage[i])+(60-33);
      if(n>=60&&n<120)
        osd_draw_char_clipped(osd_font+(n*64),draw_mem,x,start_y
          +PLASMA_H/2-OSD_LOGO_H/2,draw_line_length,col,32,&cliprect);
      x+=16;
    }//nxt i
    if(draw_grille_black<4) 
      draw_grille_black=4;
#undef THE_LEFT
#undef THE_RIGHT
  }
  // Green led for floppy disk read; red for write.
  if(OPTION_DRIVE_INFO||osd_show_disk_light)
  {
    Dma.UpdateRegs();
    bool FDCWriting=Fdc.WritingToDisk();
    if((Fdc.str&0x80) && (psg_reg[PSGR_PORT_A]&6)!=6
      &&FloppyDrive[0].ImageType.Manager!=MNGR_PRG
      && (FloppyDrive[DRIVE].m_DiskInDrive||ADAT))
    {
      int idx=32,w=20;
      if(draw_blit_source_rect.bottom>200+BORDER_TOP+BORDER_BOTTOM)
        idx=37,w=32;
      DWORD col=(FDCWriting)
        ?col_fd_red[(hbl_count/512)&1]:col_fd_green[(hbl_count/512)&1];
      if(!OPTION_DRIVE_INFO&&osd_show_disk_light)
      {
        if(FDCCantWriteDisplayTimer>timer)
        {
          col=col_red;
          osd_draw_char(osd_font+(38*64),draw_mem,(x1-w)-24,1,draw_line_length,
            col_red,16);
          if(((FDCCantWriteDisplayTimer-timer)%500)<=250)
            osd_draw_char(osd_font+(39*64),draw_mem,(x1-w)-24,1,
              draw_line_length,col_red,16);
        }
        osd_draw_char(osd_font+(idx*64),draw_mem,(x1-w)-4,4,
          draw_line_length,col,8);
        if(draw_grille_black<4) 
          draw_grille_black=4;
      }
      // Display drive, side, track, sector
      if(OPTION_DRIVE_INFO)
      {
#define THE_LEFT 0//(x1/2)
#define THE_RIGHT ((x1))
#ifdef SSE_DEBUG
#define BUFFER_LENGTH (10+4+2+3) //4bytes for ext, 2bytes for command
#elif defined(SSE_OSD_DEBUGINFO)
#define BUFFER_LENGTH (10+4+3+3) //4bytes for ext, 3bytes for command
#else
#define BUFFER_LENGTH (10+2) //"D:S-TR-SEC" drive-side-track-sector
#endif
        RECT cliprect={THE_LEFT,0,THE_RIGHT,y1}; //refactor...
        char tmp_buffer[BUFFER_LENGTH];
        char drive_letter=Psg.CurrentDrive()==TYM2149::NO_VALID_DRIVE?'?'
          : 'A' + DRIVE;
#ifdef SSE_DEBUG // add current command (CR)
        sprintf(tmp_buffer,"%2X-%C:%d-%02d-%02d",Fdc.cr,drive_letter,
          CURRENT_SIDE,CURRENT_TRACK,Fdc.sr);
#elif defined(SSE_OSD_DEBUGINFO)
        if(OPTION_OSD_DEBUGINFO)  // add current command (CR)
          sprintf(tmp_buffer,"%2X-%C:%d-%02d-%02d",Fdc.cr,drive_letter,
            CURRENT_SIDE,CURRENT_TRACK/*Fdc.tr*/,Fdc.sr);
        else
          sprintf(tmp_buffer,"%C:%d-%02d-%02d",'A'+DRIVE,
          CURRENT_SIDE,CURRENT_TRACK,Fdc.sr);
#else // not Debug
        sprintf(tmp_buffer,"%C:%d-%02d-%02d",drive_letter,
          CURRENT_SIDE,CURRENT_TRACK,Fdc.sr);
#endif
        //ASSERT(strlen(tmp_buffer)<BUFFER_LENGTH);
        DWORD drive_info_length=(int)strlen(tmp_buffer);
        int x=x1-(drive_info_length+1)*16;
        int start_y=0+8;
        for(unsigned int i=0;i<drive_info_length;i++)
        {
          int n=(int)(tmp_buffer[i])+(60-33);
          if(n>=60&&n<120)
            osd_draw_char_clipped(osd_font+(n*64),draw_mem,x,start_y+PLASMA_H/2
              -OSD_LOGO_H/2,draw_line_length,col,32,&cliprect);
          x+=16;
        }//nxt i
        if(draw_grille_black<4) 
          draw_grille_black=4;
#undef THE_LEFT
#undef THE_RIGHT
#undef BUFFER_LENGTH
      }
    }
    // Hard disk activity
    if(HDDisplayTimer>timer)
    {
      int idx=32,w=20;
      if(draw_blit_source_rect.bottom>200+BORDER_TOP+BORDER_BOTTOM)
        idx=37,w=32;
      DWORD col=col_yellow[(hbl_count/512)&1];
      osd_draw_char(osd_font+(idx*64),draw_mem,(x1-w)-4,4,
        draw_line_length,col,8);
    }
  }
  /*
  if(OsdControl.ScrollerPhase==TOsdControl::WANT_SCROLLER)
  {
    osd_start_scroller(OsdControl.ScrollText);
    OsdControl.ScrollerPhase=TOsdControl::SCROLLING;
    osd_shown_scroller=true;
    osd_scroller_start_time=timer+100;
    osd_scroller_finish_time=osd_scroller_start_time+20*20
      +(int)osd_scroller.Length()*4*20+(1280/4*20);
  }
  else */
  if(OsdControl.ScrollerPhase==TOsdControl::SCROLLING)
    ;
  else
  if(can_have_scroller && osd_shown_scroller==0)
  {
    osd_shown_scroller=true;
    osd_scroller_finish_time=0;
    osd_pick_scroller();
  }
  else if(osd_shown_scroller && osd_scroller_finish_time
    && timer>osd_scroller_finish_time+10*60*1000)
  {
    osd_shown_scroller=false;
    osd_scroller_finish_time=0;
  }
  else if(osd_shown_scroller && timer<osd_scroller_finish_time
    ||OsdControl.ScrollerPhase==TOsdControl::SCROLLING)
  {
    //int pos=(timer-osd_scroller_start_time)/20;
    OsdControl.ScrollerPosition++; // follows ST frequency, can be smooth if vsynced at that freq
    int pos=OsdControl.ScrollerPosition;
    if(pos>=20)
    {
      int i=(pos-20)/4;
      int xo=x1-4*((pos-20)&3),x;
      x=xo;
#define THE_LEFT (0)
#define THE_RIGHT ((x1))
      RECT cliprect={THE_LEFT,0,THE_RIGHT,y1};
      int scroll_len=(int)osd_scroller.Length();
      while(x>(THE_LEFT-BORDER_SIDE))
      {
        if(i<scroll_len)
        {
          //long colour=(OsdControl.ScrollerPhase==TOsdControl::SCROLLING)?
            //OsdControl.ScrollerColour:col_white;
          long colour=colour_convert(255,0,0);
          int n=int(osd_scroller[i])+(60-33);
          if(n>=60&&n<120)
          {
            if(x>=THE_LEFT && x<THE_RIGHT-BORDER_SIDE)
            {
              osd_draw_char(osd_font+(n*64),draw_mem,x,y1-24-5,
                draw_line_length,colour,32);
            }
            else if(x<THE_RIGHT)
            {
              osd_draw_char_clipped(osd_font+(n*64),draw_mem,x,y1-24-5
                ,draw_line_length,colour,32,&cliprect);
            }
          }
          if(i==(scroll_len-1))
          {
            if((x+16)<THE_LEFT)
            {
              OsdControl.ScrollerPhase=TOsdControl::NO_SCROLLER;
              //osd_scroller_finish_time=0; // keep for multi
            }
          }
        }
        x-=16;
        if((--i)<0) 
          break;
      }
#undef THE_LEFT
#undef THE_RIGHT
      if(draw_grille_black<4) 
        draw_grille_black=4;
    }
  }
#endif // ONEGAME
}


void osd_draw_plasma(int x,int start_y,int frame) {
  //ASSERT(draw_lock);
  if(draw_lock==0) 
    return;
  double tr=double(timer)/1024,tg=double(timer)/4096,tb=double(timer)/2048;
  for(int i=0;i<PLASMA_MAX*2;i++)
  {
    osd_plasma_pal[i]=colour_convert(BYTE(172+63.0*cos(i*M_PI/32+tr)),
      BYTE(172+63.0*sin(i*M_PI/32+tg)),
      BYTE(172-63.0*cos(i*M_PI/32+tb)));
  }
  int idx1_list[PLASMA_H],idx2_list[PLASMA_H];
  int base_y=timer/32;
  for(int y=0;y<PLASMA_H;y++)
  {
    idx1_list[y]=int(y*PLASMA_W+PLASMA_W/4-double(PLASMA_W/4)
      *sin(double(base_y+y*2)/PLASMA_H));
    idx2_list[y]=int(y*PLASMA_W+PLASMA_W/4+double(PLASMA_W/4)
      *sin(double(base_y+y/2)/PLASMA_H));
  }
  DWORD *p_fuji=LPDWORD(osd_font+frame*64);
  BYTE *tl_adr=draw_mem+BytesPerPixel*x;
  int y_offset=(start_y+1)*draw_line_length;
  DWORD black_pal=0;
  for(int y=0;y<PLASMA_H;y++)
  {
    int idx1=idx1_list[y],idx2=idx2_list[y];
    BYTE *p=(BYTE*)(tl_adr+y_offset);
    DWORD fuji_mask=*(p_fuji++);
    DWORD fuji_data=*(p_fuji++);
    DWORD bitmask=1;
    for(int px=0;px<PLASMA_W/2;px++)
    {
      for(int pixel=0;pixel<2;pixel++)
      {
        bool draw_data=(fuji_data & bitmask)!=0;
        bool draw_mask=(fuji_mask & bitmask)!=0;
        bitmask<<=1;
        if(frame==14)
        {
          if(px==0&&pixel==0)
            draw_mask=draw_data,draw_data=0;
          else if(px==PLASMA_W/2-1&&pixel==1)
            draw_mask=draw_data,draw_data=0;
        }
        if(pixel==(y&1)) 
          draw_data=0;
        BYTE *p_pal=NULL;
        if(draw_data) 
          p_pal=LPBYTE(osd_plasma_pal+osd_plasma[idx1]+osd_plasma[idx2]);
        else if(draw_mask) 
          p_pal=LPBYTE(&black_pal);
        if(p_pal)
        {
          switch(BytesPerPixel) {
#if !defined(SSE_VID_32BIT_ONLY)
          case 2:
            *LPWORD(p)=*LPWORD(p_pal); break;
          case 3:
            *LPWORD(p)=*LPWORD(p_pal);
            *LPBYTE(p+2)=*LPBYTE(p_pal+2); break;
#endif
          case 4:
            *LPDWORD(p)=*LPDWORD(p_pal); break;
          }
        }
        p+=BytesPerPixel;
      }
      idx1++; idx2++;
      if(bitmask==0) 
        bitmask=1;
    }
    y_offset+=draw_line_length;
  }
}


void osd_hide() {
  osd_start_time=0;
  osd_shown_scroller=true;
  osd_scroller_finish_time=0;
}


#ifndef ONEGAME

void osd_get_reset_info(EasyStringList *sl) {
  sl->Sort=eslNoSort;
  Str t;
  t=Str(T("Machine: "))+st_model_name[ST_MODEL]
    +Str(" TOS: v")+HEXSl(tos_version,3).Insert(".",1);
  sl->Add(t);
  sl->Add(T("Memory size")+": "+(mem_len/1024)+"Kb");
  t=T("Monitor")+": ";
  if(extended_monitor)
    t+=T("Extended Monitor At")+" "+em_width+"x"+em_height+"x"+em_planes;
  else
  {
    if(MONO)
      t+=T("Monochrome")+" ("+T("High Resolution")+")";
    else
      t+=T("Colour")+" ("+T("Low/Med Resolution")+")";
  }
  sl->Add(t);
  sl->Add(T("ST CPU speed")+": "+(n_millions_cycles_per_sec)+" "
    +T("Megahertz"));
  t=T("Active drives")+": A";
  if(num_connected_floppies==2) t+=", B";
  for(int n=2;n<26;n++) if(mount_flag[n]) t+=Str(", ")+char('A'+n);
  sl->Add(t);
  if(pasti_active)
    t=T("Pasti disk emulation enabled");
  else  if(floppy_instant_sector_access==0)
    t=T("Drive speed")+": "+T("Slow");
  else
    t=T("Drive speed")+": "+T("Fast");
  if(t[0]) 
    sl->Add(t);
  t=T("Active ports")+": ";
  if(MIDIPort.IsOpen()) 
    t+="MIDI ";
  if(ParallelPort.IsOpen()) 
    t+=T("Parallel")+" ";
  if(SerialPort.IsOpen()) 
    t+=T("Serial");
#if defined(SSE_DONGLE_PORT) 
  if(DONGLE_ID)
    t+=T("dongle/adapter");
#endif
  if(NotSameStr(t.Rights(2),": ")) 
    sl->Add(t);
  if(cart)
  {
    Str Name=GetFileNameFromPath(CartFile);
    char *dot=strrchr(Name,'.');
    if(dot) 
      *dot=0;
    t=T("Cartridge")+": "+Name;
    sl->Add(t);
  }
}


#ifdef WIN32
void osd_draw_reset_info(HDC dc)
#else
void osd_draw_reset_info(int win_x,int win_y,int win_w,int win_h)
#endif
{
  EasyStringList sl;
  osd_get_reset_info(&sl);
#ifdef WIN32
  int th=GetTextSize(fnt,sl[0].String).Height;
  int max_tw=9999;
#endif
#ifdef UNIX
  int th=hxc::font->ascent+hxc::font->descent;
  int max_tw=MAX(win_w-15-15,200);
#endif
  int info_h=(th+2)*sl.NumStrings;
  int tw=0,hi_tw=0;
  for(int n=sl.NumStrings-1;n>=0;n--)
  {
    UNIX_ONLY(tw=XTextWidth(hxc::font,sl[n].String,strlen(sl[n].String)); )
    WIN_ONLY(tw=GetTextSize(fnt,sl[n].String).Width; )
    if(n==0) 
      tw+=2+RC_FLAG_WIDTH;
    if(tw>=max_tw)
    {
      Str new_str=sl[n].String;
      if(new_str.RightChar()=='.')
        new_str.Delete(new_str.Length()-3,1);
      else
      {
        new_str.Delete(new_str.Length()-1,1);
        new_str+="..";
      }
      sl.SetString(n,new_str);
      n++;
    }
    else if(tw>hi_tw)
      hi_tw=tw;
  }
  int x=5,y=3;  
#ifdef WIN32
  HDC osd_ri_dc=dc;
  HANDLE old_font=SelectObject(osd_ri_dc,fnt);
  RECT fr={x-5,y-3,x+hi_tw+5,y+info_h+1};
  FrameRect(osd_ri_dc,&fr,(HBRUSH)GetStockObject(BLACK_BRUSH));
  fr.left++;fr.top++; fr.right--;fr.bottom--;
  FillRect(osd_ri_dc,&fr,(HBRUSH)GetStockObject(WHITE_BRUSH));
#endif
#ifdef UNIX
  XSetFont(XD,DispGC,hxc::font->fid);
  int px=win_x+win_w/2-(hi_tw+10)/2,py=win_y+(win_h/2)-(info_h/2);
  XSetForeground(XD,DispGC,WhiteCol);
  XFillRectangle(XD,StemWin,DispGC,px-4,py-2,hi_tw+8,info_h+2);
  hxc::draw_border(XD,StemWin,DispGC,px-5,py-3,hi_tw+10,info_h+3,1,
    BlackCol,BlackCol);
#endif
  for(int n=0;n<sl.NumStrings;n++)
  {
    WIN_ONLY(TextOut(osd_ri_dc,x,y,sl[n].String,(int)strlen(sl[n].String)); )
    UNIX_ONLY(XDrawString(XD,StemWin,DispGC,x,y+hxc::font->ascent,sl[n].String,strlen(sl[n].String)); )
    if(n==0&&tos_version)
    {
      int FlagIdx=OptionBox.TOSLangToFlagIdx(ROM_PEEK(0x1d));
#ifdef WIN32
      if(FlagIdx>=0)
      {
        HDC TempDC=CreateCompatibleDC(dc);
        HANDLE OldBmp=SelectObject(TempDC,LoadBitmap(Inst,"TOSFLAGS"));
        BitBlt(osd_ri_dc,x+tw-RC_FLAG_WIDTH,y+MAX(th-RC_FLAG_HEIGHT,0)/2,
          RC_FLAG_WIDTH,RC_FLAG_HEIGHT,TempDC,FlagIdx*RC_FLAG_WIDTH,0,SRCCOPY);
        DeleteObject(SelectObject(TempDC,OldBmp));
        DeleteDC(TempDC);
      }
#endif
#ifdef UNIX
      IcoTOSFlags.DrawIcon(FlagIdx,StemWin,DispGC,x+tw-RC_FLAG_WIDTH,y+MAX(th-RC_FLAG_HEIGHT,0)/2);
#endif
    }
    y+=th+2;
  }
  WIN_ONLY(SelectObject(dc,old_font); )
}

#else//#ifndef ONEGAME

void osd_draw_reset_info(HDC) {}

#endif


#ifdef WIN32

LRESULT CALLBACK ResetInfoWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar) {
#ifndef ONEGAME
  switch(Mess) {
  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    BeginPaint(Win,&ps);
    osd_draw_reset_info(ps.hdc);
    EndPaint(Win,&ps);
    return 0;
  }
  case WM_USER:
  {
    if(wPar!=1789) 
      break;
    // Update size and position
    EasyStringList sl;
    osd_get_reset_info(&sl);
    RECT rc;
#if defined(SSE_VID_DD)
    if(FullScreen)
    {
      get_fullscreen_rect(&rc);
      rc.top-=MENUHEIGHT;
    }
    else
#endif
    {
      GetClientRect(StemWin,&rc);
      rc.bottom-=2;
      rc.top+=MENUHEIGHT+2;
      rc.left+=2;
    }
    int th=GetTextSize(fnt,sl[0].String).Height;
    int info_h=(th+2)*sl.NumStrings;
    int tw,max_tw=0;
    for(int n=sl.NumStrings-1;n>=0;n--)
    {
      tw=GetTextSize(fnt,sl[n].String).Width;
      if(n==0) tw+=2+8;
      if(tw>max_tw) max_tw=tw;
    }
    int x=rc.left+(rc.right-rc.left)/2-(max_tw+10)/2;
    SetWindowPos(Win,NULL,x,rc.bottom-info_h-4,max_tw+10,info_h+4,SWP_NOZORDER);
    return 0;
  }//case
  }//sw
#endif
  return DefWindowProc(Win,Mess,wPar,lPar);
}

#endif


#if !defined(SSE_VID_32BIT_ONLY)

void ASMCALL osd_blueize_line_8(int x1,int y,int w) {
  BYTE*p=(BYTE*)(draw_mem+y*draw_line_length);
  p+=x1; //x1 pixels from left
  int n=0;
  if(y&1)
  {
    n=1;
    p++;
  }
  for(;n<w;n+=2)
  { //blueize w pixels
    //.Rrr rrGg gggB bbbb  0x7c00 = red, 0x03e0 = green, 0x1f = blue
    *p=*((BYTE*)&col_blue);
    p+=2;
    //             ^darken                    ^blueize
  }
}

#endif

#if defined(SSE_OSD_EXTRACT_GRAPHICS) // one-time switch
typedef struct {
    int width;
    int height;
    uint8_t *data;
    size_t size;
} pbm_image;

size_t pbm_save(pbm_image *img, FILE *outfile) {
    size_t n = 0;
    n += fprintf(outfile, "P4 %d %d\n",img->width,img->height);
    n += fwrite(img->data, 1, img->size, outfile);
    return n;
}
#endif


void osd_routines_init() {
#if !defined(SSE_VID_32BIT_ONLY)
  jump_osd_draw_char[0]=osd_draw_char_8;
  jump_osd_draw_char_clipped[0]=osd_draw_char_clipped_8;
  jump_osd_black_box[0]=osd_black_box_8;
  jump_osd_draw_char_transparent[0]=osd_draw_char_8;
  jump_osd_draw_char_clipped_transparent[0]=osd_draw_char_clipped_8;
  jump_osd_draw_char[1]=osd_draw_char_16;
  jump_osd_draw_char_clipped[1]=osd_draw_char_clipped_16;
  jump_osd_black_box[1]=osd_black_box_16;
  jump_osd_draw_char_transparent[1]=osd_draw_char_transparent_16;
  jump_osd_draw_char_clipped_transparent[1]=osd_draw_char_clipped_transparent_16;
  jump_osd_draw_char[2]=osd_draw_char_24;
  jump_osd_draw_char_clipped[2]=osd_draw_char_clipped_24;
  jump_osd_black_box[2]=osd_black_box_24;
  jump_osd_draw_char_transparent[2]=osd_draw_char_transparent_24;
  jump_osd_draw_char_clipped_transparent[2]=osd_draw_char_clipped_transparent_24;
#endif
  jump_osd_draw_char[3]=osd_draw_char_32;
  jump_osd_draw_char_clipped[3]=osd_draw_char_clipped_32;
  jump_osd_black_box[3]=osd_black_box_32;
  jump_osd_draw_char_transparent[3]=osd_draw_char_transparent_32;
  jump_osd_draw_char_clipped_transparent[3]=osd_draw_char_clipped_transparent_32;
  osd_draw_char=osd_draw_char_dont;
  osd_draw_char_clipped=osd_draw_char_clipped_dont;
  osd_draw_char_transparent=osd_draw_char_dont;
  osd_draw_char_clipped_transparent=osd_draw_char_clipped_dont;
  osd_black_box=osd_black_box_dont;
  osd_blueize_line=osd_blueize_line_dont;
#ifdef WIN32
  {
    HRSRC res;
    HGLOBAL hglob;
    res=FindResource(NULL,"OSD_FONT_BLOCK",RCNUM(300) /*RT_RCDATA*/);
    if(res)
    {
      hglob=LoadResource(NULL,res);
      if(hglob) osd_font=(long*)LockResource(hglob);
#if defined(SSE_OSD_EXTRACT_GRAPHICS)
      // finally could produce graphics of charset.blk!
      // to edit and save back we used other tools: irfanview, paint, winhex
      pbm_image descr;
      descr.width=32*2; // off + on
      descr.height=30720/(descr.width/8);
      descr.size=(descr.width/8)*descr.height;
      int ndwords=descr.size/4;
      DWORD *dwp=(DWORD*)osd_font;
      DWORD *dw=new DWORD[ndwords];
      for(int i=0;i<ndwords;i++)
      {
        dw[i]=dwp[i];
        SWAP_BIG_ENDIAN_DWORD(dw[i]); // pbm is big-endian
      }
      descr.data=((BYTE*)dw);
      FILE *outfile=fopen("charset.pbm","wb");
      pbm_save(&descr, outfile);
      delete [] dw;
#endif
    }
  }
#endif
#ifdef UNIX
  osd_font=Get_charset_blk();
#endif
  if(osd_font==NULL)
  {
    for(int n=0;n<4;n++)
    {
      jump_osd_draw_char[n]=osd_draw_char_dont;
      jump_osd_draw_char_clipped[n]=osd_draw_char_clipped_dont;
      jump_osd_draw_char_transparent[n]=osd_draw_char_dont;
      jump_osd_draw_char_clipped_transparent[n]=osd_draw_char_clipped_dont;
    }
  }
  osd_start_time=0;
}


TOsdControl OsdControl;
/*
void TOsdControl::StartScroller(EasyStr text) {
  if(!osd_show_scrollers)
    return;
  ScrollerColour=col_yellow[0]; //TODO
  // add to current scroller
  ScrollText=text;
  ScrollerPhase=WANT_SCROLLER;
  osd_shown_scroller=0;
}
*/

/*  Display some information in yellow on the top left corner of the screen,
    a bit like drive info, but it can be any (short) message. 
*/  

void TOsdControl::Trace(char *fmt,...) {
  //if(timer<MessageTimer)
    //return;
  va_list body;	
  va_start(body, fmt);	
#if defined(SSE_UNIX)
  vsnprintf(m_OsdMessage,OSD_MESSAGE_LENGTH,fmt,body); // check for overrun 
#else
  _vsnprintf(m_OsdMessage,OSD_MESSAGE_LENGTH,fmt,body); // check for overrun 
#endif
  va_end(body);	
  strupr(m_OsdMessage); // OSD font is upper-only
  MessageTimer=timer+OSD_MESSAGE_TIME*1000;
}

