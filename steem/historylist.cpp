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

DOMAIN: Debug
FILE: historylist.cpp
CONDITION: DEBUG_BUILD must be defined
DESCRIPTION: The history list window in the debug build that shows a list of
recently executed commands.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#ifdef DEBUG_BUILD

#include <steemh.h>
#include <debug_emu.h>
#include <debugger.h>
#include <debug.h>
#include <acc.h>
#include <mymisc.h>
#include <resnum.h>
#include <emulator.h>
#include <mmu.h>
#include <d2.h>
#include <gui.h>

THistoryList::THistoryList() {
  Width=300+GuiSM.cx_frame()*2;
  Height=300+GuiSM.cy_frame()*2+GuiSM.cy_caption();
  Left=(GuiSM.cx_screen()-Width)/2;
  Top=(GuiSM.cy_screen()-Height)/2;
}


void THistoryList::ManageWindowClasses(bool Unreg) {
  char *ClassName="Steem History List";
  if(Unreg)
    UnregisterClass(ClassName,Inst);
  else
    RegisterMainClass(WndProc,ClassName,RC_ICO_STCLOSE);
}


THistoryList::~THistoryList() {
  if(IsVisible()) 
    Hide();
}


void THistoryList::Show() {
  if(Handle!=NULL)
  {
    ShowWindow(Handle,SW_SHOWNORMAL);
    SetForegroundWindow(Handle);
    return;
  }

  ManageWindowClasses(SD_REGISTER);
  // can be on taskbar:
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT|mem_browser::ex_style,//WS_EX_TOOLWINDOW,
    "Steem History List","History List",
    WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_SIZEBOX|WS_MAXIMIZEBOX,
    Left,Top,Width,Height,
    NULL,NULL,HInstance,NULL);
  if(HandleIsInvalid())
  {
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }
  SetWindowLongPtr(Handle,GWLP_USERDATA,(LONG_PTR)this);
  HWND hLB=CreateWindowEx(512,"ListBox","History",WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_USETABSTOPS|
    WS_TABSTOP|LBS_NOINTEGRALHEIGHT|LBS_HASSTRINGS|LBS_NOTIFY,
    10,10,280,250,Handle,(HMENU)100,HInstance,NULL);
  INT Tabs[1]={(GetTextSize(Font,"X  ").Width*4)/LOWORD(GetDialogBaseUnits())};
  SendMessage(hLB,LB_SETTABSTOPS,1,LPARAM(Tabs));
#if defined(SSE_DEBUGGER_HISTORY_DUMP)
  CreateWindow("Button","Dump to Trace",WS_CHILDWINDOW|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
    10,267,135,23,Handle,(HMENU)101,HInstance,NULL);
#else
  CreateWindow("Button","Toggle Breakpoint",WS_CHILDWINDOW|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
    10,267,135,23,Handle,(HMENU)101,HInstance,NULL);
#endif
  CreateWindow("Button","Refresh",WS_CHILDWINDOW|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
    155,267,135,23,Handle,(HMENU)102,HInstance,NULL);

  SetWindowAndChildrensFont(Handle,Font);

  RefreshHistoryBox();

  Focus=GetDlgItem(Handle,100);

  ShowWindow(Handle,SW_SHOW);
  SetFocus(Focus);
}


void THistoryList::Hide() {
  if(Handle==NULL) 
    return;

  ShowWindow(Handle,SW_HIDE);

  DestroyWindow(Handle);Handle=NULL;
  ManageWindowClasses(SD_UNREGISTER);
}


