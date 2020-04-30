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
FILE: display.cpp
DESCRIPTION: A class to encapsulate the process of outputting to the display.
This contains the DirectDraw, Direct3D and X code used by Steem for output.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <conditions.h>
#include <cpu.h>
#include <draw.h>
#include <gui.h>
#include <computer.h>
#include <parameters.h>
#include <display.h>
#include <interface_stvl.h>
#include <debug_framereport.h>
#include <palette.h>
#include <osd.h>
#include <translate.h>
#include <dderr_meaning.h>
#include <diskman.h>
#include <infobox.h>
#include <screen_saver.h>
#include <notifyinit.h>
#include <mymisc.h>
#include <debugger.h>
#if defined(SSE_VID_RECORD_AVI)
#include <AVI/AviFile.h> // AVI (DD-only)
#endif
#include <stdarg.h>

#ifdef STEEM_CRT
    #define CRTEMU_IMPLEMENTATION
    #include "..\..\crtemu.h"

    #define CRT_FRAME_IMPLEMENTATION
    #include "..\..\crt_frame.h"

    #define FRAMETIMER_IMPLEMENTATION
    #include "..\..\frametimer.h"

    #define THREAD_IMPLEMENTATION
    #include "..\..\thread.h"
    #undef WINVER
    #undef _WIN32_WINNT
#endif

BYTE FullScreen=0;

#if defined(BCC_BUILD) && defined(SSE_VID_DD)
#pragma message DD DIRECTDRAW_VERSION
#endif


/////////////
// Borders //
/////////////

BYTE SideBorderSize=ORIGINAL_BORDER_SIDE; // 32
BYTE SideBorderSizeWin=ORIGINAL_BORDER_SIDE; // 32
BYTE BottomBorderSize=ORIGINAL_BORDER_BOTTOM; // 40

const POINT WinSizeBorderOriginal[4][5]={ 
{{320+ORIGINAL_BORDER_SIDE*2,200+(ORIGINAL_BORDER_TOP+ORIGINAL_BORDER_BOTTOM)},
{640+(ORIGINAL_BORDER_SIDE*2)*2,400+2*(ORIGINAL_BORDER_TOP+ORIGINAL_BORDER_BOTTOM)},
{960+(ORIGINAL_BORDER_SIDE*3)*2, 600+3*(ORIGINAL_BORDER_TOP+ORIGINAL_BORDER_BOTTOM)},
{1280+(ORIGINAL_BORDER_SIDE*4)*2,800+4*(ORIGINAL_BORDER_TOP+ORIGINAL_BORDER_BOTTOM)},
{-1,-1}},
{{640+(ORIGINAL_BORDER_SIDE*2)*2,200+(ORIGINAL_BORDER_TOP+ORIGINAL_BORDER_BOTTOM)},
{640+(ORIGINAL_BORDER_SIDE*2)*2,400+2*(ORIGINAL_BORDER_TOP+ORIGINAL_BORDER_BOTTOM)},
{1280+(ORIGINAL_BORDER_SIDE*4)*2,400+2*(ORIGINAL_BORDER_TOP+ORIGINAL_BORDER_BOTTOM)},
{1280+(ORIGINAL_BORDER_SIDE*4)*2,800+4*(ORIGINAL_BORDER_TOP+ORIGINAL_BORDER_BOTTOM)},
{-1,-1}},
{{640+(ORIGINAL_BORDER_SIDE*2)*2,400+2*(ORIGINAL_BORDER_TOP+ORIGINAL_BORDER_BOTTOM)},
{1280+(ORIGINAL_BORDER_SIDE*4)*2,800+4*(ORIGINAL_BORDER_TOP+ORIGINAL_BORDER_BOTTOM)},
{-1,-1}},
{{800,600},
{-1,-1}}
};


const POINT WinSizeBorderLarge[4][5]={ 
{{320+LARGE_BORDER_SIDE*2,200+(ORIGINAL_BORDER_TOP+LARGE_BORDER_BOTTOM)},
{640+(LARGE_BORDER_SIDE*2)*2,400+2*(ORIGINAL_BORDER_TOP+LARGE_BORDER_BOTTOM)}, 
{960+(LARGE_BORDER_SIDE*3)*2, 600+3*(ORIGINAL_BORDER_TOP+LARGE_BORDER_BOTTOM)},
{1280+(LARGE_BORDER_SIDE*4)*2,800+4*(ORIGINAL_BORDER_TOP+LARGE_BORDER_BOTTOM)},
{-1,-1}},
{{640+(LARGE_BORDER_SIDE*2)*2,200+(ORIGINAL_BORDER_TOP+LARGE_BORDER_BOTTOM)},
{640+(LARGE_BORDER_SIDE*2)*2,400+2*(ORIGINAL_BORDER_TOP+LARGE_BORDER_BOTTOM)},
{1280+(LARGE_BORDER_SIDE*4)*2,400+2*(ORIGINAL_BORDER_TOP+LARGE_BORDER_BOTTOM)},
{1280+(LARGE_BORDER_SIDE*4)*2,800+4*(ORIGINAL_BORDER_TOP+LARGE_BORDER_BOTTOM)},
{-1,-1}},
{{640+(LARGE_BORDER_SIDE*2)*2,400+2*(ORIGINAL_BORDER_TOP+LARGE_BORDER_BOTTOM)},
{1280+(LARGE_BORDER_SIDE*4)*2,800+4*(ORIGINAL_BORDER_TOP+LARGE_BORDER_BOTTOM)},
{-1,-1}},
{{800,600},
{-1,-1}}
};


const POINT WinSizeBorderMax[4][5]={ 
{{320+VERY_LARGE_BORDER_SIDE*2,200+(BIG_BORDER_TOP+VERY_LARGE_BORDER_BOTTOM)},
{640+(VERY_LARGE_BORDER_SIDE*2)*2,400+2*(BIG_BORDER_TOP+VERY_LARGE_BORDER_BOTTOM)}, 
{960+(VERY_LARGE_BORDER_SIDE*3)*2, 600+3*(BIG_BORDER_TOP+VERY_LARGE_BORDER_BOTTOM)},
{1280+(VERY_LARGE_BORDER_SIDE*4)*2,800+4*(BIG_BORDER_TOP+VERY_LARGE_BORDER_BOTTOM)},
{-1,-1}},
{{640+(VERY_LARGE_BORDER_SIDE*2)*2,200+(BIG_BORDER_TOP+VERY_LARGE_BORDER_BOTTOM)},
{640+(VERY_LARGE_BORDER_SIDE*2)*2,400+2*(BIG_BORDER_TOP+VERY_LARGE_BORDER_BOTTOM)},
{1280+(VERY_LARGE_BORDER_SIDE*4)*2,400+2*(BIG_BORDER_TOP+VERY_LARGE_BORDER_BOTTOM)},
{1280+(VERY_LARGE_BORDER_SIDE*4)*2,800+4*(BIG_BORDER_TOP+VERY_LARGE_BORDER_BOTTOM)},
{-1,-1}},
{{640+(VERY_LARGE_BORDER_SIDE*2)*2,400+2*(BIG_BORDER_TOP+VERY_LARGE_BORDER_BOTTOM)},
{1280+(VERY_LARGE_BORDER_SIDE*4)*2,800+4*(BIG_BORDER_TOP+VERY_LARGE_BORDER_BOTTOM)},
{-1,-1}},
{{800,600},
{-1,-1}}
};

POINT WinSizeBorder[4][5];


#define LOGSECTION LOGSECTION_VIDEO_RENDERING

void ChangeBorderSize(int size) {
  if(size<0||size>BIGGEST_DISPLAY)
    size=0;
  switch(size) {
  case 0: //no border, still need the same figures as normal border
  case 1:
    SideBorderSize=ORIGINAL_BORDER_SIDE;
    SideBorderSizeWin=ORIGINAL_BORDER_SIDE;
    BottomBorderSize=ORIGINAL_BORDER_BOTTOM;
    break;
  case 2:
    SideBorderSize=VERY_LARGE_BORDER_SIDE; // render 420
    SideBorderSizeWin=LARGE_BORDER_SIDE; // show 412
    BottomBorderSize=LARGE_BORDER_BOTTOM;
    break;
  case 3:
    SideBorderSize=VERY_LARGE_BORDER_SIDE; // render 420
    SideBorderSizeWin=VERY_LARGE_BORDER_SIDE; // show 420
    BottomBorderSize=VERY_LARGE_BORDER_BOTTOM;
    break;
  }//sw
  int i,j;
  for(i=0;i<4;i++) 
  {
    for(j=0;j<5;j++) 
    {
      switch(size) {
      case 0:
      case 1:
        WinSizeBorder[i][j]=WinSizeBorderOriginal[i][j];
        break;
      case 2:
        WinSizeBorder[i][j]=WinSizeBorderLarge[i][j];
        break;
      case 3:
        WinSizeBorder[i][j]=WinSizeBorderMax[i][j];
        break;
      }//sw
    }//nxt j
  }//nxt i
  draw_first_scanline_for_border=res_vertical_scale*(-BORDER_TOP);
  draw_last_scanline_for_border=shifter_y+res_vertical_scale*(BORDER_BOTTOM);
  TRACE_LOG("ChangeBorderSize(%d) side %d side win %d bottom %d\n",size,SideBorderSize,SideBorderSizeWin,BottomBorderSize);
#if defined(SSE_VID_STVL1)
  StvlUpdate();
#endif
  StemWinResize();
  Disp.ScreenChange();
  draw_begin(); // Lock() will update draw_line_length
  draw_end();
}

#undef LOGSECTION


extern "C"
{
#if defined(SSE_VID_32BIT_ONLY)
  const BYTE BytesPerPixel=4;
  BYTE rgb32_bluestart_bit=0;
#else
  BYTE BytesPerPixel=4,rgb32_bluestart_bit=0;
#endif
  bool rgb555=0;
  DWORD monitor_width,monitor_height; //true size of monitor, for LAPTOP mode.
}


#ifdef WIN32

bool TryDX=true;
#if !defined(SSE_VID_NO_FREEIMAGE)
// definition part of SteemFreeImage.h
FI_INITPROC FreeImage_Initialise;
FI_DEINITPROC FreeImage_DeInitialise;
FI_CONVFROMRAWPROC FreeImage_ConvertFromRawBits;
FI_SAVEPROC FreeImage_Save;
FI_FREEPROC FreeImage_Free;
FI_SUPPORTBPPPROC FreeImage_FIFSupportsExportBPP;
#endif//#if !defined(SSE_VID_NO_FREEIMAGE)

#if defined(SSE_VID_DD)
#if !defined(MINGW_BUILD)
SET_GUID(CLSID_DirectDraw,0xD7B70EE0,0x4340,0x11CF,0xB0,0x63,0x00,0x20,0xAF,
  0xC2,0xCD,0x35);
SET_GUID(IID_IDirectDraw,0x6C14DB80,0xA733,0x11CE,0xA5,0x21,0x00,0x20,0xAF,
  0x0B,0xE5,0x60);
SET_GUID(IID_IDirectDraw2,0xB3A6F3E0,0x2B43,0x11CF,0xA2,0xDE,0x00,0xAA,0x00,
  0xB9,0x33,0x56);
#endif
#if defined(SSE_VID_DD7) 
#if _MSC_VER == 1200 //VC6
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
DEFINE_GUID( IID_IDirectDraw7,0x15e65ec0,0x3b9c,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b );
#endif
#endif
BYTE HzIdxToHz[NUM_HZ]={0,50,60,71,100,120};

#if defined(SSE_VID_2SCREENS)

void get_fullscreen_totalrect(RECT *rc) {
  if(draw_fs_blit_mode==DFSM_STRETCHBLIT)
  {
    rc->left=rc->top=0;
    rc->right=Disp.fs_res[Disp.fs_res_choice].x;
    rc->bottom=Disp.fs_res[Disp.fs_res_choice].y;
  }
  else if(draw_fs_blit_mode==DFSM_FAKEFULLSCREEN)
  {
    if(OPTION_FAKE_FULLSCREEN)
      *rc=Disp.rcMonitor; // monitor 1 or 2
    else
    { // monitor 1
      rc->left=rc->top=0;
      rc->right=monitor_width;
      rc->bottom=monitor_height;
    }
  }
  else
  {
    rc->left=rc->top=0;
    rc->right=Disp.SurfaceWidth;
    rc->bottom=Disp.SurfaceHeight;
  } 
}

#endif


void get_fullscreen_rect(RECT *rc) {
  if(draw_fs_blit_mode==DFSM_STRETCHBLIT)
  {
    rc->left=rc->top=0;
    rc->right=Disp.fs_res[Disp.fs_res_choice].x;
    rc->bottom=Disp.fs_res[Disp.fs_res_choice].y;
  }
  else if(draw_fs_blit_mode==DFSM_FAKEFULLSCREEN) 
  {
#if defined(SSE_VID_DD) && defined(SSE_VID_2SCREENS)
    if(OPTION_FAKE_FULLSCREEN)
      *rc=Disp.rcMonitor;
    else
#endif
    {
      rc->left=rc->top=0;
      rc->right=monitor_width;
      rc->bottom=monitor_height;
    }
  }
  else if(extended_monitor) 
  {
    rc->left=rc->top=0;
    rc->right=MIN(em_width,Disp.SurfaceWidth);
    rc->bottom=MIN(em_height,Disp.SurfaceHeight);
    if(FullScreen && runstate!=RUNSTATE_RUNNING)
    {
      int x_gap=(Disp.SurfaceWidth-em_width)/2;
      int y_gap=(Disp.SurfaceHeight-em_height)/2;
      rc->left+=x_gap;
      rc->right+=x_gap;
      rc->top+=y_gap;
      rc->bottom+=y_gap;
    }
  }
  else if(border)
  {
    rc->left=(800-(BORDER_SIDE+320+BORDER_SIDE)*2)/2;
    rc->top=(600-(BORDER_TOP+200+BORDER_BOTTOM)*2)/2;
    rc->right=rc->left+(BORDER_SIDE+320+BORDER_SIDE)*2;
    rc->bottom=rc->top+(BORDER_TOP+200+BORDER_BOTTOM)*2;
  }
  else
  {
    rc->left=0;
    rc->top=int(using_res_640_400?0:40);
    rc->right=640;
    rc->bottom=int(using_res_640_400?400:440);
  }
}


unsigned short UNMAKE_DDHRESULT(long code) {
  long FACDD=0x876;
  return (unsigned short)((FACDD>>16)^code);
}


//If the function succeeds, the return value is the number of bytes
//copied into the buffer, not including the null-terminating character,
//or zero if the error does not exist.
int DDGetErrorDescription(HRESULT Error,char *buf,int size) {
  //HInstance is a Steem global, this didn't belong to 'include'
  return LoadString(HInstance,UNMAKE_DDHRESULT(Error),buf,size);
}

#if defined(SSE_DEBUG)

char *GetTextFromDDError(HRESULT hr) {  
  static char text[100];
  DDGetErrorDescription(hr,text,99);
  return text;
}

#define REPORT_DD_ERR(function,dderr) if(dderr) TRACE_LOG("DD ERR "function" %s\n",GetTextFromDDError(dderr))

#else

#define REPORT_DD_ERR 

#endif

#endif//dd

#if defined(SSE_DEBUG) && defined(SSE_VID_D3D)  
char *GetTextFromD3DError(HRESULT hr){  //stolen somewhere
  char *text="Undefined error";     
  switch (hr)  {    
  case D3D_OK:      text="D3D_OK";break;    
  case D3DOK_NOAUTOGEN:      text="D3DOK_NOAUTOGEN";break;    
  case D3DERR_CONFLICTINGRENDERSTATE:      text="D3DERR_CONFLICTINGRENDERSTATE";break;    
  case D3DERR_CONFLICTINGTEXTUREFILTER:      text="D3DERR_CONFLICTINGTEXTUREFILTER";break;    
  case D3DERR_CONFLICTINGTEXTUREPALETTE:      text="D3DERR_CONFLICTINGTEXTUREPALETTE";break;    
  case D3DERR_DEVICELOST:      text="D3DERR_DEVICELOST";break;    
  case D3DERR_DEVICENOTRESET:      text="D3DERR_DEVICENOTRESET";break;    
  case D3DERR_DRIVERINTERNALERROR:      text="D3DERR_DRIVERINTERNALERROR";break;    
  case D3DERR_INVALIDCALL:      text="D3DERR_INVALIDCALL";break;    
  case D3DERR_INVALIDDEVICE:      text="D3DERR_INVALIDDEVICE";break;    
  case D3DERR_MOREDATA:      text="D3DERR_MOREDATA";break;    
  case D3DERR_NOTAVAILABLE:      text="D3DERR_NOTAVAILABLE";break;    
  case D3DERR_NOTFOUND:      text="D3DERR_NOTFOUND";break;    
  case D3DERR_OUTOFVIDEOMEMORY:      text="D3DERR_OUTOFVIDEOMEMORY";break;    
  case D3DERR_TOOMANYOPERATIONS:      text="D3DERR_TOOMANYOPERATIONS";break;    
  case D3DERR_UNSUPPORTEDALPHAARG:      text="D3DERR_UNSUPPORTEDALPHAARG";break;    
  case D3DERR_UNSUPPORTEDALPHAOPERATION:      text="D3DERR_UNSUPPORTEDALPHAOPERATION";break;    
  case D3DERR_UNSUPPORTEDCOLORARG:      text="D3DERR_UNSUPPORTEDCOLORARG";break;    
  case D3DERR_UNSUPPORTEDCOLOROPERATION:      text="D3DERR_UNSUPPORTEDCOLOROPERATION";break;    
  case D3DERR_UNSUPPORTEDFACTORVALUE:      text="D3DERR_UNSUPPORTEDFACTORVALUE";break;    
  case D3DERR_UNSUPPORTEDTEXTUREFILTER:      text="D3DERR_UNSUPPORTEDTEXTUREFILTER";break;    
  case D3DERR_WRONGTEXTUREFORMAT:      text="D3DERR_WRONGTEXTUREFORMAT";break;    
  case E_FAIL:      text="E_FAIL";break;    
  case E_INVALIDARG:      text="E_INVALIDARG";break;    
  case E_OUTOFMEMORY:      text="E_OUTOFMEMORY";break;  
  }   
  return text;
}
#define REPORT_D3D_ERR(function,d3derr) if(d3derr) TRACE_LOG(function" %s\n",GetTextFromD3DError(d3derr))
#else
#define REPORT_D3D_ERR 
#endif

#endif//WIN32

#ifdef UNIX
#ifdef NO_SHM
bool TrySHM=false;
#else
bool TrySHM=true;
#endif
#endif

#define LOGSECTION LOGSECTION_VIDEO_RENDERING


TSteemDisplay::TSteemDisplay() {
#ifdef WIN32
#if defined(SSE_VID_DD)
  DDObj=NULL;
  DDPrimarySur=NULL;
  DDBackSur=NULL;
#if defined(SSE_VID_DD_3BUFFER_WIN)
  OurBackSur=DDBackSur2=NULL;
#endif
  DDClipper=NULL;
  DDBackSurIsAttached=0;
  DDExclusive=0;
#endif
  GDIBmp=NULL;
  GDIBmpMem=NULL;
#ifdef STEEM_CRT 
  CRTBmpMem = 0;
  CRTBmpLineLength = 0;
  CRThwnd = NULL;
  CRTpixels = NULL;
  CRTwidth = 0;
  CRTheight = 0;
  CRTexit = 0;
  CRTviewy = 0;
  CRTvieww = 0;
  CRTviewh = 0;
  CRTthread = NULL;
#endif
#if !defined(SSE_VID_NO_FREEIMAGE)
  hFreeImage=NULL;
  ScreenShotFormatOpts=0;
#endif
  ScreenShotExt="bmp";
  DrawToVidMem=true;
  BlitHideMouse=false;
  DrawLetterboxWithGDI=0;
#if defined(SSE_VID_D3D)
  pD3D       = NULL;	// Used to create the D3DDevice
  pD3DDevice = NULL;	// Our rendering device
  pD3DTexture=NULL;
  pD3DSprite=NULL;
  m_Adapter=D3DADAPTER_DEFAULT;
#endif
#endif
#ifdef UNIX
  X_Img=NULL;
  AlreadyWarnedOfBadMode=0;
  GoToFullscreenOnRun=0;
#ifndef NO_SHM
  XSHM_Attached=0;
  XSHM_Info.shmaddr=(char*)-1;
  XSHM_Info.shmid=-1;
  SHMCompletion=LASTEvent;
  asynchronous_blit_in_progress=false;
#endif
#ifndef NO_XVIDMODE
  XVM_Modes=NULL;
#endif
#endif
  ScreenShotFormat=0;
  ScreenShotUseFullName=0;ScreenShotAlwaysAddNum=0;
  ScreenShotMinSize=0;
  pNeoFile=NULL;
  RunOnChangeToWindow=0;
  DoAsyncBlit=0;
  Method=DISPMETHOD_NONE;
  UseMethod[0]=DISPMETHOD_NONE;
  nUseMethod=0;
  bpp_at_fullscreen=4;
}


TSteemDisplay::~TSteemDisplay() { 
  Release(); 
}


bool TSteemDisplay::BorderPossible() { 
  return (GetScreenWidth()>640); 
}


void TSteemDisplay::SetMethods(int Method1,...) {
  va_list vl;
  int arg=Method1;
  va_start(vl,Method1);
  for(int n=0;n<5;n++) 
  {
    UseMethod[n]=arg;
    if(arg==0) 
      break;
    arg=va_arg(vl,int);
  }
  va_end(vl);
  nUseMethod=0;
}


HRESULT TSteemDisplay::Init() {
  Release();
  if(FullScreen==0) 
  {
    monitor_width=(WORD)GetScreenWidth();
    monitor_height=(WORD)GetScreenHeight();
  }
#if !defined(SSE_VID_NO_FREEIMAGE)
  FreeImageLoad();
#endif
  while(nUseMethod<5) 
  {
#if defined(SSE_VID_D3D)
    if(UseMethod[nUseMethod]==DISPMETHOD_D3D) 
    {
      if(D3DInit()==D3D_OK)
      {
        D3D9_OK=true;
        return (Method=UseMethod[nUseMethod++]);
      }
    }
    else
#endif
#if defined(SSE_VID_DD)
    if(UseMethod[nUseMethod]==DISPMETHOD_DD) 
    {
      if(InitDD()==DD_OK) 
        return (Method=UseMethod[nUseMethod++]);
    }
    else 
#endif
#ifdef WIN32
    if(UseMethod[nUseMethod]==DISPMETHOD_GDI) 
    {
      TRACE2("GDI\n");
      if(InitGDI())
        return (Method=UseMethod[nUseMethod++]);
    }
#endif
#ifdef STEEM_CRT
    if(UseMethod[nUseMethod]==DISPMETHOD_CRT) 
    {
      TRACE2("CRT\n");
      if(InitCRT())
        return (Method=UseMethod[nUseMethod++]);
    }
#endif
#ifdef UNIX
    if(UseMethod[nUseMethod]==DISPMETHOD_X) {
      if(InitX()) return (Method=UseMethod[nUseMethod++]);
    }
    else if(UseMethod[nUseMethod]==DISPMETHOD_XSHM) {
      if(InitXSHM()) return (Method=UseMethod[nUseMethod++]);
    }
#endif
    if(UseMethod[nUseMethod]==0)
      break;
    nUseMethod++;
  }
  return 0;
}


// for each frame, sequence is lock-write frame (emulate)-unlock-blit


