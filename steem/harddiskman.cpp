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

FILE: harddiskman.cpp
DESCRIPTION: The code for the hard drive manager dialog.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <mymisc.h>
#include <acc.h>
#include <options.h>
#include <debug.h>
#include <diskman.h>
#include <harddiskman.h>
#include <hd_acsi.h>
#include <translate.h>
#include <choosefolder.h>
#include <gui.h>
#include <hd_gemdos.h>
#include <emulator.h>
#include <display.h>
#include <computer.h>

#if !defined(SSE_ACSI)
const bool hd_acsi=false;
#endif


THardDiskManager::THardDiskManager() {
  Section="HardDrives";
#if defined(SSE_ACSI)
  hd_acsi=false; // here is the GEMDOS HD manager
#endif
  nDrives=0;
  DisableHardDrives=true; // first start, no HD
  ApplyChanges=0;
  for(int i=0;i<MAX_HARDDRIVES;i++)
  {
    HDrive[i].Path="";
    HDrive[i].Letter=(char)('C'+i);
  }
  update_mount();

#ifdef WIN32
  Left=GuiSM.cx_screen()/2-258;
  Top=GuiSM.cy_screen()/2-90+GuiSM.cy_caption();
  FSLeft=320 - 258;
  FSTop=240 - 90+GuiSM.cy_caption();
#if defined(SSE_GUI_FONT_FIX)
  Font=SSEConfig.GuiFont();
#else
  Font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
#endif
#endif

#ifdef UNIX
  OldDrive=NULL;nOldDrives=0;
#endif//UNIX
}


#if defined(SSE_ACSI)

TAcsiHardDiskManager::TAcsiHardDiskManager() {
  hd_acsi=true;
}

#endif


void THardDiskManager::update_mount() {
  for(int n=2;!hd_acsi&&n<26;n++)
  {
    if(IsMountedDrive(char(n+'A'))) 
    {
      mount_flag[n]=true;
      mount_path[n]=GetMountedDrivePath(char(n+'A'));
    }
    else 
    {
      mount_flag[n]=false;
      mount_path[n]="";
    }
  }
  CheckResetDisplay();
}


bool THardDiskManager::IsMountedDrive(char d) {
  if(d>='C'&&!DisableHardDrives)
  {
    for(int n=0;n<nDrives;n++)
    {
      if(d==HDrive[n].Letter)
        return true;
    }
  }
  return false;
}


EasyStr THardDiskManager::GetMountedDrivePath(char d) {
  if(d>='C') 
  {
    for(int n=0;n<nDrives;n++) 
    {
      if(d==HDrive[n].Letter)
        return HDrive[n].Path;
    }
  }
  return RunDir;
}


bool THardDiskManager::NewDrive(char *Path) {
#if defined(SSE_ACSI)
  if(hd_acsi&&nDrives>=TAcsiHdc::MAX_ACSI_DEVICES
    ||!hd_acsi&&nDrives>=MAX_HARDDRIVES)
    return 0;
#else
  if(nDrives>=MAX_HARDDRIVES) return 0;
#endif
  int Idx=nDrives;
  bool Found=0;
  HDrive[Idx].Path=Path;
  NO_SLASH(HDrive[Idx].Path);
  for(int Let='C';Let<='Z' && Found==0;Let++)
  {
    for(int i=0;i<nDrives;i++)
    {
      if(HDrive[i].Letter==Let)
        break;
      else if(i==nDrives-1)
      {
        HDrive[Idx].Letter=(char)Let;
        Found=true;
      }
    }
  }
  nDrives++;
  return true;
}


void THardDiskManager::Show() {
  if(Handle!=NULL)
  {
#ifdef WIN32
    SetForegroundWindow(Handle);
#endif
    return;
  }
#ifdef WIN32
#if !defined(SSE_GUI_ALT_MENU)
  else if(DiskMan.Handle==NULL)
    return;
#endif
  HWND Win;
#if defined(SSE_GUI_ALT_MENU)
  if(DiskMan.Handle)
#endif
    EnableWindow(DiskMan.Handle,0);
  ManageWindowClasses(SD_REGISTER);
#if defined(SSE_ACSI)
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Hard Disk Manager",
    (hd_acsi?T("ACSI Hard Drives"):T("GEMDOS Hard Drives")),
    WS_CAPTION|WS_SYSMENU,
    Left,Top,516,90+GuiSM.cy_caption(),
    DiskMan.Handle,0,HInstance,NULL);
