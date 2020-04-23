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
FILE: shifter.cpp
DESCRIPTION: The Shifter is an Atari custom chip. It is the ST's fastest
chip: 32mhz.
The ST was a barebone machine made up of cheap components hastily patched
together, including the video Shifter.
A Shifter was needed to translate the video RAM into the picture.
Such a system was chosen because RAM space was precious. Chunk modes are
only practical with byte or more pixel sizes.
Some aspects of the Shifter are emulated at high level here but its core
function, shifting video rasters, is emulated through direct rendering of
ST video memory by assembly (32bit builds) or C (64bit builds) routines.
On the STE, the GST Shifter has a sound sample playing feature. This
is also emulated at high level.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <conditions.h>
#include <draw.h>
#include <palette.h>
#include <run.h>
#include <emulator.h>
#include <stports.h>
#include <debug_framereport.h>
#include <display.h>
#include <computer.h>
#include <gui.h>
#include <sound.h>
#include <interface_stvl.h>


///////////
// VIDEO //
///////////


#define LOGSECTION LOGSECTION_VIDEO


extern "C" // used by assembly routines, can't put in structure
{
  WORD STpal[16];
  BYTE shifter_hscroll;
  MEM_ADDRESS shifter_draw_pointer;
}

short shifter_pixel;
short shifter_x,shifter_y;
BYTE shifter_sound_mode;

TShifter::TShifter() {
  ZeroMemory(this,sizeof(TShifter));
#ifdef WIN32
  draw_temp_line_buf_lim=draw_temp_line_buf+DRAW_TEMP_LINE_BUF_LEN;
#endif
  WakeupShift=SHIFTER_DEFAULT_WAKEUP;
}


#ifdef WIN32

void TShifter::DrawBufferedScanlineToVideo() {
  if(draw_store_dest_ad)
  { 
    // Bytes that will be copied.
    int amount_drawn=(int)(draw_dest_ad-draw_temp_line_buf); 
#if 0 // no need anymore?
    // Don't access video memory beyond the surface, it causes a crash
    if(amount_drawn<0 || draw_store_dest_ad+amount_drawn>=Disp.VideoMemoryEnd)
    {
      TRACE_LOG("Video memory overflow\n");
      return;
    }
#endif
    // From draw_temp_line_buf to draw_store_dest_ad
    DWORD *src=(DWORD*)draw_temp_line_buf; 
    DWORD *dest=(DWORD*)draw_store_dest_ad;  
    while(src<(DWORD*)draw_dest_ad)
      *(dest++)=*(src++); 
    if(draw_med_low_double_height)
    {
      src=(DWORD*)draw_temp_line_buf;                        
      dest=(DWORD*)(draw_store_dest_ad+draw_line_length);     
      while(src<(DWORD*)draw_dest_ad) 
        *(dest++)=*(src++);       
    }
    draw_dest_ad=draw_store_dest_ad+amount_drawn;                    
    draw_store_dest_ad=NULL;                                           
    draw_scanline=draw_store_draw_scanline; 
  }
  //ScanlineBuffer=NULL;
}

#endif

// don't render if we're below or beyond video RAM (shifter trick reckoning panic "0byte"...)
#define CHECK_VIDEO_RAM \
  if(!draw_scanline\
    ||draw_dest_next_scanline-draw_line_length<draw_mem\
    ||draw_dest_next_scanline>Disp.VideoMemoryEnd)\
  {\
    return;\
  }


