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
FILE: display.h
DESCRIPTION: Declarations for video output (blitting).
class TSteemDisplay
---------------------------------------------------------------------------*/

#pragma once
#ifndef SSEDISPLAY_H
#define SSEDISPLAY_H

#include <easystr.h>
#include <easystringlist.h>
#include <various/neochrome.h> // for NEO screenshots
#include "conditions.h"
#include "steemh.h"
#include "options.h"

#if defined(SSE_VID_DD)
#include <ddraw.h>
#endif

#if defined(SSE_VID_D3D)

#ifdef BCC_BUILD
// yes sir, the old BCC5.5 will build the new DX9 option too
#pragma comment(lib, "../../3rdparty/d3d/bcc/d3d9.lib")
#pragma comment(lib, "../../3rdparty/d3d/bcc/d3dx9_43.lib")
#define D3D_DISABLE_9EX
#define sqrtf (sqrt)  // so that it compiles, hopefully not used...
#endif

#if _MSC_VER == 1200 // VC6 -  also for this dinosaur
#define D3D_DISABLE_9EX
#pragma comment(lib, "../../3rdparty/d3d/d3d9.lib")
#pragma comment(lib, "../../3rdparty/d3d/d3dx9.lib")
#endif

#if _MSC_VER == 1200 || defined(MINGW_BUILD) //mingw TODO?
#include "d3d\sal.h"
#include "d3d\d3d9.h"
#include "d3d\d3dx9core.h"
#include "d3d\D3d9types.h"
#include "d3d\D3dx9math.h"
#elif _MSC_VER >= 1900 //VS2015 
#include "d3d\d3d9.h"
#include "d3d\d3dx9core.h"
#include "d3d\D3d9types.h"
#include "d3d\D3dx9math.h"
#else //bcc too?
#include "d3d9.h"
#include "d3dx9core.h"
#include "D3d9types.h"
#include "D3dx9math.h"
#endif

#endif//d3d


void draw_init_resdependent();

enum EDisplay {
  DFSM_FLIP=0,DFSM_STRAIGHTBLIT,DFSM_STRETCHBLIT,DFSM_FAKEFULLSCREEN,
  NPC_HZ_CHOICES=4,
  DFSFX_NONE=0,DFSFX_GRILLE,DFSFX_BLUR,
  DISPMETHOD_NONE=0,DISPMETHOD_DD,DISPMETHOD_D3D,DISPMETHOD_GDI,
  DISPMETHOD_X,DISPMETHOD_XSHM,DISPMETHOD_BE,NFSRES=40,
// 501x224 = 112224
// 8021247/112224 = 71.47532613344739
// but mega
// 8010612.5/112224 = 71.38056476333048
  MONO_HZ=71,
  NUM_HZ=6,
  DISP_MAX_FREQ_LEEWAY=5,
  IF_TOCLIPBOARD=0xfff0,IF_NEO=6
};


extern "C"
{
#if defined(SSE_VID_32BIT_ONLY)
extern const BYTE BytesPerPixel;
extern BYTE rgb32_bluestart_bit;
#else
extern BYTE BytesPerPixel,rgb32_bluestart_bit;
#endif
extern bool rgb555;
extern DWORD monitor_width,monitor_height; //true size of monitor, for LAPTOP mode.
}


extern const POINT WinSizeBorderOriginal[4][5];
extern const POINT WinSizeBorderLarge[4][5];
extern const POINT WinSizeBorderMax[4][5];
extern POINT WinSizeBorder[4][5];
extern BYTE FullScreen;


#pragma pack(push, 8)

