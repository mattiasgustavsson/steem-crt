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

FILE: draw_c.cpp
DESCRIPTION: Alternative C++ drawing routines for systems that don't support
Steem's faster x86 assembler versions.
Used in the x64 build.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#define IN_DRAW_C

#include <easystr.h>
#include <conditions.h>
#include <steemh.h>
#include <draw.h>
#include <display.h>
#include <emulator.h>
#include <osd.h>
#include <shifter.h>
#include <mmu.h>

long PCpal_array[16];

extern "C" long* ASMCALL Get_PCpal(){
  return PCpal_array;
}

#define OSD_PIXEL(mask)  \
  if(dw1&mask){    \
    OSD_DRAWPIXEL(colour);  \
  }else if(dw0&mask){    \
    OSD_DRAWPIXEL(0);     \
  }else{              \
    dadd+=bpp; }

#if !defined(SSE_VID_32BIT_ONLY)

#define bpp 1

#define OSD_DRAWPIXEL(c) *(dadd++)=(BYTE)c;


extern "C" void ASMCALL osd_draw_char_8(long*source_ad,BYTE*draw_mem,long x,long y,int draw_line_length,long colour,long h){
#include "draw_c_osd_draw_char.cpp"
}

#undef OSD_DRAWPIXEL
#undef bpp

#define bpp 2

#define OSD_DRAWPIXEL(c) *(((WORD*)dadd))=(WORD)c,dadd+=2;


extern "C" void ASMCALL osd_draw_char_16(long*source_ad,BYTE*draw_mem,long x,long y,int draw_line_length,long colour,long h){
#include "draw_c_osd_draw_char.cpp"
}


extern "C" void ASMCALL osd_blueize_line_16_555(int x,int y,int w){
#include "draw_c_osd_blueize_line.cpp"
}


extern "C" void ASMCALL osd_blueize_line_16_565(int x,int y,int w){
#include "draw_c_osd_blueize_line.cpp"
}


#undef OSD_DRAWPIXEL
#undef bpp

#define bpp 3
#define OSD_DRAWPIXEL(c) *(dadd++)=(BYTE)(c&255);*(dadd++)=(BYTE)(c>>8);*(dadd++)=(BYTE)(c>>16)


extern "C" void ASMCALL osd_draw_char_24(long*source_ad,BYTE*draw_mem,long x,long y,int draw_line_length,long colour,long h){
#include "draw_c_osd_draw_char.cpp"
}


#undef RAINBOW


extern "C" void ASMCALL osd_blueize_line_24(int x,int y,int w){
#include "draw_c_osd_blueize_line.cpp"
}


#undef OSD_DRAWPIXEL
#undef bpp

#endif//#if !defined(SSE_VID_32BIT_ONLY)

#define bpp 4

#define OSD_DRAWPIXEL(c) *(((LONG*)dadd))=c,dadd+=4;


extern "C" void ASMCALL osd_draw_char_32(long*source_ad,BYTE*draw_mem,long x,long y,int draw_line_length,long colour,long h){
#include "draw_c_osd_draw_char.cpp"
}


extern "C" void ASMCALL osd_blueize_line_32(int x,int y,int w){
#include "draw_c_osd_blueize_line.cpp"
}


#undef OSD_DRAWPIXEL
#undef bpp

#if !defined(SSE_VID_32BIT_ONLY)

extern "C" void ASMCALL osd_draw_char_clipped_8
  (long* src,BYTE* dst,long x,long y,int l,long c,long s,RECT* cr){
  if(x>=cr->left && x<=(cr->right-s)) // our clipping is gross
    osd_draw_char_8(src,dst,x,y,l,c,s);
}


extern "C" void ASMCALL osd_draw_char_clipped_16
  (long* src,BYTE* dst,long x,long y,int l,long c,long s,RECT* cr){
  if(x>=cr->left && x<=(cr->right-s)) 
    osd_draw_char_16(src,dst,x,y,l,c,s);
}


extern "C" void ASMCALL osd_draw_char_clipped_24
  (long* src,BYTE* dst,long x,long y,int l,long c,long s,RECT* cr){
  if(x>=cr->left && x<=(cr->right-s)) 
    osd_draw_char_24(src,dst,x,y,l,c,s);
}

#endif

extern "C" void ASMCALL osd_draw_char_clipped_32
  (long* src,BYTE* dst,long x,long y,int l,long c,long s,RECT* cr){
  if(x>=cr->left && x<=(cr->right-s)) 
    osd_draw_char_32(src,dst,x,y,l,c,s);
}