HRESULT TSteemDisplay::Lock() {// SS called by draw_begin()
  HRESULT derr=DDERR_GENERIC;
  //TRACE2("Lock frame %d\n",FRAME);
#if defined(SSE_EMU_THREAD)
  if(SuspendRendering || VideoLock.blocked)
    return derr;
#endif
  switch(Method) {
#if defined(SSE_VID_DD)
  case DISPMETHOD_DD:
  {
    if(!OurBackSur)
      return DDERR_SURFACELOST;
    derr=DDBackSur->IsLost();
    REPORT_DD_ERR("IsLost",derr);
    // Restore surfaces after event such as screen saver
    if(derr==DDERR_SURFACELOST) 
    {
      BYTE former_msg=StatusInfo.MessageIndex;
      derr=RestoreSurfaces();
      if(derr!=DD_OK) // wait next frame
      {
         REPORT_DD_ERR("RestoreSurfaces",derr);
         StatusInfo.MessageIndex=TStatusInfo::BLIT_ERROR;
         //runstate=RUNSTATE_STOPPED; 
      }
      else if(StatusInfo.MessageIndex==TStatusInfo::BLIT_ERROR)
        StatusInfo.MessageIndex=TStatusInfo::MESSAGE_NONE;
      if(former_msg!=StatusInfo.MessageIndex)
        REFRESH_STATUS_BAR;
    }
    DDBackSurDesc.dwSize=sizeof(DDBackSurDesc);
#if defined(SSE_VID_DD_3BUFFER_WIN)
    if(OPTION_3BUFFER_WIN && DDBackSur2)
    {
      SurfaceToggle=!SurfaceToggle; // toggle at lock
      OurBackSur=(SurfaceToggle)?DDBackSur2:DDBackSur;
    }
    else
      OurBackSur=DDBackSur;
    if((derr=OurBackSur->Lock(NULL,&DDBackSurDesc,DDLOCK_WAIT|DDLockFlags,NULL))!=DD_OK) 
    {
      REPORT_DD_ERR("Lock",derr);
      //TRACE2("Lock frame %d err %d\n",FRAME,DErr);
//      if(DErr!=DDERR_SURFACELOST && DErr!=DDERR_SURFACEBUSY) 
      if(StatusInfo.MessageIndex!=TStatusInfo::BLIT_ERROR)
      {
        StatusInfo.MessageIndex=TStatusInfo::BLIT_ERROR;
        REFRESH_STATUS_BAR;
      }
      return derr;
    }
#else
    if((derr=DDBackSur->Lock(NULL,&DDBackSurDesc,DDLOCK_WAIT|DDLockFlags,NULL))!=DD_OK) {
      if(derr!=DDERR_SURFACELOST && derr!=DDERR_SURFACEBUSY) {
        DDError(T("DirectDraw Lock Error"),derr);
        Init();
      }
      return derr;
    }
#endif
    draw_line_length=DDBackSurDesc.lPitch;
    draw_mem=LPBYTE(DDBackSurDesc.lpSurface);
    break;
  }//case
#endif
#if defined(SSE_VID_D3D)
  case DISPMETHOD_D3D:
    derr=D3DLock();
    break;
#endif
#ifdef WIN32
  case DISPMETHOD_GDI:
    draw_line_length=GDIBmpLineLength;
    draw_mem=GDIBmpMem;
    derr=DD_OK;;
    break;
#endif
#ifdef STEEM_CRT
  case DISPMETHOD_CRT:
    draw_line_length=CRTBmpLineLength;
    draw_mem=CRTBmpMem;
    derr=DD_OK;;
    break;
#endif
#ifdef UNIX
  case DISPMETHOD_X:
  case DISPMETHOD_XSHM:
    if(XD==NULL)
      break;
    WaitForAsyncBlitToFinish();
    draw_mem=LPBYTE(X_Img->data);
    draw_line_length=X_Img->bytes_per_line;
    derr=DD_OK;
    break;
#endif
  }
  // compute locked video memory as pitch * #lines
  VideoMemorySize=draw_line_length*SurfaceHeight;
  VideoMemoryEnd=draw_mem+VideoMemorySize;
  //TRACE("VideoMemorySize %d (%d KB)\n",VideoMemorySize,VideoMemorySize/1024);
  return derr;
}


void TSteemDisplay::Unlock() {
  //TRACE2("Unlock frame %d\n",FRAME);
  switch(Method) {
#if defined(SSE_VID_D3D)
  case DISPMETHOD_D3D:
    D3DUnlock();
    break;
#endif
#if defined(SSE_VID_DD)
  case DISPMETHOD_DD:
  {
#if defined(SSE_VID_DD_3BUFFER_WIN)
    OurBackSur=(OPTION_3BUFFER_WIN && DDBackSur2 && SurfaceToggle) 
      ? DDBackSur2 : DDBackSur;
    HRESULT DErr;
    DErr=OurBackSur->Unlock(NULL);
#else
    HRESULT DErr=DDBackSur->Unlock(NULL);
#endif
    REPORT_DD_ERR("Unlock",DErr);
    if(DErr==DDERR_SURFACELOST)
    {
      TRACE_LOG("Unlock Surface lost\n");
      DErr=RestoreSurfaces(); //v4
      if(DErr!=DD_OK) 
      {
        REPORT_DD_ERR("RestoreSurfaces",DErr);
        StatusInfo.MessageIndex=TStatusInfo::BLIT_ERROR;
        REFRESH_STATUS_BAR;
      }
    }
#if defined(SSE_VID_RECORD_AVI)
    if(video_recording && runstate==RUNSTATE_RUNNING)
    {
      if(!pAviFile)
      {
        if(!frameskip||frameskip==8) // error||auto
          frameskip=1;
        TRACE_LOG("Start AVI recording, codec %s, frameskip %d\n",SSE_VID_RECORD_AVI_CODEC,frameskip);
        pAviFile=new CAviFile(SSE_VID_RECORD_AVI_FILENAME,
          mmioFOURCC(video_recording_codec[0],video_recording_codec[1],
          video_recording_codec[2],video_recording_codec[3]),
          video_freq_at_start_of_vbl/frameskip);
      }
      HDC SurfDC;
      DDBackSur->GetDC(&SurfDC);
      ZeroMemory(&ddsd, sizeof(ddsd));
      ddsd.dwSize = sizeof(ddsd);
      DDBackSur->GetSurfaceDesc(&ddsd);
      HBITMAP OffscrBmp = CreateCompatibleBitmap(SurfDC,ddsd.dwWidth,ddsd.
        dwHeight);
      HDC OffscrDC = CreateCompatibleDC(SurfDC);
      HBITMAP OldBmp = (HBITMAP)SelectObject(OffscrDC, OffscrBmp);
      BitBlt(OffscrDC, 0, 0,ddsd.dwWidth,ddsd.dwHeight, SurfDC, 0, 0, SRCCOPY);
      if(pAviFile->AppendNewFrame(OffscrBmp))
      {
        delete pAviFile;
        video_recording=0;
      }
      DeleteDC(OffscrDC); // important, release Windows resources!
      DeleteObject(OldBmp);
      DeleteObject(OffscrBmp);
      DDBackSur->ReleaseDC(SurfDC);
    }
#endif
    break;
  }
#endif
#ifdef WIN32
  case DISPMETHOD_GDI:
    SetBitmapBits(GDIBmp,GDIBmpSize,GDIBmpMem);
    break;
#endif
#ifdef STEEM_CRT
  case DISPMETHOD_CRT:
    break;
#endif
#ifdef UNIX
  case DISPMETHOD_X:
  case DISPMETHOD_XSHM:
    break;
#endif
  }//sw
}


bool TSteemDisplay::Blit() {
  //TRACE2("Blit frame %d\n",FRAME);
  bool success=false;
#if defined(SSE_EMU_THREAD)
  if(SuspendRendering || VideoLock.blocked)
    return success;
#endif
  switch(Method) {
#if defined(SSE_VID_D3D)
  case DISPMETHOD_D3D:
    success=D3DBlit();
    break;
#endif
#if defined(SSE_VID_DD)
  case DISPMETHOD_DD:
  {
    HRESULT DErr=NULL;
    // if we're in BLIT ERROR condition, wait until Lock can recreate the
    // surfaces
    if(StatusInfo.MessageIndex==TStatusInfo::BLIT_ERROR)
      return false;
    if(FullScreen) 
    {
      if(runstate==RUNSTATE_RUNNING) 
      {
        switch(draw_fs_blit_mode) {
        case DFSM_FLIP:
          DErr=DDPrimarySur->Flip(NULL,0); //DDFLIP_WAIT);
          break;
        case DFSM_STRAIGHTBLIT:
          DErr=DDPrimarySur->BltFast(draw_blit_source_rect.left,draw_blit_source_rect.top,
            DDBackSur,&draw_blit_source_rect,DDBLTFAST_WAIT);
          break;
        case DFSM_STRETCHBLIT:
        case DFSM_FAKEFULLSCREEN:
        {
          RECT Dest;
          Dest=LetterBoxRectangle;
         // TRACE_RECT(Dest); TRACE_RECT(draw_blit_source_rect);
         // TRACE_VID_RECT(Dest); TRACE_VID_RECT(draw_blit_source_rect); TRACE_VID_R("\n");
#if defined(SSE_VID_DD_3BUFFER_WIN)
          if(OPTION_3BUFFER_WIN && DDBackSur2)
          {
            OurBackSur=(!SurfaceToggle)?DDBackSur2:DDBackSur;
            if(OurBackSur->GetBltStatus(DDGBS_CANBLT)==DD_OK)
              DErr=DDPrimarySur->Blt(&Dest,OurBackSur,&draw_blit_source_rect,
                DDBLT_WAIT,NULL);
          }
          else
#endif
            DErr=DDPrimarySur->Blt(&Dest,DDBackSur,&draw_blit_source_rect,DDBLT_WAIT,NULL);
          break;
        }//case
        }//sw
        if(DErr==DDERR_SURFACELOST) 
        {
          DErr=RestoreSurfaces();
          if(DErr!=DD_OK)
          { // can happen if idle for long
            REPORT_DD_ERR("RestoreSurfaces",DErr);
            StatusInfo.MessageIndex=TStatusInfo::BLIT_ERROR;
            runstate=RUNSTATE_STOPPED; // fullscreen, stop on BLIT ERROR
            REFRESH_STATUS_BAR;
          }
        }
        else if(DErr) 
        {
          REPORT_DD_ERR("Fullscreen blit error",DErr);
          StatusInfo.MessageIndex=TStatusInfo::BLIT_ERROR;
          runstate=RUNSTATE_STOPPED;
          REFRESH_STATUS_BAR;
        }
      }
      else
      { //not running right now
        HCURSOR OldCur=(BlitHideMouse) ? SetCursor(NULL) : NULL;
        RECT Dest;
        get_fullscreen_rect(&Dest);
        for(int i=0;i<2;i++) 
        {
#if defined(SSE_VID_DD_3BUFFER_WIN)
          OurBackSur= (OPTION_3BUFFER_WIN && !SurfaceToggle && DDBackSur2)
            ? DDBackSur2: DDBackSur;
          DErr=DDPrimarySur->Blt(&Dest,OurBackSur,&draw_blit_source_rect,
            DDBLT_WAIT,NULL);
#else
          DErr=DDPrimarySur->Blt(&Dest,DDBackSur,&draw_blit_source_rect,DDBLT_WAIT,NULL);
#endif
          REPORT_DD_ERR("Blit",DErr);
          if(DErr==DDERR_SURFACELOST) 
          {
            if(i==0) 
              DErr=RestoreSurfaces();
            if(DErr!=DD_OK) 
            {
              REPORT_DD_ERR("RestoreSurfaces",DErr);
              StatusInfo.MessageIndex=TStatusInfo::BLIT_ERROR;
              REFRESH_STATUS_BAR;
              break;
            }
          }
          else
            break;
        }
        if(BlitHideMouse) 
          SetCursor(OldCur);
      }
    }
    else // not Fullscreen:
    {  
      HCURSOR OldCur=(stem_mousemode==STEM_MOUSEMODE_DISABLED&&BlitHideMouse) 
        ? SetCursor(NULL) : NULL;
      RECT dest;GetClientRect(StemWin,&dest);
      dest.top+=MENUHEIGHT;dest.right-=4;dest.bottom-=4;
      POINT pt={2,2};
      ClientToScreen(StemWin,&pt);
      OffsetRect(&dest,pt.x,pt.y);
      for(int i=0;i<2;i++) 
      {
#if defined(SSE_VID_DD_3BUFFER_WIN)
        OurBackSur=(OPTION_3BUFFER_WIN && !SurfaceToggle && DDBackSur2) 
          ? DDBackSur2:DDBackSur;
        DErr=DDPrimarySur->Blt(&dest,OurBackSur,&draw_blit_source_rect,
          DDBLT_WAIT,NULL);
#else
        DErr=DDPrimarySur->Blt(&dest,DDBackSur,&draw_blit_source_rect,DDBLT_WAIT,NULL);
#endif
        REPORT_DD_ERR("Blit",DErr);
        if(DErr==DDERR_SURFACELOST)
        {
          if(i==0) 
            DErr=RestoreSurfaces();
        }
        else
          break;
      }
      //TRACE2("Blit frame %d ERR %d\n",FRAME,DErr);
      if(DErr!=DD_OK) 
      { // the surface couldn't be recreated or we get another error such as
        // "DirectDraw does not have enough memory to perform the operation"
        // when the screen saver triggers or for any other reason
        // we just enter BLIT ERROR condition but go on, Steem will restore
        // the surfaces as soon as possible
        REPORT_DD_ERR("Blit",DErr);
        StatusInfo.MessageIndex=TStatusInfo::BLIT_ERROR;
        REFRESH_STATUS_BAR;
      }
      if(stem_mousemode==STEM_MOUSEMODE_DISABLED && BlitHideMouse) 
        SetCursor(OldCur);
    }
    success=(DErr==DD_OK);
    break;
  }
#endif
#ifdef WIN32
  case DISPMETHOD_GDI:
  {
    RECT dest;
    GetClientRect(StemWin,&dest);
    HDC dc=GetDC(StemWin);
    SetStretchBltMode(dc,COLORONCOLOR);
    success=(StretchBlt(dc,2,MENUHEIGHT+2,dest.right-4,dest.bottom-(MENUHEIGHT+4),
      GDIBmpDC,draw_blit_source_rect.left,draw_blit_source_rect.top,
      draw_blit_source_rect.right-draw_blit_source_rect.left,
      draw_blit_source_rect.bottom-draw_blit_source_rect.top,SRCCOPY)!=0);
    ReleaseDC(StemWin,dc);
    break;
  }
#endif
#ifdef STEEM_CRT
  case DISPMETHOD_CRT:
  {
    if( !CRTpixels ) break;
    thread_mutex_lock( &CRTmutex );
    RECT dest;
    GetClientRect(StemWin,&dest);

    CRTviewy = ( FullScreen && runstate != RUNSTATE_RUNNING ) ? MENUHEIGHT / 2 : 0;
    CRTvieww = dest.right - dest.left;
    CRTviewh = dest.bottom - dest.top - ( ( FullScreen && runstate == RUNSTATE_RUNNING ) ? 0 : MENUHEIGHT );
    if( FullScreen && runstate == RUNSTATE_RUNNING ) {
        MoveWindow( CRThwnd, 0, 0, CRTvieww, CRTviewh, FALSE );
        SetWindowLong( CRThwnd, GWL_STYLE, GetWindowLong( CRThwnd, GWL_STYLE ) & (~ WS_CLIPSIBLINGS ) ) ;  	
    } else {
        MoveWindow( CRThwnd, 0, MENUHEIGHT, CRTvieww, CRTviewh, FALSE );
        SetWindowLong( CRThwnd, GWL_STYLE, GetWindowLong( CRThwnd, GWL_STYLE ) | WS_CLIPSIBLINGS ) ;  	
    }

    if( FullScreen ) {
        SetWindowLong( StemWin, GWL_EXSTYLE, GetWindowLong( StemWin, GWL_EXSTYLE ) | WS_EX_TOPMOST ) ;  	
        SetWindowPos( StemWin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE );
    } else {
        SetWindowLong( StemWin, GWL_EXSTYLE, GetWindowLong( StemWin, GWL_EXSTYLE ) & (~ WS_EX_TOPMOST ) ) ;  	
    }

	int blit_width = draw_blit_source_rect.right-draw_blit_source_rect.left;
	int blit_height =draw_blit_source_rect.bottom-draw_blit_source_rect.top;
	CRTEMU_U32* srcpixels = (CRTEMU_U32*) CRTBmpMem;

	if( blit_width / blit_height < 2 ) {
		for( int y = 0; y < blit_height; ++y ) {
			for( int x = 0; x < blit_width; ++x ) {
				CRTEMU_U32 c = srcpixels[ x + y * (CRTBmpLineLength / 4 ) ];
				CRTEMU_U32 r = c & 0xff;
				CRTEMU_U32 g = ( c >> 8 ) & 0xff;
				CRTEMU_U32 b = ( c >> 16 ) & 0xff;
				CRTpixels[ x + y * blit_width ] = ( r << 16 ) | ( g << 8 ) | b;
			}
		}
	} else {
		for( int y = 0; y < blit_height; ++y ) {
			for( int x = 0; x < blit_width; ++x ) {
				CRTEMU_U32 c = srcpixels[ x + y * (CRTBmpLineLength / 4 ) ];
				CRTEMU_U32 r = c & 0xff;
				CRTEMU_U32 g = ( c >> 8 ) & 0xff;
				CRTEMU_U32 b = ( c >> 16 ) & 0xff;
                c = ( r << 16 ) | ( g << 8 ) | b;
				CRTpixels[ x + ( 2*y + 0 )* blit_width ] = c;
				CRTpixels[ x + ( 2*y + 1 )* blit_width ] = c;
			}
		}
		blit_height *= 2;
	}

    CRTwidth = blit_width;
    CRTheight = blit_height;
    thread_mutex_unlock( &CRTmutex );
    break;
  }
#endif
#ifdef UNIX
  case DISPMETHOD_X:
  case DISPMETHOD_XSHM:
  {
    if(XD==NULL)
      break;
    int sx,sy,sw,sh,dx,dy;
    Window ToWin;
    if(FullScreen)
    {
      ToWin=XVM_FullWin;
      sx=draw_blit_source_rect.left;
      sy=draw_blit_source_rect.top;
      sw=draw_blit_source_rect.right;
      sh=draw_blit_source_rect.bottom;
      dx=MAX((XVM_FullW-sw)/2,0);
      dy=MAX((XVM_FullH-sh)/2,0);
      if(sh>XVM_FullH) 
        sh=XVM_FullH;
    }
    else
    {
      ToWin=StemWin;
      XWindowAttributes wa;
#ifdef DOC
      typedef struct {
        int x, y; /* location of window */
        int width, height; /* width and height of window */
        int border_width; /* border width of window */
        int depth; /* depth of window */
        Visual *visual; /* the associated visual structure */
        Window root; /*root of screen containing window */
        int class; /* InputOutput, InputOnly*/
        int bit_gravity; /*one of the bit gravity values */
        int win_gravity; /*one of the window gravity values */
        int backing_store; /* NotUseful, WhenMapped, Always */
        unsigned long backing_planes; /* planes to be preserved if possible */
        unsigned long backing_pixel; /*value to be used when restoring planes */
        Bool save_under; /*boolean, should bits under be saved? */
        Colormap colormap; /* color map to be associated with window */
        Bool map_installed; /* boolean, is color map currently installed*/
        int map_state; /* IsUnmapped, IsUnviewable, IsViewable */
        long all_event_masks; /*set of events all people have interest in*/
        long your_event_mask; /*my event mask */
        long do_not_propagate_mask; /* set of events that should not propagate */
        Bool override_redirect; /* boolean value for override-redirect */
        Screen *screen; /* back pointer to correct screen */
      } XWindowAttributes;
#endif
      XGetWindowAttributes(XD,StemWin,&wa);
      int w=wa.width-4,h=wa.height-(MENUHEIGHT+4);
      if(w<=0 || h<=0) 
        return true;
      dx=(w-draw_blit_source_rect.right)/2;
      dy=(h-draw_blit_source_rect.bottom)/2;
      sx=draw_blit_source_rect.left;
      if(SideBorderSizeWin==LARGE_BORDER_SIDE)
        dx=(w-(draw_blit_source_rect.right-4))/2; // 412
      sy=draw_blit_source_rect.top;
      sw=draw_blit_source_rect.right;
      if(SideBorderSizeWin==LARGE_BORDER_SIDE)
        sw-=4*2; // 412
      sh=draw_blit_source_rect.bottom;
      if(dx<0)
      {
        sx-=dx;
        sw=w;
        dx=0;
      }
      if(dy<0)
      {
        sy-=dy;
        sh=h;
        dy=0;
      }
      dy+=MENUHEIGHT+2;
      dx+=2;
    }
    bool DoneIt=0;
    //	printf("XPutImage(... ,%i,%i,%i,%i,%i,%i)\n",draw_blit_source_rect.left,draw_blit_source_rect.top,
    //              dx,dy,sw,sh);
#ifdef DOC
    XPutImage (display, d, gc, image, src_x, src_y, dest_x, dest_y, width, height)
    Display *display;
    Drawable d;
    GC gc;
    XImage *image;
    int src_x, src_y;
    int dest_x, dest_y;
    unsigned int width, height;
    display Specifies the connection to the X server.
    d Specifies the drawable.
    gc Specifies the GC.
    image Specifies the image you want combined with the rectangle.
    src_x Specifies the offset in X from the left edge of the image defined by the XImage
    structure.
    src_y Specifies the offset in Y from the top edge of the image defined by the XImage
    structure.
    dest_x
    dest_y Specify the x and y coordinates, which are relative to the origin of the drawable
    and are the coordinates of the subimage.
    width
    height Specify the width and height of the subimage, which define the dimensions of the
    rectangle.
#endif
    switch(Method) {
    case DISPMETHOD_X:
      XPutImage(XD,ToWin,DispGC,X_Img,sx,sy,
                dx,dy,sw,sh);
      DoneIt=true;
      break;
    case DISPMETHOD_XSHM:
#ifndef NO_SHM
      Disp.WaitForAsyncBlitToFinish();
      XShmPutImage(XD,ToWin,DispGC,X_Img,sx,sy,dx,dy,sw,sh,True);
      asynchronous_blit_in_progress=true;
      if(Disp.DoAsyncBlit==0) 
        Disp.WaitForAsyncBlitToFinish();
      DoneIt=true;
#endif
      break;
    }
#ifdef DRAW_ALL_ICONS_TO_SCREEN
    XSetForeground(XD,DispGC,BlackCol);
    for (int n=Ico16.NumIcons-1;n>=0;n--){
      Ico16.DrawIcon(n,ToWin,DispGC,n*16,30);
      XDrawString(XD,ToWin,DispGC,n*16,65,EasyStr(n),strlen(EasyStr(n)));
    }
    for (int n=Ico32.NumIcons-1;n>=0;n--){
      Ico32.DrawIcon(n,ToWin,DispGC,n*32,80);
      XDrawString(XD,ToWin,DispGC,n*32,135,EasyStr(n),strlen(EasyStr(n)));
    }
#endif
#ifdef DRAW_TIMER_TO_SCREEN
    EasyStr tt=timer;
    XDrawString(XD,ToWin,DispGC,10,40,tt,tt.Length());
#endif
    if(runstate!=RUNSTATE_RUNNING && pc==rom_addr)
    {
      // If all initialisation failed might be 0x0
      if(sw>=320 && sh>=200) osd_draw_reset_info(dx,dy,sw,sh);
    }
    success=DoneIt;
    break;
  }//case
#endif//ux
  }//sw
  return success;
}


void TSteemDisplay::WaitForAsyncBlitToFinish() {
#ifdef UNIX
#ifndef NO_SHM
  if(asynchronous_blit_in_progress==0) 
    return;
  XEvent ev;
  clock_t wait_till=clock()+(CLOCKS_PER_SEC/50);
//  TRACE("Frame %d WaitForAsyncBlit...",FRAME);
  for (int wait=50000;wait>=0;wait--){
    if (XCheckTypedEvent(XD,SHMCompletion,&ev)) break;
    if (clock()>wait_till) break;
  }
///!  TRACE("Done\n");
  asynchronous_blit_in_progress=false;
#endif
#endif
}