void TShifter::DrawScanlineToEnd() {
  //ASSERT(!OPTION_C3);
  CHECK_VIDEO_RAM;
  MEM_ADDRESS nsdp;
  if(emudetect_falcon_mode!=EMUD_FALC_MODE_OFF)
  {
    int pic=320*emudetect_falcon_mode_size,bord=0;
    // We double the size of borders too to keep the aspect ratio of the screen the same
    if(border) 
      bord=BORDER_SIDE*emudetect_falcon_mode_size;
    if(scan_y>=draw_first_possible_line && scan_y<draw_last_possible_line)
    {
      bool in_border=(scan_y>=draw_first_scanline_for_border 
        && scan_y<draw_last_scanline_for_border);
      bool in_pic=(scan_y>=video_first_draw_line && scan_y<video_last_draw_line);
      if(in_pic||in_border)
      {
        // We only have 200 scanlines, but if emudetect_falcon_mode_size==2
        // then the picture must be 400 pixels high. So to make it work we
        // draw two different lines at the end of each scanline.
        // To make it more confusing in some drawing modes if
        // emudetect_falcon_mode_size==1 we have to draw two identical
        // lines. That is handled by the draw_scanline routine. 
        for(int n=0;n<emudetect_falcon_mode_size;n++)
        {
          if(in_pic)
          {
            nsdp=shifter_draw_pointer+pic*emudetect_falcon_mode;
            draw_scanline(bord,pic,bord,HSCROLL);
            shifter_draw_pointer=nsdp;
          }
          else
            draw_scanline(bord+pic+bord,0,0,0);
          draw_dest_ad=draw_dest_next_scanline;
          draw_dest_next_scanline+=draw_dest_increase_y;
        }
      }
    }
    shifter_pixel=HSCROLL; //start by drawing this pixel
    scanline_drawn_so_far=BORDER_SIDE+320+BORDER_SIDE;
  }
  else if(extended_monitor)
  {
    int h=MIN(em_height,Disp.SurfaceHeight);
    int w=MIN(em_width,Disp.SurfaceWidth);
    if(extended_monitor==1)
    {	// Borders needed, before hack
      if(em_planes==1)
      { //mono
        int y=h/2-200,x=(w/2-320)&-16;
        if(scan_y<h)
        {
          if(scan_y<y||scan_y>=y+400)
            draw_scanline(w/16,0,0,0);
          else
          {
            draw_scanline(x/16,640/16,w/16-x/16-640/16,0);
            shifter_draw_pointer+=80;
          }
        }
      }
      else
      {
        int y=h/2-100,x=(w/2-160)&-16;
        if(scan_y<h)
        {
          if(scan_y<y||scan_y>=y+200)
            draw_scanline(w,0,0,0);
          else
          {
            draw_scanline(x,320,w-x-320,0);
            shifter_draw_pointer+=160;
          }
        }
      }
      draw_dest_ad=draw_dest_next_scanline;
      draw_dest_next_scanline+=draw_dest_increase_y;
    }
    else
    {
      if(scan_y<h)
      {
        if(em_planes==1) 
          w/=16;
        if(screen_res==1) 
          w/=2; // medium res routine draws two pixels for every one w
        draw_scanline(0,w,0,0);
        draw_dest_ad=draw_dest_next_scanline;
        draw_dest_next_scanline+=draw_dest_increase_y;
      }
      int real_planes=em_planes;
      if(screen_res==1) 
        real_planes=2;
      shifter_draw_pointer+=em_width*real_planes/8;
    }
  }
  // Colour, regular monitor
  else if(screen_res<2)
  {
    Render(Glue.ScanlineTiming[Glue.RENDER_CYCLE][Glue.FREQ_50]+320+BORDER_SIDE,DISPATCHER_DSTE);
#ifdef WIN32
    DrawBufferedScanlineToVideo();
#endif
    if(scan_y>=draw_first_possible_line && scan_y<draw_last_possible_line)
    {
      // these variables are pointers to PC video memory
      draw_dest_ad=draw_dest_next_scanline;
      draw_dest_next_scanline+=draw_dest_increase_y;
    }
    HblStartingHscroll=HSCROLL; // save the real one
    shifter_pixel=HSCROLL; //start by drawing this pixel (note: for next line)
    if(screen_res==1) //it's in cycles, bad name
      shifter_pixel=HSCROLL/2;
  }
  else // Monochrome, regular monitor
  {
    if(scan_y>=video_first_draw_line && scan_y<video_last_draw_line)
    {
      nsdp=shifter_draw_pointer+80;
      if(OPTION_C2&&shifter_draw_pointer>=himem) // no RAM
        shifter_draw_pointer=0+scan_y*80; // just a bad approximation of the effect      
      if(freq_change_this_scanline)
      {
        Glue.CheckSideOverscan();
        if(draw_line_off)
        {
          for(int i=0;i<=20;i++)
          {
            Scanline[i]=LPEEK(i*4+shifter_draw_pointer); // save ST memory
            LPEEK(i*4+shifter_draw_pointer)=0;
          }
        }
      }
/*  shifter_pixel isn't used, there's no support for hscroll in the assembly
    routines drawing the monochrome scanlines.
    Those routines work with a word precision (16 pixels), so they can't be
    used to implement HSCROLL.
    Since I can't code in assembly yet I provide the feature in C.
    At least in monochrome we mustn't deal with bit planes, there's only
    one plane.*/
      if(OPTION_HACKS&&(HSCROLL||HiresRaster))
      {
        // save ST memory
        for(int i=0;i<=20;i++)
          Scanline[i]=LPEEK(i*4+shifter_draw_pointer);
        if(HSCROLL)
        {
          // hack: shift pixels of full scanline in video RAM
          for(MEM_ADDRESS i=shifter_draw_pointer; i<nsdp;i+=2)
          {
            WORD new_value=DPEEK(i)<<HSCROLL;  // shift first bits away
            new_value|=DPEEK(i+2)>>(16-HSCROLL); // add first bits of next word at the end
            DPEEK(i)=new_value;
          }
        }
      }
      if(HiresRaster)
      { // hack: apply raster effect in video memory
        for(int byte=0;byte<HiresRaster;byte++)
          PEEK(shifter_draw_pointer+byte)
          =PEEK(shifter_draw_pointer+byte)^Scanline2[byte]; // using XOR
      }
      if(!(m_ShiftMode&2)&&!freq_change_this_scanline)
        ; // no output
      else if(scan_y>=draw_first_possible_line && scan_y<draw_last_possible_line)
      {
        if(border)
        { 
          if(Glue.CurrentScanline.Tricks&TRICK_HIRES_OVERSCAN)
            ///////////////// RENDER VIDEO /////////////////
            draw_scanline(0,640/16+BORDER_SIDE/4,0,0);
          else
            ///////////////// RENDER VIDEO /////////////////
            draw_scanline(BORDER_SIDE/8,640/16,BORDER_SIDE/8,0);
        }
        else
          ///////////////// RENDER VIDEO /////////////////
          draw_scanline(0,640/16,0,0);
        draw_dest_ad=draw_dest_next_scanline;
        draw_dest_next_scanline+=draw_dest_increase_y;
      }
      if(draw_line_off||HiresRaster||HSCROLL)
      {
        for(int i=0;i<=20;i++)// restore ST memory
          LPEEK(i*4+shifter_draw_pointer)=Scanline[i];
        if(HiresRaster)
        {
          ZeroMemory(Scanline2,112);
          HiresRaster=0; 
        }
      }
      shifter_draw_pointer=nsdp;
    }
    else if(scan_y>=draw_first_scanline_for_border 
      && scan_y<draw_last_scanline_for_border)
    {
      if(scan_y>=draw_first_possible_line && scan_y<draw_last_possible_line)
      {
        if(border)
          ///////////////// RENDER VIDEO /////////////////
          draw_scanline((BORDER_SIDE*2+640+BORDER_SIDE*2)/16,0,0,0); // rasters!
        else
          ///////////////// RENDER VIDEO /////////////////
          draw_scanline(640/16,0,0,0);
        draw_dest_ad=draw_dest_next_scanline;
        draw_dest_next_scanline+=draw_dest_increase_y;
      }
    }
    scanline_drawn_so_far=BORDER_SIDE+320+BORDER_SIDE; //border1+picture+border2;
  }//end Monochrome
}