#if !defined(SSE_VID_32BIT_ONLY)

extern "C" void ASMCALL osd_draw_char_transparent_8
  (long* a,BYTE* b,long c,long d,int e,long f,long g){
  osd_draw_char_8(a,b,c,d,e,f,g);
}


extern "C" void ASMCALL osd_draw_char_transparent_16
  (long* a,BYTE* b,long c,long d,int e,long f,long g){
  osd_draw_char_16(a,b,c,d,e,f,g);
}


extern "C" void ASMCALL osd_draw_char_transparent_24
  (long* a,BYTE* b,long c,long d,int e,long f,long g){
  osd_draw_char_24(a,b,c,d,e,f,g);
}

#endif

extern "C" void ASMCALL osd_draw_char_transparent_32
  (long* a,BYTE* b,long c,long d,int e,long f,long g){
  osd_draw_char_32(a,b,c,d,e,f,g);
}


#if !defined(SSE_VID_32BIT_ONLY)

extern "C" void ASMCALL osd_draw_char_clipped_transparent_8
  (long* src,BYTE* dst,long x,long y,int l,long c,long s,RECT* cr){
  if(x>=cr->left && x<=(cr->right-s)) 
    osd_draw_char_8(src,dst,x,y,l,c,s);
}


extern "C" void ASMCALL osd_draw_char_clipped_transparent_16
  (long* src,BYTE* dst,long x,long y,int l,long c,long s,RECT* cr){
  if(x>=cr->left && x<=(cr->right-s)) 
    osd_draw_char_16(src,dst,x,y,l,c,s);
}


extern "C" void ASMCALL osd_draw_char_clipped_transparent_24
  (long* src,BYTE* dst,long x,long y,int l,long c,long s,RECT* cr){
  if(x>=cr->left && x<=(cr->right-s)) 
    osd_draw_char_24(src,dst,x,y,l,c,s);
}

#endif


extern "C" void ASMCALL osd_draw_char_clipped_transparent_32
  (long* src,BYTE* dst,long x,long y,int l,long c,long s,RECT* cr){
  if(x>=cr->left && x<=(cr->right-s)) 
    osd_draw_char_32(src,dst,x,y,l,c,s);
}

#if !defined(SSE_VID_32BIT_ONLY)
extern "C" void ASMCALL osd_black_box_8(void*,int,int,int,int,long){}
extern "C" void ASMCALL osd_black_box_16(void*,int,int,int,int,long){}
extern "C" void ASMCALL osd_black_box_24(void*,int,int,int,int,long){}
#endif
extern "C" void ASMCALL osd_black_box_32(void*,int,int,int,int,long){}


// increase = 80 or 160, doubleflag =0, not used
#define GET_START(doubleflag,increase)  \
  source=shifter_draw_pointer&0xffffff; \
  while(source+increase>mem_len)source-=increase; 

#define DRAW_BORDER(n)    \
  for(n*=8;n>0;n--){                       \
    DRAW_2_BORDER_PIXELS               \
  }


#define DRAW_BORDER_PIXELS_A(npixels) \
  for(int i=npixels;i;i--) \
  { \
    DRAWPIXEL(PCpal);\
  }


#define DRAW_BORDER_PIXELS DRAW_BORDER_PIXELS_A




#define GET_SCREEN_DATA_INTO_REGS_AND_INC_SA {\
  for(int i=0;i<4;i++,source+=2)\
    w[i]=*(WORD*)(Mem_End_minus_2-source);\
}
  

#define GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_MEDRES \
  w0=DPEEK(source);w1=DPEEK(source+2); \
  source+=4;


#define GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_HIRES \
  w0=DPEEK(source);      \
  source+=2;


#define CALC_COL_LOWRES_AND_DRAWPIXEL(mask) { \
  int nibble= ((w[0]&mask)!=0) + (((w[1]&mask)!=0)<<1) + (((w[2]&mask)!=0)<<2) + (((w[3]&mask)!=0)<<3); \
  DRAWPIXEL(PCpal+nibble)\
}


#define CALC_COL_MEDRES_AND_DRAWPIXEL(mask)   { \
  int nibble= ((w0&mask)!=0) + (((w1&mask)!=0)<<1) ; \
  DRAWPIXEL_MEDRES(PCpal+nibble)\
}


#define CALC_COL_HIRES_AND_DRAWPIXEL(mask) DRAWPIXEL((w0&mask)?fore:back)