void TSteemDisplay::VSync() {
#ifdef STEEM_CRT
    thread_signal_wait( &CRTsignal, 1000 );
    return;
#endif
#ifdef WIN32
#if !defined(SSE_VID_VSYNC_WINDOW)
  if(FullScreen==0) 
    return;
#endif
#if defined(SSE_VID_D3D)
// with D3D vsync is automatic
#else
  if(!DDObj)
    return;
  LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: VSYNC - Starting wait for VBL at ")+(timeGetTime()-run_start_time));
  if(!FullScreen||OPTION_FAKE_FULLSCREEN) 
  { // better VSync for the window and fake fullscreen
    // we can't do real vsync, so we target a line in the middle of the screen,
    // knowing we'll be off by much at times
    // 60Hz ok, not perfect, + OSD glitches if Draw to Video, sthg to do with sound,
    // depends on computer? TODO
    DWORD middle=monitor_height/2;
    DWORD line;
    HRESULT DErr;
    do {
      DErr=DDObj->GetScanLine(&line);
    } while(line<middle&&!DErr);
  }
  else
  {
    BOOL Blanking;
    DDObj->GetVerticalBlankStatus(&Blanking);
    if(Blanking==FALSE) 
    {
      DWORD botline=GetScreenHeight(); // simpler and compatible with max res
      DWORD line;
      HRESULT DErr;
      do
      {
        DErr=DDObj->GetScanLine(&line);
        if(line>=botline)
          break;
        else
        {
          //        Sleep(0);
        }
      } while(DErr==DD_OK);
      if(DErr!=DD_OK && DErr!=DDERR_VERTICALBLANKINPROGRESS)
      {
        DDObj->GetVerticalBlankStatus(&Blanking);
        if(Blanking==FALSE)
          DDObj->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,NULL);
      }
    }
  }
  LOG_TO(LOGSECTION_SPEEDLIMIT,Str("SPEED: VSYNC - Finished waiting for VBL at ")+(timeGetTime()-run_start_time));
#endif
#endif//WIN32
}


void TSteemDisplay::RunStart(bool Temp) {
#ifdef WIN32
  if(FullScreen==0)
    return;
  if(Temp==0) 
  {
    bool ChangeSize=0;
    int w=640,h=400;
    if(extended_monitor&&((int)em_width<GetScreenWidth()
      ||(int)em_height<GetScreenHeight())) 
    {
      ChangeSize=true;
      w=em_width;
      h=em_height;
    }
#if defined(SSE_VID_DD)
    int hz=0;
    if(extended_monitor==0&&draw_fs_blit_mode!=DFSM_FAKEFULLSCREEN)
    {
      if(prefer_res_640_400 && border==0
        &&DDDisplayModePossible[2][int(BytesPerPixel==1?0:1)]) 
      {
        ChangeSize=true;
        hz=prefer_pc_hz[0];
      }
    }
#endif
    if(ChangeSize) 
    {
#if defined(SSE_VID_D3D)
      if(SetDisplayMode()==DD_OK)
        ;
#elif defined(SSE_VID_DD)
      int hz_ok=0;
      if(SetDisplayMode(w,h,BytesPerPixel*8,hz,&hz_ok)==DD_OK)
      {
        if(hz)
          tested_pc_hz[0]=MAKEWORD(hz,hz_ok);
        using_res_640_400=true; // also for extended monitor, was so in Steem 3.2 ?
      }
#endif
      else
        change_fullscreen_display_mode(0);
    }
  }
#if defined(SSE_VID_DD) && !defined(SSE_VID_DD_MISC)
  if(DDPrimarySur)
    DDPrimarySur->SetClipper(NULL);
#endif
  ShowAllDialogs(0);
#if defined(SSE_GUI_MENU)
  SetMenu(StemWin,NULL);
#endif
  SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
#ifdef WIN32
#if defined(SSE_VID_DD) && !defined(SSE_VID_DD_MISC)
/*  This could interfere with user rights somehow, and fullscreen
    wouldn't display anything if Steem isn't run as administrator.
    Very strange: build in VS2008 when SSE_VID_DD_MISC isn't defined,
    it doesn't work as simple user, rename the file, it does. Win 10 Pro.
*/
  if(DrawLetterboxWithGDI==0) 
    LockWindowUpdate(StemWin);
#endif
  while(ShowCursor(0)>=0);
  SetCursor(NULL);
#endif
#if defined(SSE_VID_DD)
  // delete screen
  Lock(); // as in DDCreateSurfaces()...
  if(draw_mem)
    ZeroMemory(draw_mem,VideoMemorySize);
  Unlock();
  //if(OPTION_FAKE_FULLSCREEN)
  {
#if defined(SSE_VID_DD_3BUFFER_WIN)

    OurBackSur=(OPTION_3BUFFER_WIN && !SurfaceToggle && DDBackSur2) 
      ? DDBackSur2:DDBackSur;
    DDPrimarySur->Blt(&rcMonitor,OurBackSur,&draw_blit_source_rect,
      DDBLT_WAIT,NULL);
#else
    DErr=DDPrimarySur->Blt(&rcMonitor,DDBackSur,&draw_blit_source_rect,DDBLT_WAIT,NULL);
#endif
  }
  //  Compute LetterBoxRectangle first
  if(draw_fs_blit_mode==DFSM_STRETCHBLIT||draw_fs_blit_mode==DFSM_FAKEFULLSCREEN)
  { // correct AR like in D3D build
    float stx=(float)STXPixels();
    float sty=(float)STYPixels();
    if(OPTION_ST_ASPECT_RATIO && screen_res<2)
      sty*=ST_ASPECT_RATIO_DISTORTION; // "reserve" more pixels
    //TRACE("%dx%d %fx%f\n",SurfaceWidth,SurfaceHeight,stx,sty);
    get_fullscreen_rect(&LetterBoxRectangle);
    //TRACE_RECT(LetterBoxRectangle);
    int horiz_pixels=LetterBoxRectangle.right-LetterBoxRectangle.left;
    int vert_pixels=LetterBoxRectangle.bottom-LetterBoxRectangle.top;
    if(!OPTION_FULLSCREEN_AR || !stx || !sty)
      ; // we dont' correct the AR in this mode, take full screen
    else
    {
      float multx=horiz_pixels/stx;
      float multy=vert_pixels/sty;
      if(OPTION_FULLSCREEN_AR==2) //crisp
      { // remove rest
        multx=(float)(int)multx;
        multy=(float)(int)multy;
      }
      float mult= (multx<multy) ? multx : multy;
      int sw=(int)(mult*stx);
      int sh=(int)(mult*sty);
      int diffw=(horiz_pixels-sw);
      int diffh=(vert_pixels-sh);
      LetterBoxRectangle.left+=diffw/2;
      LetterBoxRectangle.right-=diffw/2;
      LetterBoxRectangle.top+=diffh/2;
      LetterBoxRectangle.bottom-=diffh/2;
      TRACE2("Ratio (%d) %f Zone %dx%d ",OPTION_FULLSCREEN_AR,mult,
        LetterBoxRectangle.right-LetterBoxRectangle.left,
        LetterBoxRectangle.bottom-LetterBoxRectangle.top);
    }
    //TRACE("%f %f %f %d %d\n",stx,sty,st_ar,horiz_pixels,vert_pixels);
    TRACE_VID_R("RECT "); TRACE_VID_RECT(LetterBoxRectangle);
  }
  else // 640x400 and 800x600
  {
    if(border)
    {
      LetterBoxRectangle.top=(600-400-2*(BORDER_TOP+BORDER_BOTTOM))/2;
      LetterBoxRectangle.bottom=600-LetterBoxRectangle.top;
      int SideGap=(800 - (BORDER_SIDE+320+BORDER_SIDE)*2) / 2;
      LetterBoxRectangle.left=SideGap;
      LetterBoxRectangle.right=800-SideGap;
    }
    else
    {
      LetterBoxRectangle.top=draw_fs_topgap;
      LetterBoxRectangle.bottom=440;
      LetterBoxRectangle.right=640;
    }
  }
  TRACE2("Fullscreen %d %d %d %d\n",LetterBoxRectangle.left,LetterBoxRectangle.top,LetterBoxRectangle.right,LetterBoxRectangle.bottom);
  DrawFullScreenLetterbox();
#endif//nodd
#endif//WIN32
#ifdef UNIX
  if(FullScreenBut.checked) 
    ChangeToFullScreen();
#endif
}


void TSteemDisplay::RunEnd(
#if defined(SSE_VID_DD)
  bool Temp
#endif
  ) {
#ifdef WIN32
#if defined(SSE_VID_DD)
  if(using_res_640_400 && Temp==0&&bAppActive) 
  {
    // Save background
    RECT rcDest;
    get_fullscreen_rect(&rcDest);
    OffsetRect(&rcDest,-rcDest.left,-rcDest.top);
    int w=rcDest.right,h=rcDest.bottom;
    HRESULT DErr;
#if defined(SSE_VID_DD7)
    IDirectDrawSurface7 *SaveSur=NULL;
    DDSURFACEDESC2 SaveSurDesc;
#else
    IDirectDrawSurface *SaveSur=NULL;
    DDSURFACEDESC SaveSurDesc;
#endif
    ZeroMemory(&SaveSurDesc,sizeof(SaveSurDesc));
    SaveSurDesc.dwSize=sizeof(SaveSurDesc);
    SaveSurDesc.dwFlags=DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    SaveSurDesc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    SaveSurDesc.dwWidth=w;
    SaveSurDesc.dwHeight=h;
    DErr=DDObj->CreateSurface(&SaveSurDesc,&SaveSur,NULL);
    if(DErr==DD_OK)
    {
#if defined(SSE_VID_DD_3BUFFER_WIN)
      OurBackSur= (OPTION_3BUFFER_WIN && DDBackSur2 && !SurfaceToggle) 
        ? DDBackSur2: DDBackSur;
      DErr=SaveSur->Blt(&rcDest,OurBackSur,&draw_blit_source_rect,DDBLT_WAIT,NULL);
#else
      DErr=SaveSur->Blt(&rcDest,DDBackSur,&draw_blit_source_rect,DDBLT_WAIT,NULL);
#endif
    }
    if(DErr!=DD_OK) 
    {
      SaveSur->Release(); SaveSur=NULL;
    }
    using_res_640_400=0;
    change_fullscreen_display_mode(true);
    if(SaveSur)
    {
      DDBackSur->Blt(&draw_blit_source_rect,SaveSur,NULL,DDBLT_WAIT,NULL);
      SaveSur->Release();
    }
  }
  if(OPTION_FULLSCREEN_GUI) 
    DDObj->FlipToGDISurface();
#if !defined(SSE_VID_DD_MISC)
  LockWindowUpdate(NULL);
  if(DDPrimarySur)
    DDPrimarySur->SetClipper(DDClipper);
#endif
#endif
  while(ShowCursor(true)<0);
#if defined(SSE_GUI_MENU)
  SetMenu(StemWin,StemWin_AltMenu);
#endif
  if(!OPTION_FULLSCREEN_GUI)
  {
    ChangeToWindowedMode(); // this should work on all systems
    return;
  }
  ShowAllDialogs(true);
  InvalidateRect(StemWin,NULL,true);
#endif//WIN32
#ifdef UNIX
  if(FullScreen)
  {
    ChangeToWindowedMode();
    draw(true);
  }
#endif
}


void TSteemDisplay::ScreenChange() {
#if defined(SSE_EMU_THREAD)
  if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING)
    VideoLock.Lock();
#endif
  draw_end();
  TRACE_LOG("ScreenChange()\n");
  switch(Method) {
#if defined(SSE_VID_D3D)
  case DISPMETHOD_D3D:
    if(D3DCreateSurfaces()!=DD_OK) 
      Init();
    break;
#endif
#if defined(SSE_VID_DD)
  case DISPMETHOD_DD:
    if(DDCreateSurfaces()!=DD_OK) 
      Init();
    break;
#endif
#ifdef WIN32
  case DISPMETHOD_GDI:
    if(InitGDI()==0)
      Init();
    else
      Method=DISPMETHOD_GDI;
    break;
#endif
#ifdef STEEM_CRT
  case DISPMETHOD_CRT:
    if(InitCRT()==0)
      Init();
    else
      Method=DISPMETHOD_CRT;
    break;
#endif
#ifdef UNIX
  case DISPMETHOD_X:
    if(InitX())
      Method=DISPMETHOD_X;
    else
      Init();
    break;
  case DISPMETHOD_XSHM:
    if(InitXSHM())
      Method=DISPMETHOD_XSHM;
    else
      Init();
    break;
#endif
  }//sw
#if defined(SSE_EMU_THREAD)
  VideoLock.Unlock();
#endif
}


#if defined(SSE_VID_DD) || defined(UNIX)

void TSteemDisplay::DrawFullScreenLetterbox() {
  if(FullScreen==0||extended_monitor||using_res_640_400)
    return;
#ifdef SSE_VID_DD
  if(!DDBackSur||!DDPrimarySur)
    return;
#if defined(SSE_VID_2SCREENS)
  RECT rc;
  get_fullscreen_totalrect(&rc);
#endif
  if((draw_fs_topgap||border) && Method==DISPMETHOD_DD) 
  {
    DDBLTFX bfx;
    ZeroMemory(&bfx,sizeof(DDBLTFX));
    bfx.dwSize=sizeof(DDBLTFX);
    bfx.dwFillColor=RGB(0,0,0);
    HDC dc=NULL;
    if(DrawLetterboxWithGDI) 
      dc=GetDC(StemWin);
#if defined(SSE_VID_2SCREENS)
    RECT Dest={rc.left,rc.top,rc.right,LetterBoxRectangle.top};
#else
    RECT Dest={0,0,GetScreenWidth(),LetterBoxRectangle.top};
#endif
    //TRACE("letterbox ");TRACE_RECT(Dest);
    DDBackSur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
    if(dc) 
      FillRect(dc,&Dest,(HBRUSH)GetStockObject(BLACK_BRUSH));
    else
      DDPrimarySur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
    Dest.top=LetterBoxRectangle.bottom;
#if defined(SSE_VID_2SCREENS)
    Dest.bottom=rc.bottom;
#else
    Dest.bottom=SurfaceHeight;
#endif
    DDBackSur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
    if(dc)
      FillRect(dc,&Dest,(HBRUSH)GetStockObject(BLACK_BRUSH));
    else
      DDPrimarySur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
    if(border||(draw_fs_blit_mode==DFSM_STRETCHBLIT)
      ||(draw_fs_blit_mode==DFSM_FAKEFULLSCREEN))
    {
      Dest.right=LetterBoxRectangle.left;
#if defined(SSE_VID_2SCREENS)
      Dest.top=rc.top;
#else
      Dest.bottom=SurfaceHeight;
#endif
      DDBackSur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
      if(dc)
        FillRect(dc,&Dest,(HBRUSH)GetStockObject(BLACK_BRUSH));
      else
        DDPrimarySur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
      Dest.left=LetterBoxRectangle.right;
#if defined(SSE_VID_2SCREENS)
      Dest.right=rc.right;
#else
      Dest.right=SurfaceWidth;
#endif
      DDBackSur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
      if(dc)
        FillRect(dc,&Dest,(HBRUSH)GetStockObject(BLACK_BRUSH));
      else
        DDPrimarySur->Blt(&Dest,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bfx);
    }
    if(dc) 
      ReleaseDC(StemWin,dc);
  }
#endif//dd
#ifdef UNIX
  XSetForeground(XD,DispGC,BlackCol);
  int w_gap=XVM_FullW-draw_blit_source_rect.right;
  int h_gap=XVM_FullH-draw_blit_source_rect.bottom;
  if(w_gap)
  {
    XFillRectangle(XD,XVM_FullWin,DispGC,0,0,w_gap/2,XVM_FullH);
    XFillRectangle(XD,XVM_FullWin,DispGC,w_gap/2+draw_blit_source_rect.right,0,
      w_gap/2+1,600);
  }
  if(h_gap)
  {
    XFillRectangle(XD,XVM_FullWin,DispGC,0,0,XVM_FullW,h_gap/2);
    XFillRectangle(XD,XVM_FullWin,DispGC,0,h_gap/2+draw_blit_source_rect.bottom,
      XVM_FullW,h_gap/2+1);
  }
#endif
}


HRESULT TSteemDisplay::RestoreSurfaces() {
  HRESULT DErr;
  switch(Method) {
#ifdef UNIX
  case DISPMETHOD_X:
  case DISPMETHOD_XSHM:
    ScreenChange();
    DErr=DD_OK;
    break;
#endif
#if defined(SSE_VID_DD)
  case DISPMETHOD_DD:
    DErr=DDERR_GENERIC;
    if(DDPrimarySur && DDBackSur) 
    {
      draw_end();
      DErr=DDPrimarySur->Restore();
      if(DErr==DD_OK) 
      {
        DErr=DDBackSur->Restore();
#if defined(SSE_VID_DD_3BUFFER_WIN)
        if(OPTION_3BUFFER_WIN && DErr==DD_OK && DDBackSur2) 
          DErr=DDBackSur2->Restore();
        SurfaceToggle=true;
        VSyncTiming=0;
#endif
        TRACE_LOG("Restore surfaces %d\n",DErr);
      }
    }
    break;
#endif
  default:
    DErr=DD_OK;
  }//sw
  return DErr;
}

#endif//#if defined(SSE_VID_DD) || defined(UNIX)


bool TSteemDisplay::CanGoToFullScreen() {
  bool YesWeCan;
  switch(Method) {
#if defined(SSE_VID_DD) // only normal borders for real DD fullscreen
  case DISPMETHOD_DD:
    YesWeCan=(!(border>1 && draw_fs_blit_mode!=DFSM_STRETCHBLIT 
      && draw_fs_blit_mode!=DFSM_FAKEFULLSCREEN));
    break;
#endif
#if defined(SSE_VID_D3D)
  case DISPMETHOD_D3D:
    YesWeCan=true; // ?
    break;
#endif
#if defined(STEEM_CRT)
  case DISPMETHOD_CRT:
    YesWeCan=true;
    break;
#endif
#if defined(UNIX) && !defined(NO_XVIDMODE)
  {
    int evbase,errbase;
    YesWeCan=XF86VidModeQueryExtension(XD,&evbase,&errbase);
    break;
  }
#endif
  default:
    YesWeCan=false;
  }//sw
  TRACE_LOG("Can go fullscreen 1:%d, Method #%d border %d\n",YesWeCan,Method,border);
  return YesWeCan;
}


/* Disp.ChangeToFullScreen()
    |
    L-> change_fullscreen_display_mode()
          |
          L-> Disp.SetDisplayMode()
*/

void TSteemDisplay::ChangeToFullScreen() {
  if(CanGoToFullScreen()==0||FullScreen
#if defined(SSE_VID_DD)
    || DDExclusive
#endif
    ) return;
  TRACE_LOG("Going fullscreen...\n");
  draw_end();
#ifdef WIN32
  if(runstate==RUNSTATE_RUNNING) 
  {
    Glue.m_Status.stop_emu=1;
    PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,2);
  }
  else if(runstate!=RUNSTATE_STOPPED)
  { //Keep trying until succeed!
    PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,2);
  }
  else if(bAppMinimized)
  {
    ShowWindow(StemWin,SW_RESTORE);
    PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,2);
  }
  else
  {
    bool MaximizeDiskMan=0;
    if(OPTION_FULLSCREEN_GUI && DiskMan.IsVisible())
    {
      if(IsIconic(DiskMan.Handle)) 
        ShowWindow(DiskMan.Handle,SW_RESTORE);
      MaximizeDiskMan=DiskMan.FSMaximized;
      SetWindowLong(DiskMan.Handle,GWL_STYLE,
        (GetWindowLong(DiskMan.Handle,GWL_STYLE)&~WS_MAXIMIZE)&~WS_MINIMIZEBOX);
    }
    FullScreen=true;
    DirectoryTree::PopupParent=StemWin;
    GetWindowRect(StemWin,&rcPreFS); //SS "before fullscreen"
    if(OPTION_FULLSCREEN_GUI)
    {
#if !defined(SSE_GUI_MENU)
      ShowWindow(GetDlgItem(StemWin,106),SW_SHOWNA); // icon "back to windowed"
      ShowWindow(GetDlgItem(StemWin,116),SW_SHOWNA); //Quit Steem
#endif
      if(OptionBox.IsVisible())
      {
        OptionBox.DestroyCurrentPage();
        OptionBox.CreatePage(OptionBox.Page);
      }
    }
    SetWindowLong(StemWin,GWL_STYLE,WS_VISIBLE);
#if defined(SSE_VID_D3D) 
#if !defined(SSE_VID_2SCREENS) // done in D3DCreateSurfaces()
    SetWindowPos(StemWin,HWND_TOPMOST,0,0,D3DFsW,D3DFsH,0);
#endif
#else // DD
    int w=640,h=480;
    if(border)
      w=800,h=600;
    if(extended_monitor) 
      w=em_width,h=em_height;
    if(draw_fs_blit_mode==DFSM_STRETCHBLIT) 
    {
      w=fs_res[fs_res_choice].x;
      h=fs_res[fs_res_choice].y;
    }
    else if(draw_fs_blit_mode==DFSM_FAKEFULLSCREEN) 
    {
      //w=monitor_width;
      //h=monitor_height;
      w=rcMonitor.right-rcMonitor.left;
      h=rcMonitor.bottom-rcMonitor.top;
    }
#if defined(SSE_VID_2SCREENS)
    RECT rc;
    get_fullscreen_totalrect(&rc);
    // Compute size
    int cw=rc.right-rc.left;
    int ch=rc.bottom-rc.top;
    TRACE_VID_R("SetWindowPos 1 %d %d %d %d\n",rc.left,rc.top,cw,ch);
    SetWindowPos(StemWin,0,rc.left,rc.top,cw,ch,0);
#else
    SetWindowPos(StemWin,HWND_TOPMOST,0,0,w,h,0);
#endif
#endif//#if defined(SSE_VID_D3D)
    CheckResetDisplay(true);
#if defined(SSE_VID_DD) && !defined(SSE_VID_DD_MISC)
    ClipWin=CreateWindow("Steem Fullscreen Clip Window","",WS_CHILD | WS_VISIBLE |
                          WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                          0,MENUHEIGHT,w,h-MENUHEIGHT,StemWin,(HMENU)1111,Inst,NULL);
    DDClipper->SetHWnd(0,ClipWin);
#endif
    bool ShowInfoBox=InfoBox.IsVisible();
    for(int n=0;n<nStemDialogs;n++) 
    {
      if(DialogList[n]!=&InfoBox) 
      {
        DEBUG_ONLY(if(DialogList[n]!=&HistList)) DialogList[n]->MakeParent(StemWin);
        if(OPTION_FULLSCREEN_GUI && DialogList[n]->IsVisible())
          InvalidateRect(DialogList[n]->Handle,NULL,FALSE);
      }
    }
    InfoBox.Hide();
    SetParent(ToolTip,StemWin);
#if defined(SSE_VID_DD)
    if(!OPTION_FAKE_FULLSCREEN)
    {
      HRESULT DErr=DDObj->SetCooperativeLevel(StemWin,DDSCL_EXCLUSIVE
        |DDSCL_FULLSCREEN|DDSCL_ALLOWREBOOT);
      if(DErr!=DD_OK) 
      {
        REPORT_DD_ERR("SetCooperativeLevel",DErr);
        DDError(T("Can't SetCooperativeLevel to exclusive"),DErr);
        Init();
        return;
      }
      DDExclusive=true;
    }
#endif
    if(change_fullscreen_display_mode(0)==DD_OK) 
    {
#if defined(SSE_VID_DD)
      if(OPTION_FAKE_FULLSCREEN)
      {        
        if(DDCreateSurfaces()!=DD_OK)
        {
          ChangeToWindowedMode(true);
          return;
        }
      }
#endif
      if(OPTION_FULLSCREEN_GUI) // don't show dialogs if we run at once...
      {
        if(ShowInfoBox) 
          InfoBox.Show();
        if(MaximizeDiskMan) 
        {
          SendMessage(DiskMan.Handle,WM_SETREDRAW,0,0);
          ShowWindow(DiskMan.Handle,SW_MAXIMIZE);
          PostMessage(DiskMan.Handle,WM_SETREDRAW,true,0);
        }
        OptionBox.EnableBorderOptions(true);
      }
      SetForegroundWindow(StemWin);
      SetFocus(StemWin);
      palette_convert_all();
      ONEGAME_ONLY( DestroyNotifyInitWin(); )
      if(OPTION_FULLSCREEN_GUI) 
      {
        InvalidateRect(StemWin,NULL,FALSE);
        if(OptionBox.IsVisible())
          InvalidateRect(OptionBox.Handle,NULL,FALSE);
        if(DiskMan.IsVisible())
          InvalidateRect(DiskMan.Handle,NULL,FALSE);
        #ifdef STEEM_CRT
            CLICK_PLAY_BUTTON();
        #endif
      }
      else
      {
        CLICK_PLAY_BUTTON();
      }
    }
    else
    { //back to windowed mode
      TRACE_LOG("Can't go fullscreen 2\n");
      ChangeToWindowedMode(true);
    }
  }