#else
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Hard Disk Manager",T("Hard Drives"),WS_CAPTION|WS_SYSMENU,
    Left,Top,516,90+GuiSM.cy_caption(),
    DiskMan.Handle,0,HInstance,NULL);
#endif
  if(HandleIsInvalid())
  {
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }
  SetWindowLongPtr(Handle, GWLP_USERDATA, (LONG_PTR)this);
  if(FullScreen)
    MakeParent(StemWin);
#if defined(SSE_ACSI)
  int w=GetCheckBoxSize(Font,
    (hd_acsi?T("&Disable ACSI Hard Drives"):T("&Disable GEMDOS Hard Drives"))).Width;
  Win=CreateWindow("Button",
    (hd_acsi?T("&Disable ACSI Hard Drives"):T("&Disable GEMDOS Hard Drives")),
    WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTOCHECKBOX,10,10,w,23,Handle,
    (HMENU)90,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,(UINT_PTR)(
    (hd_acsi&&!SSEOptions.Acsi||!hd_acsi&&DisableHardDrives)
    ?BST_CHECKED:BST_UNCHECKED),0);
#else
  int w=GetCheckBoxSize(Font,T("&Disable All Hard Drives")).Width;
  Win=CreateWindow("Button",T("&Disable All Hard Drives"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTOCHECKBOX,
    10,10,w,23,Handle,(HMENU)90,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,(UINT)(DisableHardDrives?BST_CHECKED:BST_UNCHECKED),0);
#endif
  SendMessage(Win,WM_SETFONT,(UINT_PTR)Font,0);
  SendMessage(CreateWindow("Button",T("&New Hard Drive"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
    300,10,200,23,Handle,(HMENU)10,HInstance,NULL)
    ,WM_SETFONT,(UINT_PTR)Font,0);
  int Wid=get_text_width(T("When drive A is empty boot from"));
  if(!hd_acsi)
  {
    SendMessage(CreateWindow("Static",T("When drive A is empty boot from"),WS_CHILD|WS_VISIBLE,
      10,44,Wid,20,Handle,(HMENU)91,HInstance,NULL)
      ,WM_SETFONT,(UINT_PTR)Font,0);
    Win=CreateWindow("Combobox","",WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL|
      CBS_DROPDOWNLIST|CBS_HASSTRINGS,
      15+Wid,40,40,300,Handle,(HMENU)92,HInstance,NULL);
    SendMessage(Win,WM_SETFONT,(UINT_PTR)Font,0);
    char DriveName[8];
    DriveName[1]=':';DriveName[2]=0;
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Off"));
    for(int i=0;i<24;i++) 
    {
      DriveName[0]=(char)('C'+i);
      SendMessage(Win,CB_ADDSTRING,0,(LPARAM)DriveName);
    }
#ifndef DISABLE_STEMDOS
    SendMessage(Win,CB_SETCURSEL,stemdos_boot_drive-1,0);
#else
    SendMessage(Win,CB_SETCURSEL,0,0);
    EnableWindow(Win,0);
#endif
  }
  Win=CreateWindow("Button",T("OK"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_DEFPUSHBUTTON,
    290,40,100,23,Handle,(HMENU)IDOK,HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT_PTR)Font,0);
  Win=CreateWindow("Button",T("Cancel"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
    400,40,100,23,Handle,(HMENU)IDCANCEL,HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT_PTR)Font,0);
  for(int i=0;i<nDrives;i++)
    CreateDriveControls(i);
  SetWindowHeight();
  nOldDrives=nDrives;
  if(nDrives)
    OldDrive=new THard_Disk_Info[nOldDrives];
  else
    OldDrive=NULL;
  for(int i=0;i<nDrives;i++) 
    OldDrive[i]=HDrive[i];
  OldDisableHardDrives=DisableHardDrives;
  ShowWindow(Handle,SW_SHOW);
  SetFocus(GetDlgItem(Handle,int(nDrives ? 100:IDOK)));
#endif

#ifdef UNIX
  if (StandardShow(590,10+60+(nDrives*30)+5,T("Hard Drives"),
      ICO16_HARDDRIVE,0,(LPWINDOWPROC)WinProc,true)) return;

  XSizeHints *pHints=XAllocSizeHints();
  if (pHints){
    pHints->flags=PMinSize | PMaxSize;
		pHints->min_width=590;
		pHints->min_height=10+60+5;
    pHints->max_width=590;
    pHints->max_height=10+60+(MAX_HARDDRIVES*30)+5;
    XSetWMSizeHints(XD,Handle,pHints,XA_WM_NORMAL_HINTS);
    XSetWMSizeHints(XD,Handle,pHints,XA_WM_ZOOM_HINTS);
    XFree(pHints);
  }

	int y=10;
	for (int n=0;n<nDrives;n++){
		CreateDriveControls(n); 				  				
  	y+=30;
	}
	all_off_but.create(XD,Handle,10,y,0,25,button_notify_proc,this,BT_CHECKBOX,StripAndT("&Disable All Hard Drives"),400,BkCol);
	SetWindowGravity(XD,all_off_but.handle,SouthEastGravity);
	
	new_but.create(XD,Handle,345,y,235,25,button_notify_proc,this,BT_TEXT,StripAndT("&New Hard Drive"),401,BkCol);
	SetWindowGravity(XD,new_but.handle,SouthEastGravity);
	
	y+=30;
	
	boot_label.create(XD,Handle,10,y,0,25,NULL,this,BT_LABEL,T("When drive A is empty boot from"),402,BkCol);
	SetWindowGravity(XD,boot_label.handle,SouthEastGravity);
	
  boot_dd.make_empty();
  boot_dd.additem("Off");
  for (int i=0;i<24;i++) boot_dd.additem(Str(char('C'+i))+":");
#ifndef DISABLE_STEMDOS
  boot_dd.changesel(stemdos_boot_drive-1);
#endif
	boot_dd.create(XD,Handle,10+boot_label.w+5,y,50,200,NULL);
	SetWindowGravity(XD,boot_dd.handle,SouthEastGravity);

	ok_but.create(XD,Handle,410,y,80,25,
												button_notify_proc,this,BT_TEXT,T("Ok"),403,BkCol);
	SetWindowGravity(XD,ok_but.handle,SouthEastGravity);

	cancel_but.create(XD,Handle,500,y,80,25,
												button_notify_proc,this,BT_TEXT,T("Cancel"),404,BkCol);
	SetWindowGravity(XD,cancel_but.handle,SouthEastGravity);

  XMapWindow(XD,Handle);

  nOldDrives=nDrives;
  if (nDrives){
    OldDrive=new THard_Disk_Info[nOldDrives];
  }else{
    OldDrive=NULL;
  }
  for (int i=0;i<nDrives;i++){
    OldDrive[i]=HDrive[i];
  }

  if (StemWin) DiskMan.HardBut.set_check(true);

#endif
}


void THardDiskManager::CreateDriveControls(int Idx) {
  int y=10+(Idx*30);

#ifdef WIN32
  HWND Win;
  if(GetDlgItem(Handle,300+Idx)!=NULL)
    return;
  Win=CreateWindow("Combobox","",WS_CHILDWINDOW | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
                CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                10,y,40,300,Handle,(HMENU)(300+Idx),HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT_PTR)Font,0);
  char DriveName[8];
  if(hd_acsi)
    DriveName[1]=0;
  else
    DriveName[1]=':';
  DriveName[2]=0;
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Off"));
  for(int i=0;i<(hd_acsi?TAcsiHdc::MAX_ACSI_DEVICES:24);i++) 
  {
    if(hd_acsi)
      DriveName[0]=(char)('0'+i);
    else
      DriveName[0]=(char)('C'+i);
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)DriveName);
  }
  SendMessage(Win,CB_SETCURSEL,(HDrive[Idx].Letter-'C')+1,0);
  Win=CreateWindowEx(512,"Edit",HDrive[Idx].Path,WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL,
    55,y,205,23,Handle,(HMENU)(100+Idx),HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT_PTR)Font,0);
  SendMessage(Win,EM_LIMITTEXT,MAX_PATH,0);
  int Len=(int)SendMessage(Win,WM_GETTEXTLENGTH,0,0);
  SendMessage(Win,EM_SETSEL,Len,Len);
  SendMessage(Win,EM_SCROLLCARET,0,0);
  Win=CreateWindow("Button",T("Browse"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
    265,y,75,23,Handle,(HMENU)(150+Idx),HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT_PTR)Font,0);
  Win=CreateWindow("Button",T("Open"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
    345,y,75,23,Handle,(HMENU)(250+Idx),HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT_PTR)Font,0);
  Win=CreateWindow("Button",T("Remove"),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
    425,y,75,23,Handle,(HMENU)(200+Idx),HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT_PTR)Font,0);
  SetWindowHeight();