#if !defined(SSE_VID_32BIT_ONLY)

#define DRAW_2_BORDER_PIXELS  *LPWORD(draw_dest_ad)=*LPWORD(PCpal);draw_dest_ad+=2;


#define DRAWPIXEL(s_add)  *(draw_dest_ad++)=*LPBYTE(s_add);


#define DRAWPIXEL_MEDRES(s) DRAWPIXEL(s)

extern "C" void ASMCALL draw_scanline_8_lowres_pixelwise(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}


extern "C" void ASMCALL draw_scanline_8_medres_pixelwise(int border1,int picture,int border2,int hscroll){
  border1*=2;border2*=2;
#include "draw_c_medres_scanline.cpp"
}


#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES


#define DRAW_2_BORDER_PIXELS  *LPDWORD(draw_dest_ad)=*LPDWORD(PCpal);draw_dest_ad+=4;


#define DRAWPIXEL(s_add)  *LPWORD(draw_dest_ad)=*LPWORD(s_add);draw_dest_ad+=2;


#define DRAWPIXEL_MEDRES(s) DRAWPIXEL(s)


extern "C" void ASMCALL draw_scanline_16_lowres_pixelwise(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}


extern "C" void ASMCALL draw_scanline_16_medres_pixelwise(int border1,int picture,int border2,int hscroll){
  border1*=2;border2*=2;
#include "draw_c_medres_scanline.cpp"
}


#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES


#define DRAW_2_BORDER_PIXELS  \
              *(draw_dest_ad++)=*LPBYTE(PCpal); \
              *(draw_dest_ad++)=*(LPBYTE(PCpal)+1); \
              *(draw_dest_ad++)=*(LPBYTE(PCpal)+2); \
              *(draw_dest_ad++)=*LPBYTE(PCpal); \
              *(draw_dest_ad++)=*(LPBYTE(PCpal)+1); \
              *(draw_dest_ad++)=*(LPBYTE(PCpal)+2);


#define DRAWPIXEL(s_add)  \
              *(draw_dest_ad++)=*LPBYTE(s_add); \
              *(draw_dest_ad++)=*(LPBYTE(s_add)+1); \
              *(draw_dest_ad++)=*(LPBYTE(s_add)+2);


#define DRAWPIXEL_MEDRES(s) DRAWPIXEL(s)


extern "C" void ASMCALL draw_scanline_24_lowres_pixelwise(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}


extern "C" void ASMCALL draw_scanline_24_medres_pixelwise(int border1,int picture,int border2,int hscroll){
  border1*=2;border2*=2;
#include "draw_c_medres_scanline.cpp"
}


#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES

#endif//#if !defined(SSE_VID_32BIT_ONLY)

#define DRAW_2_BORDER_PIXELS  \
          *LPDWORD(draw_dest_ad)=*LPDWORD(PCpal);draw_dest_ad+=4; \
          *LPDWORD(draw_dest_ad)=*LPDWORD(PCpal);draw_dest_ad+=4;


#define DRAWPIXEL(s_add)  *LPDWORD(draw_dest_ad)=*LPDWORD(s_add);draw_dest_ad+=4;


#define DRAWPIXEL_MEDRES(s) DRAWPIXEL(s)


// small display size or stretched (double, triple, D3D fullscreen...)

#undef DRAW_BORDER_PIXELS 
#define DRAW_BORDER_PIXELS(npixels) {\
  __stosd((DWORD*)draw_dest_ad,*PCpal,npixels);\
  draw_dest_ad+=4*npixels;\
}


extern "C" void ASMCALL draw_scanline_32_lowres_pixelwise(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}


extern "C" void ASMCALL draw_scanline_32_medres_pixelwise(int border1,int picture,int border2,int hscroll){
  border1*=2;border2*=2;
#include "draw_c_medres_scanline.cpp"
}


#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES
#undef DRAW_BORDER_PIXELS 

#define DRAW_BORDER_PIXELS DRAW_BORDER_PIXELS_A


///////////////////////////////////////////////////////////
////////////////////////// _dw  ///////////////////////////
///////////////////////////////////////////////////////////

#if !defined(SSE_VID_32BIT_ONLY)

#define DRAW_2_BORDER_PIXELS *(((WORD*)(draw_dest_ad)))=*(WORD*)(PCpal),draw_dest_ad+=2;*(((WORD*)(draw_dest_ad)))=*(WORD*)(PCpal),draw_dest_ad+=2;