#endif//WIN32
#if defined(UNIX) && !defined(NO_XVIDMODE)
  int Screen=XDefaultScreen(XD);
  //  int XVM_nModes,XVM_ViewX,XVM_ViewY; SS declared
  //  XF86VidModeModeInfo **XVM_Modes;   in display.h
  // memory for XVM_Modes requested by X, it's our task to free it
  if(XF86VidModeGetAllModeLines(XD,Screen,&XVM_nModes,&XVM_Modes)==0)
    return;
  TRACE("XVM_nModes=%d",XVM_nModes);
  int w=640,h=480;
  if(border)
    w=800,h=600;
  else if(prefer_res_640_400)
    w=640,h=400;
#ifndef NO_CRAZY_MONITOR
  if(extended_monitor)
    w=em_width,h=em_height;
#endif
  TRACE("Fullscreen try w=%d,h=%d",w,h);
  XF86VidModeModeInfo *Mode=NULL;
  int diff=0xffff;
  for(int a=0;a<2;a++)
  {
    if(prefer_res_640_400 && border==0) 
      h=400;
    for(int n=0;n<2;n++)
    {
      for(int i=0;i<XVM_nModes;i++)
      {
        if(a==0)
        {
          // get exact
          if(XVM_Modes[i]->hdisplay==w && XVM_Modes[i]->vdisplay==h)
          {
            TRACE("Exact match mode %d w %d h%d",i,w,h);
            Mode=XVM_Modes[i];
            break;
          }
        }
        else
        {
          // get closest
          if(XVM_Modes[i]->hdisplay>=w && XVM_Modes[i]->vdisplay>=h)
          {
            int new_diff=(XVM_Modes[i]->hdisplay-w)+(XVM_Modes[i]->vdisplay-h);
            if(new_diff<diff)
            {
              TRACE("Close match mode %d w %d h%d",i,XVM_Modes[i]->hdisplay,XVM_Modes[i]->vdisplay);
              Mode=XVM_Modes[i];
              diff=new_diff;
            }
          }
        }//if
      }//nxt i
      if(Mode || h!=400)
        break;
      h=480;
    }
    if(Mode)
      break;
  }//nxt a
  if(Mode==NULL)
  {
    Alert(T("Can't change to fullscreen. Your video card doesn't support the required screen mode")+
            " ("+w+"x"+h+")",T("Error"),MB_ICONERROR);
    XFree(XVM_Modes);
    return;
  }
  // SS: only the specified options are set, by using both mask & struct
  // XVM_FullWin is a Window declared in display.h
  w=Mode->hdisplay, h=Mode->vdisplay; // why were
  int x=(GetScreenWidth()-w)/2, y=(GetScreenHeight()-h)/2; // those 2 lines missing?
  XSetWindowAttributes swa; 
  swa.backing_store=NotUseful;
  swa.override_redirect=True;
#if defined(SSE_UNIX)
  XVM_FullWin=XCreateWindow(XD,XDefaultRootWindow(XD),0,0,w,h,Screen,
#else
  XVM_FullWin=XCreateWindow(XD,XDefaultRootWindow(XD),x,y,w,h,Screen,			    
#endif
			    CopyFromParent,InputOutput,CopyFromParent,
          CWBackingStore | CWOverrideRedirect,&swa);
  SetProp(XD,XVM_FullWin,cWinProc,(DWORD)XVM_WinProc);
  SetProp(XD,XVM_FullWin,cWinThis,(DWORD)this);
  SetProp(XD,XVM_FullWin,hxc::cModal,(DWORD)0xffffffff);
  XSelectInput(XD,XVM_FullWin,KeyPressMask | KeyReleaseMask |
                            ButtonPressMask | ButtonReleaseMask |
                            ExposureMask | FocusChangeMask);
  XMapWindow(XD,XVM_FullWin);
  XF86VidModeGetViewPort(XD,Screen,&XVM_ViewX,&XVM_ViewY);
#if defined(SSE_UNIX)
  TRACE("using XWarpPointer");
//  XWarpPointer(XD, None, XDefaultRootWindow(XD),0, 0, 0, 0, 0, 0);
  XWarpPointer(XD, None, XVM_FullWin,0, 0, 0, 0, 0, 0);
//  XWarpPointer(XD,Screen,XVM_FullWin,0,0,0,0,x,y); //
//  XWarpPointer(XD,None,XVM_FullWin,0,0,0,0,window_mouse_centre_x,window_mouse_centre_y);
//  XWarpPointer(XD,None,XDefaultRootWindow(XD),0,0,0,0,window_mouse_centre_x,window_mouse_centre_y);
#endif
  FullScreen=XF86VidModeSwitchToMode(XD,Screen,Mode);
  if(FullScreen)
  {
    // SS location of the upper left corner of the viewport into the virtual screen
    XF86VidModeSetViewPort(XD,Screen,x,y);
    //XF86VidModeSetViewPort(XD,Screen,0,0);
#if defined(SSE_BUILD)
    draw_grille_black=MAX((int)draw_grille_black,50);
#else
    draw_grille_black=MAX(draw_grille_black,50);
#endif
    XGrabPointer(XD,XVM_FullWin,False,ButtonPressMask | ButtonReleaseMask,
                  GrabModeAsync,GrabModeAsync,XVM_FullWin,EmptyCursor,CurrentTime);
    window_mouse_centre_x=w/2;
    window_mouse_centre_y=h/2;
    XWarpPointer(XD,None,XVM_FullWin,0,0,0,0,window_mouse_centre_x,
      window_mouse_centre_y);
    mouse_move_since_last_interrupt_x=0;
    mouse_move_since_last_interrupt_y=0;
    mouse_change_since_last_interrupt=false;
    XVM_FullW=w;
    XVM_FullH=h;
    if(XVM_FullH<480) 
      using_res_640_400=true;
  }
  else
  {
    XDestroyWindow(XD,XVM_FullWin);
    Alert(T("Can't change to fullscreen. There was an error switching to the required screen mode")+
            " ("+w+"x"+h+")",T("Error"),MB_ICONERROR);
    XFree(XVM_Modes);
  }
#endif
}


HRESULT TSteemDisplay::SetDisplayMode(
#if defined(SSE_VID_DD) || defined(UNIX)
  int w,int h,int bpp,int hz,int *hz_ok
#endif
  ) {
  //TRACE_LOG("SetDisplayMode\n");
  HRESULT DErr=DDERR_GENERIC;
  switch(Method) {
#if defined(SSE_VID_D3D)
  case DISPMETHOD_D3D:
    DErr=D3DCreateSurfaces();
    break;
#endif
#if defined(SSE_VID_DD)
  case DISPMETHOD_DD:
    if(DDExclusive && DDObj) 
    {
      int idx=-1;
      if(w==640&&h==480) 
        idx=0;
      if(w==800&&h==600) 
        idx=1;
      if(w==640&&h==400) 
        idx=2;
      if(draw_fs_blit_mode==DFSM_STRETCHBLIT) 
        idx=3;
      if(idx>=0) 
      {
        for(int n=1;n<NUM_HZ;n++)
        {
          if(hz==HzIdxToHz[n]) 
          {
            hz=DDClosestHz[idx][int(bpp>8)][n];
            break;
          }
        }
      }
      TRACE_LOG("SetDisplayMode %dx%d %dbit %dHz\n",w,h,bpp,hz);
      log_write(Str("PC DISPLAY: Changing mode to ")+w+"x"+h+"x"+bpp+" "+hz+"Hz");
      DErr=DDObj->SetDisplayMode(w,h,bpp,hz,0);
      if(DErr!=DD_OK)
      {
        log_write("  It failed");
        if(hz_ok)
          *hz_ok=0;
        DErr=DDObj->SetDisplayMode(w,h,bpp,0,0);
      }
      else
      {
        //TRACE2("FULLSCREEN %dx%d");
        log_write("  Success");
        if(hz_ok) 
          *hz_ok=(hz<<16)+1;
      }
      if(DErr!=DD_OK) 
      {
        //      DDError(T("Can't SetDisplayMode"),DErr);
        //      Init();
        REPORT_DD_ERR("SetDisplayMode",DErr);
      }
      if((DErr=DDCreateSurfaces())!=DD_OK) 
        Init();
    }//if
    break;
#endif
#if STEEM_CRT
  case DISPMETHOD_CRT:
    HMONITOR hmon = MonitorFromWindow( StemWin, MONITOR_DEFAULTTOPRIMARY );
    MONITORINFO info = { sizeof( MONITORINFO ) };
    GetMonitorInfo( hmon, &info );
    MoveWindow( StemWin, info.rcMonitor.left, 
        info.rcMonitor.top, info.rcMonitor.right - info.rcMonitor.left,
        info.rcMonitor.bottom - info.rcMonitor.top, TRUE );
    return DD_OK;
    break;
#endif
#ifdef UNIX
  case DISPMETHOD_X:
  case DISPMETHOD_XSHM:
    break;
#endif
  }//sw
  return DErr;
}


void TSteemDisplay::FlipToDialogsScreen() { //SS called by Alert()
#if defined(SSE_VID_DD)
  if(Method==DISPMETHOD_DD && DDObj)
    DDObj->FlipToGDISurface();
#endif
}


void TSteemDisplay::ChangeToWindowedMode(bool Emergency) {
  if(
#if defined(SSE_VID_DD)
    DDExclusive==0 && 
#endif
    FullScreen==0) 
    return;
  TRACE_LOG("Going windowed mode...\n");
#ifdef WIN32
  if(FullScreen) 
    TScreenSaver::killTimer();
  bool CanChangeNow=true;
  if(runstate==RUNSTATE_RUNNING) 
  {
    Glue.m_Status.stop_emu=1;
    PostMessage(StemWin,WM_COMMAND,MAKEWPARAM(106,BN_CLICKED),(LPARAM)GetDlgItem(StemWin,106));
    CanChangeNow=false;
  }
  else if(runstate!=RUNSTATE_STOPPED) 
  { //Keep trying until succeed!
    PostMessage(StemWin,WM_COMMAND,MAKEWPARAM(106,BN_CLICKED),(LPARAM)GetDlgItem(StemWin,106));
    CanChangeNow=false;
  }
  if(CanChangeNow||Emergency) 
  {
#if defined(SSE_VID_DD)
    if(DDExclusive && DDObj)
    {
#endif
      draw_end();
#if defined(SSE_VID_D3D)
      D3DDestroySurfaces();
#else
      DDDestroySurfaces();
      DDObj->RestoreDisplayMode();
      DDObj->SetCooperativeLevel(StemWin,DDSCL_NORMAL);
#endif
#if defined(SSE_VID_DD)
      DDExclusive=0;
    }
#endif
    FullScreen=0;
#if defined( STEEM_CRT )
#elif defined(SSE_VID_D3D)
    if(D3DCreateSurfaces()!=DD_OK)
      Init();
#else
    if(DDCreateSurfaces()!=DD_OK)
      Init();
#if !defined(SSE_VID_DD_MISC)
    else
      DDClipper->SetHWnd(0,StemWin);
#endif
#endif
    CheckResetDisplay(true); // Hide fullscreen reset display
#if defined(SSE_VID_D3D) || defined(SSE_VID_DD_MISC)
    ToolsDeleteAllChildren(ToolTip,StemWin);
#else
    ToolsDeleteAllChildren(ToolTip,ClipWin);
    DestroyWindow(ClipWin);
#endif
    DirectoryTree::PopupParent=NULL;
#if defined(SSE_VID_DD) && !defined(SSE_VID_DD_MISC)
    LockWindowUpdate(NULL);
#endif
    // Sometimes things won't work if you do them immediately after switching to
    // windowed mode, so post a message and resize all the windows back when we can
    PostMessage(StemWin,WM_USER,12,0);
    ChangeToWinTimeOut=timeGetTime()+2000;
    //InvalidateRect(StemWin,NULL,true);
    if(OptionBox.IsVisible())
    {
      OptionBox.DestroyCurrentPage();
      OptionBox.CreatePage(OptionBox.Page);
    }
  }
#endif//WIN32

#ifdef UNIX
#if !defined(NO_XVIDMODE)
  int Screen=XDefaultScreen(XD);
  XF86VidModeSwitchToMode(XD,Screen,XVM_Modes[0]);
  XF86VidModeSetViewPort(XD,Screen,XVM_ViewX,XVM_ViewY);
  XFree(XVM_Modes);
  XDestroyWindow(XD,XVM_FullWin);
  FullScreen=0;
  using_res_640_400=0;
#endif//!NO_XVIDMODE
#endif
}


void TSteemDisplay::Release() {
  log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Display::Release - calling draw_end()");
  draw_end();
#ifdef WIN32
  if(GDIBmp!=NULL) 
  {
    log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Freeing GDI stuff");
    DeleteDC(GDIBmpDC);   GDIBmpDC=NULL;
    DeleteObject(GDIBmp); GDIBmp=NULL;
    delete[] GDIBmpMem;
  }
#endif
#ifdef STEEM_CRT
    if( CRTthread != NULL ) {
        CRTexit = 1;
        thread_join( CRTthread );
        thread_destroy( CRTthread );

        thread_mutex_term( &CRTmutex );
        thread_signal_term( &CRTsignal );

        CloseWindow( CRThwnd );
    }
    if( CRTBmpMem ) {
        delete[] CRTBmpMem;
    }
    CRTBmpMem = NULL;
    CRTBmpLineLength = 0;
    CRThwnd = NULL;
    CRTpixels = NULL;
    CRTwidth = 0;
    CRTheight = 0;
    CRTexit = 0;
    CRTviewy = 0;
    CRTvieww = 0;
    CRTviewh = 0;
    CRTthread = NULL;
#endif
#if defined(SSE_VID_DD)
  if(DDObj!=NULL) 
  {
    if(DDExclusive||FullScreen) 
    {
      log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Calling ChangeToWindowedMode()");
      ChangeToWindowedMode(true);
    }
    log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Destroying surfaces");
    DDDestroySurfaces();
    if(DDClipper!=NULL) 
    {
      log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Destroying clipper");
      DDClipper->Release();
      DDClipper=NULL;
    }
    log_to_section(LOGSECTION_SHUTDOWN,"SHUTDOWN: Destroying DD object");
    DDObj->Release();
    DDObj=NULL;
  }
#endif
#if defined(SSE_VID_D3D)
  D3DRelease();
#endif
#ifdef UNIX
#ifndef NO_SHM
  if(XSHM_Attached && XD)
  {
    XSync(XD,False);
    if(XD)
      XShmDetach(XD,&XSHM_Info);XSHM_Attached=0;
    if(XD)
      XSync(XD,False);
  }
#endif
  if(X_Img)
  {
    XDestroyImage(X_Img);
    X_Img=NULL;
  }
#ifndef NO_SHM
  if(XSHM_Info.shmaddr!=(char*)-1)
  {
    shmdt(XSHM_Info.shmaddr);
    XSHM_Info.shmaddr=(char*)-1;
  }
  if(XSHM_Info.shmid!=-1)
  {
    shmctl(XSHM_Info.shmid,IPC_RMID,0);
    XSHM_Info.shmid=-1;
  }
#endif
#endif//ux
  palette_remove();
  Method=DISPMETHOD_NONE;
}


#ifdef SHOW_WAVEFORM

void TSteemDisplay::DrawWaveform()
{
#ifdef WIN32
  HDC dc;
  if (Method==DISPMETHOD_DD){
    if (DDBackSur->GetDC(&dc)!=DD_OK) return;
  }else if (Method==DISPMETHOD_GDI){
    dc=GDIBmpDC;
  #ifdef STEEM_CRT
  }else if (Method==DISPMETHOD_GDI){
    dc=CRTBmpDC;
  #endif
  }else{
    return;
  }
  int base=shifter_y-10;
  SelectObject(dc,GetStockObject((STpal[0]<0x777) ? WHITE_PEN:BLACK_PEN));
  MoveToEx(dc,0,base-129,0);
  LineTo(dc,shifter_x,base-129);
  MoveToEx(dc,0,base+1,0);
  LineTo(dc,shifter_x,base+1);
  MoveToEx(dc,0,base - temp_waveform_display[0]/2,0);
  for (int x=0;x<draw_blit_source_rect.right;x++){
    LineTo(dc,x,base - temp_waveform_display[x*SHOW_WAVEFORM]/2);
  }
  MoveToEx(dc,temp_waveform_play_counter/SHOW_WAVEFORM,0,0);
  LineTo(dc,temp_waveform_play_counter/SHOW_WAVEFORM,shifter_y);
  if (Method==DISPMETHOD_DD) DDBackSur->ReleaseDC(dc);
#endif
}
#endif



#ifdef WIN32

#if defined(SSE_VID_DD)

