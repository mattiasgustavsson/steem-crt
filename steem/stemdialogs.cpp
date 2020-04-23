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

FILE: stemdialogs.cpp
DESCRIPTION: The base class for Steem's dialogs that are used to configure
the emulator and perform additional functions.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <stemdialogs.h>
#include <display.h>
#include <gui.h>
#include <run.h>
#ifdef UNIX
#include <palette.h>
#endif


WIN_ONLY(bool StemDialog_RetDefVal; )

TStemDialog *DialogList[MAX_DIALOGS]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL};
int nStemDialogs=0;

#ifdef WIN32
// For some reason in 24-bit and 32-bit screen modes on XP ILC_COLOR24 and
// ILC_COLOR32 icons don't highlight properly, have to be 16-bit.
const UINT BPPToILC[5]={0,ILC_COLOR4,ILC_COLOR16,ILC_COLOR16,ILC_COLOR16};
#endif




TStemDialog::TStemDialog() {
  Handle=NULL;
  if(nStemDialogs<MAX_DIALOGS) 
    DialogList[nStemDialogs++]=this;
#ifdef WIN32
  Focus=NULL;
#if defined(SSE_GUI_FONT_FIX)
  Font=SSEConfig.GuiFont();
#else
  Font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
#endif
  Left=100;Top=100;
  FSLeft=50;FSTop=50;
#endif//WIN32
#ifdef UNIX
  IconPixmap=0;
  IconMaskPixmap=0;
#endif
}


#ifdef WIN32

bool TStemDialog::HasHandledMessage(MSG *mess) {
  if(Handle)
    return (IsDialogMessage(Handle,mess)!=0);
  else
    return 0;
}


void TStemDialog::RegisterMainClass(WNDPROC WndProc,char *ClassName,int nIcon) {
  WNDCLASS wc;
  wc.style=CS_DBLCLKS;
  wc.lpfnWndProc=WndProc;
  wc.cbClsExtra=0;
  wc.cbWndExtra=0;
  wc.hInstance=(HINSTANCE)GetModuleHandle(NULL);
  nMainClassIcon=nIcon;
  wc.hIcon=hGUIIcon[nIcon];
  wc.hCursor=LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
  wc.lpszMenuName=NULL;
  wc.lpszClassName=ClassName;
  RegisterClass(&wc);
}


void TStemDialog::UpdateMainWindowIcon() {
  if(Handle) 
    SetClassLongPtr(Handle,GCLP_HICON,(LONG_PTR)(hGUIIcon[nMainClassIcon]));
}


bool TStemDialog::HandleIsInvalid() {
  if(Handle) 
    if(IsWindow(Handle)==0) 
      Handle=NULL;
  return Handle==NULL;
}


void TStemDialog::MakeParent(HWND NewParent) {
  if(Handle)
  {
    UpdateMainWindowIcon();
    SendMessage(Handle,WM_USER+1011,0,(LPARAM)NewParent);
  }
}


