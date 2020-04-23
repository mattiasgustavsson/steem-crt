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
FILE: draw.cpp
DESCRIPTION: Routines to handle Steem's video output. draw_routines_init
initialises the system, draw_begin locks output, draw_scanline is used to
draw one line, draw_end unlocks output and draw_blit blits the frame to the
PC display. 
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <computer.h>
#include <draw.h>
#include <palette.h>
#include <osd.h>
#include <gui.h>
#include <debug_framereport.h>

BYTE bad_drawing=0;
BYTE border=0,border_last_chosen=0;
RECT draw_blit_source_rect;
int draw_win_mode[2]={0,0};

bool draw_lock;
BYTE *draw_mem;
int draw_line_length;
long *PCpal;

extern "C" {BYTE *draw_dest_ad=0,*draw_dest_next_scanline=0;}

int draw_dest_increase_y;
short res_vertical_scale=1;
short draw_first_scanline_for_border,draw_last_scanline_for_border; //calculated from BORDER_TOP, BORDER_BOTTOM and res_vertical_scale
short draw_first_scanline_for_border60,draw_last_scanline_for_border60;
short draw_first_possible_line=0,draw_last_possible_line=200;
int scanline_drawn_so_far;
int left_border=ORIGINAL_BORDER_SIDE,right_border=ORIGINAL_BORDER_SIDE;
#if defined(SSE_VID_32BIT_ONLY)
LPPIXELWISESCANPROC jump_draw_scanline[3][3];
#else
LPPIXELWISESCANPROC jump_draw_scanline[3][4][3];
#endif
LPPIXELWISESCANPROC draw_scanline,draw_scanline_lowres,draw_scanline_medres;
COUNTER_VAR shifter_freq_change_time[32];
BYTE shifter_freq_change[32];
BYTE shifter_freq_change_idx=0;
COUNTER_VAR shifter_mode_change_time[32];
BYTE shifter_mode_change[32];
BYTE shifter_mode_change_idx=0;
bool draw_med_low_double_height;
bool draw_line_off=0;
bool freq_change_this_scanline=false;

const BYTE scanlines_above_screen[4]={SCANLINES_ABOVE_SCREEN_50HZ,
                                    SCANLINES_ABOVE_SCREEN_60HZ,
                                    SCANLINES_ABOVE_SCREEN_70HZ,
                                    16};

#ifdef WIN32
#if defined(SSE_VID_DD) && !defined(SSE_VID_DD_MISC)
HWND ClipWin;
#endif
BYTE draw_temp_line_buf[4096]; // overkill
BYTE* draw_temp_line_buf_lim=(BYTE*)&draw_temp_line_buf+DRAW_TEMP_LINE_BUF_LEN;
BYTE *draw_store_dest_ad=NULL;
LPPIXELWISESCANPROC draw_scanline_1_line[2],draw_store_draw_scanline;
bool draw_buffer_complex_scanlines;
#endif
#ifdef UNIX
int x_draw_surround_count=(4);
#endif

#if defined(SSE_VID_DD) || defined(UNIX)
BYTE draw_fs_topgap;
bool prefer_res_640_400=(0),using_res_640_400=(0);
#endif

BYTE draw_grille_black=(6);

#if defined(SSE_VID_D3D)
const
#endif
BYTE draw_fs_fx=DFSFX_NONE;

#if defined(SSE_VID_DD)
WORD tested_pc_hz[NPC_HZ_CHOICES]={0,0,0,0};
WORD real_pc_hz[NPC_HZ_CHOICES]={0,0,0,0};
#endif

#if !defined(SSE_VID_D3D)
BYTE draw_fs_blit_mode=( UNIX_ONLY(DFSM_STRAIGHTBLIT) WIN_ONLY(DFSM_STRETCHBLIT) );
int prefer_pc_hz[NPC_HZ_CHOICES]={0,0,0,0};
#endif

