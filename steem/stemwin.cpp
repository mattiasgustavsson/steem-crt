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

DOMAIN: GUI
FILE: stemwin.cpp
DESCRIPTION: This file handles the main Steem window and its various buttons.
It used to be included from gui.cpp in older builds.
Keyboard input starts here.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <gui.h>
#include <draw.h>
#include <computer.h>
#include <debug_framereport.h>
#include <shortcutbox.h>
#include <patchesbox.h>
#include <infobox.h>
#include <translate.h>
#include <loadsave.h>
#include <diskman.h>
#include <stjoy.h>
#include <harddiskman.h>
#include <palette.h>
#include <dataloadsave.h>
#include <osd.h>
#include <debugger.h>
#include <mymisc.h>
#include <key_table.h>



#if defined(SSE_EMU_THREAD)

HANDLE hEmuThread=NULL;
DWORD EmuThreadId=0;
bool SuspendRendering=false;

// "hang instead of crashing"
// main thread calls lock, rendering parameters will change
// emu thread calls acknowledge when it's ready to wait
// main thread calls unlock so that emu thread can continue

bool TThreadFlag::Lock() {
  bool was_blocked=blocked;
  ASSERT(!was_blocked);
  DWORD id=GetCurrentThreadId();
  ASSERT(id!=EmuThreadId);
  if(id==EmuThreadId)
    return was_blocked;
  acknowledged=false;
  blocked=true;
  while(!acknowledged && runstate==RUNSTATE_RUNNING)
    Sleep(0);
  return was_blocked;
}


bool TThreadFlag::Acknowledge() {
  bool was_blocked=blocked;
  while(blocked && runstate==RUNSTATE_RUNNING)
  {
    acknowledged=true;
    Sleep(0);
  }
  return was_blocked;
}


bool TThreadFlag::Unlock() {
  bool was_blocked=blocked;
  blocked=acknowledged=false;
  return was_blocked;
}


TThreadFlag SoundLock,VideoLock;


DWORD WINAPI EmuThreadProc(PVOID pParam) { // called instead of run()
  EnableMenuItem(StemWin_AltMenu,604,MF_ENABLED);
  run();
  SendMessage((HWND)pParam,BM_SETCHECK,0,0); // reset play button
  EnableWindow(GetDlgItem(OptionBox.Handle,IDC_EMU_THREAD),TRUE);
  EnableMenuItem(StemWin_AltMenu,604,MF_DISABLED);
  hEmuThread=NULL;
  return 0;
}

#endif

#if defined(SSE_GUI_ALT_MENU)
BOOL AltMenuOn=FALSE;
#endif

#undef LOGSECTION
#define LOGSECTION LOGSECTION_VIDEO_RENDERING

void StemWinResize(int xo,int yo) {
  TRACE_LOG("StemWinResize(%d,%d)\n",xo,yo);
  int res=screen_res;
  if(video_mixed_output)
    res=1;
  int Idx=WinSizeForRes[res];
  if(extended_monitor) 
  {
#ifdef WIN32
    int FrameWidth=GuiSM.cx_frame()*2;
#else
    int FrameWidth=0;
#endif
    SetStemWinSize(MIN((int)em_width,(GetScreenWidth()-4-FrameWidth)),
      MIN((int)em_height,(GetScreenHeight()-5-MENUHEIGHT-4-30)),0,0);
  }
  else if(border)
  {
    int h=WinSizeBorder[res][Idx].y;
    //optional PAL aspect ratio in windowed mode
    if(OPTION_ST_ASPECT_RATIO && res<2) // also non stretch, even if that's not beautiful
      h=(int)((float)h*ST_ASPECT_RATIO_DISTORTION);
    SetStemWinSize(WinSizeBorder[res][Idx].x,h,
      xo*WinSize[res][Idx].x/640,yo*WinSize[res][Idx].y/400);
  }
  else
  {
    while(WinSize[res][Idx].x>GetScreenWidth()) 
      Idx--;
    int h=WinSize[res][Idx].y;
    if(OPTION_ST_ASPECT_RATIO && res<2)
    {
      h=(int)((Glue.previous_video_freq==60)
        ? (float)h*ST_ASPECT_RATIO_DISTORTION_60HZ 
        : (float)h*ST_ASPECT_RATIO_DISTORTION);
    }
    SetStemWinSize(WinSize[res][Idx].x,h,
      xo*WinSize[res][Idx].x/640,yo*WinSize[res][Idx].y/400);
  }
#if defined(SSE_VID_D3D)
  if(D3D9_OK && Disp.pD3DDevice)
    Disp.D3DSpriteInit(); //smooth res changes (eg in GEM)
#endif
  if(!FullScreen)
    REFRESH_STATUS_BAR;
}


void fast_forward_change(bool Down,bool Searchlight) {
  if(Down) 
  {
    if(fast_forward<=0) 
    {
      if(runstate==RUNSTATE_STOPPED) 
      {
        CLICK_PLAY_BUTTON();
        fast_forward=RUNSTATE_STOPPED+1;
      }
      else if(runstate==RUNSTATE_STOPPING) 
      {
        if(fast_forward==-1) 
          runstate=RUNSTATE_RUNNING;
        fast_forward=RUNSTATE_STOPPED+1;
      }
      else
        fast_forward=1;
      // keep sound if emu thread, fixing the concurrency seems hard, the effect
      // here is not too bad
      if(!OPTION_EMUTHREAD)
        Sound_Stop();
    }
    flashlight(Searchlight);
  }
  else if(fast_forward) 
  {
    if(fast_forward==RUNSTATE_STOPPED+1) 
    {
      fast_forward=0;
      if(runstate==RUNSTATE_RUNNING) 
      {
        runstate=RUNSTATE_STOPPING;
        fast_forward=-1;
      }
#ifdef WIN32
      RunMessagePosted=false;
#endif      
    }
    else
      fast_forward=0;
    fast_forward_stuck_down=0;
    flashlight(0);
    if(!OPTION_EMUTHREAD)
      Sound_Start();
  }
  floppy_access_started_ff=0;
  WIN_ONLY( SendMessage(GetDlgItem(StemWin,109),BM_SETCHECK,fast_forward,1); )
  UNIX_ONLY( FastBut.set_check(fast_forward); )
}


void flashlight(bool on) {
  if(on && !flashlight_flag) 
  { //turn flashlight on
    for(int n=0;n<9;n++)
      PCpal[n]=colour_convert(240-n*15,255-n*15,60);
    for(int n=0;n<7;n++)
      PCpal[n+9]=colour_convert(0,30+n*8,50+n*30);
    flashlight_flag=true;
  }
  else if(on==0) 
  {
    flashlight_flag=false;
    draw_init_resdependent();
    palette_convert_all();
  }
}


void slow_motion_change(bool Down) {
  if(Down) 
  {
    if(slow_motion<=0) 
    {
      if(runstate==RUNSTATE_STOPPED) 
      {
        CLICK_PLAY_BUTTON();
        slow_motion=RUNSTATE_STOPPED+1;
      }
      else if(runstate==RUNSTATE_STOPPING) 
      {
        if(slow_motion==-1) 
          runstate=RUNSTATE_RUNNING;
        slow_motion=RUNSTATE_STOPPED+1;
      }
      else
        slow_motion=1;
      //TRACE("slow %d\n",slow_motion);
      // keep sound if emu thread, fixing the concurrency seems hard, the effect
      // here is bad (stutter)
      if(!OPTION_EMUTHREAD)
        Sound_Stop();
    }
  }
  else if(slow_motion) 
  {
    if(slow_motion==RUNSTATE_STOPPED+1) 
    {
      slow_motion=0;
      if(runstate==RUNSTATE_RUNNING) 
      {
        runstate=RUNSTATE_STOPPING;
        slow_motion=-1;
#ifdef WIN32
        RunMessagePosted=false;
#endif 
      }
    }
    else
      slow_motion=0;
    //TRACE("slow %d\n",slow_motion);
    if(!OPTION_EMUTHREAD)
      Sound_Start();
  }
}


void SetStemWinSize(int w,int h,int xo,int yo) {
  TRACE_LOG("SetStemWinSize %d %d %d %d\n",xo,yo,w,h);
#ifdef WIN32
  GuiSM.Update();
  if(FullScreen)
  {
    rcPreFS.top=MAX(int(rcPreFS.top+yo),-GuiSM.cy_caption());
    rcPreFS.right=rcPreFS.left+w+4+GuiSM.cx_frame()*2;
    rcPreFS.bottom=rcPreFS.top+h+MENUHEIGHT+4+GuiSM.cy_frame()*2
      +GuiSM.cy_caption();
  }
  else
  {
    if(bAppMaximized==0&&bAppMinimized==0)
    {
#if defined(SSE_GUI_MENU) // probably good in general
      RECT rcClient, rcWind;
      POINT ptDiff;
      GetClientRect(StemWin, &rcClient);
      GetWindowRect(StemWin, &rcWind);
      ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
      ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;
      MoveWindow(StemWin, rcWind.left + xo , rcWind.top + yo, w + ptDiff.x, h + ptDiff.y, TRUE);
#else
      RECT rc;
      GetWindowRect(StemWin,&rc);
#if defined(SSE_VID_2SCREENS)
      TRACE_VID_R("SetWindowPos 4 %d %d %d %d\n",rc.left+xo,MAX((int)(rc.top+yo),
        -GuiSM.cy_caption()),w+4+GuiSM.cx_frame()*2,h+MENUHEIGHT+4+GuiSM.cy_frame()*2+GuiSM.cy_caption());
      SetWindowPos(StemWin,0,rc.left+xo,MAX((int)(rc.top+yo),
        -GuiSM.cy_caption()),w+4+GuiSM.cx_frame()*2,h+MENUHEIGHT+4
        +GuiSM.cy_frame()*2+GuiSM.cy_caption(),SWP_NOZORDER|SWP_NOACTIVATE);
#else
      SetWindowPos(StemWin,0,rc.left+xo,MAX((int)(rc.top+yo),
        -GuiSM.cy_caption()),w+4+GuiSM.cx_frame()*2,h+MENUHEIGHT+4
        +GuiSM.cy_frame()*2+GuiSM.cy_caption(),SWP_NOZORDER | SWP_NOACTIVATE);
#endif
#endif
    }
    else
    {
      WINDOWPLACEMENT wp;
      wp.length=sizeof(WINDOWPLACEMENT);
      GetWindowPlacement(StemWin,&wp);
      RECT *rc=&wp.rcNormalPosition;
      rc->left=MAX(int(rc->left+xo),-GuiSM.cy_caption());
      rc->top=MAX(rc->top+yo,0l);
      rc->right=rc->left+w+4+GuiSM.cx_frame()*2;
      rc->bottom=rc->top+h+MENUHEIGHT+4+GuiSM.cy_frame()*2+GuiSM.cy_caption();
      SetWindowPlacement(StemWin,&wp);
    }
  }
#endif//WIN32

#ifdef UNIX
  if (XD==NULL) return;

  if (bAppMaximized==0 && bAppMinimized==0){
    XResizeWindow(XD,StemWin,2+w+2,MENUHEIGHT+2+h+2);
    XClearArea(XD,StemWin,0,0,2+w+2,MENUHEIGHT+2+h+2,True);
  }else{
    ///// Adjust restore size
  }

  XSizeHints *pHints=XAllocSizeHints();
  if (pHints){
    pHints->flags=PMinSize;
		pHints->min_width=320+4;
		pHints->min_height=200+4+MENUHEIGHT;
    XSetWMSizeHints(XD,StemWin,pHints,XA_WM_NORMAL_HINTS);
    XFree(pHints);
  }
#endif
}


void MoveStemWin(int x,int y,int w,int h) {
  if(StemWin==NULL)
    return;

#ifdef WIN32
  if(FullScreen) 
  {
    if(x==MSW_NOCHANGE) x=rcPreFS.left;
    if(y==MSW_NOCHANGE) y=rcPreFS.top;
    if(w==MSW_NOCHANGE) w=rcPreFS.right-rcPreFS.left;
    if(h==MSW_NOCHANGE) h=rcPreFS.top-rcPreFS.bottom;
    rcPreFS.left=x;rcPreFS.top=y;rcPreFS.right=x+w;rcPreFS.bottom=y+h;
  }
  else 
  {
    RECT rc;
    GetWindowRect(StemWin,&rc);
    int new_x=rc.left,new_y=rc.top,new_w=rc.right-rc.left,new_h=rc.bottom-rc.top;
    if(x!=MSW_NOCHANGE) new_x=x;
    if(y!=MSW_NOCHANGE) new_y=y;
    if(w!=MSW_NOCHANGE) new_w=w;
    if(h!=MSW_NOCHANGE) new_h=h;
    MoveWindow(StemWin,new_x,new_y,new_w,new_h,true);
  }
#endif

#ifdef UNIX
  if (XD==NULL) return;

  if (x==MSW_NOCHANGE || y==MSW_NOCHANGE){
    x=MSW_NOCHANGE;
    y=MSW_NOCHANGE;
  }

  XWindowAttributes wa;
  XGetWindowAttributes(XD,StemWin,&wa);
  int new_w=int((w==MSW_NOCHANGE) ? wa.width:w);
  int new_h=int((h==MSW_NOCHANGE) ? wa.height:h);
  if (w==wa.width && h==wa.height){ // Don't resize
    if (x!=MSW_NOCHANGE) XMoveWindow(XD,StemWin,x,y);
  }else{
    if (x==MSW_NOCHANGE){
      XResizeWindow(XD,StemWin,new_w,new_h);
    }else{
      XMoveResizeWindow(XD,StemWin,x,y,new_w,new_h);
    }
  }
#endif
}


#ifdef WIN32