void TShifter::IncScanline() {
  scan_y++;
  HblPixelShift=0;
  left_border=right_border=BORDER_SIDE;
/*  With large borders, we don't shift scanlines by 4 pixels when the left border
    is removed. We must shift other scanlines in compensation, which is
    correct emulation. On the ST, the left border is larger than the right
    border. By directly changing left_border and right_border, we can get
    the right timing for palette effects.*/
  if(SideBorderSize==VERY_LARGE_BORDER_SIDE && border)
    left_border+=4,right_border-=4;

    //////////////
    // LINE +16 //
    //////////////

/*  On the STE, it is possible to enlarge or shrink the left border by 16 pixels
   (low-res) on a permanent basis.
    When writing on address $FF8264, the HSCROLL register in the Shifter is modified.
    When writing on address $FF8265, the HSCROLL register in the Shifter is modified, and
    the "prefetch" flag in the Glue is changed (set if value<>0, cleared otherwise).
    Timing doesn't matter, there are two registers.
    The net effect of writing <>0 on $FF8265 then 0 on $FF8264 is that only the "prefetch" 
    flag in the Glue stays set, so the line (DE) starts 16 pixels before. 
    The Shifter does its job without any scrolling, STF-like, and we gain 16 pixels. 
    This is a Glue effect.
    When the HSCROLL register in the Shifter is <>0, it will do some preshift on the 
    prefetched words, and start outputting 16 cycles later. So if you have "prefetch" =
    0 in the Glue, and HSCROLL<>0 in the Shifter, you will have a larger left border.
    This is a Shifter effect.
*/
  if(HSCROLL)
    left_border+=16;
  if(Glue.hscroll)
    left_border-=16;
#if defined(SSE_STATS)
  if(Glue.hscroll&&!HSCROLL)
    Stats.nLinePlus16++;
#endif
  if(HIRES_COLOUR) // This is something else, normally const FALSE
    left_border=16;
}