#undef LOGSECTION
#define LOGSECTION LOGSECTION_VIDEO_RENDERING


void ASMCALL draw_scanline_dont(int,int,int,int) {}


void draw_begin() {
  //TRACE_OSD("frame %d",FRAME); //called at each frame, it calls Lock()
  if(draw_lock) 
    return;
#if defined(SSE_EMU_THREAD)
  if(SuspendRendering || VideoLock.blocked)
    return;
#elif defined(SSE_EMU_THREAD)
  if(SuspendRendering)
    return;
#endif
  if(border) 
  {
    draw_first_possible_line=draw_first_scanline_for_border;
    draw_last_possible_line=draw_last_scanline_for_border;
  }
  else
  {
    draw_first_possible_line=0;
    draw_last_possible_line=shifter_y;
#if defined(SSE_VID_DD)
    draw_fs_topgap=40;
#endif
  }
  UNIX_ONLY( draw_fs_topgap=0; )
#if defined(SSE_VID_DD)
  if(FullScreen&&draw_grille_black>0)
    Disp.DrawFullScreenLetterbox();
#endif
  if(Disp.Lock()!=DD_OK)
    return;
#if !defined(SSE_VID_32BIT_ONLY)
  if(BytesPerPixel==1) 
    palette_copy();
#endif
  osd_draw_begin();
  draw_lock=true;
  draw_set_jumps_and_source();
  int offset=(draw_blit_source_rect.top*draw_line_length)+
            (draw_blit_source_rect.left*BytesPerPixel);
  draw_mem+=offset;
  Disp.VideoMemorySize-=offset;
  draw_dest_ad=draw_mem;
  draw_dest_next_scanline=draw_dest_ad+draw_dest_increase_y;
  WIN_ONLY( draw_store_dest_ad=NULL; )
  if(SCANLINES_OK)
    draw_grille_black=4;
  if(draw_grille_black>0) 
  {
    bool using_grille=0;
    if(draw_dest_increase_y>draw_line_length)
    {
#ifdef WIN32
      if(FullScreen && draw_fs_fx==DFSFX_GRILLE) 
        using_grille=true;
      else if(screen_res<2 && SCANLINES_OK)
        using_grille=true;
#else
      if(draw_fs_fx==DFSFX_GRILLE) using_grille=true;
#endif
    }
    if(SCANLINES_OK) //again, one too many?
      using_grille=true;
    if(using_grille) 
    {
      //TRACE_OSD("GRILLE");
      int l=640;
      if(Disp.BorderPossible()) 
        l+=BORDER_SIDE*2*2;
      l*=BytesPerPixel;
      BYTE *d=draw_dest_ad+draw_line_length;
      int y=200;
      if(Disp.BorderPossible()) 
        y=200+BottomBorderSize+BORDER_TOP;
      for(;y>0;y--) 
      {
        memset(d,0,l); // black line
        d+=draw_dest_increase_y;
      }
    }
    draw_grille_black--;
  }
#ifdef UNIX
  if(x_draw_surround_count>0) {
    Disp.Surround();
    x_draw_surround_count--;
  }
#endif
#ifdef DEBUG_BUILD
  if(debug_cycle_colours) 
  {
    debug_cycle_colours--;
    if(debug_cycle_colours==0) 
    {
      for(int i=0;i<16;i++) 
      {
        switch(BytesPerPixel) {
#if !defined(SSE_VID_32BIT_ONLY)
        case 1:
          PCpal[i]=rand()%256;
          PCpal[i]|=(PCpal[i]<<8);
          break;
        case 2:
          PCpal[i]=0x8888+(rand()%0x7777);
          PCpal[i]|=(PCpal[i]<<16);
          break;
#endif
        case 3:case 4:
          PCpal[i]=0x88888888+(rand()%0x77777777);
          break;
        }
      }
      debug_cycle_colours=CYCLE_COL_SPEED;
    }
  }
#endif
}


