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

FILE: dwin_edit.cpp
CONDITION: DEBUG_BUILD must be defined
DESCRIPTION: The edit box that is used to change values in the debugger.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#ifdef DEBUG_BUILD

#include <dwin_edit.h>
#include <debugger.h>
#include <mr_static.h>
#include <mymisc.h>
#include <d2.h>
#include <computer.h>
#include <interface_stvl.h>

void* DWin_edit_subject;
int DWin_edit_subject_type;
int DWin_edit_subject_index;
int DWin_edit_subject_col;
MEM_ADDRESS DWin_edit_subject_ad;
EDWin_edit_subject_content DWin_edit_subject_content;
bool DWin_edit_is_being_temporarily_defocussed=false;
WNDPROC Old_edit_WndProc;


void DWin_edit_pressed_return() {
  char tb[256],tb2[256];
  SendMessage(DWin_edit,WM_GETTEXT,250,(LPARAM)tb);
  if(DWin_edit_subject_type==0)
  { //mr_static;
    mr_static *ms=(mr_static*)DWin_edit_subject;
    if(ms->active && IsWindow(ms->handle)&&IsWindow(ms->owner))
    {
      ms->edit(tb);
      SendMessage(ms->handle,WM_GETTEXT,250,(LPARAM)tb);
      SetWindowText(DWin_edit,tb);
      mem_browser_update_all();
    }
    else
      DWin_edit_subject_type=-1;
  }
  else if(DWin_edit_subject_type==1)
  { //inside mem_disa;
    mem_browser *mb=(mem_browser*)DWin_edit_subject;
    if(mb->handle && IsWindow(mb->handle)&&IsWindow(mb->owner))
    {
      MEM_ADDRESS ad=DWin_edit_subject_ad;
      if(DWin_edit_subject_content==DWESC_HEX)
      {
        if(mb->disp_type==DT_REGISTERS)
        {
          int n=DWin_edit_subject_index+(mb->ad);
          MEM_ADDRESS *ptr=reg_browser_entry_pointer[n];
          int c2=0;
          for(int c=0;tb[c];c++)
            if(tb[c]!=' ')
              tb2[c2++]=tb[c];
          tb2[c2]=0;
          *ptr=HexToVal(tb2);
        }
        else if(mb->mode==MB_MODE_IOLIST)
        {
          Tiolist_entry *iol=search_iolist(ad);
          if(iol) 
            if(iol->ptr) 
              *(iol->ptr)=LOBYTE(HexToVal(tb));
        }
        else
        {
          int c=0;
          while(tb[c])
          {
            while(tb[c]==' ') 
              c++;
            if(tb[c]==0) 
              break;
            int c2=0;
            while(tb[c]!=' ' && tb[c]!=0) 
              tb2[c2++]=tb[c++];
            tb2[c2]=0;
            int val=HexToVal(tb2);
            d2_dpoke(ad,(WORD)val);
            if(ad>=pc && ad<=pc+4)
            {
              SET_PC(pc); // Refresh prefetch
            }
            ad+=2;
//            if((int)ad-(int)DWin_edit_subject_ad>=2*wpl)break;
          }
        }
      }
      DWin_edit_subject_type=-1;
      ShowWindow(DWin_edit,SW_HIDE);
      if(mb->disp_type==DT_REGISTERS)
        update_register_display(false);
      else
        mb->update();
    }
  }
#if defined(SSE_VID_STVL2) // a bit silly
  Stvl.vbase.d32=vbase;
  Stvl.linewid=Mmu.linewid;
  Stvl.hscroll=shifter_hscroll;
  Stvl.hscroll_complement=16-shifter_hscroll;
  Stvl.noscroll=(shifter_hscroll==0);
  Stvl.shift_mode=Shifter.m_ShiftMode;
  Stvl.mde0=(Glue.m_ShiftMode&1);
  Stvl.mde1=((Glue.m_ShiftMode&2)==2);
  Stvl.exts=(Glue.m_SyncMode&1);
  Stvl.pal=((Glue.m_SyncMode&2)==2);
#endif
  SendMessage(DWin_edit,EM_SETSEL,0,-1);
}