void TShifter::Render(short cycles_since_hbl,int dispatcher) {
  //ASSERT(!OPTION_C3);
  CHECK_VIDEO_RAM;
  if(screen_res>=2) 
    return; 
#ifndef NO_CRAZY_MONITOR
  if(extended_monitor || emudetect_falcon_mode!=EMUD_FALC_MODE_OFF) 
    return;
#endif
  if(OPTION_C2 && Glue.FetchingLine()&&(freq_change_this_scanline||Preload))
    Glue.CheckSideOverscan();
/*  What happens here is very confusing; we render in real time, but not
    quite.
    We can't render at once because the palette could change. 
    CYCLES_FROM_HBL_TO_LEFT_BORDER_OPEN, ScanlineTiming[RENDER_CYCLE][FREQ_50]
    (84) takes care of that delay. 
    This causes all sort of trouble because our SDP is late while rendering,
    and sometimes forward!
    This pleads for the more logical (but heavier) lle.
*/
#if defined(MMU_PREFETCH_LATENCY)
  // this may look impressive but it's just a bunch of hacks!
  switch(dispatcher) {
  case DISPATCHER_CPU:
    cycles_since_hbl+=MMU_PREFETCH_LATENCY;
    break;
  case DISPATCHER_SET_PAL: // dots or not dots, that is the question
    cycles_since_hbl-=(WakeupShift-1); // it was ++ in original Steem, = Spectrum512 compatible
    //60hz line with border, HSCROLL or not, still quite hacky!
    if(Glue.CurrentScanline.StartCycle
      ==Glue.ScanlineTiming[Glue.LINE_START_LIMIT][Glue.FREQ_60]
      || Glue.CurrentScanline.StartCycle
      ==Glue.ScanlineTiming[Glue.DE_ON][Glue.FREQ_60])
      cycles_since_hbl+=4;
    break;
  case DISPATCHER_SET_SHIFT_MODE:
    RoundCycles(cycles_since_hbl);
    break; 
  case DISPATCHER_WRITE_SDP:
    RoundCycles(cycles_since_hbl);
    break;
  }//sw
#endif
  int pixels_in=cycles_since_hbl-
    (Glue.ScanlineTiming[Glue.RENDER_CYCLE][Glue.FREQ_50]-BORDER_SIDE);
  if(pixels_in > BORDER_SIDE+320+BORDER_SIDE) 
    pixels_in=BORDER_SIDE+320+BORDER_SIDE; 
  // this is the most tricky part of our hack for large border/no shift on left off:
  int pixels_in0=pixels_in;
  if(SideBorderSize==VERY_LARGE_BORDER_SIDE && pixels_in>0)
    pixels_in+=4;
  if(pixels_in>=0) // time to render?
  {
#ifdef WIN32 // prepare buffer & ASM routine
    if(pixels_in>416)
      pixels_in=pixels_in0;
    if(draw_buffer_complex_scanlines && draw_lock)
    {
      if(scan_y>=draw_first_scanline_for_border
        && scan_y<draw_last_scanline_for_border)
      {
        if(draw_store_dest_ad==NULL && pixels_in0<=BORDER_SIDE+320+BORDER_SIDE)
        {
          draw_store_dest_ad=draw_dest_ad;
          //ScanlineBuffer=
          draw_dest_ad=draw_temp_line_buf;
          draw_store_draw_scanline=draw_scanline;
        }
        if(draw_store_dest_ad) 
          draw_scanline=draw_scanline_1_line[screen_res];
      }
    }
#endif
    if(Glue.FetchingLine())
    {
      int border1=0,border2=0,picture=0,hscroll=0; // parameters for ASM routine
      int picture_left_edge=left_border; // 0, BS, BS-4, BS-16 (...)
      //last pixel from extreme left to draw of picture
      int picture_right_edge=BORDER_SIDE+320+BORDER_SIDE-right_border;
      if(pixels_in>picture_left_edge)
      { //might be some picture to draw = fetching RAM
        if(scanline_drawn_so_far>picture_left_edge)
        {
          picture=pixels_in-scanline_drawn_so_far;
          if(picture>picture_right_edge-scanline_drawn_so_far)
            picture=picture_right_edge-scanline_drawn_so_far;
        }
        else
        {
          picture=pixels_in-picture_left_edge;
          if(picture>picture_right_edge-picture_left_edge)
            picture=picture_right_edge-picture_left_edge;
        }
        if(picture<0)
          picture=0;
      }
      if(scanline_drawn_so_far<left_border)
      {
        if(pixels_in>left_border)
          border1=left_border-scanline_drawn_so_far;
        else
          border1=pixels_in-scanline_drawn_so_far; // we're not yet at end of border
        if(border1<0) 
          border1=0;
      }
      border2=pixels_in-scanline_drawn_so_far-border1-picture;
      if(border2<0) 
        border2=0;
      // We Were distorter: must do this check later than at "left off"
      if(!left_border&&HSCROLL&&SideBorderSize!=VERY_LARGE_BORDER_SIDE&&!screen_res
        &&!scanline_drawn_so_far&&shifter_pixel>15)
        SHIFT_SDP(8), shifter_pixel-=16;
      int old_shifter_pixel=shifter_pixel;
      shifter_pixel+=(short)picture;
      MEM_ADDRESS nsdp=shifter_draw_pointer;
      if(OPTION_C2&&shifter_draw_pointer>=himem) // no RAM
        shifter_draw_pointer=0+scan_y*160; // just a bad approximation of the effect
      // On lines -2, don't fetch the last 2 bytes as if it was a 160byte line.
      // Fixes screen #2 of the venerable B.I.G. Demo.
      // The MEDRES stuff is hypothetical
      if(Glue.CurrentScanline.Tricks&TRICK_LINE_MINUS_2)
      {
        int pixels_to_skip=(screen_res ? (4*2) : (8*2));
        if(picture>=pixels_to_skip)
          picture-=pixels_to_skip,border2+=pixels_to_skip;
      }
      else if(HIRES_COLOUR && picture>=160+16)
        picture-=160+16;
      if(!screen_res) // LOW RES
      {
        hscroll=(old_shifter_pixel&15);
        nsdp-=(old_shifter_pixel/16)*8;
        nsdp+=(shifter_pixel/16)*8; // is sdp forward at end of line?
/*  This is where we do the actual shift for those rare programs using the
    overscan 4bit hardscroll trick.
    Notice it is quite simple, and also very efficient because it uses 
    the hscroll parameter of the assembly drawing routine (programmed by
    Steem authors, of course).
    hscroll>15 is handled further.
*/
        if(Glue.CurrentScanline.Tricks&TRICK_4BIT_SCROLL)
        {
          hscroll-=HblPixelShift;
          if(hscroll<0)
          {
            if(picture>-hscroll)
            {
              picture+=hscroll,border1-=hscroll;
              hscroll=0;
            }
            else if(!picture) // phew
              hscroll+=HblPixelShift;
          }
        }
      }
      else if(screen_res==1) // MEDIUM RES
      {
        hscroll=((old_shifter_pixel*2)&15);
        if(HblStartingHscroll&1) // not useful for anything known yet!
        { 
          hscroll++; // restore precision
          HblStartingHscroll=0; // only once/scanline (?)
        }
        nsdp-=(old_shifter_pixel/8)*4;
        nsdp+=(shifter_pixel/8)*4;
      }
      if(draw_lock) // draw_lock is set true in draw_begin(), false in draw_end()
      {
        // real lines
        if(scan_y>=draw_first_possible_line && scan_y<draw_last_possible_line)
        {
          // actually draw it
          if(picture_left_edge<0) 
            picture+=picture_left_edge;
          AUTO_BORDER_ADJUST; // hack borders if necessary
          DEBUG_ONLY( shifter_draw_pointer+=debug_screen_shift; );
          if(hscroll>=16) // convert excess hscroll in SDP shift
          {
            shifter_draw_pointer+=(SHIFTER_RASTER_PREFETCH_TIMING/2)
              *(hscroll/16);
            hscroll-=16*(hscroll/16);
          }
          // call to appropriate ASSEMBLER routine!
          ///////////////// RENDER VIDEO /////////////////
          draw_scanline(border1,picture,border2,hscroll); 
        }
      }
      shifter_draw_pointer=nsdp;
    }
    // overscan lines = a big "left border"
    else if(scan_y>=draw_first_scanline_for_border 
      && scan_y<draw_last_scanline_for_border)
    {
      // top lines of 60hz screens are black in our big window, it's that or resizing the
      // window, which has its drawbacks
      // we don't do the bottom here but in event_vbl_interrupt() which takes care of
      // other cases
      long savepal0=PCpal[0];
      if(DISPLAY_SIZE && Glue.previous_video_freq==60 // mimic C3's window
        && (scan_y<=draw_first_scanline_for_border60
        ||scan_y>draw_last_scanline_for_border60))
        PCpal[0]=0;
      int border1; // the only var. sent to draw_scanline
      int left_visible_edge,right_visible_edge;
      // Borders on
      if(border)
      {
        left_visible_edge=0;
        right_visible_edge=BORDER_SIDE+320+BORDER_SIDE;
      }
      // No, only the part between the borders
      else
      {
        left_visible_edge=BORDER_SIDE;
        right_visible_edge=BORDER_SIDE+320;
      }
      if(scanline_drawn_so_far<=left_visible_edge)
        border1=pixels_in-left_visible_edge;
      else
        border1=pixels_in-scanline_drawn_so_far;
      if(border1<0)
        border1=0;
      else if(border1> (right_visible_edge - left_visible_edge))
        border1=(right_visible_edge - left_visible_edge);
      if(scan_y>=draw_first_possible_line && scan_y<draw_last_possible_line)
        ///////////////// RENDER VIDEO /////////////////
        draw_scanline(border1,0,0,0);
      PCpal[0]=savepal0;
    }
    scanline_drawn_so_far=pixels_in;
  }//if(pixels_in>=0)
}