void draw_set_jumps_and_source() {
  // SS called at each frame by draw_begin()
  if(!draw_lock) 
  {
    draw_scanline=draw_scanline_dont;
    return;
  }
  bool big_draw=CanUse_400; // double pixels in lores
#ifdef WIN32
#if defined(SSE_VID_D3D)
  if(FullScreen && (!OPTION_C3))
    big_draw=false;
#else
  if(FullScreen)
  {
    if(!OPTION_C3 &&(draw_fs_blit_mode==DFSM_STRETCHBLIT
      ||draw_fs_blit_mode==DFSM_FAKEFULLSCREEN))
      big_draw=0;
  }
#endif
  if(big_draw) 
  {
    if(ResChangeResize==0) 
      big_draw=0; // always stretch
    else if(screen_res<2) 
      if(draw_win_mode[screen_res]==DWM_STRETCH)
        big_draw=0;
  }
#endif
#ifdef UNIX
  if(FullScreen) 
    big_draw=true;
#endif
  if(emudetect_falcon_mode!=EMUD_FALC_MODE_OFF) 
  {
    draw_scanline=emudetect_falcon_draw_scanline;
    draw_dest_increase_y=draw_line_length;
    int ox=0,oy=0,ow,oh;
    if(big_draw) 
    {
      ow=640;oh=400;
      if(border) 
      {
        ow=640+BORDER_SIDE*2+BORDER_SIDE*2;
        oh=400+BORDER_TOP*2+BORDER_BOTTOM*2;
#if defined(SSE_VID_DD)
        if(FullScreen) 
          ox=(800-ow)/2,oy=(600-oh)/2;
#endif
      }
#if defined(SSE_VID_DD)
      else if(FullScreen && using_res_640_400==0)
        oy=(480-oh)/2;
#endif
      if(emudetect_falcon_mode_size==1)
        draw_dest_increase_y*=2; // Have to draw double height
    }
    else
    {
      ow=320*emudetect_falcon_mode_size,oh=200*emudetect_falcon_mode_size;
      if(border) 
      {
        // We double the size of borders too to keep the aspect ratio of the screen the same
        ow+=BORDER_SIDE*2*emudetect_falcon_mode_size;
        oh+=(BORDER_TOP+BORDER_BOTTOM)*emudetect_falcon_mode_size;
      }
    }
    draw_blit_source_rect.left=ox;
    draw_blit_source_rect.right=ox+ow;
    draw_blit_source_rect.top=oy;
    draw_blit_source_rect.bottom=oy+oh;
    return;
  }//falcon
  if(extended_monitor) 
  {
    int res=screen_res; // Low/Med
    if(em_planes==1) 
      res=2;
#if defined(SSE_VID_32BIT_ONLY)
    draw_scanline=jump_draw_scanline[0][res];
#else
    draw_scanline=jump_draw_scanline[0][BytesPerPixel-1][res];
#endif
    draw_blit_source_rect.left=0;
    draw_blit_source_rect.right=MIN(em_width,Disp.SurfaceWidth);
    draw_blit_source_rect.top=0;
    draw_blit_source_rect.bottom=MIN(em_height,Disp.SurfaceHeight);
    draw_dest_increase_y=draw_line_length;
    return;
  }
  //TRACE("CanUse_400 %d big_draw %d\n",CanUse_400,big_draw);  
  if(big_draw || SCANLINES_INTERPOLATED
#if defined(SSE_VID_D3D)
    || FullScreen && SCANLINES_OK && draw_win_mode[screen_res]==DWM_NOSTRETCH
#elif defined(SSE_VID_DD)
    || FullScreen && draw_fs_fx==DFSFX_GRILLE && (draw_fs_blit_mode
    ==DFSM_STRETCHBLIT||draw_fs_blit_mode==DFSM_FAKEFULLSCREEN)
#endif
    )
  {
    int p=1; // 640 width 400 height
#ifdef WIN32
    if(FullScreen) 
    {
      if(draw_fs_fx==DFSFX_GRILLE) 
        p=2; // 640x200 low/med
    }
    else if(screen_res<2) 
    {
      if(draw_win_mode[screen_res]==DWM_NOSTRETCH && SCANLINES_OK)
        p=2; // 640x200 low/med
    }
#else
    if (draw_fs_fx==DFSFX_GRILLE) p=2; // 640x200 low/med
#endif
    if(SCANLINES_INTERPOLATED
#if defined(SSE_VID_D3D)
      ||FullScreen&&draw_win_mode[screen_res]==DWM_NOSTRETCH&&!OPTION_C3
#endif
      )
      p=0;
#if defined(SSE_VID_32BIT_ONLY)
    draw_scanline=jump_draw_scanline[p][screen_res];
    draw_scanline_lowres=jump_draw_scanline[p][0];
    draw_scanline_medres=jump_draw_scanline[p][1];
#else
    draw_scanline=jump_draw_scanline[p][BytesPerPixel-1][screen_res];
    draw_scanline_lowres=jump_draw_scanline[p][BytesPerPixel-1][0];
    draw_scanline_medres=jump_draw_scanline[p][BytesPerPixel-1][1];
#endif
    draw_med_low_double_height=(p==1);
    //TRACE("%d\n",(p==1));
#ifdef WIN32
#if defined(SSE_VID_32BIT_ONLY)
    draw_scanline_1_line[0]=jump_draw_scanline[2][0];
    draw_scanline_1_line[1]=jump_draw_scanline[2][1];
#else
    draw_scanline_1_line[0]=jump_draw_scanline[2][BytesPerPixel-1][0];
    draw_scanline_1_line[1]=jump_draw_scanline[2][BytesPerPixel-1][1];
#endif
#endif
    int ox=0,oy=0,ow=640,oh=400;
    if(border) 
    {
      oh=BORDER_TOP*2+400+BORDER_BOTTOM*2;
      ow=SideBorderSizeWin*2 + 640 + SideBorderSizeWin*2;
#if defined(SSE_VID_DD)
      if(FullScreen&&!OPTION_C3)
      {
        ox=(800-ow)/2;
        oy=(600-oh)/2;
      }
#endif
    }
    else if(FullScreen) 
    {
#if defined(SSE_VID_DD)
      oy=int(using_res_640_400 ? 0:40);
#endif
    }
    if(SCANLINES_INTERPOLATED && !screen_res && !FullScreen && !OPTION_C3) 
      ow/=2;
    draw_blit_source_rect.left=ox;
    draw_blit_source_rect.right=ox+ow;
    draw_blit_source_rect.top=oy;
    draw_blit_source_rect.bottom=oy+oh;
    if(MONO) 
      draw_dest_increase_y=draw_line_length;
    else if(HIRES_COLOUR) // FALSE
    {
      draw_dest_increase_y=draw_line_length;
      draw_blit_source_rect.right=ox+ow/2;
      draw_med_low_double_height=false;
    }
    else if((draw_med_low_double_height||SCANLINES_OK))
      draw_dest_increase_y=2*draw_line_length;
    else
      draw_dest_increase_y=draw_line_length;
      
    if(video_mixed_output&&(SCANLINES_INTERPOLATED
#if defined(SSE_VID_D3D)
      ||FullScreen&&screen_res<2&&draw_win_mode[screen_res]==DWM_NOSTRETCH
#endif
#if defined(SSE_VID_DD)
      ||FullScreen 
      && (draw_fs_blit_mode==DFSM_FAKEFULLSCREEN||draw_fs_blit_mode
      ==DFSM_STRETCHBLIT)&& draw_fs_fx==DFSFX_GRILLE
#endif
      ))
    {
#if defined(SSE_VID_32BIT_ONLY)
      draw_scanline_lowres=jump_draw_scanline[2][0]; //draw double
#else
      draw_scanline_lowres=jump_draw_scanline[2][BytesPerPixel-1][0]; //draw double
#endif
      if(screen_res==0) 
        draw_scanline=draw_scanline_lowres;
    }
  }
  else //!big_draw
  {
#if defined(SSE_VID_32BIT_ONLY)
    draw_scanline=jump_draw_scanline[0][screen_res];
    draw_scanline_lowres=jump_draw_scanline[0][0];
    draw_scanline_medres=jump_draw_scanline[0][1];
#else
    draw_scanline=jump_draw_scanline[0][BytesPerPixel-1][screen_res];
    draw_scanline_lowres=jump_draw_scanline[0][BytesPerPixel-1][0];
    draw_scanline_medres=jump_draw_scanline[0][BytesPerPixel-1][1];
#endif
    draw_med_low_double_height=0;
    int ox=0,oy=0,ow=shifter_x,oh=shifter_y;
    if(video_mixed_output||OPTION_C3) 
    {
#if defined(SSE_VID_32BIT_ONLY)
      draw_scanline_lowres=jump_draw_scanline[2][0];
#else
      draw_scanline_lowres=jump_draw_scanline[2][BytesPerPixel-1][0];
#endif
      if(screen_res==0) 
        draw_scanline=draw_scanline_lowres;
      ow=640;
    }
#ifdef WIN32
    draw_scanline_1_line[0]=draw_scanline_lowres;
    draw_scanline_1_line[1]=draw_scanline_medres;
#endif
    if(border) 
    {
      if(screen_res==0&&video_mixed_output==0&&!OPTION_C3)
        ow+=SideBorderSizeWin*2;
      else
        ow+=(SideBorderSizeWin+SideBorderSizeWin) * 2;
      oh=shifter_y+res_vertical_scale*(BORDER_TOP+BORDER_BOTTOM);
    }
#if defined(SSE_VID_DD)
    else if(FullScreen)
      oy=(int)(using_res_640_400 ? 0:40);
#endif
    if(SSEOptions.Scanlines&&OPTION_C3&&screen_res<2)
      oh*=2;
    draw_blit_source_rect.left=ox;
    draw_blit_source_rect.top=oy;
    draw_blit_source_rect.right=ox+ow;
    draw_blit_source_rect.bottom=oy+oh;
    draw_dest_increase_y=draw_line_length;
  }
#ifdef WIN32
  draw_buffer_complex_scanlines=((Disp.Method==DISPMETHOD_DD||Disp.Method
    ==DISPMETHOD_D3D)&&Disp.DrawToVidMem && draw_med_low_double_height);
#endif
  //TRACE("src %d %d %d %d len %d\n",draw_blit_source_rect.left,draw_blit_source_rect.top,draw_blit_source_rect.right,draw_blit_source_rect.bottom,draw_line_length);
  //TRACE_OSD("C%d B%d D%d SI%d",CanUse_400,big_draw,draw_med_low_double_height,SCANLINES_INTERPOLATED);
}