void set_DWin_edit(int type,void*subject,int n,int col) {
  char ttt[1024];
  RECT rc;POINT pt;
  if(DWin_edit_subject_type!=-1) 
    DWin_edit_finish(true);
  if(type==0)
  {
    DWin_edit_subject_type=0;
    DWin_edit_subject=subject;
    DWin_edit_subject_content=DWESC_HEX;
    mr_static*ms=(mr_static*)subject;
    HWND win=ms->handle;
    SetParent(DWin_edit,ms->owner);
    GetWindowRect(win,&rc);
    pt.x=rc.left;pt.y=rc.top;ScreenToClient(ms->owner,&pt);
    MoveWindow(DWin_edit,pt.x,pt.y,rc.right-rc.left,rc.bottom-rc.top,true);
    GetWindowText(win,ttt,1024);
    SetWindowText(DWin_edit,ttt);
    ShowWindow(win,SW_HIDE);
    ShowWindow(DWin_edit,SW_SHOW);
    SetFocus(DWin_edit);
    SendMessage(DWin_edit,EM_SETSEL,0,-1);
  }
  else if(type==1)
  {
    mem_browser *mb=(mem_browser*)subject;
//    HWND win=mb->handle;
    LV_ITEM item;
    int c;
    SetParent(DWin_edit,mb->owner);
    DWin_edit_subject_type=1;
    DWin_edit_subject_col=col;
    DWin_edit_subject_index=n;
    DWin_edit_subject=subject;
    if(mb->disp_type!=DT_REGISTERS)
    {
      item.iItem=n;
      item.iSubItem=3;
      item.mask=LVIF_TEXT;
      item.pszText=ttt;
      item.cchTextMax=1024;
      SendMessage(mb->handle,LVM_GETITEM,0,(LPARAM)&item);
      DWin_edit_subject_ad=HexToVal(ttt); //(mb->ad)+n*2*(mb->wpl);
      c=col;
    }
    else
      c=col+1;
    if(c==4) 
      DWin_edit_subject_content=DWESC_HEX;
    item.iItem=n;
    item.iSubItem=col;
    item.mask=LVIF_TEXT;
    item.pszText=ttt;
    item.cchTextMax=1024;
    SendMessage(mb->handle,LVM_GETITEM,0,(LPARAM)&item);
    char *end=strrchr(ttt,'\01');
    if(end) 
      *end=0;
    SetWindowText(DWin_edit,ttt);
//    rc.left=LVIR_LABEL;
//    SendMessage(mb->handle,LVM_GETITEMRECT,n,(LPARAM)&rc);
    int left_offset=4; //rc.left;
    rc.left=LVIR_BOUNDS;
    SendMessage(mb->handle,LVM_GETITEMRECT,n,(LPARAM)&rc);
    int w=(int)SendMessage(mb->handle,LVM_GETCOLUMNWIDTH,col,0);
    //adjust rc.left and rc.right by column index
    int cx=0;
    for(c=0;c<col;c++) 
      cx+=(int)SendMessage(mb->handle,LVM_GETCOLUMNWIDTH,c,0);
    rc.left+=cx;rc.right+=cx;
    pt.x=rc.left;pt.y=rc.top;
    ClientToScreen(mb->handle,&pt);
    ScreenToClient(mb->owner,&pt);
    MoveWindow(DWin_edit,pt.x-2+left_offset,pt.y-3,w+4-left_offset,rc.bottom
      -rc.top+5,true);
//    ShowWindow(win,SW_HIDE);
    ShowWindow(DWin_edit,SW_SHOW);
    SetForegroundWindow(DWin_edit);
    SetFocus(DWin_edit);
    SendMessage(DWin_edit,EM_SETSEL,0,-1);
  }
}


void DWin_edit_finish(bool record_changes) {
  if(record_changes)
    DWin_edit_pressed_return();
  if(DWin_edit_subject_type==0)
    ShowWindow(((mr_static*)DWin_edit_subject)->handle,SW_SHOW);
  ShowWindow(DWin_edit,SW_HIDE);
  DWin_edit_subject=NULL;
  DWin_edit_subject_type=-1;
}


LRESULT CALLBACK DWin_edit_WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar) {
//  char tb[256];
  switch(Mess) {
  case WM_CONTEXTMENU: 
  {
    if(DWin_edit_subject_type==0)
    {
      mr_static *ms=(mr_static*)DWin_edit_subject;
      DWin_edit_finish(true);
      ms->setup_contextmenu();
      TrackPopupMenu(insp_menu,TPM_LEFTALIGN|TPM_LEFTBUTTON,
        LOWORD(lPar),HIWORD(lPar),0,DWin,NULL);
      return 0;
    }
    break;
  }
  case WM_KEYDOWN:
    switch(wPar) {
    case 13:
      DWin_edit_pressed_return();
      return 0;
//        break;
    case VK_ESCAPE:
      DWin_edit_finish(false);
      break;
    case VK_UP:case VK_PRIOR:case VK_DOWN:case VK_NEXT:
      if(DWin_edit_subject_type==1)
      {
        mem_browser *mb=(mem_browser*)DWin_edit_subject;
        int col=DWin_edit_subject_col,index=DWin_edit_subject_index;
        DWin_edit_finish(true);
        if((mb->mode)==MB_MODE_STANDARD)
          mem_browser_WndProc(mb->handle,Mess,wPar,lPar);
        else
        {
          switch(wPar) {
          case VK_UP:index--;break;
          case VK_DOWN:index++;break;
          case VK_PRIOR:index=0;break;
          case VK_NEXT:index=(mb->lb_height)-1;break;
          }
          index=MAX(0,MIN(index,(mb->lb_height)-1));
        }
        if(mb->disp_type!=DT_REGISTERS
          ||(mb->ad)+index<NUM_REGISTERS_IN_REGISTER_BROWSER)
          set_DWin_edit(1,(void*)mb,index,col);
        return 0;
      }
      break;
    }
    break;
  case WM_CHAR:
    switch(wPar) {
    case 13:
      return 0;
    }
    break;
  case WM_KILLFOCUS:
    if(!DWin_edit_is_being_temporarily_defocussed)
      DWin_edit_finish(true);
    break;
  }
  return CallWindowProc(Old_edit_WndProc,Win,Mess,wPar,lPar);
}

#endif//#ifdef DEBUG_BUILD