HRESULT TSteemDisplay::InitDD() {
  SetNotifyInitText("DirectDraw");
  HRESULT DErr;
  try{
    IDirectDraw *DDObj1=NULL;
    DBG_LOG("STARTUP: Initialising DirectDraw, creating DirectDraw object");
    DErr=CoCreateInstance(CLSID_DirectDraw,NULL,CLSCTX_ALL,IID_IDirectDraw,(void**)&DDObj1);
    if(DErr!=S_OK||DDObj1==NULL) 
    {
      EasyStr Err="Unknown error";
      switch(DErr) {
      case REGDB_E_CLASSNOTREG:
        Err="The specified class is not registered in the registration database.";
        break;
      case E_OUTOFMEMORY:
        Err="Out of memory.";
        break;
      case E_INVALIDARG:
        Err="One or more arguments are invalid.";
        break;
      case E_UNEXPECTED:
        Err="An unexpected error occurred.";
        break;
      case CLASS_E_NOAGGREGATION:
        Err="This class cannot be created as part of an aggregate.";
        break;
      }
      Err=EasyStr("CoCreateInstance error\n\n")+Err;
      log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      log_write(Err);
      TRACE_LOG("%s\n",Err.Text); //bug (no .Text) found by MinGW
      log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
#ifndef ONEGAME
      MessageBox(NULL,Err,T("Steem Engine DirectDraw Error"),
                    MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
#endif
      return ~DD_OK;
    }
    DBG_LOG("STARTUP: Initialising DirectDraw object");
    if((DErr=DDObj1->Initialize(NULL))!=DD_OK) 
    {
      DDObj1->Release();
      REPORT_DD_ERR("Initialize",DErr);
      return DDError("Initialise FAILED",DErr);
    }
    DBG_LOG("STARTUP: Calling QueryInterface");
#if defined(SSE_VID_DD7)
/*  Apparently using DirectDraw 7 instead of 2 doesn't change anything.
    This was cheap to do anyway.
*/
    if((DErr=DDObj1->QueryInterface(IID_IDirectDraw7,(LPVOID*)&DDObj))!=DD_OK)
#else
    if((DErr=DDObj1->QueryInterface(IID_IDirectDraw2,(LPVOID*)&DDObj))!=DD_OK)
#endif
    {
      REPORT_DD_ERR("QueryInterface",DErr);
      return DDError("QueryInterface FAILED",DErr);
    }
    DBG_LOG("STARTUP: Calling SetCooperativeLevel");
    if((DErr=DDObj->SetCooperativeLevel(StemWin,DDSCL_NORMAL))!=DD_OK)
    {
      REPORT_DD_ERR("SetCooperativeLevel",DErr);
      return DDError("SetCooperativeLevel FAILED",DErr);
    }
    DBG_LOG("STARTUP: Creating the clipper");
    if((DErr=DDObj->CreateClipper(0,&DDClipper,NULL))!=DD_OK)
    {
      REPORT_DD_ERR("CreateClipper",DErr);
      return DDError("CreateClipper FAILED",DErr);
    }
    DBG_LOG("STARTUP: Associating clipper with main window");
    if((DErr=DDClipper->SetHWnd(0,StemWin))!=DD_OK)
    {
      REPORT_DD_ERR("SetHWnd",DErr);
      return DDError("SetHWnd FAILED",DErr);
    }
    DBG_LOG("STARTUP: Creating surfaces");
    Method=DISPMETHOD_DD;
    if((DErr=DDCreateSurfaces())!=DD_OK) 
    {
      Method=0;
      return DErr;
    }
    DBG_LOG("STARTUP: Performing lock test");
    DDLockFlags=DDLOCK_NOSYSLOCK;
    ddsd.dwSize=sizeof(ddsd);
    if(DDBackSur->Lock(NULL,&ddsd,DDLOCK_WAIT|DDLockFlags,NULL)!=DD_OK) 
    {
      DDLockFlags=0;
      if((DErr=DDBackSur->Lock(NULL,&ddsd,DDLOCK_WAIT|DDLockFlags,NULL))!=DD_OK) 
      {
        REPORT_DD_ERR("Lock",DErr);
        return DDError("Lock test FAILED",DErr);
      }
    }
    DDBackSur->Unlock(NULL);
    DBG_LOG("STARTUP: Enumerating display modes");

    ZeroMemory(DDDisplayModePossible,sizeof(DDDisplayModePossible));
    ZeroMemory(DDClosestHz,sizeof(DDClosestHz));
    ZeroMemory(&fs_res,NFSRES*sizeof(POINT));
    DDObj->EnumDisplayModes(DDEDM_REFRESHRATES,NULL,this,DDEnumModesCallback);
    for(int idx=0;idx<NPC_HZ_CHOICES;idx++) 
    {
      for(int hicol=0;hicol<2;hicol++) 
      {
        for(int n=1;n<NUM_HZ;n++) 
          if(DDClosestHz[idx][hicol][n]==0) 
            DDClosestHz[idx][hicol][n]=HzIdxToHz[n];
      }
    }

    TRACE_LOG("Formats 8bit %d 16bit %d 32bit %d\n",
      SSEConfig.VideoCard8bit,SSEConfig.VideoCard16bit,SSEConfig.VideoCard32bit);
#if defined(SSE_DEBUG)
    DDCAPS caps_driver;
    DDObj->GetCaps(&caps_driver,NULL);
#if defined(SSE_VID_DD7)
    TRACE_LOG("DD7 Init OK, caps %X %X\n",caps_driver.dwCaps,caps_driver.dwCaps2);
#else
    TRACE_LOG("DD2 Init OK, caps %X %X %X\n",caps_driver.dwCaps,caps_driver.dwCaps2,caps_driver.ddsCaps.dwCaps);
#endif
#endif
    return DD_OK;
  }catch(...){
    TRACE_LOG("DirectDraw caused DISASTER!\n");
    return DDError("DirectDraw caused DISASTER!",DDERR_EXCEPTION);
  }
}


#if defined(SSE_VID_DD7)
HRESULT WINAPI TSteemDisplay::DDEnumModesCallback(LPDDSURFACEDESC2 pddsd,LPVOID t)
#else
HRESULT WINAPI TSteemDisplay::DDEnumModesCallback(LPDDSURFACEDESC pddsd,LPVOID t)
#endif
{  
/*  Finally understood why DirectDraw fullscreen wouldn't work on some
    systems. It's not the resolution, it's bpp. All video card drivers
    won't support 16bit display. 
*/
  if(pddsd->ddpfPixelFormat.dwRGBBitCount==8)
    SSEConfig.VideoCard8bit=true;
  else if(pddsd->ddpfPixelFormat.dwRGBBitCount==16)
    SSEConfig.VideoCard16bit=true;
  else if(pddsd->ddpfPixelFormat.dwRGBBitCount==32)
    SSEConfig.VideoCard32bit=true;
  // this is a static function, hence the need for This 
  TSteemDisplay *This=(TSteemDisplay*)t;
  int hicol=(pddsd->ddpfPixelFormat.dwRGBBitCount>8),idx=-1; //hicol=16+bit
  if(pddsd->dwWidth==640&&pddsd->dwHeight==480) 
    idx=0;
  if(pddsd->dwWidth==800&&pddsd->dwHeight==600) 
    idx=1;
  if(pddsd->dwWidth==640&&pddsd->dwHeight==400) 
    idx=2;
  if(pddsd->dwWidth==monitor_width && pddsd->dwHeight==monitor_height) 
    idx=3;
  // record res for custom res
  for(int i=0;i<NFSRES;i++)
  {
    if(Disp.fs_res[i].x==(LONG)pddsd->dwWidth 
      && Disp.fs_res[i].y==(LONG)pddsd->dwHeight)
      break; // already recorded
    if(Disp.fs_res[i].x==0) // free
    {
      Disp.fs_res[i].x=pddsd->dwWidth;
      Disp.fs_res[i].y=pddsd->dwHeight;
      break;
    }
  }
  //TRACE2("%d %dx%d %dbit %dHz\n",idx,pddsd->dwWidth,pddsd->dwHeight,pddsd->ddpfPixelFormat.dwRGBBitCount,pddsd->dwRefreshRate);
  if(idx>=0) 
  {
    This->DDDisplayModePossible[idx][hicol]=true;
    TRACE_LOG("Adding idx %d hicol %d w %d h%d %dHz\n",idx,hicol,pddsd->dwWidth,pddsd->dwHeight,pddsd->dwRefreshRate);
    for(int n=1;n<NUM_HZ;n++) 
    {
      int diff=abs(HzIdxToHz[n]-int(pddsd->dwRefreshRate));
      int curdiff=abs(HzIdxToHz[n]-int(This->DDClosestHz[idx][hicol][n]));
      if(diff<curdiff && diff<=DISP_MAX_FREQ_LEEWAY) {
        This->DDClosestHz[idx][hicol][n]=pddsd->dwRefreshRate;
        TRACE_LOG("Adding close Hz\n");
      }
    }
  }
  return DDENUMRET_OK;
}


HRESULT TSteemDisplay::DDCreateSurfaces() {
  HRESULT DErr=1234;
  if(!DDObj)
    return DErr;
  DDDestroySurfaces();
  int ExtraFlags=0;
  for(int n=0;n<2;n++) 
  {
    ZeroMemory(&ddsd,sizeof(ddsd));
    ddsd.dwSize=sizeof(ddsd);
    ddsd.dwFlags=DDSD_CAPS;
    ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE | ExtraFlags;
    if(FullScreen&&!OPTION_FAKE_FULLSCREEN) 
    {
      ddsd.ddsCaps.dwCaps|=DDSCAPS_FLIP | DDSCAPS_COMPLEX;
      // for fullscreen, both surfaces in same memory
      if(DrawToVidMem==0)
        ExtraFlags=DDSCAPS_SYSTEMMEMORY;
      ddsd.dwFlags|=DDSD_BACKBUFFERCOUNT;
      ddsd.dwBackBufferCount=1;
#if defined(SSE_VID_3BUFFER_FS)
      // In fullscreen mode, this is as simple as that, like in the D3D build.
      if(OPTION_3BUFFER_FS)
        ddsd.dwBackBufferCount++;
#endif
    }
    if(FullScreen&&OPTION_FAKE_FULLSCREEN) 
    {
#if defined(SSE_VID_2SCREENS)
      CheckCurrentMonitorConfig(); // Update monitor rectangle
#endif
      ddsd.dwWidth=rcMonitor.right-rcMonitor.left;
      ddsd.dwHeight=rcMonitor.bottom-rcMonitor.top;
    }
    if((DErr=DDObj->CreateSurface(&ddsd,&DDPrimarySur,NULL))!=DD_OK) 
    {
      if(n==0) 
      {
        if(ExtraFlags)
          ExtraFlags = 0;
        else
          ExtraFlags=DDSCAPS_SYSTEMMEMORY;
      }
      else
      {
        REPORT_DD_ERR("DDPrimarySur",DErr);
        // Another DirectX app is fullscreen so fail silently
        if(DErr==DDERR_NOEXCLUSIVEMODE) return DErr;
        // Otherwise make a big song and dance!
        return DDError("CreateSurface for PrimarySur FAILED",DErr);
      }
    }
    else
      break;
  }
  ddsd.dwSize=sizeof(ddsd);
  DDPrimarySur->GetSurfaceDesc(&ddsd);
  if(FullScreen&&!OPTION_FAKE_FULLSCREEN)
    DDBackSurIsAttached=true;
#if defined(SSE_VID_DD_MISC)
  else // Windows 10, clipper ruins fullscreen?
#endif
  if((DErr=DDPrimarySur->SetClipper(DDClipper))!=DD_OK) 
  {
    REPORT_DD_ERR("SetClipper",DErr);
    return DDError("SetClipper FAILED",DErr);
  }
  if(FullScreen==0||OPTION_FAKE_FULLSCREEN)
  {
    if(DrawToVidMem==0) // only for buffer in window mode
      ExtraFlags=DDSCAPS_SYSTEMMEMORY; // Like malloc
    for(int n=0;n<2;n++) 
    {
      ZeroMemory(&DDBackSurDesc,sizeof(DDBackSurDesc));
      DDBackSurDesc.dwSize=sizeof(DDBackSurDesc);
      DDBackSurDesc.dwFlags=DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
      DDBackSurDesc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN | ExtraFlags;
      if(extended_monitor)
      {
        DDBackSurDesc.dwWidth=GetScreenWidth();
        DDBackSurDesc.dwHeight=GetScreenHeight();
      }
      else if(Disp.BorderPossible())
      { //SS: GetScreenWidth()>640
        DDBackSurDesc.dwWidth=640+4* (SideBorderSize); // we draw larger
        DDBackSurDesc.dwHeight=400+2*(BORDER_TOP+BottomBorderSize);
      }
      else
      {
        DDBackSurDesc.dwWidth=640;
        DDBackSurDesc.dwHeight=480;
      }
      if(draw_blit_source_rect.right>=int(DDBackSurDesc.dwWidth)) 
        draw_blit_source_rect.right=int(DDBackSurDesc.dwWidth)-1;
      if(draw_blit_source_rect.bottom>=int(DDBackSurDesc.dwHeight))
        draw_blit_source_rect.bottom=int(DDBackSurDesc.dwHeight)-1;
      if((DErr=DDObj->CreateSurface(&DDBackSurDesc,&DDBackSur,NULL))!=DD_OK) 
      {
        if(n==0)
          ExtraFlags=0;
        else
        {
          REPORT_DD_ERR("DDBackSur",DErr);
          return DDError("CreateSurface for BackSur FAILED",DErr);
        }
      }
      else
      {
#if defined(SSE_VID_DD_3BUFFER_WIN)
        if(OPTION_3BUFFER_WIN)
        {// Let's create a second back surface for our "triple buffer"
          DErr=DDObj->CreateSurface(&DDBackSurDesc,&DDBackSur2,NULL);
          if(DErr!=DD_OK)
          {
            REPORT_DD_ERR("DDBackSur2",DErr);
            //ASSERT(DDBackSur2==NULL);
            DDBackSur2=NULL; // doc doesn't state it is null
          }
          VSyncTiming=0;
          SurfaceToggle=true; // will be toggled false at first lock
        }
#endif
        break;
      }
    }
  }
  else
  {// SS Fullscreen
#if defined(SSE_VID_DD7)
    DDSCAPS2 caps;
#else
    DDSCAPS caps;
#endif
    ZeroMemory(&caps,sizeof(caps));
    caps.dwCaps=DDSCAPS_BACKBUFFER;
    if((DErr=DDPrimarySur->GetAttachedSurface(&caps,&DDBackSur))!=DD_OK) 
    {
      REPORT_DD_ERR("DDBackSur",DErr);
      return DDError("CreateSurface for BackSur FAILED",DErr);
    }
  }
  DDBackSurDesc.dwSize=sizeof(DDBackSurDesc);
  if((DErr=DDBackSur->GetSurfaceDesc(&DDBackSurDesc))!=DD_OK) 
  {
    REPORT_DD_ERR("DDBackSurDesc",DErr);
    return DDError("GetSurfaceDesc for BackSur FAILED",DErr);
  }
  SurfaceWidth=DDBackSurDesc.dwWidth;
  SurfaceHeight=DDBackSurDesc.dwHeight;
#if defined(SSE_VID_32BIT_ONLY)
  ASSERT((DDBackSurDesc.ddpfPixelFormat.dwRGBBitCount/8)==4);
#else
  BytesPerPixel=(BYTE)DDBackSurDesc.ddpfPixelFormat.dwRGBBitCount/8;
#endif
  rgb555=(DDBackSurDesc.ddpfPixelFormat.dwGBitMask==0x3E0); //%0000001111100000  //1555
  rgb32_bluestart_bit=int((DDBackSurDesc.ddpfPixelFormat.dwBBitMask==0x0000ff00) ? 8:0);
  draw_mem=NULL;
  OurBackSur=DDBackSur;
  Lock();
  if(draw_mem)
    ZeroMemory(draw_mem,VideoMemorySize);
  Unlock();
  draw_init_resdependent();
  palette_prepare(
#if !defined(SSE_VID_32BIT_ONLY)
    true
#endif
    );
  TRACE2("Primary %dx%d %dbit caps %X flags %X FS %d buffers %d pitch %d\n",ddsd.dwWidth, ddsd.dwHeight,ddsd.ddpfPixelFormat.dwRGBBitCount,ddsd.ddsCaps.dwCaps, ddsd.dwFlags,(FullScreen?(OPTION_FAKE_FULLSCREEN?2:1):0),ddsd.dwBackBufferCount,ddsd.lPitch);
  TRACE2("Back %dx%d %dbit caps %X flags %X buffers %d pitch %d\n",DDBackSurDesc.dwWidth, DDBackSurDesc.dwHeight,DDBackSurDesc.ddpfPixelFormat.dwRGBBitCount,DDBackSurDesc.ddsCaps.dwCaps,DDBackSurDesc.dwFlags,DDBackSurDesc.dwBackBufferCount,DDBackSurDesc.lPitch);
  return DD_OK;
}


void TSteemDisplay::DDDestroySurfaces() {
#if defined(SSE_VID_D3D)
  D3DDestroySurfaces();
#endif
  if(DDPrimarySur)
  {
    DDPrimarySur->Release(); 
    DDPrimarySur=NULL;
    if(DDBackSurIsAttached) 
      DDBackSur=NULL;
  }
  if(DDBackSur)
  {
    DDBackSur->Release(); 
    DDBackSur=NULL;
  }
#if defined(SSE_VID_DD_3BUFFER_WIN)
  if(DDBackSur2) 
  {
    DDBackSur2->Release(); 
    DDBackSur2=OurBackSur=NULL;
  }
#endif
  DDBackSurIsAttached=0;
}


HRESULT TSteemDisplay::DDError(char *ErrorText,HRESULT DErr) {
  Release();
  StatusInfo.MessageIndex = TStatusInfo::BLIT_ERROR;
  char Text[1000];
  strcpy(Text,ErrorText);
  strcat(Text,"\n\n");
  DDGetErrorDescription(DErr,Text+(int)strlen(Text),499-(int)strlen(Text));
  log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  log_write(Text);
  TRACE_LOG("!!!!!!!\n%s\n!!!!!!!!!!!!!!",Text);
  log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  strcat(Text,EasyStr("\n\n")+T("Would you like to disable the use of DirectDraw?"));
#ifndef ONEGAME
  int Ret=MessageBox(NULL,Text,T("Steem Engine DirectDraw Error"),
    MB_YESNO|MB_ICONEXCLAMATION|MB_SETFOREGROUND|MB_TASKMODAL|MB_TOPMOST);
  if(Ret==IDYES) 
    WriteCSFStr("Options","NoDirectDraw","1",globalINIFile);
#endif
  return DErr;
}

#endif


bool TSteemDisplay::InitGDI() { // SS generally Direct X is used instead
  Release(); // note this will kill D3D
  WORD w=640,h=480;
  if(extended_monitor) 
  {
    w=(WORD)GetScreenWidth();
    h=(WORD)GetScreenHeight();
  }
  else if(GuiSM.cx_screen()>640+4*SideBorderSizeWin
    // testing also vertical pixels (bugfix 3.4.1) but... GDI? (useless)
    && GuiSM.cy_screen()>400+2*(BORDER_TOP+BORDER_BOTTOM))
  {
    w=640+4*SideBorderSizeWin;
    h=400+2*(BORDER_TOP+BORDER_BOTTOM);
  }
  DBG_LOG(Str("STARTUP: Creating bitmap w=")+w+" h="+h);
  HDC dc=GetDC(NULL);
  GDIBmp=CreateCompatibleBitmap(dc,w,h);
  ReleaseDC(NULL,dc);
  if(GDIBmp==NULL)
    return 0;
  BITMAP BmpInf;
  GetObject(GDIBmp,sizeof(BITMAP),&BmpInf);
#if defined(SSE_VID_32BIT_ONLY)
  ASSERT(((BmpInf.bmBitsPixel+7)/8)==4);
#else
  BytesPerPixel=(BYTE)(BmpInf.bmBitsPixel+7)/8;
#endif
  GDIBmpLineLength=BmpInf.bmWidthBytes;
  GDIBmpSize=GDIBmpLineLength*BmpInf.bmHeight;
  DBG_LOG(Str("STARTUP: BytesPerPixel=")+BytesPerPixel+" GDIBmpLineLength="+GDIBmpLineLength+" GDIBmpSize="+GDIBmpSize);
  GDIBmpDC=CreateCompatibleDC(NULL);
  SelectObject(GDIBmpDC,GDIBmp);
  SelectObject(GDIBmpDC,fnt);
  DBG_LOG("STARTUP: Creating bitmap memory");
  try{
    GDIBmpMem=new BYTE[GDIBmpSize+1];
  }catch (...){
    GDIBmpMem=NULL;
    Release();
    return 0;
  }
  if(BytesPerPixel>1) 
  {
    SetPixel(GDIBmpDC,0,0,RGB(255,0,0));
    GetBitmapBits(GDIBmp,GDIBmpSize,GDIBmpMem);
    DWORD RedBitMask=0;
    for(int i=BytesPerPixel-1;i>=0;i--)
    {
      RedBitMask<<=8;
      RedBitMask|=GDIBmpMem[i];
    }
    rgb555=(RedBitMask==(hib01111100 | b00000000));
    rgb32_bluestart_bit=int((RedBitMask==0xff000000) ? 8:0);
  }
  SurfaceWidth=w;
  SurfaceHeight=h;
  DBG_LOG(Str("STARTUP: rgb555=")+rgb555+" rgb32_bluestart_bit="+rgb32_bluestart_bit+
        " SurfaceWidth="+SurfaceWidth+" SurfaceHeight="+SurfaceHeight);
  palette_prepare(
#if !defined(SSE_VID_32BIT_ONLY)
    true
#endif
    );
  draw_init_resdependent();
  return true;
}

#ifdef STEEM_CRT 

int TSteemDisplay::CRTthreadProc() {
  HMODULE dll;
  HGLRC context; 
  PROC (CRTEMU_GLCALLTYPE* wglGetProcAddress) (LPCSTR);
  HGLRC (CRTEMU_GLCALLTYPE* wglCreateContext) (HDC);
  BOOL (CRTEMU_GLCALLTYPE* wglDeleteContext) (HGLRC);
  BOOL (CRTEMU_GLCALLTYPE* wglMakeCurrent) (HDC, HGLRC);
  BOOL (CRTEMU_GLCALLTYPE* wglSwapIntervalEXT) (int);
  dll = LoadLibraryA( "opengl32.dll" );
  wglGetProcAddress = (PROC(CRTEMU_GLCALLTYPE*)(LPCSTR)) (uintptr_t) GetProcAddress( dll, "wglGetProcAddress" );
  wglCreateContext = (HGLRC(CRTEMU_GLCALLTYPE*)(HDC)) (uintptr_t) GetProcAddress( dll, "wglCreateContext" );
  wglDeleteContext = (BOOL(CRTEMU_GLCALLTYPE*)(HGLRC)) (uintptr_t) GetProcAddress( dll, "wglDeleteContext" );
  wglMakeCurrent = (BOOL(CRTEMU_GLCALLTYPE*)(HDC, HGLRC)) (uintptr_t) GetProcAddress( dll, "wglMakeCurrent" );
  PIXELFORMATDESCRIPTOR pfd;
  memset( &pfd, 0, sizeof( pfd ) );
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;
  HDC CRTdc = GetDC( CRThwnd );
  SetPixelFormat( CRTdc, ChoosePixelFormat( CRTdc, &pfd ), &pfd );
  	
  context = wglCreateContext( CRTdc ); 
  wglMakeCurrent( CRTdc, context );
  
  wglSwapIntervalEXT = (BOOL (CRTEMU_GLCALLTYPE*)(int)) (uintptr_t) wglGetProcAddress( "wglSwapIntervalEXT" );
  
  LARGE_INTEGER perfc;
  QueryPerformanceCounter( &perfc );
  CRTEMU_U64 CRTstart = perfc.QuadPart;
  
  LARGE_INTEGER perff;
  QueryPerformanceFrequency( &perff );

  crtemu_t* crtemu = crtemu_create( NULL );
  CRT_FRAME_U32* frame = (CRT_FRAME_U32*) malloc( sizeof( CRT_FRAME_U32 ) * CRT_FRAME_WIDTH * CRT_FRAME_HEIGHT );
  crt_frame( frame );
  crtemu_frame( crtemu, frame, CRT_FRAME_WIDTH, CRT_FRAME_HEIGHT );
  free( frame );
  
  if( wglSwapIntervalEXT ) wglSwapIntervalEXT( 1 );
  frametimer_t* frametimer = frametimer_create( NULL );
  if( !wglSwapIntervalEXT ) {
      frametimer_lock_rate( frametimer, 60 );
  }
  CRTpixels = (CRTEMU_U32*) malloc( sizeof( CRTEMU_U32 ) * 1280 * 1024 );
  CRTEMU_U32* CRTpixelsCopy = (CRTEMU_U32*) malloc( sizeof( CRTEMU_U32 ) * 1280 * 1024 );
  int CRTwidthCopy = 0;
  int CRTheightCopy = 0;
  int CRTviewyCopy = 0;
  int CRTviewwCopy = 0;
  int CRTviewhCopy = 0;

  thread_signal_raise( &CRTsignal );
  while( !CRTexit ) {  
      thread_mutex_lock( &CRTmutex );
      CRTwidthCopy = CRTwidth;
      CRTheightCopy = CRTheight;
      CRTviewyCopy = CRTviewy;
      CRTviewwCopy = CRTvieww;
      CRTviewhCopy = CRTviewh;
      memcpy( CRTpixelsCopy, CRTpixels, CRTwidth * CRTheight * 4 );
      thread_mutex_unlock( &CRTmutex );
  	
      frametimer_update( frametimer );
      if( CRTviewwCopy > 0 && CRTviewhCopy > 0 ) { 
          QueryPerformanceCounter( &perfc );
          CRTEMU_U64 delta = perfc.QuadPart - CRTstart; 
          CRTEMU_U64 time_us = delta / ( perff.QuadPart / 1000000 );
          crtemu->Viewport( 0, 0, CRTviewwCopy, CRTviewhCopy );    
          crtemu_present( crtemu, time_us, (CRTEMU_U32*)CRTpixelsCopy, CRTwidthCopy, CRTheightCopy, 0xffffffff, 0xff181818, CRTviewy  );      
          SwapBuffers( CRTdc );
          thread_signal_raise( &CRTsignal );
      }
  }
  frametimer_destroy( frametimer );
  crtemu_destroy( crtemu  );
  wglMakeCurrent( CRTdc, NULL );
  wglDeleteContext( context );
  ReleaseDC( CRThwnd, CRTdc );
  FreeLibrary( dll );
  free( CRTpixels );
  free( CRTpixelsCopy );
  CRTpixels = NULL;
  return 0;
}

int TSteemDisplay::CRTthreadWrapper( void* user_data ) {
    TSteemDisplay* display = (TSteemDisplay*) user_data;
    return display->CRTthreadProc();
}


bool TSteemDisplay::InitCRT() { // SS generally Direct X is used instead
  Release(); // note this will kill D3D
  WORD w=640,h=480;
  if(extended_monitor) 
  {
    w=(WORD)GetScreenWidth();
    h=(WORD)GetScreenHeight();
  }
  else if(GuiSM.cx_screen()>640+4*SideBorderSizeWin
    && GuiSM.cy_screen()>400+2*(BORDER_TOP+BORDER_BOTTOM))
  {
    w=640+4*SideBorderSizeWin;
    h=400+2*(BORDER_TOP+BORDER_BOTTOM);
  }

  CRTBmpLineLength = 4 * w;
  CRTBmpSize = CRTBmpLineLength * h;  
  CRTBmpMem=new BYTE[CRTBmpSize+1];
        
  SurfaceWidth=w;
  SurfaceHeight=h;
  DBG_LOG(Str("STARTUP: rgb555=")+rgb555+" rgb32_bluestart_bit="+rgb32_bluestart_bit+
        " SurfaceWidth="+SurfaceWidth+" SurfaceHeight="+SurfaceHeight);
  palette_prepare(
#if !defined(SSE_VID_32BIT_ONLY)
    true
#endif
    );
  draw_init_resdependent();

  WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW,  
      (WNDPROC) DefWindowProc, 0, 0, 0, 0, 0, 0, 0, TEXT( "steem_crt_wc" ), 0 };
  wc.hInstance = GetModuleHandle( NULL ); wc.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH ); 
  RegisterClassEx( &wc );
  CRThwnd = CreateWindowEx( WS_EX_TRANSPARENT, wc.lpszClassName, 0, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 0, MENUHEIGHT+2, SurfaceWidth, SurfaceHeight - (MENUHEIGHT+2), StemWin, (HMENU) 0, GetModuleHandle( NULL ), 0 );

  thread_mutex_init( &CRTmutex );
  thread_signal_init( &CRTsignal );
  CRTexit = 0;
  CRTthread = thread_create( CRTthreadWrapper, this, THREAD_STACK_SIZE_DEFAULT );
  thread_signal_wait( &CRTsignal, THREAD_SIGNAL_WAIT_INFINITE );
  return true;
}


#endif

#if !defined(SSE_VID_NO_FREEIMAGE)

void TSteemDisplay::FreeImageLoad() {
  if(hFreeImage) 
    return;
  hFreeImage=SteemLoadLibrary(FREE_IMAGE_DLL);
#ifdef SSE_DEBUG
  if(hFreeImage==NULL)
    TRACE_LOG("%s not available\n",FREE_IMAGE_DLL);   
#endif
  if(hFreeImage==NULL)
    return;
  FreeImage_Initialise=(FI_INITPROC)GetProcAddress(hFreeImage,"_FreeImage_Initialise@4");
  FreeImage_DeInitialise=(FI_DEINITPROC)GetProcAddress(hFreeImage,"_FreeImage_DeInitialise@0");
  FreeImage_ConvertFromRawBits=
    (FI_CONVFROMRAWPROC)GetProcAddress(hFreeImage,"_FreeImage_ConvertFromRawBits@36");
  FreeImage_FIFSupportsExportBPP=
    (FI_SUPPORTBPPPROC)GetProcAddress(hFreeImage,"_FreeImage_FIFSupportsExportBPP@8");
  FreeImage_Save=(FI_SAVEPROC)GetProcAddress(hFreeImage,"_FreeImage_Save@16");
  FreeImage_Free=(FI_FREEPROC)GetProcAddress(hFreeImage,"_FreeImage_Free@4");
  if(!FreeImage_Free) // breaking change!
    FreeImage_Free=(FI_FREEPROC)GetProcAddress(hFreeImage,"_FreeImage_Unload@4");
  if(FreeImage_Initialise==NULL||FreeImage_DeInitialise==NULL||
    FreeImage_ConvertFromRawBits==NULL||FreeImage_Save==NULL||
    FreeImage_FIFSupportsExportBPP==NULL||FreeImage_Free==NULL) 
  {
    FreeLibrary(hFreeImage);hFreeImage=NULL;
    return;
  }
  FreeImage_Initialise(TRUE);
  SSEConfig.FreeImageDll=TRUE;
}

#endif

#endif //WIN32

//#pragma warning (disable: 4701) //SurLineLen