void TShifter::Reset(bool Cold) {
  m_ShiftMode=0; // as hardware
#if defined(SSE_HIRES_COLOUR)
  HIRES_COLOUR=false;
#endif
  if(Cold) // Closure second run
  {
    for(int i=0;i<32;i++)
    {
      shifter_freq_change[i]=0;
      shifter_mode_change[i]=0; // Hackabonds after reset
      shifter_freq_change_time[i]=shifter_mode_change_time[i]=0;
    }
    nVbl=0;
  }
  Preload=0;
  shifter_hscroll=0;
#ifdef WIN32
  if(OPTION_RANDOM_WU)
#endif
    WakeupShift=((rand()%8)==7); // 0 (likely) or 1
#if defined(SSE_MEGASTE)
  if(IS_MEGASTE)
    WakeupShift=1-WakeupShift; // maybe
#endif
  video_first_draw_line=0;
  video_last_draw_line=shifter_y;
  shifter_sound_mode=BIT_7;
  ste_sound_freq=ste_sound_mode_to_freq[0];
  ste_sound_output_countdown=0;
  ste_sound_samples_countdown=0;
  sound_fifo_idx=0;
#if defined(SSE_SOUND_CARTRIDGE) //hack to reduce pops
  if(SSEConfig.mv16)
    ste_sound_last_word=(DONGLE_ID==TDongle::MUSIC_MASTER)?29408:8352;
  else
#endif
   ste_sound_last_word=(RENDER_SIGNED_SAMPLES) ? 0 : MAKEWORD(0,0);
}