void draw_end() {
  if(!draw_lock)
    return;
#ifndef ONEGAME
  bool draw_osd=true;
  if(DoSaveScreenShot||slow_motion
    ||Glue.m_Status.stop_emu&&OPTION_NO_OSD_ON_STOP)
    draw_osd=false;
#ifdef DEBUG_BUILD
  if(runstate!=RUNSTATE_RUNNING)
    draw_osd=false;
#endif
  if(draw_osd)
    osd_draw();
#endif
#ifdef DEBUG_BUILD
  Shifter.DrawBufferedScanlineToVideo(); 
#endif
  Disp.Unlock();
#ifdef SHOW_WAVEFORM
  Disp.DrawWaveform();
#endif
  osd_draw_end(); // resets osd_draw function pointers
  draw_scanline=draw_scanline_dont;
  WIN_ONLY( draw_store_dest_ad=NULL; )
  draw_lock=false;
  if(DoSaveScreenShot) 
  {
    Disp.SaveScreenShot();
    DoSaveScreenShot&=~1;
  }
}


bool draw_blit() {
  bool ok=false;
#if defined(SSE_EMU_THREAD) 
  if(SuspendRendering || VideoLock.blocked)
    return ok;
#endif
  // we blit the unlocked backsurface
  if((!draw_lock||OPTION_3BUFFER_WIN&&!FullScreen)&&bAppMinimized==0)
  {
#if !defined(SSE_VID_32BIT_ONLY)
    if(BytesPerPixel==1) 
      palette_flip();
#endif
    ok=Disp.Blit();
    // Check for screen change right after the blit so that we
    //  don't erase the frame (fullscreen) just before it's rendered
    if(runstate==RUNSTATE_RUNNING)
    {
      if(video_mixed_output>0) 
      {
        video_mixed_output--;
        if(video_mixed_output==2) 
        {
          init_screen();
          res_change();
        }
        else if(video_mixed_output==0) 
        {
          init_screen();
          if(screen_res==0||SCANLINES_INTERPOLATED)
            res_change();
          screen_res_at_start_of_vbl=screen_res;
        }
      }
      else if(screen_res!=screen_res_at_start_of_vbl) 
      {
        init_screen();
        res_change();
        screen_res_at_start_of_vbl=screen_res;
      }
    }//runstate
  }
  return ok;
}


