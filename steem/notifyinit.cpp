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
FILE: notifyinit.cpp
DESCRIPTION: The window that appears while Steem is initialising.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <conditions.h>
#include <steemh.h>
#include <notifyinit.h>
#include <options.h>
#include <translate.h>
#include <gui.h>
#include <resnum.h>
#include <mymisc.h>

#ifdef WIN32
LRESULT CALLBACK NotifyInitWndProc(HWND,UINT,WPARAM,LPARAM);
HWND NotifyWin=NULL;
ONEGAME_ONLY(EXT HWND NotifyWinParent; )
#endif


#ifndef ONEGAME
#define NOTIFYINIT_WIDTH 250
#define NOTIFYINIT_HEIGHT 120
#else
#define NOTIFYINIT_WIDTH (GetSystemMetrics(SM_CXSCREEN))
#define NOTIFYINIT_HEIGHT (GetSystemMetrics(SM_CYSCREEN))
#endif

void CreateNotifyInitWin(char* sCaption) {

#ifdef WIN32
  WNDCLASS wc;
  wc.style=CS_NOCLOSE;
  wc.lpfnWndProc=NotifyInitWndProc;
  wc.cbWndExtra=0;
  wc.cbClsExtra=0;
  wc.hInstance=Inst;
  wc.hIcon=hGUIIcon[RC_ICO_APP];
  wc.hCursor=LoadCursor(NULL,IDC_WAIT);
#ifndef ONEGAME
  wc.hbrBackground=(HBRUSH)((COLOR_BTNFACE)+1);
#else
  wc.hbrBackground=NULL;
#endif
  wc.lpszMenuName=NULL;
  wc.lpszClassName="Steem Init Window";
  RegisterClass(&wc);
#ifndef ONEGAME
  NotifyWin=CreateWindow("Steem Init Window",sCaption,WS_SYSMENU,
    0,0,NOTIFYINIT_WIDTH,NOTIFYINIT_HEIGHT,NULL,NULL,Inst,NULL);
  CentreWindow(NotifyWin,0);
  SetWindowPos(NotifyWin,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
  UpdateWindow(NotifyWin);
#else
  NotifyWinParent=CreateWindow("Steem Init Window","",0,0,0,2,2,NULL,NULL,Inst,NULL);
  NotifyWin=CreateWindowEx(WS_EX_TOPMOST,"Steem Init Window","",WS_POPUP,0,0,NOTIFYINIT_WIDTH,NOTIFYINIT_HEIGHT,
    NotifyWinParent,NULL,Inst,NULL);
  SetWindowLong(NotifyWin,GWL_STYLE,WS_POPUP);
  MoveWindow(NotifyWin,0,0,NOTIFYINIT_WIDTH,NOTIFYINIT_HEIGHT,0);
  ShowWindow(NotifyWin,SW_SHOW);
  SetWindowPos(NotifyWin,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
  UpdateWindow(NotifyWin);
  SetCursor(NULL);
#endif
#endif//WIN32
}


#if defined(WIN32) && !defined(ONEGAME) && !defined(_DEBUG)

void SetNotifyInitText(char* NewText) {
  if(NotifyWin) 
  {
    SendMessage(NotifyWin,WM_USER,12345,(LPARAM)NewText);
    UpdateWindow(NotifyWin);
  }
}

#else

void SetNotifyInitText(char *) {}

#endif


#ifdef WIN32

LRESULT CALLBACK NotifyInitWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar) {
  switch(Mess) {
#ifndef ONEGAME
  case WM_CREATE:
  {
    char *Text=new char[200];
    strcpy(Text,T("Please wait..."));
    SetProp(Win,"NotifyText",Text);
    break;
  }
  case WM_PAINT:
  {
    HDC DC;
    RECT rc;
    char *Text;
    SIZE sz;
    GetClientRect(Win,&rc);
    DC=GetDC(Win);
#if defined(SSE_GUI_FONT_FIX)
    SelectObject(DC,SSEConfig.GuiFont());
#else
    SelectObject(DC,GetStockObject(DEFAULT_GUI_FONT));
#endif
    HBRUSH br=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(DC,&rc,br);
    DeleteObject(br);
    SetBkMode(DC,TRANSPARENT);
    Text=(char*)GetProp(Win,"NotifyText");
    GetTextExtentPoint32(DC,Text,(int)strlen(Text),&sz);
    TextOut(DC,(rc.right-sz.cx)/2,(rc.bottom-sz.cy)/2,Text,(int)strlen(Text));
    ReleaseDC(Win,DC);
    ValidateRect(Win,NULL);
    return 0;
  }
  case WM_USER:
    if(wPar==12345)
    {
      char *Text=(char*)GetProp(Win,"NotifyText"),*NewText=(char*)lPar;
      delete[] Text;
      Text=new char[strlen(NewText)+1];
      strcpy(Text,NewText);
      SetProp(Win,"NotifyText",Text);
      InvalidateRect(Win,NULL,1);
    }
    break;
  case WM_DESTROY:
    delete[](char*)GetProp(Win,"NotifyText");
    RemoveProp(Win,"NotifyText");
    break;
  default:
    break;
#else // "one game"
  case WM_PAINT:
  {
    HDC DC=GetDC(Win);
    RECT rc;
    GetClientRect(Win,&rc);
    FillRect(DC,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
    ReleaseDC(Win,DC);
    ValidateRect(Win,NULL);
    return 0;
  }
  case WM_SETCURSOR:
    SetCursor(NULL);
    return TRUE;
#endif
  }

  return DefWindowProc(Win,Mess,wPar,lPar);
}

#endif//WIN32

void DestroyNotifyInitWin() {

#ifdef WIN32
  if(NotifyWin==NULL)
    return;
  ShowWindow(NotifyWin,SW_HIDE);
  UpdateWindow(NotifyWin);
  DestroyWindow(NotifyWin);
  ONEGAME_ONLY(DestroyWindow(NotifyWinParent); )
  NotifyWin=NULL;
  UnregisterClass("Steem Init Window",Inst);
#endif//WIN32

}


// struct TNotify for RAII use

#ifdef WIN32

TNotify::TNotify(char *caption) {
  if(!SSEConfig.ShowNotify)
    m_OurOwn=false;
  else if(NotifyWin==NULL)
  {
    CreateNotifyInitWin(caption);
    m_OurOwn=true;
  }
  else
  {
    m_sPreviousCaption=(char*)GetProp(NotifyWin,"NotifyText");
    SetNotifyInitText(caption);
    m_OurOwn=false;
  }
}


TNotify::~TNotify() {
  if(m_OurOwn)
    DestroyNotifyInitWin();
  else
    SetNotifyInitText(m_sPreviousCaption);
}

#endif//WIN32