void TShifter::Restore() {
  m_ShiftMode&=3;
  HiresRaster=0;
  shifter_hscroll&=15;
  sound_fifo_idx&=3;
  Microwire.old_top_val_l=Microwire.top_val_l;
  Microwire.old_top_val_r=Microwire.top_val_r;
}


void TShifter::RoundCycles(short& cycles_in) { // hacky but effective?
  cycles_in-=Glue.ScanlineTiming[Glue.RENDER_CYCLE][Glue.FREQ_50];
  if(Glue.hscroll && !HSCROLL) 
    cycles_in+=16;
  cycles_in+=SHIFTER_RASTER_PREFETCH_TIMING;
  cycles_in&=-SHIFTER_RASTER_PREFETCH_TIMING;
  cycles_in+=Glue.ScanlineTiming[Glue.RENDER_CYCLE][Glue.FREQ_50];
  if(Glue.hscroll && !HSCROLL) 
    cycles_in-=16;
}


//static (for STVL)
void TShifter::SetPal(int n, WORD NewPal) {
  NewPal&=(IS_STF) ? 0x0777 : 0x0FFF;
#if defined(SSE_STATS)
  Stats.nPal++;
#endif
  if(STpal[n]!=NewPal)
  {
#if defined(SSE_STATS)
    if(NewPal&0x888)
    {
      Stats.nExtendedPal++;
      if(LITTLE_PC>rom_addr)
        Stats.nExtendedPalT++;
    }
#endif
    short CyclesIn=(short)LINECYCLES;
    if(!OPTION_C3 && draw_lock
      && CyclesIn>MMU_PREFETCH_LATENCY+SHIFTER_RASTER_PREFETCH_TIMING)
      Shifter.Render(CyclesIn,DISPATCHER_SET_PAL);
/*  v3.8.0
    Record when palette is changed during display, so that we apply effect
    at rendering time (there's no real-time rendering for HIRES).
    Limitations:
    -Our trick uses video RAM, so this limits the field to real picture.
    -Resolution is byte, not bit/cycle.
    Seems to be enough for Time Slices
*/
    if(screen_res==2&&Glue.FetchingLine())
    {
      int time_to_first_pixel
        =Glue.ScanlineTiming[TGlue::LINE_START_LIMIT][TGlue::FREQ_71]+28-6;
      int cycle=CyclesIn-time_to_first_pixel;
      if(cycle>=0&&cycle<160) // only during Shifter DE
      {
        BYTE byte=(BYTE)(cycle/2); // HIRES: 4 pixels/cycle
        for(int i=Shifter.HiresRaster;i<=byte;i++) // from previous pixel chunk to now
          Shifter.Scanline2[i]=(NewPal&1)?0xFF:0x00; // and not the reverse...
        Shifter.HiresRaster=byte; //record where we are
      }
    }
    STpal[n]=NewPal;
    PAL_DPEEK(n*2)=STpal[n];
#if defined(SSE_VID_STVL2)
    Stvl.ST_pal[n]=NewPal;
#endif
    if(!flashlight_flag&&!draw_line_off DEBUG_ONLY(&&!debug_cycle_colours))
      palette_convert(n);
  }
}