void draw(bool osd) {
  // This is called by init, load... not for actual emulation
  // It draws the screen in one time. We don't try to get it perfect (60Hz, overscan...)
#if defined(SSE_EMU_THREAD)
  if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING
    && !(extended_monitor&&(bad_drawing&6)))
    return;
#endif
  short save_scan_y=scan_y;
  MEM_ADDRESS save_sdp=shifter_draw_pointer;
  MEM_ADDRESS save_sdp_at_start_of_line=shifter_draw_pointer_at_start_of_line;
  int save_drawn_so_far=scanline_drawn_so_far;
  short save_pixel=shifter_pixel;
  shifter_draw_pointer=vbase;
#if defined(SSE_DEBUGGER_FRAME_REPORT)
  int save_shifter_first_draw_line=video_first_draw_line;
  int save_shifter_last_draw_line=video_last_draw_line;
#endif
  // we blit the unlocked backsurface
  if(!draw_lock || OPTION_3BUFFER_WIN && !FullScreen)
  { 
    draw_begin();
    short yy,yy2;
    if(extended_monitor)
    {
      //ASSERT(shifter_draw_pointer==vbase);
      yy=0;
      yy2=(short)MIN(em_height,Disp.SurfaceHeight);
    }
    else if(border)
    {
      yy=draw_first_scanline_for_border;
      yy2=draw_last_scanline_for_border;
    }
    else
    {
      yy=0;
      yy2=shifter_y;
    }
    BYTE option_vle=OPTION_VLE;
    OPTION_VLE=0;
    for(;yy<yy2;yy++) 
    {
      scan_y=yy;
      scanline_drawn_so_far=0;
      shifter_draw_pointer_at_start_of_line=shifter_draw_pointer;
      Shifter.DrawScanlineToEnd();
#if defined(SSE_DEBUGGER_FRAME_REPORT)
      DWORD Tricks=FrameEvents.GetShifterTricks(scan_y);
      if(Tricks&TRICK_TOP_OVERSCAN)
        video_first_draw_line=-29;
      else if(Tricks&(TRICK_BOTTOM_OVERSCAN|TRICK_BOTTOM_OVERSCAN_60HZ))
        video_last_draw_line=247;  
#endif
    }
    OPTION_VLE=option_vle;
    if(osd) 
      osd_init_draw_static();
    draw_end();
    draw_blit();
  }
  scan_y=save_scan_y;
  shifter_draw_pointer=save_sdp;
  shifter_draw_pointer_at_start_of_line=save_sdp_at_start_of_line;
  scanline_drawn_so_far=save_drawn_so_far;
  shifter_pixel=save_pixel;
