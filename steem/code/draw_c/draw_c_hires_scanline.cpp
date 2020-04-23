/*---------------------------------------------------------------------------
FILE: draw_c_hires_scanline.cpp
MODULE: draw_c
DESCRIPTION: High res C++ drawing routine
---------------------------------------------------------------------------*/

//void draw_scanline_hires_pixelwise_8(int border1,int picture,int border2,int hscroll){
  int n;
  WORD w0;
  DWORD fore,back;
  if (STpal[0] & 1){
    back=0xffffffff;fore=0;
  }else{
    fore=0xffffffff;back=0;
  }

  MEM_ADDRESS source;
  GET_START(0,80)
  DRAW_BORDER_PIXELS(border1*16)
  for(n=picture;n>0;n--){
    GET_SCREEN_DATA_INTO_REGS_AND_INC_SA_HIRES
      for(int mask=BIT_15;mask;mask>>=1)
      {
        CALC_COL_HIRES_AND_DRAWPIXEL(mask);
      }
  }
  DRAW_BORDER_PIXELS(border2*16)