void THistoryList::RefreshHistoryBox(bool dump) {
  HWND Win=GetDlgItem(Handle,100);
#if defined(SSE_DEBUGGER_HISTORY_DUMP)
  if(!dump) // if dump is true, we don't actually refresh
#endif
    SendMessage(Win,LB_RESETCONTENT,0,0);
  int n=pc_history_idx;
  EasyStr Disassembly;
#if defined(SSE_DEBUGGER_BIG_HISTORY)
  int nentries=(DEBUGGER_CONTROL_MASK1&DEBUGGER_CONTROL_LARGE_HISTORY)
    ? HISTORY_SIZE : 1000;
  int nentries2=nentries;
#endif
  do // go back to first instruction to display/dump
  {
    n--;
    if(n<0) 
      n=HISTORY_SIZE-1;
#if defined(SSE_DEBUGGER_BIG_HISTORY)
    nentries2--;
    if(!nentries2)
      break;
#endif
    if(pc_history[n]==0xffffff71) 
      break;
  } while(n!=pc_history_idx);
  int i=0;
  do
  {
    if(pc_history[n]==0xffffff71) 
      break;
    EasyStr header;
    if(pc_history[n]==LPEEK(0x120))
      header="TB ";
    else if(pc_history[n]==LPEEK(0x118))
      header="Acia ";
    else if(pc_history[n]==LPEEK(0x24))
      header="trc ";
    else if(pc_history[n]==LPEEK(0x10))
      header="Ill ";
    else if(pc_history[n]==LPEEK(0xC))
      header="AddE ";
    else if(pc_history[n]==LPEEK(0x8))
      header="BusE ";
    else if(pc_history[n]==LPEEK(0x70))
      header="VBi ";
    else if(pc_history[n]==LPEEK(0x68))
      header="HBi ";
    else if(pc_history[n]==LPEEK(0x114))
      header="TC ";
    else if(pc_history[n]==LPEEK(0x134))
      header="TA ";
    else if(pc_history[n]==LPEEK(0x110))
      header="TD ";
    else if(pc_history[n]==LPEEK(0x18))
      header="chk ";
    else if(pc_history[n]==LPEEK(0x2C))
      header="LF ";
    else if(pc_history[n]==LPEEK(0x28))
      header="LA ";
    else if(pc_history[n]==LPEEK(0x11c))
      header="fdc ";

    if(pc_history[n]==0x98764321)
      Disassembly="BLiT";
    else if(pc_history[n]==0x12346789)
      Disassembly="DMA";
    else if((pc_history[n]&0xFF0000FF)==0x99000001)
      Disassembly=Str("irq ")+Str((pc_history[n]>>16)&0xff)+"-"
      +Str((pc_history[n]>>8)&0xFF);
    else
      Disassembly=debug_parse_disa_for_display(disa_d2(pc_history[n]));

    char timing[30];
    sprintf(timing,"%d %d",pc_history_y[n],pc_history_c[n]);
#if defined(SSE_DEBUGGER_HISTORY_DUMP)
    if(dump)
    {
      char tmp[256];
      if(DEBUGGER_CONTROL_MASK1&DEBUGGER_CONTROL_HISTORY_TMG)
        sprintf(tmp,"%s%s - %s - %s",header.Text,HEXSl(pc_history[n],6).Text,timing,Disassembly.Text);
      else
        sprintf(tmp,"%s%s - %s",header.Text,HEXSl(pc_history[n],6).Text,Disassembly.Text);
      TRACE("%s\n",tmp);
    }
    else
#endif
    {
      if(DEBUGGER_CONTROL_MASK1&DEBUGGER_CONTROL_HISTORY_TMG)
        SendMessage(Win,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)((header+HEXSl(pc_history[n],6)
          +" - "+timing+" - "+Disassembly).Text)); //-1 end of list
      else
        SendMessage(Win,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)((header+HEXSl(pc_history[n],6)
          +" - "+Disassembly).Text));
    }
    SendMessage(Win,LB_SETITEMDATA,i,n);
    n++;
    if(n==HISTORY_SIZE)
      n=0;
    i++;
#if defined(SSE_DEBUGGER_BIG_HISTORY)
    nentries--;
    if(!nentries)
      break;
#endif
  } while(n!=pc_history_idx);

  RECT rc;
  GetClientRect(Win,&rc);
  LONG_PTR Selected=SendMessage(Win,LB_GETCOUNT,0,0)-1;
  SendMessage(Win,LB_SETCURSEL,Selected,0);
  SendMessage(Win,LB_SETTOPINDEX,MAX(Selected-(rc.bottom/
    SendMessage(Win,LB_GETITEMHEIGHT,0,0)),(LONG_PTR)0),0);
  SendMessage(Win,WM_SETFONT,
    WPARAM(debug_monospace_disa?GetStockObject(ANSI_FIXED_FONT):Font),TRUE);
}