#if defined(SSE_DEBUGGER_FRAME_REPORT)
  video_first_draw_line=save_shifter_first_draw_line;
  video_last_draw_line=save_shifter_last_draw_line;
#endif
#if defined(SSE_EMU_THREAD)
  SuspendRendering=false;
#endif
}


void init_screen() {
  draw_end();
  // shifter_x/shifter_y/res_vertical_scale are used to get the source rect.
  switch(screen_res) {
  case 0:
    shifter_x=320;shifter_y=200;
    res_vertical_scale=1;
    break;
  case 1:
    shifter_x=640;
    res_vertical_scale=(HIRES_COLOUR)?2:1;
    shifter_y=200*res_vertical_scale;
    break;
  case 2:
    shifter_x=640;shifter_y=400;
    res_vertical_scale=2;
    break;
  }
  // These are used to determine where to draw
  draw_first_scanline_for_border=res_vertical_scale*(-BORDER_TOP);
  draw_last_scanline_for_border=shifter_y+res_vertical_scale*(BottomBorderSize);
  if(res_vertical_scale==2) // shorter top border
  {
    draw_first_scanline_for_border+=26; 
    draw_last_scanline_for_border+=26;
  }

  // 60Hz
  draw_first_scanline_for_border60=draw_first_scanline_for_border+11;
  draw_last_scanline_for_border60=draw_last_scanline_for_border-17;
  if(DISPLAY_SIZE>1)
    draw_last_scanline_for_border60-=5;

  // This is used to know where to cause the timer B event
  if(!OPTION_68901)
  {
    CALC_CYCLES_FROM_HBL_TO_TIMER_B(Glue.video_freq);
  }
  TRACE_LOG("init_screen() %dx%d,%d-%d\n",shifter_x,shifter_y,draw_first_scanline_for_border,draw_last_scanline_for_border);
}