#endif//WIN32

#ifdef UNIX
  drive_dd[Idx].make_empty();
  drive_dd[Idx].additem("Off");
  for (int i=0;i<24;i++) drive_dd[Idx].additem(Str(char('C'+i))+":");
  drive_dd[Idx].changesel((HDrive[Idx].Letter-'C')+1);
  drive_dd[Idx].create(XD,Handle,10,y,50,200,NULL);

	drive_ed[Idx].create(XD,Handle,70,y,240,25,NULL);
	drive_ed[Idx].set_text(HDrive[Idx].Path+"/");

	drive_browse_but[Idx].create(XD,Handle,320,y,80,25,
											button_notify_proc,this,BT_TEXT,T("Browse"),Idx*10,BkCol);

	drive_open_but[Idx].create(XD,Handle,410,y,80,25,
											button_notify_proc,this,BT_TEXT,T("Open"),Idx*10+2,BkCol);

	drive_remove_but[Idx].create(XD,Handle,500,y,80,25,
											button_notify_proc,this,BT_TEXT,T("Remove"),Idx*10+1,BkCol);
  
#endif
}


void THardDiskManager::SetWindowHeight() {

#ifdef WIN32
  RECT rc;
  SetWindowPos(Handle,0,0,0,516,80+GuiSM.cy_caption()+(nDrives*30),SWP_NOZORDER | SWP_NOMOVE | SWP_NOCOPYBITS);
  SetWindowPos(GetDlgItem(Handle,90),0,10,12+(nDrives*30),0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
  SetWindowPos(GetDlgItem(Handle,10),0,300,12+(nDrives*30),0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
  GetClientRect(GetDlgItem(Handle,91),&rc);
  SetWindowPos(GetDlgItem(Handle,91),0,10,46+(nDrives*30),0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
  SetWindowPos(GetDlgItem(Handle,92),0,15+rc.right,42+(nDrives*30),0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
  SetWindowPos(GetDlgItem(Handle,IDOK),0,290,42+(nDrives*30),0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
  SetWindowPos(GetDlgItem(Handle,IDCANCEL),0,400,42+(nDrives*30),0,0,SWP_NOZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
#endif

#ifdef UNIX
	XResizeWindow(XD,Handle,590,10+(nDrives*30)+60+5);
//	unix_non_resizable_window(XD,Handle);
#endif
}


void THardDiskManager::GetDriveInfo() {
  for(int i=0;i<nDrives;i++)
  {
#ifdef WIN32
    HDrive[i].Path.SetLength(MAX_PATH+1);
    SendMessage(GetDlgItem(Handle,100+i),WM_GETTEXT,MAX_PATH,(LPARAM)HDrive[i].Path.Text);
    NO_SLASH(HDrive[i].Path);
    if(HDrive[i].Path.Length()==1) 
      HDrive[i].Path+=":";
    HDrive[i].Letter=char(SendMessage(GetDlgItem(Handle,300+i),CB_GETCURSEL,0,0)+'C'-1);
#endif

#ifdef UNIX
		HDrive[i].Letter='B'+drive_dd[i].sel;
		HDrive[i].Path=drive_ed[i].text;
		NO_SLASH(HDrive[i].Path.Text);
#endif
  }
}


void THardDiskManager::Hide() {
  if(Handle==NULL)
    return;
#ifdef UNIX
  if(XD==NULL)
    return;
#endif

  if(ApplyChanges) 
  {
//    ApplyChanges=0;
    GetDriveInfo();
    for(int i=0;i<nDrives;i++) 
    {
#ifdef WIN32
      if(HDrive[i].Path.IsEmpty())
      {
        Alert(T("One of the mounted paths is empty!"),T("Empty Path"),
          MB_ICONEXCLAMATION);
        return;
      }
      else 
      {
        UINT Type=GetDriveType(EasyStr(HDrive[i].Path[0])+":\\");
        if(Type==1) 
        {
          Alert(EasyStr(HDrive[i].Path[0])+" "+T("is not a valid drive letter."),T("Invalid Drive"),MB_ICONEXCLAMATION);
          return;
        }
        else if(Type!=DRIVE_REMOVABLE && Type!=DRIVE_CDROM &&!hd_acsi)
        {
          DWORD Attrib=GetFileAttributes(HDrive[i].Path);
          if(Attrib==0xffffffff) 
          {
            if(Alert(HDrive[i].Path+" "+T("does not exist. Do you want to create it?"),T("New Folder?"),
              MB_ICONQUESTION|MB_YESNO)==IDYES) {
              if(CreateDirectory(HDrive[i].Path,NULL)==0) 
              {
                Alert(T("Could not create the folder")+" "+HDrive[i].Path,T("Invalid Path"),MB_ICONEXCLAMATION);
                return;
              }
            }
            else 
              return;
          }
          else if((Attrib & FILE_ATTRIBUTE_DIRECTORY)==0) 
          {
            Alert(HDrive[i].Path+" "+T("is not a folder."),T("Invalid Path"),MB_ICONEXCLAMATION);
            return;
          }
        }
#if defined(SSE_ACSI)
        else if(hd_acsi && AcsiHdc[i].Init(i,HDrive[i].Path))
          SSEConfig.AcsiImg=true;
#endif
      }
#endif//WIN32

#ifdef UNIX
      if(HDrive[i].Path.Text[0])
      {
        if(!Exists(HDrive[i].Path))
        {
          if(Alert(HDrive[i].Path+" "+T("does not exist. Do you want to create it?"),
            T("New Folder?"),MB_ICONQUESTION|MB_YESNO)==IDYES)
          {
            if(CreateDirectory(HDrive[i].Path,NULL)==0)
            {
              Alert(T("Could not create the folder")+" "+HDrive[i].Path,
                T("Invalid Path"),MB_ICONEXCLAMATION);
              return;
            }
          }
          else
            return;
        }
      }
#endif//UNIX
    }//nxt
    if(!hd_acsi)
    {
      //Remove old stemdos drives from ST memory
      long DrvMask=LPEEK(SV_drvbits);
      for(int i=0;i<nOldDrives;i++)
      {
#ifdef WIN32
        if(OldDrive[i].Letter>='C') // Don't remove it if off!
          DrvMask&=~(1<<(2+OldDrive[i].Letter-'C'));
#endif
#ifdef UNIX
        DrvMask &= ~(1 << (OldDrive[i].Letter-'A'));
#endif
      }
      LPEEK(SV_drvbits)=DrvMask;
      update_mount();
#ifndef DISABLE_STEMDOS
#ifdef WIN32
      stemdos_boot_drive=(BYTE)SendDlgItemMessage(Handle,92,CB_GETCURSEL,0,0)+1;
#endif
#ifdef UNIX
      stemdos_boot_drive=boot_dd.sel+1;
#endif
      stemdos_update_drvbits();
      stemdos_check_paths();
#endif
    }
  }
  else
  {// cancel
    nDrives=nOldDrives;
    for(int i=0;i<nOldDrives;i++) 
      HDrive[i]=OldDrive[i];
    DisableHardDrives=OldDisableHardDrives;
    update_mount();
  }
  ApplyChanges=0;
  if(OldDrive) 
    delete[] OldDrive;

#ifdef WIN32
#if !defined(SSE_GUI_ALT_MENU)
  ASSERT(DiskMan.Handle);
#endif
  if(DiskMan.Handle)
  {
    EnableWindow(DiskMan.Handle,true);
    if(FullScreen)
      SetFocus(DiskMan.Handle);
    else
      SetForegroundWindow(DiskMan.Handle);
  }
  ShowWindow(Handle,SW_HIDE);
  DestroyWindow(Handle);Handle=NULL;
  if(DiskMan.Handle)
    PostMessage(DiskMan.Handle,WM_USER,0,0);
  ManageWindowClasses(SD_UNREGISTER);
#endif

#ifdef UNIX
  StandardHide();
  if(StemWin)
    DiskMan.HardBut.set_check(0);
#endif
}




#ifdef WIN32

void THardDiskManager::ManageWindowClasses(bool Unreg) {
  char *ClassName="Steem Hard Disk Manager";
  if(Unreg)
    UnregisterClass(ClassName,Inst);
  else
    RegisterMainClass(WndProc,ClassName,RC_ICO_HARDDRIVE16);
}


#define GET_THIS This=(THardDiskManager*)GetWindowLongPtr(Win,GWLP_USERDATA);

LRESULT CALLBACK THardDiskManager::WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar) {
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if(StemDialog_RetDefVal) 
    return Ret;
  THardDiskManager *This;
  //TRACE("Mess %d w %x l %x\n",Mess,wPar,lPar);
  switch(Mess) {
  case WM_COMMAND: {
    int ID=LOWORD(wPar);
    GET_THIS;
    if(ID==10) 
    {
      // New Hard Drive
      if(HIWORD(wPar)==BN_CLICKED) 
      {
#if defined(SSE_ACSI)
        if(This->hd_acsi && This->nDrives<TAcsiHdc::MAX_ACSI_DEVICES
          ||!This->hd_acsi && This->nDrives<MAX_HARDDRIVES) 
#else
        if(This->nDrives<MAX_HARDDRIVES) 
#endif
        {
          This->GetDriveInfo();
          This->NewDrive(WriteDir);
          This->CreateDriveControls(This->nDrives-1);
          SetFocus(GetDlgItem(Win,100+This->nDrives-1));
          SendMessage(GetDlgItem(Win,10),BM_SETSTYLE,0,true);
          SendMessage(GetDlgItem(Win,IDOK),BM_SETSTYLE,1,true);
        }
      }
    }
    else if(ID==90) 
    {
      This->DisableHardDrives=SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED;
#if defined(SSE_ACSI)
      if(This->hd_acsi)
        SSEOptions.Acsi=!SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED;
#ifdef SSE_DEBUG
      if(This->hd_acsi)
        TRACE_INIT("Option ACSI %d\n",SSEOptions.Acsi);
      else
        TRACE_INIT("Option GEMDOS HD %d\n",!This->DisableHardDrives);
#endif  
#endif
      REFRESH_STATUS_BAR;
    }
    else if(ID==IDOK||ID==IDCANCEL) 
    {
      if(HIWORD(wPar)==BN_CLICKED) 
      {
        if(ID==IDOK)
          This->ApplyChanges=true;
        PostMessage(Win,WM_CLOSE,0,0);
      }
    }
    else if(ID>=150&&ID<300) 
    {
      if(HIWORD(wPar)==BN_CLICKED) 
      {
        if(ID<200) 
        {
          ID-=150;
          // Browse
          SendMessage(HWND(lPar),BM_SETCHECK,1,true);
          EnableAllWindows(0,Win);
          This->GetDriveInfo();
          EasyStr NewPath;
          if(This->hd_acsi)
            NewPath=FileSelect(NULL,T("Select ACSI Image"),
              This->HDrive[ID].Path,FSTypes(4,NULL),1,true);
          else
            NewPath=ChooseFolder(HWND(FullScreen?StemWin:Win),
              T("Pick a Folder"),This->HDrive[ID].Path);
          if(NewPath.NotEmpty())
            SendMessage(GetDlgItem(This->Handle,100+ID),WM_SETTEXT,0,
              (LPARAM)NewPath.Text);
          SetForegroundWindow(Win);
          EnableAllWindows(true,Win);
          SetFocus(HWND(lPar));
          SendMessage(HWND(lPar),BM_SETCHECK,0,true);
        }
        else if(ID<250) 
        {
          // Remove
          ID-=200;
          char Text[MAX_PATH+1];
          This->nDrives--;
          for(int i=ID;i<This->nDrives;i++) 
          {
            SendMessage(GetDlgItem(This->Handle,100+i+1),WM_GETTEXT,MAX_PATH,
              (LPARAM)Text);
            SendMessage(GetDlgItem(This->Handle,100+i),WM_SETTEXT,0,(LPARAM)Text);
            SendMessage(GetDlgItem(This->Handle,300+i),CB_SETCURSEL,
            SendMessage(GetDlgItem(This->Handle,300+i+1),CB_GETCURSEL,0,0),0);
          }
          DestroyWindow(GetDlgItem(This->Handle,100+This->nDrives));
          DestroyWindow(GetDlgItem(This->Handle,150+This->nDrives));
          DestroyWindow(GetDlgItem(This->Handle,200+This->nDrives));
          DestroyWindow(GetDlgItem(This->Handle,250+This->nDrives));
          DestroyWindow(GetDlgItem(This->Handle,300+This->nDrives));
          This->GetDriveInfo();
          This->SetWindowHeight();
          if(This->nDrives)
            SetFocus(GetDlgItem(This->Handle,200+MIN(ID,This->nDrives-1)));
          else
            SetFocus(GetDlgItem(Win,IDOK));
          SendMessage(GetFocus(),BM_SETSTYLE,1,true);
        }
        else 
        {
          ID-=250;
          This->GetDriveInfo();
          ShellExecute(NULL,NULL,This->HDrive[ID].Path,"","",SW_SHOWNORMAL);
        }
      }
    }
    break;
  }
  case (WM_USER+1011):  
  {
    GET_THIS;
    HWND NewParent=(HWND)lPar;
    if(NewParent) 
    {
      This->CheckFSPosition(NewParent);
      SetWindowPos(Win,NULL,This->FSLeft,This->FSTop,0,0,SWP_NOZORDER|SWP_NOSIZE);
    }
    else
      SetWindowPos(Win,NULL,This->Left,This->Top,0,0,SWP_NOZORDER|SWP_NOSIZE);
    This->ChangeParent(NewParent);
    break;
  }
  case WM_CLOSE:
    GET_THIS;
    This->Hide();
    return 0;
  case DM_GETDEFID:
    return MAKELONG(IDOK,DC_HASDEFID);
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}

#undef GET_THIS
#endif//WIN32

#ifdef UNIX //TODO ACSI...

void THardDiskManager::RemoveLine(int dn)
{
 	for (int n=dn;n<nDrives-1;n++){
   	// copy info from line n+1 to line n
 		drive_dd[n].changesel(drive_dd[n+1].sel);
 		drive_dd[n].draw();
 		drive_ed[n].set_text(drive_ed[n+1].text);
 	}
 	nDrives--;
 	drive_dd[nDrives].destroy(&(drive_dd[nDrives]));
 	drive_ed[nDrives].destroy(&(drive_ed[nDrives]));
 	drive_browse_but[nDrives].destroy(&(drive_browse_but[nDrives]));
 	drive_open_but[nDrives].destroy(&(drive_open_but[nDrives]));
 	drive_remove_but[nDrives].destroy(&(drive_remove_but[nDrives]));
 	SetWindowHeight();
}  					

int THardDiskManager::button_notify_proc(hxc_button *But,int Mess,int *Inf)
{
	THardDiskManager *This=(THardDiskManager*)But->owner;
	if (Mess==BN_CLICKED){
		switch (But->id){
			case 400:
        This->DisableHardDrives=But->checked;
				break;
			case 401:
        if (This->nDrives<MAX_HARDDRIVES){
          This->GetDriveInfo();
          This->NewDrive(WriteDir);
          This->CreateDriveControls(This->nDrives-1);
          This->SetWindowHeight();
        }	
				break;
			case 403:
				This->ApplyChanges=true;
			case 404:
				This->Hide();
				break;			
			default:{
				if (But->id<MAX_HARDDRIVES*10){
  				int r=((But->id)%10);
  				int dn=((But->id)/10);
  				if (r==0){ //browse
  					char*path=This->drive_ed[dn].text.Text;
						fileselect.set_corner_icon(&Ico16,ICO16_FOLDER);
					  EasyStr new_path=fileselect.choose(XD,path,"",T("Pick a Folder"),
		   				FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");
	   				if(new_path[0]){
	   					NO_SLASH(new_path);
	   					This->drive_ed[dn].set_text(new_path+"/");
	   				}
  				}else if (r==1){  //remove
  					This->RemoveLine(dn);
  				}else if (r==2){  // open
            shell_execute(Comlines[COMLINE_FM],Str("[PATH]\n")+This->drive_ed[dn].text);
          }
  			}
			}
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
int THardDiskManager::WinProc(THardDiskManager *This,Window Win,XEvent*Ev)
{
  switch (Ev->type){
    case ClientMessage:
      if (Ev->xclient.message_type==hxc::XA_WM_PROTOCOLS){
        if (Atom(Ev->xclient.data.l[0])==hxc::XA_WM_DELETE_WINDOW){
          This->Hide();
        }
      }
      break;
  }
  return PEEKED_MESSAGE;
}

#endif//UNIX