#undef LOGSECTION


///////////
// SOUND //
///////////

#define LOGSECTION LOGSECTION_SOUND

WORD *ste_sound_channel_buf=NULL;
DWORD ste_sound_channel_buf_len=0; // variable
DWORD ste_sound_channel_buf_idx;
WORD ste_sound_last_word;
const WORD ste_sound_mode_to_freq[4]={6258,12517,25033,50066}; // available frequencies :(
WORD ste_sound_freq;
int ste_sound_output_countdown,ste_sound_samples_countdown;


void TShifter::sound_set_mode(BYTE new_mode) {
/*   FF8920 0000 0000 m000 00rr RW Sound Mode Control
     rr:
     00   6258 Hz sample rate (reset state)
     01  12517 Hz sample rate
     10  25033 Hz sample rate
     11  50066 Hz sample rate
     m:
     0	Stereo Mode (reset state)
     1 Mono Mode
*/
  new_mode&=0x83;
  TRACE_LOG("DMA sound mode %X freq %d\n",new_mode,ste_sound_mode_to_freq[new_mode&3]);
  shifter_sound_mode=new_mode;
  ste_sound_freq=ste_sound_mode_to_freq[shifter_sound_mode&3];
  SampleRate=sound_freq;
  LOG_TO(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound mode set to $"+HEXSl(shifter_sound_mode,2)+" freq="+ste_sound_freq);
}


void TShifter::sound_get_last_sample(WORD *pw1,WORD *pw2) {
  if(shifter_sound_mode&BIT_7)
  {  // ST plays HIBYTE, LOBYTE, so last sample is LOBYTE
    *pw1=WORD((ste_sound_last_word & 0x00ff));
    if(RENDER_SIGNED_SAMPLES)
      *pw1 <<= 6;
    *pw2=*pw1; // play the same in both channels, or ignored in when sound_num_channels==1
  }
  else
  {
    if(sound_num_channels==1)
    {
      //average the channels out
      if(RENDER_SIGNED_SAMPLES)
        *pw1=BYTE(((char)(ste_sound_last_word & 255)
          +(char)(ste_sound_last_word >> 8))/2);
      else
        *pw1=WORD(((ste_sound_last_word & 255)+(ste_sound_last_word>>8)) << 5);
      *pw2=0; // skipped
    }
    else
    {
      if(RENDER_SIGNED_SAMPLES)
      {
        *pw1=WORD((ste_sound_last_word & 0xff00)>>8);
        *pw2=WORD((ste_sound_last_word & 0x00ff));
      }
      else
      {
        *pw1=WORD((ste_sound_last_word & 0xff00) >> 2);
        *pw2=WORD((ste_sound_last_word & 0x00ff) << 6);
      }
    }
  }
}


void TShifter::sound_play() {
  bool Mono=((shifter_sound_mode&BIT_7)!=0);
  //we want to play a/b samples, where a is the DMA sound frequency
  //and b is the number of scanlines a second
  int left_vol_top_val=Microwire.top_val_l,right_vol_top_val=Microwire.top_val_r;
  //this a/b is the same as ste_sound_freq*scanline_time_in_cpu_cycles/8million
  if(Mono)
  {  //play half as many words
    ste_sound_samples_countdown+=ste_sound_freq*scanline_time_in_cpu_cycles_at_start_of_vbl/2;
    left_vol_top_val=(Microwire.top_val_l >> 1)+(Microwire.top_val_r >> 1);
    right_vol_top_val=left_vol_top_val;
  }
  else //stereo, 1 word per sample
    ste_sound_samples_countdown+=ste_sound_freq*scanline_time_in_cpu_cycles_at_start_of_vbl;
  bool vol_change_l=(left_vol_top_val<128),vol_change_r=(right_vol_top_val<128);
  while(ste_sound_samples_countdown>=0)
  {  //play word from buffer
    if(Shifter.sound_fifo_idx>0)
    {
      ste_sound_last_word=Shifter.sound_fifo[0];
      for(int i=0;i<3;i++)
        Shifter.sound_fifo[i]
          =Shifter.sound_fifo[i+1];
      Shifter.sound_fifo_idx--;
      if(vol_change_l)
      {
        int b1=(signed char)(HIBYTE(ste_sound_last_word));
        b1*=left_vol_top_val;
        b1/=128;
        ste_sound_last_word&=0x00ff;
        ste_sound_last_word|=WORD(BYTE(b1)<<8);
      }
      if(vol_change_r)
      {
        int b2=(signed char)(LOBYTE(ste_sound_last_word));
        b2*=right_vol_top_val;
        b2/=128;
        ste_sound_last_word&=0xff00;
        ste_sound_last_word|=BYTE(b2);
      }
      if(!RENDER_SIGNED_SAMPLES)
        ste_sound_last_word^=WORD((128 << 8) | 128); // unsign
    }
    ste_sound_output_countdown+=sound_freq;
    WORD w1,w2;
    if(Mono)
    {       //mono, play half as many words
      if(RENDER_SIGNED_SAMPLES)
      {
        w1=WORD((ste_sound_last_word & 0xff00)>>8);
        w2=WORD((ste_sound_last_word & 0x00ff));
      }
      else
      {
        w1=WORD((ste_sound_last_word & 0xff00) >> 2);
        w2=WORD((ste_sound_last_word & 0x00ff) << 6);
      }
      // ste_sound_channel_buf always stereo, so put each mono sample in twice
      while(ste_sound_output_countdown>=0)
      {
        if(ste_sound_channel_buf_idx>=STE_SOUND_BUFFER_LENGTH)
          break;
        ste_sound_channel_buf[ste_sound_channel_buf_idx++]=w1;
        ste_sound_channel_buf[ste_sound_channel_buf_idx++]=w1;
        ste_sound_output_countdown-=ste_sound_freq;
      }
      ste_sound_output_countdown+=sound_freq;
      while(ste_sound_output_countdown>=0)
      {
        if(ste_sound_channel_buf_idx>=STE_SOUND_BUFFER_LENGTH)
          break;
        ste_sound_channel_buf[ste_sound_channel_buf_idx++]=w2;
        ste_sound_channel_buf[ste_sound_channel_buf_idx++]=w2;
        ste_sound_output_countdown-=ste_sound_freq;
      }
    }
    else
    {//stereo , 1 word per sample
/*
 Stereo Samples have to be organized wordwise like 
 Lowbyte -> right channel
 Hibyte  -> left channel
*/
      if(sound_num_channels==1)
      {  //average the channels out
        if(RENDER_SIGNED_SAMPLES) // avoid clipping (Rebirth)
          w1=BYTE(((char)(ste_sound_last_word & 255)
            +(char)(ste_sound_last_word>>8))/2); 
        else
          w1=WORD(((ste_sound_last_word & 255)+(ste_sound_last_word >> 8)) << 5);
        w2=0; // skipped //SS no mixdown...
      }
      else if(RENDER_SIGNED_SAMPLES)
      {
        w1=WORD((ste_sound_last_word & 0xff00)>>8);
        w2=WORD((ste_sound_last_word & 0x00ff));
      }
      else
      {
        w1=WORD((ste_sound_last_word & 0xff00) >> 2);
        w2=WORD((ste_sound_last_word & 0x00ff) << 6);
      }
      while(ste_sound_output_countdown>=0)
      {
        if(ste_sound_channel_buf_idx>=STE_SOUND_BUFFER_LENGTH) 
          break;
        ste_sound_channel_buf[ste_sound_channel_buf_idx++]=w1;
        ste_sound_channel_buf[ste_sound_channel_buf_idx++]=w2;
        ste_sound_output_countdown-=ste_sound_freq;
      }
    }
    ste_sound_samples_countdown-=n_cpu_cycles_per_second; 
  }//while (ste_sound_samples_countdown>=0)
#if defined(SSE_VID_STVL_SREQ)
  if(OPTION_C3 && SSEConfig.Stvl>=0x101)
    Stvl.sreq=(sound_fifo_idx<4 && (Mmu.sound_control&BIT_0));
#endif
}