#define GET_THIS This=(THistoryList*)GetWindowLongPtr(Win,GWLP_USERDATA);

LRESULT CALLBACK THistoryList::WndProc(HWND Win,UINT Mess,
                                        WPARAM wPar,LPARAM lPar) {
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if(StemDialog_RetDefVal) 
    return Ret;
  THistoryList *This;
  WORD wpar_lo=LOWORD(wPar);
  WORD wpar_hi=HIWORD(wPar);
  switch(Mess) {
  case WM_COMMAND:
    if(wpar_lo==100)
    {
      if(wpar_hi==LBN_DBLCLK)
      {
        int Selected=(int)SendMessage(GetDlgItem(Win,100),LB_GETCURSEL,0,0);
        PostMessage(DWin,WM_COMMAND,17000+SendMessage(GetDlgItem(Win,100),
          LB_GETITEMDATA,Selected,0),0);
      }
    }
    else if(wpar_lo==101)
    {
      if(wpar_hi==BN_CLICKED)
      {
#if defined(SSE_DEBUGGER_HISTORY_DUMP)
        GET_THIS;
        This->RefreshHistoryBox(true);
#else
        HWND HistBox=GetDlgItem(Win,100);
        int Selected=SendMessage(HistBox,LB_GETCURSEL,0,0);
        if(Selected>-1)
        {
          int SelHistNum=SendMessage(HistBox,LB_GETITEMDATA,Selected,0);
          EasyStr Text;
          Text.SetLength(SendMessage(HistBox,LB_GETTEXTLEN,Selected,0)+1);
          SendMessage(HistBox,LB_GETTEXT,Selected,(LPARAM)Text.Text);
          SendMessage(HistBox,LB_DELETESTRING,Selected,0);
          SendMessage(HistBox,LB_INSERTSTRING,Selected,(LPARAM)Text.Text);
          SendMessage(HistBox,LB_SETITEMDATA,Selected,SelHistNum);
          SendMessage(HistBox,LB_SETCURSEL,Selected,0);
        }
#endif
      }
    }
    else if(wpar_lo==102)
    {
      if(wpar_hi==BN_CLICKED)
      {
        GET_THIS;
        This->RefreshHistoryBox();
      }
    }
    break;
  case WM_SIZE:
    SetWindowPos(GetDlgItem(Win,100),0,0,0,LOWORD(lPar)-20,HIWORD(lPar)-50,SWP_NOMOVE|SWP_NOZORDER);
    SetWindowPos(GetDlgItem(Win,101),0,10,HIWORD(lPar)-30,(LOWORD(lPar)/2)-15,23,SWP_NOZORDER);
    SetWindowPos(GetDlgItem(Win,102),0,LOWORD(lPar)/2+5,HIWORD(lPar)-30,(LOWORD(lPar)/2)-15,23,SWP_NOZORDER);

    GET_THIS;
    if(IsIconic(Win)==0)
    {
      if(IsZoomed(Win)==0)
      {
        This->Maximized=0;

        RECT rc;GetWindowRect(Win,&rc);
        This->Left=rc.left;This->Top=rc.top;
        This->Width=rc.right-rc.left;This->Height=rc.bottom-rc.top;
      }
      else
      {
        This->Maximized=true;
      }
    }
    break;
  case WM_GETMINMAXINFO:
    ((MINMAXINFO*)lPar)->ptMinTrackSize.x=200+GuiSM.cx_frame()*2+GuiSM.cx_vscroll();
    ((MINMAXINFO*)lPar)->ptMinTrackSize.y=100+GuiSM.cy_caption()+GuiSM.cy_frame()*2;
    break;
  case WM_CLOSE:
    GET_THIS;
    This->Hide();
    return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}

#undef GET_THIS

#endif//DEBUG_BUILD