void GetRealVKCodeForKeypad(WPARAM &wPar,LPARAM &lPar) {
  UINT Scancode=BYTE(HIWORD(lPar));
/*
24 Indicates whether the key is an extended key, such as the right-hand ALT 
and CTRL keys that appear on an enhanced 101- or 102-key keyboard. The
 value is 1 if it is an extended key; otherwise, it is zero. 
*/
  bool Extend=(lPar & BIT_24)!=0;
  if(Scancode==MapVirtualKey(VK_INSERT,0)) wPar=Extend?VK_INSERT:VK_NUMPAD0;
  if(Scancode==MapVirtualKey(VK_DELETE,0)) wPar=Extend?VK_DELETE:VK_DECIMAL;
  if(Scancode==MapVirtualKey(VK_END,0)) wPar=Extend?VK_END:VK_NUMPAD1;
  if(Scancode==MapVirtualKey(VK_DOWN,0)) wPar=Extend?VK_DOWN:VK_NUMPAD2;
  if(Scancode==MapVirtualKey(VK_NEXT,0)) wPar=Extend?VK_NEXT:VK_NUMPAD3;
  if(Scancode==MapVirtualKey(VK_LEFT,0)) wPar=Extend?VK_LEFT:VK_NUMPAD4;
  if(Scancode==MapVirtualKey(VK_CLEAR,0)) wPar=Extend?VK_CLEAR:VK_NUMPAD5;
  if(Scancode==MapVirtualKey(VK_RIGHT,0)) wPar=Extend?VK_RIGHT:VK_NUMPAD6;
  if(Scancode==MapVirtualKey(VK_HOME,0)) wPar=Extend?VK_HOME:VK_NUMPAD7;
  if(Scancode==MapVirtualKey(VK_UP,0)) wPar=Extend?VK_UP:VK_NUMPAD8;
  if(Scancode==MapVirtualKey(VK_PRIOR,0)) wPar=Extend?VK_PRIOR:VK_NUMPAD9;
}