class TSteemDisplay {
  //FUNCTIONS
private:
#ifdef WIN32
#if defined(SSE_VID_DD)
  HRESULT InitDD();
#if defined(SSE_VID_DD7)
  static HRESULT WINAPI DDEnumModesCallback(LPDDSURFACEDESC2,LPVOID);
#else
  static HRESULT WINAPI DDEnumModesCallback(LPDDSURFACEDESC,LPVOID);
#endif
  HRESULT DDCreateSurfaces();
  void DDDestroySurfaces();
  HRESULT DDError(char *,HRESULT);
#endif
  int STXPixels();
  int STYPixels();
#if defined(SSE_VID_D3D)
public:
  HRESULT D3DInit(); 
  HRESULT D3DCreateSurfaces();
  HRESULT D3DSpriteInit();
  void D3DDestroySurfaces();
  VOID D3DRelease();
  inline HRESULT D3DLock();
  inline void D3DUnlock();
  inline bool D3DBlit();
  friend HRESULT check_device_type(D3DDEVTYPE DeviceType,D3DFORMAT DisplayFormat);
  void D3DUpdateWH(UINT mode);
private:
#endif//d3d
  // GDI Only
  bool InitGDI();
#endif//WIN32
#ifdef UNIX
  bool CheckDisplayMode(DWORD,DWORD,DWORD);
  bool InitX();
  bool InitXSHM();
#ifndef NO_XVIDMODE
  static int XVM_WinProc(void*,Window,XEvent*);
#endif
#endif

public:
  TSteemDisplay();
  ~TSteemDisplay();
  void SetMethods(int,...);
  HRESULT Init();
  HRESULT Lock();
  void VSync();
#if defined(SSE_VID_DD_3BUFFER_WIN)
  BOOL BlitIfVBlank(); // our polling function
#endif
  bool Blit();
  void WaitForAsyncBlitToFinish();
  void Unlock();
#ifdef SHOW_WAVEFORM
  void DrawWaveform();
#endif
#if defined(SSE_VID_D3D)
  void RunStart(bool Temp=false),RunEnd();
  HRESULT SetDisplayMode();
#else
  void RunStart(bool=0),RunEnd( WIN_ONLY(bool=0) );
  HRESULT SetDisplayMode(int,int,int,int=0,int* = NULL);
#endif
  void ScreenChange();
  void ChangeToFullScreen(),ChangeToWindowedMode(bool=0);
  void DrawFullScreenLetterbox(),FlipToDialogsScreen();
  bool CanGoToFullScreen();
  HRESULT RestoreSurfaces();
  void Release();
  HRESULT SaveScreenShot();
  bool BorderPossible();
#ifdef WIN32
#if !defined(SSE_VID_NO_FREEIMAGE)
  void FreeImageLoad();
  //bool ScreenShotIsFreeImageAvailable();
  void ScreenShotGetFormatOpts(EasyStringList*);
#endif
  void ScreenShotGetFormats(EasyStringList*);
#if defined(SSE_VID_2SCREENS)
  void CheckCurrentMonitorConfig(HMONITOR hCurrentMonitor=NULL);
#endif
#endif//WIN32
#if defined(UNIX)
  void Surround();
#endif
  //DATA
public: //temp
  BYTE* VideoMemoryEnd; // =draw_mem+size
  int VideoMemorySize;
  neochrome_file *pNeoFile;
#ifdef WIN32
  HBITMAP GDIBmp;
  BYTE *GDIBmpMem;
  HDC GDIBmpDC;
#if !defined(SSE_VID_NO_FREEIMAGE)
  HINSTANCE hFreeImage;
#endif
#if defined(SSE_VID_D3D)
#if defined(SSE_VID_D3D_FLIPEX)
  LPDIRECT3D9EX pD3D;
  LPDIRECT3DDEVICE9EX  pD3DDevice;
#else
  LPDIRECT3D9 pD3D;
  LPDIRECT3DDEVICE9  pD3DDevice;
#endif
  IDirect3DTexture9* pD3DTexture;
  ID3DXSprite* pD3DSprite;
  UINT m_Adapter;
  UINT D3DMode,oldD3DMode; // depends on video card and format
  UINT D3DFsW,D3DFsH;
  D3DPRESENT_PARAMETERS d3dpp; // double use for w,h, but we need other params
  D3DFORMAT m_DisplayFormat;
  D3DDEVTYPE m_DeviceType;
  DWORD m_vtx_proc;
#if defined(SSE_VID_D3D_SWAPCHAIN)
  IDirect3DSwapChain9* pSwapChain;
  IDirect3DSurface9* pBackBuffer;
#endif
#endif//#if defined(SSE_VID_D3D)
#if defined(SSE_VID_DD)
  IDirectDrawClipper *DDClipper;
#if defined(SSE_VID_DD7)
  IDirectDraw7 *DDObj;
  IDirectDrawSurface7 *DDPrimarySur,*DDBackSur;
  IDirectDrawSurface7 *OurBackSur,*DDBackSur2; // our second back buffer
  DDSURFACEDESC2 DDBackSurDesc,ddsd;
#else
  IDirectDraw2 *DDObj;
  IDirectDrawSurface *DDPrimarySur,*DDBackSur;
  IDirectDrawSurface *OurBackSur,*DDBackSur2; // our second back buffer
  DDSURFACEDESC DDBackSurDesc,ddsd;
#endif
  DWORD DDLockFlags;
  int DDClosestHz[5][2][NUM_HZ];
  COUNTER_VAR VSyncTiming; // must be public
  RECT LetterBoxRectangle;
  bool DDDisplayModePossible[5][2];
  bool DDBackSurIsAttached,DDExclusive;
#endif
  int GDIBmpLineLength;
  DWORD GDIBmpSize;
  int ScreenShotFormatOpts;
  Str ScreenShotExt;
#endif//WIN32
  POINT fs_res[NFSRES];
  BYTE fs_res_choice;
  RECT rcMonitor;
  Str ScreenShotNextFile;
  int Method,UseMethod[5],nUseMethod;
  int ScreenShotFormat;
  int ScreenShotMinSize;
  DWORD ChangeToWinTimeOut;
  DWORD SurfaceWidth,SurfaceHeight;
#ifdef WIN32
#if defined(SSE_VID_DD_3BUFFER_WIN)
  bool SurfaceToggle;
#endif
  bool DrawToVidMem,BlitHideMouse;
  bool DrawLetterboxWithGDI;
#endif//WIN32
#if defined(UNIX)
  XImage *X_Img;
  Window XVM_FullWin;
#ifndef NO_XVIDMODE
  XF86VidModeModeInfo **XVM_Modes;
  int XVM_nModes,XVM_ViewX,XVM_ViewY;
  int XVM_FullW,XVM_FullH;
#endif
#ifndef NO_SHM
  int SHMCompletion;
  XShmSegmentInfo XSHM_Info;
  bool XSHM_Attached;
  bool asynchronous_blit_in_progress;
#endif
  bool AlreadyWarnedOfBadMode;
  bool GoToFullscreenOnRun;
#endif//#if defined(UNIX)
  bool RunOnChangeToWindow;
  bool ScreenShotUseFullName,ScreenShotAlwaysAddNum;
  bool DoAsyncBlit;
  BYTE bpp_at_fullscreen;
};