#define DRAWPIXEL(s_add) *(((WORD*)(draw_dest_ad)))=*(WORD*)(s_add),draw_dest_ad+=2;


extern "C" void ASMCALL draw_scanline_8_lowres_pixelwise_dw(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}


#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL


#define DRAW_2_BORDER_PIXELS  *(((DWORD*)(draw_dest_ad)))=*(DWORD*)(PCpal),draw_dest_ad+=4;*(((DWORD*)(draw_dest_ad)))=*(DWORD*)(PCpal),draw_dest_ad+=4;
#define DRAWPIXEL(s_add)  *(((DWORD*)(draw_dest_ad)))=*(DWORD*)(s_add),draw_dest_ad+=4;


extern "C" void ASMCALL draw_scanline_16_lowres_pixelwise_dw(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}


#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL


#define DRAW_2_BORDER_PIXELS \
  {                              \
   for(int r=3*3;r>0;r-=3){                                                                                         \
    *(draw_dest_ad++)=*(BYTE*)(PCpal);*(draw_dest_ad++)=*((BYTE*)(PCpal)+1);*(draw_dest_ad++)=*((BYTE*)(PCpal)+2);   \
   } \
  }

#if 1 // probably a typo, compiler saw it
#define DRAWPIXEL(s_add)  \
  *(draw_dest_ad++)=*(BYTE*)(s_add);*(draw_dest_ad++)=*(((BYTE*)(s_add))+1);*(draw_dest_ad++)=*(((BYTE*)(s_add))+2);   \
  *(draw_dest_ad++)=*(BYTE*)(s_add);*(draw_dest_ad++)=*(((BYTE*)(s_add))+1);*(draw_dest_ad++)=*(((BYTE*)(s_add))+2); 
#else
#define DRAWPIXEL(s_add)  \
  *(draw_dest_ad++)=*(BYTE*)(PCpal);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+1);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+2);   \
  *(draw_dest_ad++)=*(BYTE*)(PCpal);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+1);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+2); 
#endif

extern "C" void ASMCALL draw_scanline_24_lowres_pixelwise_dw(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}


#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL

#endif//#if !defined(SSE_VID_32BIT_ONLY)

// double no stretch + scanlines
#define DRAW_2_BORDER_PIXELS *(((DWORD*)(draw_dest_ad)))=*(DWORD*)(PCpal),draw_dest_ad+=4;*(((DWORD*)(draw_dest_ad)))=*(DWORD*)(PCpal),draw_dest_ad+=4;*(((DWORD*)(draw_dest_ad)))=*(DWORD*)(PCpal),draw_dest_ad+=4;*(((DWORD*)(draw_dest_ad)))=*(DWORD*)(PCpal),draw_dest_ad+=4;
#define DRAWPIXEL(s_add)  *(((DWORD*)(draw_dest_ad)))=*(DWORD*)(s_add),draw_dest_ad+=4;*(((DWORD*)(draw_dest_ad)))=*(DWORD*)(s_add),draw_dest_ad+=4;


extern "C" void ASMCALL draw_scanline_32_lowres_pixelwise_dw(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}


#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL

#undef DRAW_BORDER_PIXELS 
#define DRAW_BORDER_PIXELS DRAW_BORDER_PIXELS_A


///////////////////////////////////////////////////////////
////////////////////////// _400 ///////////////////////////
///////////////////////////////////////////////////////////


#if !defined(SSE_VID_32BIT_ONLY)

#define DRAW_2_BORDER_PIXELS  *(((WORD*)(draw_dest_ad+draw_line_length)))=*(WORD*)(PCpal);*(((WORD*)(draw_dest_ad+draw_line_length+2)))=*(WORD*)(PCpal); \
                              *(((WORD*)(draw_dest_ad)))=*(WORD*)(PCpal),draw_dest_ad+=2;*(((WORD*)(draw_dest_ad)))=*(WORD*)(PCpal),draw_dest_ad+=2;


#define DRAWPIXEL(s_add)  *((WORD*)(draw_dest_ad+draw_line_length))=*(WORD*)(s_add);*(((WORD*)(draw_dest_ad)))=*(WORD*)(s_add),draw_dest_ad+=2;


#define DRAWPIXEL_MEDRES(s_add)  *((BYTE*)(draw_dest_ad+draw_line_length))=*(BYTE*)(s_add);*draw_dest_ad=*(BYTE*)(s_add),draw_dest_ad++;