void TStemDialog::ChangeParent(HWND NewParent) {
  if(NewParent!=NULL)
  {
    RECT rc;
    GetWindowRect(Handle,&rc);
    if(rc.top<MENUHEIGHT)
      SetWindowPos(Handle,NULL,rc.left,MENUHEIGHT,0,0,SWP_NOZORDER|SWP_NOSIZE
        |SWP_NOACTIVATE);
    SetParent(Handle,NewParent);
    SetWindowPos(Handle,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
  }
  else
  {
    SetParent(Handle,NULL);
    SetWindowPos(Handle,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
  }
}


HTREEITEM TStemDialog::AddPageLabel(char *t,int i) {
  TV_INSERTSTRUCT tvis;
  tvis.hParent=TVI_ROOT;
  tvis.hInsertAfter=TVI_LAST;
  tvis.item.mask=TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
  tvis.item.pszText=t;
  tvis.item.lParam=i;
  tvis.item.iImage=i;
  tvis.item.iSelectedImage=i;
  return (HTREEITEM)SendMessage(PageTree,TVM_INSERTITEM,0,(LPARAM)&tvis);
}


void TStemDialog::GetPageControlList(DynamicArray<HWND> &ChildList) {
  HWND FirstChild=GetWindow(Handle,GW_CHILD);
  HWND Child=FirstChild;
  while(Child)
  {
    if(GetDlgCtrlID(Child)<60000) 
      ChildList.Add(Child);
    Child=GetWindow(Child,GW_HWNDNEXT);
    if(Child==FirstChild) 
      break;
  }
}


void TStemDialog::DestroyCurrentPage() {
  DynamicArray<HWND> ChildList;
  GetPageControlList(ChildList);
  for(int n=0;n<ChildList.NumItems;n++) 
    DestroyWindow(ChildList[n]);
}


void TStemDialog::ShowPageControls() {
  DynamicArray<HWND> ChildList;
  GetPageControlList(ChildList);
  for(int n=0;n<ChildList.NumItems;n++) 
    ShowWindow(ChildList[n],SW_SHOW);
}


void TStemDialog::SetPageControlsFont() {
  DynamicArray<HWND> ChildList;
  GetPageControlList(ChildList);
  for(int n=0;n<ChildList.NumItems;n++) 
    SendMessage(ChildList[n],WM_SETFONT,WPARAM(Font),0);
}


#define GET_THIS This=(TStemDialog*)GetWindowLongPtr(Win,GWLP_USERDATA);

LRESULT TStemDialog::DefStemDialogProc(HWND Win,UINT Mess,
                                      WPARAM wPar,LPARAM lPar) {
  StemDialog_RetDefVal=0;
  TStemDialog *This;
  switch(Mess) {
  case WM_SYSCOMMAND:
    switch(wPar) {
    case SC_MONITORPOWER:
      if(runstate==RUNSTATE_RUNNING) 
        return 0;
      break;
    case SC_SCREENSAVE:
      if(runstate==RUNSTATE_RUNNING||FullScreen) 
        return 0;
      break;
    }
    break;
  case WM_MOVING:case WM_SIZING:
    if(FullScreen)
    {
      RECT *rc=(RECT*)lPar;
      if(rc->top<MENUHEIGHT)
      {
        if(Mess==WM_MOVING) rc->bottom+=MENUHEIGHT-rc->top;
        rc->top=MENUHEIGHT;
        StemDialog_RetDefVal=true;
        return true;
      }
#if !defined(SSE_VID_2SCREENS) // no clipping at all
      RECT LimRC={0,MENUHEIGHT+GuiSM.cy_frame(),GuiSM.cx_screen(),GuiSM.cy_screen()};
      ClipCursor(&LimRC);
#endif
    }
    break;
  case WM_MOVE:
  {
    GET_THIS;
    RECT rc;
    GetWindowRect(Win,&rc);
    if(FullScreen)
    {
      if(IsIconic(StemWin)==0&&IsZoomed(StemWin)==0)
      {
#if defined(SSE_VID_2SCREENS) // it's relative to main window, not absolute
        POINT myPoint={rc.left,rc.top};
        ScreenToClient(StemWin,&myPoint);
        This->FSLeft=myPoint.x;
        This->FSTop=myPoint.y;
#else
        This->FSLeft=rc.left;This->FSTop=rc.top;
        //TRACE("WM_MOVE FSLeft %d FSTop %d\n",This->FSLeft,This->FSTop);
#endif
      }
    }
    else
    {
      if(IsIconic(Win)==0&&IsZoomed(Win)==0)
      {
        This->Left=rc.left;This->Top=rc.top;
      }
    }
    break;
  }
  case WM_CAPTURECHANGED:   //Finished
    if(FullScreen) 
      ClipCursor(NULL);
    break;
  case WM_ACTIVATE:
    if(wPar==WA_INACTIVE)
    {
      GET_THIS;
      This->Focus=GetFocus();
    }
    else
    {
      if(IsWindowEnabled(Win)==0)
        PostMessage(StemWin,WM_USER,12345,(LPARAM)Win);
    }
    break;
  case WM_SETFOCUS:
    GET_THIS;
    SetFocus(This->Focus);
    break;
  }
  return 0;
}

#undef GET_THIS

#endif


void TStemDialog::LoadPosition(TConfigStoreFile *pCSF) {
#if defined(SSE_VID_2SCREENS)
  Disp.CheckCurrentMonitorConfig(); // Update monitor rectangle
  int W=Disp.rcMonitor.right-Disp.rcMonitor.left;
  int H=Disp.rcMonitor.bottom-Disp.rcMonitor.top;
  Left=MAX(MIN((int)pCSF->GetInt(Section,"Left",Left),W),Disp.rcMonitor.left);
  Top=MAX(MIN((int)pCSF->GetInt(Section,"Top",Top),H),Disp.rcMonitor.top);
  FSLeft=pCSF->GetInt(Section,"FSLeft",FSLeft);
  FSLeft=MAX(MIN(FSLeft,W),Disp.rcMonitor.left);
  FSTop=pCSF->GetInt(Section,"FSTop",FSTop);
  FSTop=MAX(MIN(FSTop,H),Disp.rcMonitor.top);
#elif 1
  int W=GetScreenWidth(),H=GetScreenHeight();
  Left=MAX(MIN((int)pCSF->GetInt(Section,"Left",Left),W),0);
  Top=MAX(MIN((int)pCSF->GetInt(Section,"Top",Top),H),0);
  FSLeft=pCSF->GetInt(Section,"FSLeft",FSLeft);
  FSLeft=MAX(MIN(FSLeft,W),0);
  FSTop=pCSF->GetInt(Section,"FSTop",FSTop);
  FSTop=MAX(MIN(FSTop,H),0);
#else
  int W=GetScreenWidth()-100,H=GetScreenHeight()-70;
  Left=MAX(MIN((int)pCSF->GetInt(Section,"Left",Left),W),-100);
  Top=MAX(MIN((int)pCSF->GetInt(Section,"Top",Top),H),-70);
  FSLeft=pCSF->GetInt(Section,"FSLeft",FSLeft);
  FSLeft=MAX(MIN(FSLeft,W),-100);
  FSTop=pCSF->GetInt(Section,"FSTop",FSTop);
  FSTop=MAX(MIN(FSTop,H),-70);
#endif
}


void TStemDialog::SavePosition(bool FinalSave,TConfigStoreFile *pCSF) {
  pCSF->SetInt(Section,"Left",Left);
  pCSF->SetInt(Section,"Top",Top);
  pCSF->SetInt(Section,"FSLeft",FSLeft);
  pCSF->SetInt(Section,"FSTop",FSTop);
  if(FinalSave==0) 
    SaveVisible(pCSF);
}


void TStemDialog::SaveVisible(TConfigStoreFile *pCSF) {
  if(Section.NotEmpty())
    pCSF->SetInt(Section,"Visible",IsVisible());
}


#ifdef WIN32

void TStemDialog::CheckFSPosition(HWND Par) {
  RECT rc;
  GetClientRect(Par,&rc);
  FSLeft=MAX(MIN(FSLeft,(int)rc.right-100),-100);
  FSTop=MAX(MIN(FSTop,(int)rc.bottom-70),-70);
}


void TStemDialog::UpdateDirectoryTreeIcons(DirectoryTree *pTree) {
  for(int n=0;n<pTree->FileMasksESL.NumStrings;n++)
  {
    pTree->FileMasksESL[n].Data[0]
      =(LONG_PTR)(hGUIIcon[pTree->FileMasksESL[n].Data[1]]);
  }
  pTree->ReloadIcons(BPPToILC[BytesPerPixel]);
}

#endif


#ifdef UNIX

void TStemDialog::StandardHide()
{
  if (Handle){
    hxc::destroy_children_of(Handle);

    RemoveProp(XD,Handle,cWinProc);
    RemoveProp(XD,Handle,cWinThis);
    XDestroyWindow(XD,Handle);

    if(IconPixmap)XFreePixmap(XD,IconPixmap);
    if(IconMaskPixmap)XFreePixmap(XD,IconMaskPixmap);

    Handle=0;
  }
}


bool TStemDialog::StandardShow(int w,int h,char* name,
      int icon_index,long input_mask,LPWINDOWPROC WinProc,bool resizable)
{
  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.background_pixel=BkCol;
	swa.colormap=colormap;
  Handle=XCreateWindow(XD,XDefaultRootWindow(XD),
                           (GetScreenWidth()-w)/2,(GetScreenHeight()-h)/2,
                           w,h,0,CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore | CWBackPixel |
                           int(colormap ? CWColormap:0),&swa);
  if (Handle==0) return true; //fail

  Atom Prots[1]={hxc::XA_WM_DELETE_WINDOW};
  XSetWMProtocols(XD,Handle,Prots,1);

  if(!resizable)unix_non_resizable_window(XD,Handle);

  IconPixmap=Ico16.CreateIconPixmap(icon_index,DispGC);
  IconMaskPixmap=Ico16.CreateMaskBitmap(icon_index);
  SetWindowHints(XD,Handle,True,NormalState,IconPixmap,IconMaskPixmap,SteemWindowGroup,0);

  XStoreName(XD,Handle,name);

  XSelectInput(XD,Handle,input_mask);

  SetProp(XD,Handle,cWinProc,(DWORD)WinProc);
  SetProp(XD,Handle,cWinThis,(DWORD)this);

  return false; //no error
}

#endif//UNIX