extern TSteemDisplay Disp;

#pragma pack(pop)


#ifdef WIN32

extern bool TryDX;

#if defined(SSE_VID_DD)
extern BYTE HzIdxToHz[NUM_HZ];
#endif

#if defined(SSE_VID_NO_FREEIMAGE)

enum EFI {FIF_BMP=D3DXIFF_BMP,FIF_JPEG=D3DXIFF_JPG,FIF_PNG=D3DXIFF_PNG};

#else

#define FI_ENUM(x)      enum x
#define FI_STRUCT(x)	struct x

FI_STRUCT (FIBITMAP) { void *data; };

FI_ENUM(FREE_IMAGE_FORMAT) {
	FIF_UNKNOWN = -1,
	FIF_BMP = 0,
	FIF_ICO,
	FIF_JPEG,
	FIF_JNG,
	FIF_KOALA,
	FIF_LBM,
	FIF_MNG,
	FIF_PBM,
	FIF_PBMRAW,
	FIF_PCD,
	FIF_PCX,
	FIF_PGM,
	FIF_PGMRAW,
	FIF_PNG,
	FIF_PPM,
	FIF_PPMRAW,
	FIF_RAS,
	FIF_TARGA,
	FIF_TIFF,
	FIF_WBMP,
	FIF_PSD,
	FIF_CUT,
	FIF_IFF = FIF_LBM,
};

#define BMP_DEFAULT         0
#define BMP_SAVE_RLE        1
#define CUT_DEFAULT         0
#define ICO_DEFAULT         0
#define ICO_FIRST           0
#define ICO_SECOND          0
#define ICO_THIRD           0
#define IFF_DEFAULT         0
#define JPEG_DEFAULT        0
#define JPEG_FAST           1
#define JPEG_ACCURATE       2
#define JPEG_QUALITYSUPERB  0x80
#define JPEG_QUALITYGOOD    0x100
#define JPEG_QUALITYNORMAL  0x200
#define JPEG_QUALITYAVERAGE 0x400
#define JPEG_QUALITYBAD     0x800
#define KOALA_DEFAULT       0
#define LBM_DEFAULT         0
#define MNG_DEFAULT         0
#define PCD_DEFAULT         0
#define PCD_BASE            1
#define PCD_BASEDIV4        2
#define PCD_BASEDIV16       3
#define PCX_DEFAULT         0
#define PNG_DEFAULT         0
#define PNG_IGNOREGAMMA		  1		// avoid gamma correction
#define PNM_DEFAULT         0
#define PNM_SAVE_RAW        0       // If set the writer saves in RAW format (i.e. P4, P5 or P6)
#define PNM_SAVE_ASCII      1       // If set the writer saves in ASCII format (i.e. P1, P2 or P3)
#define RAS_DEFAULT         0
#define TARGA_DEFAULT       0
#define TARGA_LOAD_RGB888   1       // If set the loader converts RGB555 and ARGB8888 -> RGB888.
#define TARGA_LOAD_RGB555   2       // This flag is obsolete
#define TIFF_DEFAULT        0
#define WBMP_DEFAULT        0
#define PSD_DEFAULT         0

typedef void (PASCAL *FI_INITPROC)(BOOL);
typedef void (PASCAL *FI_DEINITPROC)();
typedef FIBITMAP* (PASCAL *FI_CONVFROMRAWPROC)(BYTE*,int,int,int,UINT,UINT,UINT,UINT,BOOL);
typedef BOOL (PASCAL *FI_SAVEPROC)(FREE_IMAGE_FORMAT,FIBITMAP*,const char *,int);
typedef void (PASCAL *FI_FREEPROC)(FIBITMAP*);
typedef BOOL (PASCAL *FI_SUPPORTBPPPROC)(FREE_IMAGE_FORMAT,int);

#endif
#endif//WIN32

#ifdef UNIX
#ifdef NO_SHM
extern bool TrySHM;
#else
extern bool TrySHM;
#endif
#endif

#if defined(SSE_VID_DD) || defined(UNIX)
extern BYTE draw_fs_topgap;
extern int prefer_pc_hz[NPC_HZ_CHOICES]; // 640x400, 640x480, 800x600, stretch
extern WORD tested_pc_hz[NPC_HZ_CHOICES];
extern WORD real_pc_hz[NPC_HZ_CHOICES];
#endif


#endif//SSEDISPLAY_H