extern "C" void ASMCALL draw_scanline_8_lowres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}


extern "C" void ASMCALL draw_scanline_8_medres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_medres_scanline.cpp"
}


#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES


#define DRAW_2_BORDER_PIXELS  *(((DWORD*)(draw_dest_ad+draw_line_length)))=*(DWORD*)(PCpal);*(((DWORD*)(draw_dest_ad+draw_line_length+4)))=*(DWORD*)(PCpal);*(((DWORD*)(draw_dest_ad)))=*(DWORD*)(PCpal),draw_dest_ad+=4;*(((DWORD*)(draw_dest_ad)))=*(DWORD*)(PCpal),draw_dest_ad+=4;


#define DRAWPIXEL(s_add)  *(((DWORD*)(draw_dest_ad+draw_line_length)))=*(DWORD*)(s_add);*(((DWORD*)(draw_dest_ad)))=*(DWORD*)(s_add),draw_dest_ad+=4;


#define DRAWPIXEL_MEDRES(s_add)  *(((WORD*)(draw_dest_ad+draw_line_length)))=*(WORD*)(s_add);*(((WORD*)(draw_dest_ad)))=*(WORD*)(s_add),draw_dest_ad+=4;


extern "C" void ASMCALL draw_scanline_16_lowres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}


extern "C" void ASMCALL draw_scanline_16_medres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_medres_scanline.cpp"
}


#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES


#define DRAW_2_BORDER_PIXELS \
  {BYTE* dadd=draw_dest_ad+draw_line_length;                                                                    \
   for(int r=3*3;r>0;r-=3){                                                                                      \
    *(dadd++)=*(BYTE*)(PCpal);*(dadd++)=*((BYTE*)(PCpal)+1);*(dadd++)=*((BYTE*)(PCpal)+2);                        \
   }                                                                                                               \
   for(int r=3*3;r>0;r-=3){                                                                                         \
    *(draw_dest_ad++)=*(BYTE*)(PCpal);*(draw_dest_ad++)=*((BYTE*)(PCpal)+1);*(draw_dest_ad++)=*((BYTE*)(PCpal)+2);   \
   } \
  }

#if 1
#define DRAWPIXEL(s_add)  \
  {BYTE* dadd=draw_dest_ad+draw_line_length;                                                                    \
  *(dadd++)=*(BYTE*)(s_add);*(dadd++)=*(((BYTE*)(s_add))+1);*(dadd++)=*(((BYTE*)(s_add))+2);                        \
  *(dadd++)=*(BYTE*)(s_add);*(dadd++)=*(((BYTE*)(s_add))+1);*(dadd++)=*(((BYTE*)(s_add))+2);                        \
  *(draw_dest_ad++)=*(BYTE*)(s_add);*(draw_dest_ad++)=*(((BYTE*)(s_add))+1);*(draw_dest_ad++)=*(((BYTE*)(s_add))+2);   \
  *(draw_dest_ad++)=*(BYTE*)(s_add);*(draw_dest_ad++)=*(((BYTE*)(s_add))+1);*(draw_dest_ad++)=*(((BYTE*)(s_add))+2); \
  }
#else
#define DRAWPIXEL(s_add)  \
  {BYTE* dadd=draw_dest_ad+draw_line_length;                                                                    \
  *(dadd++)=*(BYTE*)(PCpal);*(dadd++)=*(((BYTE*)(PCpal))+1);*(dadd++)=*(((BYTE*)(PCpal))+2);                        \
  *(dadd++)=*(BYTE*)(PCpal);*(dadd++)=*(((BYTE*)(PCpal))+1);*(dadd++)=*(((BYTE*)(PCpal))+2);                        \
  *(draw_dest_ad++)=*(BYTE*)(PCpal);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+1);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+2);   \
  *(draw_dest_ad++)=*(BYTE*)(PCpal);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+1);*(draw_dest_ad++)=*(((BYTE*)(PCpal))+2); \
  }
#endif

#define DRAWPIXEL_MEDRES(s_add)  \
  {BYTE* dadd=draw_dest_ad+draw_line_length;                                                                    \
  *(dadd++)=*(BYTE*)(s_add);*(dadd++)=*(((BYTE*)(s_add))+1);*(dadd++)=*(((BYTE*)(s_add))+2);                        \
  *(draw_dest_ad++)=*(BYTE*)(s_add);*(draw_dest_ad++)=*(((BYTE*)(s_add))+1);*(draw_dest_ad++)=*(((BYTE*)(s_add))+2);   \
  }