HRESULT TSteemDisplay::SaveScreenShot() {
  Str ShotFile=ScreenShotNextFile;
  ScreenShotNextFile="";
  bool ToClipboard=(int(ScreenShotFormat)==IF_TOCLIPBOARD);
  if(!ToClipboard && ShotFile.Empty()) // create file name
  {
    DWORD Attrib=GetFileAttributes(ScreenShotFol);
    if(Attrib==0xffffffff||(Attrib & FILE_ATTRIBUTE_DIRECTORY)==0) 
      return DDERR_GENERIC;
#if defined(SSE_VID_D3D)
    Str Exts=ScreenShotExt; // can be JPG or PNG too
#else
    Str Exts="bmp";
    WIN_ONLY(if(hFreeImage) Exts=ScreenShotExt; )
#endif
    if(ScreenShotFormat==IF_NEO)
      Exts="NEO";
    EasyStr FirstWord="Steem_";
    if(FloppyDisk[0].DiskName.NotEmpty())
    {
      FirstWord=FloppyDisk[0].DiskName;
      if(!ScreenShotUseFullName) 
      {
        char *spc=strchr(FirstWord,' ');
        if(spc) 
          *spc=0;
      }
    }
    bool AddNumExt=true;
    if(ScreenShotUseFullName)
    {
      ShotFile=ScreenShotFol+SLASH+FirstWord+"."+Exts;
      if(Exists(ShotFile)==0) 
        AddNumExt=ScreenShotAlwaysAddNum;
    }
    if(AddNumExt) 
    {
      int Num=0;
      do {
        if(++Num>=100000) 
          return DDERR_GENERIC;
        ShotFile=ScreenShotFol+SLASH+FirstWord+"_"
          +(EasyStr("00000")+Num).Rights(5)+"."+Exts;
      } while(Exists(ShotFile));
    }
  }
  if(ScreenShotFormat==IF_NEO && pNeoFile!=NULL)
  {
    //ASSERT(!ToClipboard);
    pNeoFile->resolution=screen_res;
    SWAP_BIG_ENDIAN_WORD(pNeoFile->resolution);
    // palette was already copied (sooner=better)
    for(int i=0;i<16000;i++)
    {
      pNeoFile->data[i]=DPEEK(vbase+i*2);
      SWAP_BIG_ENDIAN_WORD(pNeoFile->data[i]);
    }
    FILE *f=fopen(ShotFile,"wb");
    if(f)
    {
      fwrite(pNeoFile,sizeof(neochrome_file),1,f);
      TRACE_LOG("Save screenshot %s res %d\n",ShotFile.Text,screen_res);
      fclose(f);
    }
    delete pNeoFile;
    pNeoFile=NULL;
#if defined(SSE_STATS)
    if(OPTION_WARNINGS && Stats.nPal)
      Alert(T("Palette changes during frame, check colors!"),T("Warning"),
        MB_ICONEXCLAMATION);
#endif
    return DD_OK;
  }
  BYTE *SurMem=NULL;
  long SurLineLen;
  int w,h;
#ifdef WIN32
#if defined(SSE_VID_D3D)
  IDirect3DSurface9 *BackBuff=NULL,*SaveSur=NULL;
#elif defined(SSE_VID_DD)
#if defined(SSE_VID_DD7)
  IDirectDrawSurface7 *SaveSur=NULL;
#else
  IDirectDrawSurface *SaveSur=NULL;
#endif
#endif
  HBITMAP SaveBmp=NULL;
#endif//win32
  // Need to create new surfaces so we can blit in the same way we do to the
  // window, just in case image must be stretched. We can't do this ourselves
  // (even if we wanted to) because some video cards will blur.
  switch(Method) {
#if defined(SSE_VID_D3D)
  case DISPMETHOD_D3D:
  {
    if(!pD3D||!pD3DDevice)
      return DDERR_GENERIC;
    HRESULT DErr;
    D3DDISPLAYMODE d3ddm;
    if((DErr=pD3D->GetAdapterDisplayMode(m_Adapter,&d3ddm))!=D3D_OK)
    {
      REPORT_D3D_ERR("GetAdapterDisplayMode",DErr);
      return DErr;
    }
    w=draw_blit_source_rect.right-draw_blit_source_rect.left;
    if(!screen_res && SCANLINES_INTERPOLATED)
      w*=2; //yeah, yeah...
    h=draw_blit_source_rect.bottom-draw_blit_source_rect.top;
    //ASSERT((w)&&(h));
/*  Source = BackBuff, Destination = SaveSur
    We just get a pointer to the back buffer.
*/
    if((DErr=pD3DDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&BackBuff))!=0)
    {
      REPORT_D3D_ERR("GetBackBuffer",DErr);
      return DErr;
    }
/*  If the target surface is a plain surface, we can't use StretchRect on it,
    so we use CreateRenderTarget() instead.
    TRUE for lockable, necessary for FreeImage.
*/
    if((DErr=pD3DDevice->CreateRenderTarget(w,h,d3ddm.Format,D3DMULTISAMPLE_NONE,
      0,TRUE,&SaveSur,NULL))!=D3D_OK)
    {
      REPORT_D3D_ERR("CreateRenderTarget",DErr);
      return DErr;
    }
    RECT rcDest={0,0,0,0};
    rcDest.right=w;
    rcDest.bottom=h;
    if(ScreenShotMinSize) // option
    {
      if(border)
      {
        rcDest.right=WinSizeBorder[screen_res][0].x;
        rcDest.bottom=WinSizeBorder[screen_res][0].y;
      }
      else
      {
        rcDest.right=WinSize[screen_res][0].x;
        rcDest.bottom=WinSize[screen_res][0].y;
      }
    }
    // copy source->destination
    if((DErr=pD3DDevice->StretchRect(BackBuff,&draw_blit_source_rect,SaveSur,&rcDest,D3DTEXF_NONE))!=DS_OK)
    {
      REPORT_D3D_ERR("StretchRect",DErr);
      // fall back on making SaveSur the back buffer
      if((DErr=pD3DDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&SaveSur))!=D3D_OK)
      {
        REPORT_D3D_ERR("StretchRect",DErr);
      }
    }
    if(BackBuff)
      BackBuff->Release();
    w=rcDest.right;h=rcDest.bottom;
#if !defined(SSE_VID_NO_FREEIMAGE)
    if(hFreeImage)
    {
      D3DLOCKED_RECT LockedRect;
      if((DErr=SaveSur->LockRect(&LockedRect,NULL,0))!=0)//390
      {
        REPORT_D3D_ERR("LockRect",DErr);
        SaveSur->Release();
        return DErr;
      }
      SurLineLen=LockedRect.Pitch;
      SurMem=(BYTE*)LockedRect.pBits;
      //ASSERT(SurLineLen);
      //ASSERT(SurMem);
    }
    else
#endif
    if(!ToClipboard)
    {
#if !defined(SSE_VID_NO_FREEIMAGE)
      D3DXIMAGE_FILEFORMAT fileformat=D3DXIFF_BMP;
      switch(ScreenShotFormat) { //note the function can't save in tga or ppm format
      case FIF_JPEG:
        fileformat=D3DXIFF_JPG;
        break;
      case FIF_PNG:
        fileformat=D3DXIFF_PNG;
        break;
      }
      DErr=D3DXSaveSurfaceToFile(ShotFile,fileformat,SaveSur,NULL,&rcDest);
#else
      DErr=D3DXSaveSurfaceToFile(ShotFile,(D3DXIMAGE_FILEFORMAT)ScreenShotFormat,
        SaveSur,NULL,&rcDest);
#endif
      TRACE_LOG("Save screenshot %s %dx%d native ERR%d\n",ShotFile.Text,w,h,DErr);
      SaveSur->Release();
      return DErr;
    }
    break;
  }
#endif
#ifdef SSE_VID_DD
  case DISPMETHOD_DD:
  {
    if(DDBackSur==NULL)
      return DDERR_GENERIC;
    RECT rcDest={0,0,0,0};
    if(ScreenShotMinSize) 
    {
      if(border) 
      {
        rcDest.right=WinSizeBorder[screen_res][0].x;
        rcDest.bottom=WinSizeBorder[screen_res][0].y;
      }
      else
      {
        rcDest.right=WinSize[screen_res][0].x;
        rcDest.bottom=WinSize[screen_res][0].y;
      }
    }
    else
    {
      if(FullScreen)
      {
        get_fullscreen_rect(&rcDest);
        OffsetRect(&rcDest,-rcDest.left,-rcDest.top);
      }
      else
      {
        GetClientRect(StemWin,&rcDest);
        rcDest.right-=4;rcDest.bottom-=4+MENUHEIGHT;
      }
    }
    w=rcDest.right;h=rcDest.bottom;
    HRESULT DErr;
#if defined(SSE_VID_DD7)
    DDSURFACEDESC2 SaveSurDesc;
#else
    DDSURFACEDESC SaveSurDesc;
#endif
    ZeroMemory(&SaveSurDesc,sizeof(SaveSurDesc));
    SaveSurDesc.dwSize=sizeof(SaveSurDesc);
    SaveSurDesc.dwFlags=DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    SaveSurDesc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    SaveSurDesc.dwWidth=w;
    SaveSurDesc.dwHeight=h;
    DErr=DDObj->CreateSurface(&SaveSurDesc,&SaveSur,NULL);
    if(DErr!=DD_OK) 
      return DErr;
    DErr=SaveSur->Blt(&rcDest,DDBackSur,&draw_blit_source_rect,DDBLT_WAIT,NULL);
    if(DErr!=DD_OK) 
    {
      SaveSur->Release();
      return DErr;
    }
    if(SaveSur->IsLost()==DDERR_SURFACELOST) 
    {
      SaveSur->Release();
      return DErr;
    }
    if(!ToClipboard) 
    {
      SaveSurDesc.dwSize=sizeof(DDSURFACEDESC);
      DErr=SaveSur->Lock(NULL,&SaveSurDesc,DDLOCK_WAIT|DDLockFlags,NULL);
      if(DErr!=DD_OK) 
      {
        SaveSur->Release();
        return DErr;
      }
      SurMem=(BYTE*)SaveSurDesc.lpSurface;
      SurLineLen=SaveSurDesc.lPitch;
    }
    break;
  }
#endif
#ifdef WIN32
  case DISPMETHOD_GDI:
  {
    if(GDIBmp==NULL) 
      return DDERR_GENERIC;
    BITMAP BmpInf;
    RECT rcDest;
    GetClientRect(StemWin,&rcDest);
    w=rcDest.right-4;h=rcDest.bottom-(4+MENUHEIGHT);
    HDC dc=GetDC(NULL);
    SaveBmp=CreateCompatibleBitmap(dc,w,h);
    ReleaseDC(NULL,dc);
    HDC SaveBmpDC=CreateCompatibleDC(NULL);
    SelectObject(SaveBmpDC,SaveBmp);
    SetStretchBltMode(SaveBmpDC,COLORONCOLOR);
    StretchBlt(SaveBmpDC,0,0,w,h,GDIBmpDC,draw_blit_source_rect.left,
      draw_blit_source_rect.top,draw_blit_source_rect.right
      -draw_blit_source_rect.left,draw_blit_source_rect.bottom
      -draw_blit_source_rect.top,SRCCOPY);
    DeleteDC(SaveBmpDC);
    if(!ToClipboard) 
    {
      GetObject(SaveBmp,sizeof(BITMAP),&BmpInf);
      SurLineLen=BmpInf.bmWidthBytes;
      try {
        DWORD BmpBytes=SurLineLen*BmpInf.bmHeight;
        SurMem=new BYTE[BmpBytes];
        GetBitmapBits(SaveBmp,BmpBytes,SurMem);
      } catch(...) {
        DeleteObject(SaveBmp);
        return DDERR_GENERIC;
      }
    }
    break;
  }
#endif
#ifdef STEEM_CRT
  case DISPMETHOD_CRT:
  {
	int blit_width = draw_blit_source_rect.right-draw_blit_source_rect.left;
	int blit_height =draw_blit_source_rect.bottom-draw_blit_source_rect.top;
    CRTEMU_U32* pixels = new CRTEMU_U32[ blit_width * blit_height * 2 ];
	CRTEMU_U32* srcpixels = (CRTEMU_U32*) CRTBmpMem;	
	if( blit_width / blit_height < 2 ) {
		for( int y = 0; y < blit_height; ++y ) {
			for( int x = 0; x < blit_width; ++x ) {
				CRTEMU_U32 c = srcpixels[ x + y * (CRTBmpLineLength / 4 ) ];
				pixels[ x + y * blit_width ] = c;
			}
		}
	} else {
		for( int y = 0; y < blit_height; ++y ) {
			for( int x = 0; x < blit_width; ++x ) {
				CRTEMU_U32 c = srcpixels[ x + y * (CRTBmpLineLength / 4 ) ];
				pixels[ x + ( 2*y + 0 )* blit_width ] = c;
				pixels[ x + ( 2*y + 1 )* blit_width ] = c;
			}
		}
		blit_height *= 2;
	}

    w = blit_width;
	h = blit_height;
    SurMem = (LPBYTE) pixels;
    SurLineLen = w * 4;
    break;
  }
#endif
#if defined(UNIX)
  // No need to create a new surface here, X can't stretch
  case DISPMETHOD_XSHM:
  case DISPMETHOD_X:
    if(X_Img==NULL) 
      return DDERR_GENERIC;
    w=draw_blit_source_rect.right;
    h=draw_blit_source_rect.bottom;
    SurMem=LPBYTE(X_Img->data);
    SurLineLen=X_Img->bytes_per_line;
    break;
#endif
  default:
    return DDERR_GENERIC;
  }//sw
  BYTE *Pixels=SurMem;
  bool ConvertPixels=true;
#ifdef WIN32
#if 0 && !defined(SSE_VID_NO_FREEIMAGE) // why was it removed?
  if(hFreeImage && !ToClipboard && BytesPerPixel>1)
  {
    if(FreeImage_FIFSupportsExportBPP((FREE_IMAGE_FORMAT)ScreenShotFormat,
      BytesPerPixel*8))
      ConvertPixels=0;
  }
#endif
#endif
  if(ToClipboard) 
  {
    ConvertPixels=0;
#ifdef WIN32
    if(Method==DISPMETHOD_DD || Method==DISPMETHOD_D3D) 
    {
      HDC DDSaveSurDC=NULL;
      HRESULT DErr=SaveSur->GetDC(&DDSaveSurDC);
      if(DErr!=DD_OK) 
      {
        SaveSur->Release();
        return DErr;
      }
      HDC dc=GetDC(NULL);
      SaveBmp=CreateCompatibleBitmap(dc,w,h);
      ReleaseDC(NULL,dc);
      HDC SaveBmpDC=CreateCompatibleDC(NULL);
      SelectObject(SaveBmpDC,SaveBmp);
      BitBlt(SaveBmpDC,0,0,w,h,DDSaveSurDC,0,0,SRCCOPY);
      DeleteDC(SaveBmpDC);
      SaveSur->ReleaseDC(DDSaveSurDC);
    }
    if(OpenClipboard(StemWin)) 
    {
      EmptyClipboard();
      SetClipboardData(CF_BITMAP,SaveBmp);
      TRACE_LOG("Copy screenshot %dx%d to clipboard\n",w,h);
      CloseClipboard(); // don't return here, must clean up
    }
#endif
  }
  else if(ConvertPixels)
  {
    Pixels=new BYTE[w*h*3 + 16];
    BYTE *pPix=Pixels;
    switch(BytesPerPixel) {
#if !defined(SSE_VID_32BIT_ONLY)
    case 1:
    {
      DWORD Col;
      BYTE *pSur=SurMem+((h-1)*SurLineLen),*pSurLineEnd;
      while(pSur>=SurMem) {
        pSurLineEnd=pSur+w;
        for(;pSur<pSurLineEnd;pSur++) 
        {
          Col=(DWORD)logpal[(*pSur)-1];
          *LPDWORD(pPix)=((Col&0xff)<<16)|(Col&0x00ff00)|((Col&0xff0000)>>16);
          pPix+=3;
        }
        pSur-=SurLineLen+w;
      }
      break;
    }
    case 2:
    {
      WORD Col;
      WORD *pSur=LPWORD(SurMem+((h-1)*SurLineLen)),*pSurLineEnd;
      if(rgb555) {
        while(LPBYTE(pSur)>=SurMem) {
          pSurLineEnd=pSur+w;
          for(;pSur<pSurLineEnd;pSur++) 
          {
            Col=*pSur;
            pPix[0]=BYTE((Col<<3) & b11111000);
            pPix[1]=BYTE((Col>>2) & b11111000);
            pPix[2]=BYTE((Col>>7) & b11111000);
            pPix+=3;
          }
          pSur=LPWORD(LPBYTE(pSur)-SurLineLen)-w;
        }
      }
      else 
      {
        while(LPBYTE(pSur)>=SurMem) {
          pSurLineEnd=pSur+w;
          for(;pSur<pSurLineEnd;pSur++) 
          {
            Col=*pSur;
            pPix[0]=BYTE((Col<<3) & b11111000);
            pPix[1]=BYTE((Col>>3) & b11111100);
            pPix[2]=BYTE((Col>>8) & b11111000);
            pPix+=3;
          }
          pSur=LPWORD(LPBYTE(pSur)-SurLineLen)-w;
        }
      }
      break;
    }
    case 3:
    {
      long WidBytes=(w*3+3) & -4;
      BYTE *pSur=SurMem+((h-1)*SurLineLen);
      while(pSur>=SurMem) {
        memcpy(pPix,pSur,WidBytes);
        pSur-=SurLineLen;
        pPix+=WidBytes;
      }
      break;
    }
#endif
    case 4:
    {
      DWORD *pSur=LPDWORD(SurMem+((h-1)*SurLineLen)),*pSurLineEnd;
      if(rgb32_bluestart_bit) {
        while(LPBYTE(pSur)>=SurMem) {
          pSurLineEnd=pSur+w;
          for(;pSur<pSurLineEnd;pSur++) 
          {
            *LPDWORD(pPix)=(*pSur)>>rgb32_bluestart_bit;
            pPix+=3;
          }
          pSur=LPDWORD(LPBYTE(pSur)-SurLineLen)-w;
        }
      }
      else 
      {
        while(LPBYTE(pSur)>=SurMem) {
          pSurLineEnd=pSur+w;
          for(;pSur<pSurLineEnd;pSur++) 
          {
            *LPDWORD(pPix)=*pSur;
            pPix+=3;
          }
          pSur=LPDWORD(LPBYTE(pSur)-SurLineLen)-w;
        }
      }
      break;
    }
    default:
      break;
    }
  }
  if(!ToClipboard) // save
  {
#ifdef WIN32
#if !defined(SSE_VID_NO_FREEIMAGE)
    if(hFreeImage) 
    {
      FIBITMAP *FIBmp;
      //ASSERT(ConvertPixels!=0); //right?
      {
        FIBmp=FreeImage_ConvertFromRawBits(Pixels,w,h,w*3,24,0xff0000,
          0x00ff00,0x0000ff,false); //flip pic
      }
      TRACE_LOG("Save screenshot %s %dx%d opts %d FreeImage\n",ShotFile.Text,w,h,ScreenShotFormatOpts);
      FreeImage_Save((FREE_IMAGE_FORMAT)ScreenShotFormat,FIBmp,ShotFile,ScreenShotFormatOpts);
      FreeImage_Free(FIBmp);
    }
    else
#endif//#if !defined(SSE_VID_NO_FREEIMAGE)
#endif
    {
      BITMAPINFOHEADER bih;
      bih.biSize=sizeof(BITMAPINFOHEADER);
      bih.biWidth=w;
      bih.biHeight=h;
      WIN_ONLY(	bih.biPlanes=1; )
      WIN_ONLY(	bih.biBitCount=24; )
      UNIX_ONLY( bih.biPlanes_biBitCount=MAKELONG(1,24); )
      bih.biCompression=0 /*BI_RGB*/;
      bih.biSizeImage=0;
      bih.biXPelsPerMeter=0;
      bih.biYPelsPerMeter=0;
      bih.biClrUsed=0;
      bih.biClrImportant=0;
      FILE *f=fopen(ShotFile,"wb");
      if(f)
      {
        // File header
        WORD bfType=19778; //'BM';
        DWORD bfSize=14 /*sizeof(BITMAPFILEHEADER)*/ + sizeof(BITMAPINFOHEADER)+(w*h*3);
        WORD bfReserved1=0;
        WORD bfReserved2=0;
        DWORD bfOffBits=14 /*sizeof(BITMAPFILEHEADER)*/ + sizeof(BITMAPINFOHEADER);
        fwrite(&bfType,sizeof(bfType),1,f);
        fwrite(&bfSize,sizeof(bfSize),1,f);
        fwrite(&bfReserved1,sizeof(bfReserved1),1,f);
        fwrite(&bfReserved2,sizeof(bfReserved2),1,f);
        fwrite(&bfOffBits,sizeof(bfOffBits),1,f);
        fflush(f);
        fwrite(&bih,sizeof(bih),1,f);
        fflush(f);
        fwrite(Pixels,w*h*3,1,f);
        fflush(f);
        fclose(f);
      }
    }
  }
#ifdef WIN32
  switch(Method) {
#if defined(SSE_VID_D3D)
  case DISPMETHOD_D3D:
    if(ToClipboard==0) 
      SaveSur->UnlockRect();
    SaveSur->Release();
    break;
#endif
#if defined(SSE_VID_DD)
  case DISPMETHOD_DD:
    if(ToClipboard==0) 
      SaveSur->Unlock(NULL);
    SaveSur->Release();
    break;
#endif
#ifdef WIN32
  case DISPMETHOD_GDI:
    delete[] SurMem;
#endif
#ifdef STEEM_CRT
  case DISPMETHOD_CRT:
    delete[] SurMem;
#endif
  }//sw
  if(SaveBmp) 
    DeleteObject(SaveBmp);
#endif
  if(ConvertPixels) 
    delete[] Pixels;
  return DD_OK;
}

//#pragma warning (default: 4701)


void draw_init_resdependent() {
  if(draw_grille_black<4) 
    draw_grille_black=4;
  make_palette_table(Brightness,Contrast);
  palette_convert_all();
#if !defined(SSE_VID_32BIT_ONLY)
  if(BytesPerPixel==1) 
    palette_copy();
#endif
  if(osd_plasma_pal) 
  {
    delete[] osd_plasma_pal; osd_plasma_pal=NULL;
    delete[] osd_plasma;     osd_plasma=NULL;
  }
}


#ifdef WIN32

#if 0 && !defined(SSE_VID_NO_FREEIMAGE)

bool TSteemDisplay::ScreenShotIsFreeImageAvailable() {
  if(hFreeImage)
    return true;
  return (SteemLoadLibrary(FREE_IMAGE_DLL)!=NULL); //so we load it anyway now
  /*
  Str Path;
  Path.SetLength(MAX_PATH);
  char *FilNam;
  if(SearchPath(NULL,FREE_IMAGE_DLL,NULL,MAX_PATH,Path.Text,&FilNam)>0) 
    return true;
  if(Exists(RunDir+"\\FreeImage\\" FREE_IMAGE_DLL)) 
    return true;
  if(Exists(RunDir+"\\FreeImage\\FreeImage\\" FREE_IMAGE_DLL)) 
    return true;
  return 0;
  */
}

#endif//#if !defined(SSE_VID_NO_FREEIMAGE)


void TSteemDisplay::ScreenShotGetFormats(EasyStringList *pSL) {
#if !defined(SSE_VID_NO_FREEIMAGE)
  //bool FIAvailable=ScreenShotIsFreeImageAvailable();
#endif
  pSL->Sort=eslNoSort;
  pSL->Add(T("To Clipboard"),IF_TOCLIPBOARD);
  pSL->Add("BMP",FIF_BMP);
#if !defined(SSE_VID_NO_FREEIMAGE)
  //if(FIAvailable) 
  if(SSEConfig.FreeImageDll)
  {
    pSL->Add("JPEG (.jpg)",FIF_JPEG);
    pSL->Add("PNG",FIF_PNG);
    pSL->Add("TARGA (.tga)",FIF_TARGA);
    pSL->Add("TIFF",FIF_TIFF);
    pSL->Add("PBM",FIF_PBM);
    pSL->Add("PGM",FIF_PGM);
    pSL->Add("PPM",FIF_PPM);
  }
#endif//#if !defined(SSE_VID_NO_FREEIMAGE)
#if defined(SSE_VID_D3D)
#if !defined(SSE_VID_NO_FREEIMAGE)
  else
#endif
  {
    pSL->Add("JPEG (.jpg)",FIF_JPEG);
    pSL->Add("PNG",FIF_PNG);
  }
#endif
  pSL->Add("NEO",IF_NEO);
}


#if !defined(SSE_VID_NO_FREEIMAGE)