LRESULT PASCAL WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar) {
  WORD wpar_lo=LOWORD(wPar);
  switch(Mess) {
  case WM_PAINT:
  {
    RECT dest;
    GetClientRect(Win,&dest);
    int Height=dest.bottom;
    dest.bottom=MENUHEIGHT;
    // copy the region before BeginPaint(), which will reset it
    HRGN hRgn=0;
    int region_type=0;
    if(FullScreen) 
    {
      hRgn=CreateRectRgn(0,0,0,0);
      region_type=GetUpdateRgn(Win,hRgn,TRUE); // do draw NC areas
    }
    PAINTSTRUCT ps;
    BeginPaint(Win,&ps);
/*  When a dialog box is moved in the fullscreen GUI, it trashes the background.
    It's no big problem but it looks bad.
    It is possible to redraw the picture by blitting on dirty rectangles.
    In Direct3D, one call is enough:
    Disp.pD3DDevice->Present(NULL,NULL,NULL,lpRgnData);
    Unfortunately, I've only seen it work in Windows 10, not XP nor Vista.
    In DirectDraw, we need to do a blit for each rectangle. It works on
    most systems, but only in flip and straight blit modes, and not with
    Triple Buffering.
    So for a consistent experience, we erase the rectangles in all cases.
*/
    if(FullScreen && OPTION_FULLSCREEN_GUI)
    {
      if(region_type!=NULLREGION && region_type!=ERROR)
      {
        DWORD dwCount=GetRegionData(hRgn,0,NULL); // 1st call to get #bytes
        if(dwCount)
        {
          RGNDATA *lpRgnData=(RGNDATA*)new BYTE[dwCount];
          dwCount=GetRegionData(hRgn,dwCount,lpRgnData); // 2nd call to get rectangles
          if(dwCount)
          {
            HBRUSH br=CreateSolidBrush(GetSysColor(COLOR_BACKGROUND));
            LPRECT pRect=(LPRECT)lpRgnData->Buffer;
            for(DWORD i=0;i<lpRgnData->rdh.nCount;i++)
              FillRect(ps.hdc,&pRect[i],br); // erase all rectangles
            DeleteObject(br);
          } //if(dwCount)
          delete[] lpRgnData;
        }//if(dwCount)
      }
      DeleteObject(hRgn);
    }
#ifndef ONEGAME
    //SS background for menu bar, must do that AFTER we redraw the 
    // invalidated rectangles in fullscreen mode or we get those stripes...
    HBRUSH br=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(ps.hdc,&dest,br);
    DeleteObject(br);
#endif
    if(FullScreen)
    {
      int menu_bottom=0 NOT_ONEGAME(+MENUHEIGHT+2);
#ifndef ONEGAME
      dest.bottom=menu_bottom;
      DrawEdge(ps.hdc,&dest,EDGE_RAISED,BF_BOTTOM);
#endif
      int x_gap=0,y_gap=0;
#if defined(SSE_VID_DD)
      if(draw_fs_blit_mode<DFSM_STRETCHBLIT)
      {
        if(extended_monitor)
        {
          x_gap=(Disp.SurfaceWidth-em_width)/2;
          y_gap=(Disp.SurfaceHeight-em_height)/2;
        }
        else if(border)
        {
          x_gap=(800-(BORDER_SIDE+320+BORDER_SIDE)*2)/2;
          y_gap=(600-(BORDER_TOP*2+400+BORDER_BOTTOM*2))/2;
        }
        else if(draw_fs_topgap)
          y_gap=draw_fs_topgap;
      }
#endif
      br=(HBRUSH)GetStockObject(BLACK_BRUSH);
      RECT rc;
      if(x_gap)
      {
        rc.top=menu_bottom;rc.left=0;rc.bottom=Height;rc.right=x_gap;
        FillRect(ps.hdc,&rc,br);
        rc.left=dest.right-x_gap;rc.right=dest.right;
        FillRect(ps.hdc,&rc,br);
      }
      if(y_gap)
      {
        rc.top=menu_bottom;rc.left=0;rc.bottom=y_gap;rc.right=dest.right;
        FillRect(ps.hdc,&rc,br);
        rc.top=Height-y_gap;rc.bottom=Height;
        FillRect(ps.hdc,&rc,br);
      }
      draw_grille_black=50;
    }
    else //if(FullScreen) 
    {
      dest.top+=MENUHEIGHT;
      dest.bottom=Height;
      DrawEdge(ps.hdc,&dest,EDGE_SUNKEN,BF_RECT);
      if(runstate==RUNSTATE_STOPPED)
      {
        draw_end();
        if(draw_blit()==0)
        {
          dest.left+=2;dest.top+=2;dest.right-=2;dest.bottom-=2;
          FillRect(ps.hdc,&dest,(HBRUSH)GetStockObject(BLACK_BRUSH));
        }
      }
      else 
      {
        dest.bottom=MENUHEIGHT;
        FillRect(ps.hdc,&dest,(HBRUSH)GetStockObject(BLACK_BRUSH));
      }
    }
    EndPaint(Win,&ps);
    return 0;
  }

  case (WM_USER+2):   // Update commands
    if(wPar==2323)  // Running?
      return (runstate==RUNSTATE_RUNNING);
#if !defined(SSE_NO_UPDATE)
    if(wPar==54542) {       // New Steem
      UpdateWin=(HWND)lPar;
      ShowWindow(GetDlgItem(Win,120),int(UpdateWin?SW_SHOW:SW_HIDE));
      if(runstate==RUNSTATE_RUNNING && UpdateWin) {
        osd_start_scroller(EasyStr(T("Steem update! Steem version "))+
          GetCSFStr("Update","LatestVersion","1.3",globalINIFile)+" "+
          T("is ready to be downloaded. Click on the new button in the toolbar (to the right of paste) for more details."));
      }
      return 0;
    }
    else if(wPar==12345) { // New Patches
      if(PatchesBox.IsVisible()) {
        PatchesBox.RefreshPatchList();
      }
      else {
        PatchesBox.SetButtonIcon();
      }
      return 0;
    }
#endif
    break;

  case WM_COMMAND:
    if(wpar_lo>=IDC_DISK_MANAGER&&wpar_lo<200)
    {
      int NotifyMess=HIWORD(wPar);
      if(NotifyMess==BN_CLICKED)
        HandleButtonMessage(wpar_lo,HWND(lPar));
      else if(wpar_lo==109) 
      {
        if(NotifyMess==BN_PUSHED||NotifyMess==BN_UNPUSHED||NotifyMess==BN_DBLCLK) 
        {
          if(NotifyMess==BN_DBLCLK) 
            fast_forward_stuck_down=true;
          if(fast_forward_stuck_down) 
          {
            if(NotifyMess==BN_UNPUSHED) 
              break;
            if(NotifyMess==BN_PUSHED) 
              NotifyMess=BN_UNPUSHED;  // Click to turn off
          }
          fast_forward_change(NotifyMess!=BN_UNPUSHED,SendMessage(HWND(lPar),BM_GETCLICKBUTTON,0,0)==2);
        }
      }
      else if(wpar_lo==101) 
      {
        if(NotifyMess==BN_PUSHED||NotifyMess==BN_UNPUSHED)
        {
          if(SendMessage(HWND(lPar),BM_GETCLICKBUTTON,0,0)==2||NotifyMess==BN_UNPUSHED)
            slow_motion_change(NotifyMess==BN_PUSHED);
        }
      }
    }
    else if((wpar_lo>=207&&wpar_lo<220)||wpar_lo==203) 
    {
      if(runstate==RUNSTATE_STOPPED) 
      {
        bool AddToHistory=true;
        if(wpar_lo>=210) 
          LastSnapShot=StateHist[wpar_lo-210];
        EasyStr fn=LastSnapShot;
        if(wpar_lo==207) 
        {
#if !defined(SSE_GUI_MENU)
          if(AltMenuOn)
          {
            SetMenu(StemWin,NULL);
            AltMenuOn=FALSE;
          }
#endif
          fn=WriteDir+SLASH+"auto_reset_backup.sts",AddToHistory=0;
        }
        if(wpar_lo==208) 
          fn=WriteDir+SLASH+"auto_loadsnapshot_backup.sts",AddToHistory=0;
        if(wpar_lo==209)
          fn=DefaultSnapshotFile;
        LoadSnapShot(fn,AddToHistory);
        if(wpar_lo==207||wpar_lo==208) 
          DeleteFile(fn);
      }
      else 
      {
        Glue.m_Status.stop_emu=(OPTION_NO_OSD_ON_STOP) ? 2 : 1;
        PostMessage(Win,Mess,wPar,lPar); // Keep delaying message until stopped
        return 0;
      }
    }
    else if(wpar_lo>=300&&wpar_lo<311)
      PasteSpeed=wpar_lo-299;
    else if(wpar_lo>=400&&wpar_lo<450) 
    {
      if(wpar_lo<420) 
      { // Change screenshot format
        EasyStringList format_sl;
        Disp.ScreenShotGetFormats(&format_sl);
        OptionBox.ChangeScreenShotFormat((int)format_sl[wpar_lo-400].Data[0],
          format_sl[wpar_lo-400].String);
      }
      else if(wpar_lo<440) 
      { // Change screenshot format options
        EasyStringList format_sl;
#if !defined(SSE_VID_NO_FREEIMAGE)
        Disp.ScreenShotGetFormatOpts(&format_sl);
        OptionBox.ChangeScreenShotFormatOpts((int)format_sl[wpar_lo-420].Data[0]);
#endif
      }
      else if(wpar_lo==440) // Change folder
        OptionBox.ChooseScreenShotFolder(Win);
      else if(wpar_lo==441) // Open folder
        ShellExecute(NULL,NULL,ScreenShotFol,"","",SW_SHOWNORMAL);
      else if(wpar_lo==442) 
      { // Minimum size shots
        Disp.ScreenShotMinSize=!Disp.ScreenShotMinSize;
        if(OptionBox.Handle) {
          if(GetDlgItem(OptionBox.Handle,1024)) {
            SendMessage(GetDlgItem(OptionBox.Handle,1024),BM_SETCHECK,Disp.ScreenShotMinSize,0);
          }
        }
      }
#if defined(SSE_GUI_CONFIG_FILE)
/*  v3.8.0 Player has clicked on the 'Configuration' icon, then
    on 'Load configuration file' or 'Save configuration file'.
    Duplicates the options/configurations feature!
*/
      else if(wpar_lo==443||wpar_lo==444)
      {
#if !defined(SSE_GUI_MENU)
        if(AltMenuOn)
        {
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
        }
#endif
        EasyStr FilNam=FileSelect(Win,wpar_lo==443
          ?T("Load configuration file"):T("Save configuration file"),
          OptionBox.ProfileDir,
          FSTypes(0,T("Configuration files").Text,"*." CONFIG_FILE_EXT,NULL),
          1,(wpar_lo==443),CONFIG_FILE_EXT,"");
        if(FilNam.NotEmpty()) 
        {
          TConfigStoreFile CSF; //on the stack
          bool ok=CSF.Open(FilNam);
          // Load
          if(wpar_lo==443) 
          {
            if(ok)
            {
              OPTION_WS=CSF.GetByte("Machine","WakeUpState",OPTION_WS);
              LoadAllDialogData(false,"",NULL,&CSF); // radical!
              ROMFile=CSF.GetStr("Machine","ROM_File",ROMFile);
              // add current TOS path if necessary
              if(strchr(ROMFile.Text,SLASHCHAR)==NULL) // no slash = no path
              {
                EasyStr tmp=OptionBox.TOSBrowseDir+SLASH+ROMFile;
                ROMFile=tmp;
              }
              OptionBox.NewROMFile=ROMFile;
              reset_st(RESET_COLD|RESET_STOP|RESET_CHANGESETTINGS|RESET_BACKUP);
              SetForegroundWindow(StemWin);
            }
            else
              Alert(T("ini file not recognised"),T("Configuration Error"),MB_ICONERROR);
          }
          // Save
          else
          {
            SaveAllDialogData(false,"",&CSF); // radical!
            CSF.SetStr("Machine","WakeUpState",EasyStr(OPTION_WS));
          }
          CSF.Close();
        }
      }
#endif//SSE_GUI_CONFIG_FILE
    }
    else 
    {
      if(HIWORD(wPar)==0) 
      {
        switch(wpar_lo) {
        case 200:       //Load SnapShot
        case 201:       //Save SnapShot
        {
#if !defined(SSE_GUI_MENU)
          if(AltMenuOn)
          {
            SetMenu(StemWin,NULL);
            AltMenuOn=FALSE;
          }
#endif
          EnableAllWindows(0,Win);
          Sound_Stop();
          int old_runstate=runstate;
          if(FullScreen && runstate==RUNSTATE_RUNNING) 
          {
            runstate=RUNSTATE_STOPPED;
            Disp.RunEnd();
            UpdateWindow(StemWin);
          }
          EasyStr FilNam;
          Str LastStateFol=LastSnapShot;
          RemoveFileNameFromPath(LastStateFol,REMOVE_SLASH);
          if(wpar_lo==200) 
          {
            FilNam=FileSelect(Win,T("Load Memory Snapshot"),LastStateFol,
              FSTypes(0,T("Steem Memory Snapshots").Text,"*.sts",NULL),
              1,true,"sts",GetFileNameFromPath(LastSnapShot));
          }
          else 
          {
            FilNam=FileSelect(Win,T("Save Memory Snapshot"),LastStateFol,
              FSTypes(0,T("Steem Memory Snapshots").Text,"*.sts",NULL),
              1,0,"sts",GetFileNameFromPath(LastSnapShot));
          }
          if(FilNam.NotEmpty()) 
          {
            if(SnapShotGetLastBackupPath().NotEmpty())
              DeleteFile(SnapShotGetLastBackupPath());
            LastSnapShot=FilNam;
            if(wpar_lo==200) 
            {
              if(old_runstate==RUNSTATE_STOPPED)
                LoadSnapShot(LastSnapShot);
              else 
              {
                old_runstate=RUNSTATE_STOPPING;
                PostMessage(Win,WM_COMMAND,203,lPar); // Delay load until stopped
              }
            }
            else
              SaveSnapShot(LastSnapShot,-1);
          }
          SetForegroundWindow(Win);
          runstate=old_runstate;
          if(FullScreen && runstate==RUNSTATE_RUNNING)
            Disp.RunStart(0);
          timer=timeGetTime();
          avg_frame_time_timer=timer;
          avg_frame_time_counter=0;
          auto_frameskip_target_time=timer;
          Sound_Start();
          EnableAllWindows(true,Win);
          break;
        }
        case 205:
          if(SnapShotGetLastBackupPath().NotEmpty()) 
          {
            DeleteFile(SnapShotGetLastBackupPath());
            MoveFile(LastSnapShot,SnapShotGetLastBackupPath()); // Make backup
          }
          SaveSnapShot(LastSnapShot,-1);
          break;
        case 206:
          // Restore backup, can only get here if backup path is valid
          DeleteFile(LastSnapShot);
          MoveFile(SnapShotGetLastBackupPath(),LastSnapShot);
          break;
#if defined(SSE_GUI_ALT_MENU)
        case 601:
          CLICK_PLAY_BUTTON();
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
        case 602:
          DiskMan.Show();
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
        case 603:
          PatchesBox.Show();
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
#if defined(SSE_EMU_THREAD)
        case 604:
          if(OPTION_EMUTHREAD&&runstate!=RUNSTATE_STOPPED&&hEmuThread)
          {// kill thread - radical
            TRACE2("kill thread %x\n",EmuThreadId);
            TerminateThread(hEmuThread,0);
            hEmuThread=NULL;
            runstate=RUNSTATE_STOPPED;
            SendMessage(GetDlgItem(Win,101),BM_SETCHECK,0,0); // reset play button
            EnableMenuItem(StemWin_AltMenu,wpar_lo,MF_DISABLED);
          }
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
#endif
        case 605:
          if(FullScreen)
          {
            Disp.ChangeToWindowedMode(StatusInfo.MessageIndex
              ==TStatusInfo::BLIT_ERROR);
          }
          else
            PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,2);
          break;
        case 609:
          PostMessage(Win,WM_CLOSE,0,0);
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
        case 611:
          reset_st((DWORD)(RESET_COLD|RESET_NOSTOP|RESET_CHANGESETTINGS|RESET_BACKUP));
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
        case 612:
          reset_st((DWORD)RESET_WARM|(DWORD)RESET_NOSTOP|(DWORD)RESET_BACKUP);
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
        case 651:
          OptionBox.Show();
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
        case 613:
        case 614:
        {
          EasyStr path=FileSelect(NULL,T("Select Disk Image"),DiskMan.DisksFol,
            FSTypes(2,NULL),1,true,"");
          EasyStr name=GetFileNameFromPath(path);
          DiskMan.InsertDisk(wpar_lo-613,name,path,0,0,"",true); // can be ""
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
        }
        case 615:
          HardDiskMan.Show();
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
#if defined(SSE_ACSI)
        case 616:
          AcsiHardDiskMan.Show();
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
#endif
        case 652:
          ShortcutBox.Show();
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
        case 653:
          JoyConfig.Show();
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
        case 654:
          InfoBox.Show();
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
#if defined(SSE_DEBUGGER_TOGGLE)
        case 655: // but the Debugger itself needs the mouse AFAIK
          ShowWindow(DWin,SW_SHOW);
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
#endif
#endif
#if defined(SSE_GUI_TOOLBAR)
        case 656:
          ToolBar.Show();
#if !defined(SSE_GUI_MENU)
          SetMenu(StemWin,NULL);
          AltMenuOn=FALSE;
#endif
          break;
#endif
        }//sw
      }//if
    }//if
    break;

  case WM_USER:
    if(wPar==1234) 
    {
#ifndef ONEGAME
#if defined(SSE_GUI_TOOLBAR)
#define Win ToolBar.Handle
#endif
      SendMessage(GetDlgItem(Win,IDC_DISK_MANAGER),BM_SETCHECK,
        DiskMan.IsVisible(),0);
      SendMessage(GetDlgItem(Win,IDC_JOYSTICKS),BM_SETCHECK,
        JoyConfig.IsVisible(),0);
      SendMessage(GetDlgItem(Win,IDC_INFO),BM_SETCHECK,
        InfoBox.IsVisible(),0);
      SendMessage(GetDlgItem(Win,IDC_OPTIONS),BM_SETCHECK,
        OptionBox.IsVisible(),0);
      SendMessage(GetDlgItem(Win,IDC_SHORTCUTS),BM_SETCHECK,
        ShortcutBox.IsVisible(),0);
      SendMessage(GetDlgItem(Win,IDC_PATCHES),BM_SETCHECK,
        PatchesBox.IsVisible(),0);
#if defined(SSE_DEBUGGER_TOGGLE)
      SendMessage(GetDlgItem(Win,IDC_DEBUGGER),BM_SETCHECK,DebuggerVisible,0);
#endif
#if defined(SSE_GUI_TOOLBAR)
#undef Win
#endif

#endif
    }
    else if(wPar==12345 && DisableFocusWin) 
      SetForegroundWindow(DisableFocusWin);
    else if(wPar==123) 
    {
      // Allows external programs to press ST keys
      WORD VKCode=LOWORD(lPar);
      if(VKCode==VK_LCONTROL||VKCode==VK_RCONTROL) VKCode=VK_CONTROL;
      if(VKCode==VK_LMENU||VKCode==VK_RMENU) VKCode=VK_MENU;
      int ChangeModMask=0;
      if(VKCode==VK_SHIFT)   ChangeModMask=b00000011;
      if(VKCode==VK_LSHIFT)  ChangeModMask=b00000001;
      if(VKCode==VK_RSHIFT)  ChangeModMask=b00000010;
      if(VKCode==VK_CONTROL) ChangeModMask=b00001100;
      if(VKCode==VK_MENU)    ChangeModMask=b00110000;
      if(HIWORD(lPar))
        ExternalModDown&=~ChangeModMask;
      else
        ExternalModDown|=ChangeModMask;
      if(VKCode==VK_SHIFT) 
      {
        if(ST_Key_Down[key_table[VK_LSHIFT]]!=!HIWORD(lPar)) 
          HandleKeyPress(VK_LSHIFT,HIWORD(lPar),IGNORE_EXTEND|NO_SHIFT_SWITCH);
        if(ST_Key_Down[key_table[VK_RSHIFT]]!=!HIWORD(lPar)) 
          HandleKeyPress(VK_RSHIFT,HIWORD(lPar),IGNORE_EXTEND|NO_SHIFT_SWITCH);
      }
      else
        HandleKeyPress(VKCode,HIWORD(lPar),IGNORE_EXTEND|NO_SHIFT_SWITCH);
    }
    else if(wPar==12) 
    {// Return from fullscreen
      SetWindowLong(StemWin,GWL_STYLE,WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX
        |WS_SIZEBOX|WS_MAXIMIZEBOX|WS_VISIBLE);
      if((GetWindowLong(StemWin,GWL_STYLE) & WS_SIZEBOX)==0
        && timeGetTime()<Disp.ChangeToWinTimeOut)
      {
        PostMessage(StemWin,WM_USER,12,0);
        break;
      }
      SetWindowLong(DiskMan.Handle,GWL_STYLE,(GetWindowLong(DiskMan.Handle,
        GWL_STYLE) & ~WS_MAXIMIZE)|WS_MINIMIZEBOX);
      bool MaximizeDiskMan=DiskMan.Maximized && DiskMan.IsVisible();
      for(int n=0;n<nStemDialogs;n++) 
      {
        DEBUG_ONLY(if(DialogList[n]!=&HistList)) 
          DialogList[n]->MakeParent(NULL);
      }
      SetParent(ToolTip,NULL);
      if(Disp.BorderPossible()==0) 
      {
        border=0;
        OptionBox.EnableBorderOptions(0);
      }
#if defined(SSE_GUI_TOOLBAR)
      ShowWindow(GetDlgItem(StemWin,106),SW_HIDE);
      ShowWindow(GetDlgItem(StemWin,116),SW_HIDE);
#endif
      if(MaximizeDiskMan) 
        ShowWindow(DiskMan.Handle,SW_MAXIMIZE);
      SendMessage(StemWin,WM_USER,13,0);
    }
    else if(wPar==13) 
    { // Return from fullscreen
      SetWindowPos(StemWin,HWND(bAOT?HWND_TOPMOST:HWND_NOTOPMOST),rcPreFS.left,
        rcPreFS.top,rcPreFS.right-rcPreFS.left,rcPreFS.bottom-rcPreFS.top,0);
      UpdateWindow(StemWin);
      RECT rc;
      GetWindowRect(StemWin,&rc);
      if(EqualRect(&rc,&rcPreFS)==0&&timeGetTime()<Disp.ChangeToWinTimeOut) 
      {
        PostMessage(StemWin,WM_USER,13,0);
        break;
      }

     // SendMessage(GetDlgItem(Win,IDC_DEBUGGER),BM_RELOADICON,0,0);//test
       //SendMessage(GetDlgItem(Win,IDC_DEBUGGER),WM_DESTROY,0,0);//test
        //SendMessage(GetDlgItem(Win,IDC_DEBUGGER),WM_CREATE,0,0);//test

      SetWindowLong(StemWin,GWL_STYLE,WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX
        |WS_SIZEBOX|WS_MAXIMIZEBOX|WS_VISIBLE);
      SetForegroundWindow(StemWin);
      CheckResetDisplay();
      InvalidateRect(StemWin,NULL,true);
      REFRESH_STATUS_BAR;
      palette_convert_all();
      draw(true);
      if(Disp.RunOnChangeToWindow) 
      {
        CLICK_PLAY_BUTTON();
        Disp.RunOnChangeToWindow=0;
      }
    }
    break;

  case WM_NCLBUTTONDBLCLK:
    if(wPar==HTCAPTION) 
    {
      PostMessage(Win,WM_SYSCOMMAND,WPARAM(IsZoomed(Win)?SC_RESTORE:SC_MAXIMIZE),0);
      return 0;
    }
    break;

  case WM_SYSCOMMAND:
    if(wPar>=IDSYS_NORMAL&&wPar<IDSYS_BORDEROFF&&wPar!=IDSYS_TOP)
    {
      if(IsZoomed(Win)) 
        ShowWindow(Win,SW_SHOWNORMAL);
    }
    switch(wPar) {
    case IDSYS_NORMAL: //1:1
      StemWinResize();
      return 0;
    case IDSYS_ASPECT: //Aspect Ratio
    {
      RECT rc;
      GetClientRect(Win,&rc);
      double ratio;
      int res=int(video_mixed_output?1:screen_res);
      int Idx=WinSizeForRes[res];
      if(border)
        ratio=(double)(WinSizeBorder[res][Idx].x)/(double)(WinSizeBorder[res][Idx].y);
      else
        ratio=(double)(WinSize[res][Idx].x)/(double)(WinSize[res][Idx].y);
      double sz=((double)(rc.right-4)/ratio+(double)(rc.bottom-(MENUHEIGHT+4)))/2.0;
      SetStemWinSize((int)(sz*ratio+0.5),(int)(sz+0.5));
      return 0;
    }
    case IDSYS_TOP:
      bAOT=!bAOT;
      CheckMenuItem(StemWin_SysMenu,102,
        MF_BYCOMMAND|int(bAOT?MF_CHECKED:MF_UNCHECKED));
      SetWindowPos(StemWin,HWND(bAOT?HWND_TOPMOST:HWND_NOTOPMOST),0,0,0,0,
        SWP_NOMOVE|SWP_NOSIZE);
      return 0;
    case IDSYS_BIGGER: //bigger window
    case IDSYS_SMALLER: //smaller window
      if(ResChangeResize) 
      {
        int res=int(video_mixed_output?1:screen_res);
        int size=WinSizeForRes[res];
        if(wPar==104) 
        {
          if(size<3)
            size++;
        }
        else
        {
          if(size>0)
            size--;
        }
        WinSizeForRes[res]=size;
        StemWinResize();
        OptionBox.UpdateWindowSizeAndBorder();
      }
      else 
      {
        RECT rc;
        GetClientRect(StemWin,&rc);
        if(wPar==104) 
        {
          rc.right=rc.right*14142/10000;
          rc.bottom=rc.bottom*14142/10000;
        }
        else 
        {
          rc.right=rc.right*10000/14142;
          rc.bottom=rc.bottom*10000/14142;
        }
        SetStemWinSize(rc.right,rc.bottom);
      }
      return 0;
    case IDSYS_BORDEROFF:case IDSYS_BORDERON: //Borders
      OptionBox.SetBorder((int)(wPar-110));
      OptionBox.UpdateWindowSizeAndBorder();
      CheckMenuRadioItem(StemWin_SysMenu,IDSYS_BORDEROFF,IDSYS_BORDERON,
        110+MIN((int)border,1),MF_BYCOMMAND);
      return 0;
    case IDSYS_NOOSD:
      OptionBox.ChangeOSDDisable(!osd_disable);
      return 0;
    }//sw
    switch(wPar&0xFFF0) {
    case SC_MAXIMIZE:
      // when Steem posts the message itself, it sets lParam to 2
      // which means 'fullscreen, not maximize'
      // if the message comes from Windows (click on maximize), it
      // will be 0 and then what we do depends on the option (v4.0.1)
      if((OPTION_MAX_FS || lPar==2) && Disp.CanGoToFullScreen())
      {
        Disp.ChangeToFullScreen(); // fullscreen
        return 0;
      }
      break; // maximize
    case SC_MONITORPOWER:
      if(runstate==RUNSTATE_RUNNING) 
        return 0;
      break;
    case SC_SCREENSAVE:
      //SS this prevents screensaver from activating but only if Steem has 
      // focus, else we don't get this message
      if(runstate==RUNSTATE_RUNNING||FullScreen) 
        return 0;
      break;
    case SC_TASKLIST:case SC_PREVWINDOW:case SC_NEXTWINDOW:
      if(runstate==RUNSTATE_RUNNING) 
        return 0;
      break;
    }//sw
    break;

  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYDOWN:case WM_SYSKEYUP:
  {
    short vk_menu_state=GetKeyState(VK_MENU);
    short vk_control_state=GetKeyState(VK_CONTROL);
    short vk_shift_state=GetKeyState(VK_SHIFT);
    if(bAppActive==0)
      return 0;
#if defined(SSE_GUI_ALT_MENU)
    if(Mess==WM_SYSKEYUP && wPar==VK_MENU && runstate!=RUNSTATE_RUNNING)
    {
#if defined(SSE_GUI_MENU)
      SetMenu(StemWin,StemWin_AltMenu); // force menu reappear
#else
      if(AltMenuOn==FALSE)
      {
        SetMenu(StemWin,StemWin_AltMenu);
        AltMenuOn=TRUE;
      }
      else
      {
        SetMenu(StemWin,NULL);
        AltMenuOn=FALSE;
      }
#endif
      return DefWindowProc(Win,Mess,wPar,lPar); //so the system will highlight it...
    }
    if(runstate==RUNSTATE_RUNNING
      &&(wPar==VK_SHIFT||wPar==VK_CONTROL||wPar==VK_MENU)) 
      return 0; // when running, keep Steem's original system
#else
    if(wPar==VK_SHIFT||wPar==VK_CONTROL||wPar==VK_MENU) return 0;
#endif
#ifndef ONEGAME
    if(TaskSwitchDisabled)
    {
      int n=0;
      while(TaskSwitchVKList[n])
        if(LOBYTE(wPar)==TaskSwitchVKList[n++])
          return 0;
    }
#endif
    // using F12 as emulator start/stop
#if defined(_DEBUG) && (_MSC_VER==1900) // except VC takes F12 for Break so so we use Break as F12
    if(wPar==VK_PAUSE && vk_shift_state>=0&&vk_control_state>=0&&vk_menu_state>=0)
#else
    if(wPar==VK_F12 && vk_shift_state>=0&&vk_control_state>=0&&vk_menu_state>=0)
#endif
    {
      if(Mess==WM_KEYUP||Mess==WM_SYSKEYUP)
      {
        if(runstate==RUNSTATE_STOPPED)
        {
          CLICK_PLAY_BUTTON(); // it's a macro
        }
        else
        {
#if defined(SSE_DEBUGGER_FRAME_REPORT)
          FrameEvents.Report();
#endif
#if defined(SSE_EMU_THREAD)
          if(OPTION_EMUTHREAD&&Glue.m_Status.stop_emu)
          {
            if(Alert(T("The emulation thread isn't responding. Kill it?"),
              T("STEEM CRASHED AGAIN"),MB_ICONQUESTION|MB_YESNO)==IDYES)
            {
              TRACE2("kill thread %x\n",EmuThreadId);
              TerminateThread(hEmuThread,0);
              hEmuThread=NULL;
              SendMessage(GetDlgItem(Win,101),BM_SETCHECK,0,0); // reset play button
              runstate=RUNSTATE_STOPPED;
            }
          }
          else
#endif
            Glue.m_Status.stop_emu=(OPTION_NO_OSD_ON_STOP) ? 2 : 1;
        }

      }
      return 0;
    }
#if defined(SSE_GUI_KBD) // F1 for help
    if(wPar==VK_F1 && vk_shift_state>=0&&vk_control_state>=0&&vk_menu_state>=0
      &&(runstate==RUNSTATE_STOPPED))
    {
      InfoBox.Page=0; // = about
      InfoBox.Hide(); // just in case
      InfoBox.Show();
    }
#endif
    if(runstate==RUNSTATE_RUNNING)
    {
#ifndef ONEGAME
      if((wPar==VK_F11||wPar==VK_PAUSE) // v402
        &&vk_shift_state>=0&&vk_control_state>=0&&vk_menu_state>=0)
      {
        if(Mess==WM_KEYUP||Mess==WM_SYSKEYUP)
        {
          if(stem_mousemode==STEM_MOUSEMODE_DISABLED) 
          {
            SetForegroundWindow(StemWin);
            SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
            if(OPTION_CAPTURE_MOUSE&2) // auto
              OPTION_CAPTURE_MOUSE=3; // make it sticky
          }
          else
          {
            SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
            if(OPTION_CAPTURE_MOUSE&2) // auto
              OPTION_CAPTURE_MOUSE=2;
          }
        }
        return 0;
      }
#else
      if(wPar==VK_PAUSE||wPar==VK_F12||wPar==VK_ESCAPE) {
        if(Mess==WM_KEYUP||Mess==WM_SYSKEYUP) {
          if(runstate==RUNSTATE_RUNNING) {
            OGStopAction=OG_QUIT;
            runstate=RUNSTATE_STOPPING;
          }
        }
      }
#endif
      else if(Mess==WM_KEYUP||Mess==WM_SYSKEYUP||(lPar&0x40000000)==0) 
      {
        GetRealVKCodeForKeypad(wPar,lPar);
        bool Extended=(lPar&0x1000000)!=0;
        if(joy_is_key_used(BYTE(wPar))==0&&CutDisableKey[BYTE(wPar)]==0) 
          HandleKeyPress((UINT)wPar,Mess==WM_KEYUP||Mess==WM_SYSKEYUP,Extended);
      }
    }
    else if(runstate==RUNSTATE_STOPPED) 
    {
      if(Mess==WM_SYSKEYUP && wPar==VK_RETURN &&
        GetForegroundWindow()==Win && GetAsyncKeyState(VK_MENU)<0) 
      {
        if(FullScreen)
          Disp.ChangeToWindowedMode();
        else 
        {
          if(Disp.CanGoToFullScreen()) 
            Disp.ChangeToFullScreen();
        }
      }
      else if((Mess==WM_KEYUP||Mess==WM_SYSKEYUP)&&(wPar==VK_CANCEL)) 
      {
        CLICK_PLAY_BUTTON();
      }
      else if((Mess==WM_SYSKEYDOWN && wPar==VK_F4)||
        (Mess==WM_KEYDOWN && wPar=='W' && GetKeyState(VK_CONTROL)<0)) 
      {
        PostMessage(Win,WM_CLOSE,0,0);
      }
#if defined(SSE_GUI_ALT_MENU)
      else
        return DefWindowProc(Win,Mess,wPar,lPar);
#endif
    }
    return 0;
  }

  case WM_SYSCHAR:
#if defined(SSE_GUI_KBD) // window menu on alt+space
    if(runstate==RUNSTATE_STOPPED && GetKeyState(VK_MENU)<0)
    {
      if(wPar==VK_SPACE)
        return DefWindowProc(Win,Mess,wPar,lPar);
    }
    //no break
#endif

  case WM_SYSDEADCHAR:
    return 0;

  case WM_LBUTTONDOWN:case WM_LBUTTONUP:
  case WM_RBUTTONDOWN:case WM_RBUTTONUP:
#ifdef DEBUG_BUILD // SS oops, one of my mods was already existing?
    if(runstate==RUNSTATE_STOPPED && stem_mousemode==STEM_MOUSEMODE_BREAKPOINT) 
    {
      if(wPar & MK_LBUTTON) {
        int x=LOWORD(lPar)-2,y=HIWORD(lPar)-2-MENUHEIGHT;
        x&=0xfffffff0; //16 pixels per raster
        x/=16;         //to raster number
        MEM_ADDRESS ad=vbase;
        if(screen_res==2) ad+=y*80;else ad+=y*160;
        if(screen_res==0) ad+=x*8;else if(screen_res==1)ad+=x*4;else ad+=x*2;
        d2_dpoke(ad,0xface);
        debug_set_mon(ad,0,0xffff);
        SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
        }
      }
#endif
    if(HIWORD(lPar)>MENUHEIGHT && stem_mousemode==STEM_MOUSEMODE_DISABLED) 
    {
      if(GetForegroundWindow()==StemWin) 
      {
        if(runstate==RUNSTATE_RUNNING)
          SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
        else if(StartEmuOnClick) 
        {
          CLICK_PLAY_BUTTON();
        }
#if defined(SSE_DEBUGGER_REPORT_SCAN_Y_ON_CLICK)
/*  When emulation is stopped, right click in window will tell which
    scanline we're at.
    To do this we use the "status bar".
    We assume double height, we don't compute this.
    While we're at it, we also report guessed X
*/
        else if(Mess==WM_RBUTTONDOWN)
        {
          int guessed_scan_y=(HIWORD(lPar)-MENUHEIGHT)-1;
          if(screen_res<2)
            guessed_scan_y/=2;
          if(border)
            guessed_scan_y-=(screen_res==2)?BORDER_TOP*2:BORDER_TOP;
          int guessed_x=LOWORD(lPar)/2-SideBorderSizeWin;
#if defined(SSE_DEBUGGER_REPORT_SDP_ON_CLICK)
          MEM_ADDRESS computed_sdp=FrameEvents.GetSDP(guessed_x,guessed_scan_y);
          sprintf(StatusInfo.text,"X%d Y%d $%X",guessed_x,guessed_scan_y,computed_sdp);
#else
          sprintf(StatusInfo.text,"X%d Y%d",guessed_x,guessed_scan_y);
#endif
          StatusInfo.MessageIndex=TStatusInfo::BOILER_MESSAGE;
          HWND status_bar_win=GetDlgItem(StemWin,IDC_STATUS_BAR); // get handle
          InvalidateRect(status_bar_win,NULL,false);
        }
#endif
      }
    }
    break;

  case WM_MOUSEWHEEL:
    MouseWheelMove+=short(HIWORD(wPar));
    return 0;

  case WM_TIMER:
    if(wPar==SHORTCUTS_TIMER_ID) 
    {
      if(bAppActive) 
        JoyGetPoses();
      ShortcutsCheck();
    }
    else if(wPar==DISPLAYCHANGE_TIMER_ID) 
    {
      KillTimer(Win,DISPLAYCHANGE_TIMER_ID);
      TConfigStoreFile CSF(globalINIFile);
      LoadAllIcons(&CSF,0);
      CSF.Close();
    }
    break;

  case WM_COPYDATA:
  case WM_DROPFILES:
  {
    EasyStr *Files=NULL;
    char **lpFile;
    int nFiles;
    if(Mess==WM_COPYDATA) 
    {
      COPYDATASTRUCT *cds=(COPYDATASTRUCT*)lPar;
      if(cds->dwData!=MAKECHARCONST('S','C','O','M')) break;  // Not Steem comline file
      nFiles=1;
      lpFile=(char**)&(cds->lpData); // lpFile is an array of nFiles pointers to char*s, cds->lpData is a char*
    }
    else 
    {
      nFiles=DragQueryFile((HDROP)wPar,0xffffffff,NULL,0);
      Files=new EasyStr[nFiles];
      lpFile=new char*[nFiles];
      for(int i=0;i<nFiles;i++) {
        Files[i].SetLength(MAX_PATH);
        DragQueryFile((HDROP)wPar,i,Files[i],MAX_PATH);
        lpFile[i]=Files[i].Text;
      }
      DragFinish((HDROP)wPar);
    }
    EasyStr OldDiskA=FloppyDrive[0].GetDisk();
    BootStateFile="";BootTOSImage=0;
    BootDisk[0]="";BootDisk[1]="";
    BootInMode=0;
    ParseCommandLine(nFiles,lpFile);
    if(BootStateFile.NotEmpty()) 
    {
      if(LoadSnapShot(BootStateFile)) 
      {
        SetForegroundWindow(Win);
        CLICK_PLAY_BUTTON();
      }
    }
    else 
    {
      for(int drive=0;drive<2;drive++) 
      {
        if(BootDisk[drive].NotEmpty()) 
        {
          EasyStr Name=GetFileNameFromPath(BootDisk[drive]);
          *strrchr(Name,'.')=0;
          DiskMan.InsertDisk(drive,Name,BootDisk[drive],0,0);
        }
      }
      bool ChangedDisk=NotSameStr_I(OldDiskA,FloppyDrive[0].GetDisk());
      if(BootTOSImage||ChangedDisk) 
      {
        SetForegroundWindow(Win);
        reset_st(RESET_COLD|DWORD(ChangedDisk?RESET_NOSTOP:RESET_STOP)|RESET_CHANGESETTINGS|RESET_BACKUP);
        if(ChangedDisk && runstate!=RUNSTATE_RUNNING) 
          CLICK_PLAY_BUTTON();
      }
      else if(BootInMode & BOOT_MODE_RUN) 
      {
        if(runstate==RUNSTATE_STOPPED)
        {
          CLICK_PLAY_BUTTON();
        }
      }
    }
    if(Mess==WM_COPYDATA) 
      return MAKECHARCONST('Y','A','Y','S');
    delete[] Files;
    delete[] lpFile;
    return 0;
  }

  case WM_GETMINMAXINFO:
    ((MINMAXINFO*)lPar)->ptMinTrackSize.x=320+GuiSM.cx_frame()*2+4;
    ((MINMAXINFO*)lPar)->ptMinTrackSize.y=200+GuiSM.cy_frame()*2
      +GuiSM.cy_caption()+MENUHEIGHT+4;
    break;

/*  v3.7
    Prevent player from resizing the window by dragging the border.
    Optional because stretching is cool and handy too.
    We pretend the mouse is on the client area, so the resizing cursor
    won't even appear.
    All border values are between HTLEFT and HTBOTTOMRIGHT.
    Returning HTCLIENT all the time would work with Windows 7 but not Vista
    (can't move or close window).
*/
  case WM_NCHITTEST:
  {
    LRESULT val=DefWindowProc(Win,Mess,wPar,lPar); // real area
    if(OPTION_BLOCK_RESIZE && val>=HTLEFT && val<=HTBOTTOMRIGHT)
      val=HTCLIENT;
    return val;
  }

/*  v3.7
    if option above isn't checked, this one enforces a +- correct aspect ratio
    lPar points to the absolute resizing rectangle, its values may be changed
    GetWindowRect() gives the current rectangle of the window, hopefully the
    same concept.
    TODO: keep AR really constant (computing may produce deviation)
*/
  case WM_SIZING:
    if(OPTION_LOCK_ASPECT_RATIO)
    {
      RECT current_coord;
      GetWindowRect(StemWin,&current_coord);
      LONG w=current_coord.bottom-current_coord.top;
      float a_r=(w)?( (float)(current_coord.right-current_coord.left)
        /(float)(w)):0;
      if(a_r)
      {
        if(((RECT*)lPar)->right>current_coord.right)
          ((RECT*)lPar)->bottom=(int)((float)(((RECT*)lPar)->right
          -((RECT*)lPar)->left) / a_r+((RECT*)lPar)->top);
        else
          ((RECT*)lPar)->right=(int)((float)(((RECT*)lPar)->bottom
          -((RECT*)lPar)->top) * a_r+((RECT*)lPar)->left);
      }
    }
    break;

  case WM_SIZE:
  {
    int cw=LOWORD(lPar),ch=HIWORD(lPar);
    RECT rc={0,MENUHEIGHT,cw,ch};
    //TRACE("WM_SIZE ");TRACE_RECT(rc);
    InvalidateRect(Win,&rc,0);
#ifndef ONEGAME
#if !defined(SSE_GUI_TOOLBAR)
    if(FullScreen) 
    {
      SetWindowPos(GetDlgItem(Win,106),0,cw-43,0,0,0,
        SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
      SetWindowPos(GetDlgItem(Win,116),0,cw-20,0,0,0,
        SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
      cw-=50;
    }
#endif
#if defined(SSE_GUI_TOOLBAR)
#elif defined(SSE_GUI_LEGACY_TOOLBAR)
    GUIToolbarArrangeIcons(cw);
#else
    {
      UINT mask=(SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS);
      int offset=20+5;
      SetWindowPos(GetDlgItem(Win,IDC_INFO),0,cw-offset,0,0,0,mask);
      offset+=23;
      SetWindowPos(GetDlgItem(Win,IDC_OPTIONS),0,cw-offset,0,0,0,mask);
      offset+=23;
      SetWindowPos(GetDlgItem(Win,IDC_SHORTCUTS),0,cw-offset,0,0,0,mask);
      offset+=23;
      SetWindowPos(GetDlgItem(Win,IDC_PATCHES),0,cw-offset,0,0,0,mask);
      offset+=23;
      SetWindowPos(GetDlgItem(Win,IDC_JOYSTICKS),0,cw-offset,0,0,0,mask);
      offset+=23;
      SetWindowPos(GetDlgItem(Win,IDC_DISK_MANAGER),0,cw-offset,0,0,0,mask);
    }
#endif
    if(ResetInfoWin)
      SendMessage(ResetInfoWin,WM_USER,1789,0);
#endif
    if(draw_grille_black<4) 
      draw_grille_black=4;
    if(FullScreen)
      CanUse_400=true;
    else if(border)
    {
      CanUse_400=(cw>=640+4*SideBorderSizeWin);
      TRACE_LOG("CanUse_400 %d cw %d %d ch %d %d\n",CanUse_400,cw,(4+640+4*SideBorderSizeWin),ch,(MENUHEIGHT+4+400+2*(BORDER_TOP+BottomBorderSize)));
    }
    else 
    {
      CanUse_400=(cw>=640);
      TRACE_LOG("CanUse_400 %d cw %d ch %d\n",CanUse_400,cw,ch);
    }
    switch(wPar) {
    case SIZE_MAXIMIZED: bAppMaximized=true; break;
    case SIZE_MINIMIZED: bAppMinimized=true; break;
    case SIZE_RESTORED:
      if(bAppMinimized)
        bAppMinimized=0;
      else if(bAppMaximized)
        bAppMaximized=0;
      break;
    }
    InvalidateRect(Win,NULL,FALSE);
    break;
  }

  case WM_DISPLAYCHANGE:
    if(FullScreen==0) 
    {
      bool old_draw_lock=draw_lock;
      OptionBox.EnableBorderOptions(Disp.BorderPossible());
      Disp.ScreenChange();
      palette_convert_all();
      draw(false);
      if(old_draw_lock) 
        draw_begin();
    }
    SetTimer(Win,DISPLAYCHANGE_TIMER_ID,500,NULL);
    break;
  case WM_SETTINGCHANGE:
    GuiSM.Update();
    return 0;
  case WM_CHANGECBCHAIN:
    if((HWND)wPar==NextClipboardViewerWin) 
      NextClipboardViewerWin=(HWND)lPar;
    else if(NextClipboardViewerWin) 
      SendMessage(NextClipboardViewerWin,Mess,wPar,lPar);
    break;

  case WM_DRAWCLIPBOARD:
    UpdatePasteButton();
    if(NextClipboardViewerWin) 
      SendMessage(NextClipboardViewerWin,Mess,wPar,lPar);
    break;

  case WM_ACTIVATEAPP:
    bAppActive=(wPar!=0);
    if(MuteWhenInactive && SoundBuf &&runstate==RUNSTATE_RUNNING)
    {
      DWORD dwStatus ;
      SoundBuf->GetStatus(&dwStatus);
      if(MuteWhenInactive==2&&bAppActive)
      {
        if(!(dwStatus&DSBSTATUS_PLAYING)) 
          SoundBuf->Play(0,0,DSBPLAY_LOOPING);
        MuteWhenInactive--;
      }
      else if(MuteWhenInactive==1&&!bAppActive)
      {
        if((dwStatus&DSBSTATUS_PLAYING))
          SoundBuf->Stop();
        MuteWhenInactive++;
      }
    }
    if(FullScreen) 
    {
      if(wPar) 
      {  //Activating
#if defined(SSE_VID_DD)
        if(using_res_640_400) 
        {
          using_res_640_400=0;
          change_fullscreen_display_mode(true);
        }
#endif
        for(int n=0;n<nStemDialogs;n++) 
        {
          if(DialogList[n]->Handle) 
          {
            SetWindowPos(DialogList[n]->Handle,NULL,DialogList[n]->FSLeft,
              DialogList[n]->FSTop,0,0,SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
          }
        }
#if !defined(SSE_VID_D3D_MISC)
        Disp.ScreenChange(); // double creation at "activate"?
#endif
#ifndef ONEGAME
        draw(true);
#else
        CLICK_PLAY_BUTTON();
#endif
      }
      else if(runstate==RUNSTATE_RUNNING) 
        Glue.m_Status.stop_emu=1;
    }
#if !defined(SSE_VID_32BIT_ONLY)
    if(BytesPerPixel==1) 
    {
      if(wPar) 
      {
        palette_prepare(true);
        AnimatePalette(winpal,palhalf+10,118,(PALETTEENTRY*)(logpal+palhalf+10));
      }
      else
        palette_remove();
    }
#endif
    break;

  case WM_SETCURSOR:
    switch(LOWORD(lPar)) {
    case HTCLIENT:
    {
      POINT pt;GetCursorPos(&pt);ScreenToClient(Win,&pt);
      RECT rc;GetClientRect(Win,&rc);
      if(pt.x>2&&pt.x<rc.right-2&&pt.y>MENUHEIGHT+1&&pt.y<rc.bottom-2)
      {
        if(stem_mousemode==STEM_MOUSEMODE_WINDOW)
        {
          if(no_set_cursor_pos)
            SetCursor(LoadCursor(NULL,IDC_CROSS));
          else
            SetCursor(NULL);
        }
        else if(stem_mousemode==STEM_MOUSEMODE_BREAKPOINT)
          SetCursor(LoadCursor(NULL,IDC_CROSS));
        else
          SetCursor(PCArrow);
        return true;
      }
    }//case
    }//sw
    break;

  case WM_ACTIVATE:
    if(wPar==WA_INACTIVE) 
    {
      SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
      UpdateSTKeys();
      EnableTaskSwitch();
    }
    else 
    {
      if(IsWindowEnabled(Win)==0)
        PostMessage(StemWin,WM_USER,12345,(LPARAM)Win);
      SetFocus(StemWin);
      if(runstate==RUNSTATE_RUNNING && AllowTaskSwitch==0) 
        DisableTaskSwitch();
    }
    break;

  case WM_KILLFOCUS:
    SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
    break;

  case WM_CLOSE:
    QuitSteem();
    return false;

  case WM_QUERYENDSESSION:
    QuitSteem();
    return true;

  case WM_DESTROY:
    ChangeClipboardChain(StemWin,NextClipboardViewerWin);
    StemWin=NULL;
    break;

  case WM_DRAWITEM: 
#if defined(SSE_GUI_MENU) // code duplication but not exactly the same status bar
   if(!wPar)
    {
      PDRAWITEMSTRUCT pdi=(PDRAWITEMSTRUCT)lPar;
      if(pdi->CtlType==ODT_MENU && pdi->itemID==IDC_STATUS_BAR && OPTION_STATUS_BAR)
      {
#if defined(SSE_EMU_THREAD)
      if(OPTION_EMUTHREAD&&runstate==RUNSTATE_RUNNING)
        SuspendRendering=true;
#endif
      HDC myHdc=((DRAWITEMSTRUCT*)lPar)->hDC;
      RECT myRect=((DRAWITEMSTRUCT*)lPar)->rcItem;
      char *status_bar_text=StatusInfo.text;
      if(FullScreen&&(!OPTION_FULLSCREEN_GUI||runstate==RUNSTATE_RUNNING))
        return TRUE;
      RECT rc1;
      GetClientRect(Win,&rc1);
      myRect.right=rc1.right;
      // erase rectangle
      VERIFY(FillRect(myHdc,&myRect,(HBRUSH)(COLOR_MENU+1)));
      myRect.left=0;
      myRect.top-=1;
      int nchars=(int)strlen(status_bar_text); // safe version?
      SIZE Size;
      GetTextExtentPoint32(myHdc,status_bar_text,nchars,&Size);
      int left=(myRect.right+myRect.left)/2-Size.cx/2;
      // adjust for trailing icons
      if(OPTION_STATUS_BAR)
      { // manual computing...
        left-=9*((ADAT)+(OPTION_C1)+(OPTION_VLE>0)+(OPTION_HACKS)
          +(!HardDiskMan.DisableHardDrives||ACSI_EMU_ON));
      }
      // Text
      VERIFY(TextOut(myHdc,left,myRect.top+3,status_bar_text,nchars));
      // Icons
      if(OPTION_STATUS_BAR&&StatusInfo.MessageIndex==TStatusInfo::MESSAGE_NONE)
      {
        HDC TempDC=CreateCompatibleDC(myHdc);
        HANDLE OldBmp=SelectObject(TempDC,LoadBitmap(Inst,"TOSFLAGS"));
        int FlagIdx=OptionBox.TOSLangToFlagIdx((int)ROM_PEEK(0x1D));
        if(FlagIdx>=0) 
        {
          int left2=left;
          if(ST_MODEL==STFM)
            left+=58;
          else if(SSEConfig.Mega)
            left+=58+16;
          else
            left+=51;
          VERIFY(BitBlt(myHdc,left,myRect.top+4,RC_FLAG_WIDTH,RC_FLAG_HEIGHT,TempDC,
            FlagIdx*RC_FLAG_WIDTH,0,SRCCOPY));
          left=left2;
        }
        DeleteObject(SelectObject(TempDC,OldBmp));
        DeleteDC(TempDC);
        int myx=left+Size.cx+3;
        if(n_cpu_cycles_per_second>CpuNormalHz||run_speed_ticks_per_second!=1000
          ||OPTION_CPU_CLOCK)
        {
          DrawIconEx(myHdc,myx,myRect.top+2,
            hGUIIcon[RC_ICO_CPUALTSPEED],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
        if(ADAT)
        {
          DrawIconEx(myHdc,myx,myRect.top+0,
            hGUIIcon[RC_ICO_ACCURATEFDC],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
        if(!HardDiskMan.DisableHardDrives||ACSI_EMU_ON)
        {
          DrawIconEx(myHdc,myx,myRect.top+2,
            hGUIIcon[RC_ICO_HARDDRIVE16],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
        if(OPTION_C1)
        {
          DrawIconEx(myHdc,myx,myRect.top+2,
            hGUIIcon[RC_ICO_OPS_C1],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
        if(OPTION_VLE==1) // II // change icons?
        {
          DrawIconEx(myHdc,myx,myRect.top+2,
            hGUIIcon[RC_ICO_OPS_C2],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
        if(OPTION_VLE==2) // III
        {
          DrawIconEx(myHdc,myx,myRect.top+2,
            hGUIIcon[RC_ICO_OPS_C3],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
        if(OPTION_HACKS)
        {
          DrawIconEx(myHdc,myx,myRect.top+2,
            hGUIIcon[RC_ICO_OPS_HACKS],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
      }
#if defined(SSE_EMU_THREAD)
      SuspendRendering=false;
#endif
      return TRUE;
      }
    }
    break;  
#endif
#if defined(SSE_GUI_STATUS_BAR) && !defined(SSE_GUI_MENU)
    if(wPar==IDC_STATUS_BAR) // status bar 
    {
#if defined(SSE_EMU_THREAD)
      if(OPTION_EMUTHREAD&&runstate==RUNSTATE_RUNNING)
        SuspendRendering=true;
#endif
#define myHdc ((DRAWITEMSTRUCT*)lPar)->hDC 
#define myRect ((DRAWITEMSTRUCT*)lPar)->rcItem
      char *status_bar_text=StatusInfo.text;
      if(FullScreen&&(!OPTION_FULLSCREEN_GUI||runstate==RUNSTATE_RUNNING))
      {
#if defined(SSE_EMU_THREAD)
        SuspendRendering=false;
#endif
        return TRUE;
        //FillRect(myHdc,&myRect,(HBRUSH)GetStockObject(BLACK_BRUSH));
      }
      // erase rectangle (different colour for hires)
      FillRect(myHdc,&myRect,((COLOUR_MONITOR)?
        (HBRUSH)(COLOR_MENU+1):(HBRUSH)(COLOR_WINDOWFRAME+1)));
      int nchars=(int)strlen(status_bar_text); // safe version?
      SIZE Size;
      GetTextExtentPoint32(myHdc,status_bar_text,nchars,&Size);
      int left=myRect.right/2-Size.cx/2;
      // adjust for trailing icons
      if(OPTION_STATUS_BAR)
      { // manual computing...
        left-=9*((ADAT)+(OPTION_C1)+(OPTION_VLE>0)+(OPTION_HACKS)
          +(!HardDiskMan.DisableHardDrives||ACSI_EMU_ON));
        // Text
        TextOut(myHdc,left,3,status_bar_text,nchars);
      }
      // Icons
      if(OPTION_STATUS_BAR&&StatusInfo.MessageIndex==TStatusInfo::MESSAGE_NONE)
      {
        HDC TempDC=CreateCompatibleDC(myHdc);
        HANDLE OldBmp=SelectObject(TempDC,LoadBitmap(Inst,"TOSFLAGS"));
        int FlagIdx=OptionBox.TOSLangToFlagIdx(SSEConfig.TosLanguage);
        if(FlagIdx>=0) 
        {
          int left2=left;
          if(ST_MODEL==STFM)
            left+=58;
#if defined(SSE_MEGASTE)
          else if(IS_MEGASTE)
            left+=58+16+8;
#endif
          else if(SSEConfig.Mega)
            left+=58+16;
          else
            left+=51;
          BitBlt(myHdc,left,4,RC_FLAG_WIDTH,RC_FLAG_HEIGHT,TempDC,
            FlagIdx*RC_FLAG_WIDTH,0,SRCCOPY);
          left=left2;
        }
        DeleteObject(SelectObject(TempDC,OldBmp));
        DeleteDC(TempDC);
        int myx=left+Size.cx+3;
        if(n_cpu_cycles_per_second>CpuNormalHz||run_speed_ticks_per_second!=1000
          ||OPTION_CPU_CLOCK)
        {
          DrawIconEx(myHdc,myx,2,
            hGUIIcon[RC_ICO_CPUALTSPEED],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
        if(ADAT)
        {
          DrawIconEx(myHdc,myx,0,
            hGUIIcon[RC_ICO_ACCURATEFDC],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
        if(!HardDiskMan.DisableHardDrives||ACSI_EMU_ON)
        {
          DrawIconEx(myHdc,myx,2,
            hGUIIcon[RC_ICO_HARDDRIVE16],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
        if(OPTION_C1)
        {
          DrawIconEx(myHdc,myx,2,
            hGUIIcon[RC_ICO_OPS_C1],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
        if(OPTION_VLE==1) // II // change icons?
        {
          DrawIconEx(myHdc,myx,2,
            hGUIIcon[RC_ICO_OPS_C2],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
        if(OPTION_VLE==2) // III
        {
          DrawIconEx(myHdc,myx,2,
            hGUIIcon[RC_ICO_OPS_C3],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
        if(OPTION_HACKS)
        {
          DrawIconEx(myHdc,myx,2,
            hGUIIcon[RC_ICO_OPS_HACKS],16,16,0,NULL,DI_NORMAL);
          myx+=19;
        }
      }
#if defined(SSE_EMU_THREAD)
      SuspendRendering=false;
#endif
      return TRUE;
#undef myHdc
#undef myRect
    }
#endif
    break;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}


void HandleButtonMessage(UINT Id,HWND hBut) {
  switch(Id) {
  case IDC_DISK_MANAGER:
    DiskMan.ToggleVisible();
    SendMessage(hBut,BM_SETCHECK,DiskMan.IsVisible(),0);
    break;
  case IDC_JOYSTICKS:
    JoyConfig.ToggleVisible();
    SendMessage(hBut,BM_SETCHECK,JoyConfig.IsVisible(),0);
    break;
  case IDC_INFO:
    InfoBox.ToggleVisible();
    SendMessage(hBut,BM_SETCHECK,InfoBox.IsVisible(),0);
    break;
  case IDC_OPTIONS:
    OptionBox.ToggleVisible();
    SendMessage(hBut,BM_SETCHECK,OptionBox.IsVisible(),0);
    break;
  case IDC_SHORTCUTS:
    ShortcutBox.ToggleVisible();
    SendMessage(hBut,BM_SETCHECK,ShortcutBox.IsVisible(),0);
    break;
  case IDC_PATCHES:
    PatchesBox.ToggleVisible();
    SendMessage(hBut,BM_SETCHECK,PatchesBox.IsVisible(),0);
    break;
  case IDC_PLAY: // TOGGLE EMULATION START/STOP
    if(SendMessage(hBut,BM_GETCLICKBUTTON,0,0)==2) 
      break;
    RunMessagePosted=false;
    if(runstate==RUNSTATE_STOPPED) 
    {
      if(FullScreen && bAppActive==0) 
        return;
      if(Cpu.ProcessingState==TMC68000::HALTED)
        break; // cancel "run" until reset
      if(GetForegroundWindow()==StemWin && GetCapture()==NULL 
        && IsIconic(StemWin)==0 && fast_forward!=(RUNSTATE_STOPPED+1) 
        && slow_motion!=(RUNSTATE_STOPPED+1)) 
      {
        if(OPTION_CAPTURE_MOUSE&1)
          SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
      }
      SendMessage(hBut,BM_SETCHECK,1,0);
#if defined(SSE_EMU_THREAD)
      EnableWindow(GetDlgItem(OptionBox.Handle,IDC_EMU_THREAD),FALSE);
      if(OPTION_EMUTHREAD)
      {
        // run() encapsulated in an apart thread
        if(hEmuThread==NULL)
          hEmuThread=CreateThread(NULL,0,EmuThreadProc,hBut,0,&EmuThreadId); 
      }
      else
      {
        EnableMenuItem(StemWin_AltMenu,604,MF_DISABLED);
        run(); // and we re-enter when checking messages at VBL
        SendMessage(hBut,BM_SETCHECK,0,0);
        EnableWindow(GetDlgItem(OptionBox.Handle,IDC_EMU_THREAD),TRUE);
      }
#else
      run(); // unique call
      SendMessage(hBut,BM_SETCHECK,0,0);
#endif
    }
    else 
    {
      if(runstate==RUNSTATE_RUNNING) 
      {
#if defined(SSE_DEBUGGER_FRAME_REPORT)
        FrameEvents.Report();
#endif
        Glue.m_Status.stop_emu=(OPTION_NO_OSD_ON_STOP) ? 2 : 1;
        SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
      }
#if defined(SSE_EMU_THREAD)
      else if(Glue.m_Status.stop_emu && OPTION_EMUTHREAD)
      { // try to stop twice
        if(Alert(T("The emulation thread isn't responding. Kill it?"),
          T("STEEM CRASH"),MB_ICONQUESTION|MB_YESNO)==IDYES)
        {
          TRACE2("kill thread %x\n",EmuThreadId);
          TerminateThread(hEmuThread,0);
          hEmuThread=NULL;
          SendMessage(hBut,BM_SETCHECK,0,0); // reset play button
          runstate=RUNSTATE_STOPPED;
        }
      }
#endif
    }
    break;
  case IDC_RESET:
  {
    bool Warm=(SendMessage(hBut,BM_GETCLICKBUTTON,0,0)==1);
    reset_st(DWORD(Warm?RESET_WARM:RESET_COLD)|DWORD(RESET_NOSTOP)|
      RESET_CHANGESETTINGS|RESET_BACKUP);
    break;
  }
  case 106:
    Disp.ChangeToWindowedMode(StatusInfo.MessageIndex==TStatusInfo::BLIT_ERROR);
    break;
#if !defined(SSE_NO_UPDATE)
  case 120:
    if(UpdateWin) {
      SendMessage(hBut,BM_SETCHECK,1,0);
      ShowWindow(UpdateWin,SW_SHOW);
      SetForegroundWindow(UpdateWin);
    }
    break;
#endif
#if defined(SSE_GUI_CONFIG_FILE)
/*  Player has clicked on the 'Configuration' icon, this makes a
    popup menu appear, 'Load configuration file' or 'Save configuration file'.
*/
  case IDC_CONFIGS:
  {
    RECT rc;
    GetWindowRect(hBut,&rc);
    HMENU Pop=CreatePopupMenu();
    AppendMenu(Pop,MF_STRING,443,T("Load configuration file"));
    AppendMenu(Pop,MF_STRING,444,T("Save configuration file"));
    SendMessage(hBut,BM_SETCHECK,1,0);
    TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
      rc.left,rc.bottom,0,StemWin,NULL);
    SendMessage(hBut,BM_SETCHECK,0,0);
    DestroyMenu(Pop);
    break;
  }
#endif
  case 108:
  {
    EasyStringList sl;
    SnapShotGetOptions(&sl);
    HMENU SnapShotMenu=CreatePopupMenu();
    for(int i=0;i<sl.NumStrings;i++) 
    {
      if(IsSameStr(sl[i].String,"-"))
        AppendMenu(SnapShotMenu,MF_SEPARATOR,0,NULL);
      else
        AppendMenu(SnapShotMenu,MF_STRING|int(sl[i].Data[1]?MF_GRAYED:0),sl[i].Data[0],sl[i].String);
    }
    RECT rc;
    GetWindowRect(hBut,&rc);
    SendMessage(hBut,BM_SETCHECK,1,0);
    TrackPopupMenu(SnapShotMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
      rc.left,rc.bottom,0,StemWin,NULL);
    SendMessage(hBut,BM_SETCHECK,0,0);
    DestroyMenu(SnapShotMenu);
    break;
  }
  case 114:
  {
    if(SendMessage(hBut,BM_GETCLICKBUTTON,0,0)==2) 
    {
      HMENU Pop=CreatePopupMenu();
      for(int n=0;n<11;n++) 
        AppendMenu(Pop,MF_STRING,300+n,T("Delay")+" - "+n);
      CheckMenuRadioItem(Pop,300,310,299+PasteSpeed,MF_BYCOMMAND);
      RECT rc;
      GetWindowRect(hBut,&rc);
      SendMessage(hBut,BM_SETCHECK,1,0);
      TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
        rc.left,rc.bottom,0,StemWin,NULL);
      if(PasteText.Empty()) 
        SendMessage(hBut,BM_SETCHECK,0,0);
      DestroyMenu(Pop);
      break;
    }
    else
      PasteIntoSTAction(STPASTE_TOGGLE);
    break;
  }
  case IDC_SCREENSHOT:
  {
    if(SendMessage(hBut,BM_GETCLICKBUTTON,0,0)==2) 
    {
      HMENU Pop=CreatePopupMenu();
      EasyStringList format_sl;
      Disp.ScreenShotGetFormats(&format_sl);
      AppendMenu(Pop,MF_STRING,440,T("Change Screenshots Folder"));
      AppendMenu(Pop,MF_STRING,441,T("Open Screenshots Folder"));
      AppendMenu(Pop,MF_STRING|int(Disp.ScreenShotMinSize?MF_CHECKED:MF_UNCHECKED),
        442,T("Minimum Size Screenshots"));
      AppendMenu(Pop,MF_SEPARATOR,0,NULL);
      int sel=0;
      for(int n=0;n<format_sl.NumStrings;n++) 
      {
        AppendMenu(Pop,MF_STRING,400+n,format_sl[n].String);
        if(format_sl[n].Data[0]==Disp.ScreenShotFormat) sel=400+n;
      }
      CheckMenuRadioItem(Pop,400,400+format_sl.NumStrings,sel,MF_BYCOMMAND);
      format_sl.DeleteAll();
#if !defined(SSE_VID_NO_FREEIMAGE)
      Disp.ScreenShotGetFormatOpts(&format_sl);
      if(format_sl.NumStrings) 
      {
        AppendMenu(Pop,MF_SEPARATOR,0,NULL);
        for(int n=0;n<format_sl.NumStrings;n++)
          AppendMenu(Pop,MF_STRING,420+n,format_sl[n].String);
        CheckMenuRadioItem(Pop,420,420+format_sl.NumStrings,
          ((Disp.ScreenShotFormat==FIF_JPEG)?420+(Disp.ScreenShotFormatOpts>>
          (8+(Disp.ScreenShotFormatOpts==0x800)))-(Disp.ScreenShotFormatOpts==0x400)
            :420+Disp.ScreenShotFormatOpts),
          MF_BYCOMMAND);
      }
#endif
      RECT rc;
      GetWindowRect(hBut,&rc);
      SendMessage(hBut,BM_SETCHECK,1,0);
      TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
        rc.left,rc.bottom,0,StemWin,NULL);
      if(PasteText.Empty()) 
        SendMessage(hBut,BM_SETCHECK,0,0);
      DestroyMenu(Pop);
    }
    else 
    {
      if(Disp.ScreenShotFormat==IF_NEO)
      {
        Disp.pNeoFile=new neochrome_file; //32KB
        ZeroMemory(Disp.pNeoFile,sizeof(neochrome_file));
        for(int i=0;i<16;i++)
        {
          Disp.pNeoFile->palette[i]=STpal[i];
          SWAP_BIG_ENDIAN_WORD(Disp.pNeoFile->palette[i]);
        }
      }
      if(runstate==RUNSTATE_RUNNING)
        DoSaveScreenShot|=1;
      else
        Disp.SaveScreenShot();
    }
    break;
  }//case
#if defined(SSE_DEBUGGER_TOGGLE)
  case IDC_DEBUGGER:
    ShowWindow(DWin,DebuggerVisible ? SW_HIDE : SW_SHOW);
    break;
#endif
  }//sw
}


LRESULT CALLBACK FSClipWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar) {
  switch (Mess) {
  case WM_PAINT:
    if(draw_blit()==0) 
    {
      RECT dest;
      GetClientRect(Win,&dest);

      PAINTSTRUCT ps;
      BeginPaint(Win,&ps);
      FillRect(ps.hdc,&dest,(HBRUSH)GetStockObject(BLACK_BRUSH));
      EndPaint(Win,&ps);
    }
    else
      ValidateRect(Win,NULL);
    return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}


LRESULT CALLBACK FSQuitWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar) {
  bool CheckDown=0;
  switch(Mess) {
  case WM_CREATE:
    SetProp(Win,"Down",(HANDLE)0);
    break;
  case WM_DESTROY:
    RemoveProp(Win,"Down");
    break;
  case WM_PAINT:
  {
    RECT rc;
    GetClientRect(Win,&rc);
    PAINTSTRUCT ps;
    BeginPaint(Win,&ps);
    FillRect(ps.hdc,&rc,(HBRUSH)GetSysColorBrush(COLOR_BTNFACE));
    int Down=int((GetProp(Win,"Down"))?1:0);
    DrawIconEx(ps.hdc,Down,3+Down,(HICON)hGUIIcon[RC_ICO_FULLQUIT],16,16,0,NULL,
      DI_NORMAL);
    EndPaint(Win,&ps);
    return 0;
  }
  case WM_MOUSEMOVE:case WM_CAPTURECHANGED:
    CheckDown=true;
    break;
  case WM_LBUTTONDOWN:
    SetCapture(Win);
    CheckDown=true;
    break;
  case WM_LBUTTONUP:
    ReleaseCapture();
    CheckDown=true;
    PostMessage(Win,WM_USER,0xface,lPar);
    break;
  case WM_USER:
  {
    if(wPar!=0xface) 
      break;
    RECT dest;
    GetClientRect(Win,&dest);
    if(LOWORD(lPar)<dest.right && HIWORD(lPar)<dest.bottom) 
    {
      int Quit=IDYES;
      if(FSQuitAskFirst)
        Quit=Alert(T("Are you sure?"),T("Quit Steem"),MB_ICONQUESTION|MB_YESNO);
      if(Quit==IDYES) 
        QuitSteem();
    }
    return 0;
  }//case
  }//sw
  if(CheckDown) 
  {
    bool OldDown=(GetProp(Win,"Down")!=0);
    bool NewDown=0;
    if(GetCapture()==Win) 
    {
      RECT rc;
      GetClientRect(Win,&rc);
      POINT pt;
      GetCursorPos(&pt);
      ScreenToClient(Win,&pt);
      NewDown=(pt.x>=0&&pt.x<rc.right && pt.y>=0&&pt.y<rc.bottom);
    }
    if(OldDown!=NewDown) 
    {
      SetProp(Win,"Down",(HANDLE)NewDown);
      InvalidateRect(Win,NULL,0);
    }
    return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}

#endif//win32


HRESULT change_fullscreen_display_mode(bool resizeclippingwindow) {
  HRESULT Ret;

#if defined(SSE_VID_D3D)
  RECT rc={Disp.rcMonitor.left,MENUHEIGHT,Disp.rcMonitor.right,
    Disp.rcMonitor.bottom};
#else
  int bpp=32; // new default
#ifdef WIN32
  RECT rc={Disp.rcMonitor.left,MENUHEIGHT,640,480};
#else
  RECT rc={0,MENUHEIGHT,640,480};
#endif
  int hz_ok=0,hz=prefer_pc_hz[1+(border!=0)];
  if(draw_fs_blit_mode==DFSM_STRETCHBLIT)
  {
    hz=prefer_pc_hz[3];
    rc.right=Disp.fs_res[Disp.fs_res_choice].x;
    rc.bottom=Disp.fs_res[Disp.fs_res_choice].y;
  }
  else if(extended_monitor)
  {
    rc.right=MAX((int)em_width,640);
    rc.bottom=MAX((int)em_height,480);
    hz=0;
  }
  else if(draw_fs_blit_mode==DFSM_FAKEFULLSCREEN) 
  {
    rc.right=monitor_width;
    rc.bottom=monitor_height;
  }
  else if(border)
  {
    rc.right=800;
    rc.bottom=600;
  }
#endif
#if defined(SSE_VID_FAKE_FULLSCREEN) && defined(SSE_VID_DD)
  if(OPTION_FAKE_FULLSCREEN); else
#endif
#if defined(SSE_VID_D3D)
  if((Ret=Disp.SetDisplayMode())!=DD_OK)
#else
  if((Ret=Disp.SetDisplayMode(rc.right,rc.bottom,bpp,hz,&hz_ok))!=DD_OK)
#endif
    return Ret;
#if defined(SSE_VID_DD)
  if(hz)
  {
    if(draw_fs_blit_mode==DFSM_STRETCHBLIT)
    {
      tested_pc_hz[3]=MAKEWORD(hz,(hz_ok&1));
      real_pc_hz[3]=hz_ok>>16;
    }
    else
    {
      tested_pc_hz[1+(border!=0)]=MAKEWORD(hz,(hz_ok&1));
      real_pc_hz[1+(border!=0)]=hz_ok>>16;
    }
  }
#endif
#ifdef WIN32
#if defined(SSE_VID_2SCREENS)
#if defined(SSE_VID_DD) 
  get_fullscreen_totalrect(&rc);
  // Compute size
  int cw=rc.right-rc.left;
  int ch=rc.bottom-rc.top;
  TRACE_VID_R("SetWindowPos 2 %d %d %d %d\n",rc.left,rc.top,cw,ch);
  SetWindowPos(StemWin,0,rc.left,rc.top,cw,ch,0);
#endif
  // D3D: done in D3DCreateSurfaces()
#else
  SetWindowPos(StemWin,HWND_TOPMOST,0,0,rc.right,rc.bottom,0);
#endif
  if(resizeclippingwindow) 
  {
#if defined(SSE_VID_D3D) 
#elif defined(SSE_VID_DD) && defined(SSE_VID_2SCREENS)
    get_fullscreen_totalrect(&rc);
    // Compute size
    cw=rc.right-rc.left;
    ch=rc.bottom-rc.top;
    TRACE_VID_R("SetWindowPos 3 %d %d %d %d\n",rc.left,rc.top,cw,ch);
    SetWindowPos(StemWin,0,rc.left,rc.top,cw,ch,0);
#elif defined(SSE_VID_DD_MISC)
    SetWindowPos(StemWin,0,0,0,rc.right,rc.bottom,SWP_NOZORDER);
#else
    SetWindowPos(ClipWin,0,0,MENUHEIGHT,rc.right,rc.bottom-MENUHEIGHT,SWP_NOZORDER);
#endif
  }
  if(DiskMan.IsVisible())
  {
    if(DiskMan.FSMaximized)
    {
      SetWindowPos(DiskMan.Handle,NULL,-GuiSM.cx_frame(),MENUHEIGHT,rc.right+GuiSM.cx_frame()*2,
        rc.bottom+GuiSM.cy_frame()-MENUHEIGHT,SWP_NOZORDER|SWP_NOACTIVATE);
    }
    else 
    {
      SetWindowPos(DiskMan.Handle,NULL,0,0,
        MIN(DiskMan.FSWidth,(int)rc.right),MIN(DiskMan.FSHeight,(int)rc.bottom-MENUHEIGHT),
        SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
    }
  }
#if defined(SSE_VID_DD)
  OptionBox.UpdateFullscreen();
#endif
  HDC DC=GetDC(StemWin);
  FillRect(DC,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
  ReleaseDC(StemWin,DC);
#endif//WIN32
  draw_grille_black=50; // Redraw black areas for 1 second
  return DD_OK;
}

#if 0 // this would put the window partly out of screen on border off
void change_window_size_for_border_change(int oldborder,int newborder) {
  if(ResChangeResize==0)
    return;
  if((newborder)&&!(oldborder))
    StemWinResize(-(16*4),-(BORDER_TOP*2));
  else if(!(newborder)&&(oldborder))
    StemWinResize((16*4),(BORDER_TOP*2));
}
#endif

Str SnapShotGetLastBackupPath() {
  if(has_extension(LastSnapShot,".sts")==0) 
    return ""; // Just in case folder
  Str Backup=WriteDir+SLASH+GetFileNameFromPath(LastSnapShot);
  char *ext=strrchr(Backup,'.');
  *ext=0;
  Backup+=".stsbackup";
  return Backup;
}


void SnapShotGetOptions(EasyStringList *p_sl) {
  p_sl->Sort=eslNoSort;
  p_sl->Add(T("&Load Memory Snapshot"),200,0);
  EasyStr NoSaveExplain="";
#ifndef DISABLE_STEMDOS
  if(on_rte!=ON_RTE_RTE)
    NoSaveExplain=T("The ST is in the middle of a disk operation");
  else if(stemdos_any_files_open())
    NoSaveExplain=T("The ST has file(s) open");
#endif
#if USE_PASTI
  if(NoSaveExplain.Empty())
  {
    if(hPasti && pasti_active) {
      NoSaveExplain=T("The ST is in the middle of a disk operation");
      pastiSTATEINFO psi;
      psi.bufSize=0;
      psi.buffer=NULL;
      psi.cycles=ABSOLUTE_CPU_TIME;
      pasti->SaveState(&psi);
      if(psi.bufSize>0) NoSaveExplain="";
    }
  }
#endif
  if(NoSaveExplain.IsEmpty())
  {
    p_sl->Add(T("&Save Memory Snapshot"),201,0);
    Str Name=GetFileNameFromPath(LastSnapShot);
    char *ext=strrchr(Name,'.');
    if(ext)
    {
      *ext=0;
      p_sl->Add("-",0,0);
      p_sl->Add(T("Save Over")+" "+Name,205,0);
      if(Exists(SnapShotGetLastBackupPath()))
        p_sl->Add(T("Undo Save Over")+" "+Name,206,0);
    }
  }
  else
  {
    p_sl->Add(T("Can't save snapshot because"),0,1);
    p_sl->Add(NoSaveExplain,0,1);
  }
  if(Exists(WriteDir+SLASH+"auto_reset_backup.sts"))
  {
    p_sl->Add("-",0,0);
    p_sl->Add(T("Undo Last Reset"),207,0);
  }
  if(Exists(WriteDir+SLASH+"auto_loadsnapshot_backup.sts"))
  {
    p_sl->Add("-",0,0);
    p_sl->Add(T("Undo Last Memory Snapshot Load"),208,0);
  }
  // Add history
  bool AddedLine=0;
  for(int n=0;n<10;n++)
  {
    if(StateHist[n].NotEmpty())
    {
      bool FileExists;
#ifdef WIN32
      FileExists=true;
      UINT HostDriveType=GetDriveType(StateHist[n].Lefts(2)+SLASH);
      if(HostDriveType==DRIVE_NO_ROOT_DIR)
        FileExists=0;
      else if(HostDriveType!=DRIVE_REMOVABLE && HostDriveType!=DRIVE_CDROM)
        FileExists=Exists(StateHist[n]);
#else
      FileExists=Exists(StateHist[n]);
#endif
      if(FileExists)
      {
        EasyStr Name=GetFileNameFromPath(StateHist[n]);
        char *dot=strrchr(Name,'.');
        if(dot)
          *dot=0;
        if(AddedLine==0)
        {
          p_sl->Add("-",0,0);
          AddedLine=true;
        }
        p_sl->Add(Name,210+n,0);
      }
    }
  }
}

#undef LOGSECTION

#ifdef UNIX

int StemWinProc(void*,Window Win,XEvent *Ev)
{
//	printf("%i\n",Ev->type);
#ifndef NO_SHM
  if (Ev->type==Disp.SHMCompletion){
    Disp.asynchronous_blit_in_progress=false;
  }else
#endif

  switch (Ev->type){
    case Expose:
    {
      XWindowAttributes wa;
      XGetWindowAttributes(XD,StemWin,&wa);

      hxc::clip_to_expose_rect(XD,&(Ev->xexpose),DispGC);

      if (Ev->xexpose.y+Ev->xexpose.height>MENUHEIGHT){
        draw_end();
        if (draw_blit()==0){
          XSetForeground(XD,DispGC,BlackCol);
          XFillRectangle(XD,StemWin,DispGC,2,MENUHEIGHT+2,
                          wa.width-4,wa.height-(MENUHEIGHT+4));
        }
        Disp.Surround();
      }

      XSetForeground(XD,DispGC,BkCol);
      XFillRectangle(XD,StemWin,DispGC,0,0,wa.width,MENUHEIGHT);
      XSetClipMask(XD,DispGC,None);

      XSync(XD,False);
      break;
    }
    case ButtonPress:
      if (Ev->xbutton.button==Button4 || Ev->xbutton.button==Button5) break;
      if (runstate==RUNSTATE_RUNNING && stem_mousemode==STEM_MOUSEMODE_DISABLED){
        if (Ev->xbutton.y>MENUHEIGHT){
          SetForegroundWindow(Win,Ev->xbutton.time);
          SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
        }
      }else if (runstate==RUNSTATE_STOPPED && StartEmuOnClick){
        PostRunMessage();
      }
    case ButtonRelease:
      break;
    case KeyPress:
    case KeyRelease:
    {
      bool Up=(Ev->type==KeyRelease);
      if (Up==0 && GetKeyState(Ev->xkey.keycode)<0){ //Key repeat
      	return PEEKED_MESSAGE;
      }
      SetKeyState(Ev->xkey.keycode,!Up);

      KeySym ks=XKeycodeToKeysym(XD,Ev->xkey.keycode,0);
      if (ks!=XK_Shift_L && ks!=XK_Shift_R &&
          ks!=XK_Control_L && ks!=XK_Control_R &&
          ks!=XK_Alt_L && ks!=XK_Alt_R){
#if defined(SSE_UNIX)
        if(Ev->xkey.keycode==VK_F11
        || Ev->xkey.keycode==Key_Pause)
#else
        if(Ev->xkey.keycode==Key_Pause)
#endif
        {
          if (Up==0) return PEEKED_MESSAGE;
          if (runstate==RUNSTATE_RUNNING){
            if (GetKeyStateSym(XK_Shift_R)<0 || GetKeyStateSym(XK_Shift_L)<0 || FullScreen){
              runstate=RUNSTATE_STOPPING;
              SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
              break;
            }else{
              if (stem_mousemode==STEM_MOUSEMODE_DISABLED){
                SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
              }else if (stem_mousemode==STEM_MOUSEMODE_WINDOW){
                SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
              }
            }
          }else if (runstate==RUNSTATE_STOPPED){
            return PEEKED_RUN;
          }
        }
#if defined(SSE_UNIX)/// && defined(SSE_GUI_F12)   
        else if(Ev->xkey.keycode==VK_F12 && !Up)
        {
          if (runstate==RUNSTATE_RUNNING)
          {
            runstate=RUNSTATE_STOPPING;
            SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
          }
          else if (runstate==RUNSTATE_STOPPED)
          {
            PostRunMessage();
          }
        }
#endif        
#if defined(SSE_UNIX) // F1 for help
        else if(Ev->xkey.keycode==VK_F1 && (runstate==RUNSTATE_STOPPED))
        {
          InfoBox.Page=0; // = about
          InfoBox.Hide(); // just in case
          InfoBox.Show();
        }
#endif        
        else if (joy_is_key_used(BYTE(Ev->xkey.keycode))==0 &&
                    CutDisableKey[BYTE(Ev->xkey.keycode)]==0){
          HandleKeyPress(Ev->xkey.keycode,Up,0);
        }
      }
      break;
    }
    case ClientMessage:
      if (Ev->xclient.message_type==hxc::XA_WM_PROTOCOLS){
        if (Atom(Ev->xclient.data.l[0])==hxc::XA_WM_DELETE_WINDOW){
          QuitSteem();
        }
      }else if (Ev->xclient.message_type==RunSteemAtom){
          if (runstate==RUNSTATE_STOPPED){
            return PEEKED_RUN;
          }else if (runstate==RUNSTATE_RUNNING){
            runstate=RUNSTATE_STOPPING;
            SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
          }
      }else if (Ev->xclient.message_type==LoadSnapShotAtom){
    		if (runstate==RUNSTATE_STOPPED){
          bool AddToHistory=true;
          Str fn=LastSnapShot;
          if (Ev->xclient.data.l[0]==207) fn=WriteDir+SLASH+"auto_reset_backup.sts", AddToHistory=0;
          if (Ev->xclient.data.l[0]==208) fn=WriteDir+SLASH+"auto_loadsnapshot_backup.sts", AddToHistory=0;
    	    LoadSnapShot(fn,AddToHistory);
          if (Ev->xclient.data.l[0]==207 || Ev->xclient.data.l[0]==208) DeleteFile(fn);
    	  }else{
    	  	runstate=RUNSTATE_STOPPING;

          XEvent SendEv;
          SendEv.type=ClientMessage;
          SendEv.xclient.window=StemWin;
          SendEv.xclient.message_type=LoadSnapShotAtom;
          SendEv.xclient.format=32;
          SendEv.xclient.data.l[0]=Ev->xclient.data.l[0];
          XSendEvent(XD,StemWin,0,0,&SendEv);
    	  }
      }
      break;
    case ConfigureNotify:
    {
      XWindowAttributes wa;
      XGetWindowAttributes(XD,StemWin,&wa);

      if (DiskBut.handle){
        XMoveWindow(XD,InfBut,wa.width-135,0);
        XMoveWindow(XD,PatBut,wa.width-112,0);
        XMoveWindow(XD,CutBut,wa.width-89,0);
        XMoveWindow(XD,OptBut,wa.width-66,0);
        XMoveWindow(XD,JoyBut,wa.width-43,0);
        XMoveWindow(XD,DiskBut,wa.width-20,0);
      }
      bool OldCanUse=CanUse_400;
      if (draw_grille_black<10) draw_grille_black=10;
      if (border & 1){
        CanUse_400=(wa.width>=(2+BORDER_SIDE*2+640+BORDER_SIDE*2+2) &&
                      wa.height>=(MENUHEIGHT + 2+BORDER_TOP*2+400+BORDER_BOTTOM*2+2));
      }else{
        CanUse_400=(wa.width>=(2+640+2) && wa.height>=(MENUHEIGHT+2+400+2));
      }
      if (OldCanUse!=CanUse_400 && FullScreen==0){
        draw_end();
        draw(0);
      }
      x_draw_surround_count=MAX(x_draw_surround_count,10);
      break;
    }
    case SelectionNotify:
      if (Ev->xselection.property!=None){
        if (Ev->xselection.target==XA_STRING){
          Atom actual_type_return;
          int actual_format_return;
          unsigned long nitems_return;
          unsigned long bytes_after_return;
          char *t;
          XGetWindowProperty(XD,Win,Ev->xselection.property,
                        /*long_offset*/ 0, /*long_length*/ 5000,
                        /*delete*/ True, XA_STRING,
                        &actual_type_return, &actual_format_return,
                        &nitems_return, &bytes_after_return,
                        (BYTE**)(&t));
          if (actual_type_return==XA_STRING){
            PasteText=t;
            PasteVBLCount=PasteSpeed;
            PasteBut.set_check(true);
            XFree(t);
          }
        }
      }
      break;
    case FocusIn:
      bAppActive=true;
      XAutoRepeatOff(XD);
      break;
    case FocusOut:
    {
      Window Foc=0;
      int RevertTo;
      XGetInputFocus(XD,&Foc,&RevertTo);
    	if (Foc!=StemWin){
				XAutoRepeatOn(XD);
        SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
        ZeroMemory(KeyState,sizeof(KeyState));
        UpdateSTKeys();
      	bAppActive=0;
      }
      break;
    }
    case MapNotify:
	  	bAppActive=true;
    	bAppMinimized=0;
    	break;
    case UnmapNotify:
    	bAppMinimized=true;
    	break;
    case DestroyNotify:
      StemWin=0;
      QuitSteem();
      return PEEKED_QUIT;
  }
  return PEEKED_MESSAGE;
}
//---------------------------------------------------------------------------
int snapshot_parse_filename(char*fn,struct stat*s)
{
  if (S_ISDIR(s->st_mode)) return FS_FTYPE_FOLDER;
  if (has_extension(fn,".STS")){
    return FS_FTYPE_FILE_ICON+ICO16_SNAPSHOTS;
  }
  return FS_FTYPE_REJECT;
}
//---------------------------------------------------------------------------
void SnapShotProcess(int i)
{
  bool WaitUntilStopped=0;
  if (i==200 /* Load Snapshot */ || i==201 /* Save Snapshot */){
    fileselect.set_corner_icon(&Ico16,ICO16_SNAPSHOT);
    Str LastSnapShotFol=LastSnapShot;
    RemoveFileNameFromPath(LastSnapShotFol,REMOVE_SLASH);
    EasyStr fn=fileselect.choose(XD,LastSnapShotFol,GetFileNameFromPath(LastSnapShot),
                T("Memory Snapshots"),((i==200) ? FSM_LOAD:FSM_SAVE) | FSM_LOADMUSTEXIST |
                FSM_CONFIRMOVERWRITE,snapshot_parse_filename,".sts");
    if (fileselect.chose_option==FSM_LOAD){
      LastSnapShot=fn;
      WaitUntilStopped=true;
    }else if (fileselect.chose_option==FSM_SAVE){
      LastSnapShot=fn;
      SaveSnapShot(fn,-1);
    }
  }else if (i==205){ // Save over last
    if (SnapShotGetLastBackupPath().NotEmpty()){
      // Make backup, could be on different drive so do it the slow way
      copy_file_byte_by_byte(LastSnapShot,SnapShotGetLastBackupPath());
    }
    SaveSnapShot(LastSnapShot,-1);
  }else if (i==206){ // Undo save over
    // Restore backup, can only get here if backup path is valid
    copy_file_byte_by_byte(SnapShotGetLastBackupPath(),LastSnapShot);
    remove(SnapShotGetLastBackupPath());
  }else if (i>=210 && i<220){ // Load recent
    LastSnapShot=StateHist[i-210];
    WaitUntilStopped=true;
  }else if (i==207 || i==208){ // undo reset/last snap
    WaitUntilStopped=true;
  }
  if (WaitUntilStopped){
    if (runstate==RUNSTATE_RUNNING) runstate=RUNSTATE_STOPPING;

    XEvent SendEv;
    SendEv.type=ClientMessage;
    SendEv.xclient.window=StemWin;
    SendEv.xclient.message_type=LoadSnapShotAtom;
    SendEv.xclient.format=32;
    SendEv.xclient.data.l[0]=i;
    XSendEvent(XD,StemWin,0,0,&SendEv);
  }
}
//---------------------------------------------------------------------------
int stemwin_popup_notify(hxc_popup *pop,int mess,int idx)
{
	if (mess==POP_CHOOSE){
    int i=pop->menu[idx].Data[1];
    if (i>=100 && i<200){
      PasteSpeed=(i-100)+1;
    }else if (i>=200 && i<300){
      SnapShotProcess(i);
    }
  }
	SnapShotBut.set_check(0);
	PasteBut.set_check(PasteText.NotEmpty());
	return 0;
}

int StemWinButtonNotifyProc(hxc_button *But,int Mess,int *Inf)
{
  switch (But->id){
    case 101:
      if (Inf[0]==Button3){
      	slow_motion_change(Mess==BN_DOWN);
      }else if (Mess==BN_CLICKED){
	      PostRunMessage();
	    }
      break;
    case 109:
      if (Mess!=BN_DOWN && Mess!=BN_UP) break;
      
      if (Mess==BN_DOWN){
        if (fast_forward_stuck_down){
          fast_forward_stuck_down=0;
          Mess=BN_UP;
        }else{
          if (DWORD(Inf[1])<ff_doubleclick_time){
            fast_forward_stuck_down=true;
            ff_doubleclick_time=0;
          }else{
            ff_doubleclick_time=DWORD(Inf[1])+FF_DOUBLECLICK_MS;
          }
        }
      }else{
        if (fast_forward_stuck_down) break;
      }
      fast_forward_change(Mess==BN_DOWN,Inf[0]==Button3);
      break;
    case 102:
    {

#if defined(SSE_UNIX) && defined(SSE_GUI_RESET_BUTTON)
      bool Warm=!(Inf[0]==Button3);
#else
      bool Warm=(Inf[0]==Button3);
#endif
      reset_st(DWORD(Warm ? RESET_WARM:RESET_COLD) | DWORD(Warm ? RESET_NOSTOP:RESET_STOP) |
                  RESET_CHANGESETTINGS | RESET_BACKUP);
      break;
    }
    case 108: // Memory Snapshots
    	if (Mess==BN_CLICKED){
        But->set_check(true);

        EasyStringList sl;
        SnapShotGetOptions(&sl);

    		pop.lpig=NULL;
        pop.menu.DeleteAll();
        for (int i=0;i<sl.NumStrings;i++){
          Str Text=sl[i].String;
          while (Text.InStr("&")>=0) Text.Delete(Text.InStr("&"),1);
          pop.menu.Add(Text,-1,sl[i].Data[0]);
        }
        pop.create(XD,But->handle,0,But->h,stemwin_popup_notify,NULL);
      }
      break;
    case 116:
      if (runstate==RUNSTATE_RUNNING){
        DoSaveScreenShot|=1;
      }else{
        Disp.SaveScreenShot();
      }
      break;
    case 114: // Paste
      if (Inf[0]==Button3){
      	if (Mess==BN_CLICKED){
      		But->set_check(true);
      		pop.lpig=&Ico16;
	      	pop.menu.DeleteAll();
	        for (int n=0;n<11;n++){
	        	long ico=ICO16_UNRADIOMARKED;
	        	if (PasteSpeed==(1+n)) ico=ICO16_RADIOMARK;
	        	pop.menu.Add(T("Delay")+" - "+n,ico,100+n);
	        }
	        pop.create(XD,But->handle,0,But->h,stemwin_popup_notify,NULL);
	      }
      }else{
		    PasteIntoSTAction(STPASTE_TOGGLE);
		  }
      break;
    case 115:
      Disp.GoToFullscreenOnRun=But->checked;
      if (runstate==RUNSTATE_RUNNING){
        runstate=RUNSTATE_STOPPING;
        RunWhenStop=true;
      }
      break;

    case 100: //DiskMan
      DiskMan.ToggleVisible();
      break;
    case 103: //Joy
      JoyConfig.ToggleVisible();
      break;
    case 105:
      InfoBox.ToggleVisible();
      break;
    case 107:
      OptionBox.ToggleVisible();
      break;
    case 112:
      ShortcutBox.ToggleVisible();
      break;
    case 113:
      PatchesBox.ToggleVisible();
      break;
    case 120: // AutoUpdate
      break;
  }
  return 0;
}
//---------------------------------------------------------------------------
int timerproc(void*,Window,int id)
{
  if (id==SHORTCUTS_TIMER_ID){
    JoyGetPoses();
    ShortcutsCheck();
  }
  return HXC_TIMER_REPEAT;
}

#endif//UNIX