void res_change() {
  TRACE_LOG("res_change()\n");
  if(ResChangeResize) 
    StemWinResize();
  draw_set_jumps_and_source();
}


bool draw_routines_init() { 
  PCpal=Get_PCpal(); // SS defined in asm_draw.asm
  for(int a=0;a<3;a++)
#if defined(SSE_VID_32BIT_ONLY)
    for(int c=0;c<3;c++)
      jump_draw_scanline[a][c]=draw_scanline_dont;
#else
    for(int b=0;b<4;b++)
      for(int c=0;c<3;c++)
        jump_draw_scanline[a][b][c]=draw_scanline_dont;
#endif
  // [0=Smallest size possible, 1=640x400 (all reses), 2=640x200 (med/low res)]
  //  [BytesPerPixel-1]
  //    [screen_res]
  // SS Smallest size possible
#if defined(SSE_VID_32BIT_ONLY)
  jump_draw_scanline[0][0]=draw_scanline_32_lowres_pixelwise; //LO
  jump_draw_scanline[0][1]=draw_scanline_32_medres_pixelwise; //ME
  jump_draw_scanline[0][2]=draw_scanline_32_hires; //HI
  jump_draw_scanline[1][0]=draw_scanline_32_lowres_pixelwise_400; //LO
  jump_draw_scanline[1][1]=draw_scanline_32_medres_pixelwise_400; //ME
  jump_draw_scanline[1][2]=draw_scanline_32_hires; //HI
  jump_draw_scanline[2][0]=draw_scanline_32_lowres_pixelwise_dw; //LO
  jump_draw_scanline[2][1]=draw_scanline_32_medres_pixelwise; //ME
  jump_draw_scanline[2][2]=draw_scanline_32_hires; //HI
#else
  // 1 byte per pixel
  jump_draw_scanline[0][0][0]=draw_scanline_8_lowres_pixelwise; //LO
  jump_draw_scanline[0][0][1]=draw_scanline_8_medres_pixelwise; //ME
  jump_draw_scanline[0][0][2]=draw_scanline_8_hires; //HI
  // 2 bytes per pixel
  jump_draw_scanline[0][1][0]=draw_scanline_16_lowres_pixelwise; //LO
  jump_draw_scanline[0][1][1]=draw_scanline_16_medres_pixelwise; //ME
  jump_draw_scanline[0][1][2]=draw_scanline_16_hires; //HI
  // 3 bytes per pixel
  jump_draw_scanline[0][2][0]=draw_scanline_24_lowres_pixelwise; //LO
  jump_draw_scanline[0][2][1]=draw_scanline_24_medres_pixelwise; //ME
  jump_draw_scanline[0][2][2]=draw_scanline_24_hires; //HI
  // 4 bytes per pixel
  jump_draw_scanline[0][3][0]=draw_scanline_32_lowres_pixelwise; //LO
  jump_draw_scanline[0][3][1]=draw_scanline_32_medres_pixelwise; //ME
  jump_draw_scanline[0][3][2]=draw_scanline_32_hires; //HI
  // SS 640x400 (all reses)
  // 1 byte per pixel
  jump_draw_scanline[1][0][0]=draw_scanline_8_lowres_pixelwise_400; //LO
  jump_draw_scanline[1][0][1]=draw_scanline_8_medres_pixelwise_400; //ME
  jump_draw_scanline[1][0][2]=draw_scanline_8_hires; //HI
  // 2 bytes per pixel
  jump_draw_scanline[1][1][0]=draw_scanline_16_lowres_pixelwise_400; //LO
  jump_draw_scanline[1][1][1]=draw_scanline_16_medres_pixelwise_400; //ME
  jump_draw_scanline[1][1][2]=draw_scanline_16_hires; //HI
  // 3 bytes per pixel
  jump_draw_scanline[1][2][0]=draw_scanline_24_lowres_pixelwise_400; //LO
  jump_draw_scanline[1][2][1]=draw_scanline_24_medres_pixelwise_400; //ME
  jump_draw_scanline[1][2][2]=draw_scanline_24_hires; //HI
  // 4 bytes per pixel
  jump_draw_scanline[1][3][0]=draw_scanline_32_lowres_pixelwise_400; //LO
  jump_draw_scanline[1][3][1]=draw_scanline_32_medres_pixelwise_400; //ME
  jump_draw_scanline[1][3][2]=draw_scanline_32_hires; //HI
  // SS 640x200 (med/low res) dw=1
  // 1 byte per pixel
  jump_draw_scanline[2][0][0]=draw_scanline_8_lowres_pixelwise_dw; //LO
  jump_draw_scanline[2][0][1]=draw_scanline_8_medres_pixelwise; //ME
  jump_draw_scanline[2][0][2]=draw_scanline_8_hires; //HI
  // 2 bytes per pixel
  jump_draw_scanline[2][1][0]=draw_scanline_16_lowres_pixelwise_dw; //LO
  jump_draw_scanline[2][1][1]=draw_scanline_16_medres_pixelwise; //ME
  jump_draw_scanline[2][1][2]=draw_scanline_16_hires; //HI
  // 3 bytes per pixel
  jump_draw_scanline[2][2][0]=draw_scanline_24_lowres_pixelwise_dw; //LO
  jump_draw_scanline[2][2][1]=draw_scanline_24_medres_pixelwise; //ME
  jump_draw_scanline[2][2][2]=draw_scanline_24_hires; //HI
  // 4 bytes per pixel
  jump_draw_scanline[2][3][0]=draw_scanline_32_lowres_pixelwise_dw; //LO
  jump_draw_scanline[2][3][1]=draw_scanline_32_medres_pixelwise; //ME
  jump_draw_scanline[2][3][2]=draw_scanline_32_hires; //HI
#endif
  draw_scanline=draw_scanline_dont; // SS init the general pointer
  osd_routines_init();
  return true;
}