void TSteemDisplay::ScreenShotGetFormatOpts(EasyStringList *pSL) {
  pSL->Sort=eslNoSort;
  switch(ScreenShotFormat) {
  case FIF_BMP:
    if(SSEConfig.FreeImageDll)
    {
      pSL->Add(T("Normal"),BMP_DEFAULT);
      pSL->Add("RLE",BMP_SAVE_RLE);
    }
    break;
  case FIF_JPEG:
    pSL->Add(T("Superb Quality"),JPEG_QUALITYSUPERB);
    pSL->Add(T("Good Quality"),JPEG_QUALITYGOOD);
    pSL->Add(T("Normal"),JPEG_QUALITYNORMAL);
    pSL->Add(T("Average Quality"),JPEG_QUALITYAVERAGE);
    pSL->Add(T("Bad Quality"),JPEG_QUALITYBAD);
    break;
  case FIF_PBM:case FIF_PGM:case FIF_PPM:
    pSL->Add(T("Binary"),PNM_SAVE_RAW);
    pSL->Add("ASCII",PNM_SAVE_ASCII);
    break;
  }
}

#endif//#if !defined(SSE_VID_NO_FREEIMAGE)


#if defined(SSE_VID_DD_3BUFFER_WIN)
/*  When the option is on, this function is called a lot
    during emulation (each scanline) and during VBL idle
    times too, so the processor is always busy. TODO
    Scrolling is also sketchy, triple buffering only removes
    tearing.
*/

BOOL TSteemDisplay::BlitIfVBlank() {
  BOOL Blanking=FALSE;  
  if(Disp.DDObj && ACT-Disp.VSyncTiming>80000-60000) // avoid bursts
  {
    Disp.DDObj->GetVerticalBlankStatus(&Blanking);
    if(Blanking)
    {
      Disp.VSyncTiming=ACT;
      draw_blit();
    }
  }
  return Blanking;
}

#endif


// TODO
int TSteemDisplay::STXPixels() {
  int st_x_pixels=320+(border!=0)*SideBorderSizeWin*2; //displayed
  if(screen_res || video_mixed_output
#if defined(SSE_VID_STVL1)
    || OPTION_C3 && COLOUR_MONITOR 
#endif
    )
    st_x_pixels*=2;
  return st_x_pixels;
}


int TSteemDisplay::STYPixels() {
  int st_y_pixels=200+(border!=0)*(BORDER_TOP+BORDER_BOTTOM);
  if(screen_res || video_mixed_output
#if defined(SSE_VID_STVL1)
    || OPTION_C3 && screen_res<2 && (draw_win_mode[screen_res]==DWM_NOSTRETCH)
#endif
    )
    st_y_pixels*=2;
  return st_y_pixels;
}


#if defined(SSE_VID_D3D)
/*  Direct3D9 support

    D3D support was introduced in v3.7.0, only for fullscreen.

    As of v3.8.2 there are two separate builds for DirectDraw support
    and for Direct3D support, both windowed and fullscreen modes.
    This makes options (a little) simpler.

    We use DirectX9 and the ID3DXSprite interface.

    Notice there's 64bit support for DirectDraw, strange for a deprecated library.
 */

#if defined(SSE_VID_D3D)
#ifdef BCC_BUILD
// yes sir, the old BCC5.5 will build the D3D9 version too
#pragma comment(lib, "../../3rdparty/d3d/bcc/d3d9.lib")
#pragma comment(lib, "../../3rdparty/d3d/bcc/d3dx9_43.lib")
#pragma message D3D DIRECT3D_VERSION
#endif

#if _MSC_VER == 1200 // VC6 -  also for this dinosaur
#define D3D_DISABLE_9EX
#pragma comment(lib, "../../3rdparty/d3d/d3d9.lib")
#pragma comment(lib, "../../3rdparty/d3d/d3dx9.lib")
#endif

#if _MSC_VER >= 1500
// d3d9.lib d3dx9d.lib
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "D3dx9.lib")
#endif

#endif


inline bool TSteemDisplay::D3DBlit() {
  HRESULT d3derr=E_FAIL;
  if(pD3DDevice && pD3DSprite)
  {
    RECT dest;
    HCURSOR OldCur=NULL;
    if(!FullScreen)
    {
      if(stem_mousemode==STEM_MOUSEMODE_DISABLED && BlitHideMouse)
        OldCur=SetCursor(NULL);
      GetClientRect(StemWin,&dest);
      dest.top+=MENUHEIGHT;
      dest.right-=4;dest.bottom-=4;
      POINT pt={2,2};
      OffsetRect(&dest,pt.x,pt.y);
    }
    d3derr=pD3DDevice->BeginScene();
    if(FullScreen && runstate==RUNSTATE_RUNNING) 
      pD3DDevice->Clear(0,NULL,D3DCLEAR_TARGET,0,0,0); //problem: not the backbuffer
    if(!d3derr)
      d3derr=pD3DSprite->Begin(0); // the picture is one big sprite
    if(!d3derr&&OPTION_CRISP_FULLSCREEN)
      pD3DDevice->SetSamplerState(0,D3DSAMP_MAGFILTER ,D3DTEXF_POINT); //v3.7.2
    if(!d3derr)
      d3derr=pD3DSprite->Draw(pD3DTexture,&draw_blit_source_rect,NULL,NULL,0xFFFFFFFF);
    if(!d3derr)
      d3derr=pD3DSprite->End();
    if(!d3derr)
      d3derr=pD3DDevice->EndScene();
#if defined(SSE_VID_D3D_FLIPEX) // was a test
    if(!d3derr && d3dpp.SwapEffect==D3DSWAPEFFECT_FLIPEX)
    {
      // we must blit 1:1 from backbuffer to client area! 
      // -> SSE_GUI_MENU, SSE_GUI_TOOLBAR mods
      d3derr=pD3DDevice->PresentEx(NULL,NULL,NULL,NULL,(OPTION_WIN_VSYNC 
        ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_FORCEIMMEDIATE));
      if(stem_mousemode==STEM_MOUSEMODE_DISABLED && BlitHideMouse)
        SetCursor(OldCur);
    }
    else if(!FullScreen)
    {
      d3derr=pD3DDevice->Present(NULL,NULL,NULL,NULL);
      if(stem_mousemode==STEM_MOUSEMODE_DISABLED && BlitHideMouse)
        SetCursor(OldCur);
    }
    else
      d3derr=pD3DDevice->Present(NULL,NULL,NULL,NULL);
#elif defined(SSE_VID_D3D_SWAPCHAIN) // was a test
    if(pSwapChain)
    {
      if(!FullScreen)
      {
        do {
          d3derr=pSwapChain->Present(&draw_blit_source_rect,&dest,0,NULL,
            //D3DPRESENT_FORCEIMMEDIATE);
            D3DPRESENT_DONOTWAIT);
        } while(0);
          //d3derr==D3DERR_WASSTILLDRAWING);
      }
      else
      {
        d3derr=pSwapChain->Present(NULL,NULL,0,NULL,D3DPRESENT_DONOTWAIT);
      }
    }
#else
#if defined(SSE_EMU_THREAD) // we redo the test, could have changed since Blit()!
    if(d3derr || SuspendRendering || VideoLock.blocked)
      ;
#else
    if(d3derr)
      ;
#endif
    else if(!FullScreen)
    {
      d3derr=pD3DDevice->Present(&draw_blit_source_rect,&dest,NULL,NULL);
      if(stem_mousemode==STEM_MOUSEMODE_DISABLED && BlitHideMouse)
        SetCursor(OldCur);
    }
    else
    {
      //TRACE("blit "); TRACE_RECT(draw_blit_source_rect); 
      d3derr=pD3DDevice->Present(NULL,NULL,NULL,NULL);
    }
#endif
  }
  //ASSERT(d3derr != D3DERR_WASSTILLDRAWING);
  if(d3derr && d3derr!=D3DERR_WASSTILLDRAWING) 
  {
    TRACE2("BLIT ERROR\n");
    REPORT_D3D_ERR("Blit",d3derr);
    if(StatusInfo.MessageIndex!=TStatusInfo::BLIT_ERROR)
    {
      StatusInfo.MessageIndex=TStatusInfo::BLIT_ERROR;
      REFRESH_STATUS_BAR;
    }
  }
  else if(!d3derr && StatusInfo.MessageIndex==TStatusInfo::BLIT_ERROR)
  {
    StatusInfo.MessageIndex=TStatusInfo::MESSAGE_NONE;
    REFRESH_STATUS_BAR;
  }
  return !d3derr;
}


HRESULT check_device_type(D3DDEVTYPE DeviceType,D3DFORMAT DisplayFormat) {
// local helper for D3DInit()
  HRESULT d3derr;
  d3derr=Disp.pD3D->CheckDeviceType(Disp.m_Adapter,DeviceType,DisplayFormat,
    DisplayFormat,false);
  return d3derr;
}


HRESULT TSteemDisplay::D3DCreateSurfaces() {
  HRESULT d3derr=(pD3D)?D3D_OK:~D3D_OK;
  if(d3derr==D3D_OK)
    D3DDestroySurfaces();
  ZeroMemory(&d3dpp, sizeof(d3dpp));
  d3dpp.Windowed=!FullScreen;
#if defined(SSE_VID_D3D_FLIPEX)
  if(OPTION_FLIPEX&&(!FullScreen||OPTION_FAKE_FULLSCREEN))
  {
    d3dpp.SwapEffect=D3DSWAPEFFECT_FLIPEX;
    d3dpp.Flags=D3DPRESENT_INTERVAL_DEFAULT;
  }
  else
  {
    d3dpp.Flags=(OPTION_WIN_VSYNC) ? D3DPRESENT_INTERVAL_ONE 
      : D3DPRESENT_INTERVAL_IMMEDIATE; // imm. doesn't work for me
    d3dpp.SwapEffect=D3DSWAPEFFECT_DISCARD;
  }
#else
  d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; // recommended by Microsoft
  d3dpp.Flags=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
  if(FullScreen&&FSDoVsync||(!FullScreen||OPTION_FAKE_FULLSCREEN)
    &&OPTION_WIN_VSYNC)
    d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
  else
    d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
#endif
  d3dpp.hDeviceWindow=StemWin;
  UINT Width=monitor_width; // default
  UINT Height=monitor_height;
#if defined(SSE_VID_D3D_FLIPEX)
  d3dpp.BackBufferCount=(d3dpp.SwapEffect==D3DSWAPEFFECT_FLIPEX
    || OPTION_3BUFFER_FS&&FullScreen) ? 2 : 1;
#else
  d3dpp.BackBufferCount=(OPTION_3BUFFER_FS&&FullScreen)?2:1; // as simple as this
#endif
#if defined(SSE_VID_2SCREENS)
  if(d3derr==D3D_OK)
    CheckCurrentMonitorConfig(); // Update monitor rectangle
#endif
  if(pD3D  && d3derr==D3D_OK && FullScreen && !OPTION_FAKE_FULLSCREEN)
  {
    D3DDISPLAYMODE Mode; 
    d3derr=pD3D->EnumAdapterModes(m_Adapter,m_DisplayFormat,D3DMode,&Mode);
    TRACE_LOG("D3D adapter %d mode %d %dx%d %dHz format %d ERR %d\n",m_Adapter,
      D3DMode,Mode.Width,Mode.Height,Mode.RefreshRate,Mode.Format,d3derr);
    d3dpp.BackBufferFormat=Mode.Format;
    d3dpp.FullScreen_RefreshRateInHz=(OPTION_FULLSCREEN_DEFAULT_HZ)
      ? 0 : Mode.RefreshRate;
    d3dpp.BackBufferWidth=Width=Mode.Width;
    d3dpp.BackBufferHeight=Height=Mode.Height;
  }
  else
  {
    d3dpp.BackBufferFormat=m_DisplayFormat; //D3DFMT_UNKNOWN
#if !defined(SSE_VID_32BIT_ONLY)
    BytesPerPixel=4;
#endif
  }
  if(d3derr!=D3D_OK)
    ;
  else if(!FullScreen)
  {
    if(extended_monitor)
    {
      Width=GetScreenWidth();
      Height=GetScreenHeight();
    }
#if defined(SSE_VID_D3D_FLIPEX)
    else if(border)
#else
    else if(Disp.BorderPossible())
#endif
    { //SS: GetScreenWidth()>640
      Width=640+4*SideBorderSizeWin;
      Height=400+2*(BORDER_TOP+BottomBorderSize);
    }
    else
    {
      Width=640;
#if defined(SSE_VID_D3D_FLIPEX)
      Height=400;
#else
      Height=480;
#endif
    }
#if defined(SSE_VID_D3D_FLIPEX) // smaller windows
    if(!extended_monitor && !FullScreen && d3dpp.SwapEffect==D3DSWAPEFFECT_FLIPEX)
    {
      if(screen_res<2)
      {
        if(!WinSizeForRes[screen_res])
          Height/=2;
        if(!screen_res && !WinSizeForRes[screen_res])
          Width/=2;
      }
    }
#endif
    if(draw_blit_source_rect.right>=int(Width))
      draw_blit_source_rect.right=int(Width)-1;
    if(draw_blit_source_rect.bottom>=int(Height))
      draw_blit_source_rect.bottom=int(Height)-1;
    SurfaceWidth=d3dpp.BackBufferWidth=Width;
    SurfaceHeight=d3dpp.BackBufferHeight=Height;
  }
#if defined(SSE_VID_D3D_FAKE_FULLSCREEN)
/*  Create a borderless window instead of a fullscreen surface.
    Apparently there's not more to it than this.
*/
  else if(FullScreen && OPTION_FAKE_FULLSCREEN)
  {
//    ASSERT(d3dpp.FullScreen_RefreshRateInHz==0);
#if defined(SSE_VID_D3D_FLIPEX)
    if(d3dpp.SwapEffect!=D3DSWAPEFFECT_FLIPEX)
#endif
      d3dpp.BackBufferCount=1;
    d3dpp.Windowed=true;
    SurfaceWidth=d3dpp.BackBufferWidth=monitor_width;
    SurfaceHeight=d3dpp.BackBufferHeight=monitor_height;
  }
#endif
#if defined(SSE_VID_D3D_FLIPEX)
  D3DDISPLAYMODEEX FullscreenDisplayMode;
  D3DDISPLAYMODEEX *pFullscreenDisplayMode;
  if(FullScreen && !OPTION_FAKE_FULLSCREEN)
  {
    ZeroMemory(&FullscreenDisplayMode,sizeof(FullscreenDisplayMode));
    FullscreenDisplayMode.Size=sizeof(FullscreenDisplayMode);
    FullscreenDisplayMode.Format=d3dpp.BackBufferFormat;
    FullscreenDisplayMode.RefreshRate=d3dpp.FullScreen_RefreshRateInHz;
    FullscreenDisplayMode.Width=d3dpp.BackBufferWidth;
    FullscreenDisplayMode.Height=d3dpp.BackBufferHeight;
    FullscreenDisplayMode.ScanLineOrdering=D3DSCANLINEORDERING_PROGRESSIVE;
    pFullscreenDisplayMode=&FullscreenDisplayMode;
  }
  else
    pFullscreenDisplayMode=NULL;
#endif
  if(d3derr!=D3D_OK)
    pD3DDevice=NULL;
  else
  {
#if defined(SSE_VID_D3D_FLIPEX)
    d3derr=pD3D->CreateDeviceEx(m_Adapter,m_DeviceType,StemWin,m_vtx_proc,
      &d3dpp,pFullscreenDisplayMode,&pD3DDevice);
#else
    d3derr=pD3D->CreateDevice(m_Adapter,m_DeviceType,StemWin,m_vtx_proc,
      &d3dpp,&pD3DDevice);
#endif
#ifdef SSE_DEBUG
    if(!d3dpp.Windowed)
      TRACE_LOG("D3D Create fullscreen surface %dx%d screen %d format %d bpp %d buffers %d %dHz flags %X effect %X err %d\n",
      d3dpp.BackBufferWidth,d3dpp.BackBufferHeight,m_Adapter,d3dpp.BackBufferFormat,
      BytesPerPixel,d3dpp.BackBufferCount,d3dpp.FullScreen_RefreshRateInHz,d3dpp.Flags,d3dpp.SwapEffect,d3derr);
    else
      TRACE_LOG("D3D Create windowed surface %dx%d screen %d format %d bpp %d flags %X effect %X err %d\n",
      d3dpp.BackBufferWidth,d3dpp.BackBufferHeight,m_Adapter,d3dpp.BackBufferFormat,
      BytesPerPixel,d3dpp.Flags,d3dpp.SwapEffect,d3derr);
#else
    // release trace 
    TRACE2("scr %d fmt %d bpp %d flg %X fx %X FS%d W%d %dx%dx%d %dHz ERR %d\n",
      m_Adapter,d3dpp.BackBufferFormat,BytesPerPixel,d3dpp.Flags,d3dpp.SwapEffect,FullScreen,
      d3dpp.Windowed,d3dpp.BackBufferWidth,d3dpp.BackBufferHeight,d3dpp.BackBufferCount,
      d3dpp.FullScreen_RefreshRateInHz,d3derr);
#endif
    if(d3derr)
    {
      REPORT_D3D_ERR("CreateSurfaces",d3derr);
#if defined(SSE_VID_D3D_FLIPEX)
      if(d3dpp.SwapEffect==D3DSWAPEFFECT_FLIPEX)
      {
        if(OPTION_WARNINGS)
          Alert(T("FlipEx is not supported on this computer"),T("Warning"),
          MB_ICONEXCLAMATION);
        OPTION_FLIPEX=FALSE;
        PostMessage(GetDlgItem(OptionBox.Handle,IDC_FLIPEX),BM_SETCHECK,0,0);
      }
#endif
    }
  }
#if defined(SSE_VID_2SCREENS)
  if(!d3derr && FullScreen)
  {
    // Compute size
    LONG cw=rcMonitor.right-rcMonitor.left;
    LONG ch=rcMonitor.bottom-rcMonitor.top;
    // Update window in absolute coordinates (depend on which screen we're on)
    SetWindowPos(StemWin,HWND_TOPMOST,rcMonitor.left,rcMonitor.top,
      cw,ch,SWP_FRAMECHANGED);
    //TRACE_LOG("SetWindowPos %d %d %d %d\n",rcMonitor.left,rcMonitor.top,cw,ch);
    // Need this message to position icons and SetWindowPos won't trigger it!
    LPARAM lpar=cw+(ch<<16);
    PostMessage(StemWin,WM_SIZE,0,lpar); // (dubious Windows programming)
    InvalidateRect(StemWin,NULL,FALSE); // Redraw menu bar
  }
#endif
#if defined(SSE_VID_D3D_SWAPCHAIN)
  if(d3derr==D3D_OK)
    d3derr = pD3DDevice->GetSwapChain(0, &pSwapChain);
  if(d3derr==D3D_OK)
    d3derr=pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
#endif
  // Create texture
  if(d3derr==D3D_OK)
    d3derr=pD3DDevice->CreateTexture(Width,Height,1,D3DUSAGE_DYNAMIC,
    d3dpp.BackBufferFormat,D3DPOOL_DEFAULT,&pD3DTexture,NULL);
  if(d3derr==D3D_OK && FullScreen)
  {
    SurfaceWidth=d3dpp.BackBufferWidth;
    SurfaceHeight=d3dpp.BackBufferHeight;
/*  We must call this once at surface creation time and use "discard" blit
    effect and the GUI will not fail to appear anymore in older OS (maybe).
*/
    if(OPTION_FULLSCREEN_GUI) 
      pD3DDevice->SetDialogBoxMode(TRUE);
  }
  if(d3derr==D3D_OK)
    d3derr=D3DSpriteInit();
  if(d3derr==D3D_OK && StatusInfo.MessageIndex==TStatusInfo::BLIT_ERROR)
    StatusInfo.MessageIndex=TStatusInfo::MESSAGE_NONE;
  if(d3derr==D3D_OK)
  {
    draw_mem=NULL;
    Lock();
    if(draw_mem)
      ZeroMemory(draw_mem,Disp.VideoMemorySize);
    Unlock();
    draw_init_resdependent();
    palette_prepare(
#if !defined(SSE_VID_32BIT_ONLY)      
      true
#endif
      );
  }
  if(d3derr!=D3D_OK)
  {
    TRACE2("BLIT ERROR\n");
    if(StatusInfo.MessageIndex!=TStatusInfo::BLIT_ERROR)
    {
      StatusInfo.MessageIndex=TStatusInfo::BLIT_ERROR;
      REFRESH_STATUS_BAR;
    }
    REPORT_D3D_ERR("CreateSurfaces",d3derr);
    FullScreen=0;
  }
  return d3derr;
}


VOID TSteemDisplay::D3DDestroySurfaces() {
  //TRACE_LOG("D3D destroy surfaces S %x T %x D %x\n",pD3DSprite,pD3DTexture,pD3DDevice);
  if(pD3D && pD3DDevice)
  {
    if(pD3DSprite)
    {
      pD3DSprite->Release();
      pD3DSprite=NULL;
    }
    if(pD3DTexture)
    {
      pD3DTexture->Release();
      pD3DTexture=NULL;
    }
#if defined(SSE_VID_D3D_SWAPCHAIN)
    if(pSwapChain)
    {
      pSwapChain->Release();
      pSwapChain=NULL;
    }
#endif
    pD3DDevice->Release();
    pD3DDevice = NULL;
  }
}

#undef LOGSECTION
#define LOGSECTION LOGSECTION_INIT


HRESULT TSteemDisplay::D3DInit() {
  SetNotifyInitText("DirectD3D");
  if(pD3D)
    pD3D->Release();
  // Create the D3D object - computer needs DirectX9
#if defined(SSE_VID_D3D_FLIPEX)
  if((Direct3DCreate9Ex(D3D_SDK_VERSION,&pD3D))!=S_OK)
#else
  if((pD3D=Direct3DCreate9(D3D_SDK_VERSION))==NULL)
#endif
  {
    TRACE_INIT("D3D9 Init Fail!\n");
    return E_FAIL; 
  }
  m_Adapter=D3DADAPTER_DEFAULT;
  // do it once, keeping result
#if defined(SSE_VID_2SCREENS)
  CheckCurrentMonitorConfig(); // could Steem be started on second monitor?
  // Probe capacities of video card, starting with desktop mode, HW
  // http://en.wikibooks.org/wiki/DirectX/9.0/Direct3D/Initialization
  m_DeviceType=D3DDEVTYPE_HAL; // first suppose good hardware
  HRESULT d3derr=check_device_type(m_DeviceType,m_DisplayFormat);
  if(d3derr) // could be "alpha" in desktop format?
  {
    REPORT_D3D_ERR("check_device_type1",d3derr);
    d3derr=check_device_type(m_DeviceType,D3DFMT_X8R8G8B8); // try X8R8G8B8 format
    if(d3derr) // no HW abilities?
    {
      REPORT_D3D_ERR("check_device_type2",d3derr);
      m_DeviceType=D3DDEVTYPE_REF; // try software processing (slow)
      d3derr=check_device_type(m_DeviceType,m_DisplayFormat);
      TRACE_INIT("D3D: poor hardware detected, software rendering ERR %d\n",d3derr);
    }
  }
  //ASSERT(!d3derr);
  D3DCAPS9 caps;
  d3derr=pD3D->GetDeviceCaps(m_Adapter,m_DeviceType,&caps);
  TRACE_INIT("DevCaps $%X HW quality %X intervals %X err %d\n",caps.DevCaps,caps.DevCaps&(D3DDEVCAPS_HWTRANSFORMANDLIGHT|D3DDEVCAPS_PUREDEVICE),caps.PresentationIntervals,d3derr);
  if(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) 
  {
    TRACE_INIT("T&L ");
    m_vtx_proc=D3DCREATE_HARDWARE_VERTEXPROCESSING;
    if(caps.DevCaps & D3DDEVCAPS_PUREDEVICE) 
    {
      TRACE_INIT("Pure device ");
      m_vtx_proc|=D3DCREATE_PUREDEVICE;
    }
  }
  else 
  {
    TRACE_INIT("Software vertex ");
    m_vtx_proc=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
  }
  if(DrawToVidMem==0)
    m_vtx_proc|=D3DDEVCAPS_EXECUTESYSTEMMEMORY|D3DDEVCAPS_TEXTURESYSTEMMEMORY;
  TRACE_INIT("vtx_proc = $%X\n",m_vtx_proc);

#else // BCC D3D

  UINT Adapter=D3DADAPTER_DEFAULT;
  // Get the current desktop display info
  D3DDISPLAYMODE d3ddm;
  HRESULT d3derr=pD3D->GetAdapterDisplayMode(Adapter, &d3ddm);
  HDC hdc = GetDC(StemWin);
  WORD bitsperpixel= GetDeviceCaps(hdc, BITSPIXEL); // another D3D shortcoming
  ReleaseDC(StemWin, hdc);
  TRACE_INIT("Screen %dx%d %dHz format %d %dbit err %d\n",d3ddm.Width,d3ddm.Height,d3ddm.RefreshRate,d3ddm.Format,bitsperpixel,d3derr);
  // Probe capacities of video card, starting with desktop mode, HW
  // http://en.wikibooks.org/wiki/DirectX/9.0/Direct3D/Initialization
  m_DisplayFormat=d3ddm.Format; //should never change
  D3DFORMAT checkDisplayFormat=m_DisplayFormat;
  m_DeviceType=D3DDEVTYPE_HAL;
  d3derr=check_device_type(m_DeviceType,checkDisplayFormat);
  if(d3derr) // could be "alpha" in desktop format?
  {
    REPORT_D3D_ERR("check_device_type1",d3derr);
    checkDisplayFormat=D3DFMT_X8R8G8B8; // try X8R8G8B8 format
    d3derr=check_device_type(m_DeviceType,checkDisplayFormat);
    if(d3derr) // no HW abilities?
    {
      REPORT_D3D_ERR("check_device_type2",d3derr);
      D3DDEVTYPE DeviceType=D3DDEVTYPE_REF; // try software processing (slow)
      d3derr=check_device_type(DeviceType,checkDisplayFormat);
      TRACE_INIT("D3D: poor hardware detected, software rendering ERR %d\n",d3derr);
    }
  }
  //ASSERT(!d3derr);
  D3DCAPS9 caps;
  d3derr=pD3D->GetDeviceCaps(Adapter,m_DeviceType,&caps);
//  TRACE_INIT("DevCaps $%X HW quality %X err %d\n",caps.DevCaps,caps.DevCaps&(D3DDEVCAPS_HWTRANSFORMANDLIGHT|D3DDEVCAPS_PUREDEVICE),d3derr);
  TRACE_INIT("DevCaps $%X HW quality %X intervals %X err %d\n",caps.DevCaps,caps.DevCaps&(D3DDEVCAPS_HWTRANSFORMANDLIGHT|D3DDEVCAPS_PUREDEVICE),caps.PresentationIntervals,d3derr);
  if( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) {
    TRACE_INIT("T&L\n");
    m_vtx_proc = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    if( caps.DevCaps & D3DDEVCAPS_PUREDEVICE ) {
      TRACE_INIT("Pure device\n");
      m_vtx_proc |= D3DCREATE_PUREDEVICE;
    }
  } else {
    TRACE_INIT("Software vertex\n");
    m_vtx_proc = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
  }
#if defined(SSE_VID_D3D) // not sure it makes much sense but...
  if(DrawToVidMem==0)
    m_vtx_proc|=D3DDEVCAPS_EXECUTESYSTEMMEMORY|D3DDEVCAPS_TEXTURESYSTEMMEMORY;
#endif
  TRACE_INIT("vtx_proc = $%X\n",m_vtx_proc);
#if defined(SSE_VID_32BIT_ONLY)
  ASSERT((bitsperpixel/8)==4);
#else
  BytesPerPixel= bitsperpixel/8; // Steem can do 8bit, 16bit, 32bit
#endif
#endif
  TRACE_INIT("D3D9 Init OK\n");
  return S_OK;
}