extern "C" void ASMCALL draw_scanline_24_lowres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}


extern "C" void ASMCALL draw_scanline_24_medres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_medres_scanline.cpp"
}


#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES

#endif//#if !defined(SSE_VID_32BIT_ONLY)

#define DRAW_2_BORDER_PIXELS \
  {DWORD* dadd=(DWORD*)(draw_dest_ad+draw_line_length);                                                                    \
   *(dadd++)=*(DWORD*)(PCpal);*(dadd++)=*(DWORD*)(PCpal);*(dadd++)=*(DWORD*)(PCpal);*(dadd++)=*(DWORD*)(PCpal);                                                                                \
   *(((DWORD*)(draw_dest_ad)))=*(DWORD*)(PCpal),draw_dest_ad+=4;*(((DWORD*)(draw_dest_ad)))=*(DWORD*)(PCpal),draw_dest_ad+=4;*(((DWORD*)(draw_dest_ad)))=*(DWORD*)(PCpal),draw_dest_ad+=4;*(((DWORD*)(draw_dest_ad)))=*(DWORD*)(PCpal),draw_dest_ad+=4; \
  }


#define DRAWPIXEL(s_add)  *(((DWORD*)(draw_dest_ad+draw_line_length)))=*(DWORD*)(s_add); \
                          *(((DWORD*)(draw_dest_ad+draw_line_length+4)))=*(DWORD*)(s_add); \
                          *(((DWORD*)(draw_dest_ad)))=*(DWORD*)(s_add),draw_dest_ad+=4;*(((DWORD*)(draw_dest_ad)))=*(DWORD*)(s_add),draw_dest_ad+=4;


#define DRAWPIXEL_MEDRES(s_add)  *(((DWORD*)(draw_dest_ad+draw_line_length)))=*(DWORD*)(s_add); \
                          *(((DWORD*)(draw_dest_ad)))=*(DWORD*)(s_add),draw_dest_ad+=4;


extern "C" void ASMCALL draw_scanline_32_lowres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_lowres_scanline.cpp"
}


extern "C" void ASMCALL draw_scanline_32_medres_pixelwise_400(int border1,int picture,int border2,int hscroll){
#include "draw_c_medres_scanline.cpp"
}


#undef DRAW_2_BORDER_PIXELS
#undef DRAWPIXEL
#undef DRAWPIXEL_MEDRES


#define DRAWPIXEL(col) *(draw_dest_ad++)=BYTE(col);


#undef DRAW_BORDER_PIXELS


#if !defined(SSE_VID_32BIT_ONLY)

#define DRAW_BORDER_PIXELS(npixels) \
  for(int i=npixels;i;i--) \
  { \
    DRAWPIXEL(0/*fore*/);\
  }

extern "C" void ASMCALL draw_scanline_8_hires(int border1,int picture,int border2,int){
#include "draw_c_hires_scanline.cpp"
}


#undef DRAWPIXEL


#define DRAWPIXEL(col) *(((WORD*)draw_dest_ad))=WORD(col),draw_dest_ad+=2;


extern "C" void ASMCALL draw_scanline_16_hires(int border1,int picture,int border2,int){
#include "draw_c_hires_scanline.cpp"
}


#undef DRAWPIXEL

#define DRAWPIXEL(col) *(draw_dest_ad++)=BYTE(col);*(draw_dest_ad++)=BYTE(col);*(draw_dest_ad++)=BYTE(col);


extern "C" void ASMCALL draw_scanline_24_hires(int border1,int picture,int border2,int){
#include "draw_c_hires_scanline.cpp"
}

#endif//#if !defined(SSE_VID_32BIT_ONLY)

/* Engineering Hardware Specification Atari Corp:
"In  monochrome mode the border color is always black."
*/

#undef DRAW_BORDER_PIXELS
#define DRAW_BORDER_PIXELS(npixels) {\
  __stosd((DWORD*)draw_dest_ad,0/*fore*//*scan_y>478?back:fore*/,npixels);\
  draw_dest_ad+=4*npixels;\
}

#undef DRAWPIXEL
#define DRAWPIXEL(col) *(((DWORD*)draw_dest_ad))=DWORD(col),draw_dest_ad+=4;

extern "C" void ASMCALL draw_scanline_32_hires(int border1,int picture,int border2,int){
#include "draw_c_hires_scanline.cpp"
}

#undef DRAWPIXEL