#undef LOGSECTION
#define LOGSECTION LOGSECTION_VIDEO_RENDERING


inline HRESULT TSteemDisplay::D3DLock() {
  HRESULT d3derr=E_FAIL;
  BYTE former_msg=StatusInfo.MessageIndex;
  // Restore surfaces after event such as screen saver
  if(StatusInfo.MessageIndex==TStatusInfo::BLIT_ERROR)
    d3derr=D3DCreateSurfaces(); 
  if(pD3DDevice&&pD3DTexture)
  {
    D3DLOCKED_RECT LockedRect;
#if defined(SSE_EMU_THREAD) // we redo the test, could have changed since Lock()!
    if(SuspendRendering || VideoLock.blocked)
      return d3derr;
#endif
    d3derr=pD3DTexture->LockRect(0,&LockedRect,NULL,0);
    //pBackBuffer->LockRect(&LockedRect,NULL,D3DLOCK_DISCARD);
    if(d3derr)
    {
      REPORT_D3D_ERR("LockRect",d3derr);
      //ASSERT(!draw_mem);
      draw_mem=NULL;
      StatusInfo.MessageIndex=TStatusInfo::BLIT_ERROR;
    }
    else
    {
      draw_line_length=LockedRect.Pitch;
      draw_mem=(BYTE*)LockedRect.pBits;
      // compute locked video memory as pitch * #lines
      VideoMemorySize=draw_line_length*SurfaceHeight;
      VideoMemoryEnd=draw_mem+VideoMemorySize;
    }
  }
  if(former_msg!=StatusInfo.MessageIndex)
    REFRESH_STATUS_BAR;
  return d3derr;
}


VOID TSteemDisplay::D3DRelease() {
  // called by void TSteemDisplay::Release()
  //TRACE_LOG("D3DRelease()\n");
  D3DDestroySurfaces(); // destroys texture, sprite, device
  if(pD3D)
  {
    pD3D->Release();
    pD3D=NULL;
    D3D9_OK=false;
  }
}


inline void TSteemDisplay::D3DUnlock() {
  HRESULT d3derr=E_FAIL;
#if defined(SSE_EMU_THREAD)
  if(!(SuspendRendering||VideoLock.blocked))
#endif
  if(pD3DDevice&&pD3DTexture)
    d3derr=pD3DTexture->UnlockRect(0);
  return;
}


HRESULT TSteemDisplay::D3DSpriteInit() {
  HRESULT hr=E_FAIL;
  if(!pD3D||!pD3DDevice)
    return hr;
  if(pD3DSprite)
    pD3DSprite->Release(); //so we can init sprite anytime
  hr=D3DXCreateSprite(pD3DDevice,&pD3DSprite); 
  if(!pD3DSprite)
    return hr;
/*  Use SetTransform to stretch & translate if needs be, restore if not.
    We will draw small on the big backbuffer and this will be stretched and
    translated by the hardware before rendering.
*/
  float sw=1,sh=1;
  float tx=0,ty=0;
  DWORD stx=STXPixels();
  DWORD sty=STYPixels();
  if(extended_monitor && em_width && em_height)
  {
    stx=em_width;
    sty=em_height;
  }
/*  Imitating a feature first seen in SainT, the screen is higher than it
    should, so that circles aren't perfect, squares are rectangles, etc.
    Note: you could use some settings on your monitor to cancel this effect,
    but many magazine screenshots show it.
    On an American display, 60hz, the picture is better, so Atari wasn't really
    aware of the problem.
    Some references:
    TV AR 4:3 = 1.333
    PAL 720:576 = 1.25
    NTSC 720:480 = 1.5
    ST AR LORES
      320:200 = 1.6 (useful picture)
      416:281 = 1.480 (plasma)
*/
  if(OPTION_ST_ASPECT_RATIO && screen_res<2)
    sty=(int)((float)sty*ST_ASPECT_RATIO_DISTORTION); // "reserve" more pixels
  TRACE_LOG("D3D Sprite STX %d STY %d Surface W %d H %d res %d mixed %d 16/9 %d\n",
    stx,sty,SurfaceWidth,SurfaceHeight,screen_res,video_mixed_output,
    (SurfaceWidth>(stx*SurfaceHeight)/sty));
  if(FullScreen)
  {
    if(SurfaceWidth>(stx*SurfaceHeight)/sty) // "16:9"
    {
      sw=sh=(float)SurfaceHeight/sty;
      tx=((float)SurfaceWidth-(float)SurfaceHeight*stx/sty)/2;
    }
    else // "4:3"
    {
      sh=sw= (float)SurfaceWidth/stx;
      ty=((float)SurfaceHeight-(float)SurfaceWidth*sty/stx)/2;
    }
    //  Trade-off, we sacrifice some screen space to have better proportions.
    if(OPTION_CRISP_FULLSCREEN)
    {
      sw=sh=(float)(int)sw; // no artefacts
      tx= ((float)SurfaceWidth-(sw*stx))/2; // correct translation (on my 4:3 anyway)
      ty= ((float)SurfaceHeight-(sw*sty))/2;
    }
    //ASSERT(FullScreen);
    if(OPTION_C3 && (screen_res>=2 
      || screen_res==1 && draw_win_mode[screen_res]!=DWM_NOSTRETCH) //WinSizeForRes[screen_res]>=1)
      ||!OPTION_C3&&(screen_res>=1||video_mixed_output))
      if(!extended_monitor)
        sh*=2; // double # lines
  }
  if(OPTION_ST_ASPECT_RATIO && FullScreen && screen_res<2)
      sh*=ST_ASPECT_RATIO_DISTORTION; // stretch more
  // resolution adjustments, a bit ad hoc for now...
  if(FullScreen && !extended_monitor && (screen_res>=2 || SCANLINES_OK
    && (!OPTION_C3/*||draw_win_mode[screen_res]==DWM_STRETCH*/) ))
  {
    sh/=2; // this may become a ratio...
  }
  if(extended_monitor && stx==SurfaceWidth)
    sw=sh=1;
  TRACE_LOG("Sprite sw %f tx %f sh %f ty %f\n",sw,tx,sh,ty);
  D3DMATRIX matrix= {
    sw,              0.0f,            0.0f,            0.0f,
    0.0f,            sh,              0.0f,            0.0f,
    0.0f,            0.0f,            1.0f,            0.0f,
    tx,              ty,              0.0f,            1.0f
  };
  pD3DSprite->SetTransform((D3DXMATRIX*)&matrix);
  if(pD3DDevice)
    pD3DDevice->Clear(0,0,D3DCLEAR_TARGET,0,0,0);
  return hr;
}

  
void TSteemDisplay::D3DUpdateWH(UINT display_mode) {
  if(!pD3D)
    return;
  D3DDISPLAYMODE d3ddm;
  pD3D->GetAdapterDisplayMode(m_Adapter, &d3ddm);
  D3DDISPLAYMODE Mode; 
  pD3D->EnumAdapterModes(m_Adapter,d3ddm.Format,display_mode,&Mode);
  D3DFsW=Mode.Width;
  D3DFsH=Mode.Height;
  TRACE_LOG("D3DUpdateWH mode %d w %d h %d\n",display_mode,D3DFsW,D3DFsH);
}

#endif//d3d


#if defined(SSE_VID_2SCREENS)

void TSteemDisplay::CheckCurrentMonitorConfig(HMONITOR hCurrentMonitor) {
  if(!hCurrentMonitor)
    // Get Windows handle to monitor. This function requires Windows 2000.
    hCurrentMonitor = MonitorFromWindow(StemWin, MONITOR_DEFAULTTOPRIMARY);
  // Get and memorize monitor's Windows rectangle
  MONITORINFO myMonitorInfo;
  myMonitorInfo.cbSize = sizeof(myMonitorInfo);
  GetMonitorInfo(hCurrentMonitor, &myMonitorInfo);
  rcMonitor = myMonitorInfo.rcMonitor;
  TRACE_VID_R("rcMonitor "); TRACE_VID_RECT(rcMonitor);
#if defined(SSE_VID_D3D)
  if(pD3D)
  {
    // Determine current display and recreate surfaces if it's changed
    UINT n_monitors=pD3D->GetAdapterCount();
    for(UINT i=0;i<n_monitors;i++)
    {
      HMONITOR that_monitor_handle=pD3D->GetAdapterMonitor(i);
      if(that_monitor_handle==hCurrentMonitor)
      {
        if(i!=m_Adapter)
        {
          TRACE_LOG("Change D3D adapter to %d\n",i);
          m_Adapter=i;
          // Classy interface, change mode (2 max) and update fullscreen page
          UINT buf=oldD3DMode;
          oldD3DMode=D3DMode;
          D3DMode=buf;
          if(OptionBox.Handle && OptionBox.Page==3) 
          {
            OptionBox.DestroyCurrentPage();
            OptionBox.CreateFullscreenPage();
          }
          D3DCreateSurfaces();
        }
      }
    }
    // Get the current desktop display info
    D3DDISPLAYMODE d3ddm;
#ifdef SSE_DEBUG
    HRESULT d3derr=
#endif
      pD3D->GetAdapterDisplayMode(m_Adapter, &d3ddm);
    m_DisplayFormat=d3ddm.Format;
    HDC hdc = GetDC(StemWin);

#if defined(SSE_VID_32BIT_ONLY)
#ifdef SSE_DEBUG
    int bitsperpixel= GetDeviceCaps(hdc, BITSPIXEL);
    ASSERT((bitsperpixel/8)==4);
#endif
#else
    int bitsperpixel= GetDeviceCaps(hdc, BITSPIXEL);
    BytesPerPixel=(BYTE)(bitsperpixel/8); // Steem can do 8bit, 16bit, 32bit
#endif
    ReleaseDC(StemWin, hdc);
#ifdef SSE_DEBUG
    TRACE_LOG("Screen %d/%d handle %p %dx%d %dHz format %d %dbit err %d\n",
      m_Adapter,n_monitors,hCurrentMonitor,d3ddm.Width,d3ddm.Height,
      d3ddm.RefreshRate,d3ddm.Format,bitsperpixel,d3derr);
#endif
    monitor_width=d3ddm.Width;
    monitor_height=d3ddm.Height;
  }//pD3D
#endif
}

#endif

#endif//WIN32


#ifdef UNIX

bool TSteemDisplay::CheckDisplayMode(DWORD red_mask,DWORD green_mask,DWORD blue_mask)
{
  bool Valid=0;
  if (BytesPerPixel==1){
//    if (AlreadyWarnedOfBadMode==0){
//    	MessageBox(0,T("XSteem does not yet support 8-bit mode.  If you want us to add this feature write to us and let us know."),T("Display Error"),MB_ICONEXCLAMATION);
//    	AlreadyWarnedOfBadMode=true;
//    }
//    return 0;
    Valid=true;
  }else if (BytesPerPixel==2){
    if (blue_mask==   MAKEBINW(b00000000,b00011111)){
      if (green_mask==MAKEBINW(b00000011,b11100000) &&
          red_mask==  MAKEBINW(b01111100,b00000000)){
        rgb555=true;
        Valid=true;
      }else if (green_mask==MAKEBINW(b00000111,b11100000) &&
                red_mask==  MAKEBINW(b11111000,b00000000)){
        rgb555=0;
        Valid=true;
      }
    }
  }else if (BytesPerPixel<=4){
    rgb555=0;
    rgb32_bluestart_bit=0;
    Valid=(blue_mask==0x0000ff && green_mask==0x00ff00 && red_mask==0xff0000);
    if (BytesPerPixel==4 && Valid==0){
      if (blue_mask==0x0000ff00 && green_mask==0x00ff0000 && red_mask==0xff000000){
        Valid=true;
        rgb32_bluestart_bit=8;
      }
    }
  }
  if (Valid==0){
    if (AlreadyWarnedOfBadMode==0){
    	EasyStr Text=T("Sorry, your current screen mode is not supported by Steem.");
      if (BytesPerPixel<=4){
      	Text+="\n\n";
        Text+=T("If you want you can e-mail us with the below text and we'll consider adding support for it:")+"\n\n";

        EasyStr Bin;
        for (int n=0;n<BytesPerPixel*8;n++){
          char c='0';
          if ((red_mask >> n) & 1) c='R';
          if ((green_mask >> n) & 1) c='G';
          if ((blue_mask >> n) & 1) c='B';
          Bin.Insert(c,0);
        }
        Text+=Bin;
      }
      MessageBox(0,Text,T("Display Error"),MB_ICONINFORMATION);
    	AlreadyWarnedOfBadMode=true;
    }
    return 0;
  }
  return true;
}


bool TSteemDisplay::InitX() 
{
  if (XD==NULL) return 0;
//  TRACE("SteemDisplay::InitX()\n");
  Release();

  int Scr=XDefaultScreen(XD);
  int w=640,h=480;
  if (Disp.BorderPossible()){
#if defined(SSE_VID_BORDERS) && defined(SSE_UNIX)
    w=640+4* (SideBorderSize); // 768 or 800 or 832
    h=400+2*(BORDER_TOP+BottomBorderSize);
#else
    w=768;h=400+2*(BORDER_TOP+BORDER_BOTTOM);
#endif
  }
#ifndef NO_CRAZY_MONITOR
  if(extended_monitor){
    w=GetScreenWidth();
    h=GetScreenHeight();
  }
#endif

  int Depth=XDefaultDepth(XD,Scr);

  BytesPerPixel=(Depth+7)/8;
  if (Depth>=24) BytesPerPixel=4;

  char *ImgMem=(char*)malloc(w*h*BytesPerPixel);
  X_Img=XCreateImage(XD,XDefaultVisual(XD,Scr),
                      Depth,ZPixmap,0,ImgMem,
                      w,h,BytesPerPixel*8,0);
  if (X_Img){
    BytesPerPixel=(X_Img->bits_per_pixel+7)/8;
    if (CheckDisplayMode(X_Img->red_mask,X_Img->green_mask,X_Img->blue_mask)==0){
      Release();
      return 0;
    }
  }else{
    free(ImgMem);
    MessageBox(0,T("Couldn't create XImage."),T("Display Error"),MB_ICONINFORMATION);
    return 0;
  }

  SurfaceWidth=w;
  SurfaceHeight=h;
  draw_init_resdependent();
  palette_prepare(true);

  return true;
}
//---------------------------------------------------------------------------
#ifndef NO_SHM
_XFUNCPROTOBEGIN
int XShmGetEventBase(
#if NeedFunctionPrototypes
Display *
#endif
);
_XFUNCPROTOEND
#endif

bool TSteemDisplay::InitXSHM()
{
// SS SHM = shared memory; this is the general case, more performant
#ifdef NO_SHM
  return 0;
#else
//  TRACE("SteemDisplay::InitXSHM()\n");
  if (XD==NULL) return 0;

  Release();

  if (XShmQueryExtension(XD)==0){
    MessageBox(0,T("MIT shared memory extension not available."),T("SHM Error"),MB_ICONINFORMATION | MB_OK);
    return 0;
  }

  int Scr=XDefaultScreen(XD);
  int w=640,h=480;
  if (BorderPossible()){
#if defined(SSE_VID_BORDERS) && defined(SSE_UNIX)
    w=640+4* (SideBorderSize); // we draw larger
    h=400+2*(BORDER_TOP+BottomBorderSize);
#else
    w=768;h=400+2*(BORDER_TOP+BORDER_BOTTOM);
#endif
  }
#ifndef NO_CRAZY_MONITOR
  if(extended_monitor){
    w=GetScreenWidth();
    h=GetScreenHeight();
  }
#endif

  X_Img=XShmCreateImage(XD,XDefaultVisual(XD,Scr),
                 XDefaultDepth(XD,Scr),ZPixmap,NULL,&XSHM_Info,w,h);
  if (X_Img==NULL){
    MessageBox(0,T("Couldn't create shared memory XImage."),T("SHM Error"),MB_ICONINFORMATION);
    Release();return 0;
  }

  XSHM_Info.shmid=shmget(IPC_PRIVATE,X_Img->bytes_per_line*X_Img->height,IPC_CREAT | 0777);
  if (XSHM_Info.shmid==-1){
    MessageBox(0,T("Couldn't allocate shared memory."),T("SHM Error"),MB_ICONINFORMATION);
    Release();return 0;
  }

  XSHM_Info.shmaddr=(char*)shmat(XSHM_Info.shmid,0,0);
  if (XSHM_Info.shmaddr==(char*)-1){
    MessageBox(0,T("Couldn't attach shared memory."),T("SHM Error"),MB_ICONINFORMATION);
    Release();return 0;
  }
  X_Img->data=XSHM_Info.shmaddr;

  XSHM_Info.readOnly=0;
  if (XShmAttach(XD,&XSHM_Info)==0){
    MessageBox(0,T("The X server couldn't attach the shared memory."),T("SHM Error"),MB_ICONINFORMATION);
    Release();return 0;
  }
  XSHM_Attached=true;

  SHMCompletion=XShmGetEventBase(XD)+ShmCompletion;
//	SHMCompletion=65; //it is for us!

  BytesPerPixel=(X_Img->bits_per_pixel+7)/8;
  if (CheckDisplayMode(X_Img->red_mask,X_Img->green_mask,X_Img->blue_mask)==0){
    Release();return 0;
  }

//  printf(EasyStr("Bytes per pixel=")+BytesPerPixel+"  Depth="+XDefaultDepth(XD,Scr)+"\n");
  TRACE2("Bytes per pixel=%d Depth=%d\n",BytesPerPixel,XDefaultDepth(XD,Scr));
  SurfaceWidth=w;
  SurfaceHeight=h;
  draw_init_resdependent();
  palette_prepare(true);

  return true;
#endif
}

//---------------------------------------------------------------------------
void TSteemDisplay::Surround()
{
  if (FullScreen) return;

  XWindowAttributes wa;
  XGetWindowAttributes(XD,StemWin,&wa);

  int w=wa.width,h=wa.height-(MENUHEIGHT);

  int sw=draw_blit_source_rect.right;
  int sh=draw_blit_source_rect.bottom;
  int dx=(w-(sw+4))/2;
  int dy=(h-(sh+4))/2;
  int fx1=dx,fy1=dy,fx2=dx+sw+4,fy2=dy+sh+4;
  XSetForeground(XD,DispGC,BkCol);

  int bh=dy;
  if (h & 1) bh++;
  if (dy>0){ //draw grey border top and bottom
    XFillRectangle(XD,StemWin,DispGC,0,MENUHEIGHT,w,dy);
  }else{
    dy=0;
    fy1=0;
    fy2=h;
  }
  if (bh>0) XFillRectangle(XD,StemWin,DispGC,0,dy+sh+(MENUHEIGHT+4),w,bh);

  int rw=dx;
  if (w & 1) rw++;
  if (dx>0){ //draw grey border left and right
    XFillRectangle(XD,StemWin,DispGC,0,dy+(MENUHEIGHT),dx,sh+4);
  }else{
    fx1=0;
    fx2=w;
  }
  if (rw>0) XFillRectangle(XD,StemWin,DispGC,dx+sw+4,dy+(MENUHEIGHT),rw,sh+4);

  fy1+=MENUHEIGHT;fy2+=MENUHEIGHT;
  XSetForeground(XD,DispGC,BlackCol);
  XDrawLine(XD,StemWin,DispGC,fx1+1,fy1+1,fx2-1,fy1+1);
  XDrawLine(XD,StemWin,DispGC,fx1+1,fy1+2,fx1+1,fy2-1);
  XSetForeground(XD,DispGC,BorderDarkCol);
  XDrawLine(XD,StemWin,DispGC,fx1,fy1,fx2-1,fy1);
  XDrawLine(XD,StemWin,DispGC,fx1,fy1,fx1,fy2-1);
  XSetForeground(XD,DispGC,WhiteCol);
  XDrawLine(XD,StemWin,DispGC,fx1+1,fy2-1,fx2-1,fy2-1);
  XDrawLine(XD,StemWin,DispGC,fx2-1,fy1+1,fx2-1,fy2-1);
  XSetForeground(XD,DispGC,BorderLightCol);
  XDrawLine(XD,StemWin,DispGC,fx1+2,fy2-2,fx2-2,fy2-2);
  XDrawLine(XD,StemWin,DispGC,fx2-2,fy1+2,fx2-2,fy2-2);
}


//---------------------------------------------------------------------------
#ifndef NO_XVIDMODE
int TSteemDisplay::XVM_WinProc(void*,Window Win,XEvent *Ev)
{
#ifndef NO_SHM
  if (Ev->type==Disp.SHMCompletion){
    Disp.asynchronous_blit_in_progress=false;
    return PEEKED_MESSAGE;
  }
#endif
  switch (Ev->type){
    case Expose:
#if SSE_VERSION>=370
      draw_grille_black=MAX((int)draw_grille_black,50);
#else
      draw_grille_black=MAX(draw_grille_black,50);
#endif
      break;
    case ButtonPress: // For MMB
    case ButtonRelease:
    case KeyPress:
    case KeyRelease:
      return StemWinProc(NULL,StemWin,Ev);
    case FocusOut:
      runstate=RUNSTATE_STOPPING;
      break;
  }
  return PEEKED_MESSAGE;
}
#endif//!NO_XVIDMODE
//---------------------------------------------------------------------------

#endif//UNIX
