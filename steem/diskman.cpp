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

DOMAIN: Disk image, GUI
FILE: diskman.cpp
DESCRIPTION: This file contains the code for Steem's disk manager. Some of
these functions are used in the emulation of Steem for vital processes such
as changing disk images and determining what files are disks.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <diskman.h>
#include <acc.h>
#include <loadsave.h>
#include <floppy_disk.h>
#include <computer.h>
#include <gui.h>
#include <osd.h>
#include <translate.h>
#include <harddiskman.h>
#include <archive.h>
#include <di_get_contents.h>
#include <mymisc.h>
#include <choosefolder.h>

#ifdef UNIX
#include <x/hxc_prompt.h>
#endif

#define LOGSECTION LOGSECTION_IMAGE_INFO


#ifdef WIN32

//#define LVS_SMALLVIEW LVS_SMALLICON
#define LVS_SMALLVIEW LVS_LIST

void TDiskManager::RefreshDiskView(EasyStr SelPath,bool EditLabel,
                                    EasyStr SelLinkPath,int iItem) {
  SetDir(DisksFol,0,SelPath,EditLabel,SelLinkPath,iItem);
}

#endif


bool ExtensionIsPastiDisk(char *Ext) {
#if USE_PASTI
  if(Ext==NULL||hPasti==NULL) 
    return false;
  if(*Ext=='.') 
    Ext++;
/*  If the option isn't checked (pasti_active is false), Pasti will run
    STX images only, without us changing pasti_active.
    If the option is checked, Pasti will get all DMA/FDC writes.
*/
  if(!pasti_active)
    return (IsSameStr_I(Ext,DISK_EXT_STX))?true:false;
  char *t=pasti_file_exts;
  while(*t) 
  {
    if(IsSameStr_I(Ext,t)) 
      return true;
    t+=strlen(t)+1;
  }
#endif
  return false;
}


bool ExtensionIsArchive(char *TestedExt) {
  return (MatchesAnyString_I(TestedExt,"STZ","ZIP",
#if defined(SSE_UNRAR_SUPPORT) || defined(RARLIB_SUPPORT)
    "RAR",
#endif
#if defined(SSE_ARCHIVEACCESS_SUPPORT) || defined(SSE_7Z_SUPPORT_UNIX)
    "7Z","BZ2","GZ","TAR","ARJ",
#endif
    NULL));
}


int ExtensionIsDisk(char *TestedExt) {
  int ret=0;
  if(TestedExt==NULL) 
    return ret;
  if(*TestedExt=='.') 
    TestedExt++;
  if(MatchesAnyString_I(TestedExt,"STZ","ZIP",
#if defined(SSE_UNRAR_SUPPORT) || defined(RARLIB_SUPPORT)
    "RAR",
#endif
#if defined(SSE_ARCHIVEACCESS_SUPPORT) || defined(SSE_7Z_SUPPORT_UNIX)
    "7Z","BZ2","GZ","TAR","ARJ",
#endif
    NULL))
    ret=DISK_COMPRESSED;
#if USE_PASTI
  else if(hPasti && !ret && ExtensionIsPastiDisk(TestedExt))
    ret=DISK_PASTI;
#endif
  else if(MatchesAnyString_I(TestedExt,DISK_EXT_ST,DISK_EXT_STT,DISK_EXT_DIM,DISK_EXT_MSA,
#if defined(SSE_DISK_CAPS)
    DISK_EXT_IPF,DISK_EXT_CTR,
#endif    
#if defined(SSE_DISK_SCP)
    DISK_EXT_SCP,
#endif    
#if defined(SSE_DISK_STW)
    DISK_EXT_STW,
#endif  
#if defined(SSE_DISK_HFE)
    DISK_EXT_HFE,
#endif  
    NULL))
    ret=DISK_UNCOMPRESSED;
#if defined(SSE_TOS_PRG_AUTORUN)
  else if(OPTION_PRG_SUPPORT
    && MatchesAnyString_I(TestedExt,DISK_EXT_PRG,DISK_EXT_TOS,NULL))
    ret=DISK_UNCOMPRESSED;
#endif
  else if(!ret && MatchesAnyString_I(TestedExt,CONFIG_FILE_EXT,NULL))
    ret=DISK_IS_CONFIG;
  return ret;
}


EasyStr TDiskManager::CreateDiskName(char *Name,char *DiskInZip) {
  EasyStr Ret=Name;
  if(DiskInZip[0]) 
    Ret=Ret+" ("+DiskInZip+")";
  return Ret;
}


void TDiskManager::PerformInsertAction(int Action,EasyStr Name,EasyStr Path,
                                        EasyStr DiskInZip) {
  bool InsertSucceeded=true;
  if(Path.NotEmpty())
    InsertSucceeded=InsertDisk((Action==1)?1:0,Name,Path,0,false,DiskInZip,
      0,true);
  else
    EjectDisk((Action==1)?1:0);
  if(InsertSucceeded && Action==2) 
  {
#ifdef WIN32
    if(CloseAfterIRR && Handle) 
      PostMessage(Handle,WM_CLOSE,0,0);
    if(IsIconic(StemWin)) 
      OpenIcon(StemWin);
    SetForegroundWindow(StemWin);
#else
    if(SetForegroundWindow(StemWin)==0) return;
    if(CloseAfterIRR && Handle) {
      XEvent SendEv;
      SendEv.type=ClientMessage;
      SendEv.xclient.window=Handle;
      SendEv.xclient.message_type=hxc::XA_WM_PROTOCOLS;
      SendEv.xclient.format=32;
      SendEv.xclient.data.l[0]=hxc::XA_WM_DELETE_WINDOW;
      XSendEvent(XD,Handle,0,0,&SendEv);
    }
#endif
    reset_st(RESET_COLD|RESET_NOSTOP|RESET_CHANGESETTINGS|RESET_BACKUP);
    if(runstate!=RUNSTATE_RUNNING) 
    {
      CLICK_PLAY_BUTTON();
    }
    else 
    {
      if(OPTION_CAPTURE_MOUSE&1)
        SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
      else
        SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
      osd_init_run(true);
    }
  }
}


void TDiskManager::SetNumFloppies(BYTE NewNum) {
  //ASSERT(num_connected_floppies==1||num_connected_floppies==2);
  //ASSERT(NewNum==1||NewNum==2);
  num_connected_floppies=NewNum;
#if defined(SSE_DISK_CAPS)
  // it's not easy mixing IPF and native!
  if(FloppyDrive[1].ImageType.Manager==MNGR_CAPS)
  {
    if(NewNum==1) // deactivated
      Caps.Drive[1].diskattr&=~CAPSDRIVE_DA_IN;
    else // active
      Caps.Drive[1].diskattr|=CAPSDRIVE_DA_IN;
  } // maybe there's more to do for TOS
  Caps.fdc.drivecnt=NewNum;
#endif
#if USE_PASTI
  if(hPasti) 
  {
    struct pastiCONFIGINFO pci;
    pci.flags=PASTI_CFDRIVES;
    pci.ndrives=NewNum;
    pci.drvFlags=0;
    pasti->Config(&pci);
  }
#endif
  if(Handle) 
  {
#ifdef WIN32
    if(GetDlgItem(Handle,99)) 
      InvalidateRect(GetDlgItem(Handle,99),NULL,0);
#endif
#ifdef UNIX
    if(num_connected_floppies==2)
      drive_icon[1].set_icon(&Ico32,ICO32_DRIVE_B);
    else
      drive_icon[1].set_icon(&Ico32,ICO32_DRIVE_B_OFF);
#endif
  }
  CheckResetDisplay();
  REFRESH_STATUS_BAR;
}


TDiskManager::TDiskManager() {
  Section="Disks";
  EjectDisksWhenQuit=ShowHiddenFiles=HideBroken=false;
  HideExtension=true;
  DoubleClickAction=2;

#ifndef SSE_NO_WINSTON_IMPORT
  ImportOnlyIfExist=true;
  ImportConflictAction=0;
  ContentConflictAction=2;
#endif

#ifdef WIN32
#ifdef SSE_ACSI // more space for the ACSI icon in initial disk manager
  Width=403+70+GuiSM.cx_frame()*2+GuiSM.cx_vscroll();
#else 
  Width=403+GuiSM.cx_frame()*2+GuiSM.cx_vscroll();
#endif
  Height=331+GuiSM.cy_caption();
  Left=(GuiSM.cx_screen()-Width)/2;
  Top=(GuiSM.cy_screen()-Height)/2;
  FSWidth=Width;
  FSHeight=Height;
  FSLeft=320-FSWidth/2;
  FSTop=240-FSHeight/2;
  il[0]=il[1]=NULL;
  Dragging=DropTarget=-1;
  DiskDiag=NULL;
#if !defined(SSE_NO_WINSTON_IMPORT)
  LinksDiag=NULL;
  ImportDiag=NULL;
#endif
  PropDiag=NULL;ContentDiag=NULL;DatabaseDiag=NULL;
#if defined(SSE_DISK_CREATE_MSA_DIM)
  SecsPerTrackIdx=9;TracksIdx=80;SidesIdx=2;
#else
  SecsPerTrackIdx=1;TracksIdx=5;SidesIdx=1;
#endif
  SaveScroll=0;
  SmallIcons=0;
  IconSpacing=1;

  DoExtraShortcutCheck=0;

  MSAConvProcess=NULL;
#ifndef SSE_NO_WINSTON_IMPORT
  HKEY Key;
  WinSTonPath="C:\\Program Files\\WinSTon";
  if(RegOpenKey(HKEY_CURRENT_USER,"Software\\WinSTon",&Key)==ERROR_SUCCESS) {
    DWORD Size=500;
    EasyStr Path;
    Path.SetLength(Size);
    if(RegQueryValueEx(Key,"InstalledDirectory",NULL,NULL,(BYTE*)Path.Text,&Size)==ERROR_SUCCESS) {
      WinSTonPath=Path;
    }
    RegCloseKey(Key);
  }
  NO_SLASH(WinSTonPath);
  WinSTonDiskPath=WinSTonPath+"\\Discs";
#endif
  MSAConvPath="";
  DatabaseFind="";
  DatabaseDiag=NULL;
#endif//WIN32

#ifdef UNIX
  DisksFol="///";
  HomeFol="///";
  Width=500;Height=400;
  HistBackLength=0;
  HistForwardLength=0;
#endif
}


void TDiskManager::Show() {
  if(Handle!=NULL) 
  {
#ifdef WIN32
    if(IsIconic(Handle)) 
      ShowWindow(Handle,SW_SHOWNORMAL);
    SetForegroundWindow(Handle);
#endif
    return;
  }

#ifdef WIN32
  bool MaximizeIt=bool(FullScreen?FSMaximized:Maximized);
  ManageWindowClasses(SD_REGISTER);
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT|WS_EX_APPWINDOW,"Steem Disk Manager"
    ,T("Disk Manager"),WS_CAPTION|WS_SYSMENU|WS_SIZEBOX|WS_MAXIMIZEBOX
    |WS_MINIMIZEBOX,Left,Top,Width,Height,ParentWin,NULL,HInstance,NULL);
  if(HandleIsInvalid()) 
  {
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }
  SetWindowLongPtr(Handle,GWLP_USERDATA,(LONG_PTR)this);
  MakeParent(HWND(FullScreen?StemWin:NULL));
  HWND Win;
  int Countdown=10;
  for(;;) {
    DiskView=CreateWindowEx(WS_EX_ACCEPTFILES|512,WC_LISTVIEW,"",WS_CHILD
      |WS_VISIBLE|WS_TABSTOP|LVS_ICON|LVS_SHAREIMAGELISTS|LVS_SINGLESEL
      |LVS_EDITLABELS,10,105,480,200,Handle,(HMENU)102,HInstance,NULL);
    if(DiskView) 
      break;
    Sleep(50);
    if((--Countdown)<=0) 
    {
      DestroyWindow(Handle);Handle=NULL;
      ManageWindowClasses(SD_UNREGISTER);
      return;
    }
  }
  LoadIcons();
  ListView_SetImageList(DiskView,il[0],LVSIL_NORMAL);
  ListView_SetImageList(DiskView,il[1],LVSIL_SMALL);
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_BACK),WS_CHILD|WS_VISIBLE
    |WS_DISABLED|WS_TABSTOP|PBS_RIGHTCLICK,10,80,21,21,Handle,(HMENU)82,
    HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Back"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_FORWARD),WS_CHILD
    |WS_VISIBLE|WS_DISABLED|WS_TABSTOP|PBS_RIGHTCLICK,33,80,21,21,Handle,
    (HMENU)83,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Forward"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_HOME),WS_CHILD|WS_VISIBLE
    |WS_TABSTOP|PBS_RIGHTCLICK,56,80,21,21,Handle,(HMENU)80,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("To home folder"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_SETHOME),WS_CHILD
    |WS_VISIBLE|WS_TABSTOP|PBS_RIGHTCLICK,79,80,21,21,Handle,(HMENU)81,
    HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Make this folder your home folder"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_DRIVEDROPDOWN),WS_CHILD
    |WS_VISIBLE|WS_TABSTOP|PBS_RIGHTCLICK,102,80,21,21,Handle,(HMENU)84,
    HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Disk Manager options"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_DISKMANTOOLS),WS_CHILD
    |WS_VISIBLE|WS_TABSTOP|PBS_RIGHTCLICK,125,80,21,21,Handle,(HMENU)85,
    HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Disk image management tools"));
  Win=CreateWindow("Combobox","",WS_CHILDWINDOW|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL
    |CBS_HASSTRINGS|CBS_DROPDOWNLIST,125+26,80,45,200,Handle,(HMENU)90,
    HInstance,NULL);
  char Root[4]={0,':','\\',0};
  for(int d=0;d<27;d++) 
  {
    Root[0]=char('A'+d);
    if(GetDriveType(Root)>1)
      SendMessage(Win,CB_ADDSTRING,0,(LPARAM)Root);
  }
  CreateWindowEx(512,"Steem Path Display","",WS_CHILD|WS_VISIBLE,
    125+26+50,80,300,20,Handle,(HMENU)97,HInstance,NULL);
  Win=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Disk Manager Drive Icon","A",
    WS_CHILD|WS_VISIBLE,10,10,64,64,Handle,(HMENU)98,HInstance,NULL);
  int Disabled=(AreNewDisksInHistory(0)?0:WS_DISABLED);
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_SMALLDOWNARROW),
    WS_CHILDWINDOW|WS_VISIBLE|WS_TABSTOP|Disabled,
    52,52,12,12,Win,(HMENU)100,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Drive A disk history"));
  Win=CreateWindowEx(WS_EX_ACCEPTFILES|512,WC_LISTVIEW,"",WS_CHILDWINDOW
    |WS_VISIBLE|WS_TABSTOP|LVS_ICON|LVS_SHAREIMAGELISTS|LVS_SINGLESEL
    |LVS_NOSCROLL,75,10,90,64,Handle,(HMENU)100,HInstance,NULL);
  ListView_SetIconSpacing(Win,88,200);
  ListView_SetImageList(Win,il[0],LVSIL_NORMAL);
  SetDriveViewEnable(0,0);
  Win=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Disk Manager Drive Icon","B",
    WS_CHILD|WS_VISIBLE,175,10,64,64,Handle,(HMENU)99,HInstance,NULL);
  Disabled=(AreNewDisksInHistory(1)?0:WS_DISABLED);
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_SMALLDOWNARROW),
    WS_CHILDWINDOW|WS_VISIBLE|WS_TABSTOP|Disabled,52,52,12,12,Win,
    (HMENU)100,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Drive B disk history"));
  Win=CreateWindowEx(WS_EX_ACCEPTFILES|512,WC_LISTVIEW,"",WS_CHILDWINDOW
    |WS_VISIBLE|WS_TABSTOP|LVS_ICON|LVS_SHAREIMAGELISTS|LVS_SINGLESEL
    |LVS_NOSCROLL,240,10,90,64,Handle,(HMENU)101,HInstance,NULL);
  ListView_SetIconSpacing(Win,88,200);
  ListView_SetImageList(Win,il[0],LVSIL_NORMAL);
  SetDriveViewEnable(1,0);
  {
    int ico=RC_ICO_HARDDRIVES;
    if(IsSameStr_I(T("File"),"Fichier")) ico=RC_ICO_HARDDRIVES_FR;
    Win=CreateWindow("Steem Flat PicButton",Str(ico),
      WS_CHILD|WS_VISIBLE|WS_TABSTOP|PBS_RIGHTCLICK,
      400,10,60,64,Handle,(HMENU)10,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,!HardDiskMan.DisableHardDrives,0);
#if defined(SSE_ACSI)
    ToolAddWindow(ToolTip,Win,
      T("GEMDOS Hard Drive Manager - right click to toggle on/off"));
    ico=RC_ICO_HARDDRIVES_ACSI;
    DWORD style=WS_CHILD|WS_VISIBLE|WS_TABSTOP|PBS_RIGHTCLICK;
    Win=CreateWindow("Steem Flat PicButton",Str(ico),style,
      400,10,60,64,Handle,(HMENU)11,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,SSEOptions.Acsi,0);
    ToolAddWindow(ToolTip,Win,
      T("ACSI Hard Drive Manager - right click to toggle on/off"));
#else
    ToolAddWindow(ToolTip,Win,T("Hard Drive Manager"));
#endif
  }
  SetWindowAndChildrensFont(Handle,Font);
  SetWindowLongPtr(GetDlgItem(Handle,98),GWLP_USERDATA,(LONG_PTR) this);
  SetWindowLongPtr(GetDlgItem(Handle,99),GWLP_USERDATA,(LONG_PTR) this);
  Old_ListView_WndProc=(WNDPROC)GetClassLongPtr(GetDlgItem(Handle,100),
    GCLP_WNDPROC);
  SetWindowLongPtr(GetDlgItem(Handle,100),GWLP_USERDATA,(LONG_PTR) this);
  SetWindowLongPtr(GetDlgItem(Handle,100),GWLP_WNDPROC,
    (LONG_PTR)DriveView_WndProc);
  SetWindowLongPtr(GetDlgItem(Handle,101),GWLP_USERDATA,(LONG_PTR) this);
  SetWindowLongPtr(GetDlgItem(Handle,101),GWLP_WNDPROC,
    (LONG_PTR)DriveView_WndProc);
  SetWindowLongPtr(GetDlgItem(Handle,102),GWLP_USERDATA,(LONG_PTR) this);
  SetWindowLongPtr(GetDlgItem(Handle,102),GWLP_WNDPROC,
    (LONG_PTR)DiskView_WndProc);
  for(int i=0;i<2;i++) 
  {
    if(FloppyDrive[i].DiskInDrive())
      InsertDisk(i,FloppyDisk[i].DiskName,FloppyDrive[i].GetDisk(),
        true,0,FloppyDisk[i].DiskInZip);
  }
  ShowWindow(Handle,int(MaximizeIt?SW_MAXIMIZE:SW_SHOW));
  UpdateWindow(Handle);
  SetDiskViewMode(SmallIcons?LVS_SMALLVIEW:LVS_ICON);
  RefreshDiskView();
  // point to disk in A: if possible
  if(FloppyDrive[0].NotEmpty())
  {
    TDiskManFileInfo *Inf=GetItemInf(0,GetDlgItem(Handle,100));
    EasyStr Fol=Inf->Path;
    char *slash=strrchr(Fol,SLASHCHAR);
    if(slash) 
      *slash=0;
    if(IsSameStr_I(Fol,DisksFol))
      GoToDisk(Inf->Path,0);
  }
  SetFocus(DiskView);
  if(StemWin) 
    PostMessage(StemWin,WM_USER,1234,0);
#endif//WIN32

#ifdef UNIX
  if (StandardShow(Width,Height,T("Disk Manager"),
        ICO16_DISKMAN,ExposureMask | StructureNotifyMask,
        (LPWINDOWPROC)WinProc,true)) return;

  SetWindowNormalSize(XD,Handle,10+32+10+10+10+60+60+10,110+50+10);

  int y=10;
  for(int d=0;d<2;d++){
    drive_icon[d].create(XD,Handle,10,y-2,32,32,button_notify_handler,this,
                          BT_ICON | BT_STATIC | BT_BORDER_NONE | BT_TEXT_CENTRE,
                          EasyStr(char('A'+d)),100+d,BkCol);
    drive_icon[d].set_icon(&Ico32,ICO32_DRIVE_A+d);

    disk_name[d].create(XD,Handle,43,y,320,25,button_notify_handler,this,
                          BT_TEXT | BT_STATIC | BT_TEXT_CENTRE | BT_BORDER_INDENT,
                          "",200+d,WhiteCol);

    eject_but[d].create(XD,Handle,Width-103,y+1,25,25,
              button_notify_handler,this,BT_ICON,"Eject",302+d,BkCol);
	  eject_but[d].set_icon(&Ico16,ICO16_EJECTDISK);
	
    SetWindowGravity(XD,eject_but[d].handle,NorthEastGravity);
    y+=34;
  }

  HardBut.create(XD,Handle,Width-70,10,60,60,
              button_notify_handler,this,BT_ICON,"",10,BkCol);
  if (IsSameStr_I(T("File"),"Fichier")){
    HardBut.set_icon(&Ico64,ICO64_HARDDRIVES_FR);
  }else{
    HardBut.set_icon(&Ico64,ICO64_HARDDRIVES);
  }
  SetWindowGravity(XD,HardBut.handle,NorthEastGravity);
  hints.add(HardBut.handle,T("Hard Drive Manager"),Handle);


  BackBut.create(XD,Handle,10,82,21,21,
              button_notify_handler,this,BT_ICON,"",2,BkCol);
  BackBut.set_icon(&Ico16,ICO16_BACK);
  hints.add(BackBut.handle,T("Back"),Handle);

  ForwardBut.create(XD,Handle,35,82,21,21,
              button_notify_handler,this,BT_ICON,"",3,BkCol);
  ForwardBut.set_icon(&Ico16,ICO16_FORWARD);
  hints.add(ForwardBut.handle,T("Forward"),Handle);

  HomeBut.create(XD,Handle,60,82,21,21,
              button_notify_handler,this,BT_ICON,"",4,BkCol);
  HomeBut.set_icon(&Ico16,ICO16_HOMEFOLDER);
  hints.add(HomeBut.handle,T("To home folder"),Handle);

  SetHomeBut.create(XD,Handle,85,82,21,21,
              button_notify_handler,this,BT_ICON,"",5,BkCol);
  SetHomeBut.set_icon(&Ico16,ICO16_SETHOMEFOLDER);
  hints.add(SetHomeBut.handle,T("Make this folder your home folder"),Handle);

  MenuBut.create(XD,Handle,110,82,21,21,
              button_notify_handler,this,BT_ICON,"",6,BkCol);
  MenuBut.set_icon(&Ico16,ICO16_DISKMANMENU);
  hints.add(MenuBut.handle,T("Disk Manager options"),Handle);

  DirOutput.create(XD,Handle,135,80,445,25,NULL,this,
                    BT_TEXT | BT_STATIC | BT_TEXT_PATH | BT_BORDER_INDENT,
                    DisksFol,0,WhiteCol);

  if (StemWin) DiskBut.set_check(true);

  dir_lv.ext_sl.DeleteAll();
  dir_lv.ext_sl.Add(3,T("Parent Directory"),1,1,1);
  dir_lv.ext_sl.Add(3,"",ICO16_FOLDER,ICO16_FOLDERLINK,ICO16_FOLDERLINKBROKEN);
  dir_lv.ext_sl.Add(4,"st",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);
  dir_lv.ext_sl.Add(4,"stt",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);
  dir_lv.ext_sl.Add(4,"dim",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);
  dir_lv.ext_sl.Add(4,"msa",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);
#if defined(SSE_GUI_DM_STW)
  dir_lv.ext_sl.Add(4,"stw",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);
#endif
#if defined(SSE_DISK_SCP)
  dir_lv.ext_sl.Add(4,"scp",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);
#endif
#if defined(SSE_DISK_HFE)
  dir_lv.ext_sl.Add(4,"hfe",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);
#endif
#if defined(SSE_DISK_CAPS)
  dir_lv.ext_sl.Add(4,"ipf",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);
  dir_lv.ext_sl.Add(4,"ctr",ICO16_DISK,ICO16_DISKLINK,ICO16_DISKLINKBROKEN,ICO16_DISK_RO);
#endif
  ArchiveTypeIdx=dir_lv.ext_sl.NumStrings;
  int zipicon=ICO16_ZIP_RO;
  if (FloppyArchiveIsReadWrite) zipicon=ICO16_ZIP_RW;
  dir_lv.ext_sl.Add(3,"zip",zipicon,ICO16_DISKLINK,ICO16_DISKLINKBROKEN);
  dir_lv.ext_sl.Add(3,"stz",zipicon,ICO16_DISKLINK,ICO16_DISKLINKBROKEN);
#if defined(RAR_SUPPORT) || defined(SSE_UNRAR_SUPPORT_UNIX)
  dir_lv.ext_sl.Add(3,"rar",zipicon,ICO16_DISKLINK,ICO16_DISKLINKBROKEN);
#endif
#if defined(SSE_7Z_SUPPORT_UNIX) // "7Z","BZ2","GZ","TAR","ARJ"
  dir_lv.ext_sl.Add(3,"7z",zipicon,ICO16_DISKLINK,ICO16_DISKLINKBROKEN);
  dir_lv.ext_sl.Add(3,"bz2",zipicon,ICO16_DISKLINK,ICO16_DISKLINKBROKEN);
  dir_lv.ext_sl.Add(3,"gz",zipicon,ICO16_DISKLINK,ICO16_DISKLINKBROKEN);
  dir_lv.ext_sl.Add(3,"tar",zipicon,ICO16_DISKLINK,ICO16_DISKLINKBROKEN);
  dir_lv.ext_sl.Add(3,"arj",zipicon,ICO16_DISKLINK,ICO16_DISKLINKBROKEN);
#endif
  dir_lv.lpig=&Ico16;
  dir_lv.base_fol="";
  dir_lv.fol=DisksFol;
  dir_lv.allow_type_change=0;
  dir_lv.show_broken_links=0; //(HideBroken==0);
  dir_lv.create(XD,Handle,10,110,285,120,dir_lv_notify_handler,this);

  SetNumFloppies(num_connected_floppies);

  UpdateDiskNames(0);
  UpdateDiskNames(1);

  XMapWindow(XD,Handle);
#endif//UNIX
}


void TDiskManager::Hide() {
  if(Handle==NULL) 
    return;

#ifdef WIN32
  HardDiskMan.Hide();
  if(HardDiskMan.Handle) 
    return;
  if(DatabaseDiag) // v4.0
    SendMessage(DatabaseDiag,WM_DESTROY,0,0);
#if defined(SSE_ACSI)//v4.0
  AcsiHardDiskMan.Hide();
  if(AcsiHardDiskMan.Handle) 
    return;
#endif
  ShowWindow(Handle,SW_HIDE);
  if(FullScreen) 
    SetFocus(StemWin);
  ToolsDeleteAllChildren(ToolTip,Handle);
  ToolsDeleteAllChildren(ToolTip,GetDlgItem(Handle,98));
  ToolsDeleteAllChildren(ToolTip,GetDlgItem(Handle,99));
  int c=(int)SendMessage(DiskView,LVM_GETITEMCOUNT,0,0);
  for(int i=0;i<c;i++)
    SendMessage(DiskView,LVM_DELETEITEM,0,0);
  DestroyWindow(Handle);Handle=NULL;DiskView=NULL;
  for(int n=0;n<2;n++) 
  {
    ImageList_Destroy(il[n]);
    il[n]=NULL;
  }
  if(StemWin) 
    PostMessage(StemWin,WM_USER,1234,0);
  ManageWindowClasses(SD_UNREGISTER);
#endif//WIN32

#ifdef UNIX
  if(XD==NULL)
    return;
  if(HardDiskMan.IsVisible())
    return;
  hints.remove_all_children(Handle);
  StandardHide();
  if(StemWin)
    DiskBut.set_check(0);
#endif
}


TDiskManager::~TDiskManager() { 
  Hide(); 
}


#ifdef WIN32

void TDiskManager::ManageWindowClasses(bool Unreg) {
  WNDCLASS wc;
  char *ClassName[3]={"Steem Disk Manager","Steem Disk Manager Dialog",
    "Steem Disk Manager Drive Icon"};
  if(Unreg)
    for(int n=0;n<3;n++) 
      UnregisterClass(ClassName[n],Inst);
  else 
  {
    RegisterMainClass(WndProc,ClassName[0],RC_ICO_DISKMAN);
    wc.style=CS_DBLCLKS;
    wc.lpfnWndProc=Dialog_WndProc;
    wc.cbClsExtra=0;
    wc.cbWndExtra=0;
    wc.hInstance=Inst;
    wc.hIcon=hGUIIconSmall[RC_ICO_DRIVE];
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszMenuName=NULL;
    wc.lpszClassName=ClassName[1];
    RegisterClass(&wc);
    wc.style=0;
    wc.lpfnWndProc=Drive_Icon_WndProc;
    wc.cbClsExtra=0;
    wc.cbWndExtra=0;
    wc.hInstance=Inst;
    wc.hIcon=NULL;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszMenuName=NULL;
    wc.lpszClassName=ClassName[2];
    RegisterClass(&wc);
  }
}


void TDiskManager::LoadIcons() {
  if(Handle==NULL) 
    return;
  HIMAGELIST old_il[2]={il[0],il[1]};
  HICON *pIcons=hGUIIcon;
  for(int n=0;n<2;n++) 
  {
    il[n]=ImageList_Create(32-n*16,32-n*16,
      BPPToILC[BytesPerPixel]|ILC_MASK,9+1+1,9+1+1);
    if(il[n]) 
    {
      ImageList_AddIcon(il[n],pIcons[RC_ICO_FOLDER]);
      ImageList_AddIcon(il[n],pIcons[RC_ICO_DRIVE]);
      ImageList_AddIcon(il[n],pIcons[RC_ICO_PARENTDIR]);
      ImageList_AddIcon(il[n],pIcons[RC_ICO_FOLDERLINK]);
      ImageList_AddIcon(il[n],pIcons[RC_ICO_DRIVELINK]);
      ImageList_AddIcon(il[n],pIcons[RC_ICO_DRIVEREADONLY]);
      ImageList_AddIcon(il[n],pIcons[RC_ICO_FOLDERBROKEN]);
      ImageList_AddIcon(il[n],pIcons[RC_ICO_DRIVEBROKEN]);
      if(FloppyArchiveIsReadWrite)
        ImageList_AddIcon(il[n],pIcons[RC_ICO_DRIVEZIPPED_RW]);
      else
        ImageList_AddIcon(il[n],pIcons[RC_ICO_DRIVEZIPPED_RO]);
      ImageList_AddIcon(il[n],pIcons[RC_ICO_PRGFILEICO]); //9
      ImageList_AddIcon(il[n],pIcons[RC_ICO_CFG]); //10
    }
    pIcons=hGUIIconSmall;
  }
  if(VisibleDiag()) 
    SetClassLongPtr(VisibleDiag(),GCLP_HICON,(LONG_PTR)(hGUIIconSmall[RC_ICO_DRIVE]));
  if(GetDlgItem(Handle,10)) 
  {
    // Update controls
    for(int id=80;id<90;id++)
      if(GetDlgItem(Handle,id)) 
        PostMessage(GetDlgItem(Handle,id),BM_RELOADICON,0,0);
    PostMessage(GetDlgItem(Handle,10),BM_RELOADICON,0,0);
#ifdef SSE_ACSI
    PostMessage(GetDlgItem(Handle,11),BM_RELOADICON,0,0);
#endif
    PostMessage(GetDlgItem(GetDlgItem(Handle,98),100),BM_RELOADICON,0,0);
    PostMessage(GetDlgItem(GetDlgItem(Handle,99),100),BM_RELOADICON,0,0);
    InvalidateRect(GetDlgItem(Handle,98),NULL,true);
    InvalidateRect(GetDlgItem(Handle,99),NULL,true);
    ListView_SetImageList(GetDlgItem(Handle,100),il[0],LVSIL_NORMAL);
    ListView_SetImageList(GetDlgItem(Handle,101),il[0],LVSIL_NORMAL);
    ListView_SetImageList(DiskView,il[0],LVSIL_NORMAL);
    ListView_SetImageList(DiskView,il[1],LVSIL_SMALL);
    SendMessage(DiskView,LVM_REDRAWITEMS,0,
      SendMessage(DiskView,LVM_GETITEMCOUNT,0,0));
  }
  for(int n=0;n<2;n++) 
    if(old_il[n]) 
      ImageList_Destroy(old_il[n]);
}


void TDiskManager::SetDiskViewMode(int Mode) {
  SetWindowLong(DiskView,GWL_STYLE,
    (GetWindowLong(DiskView,GWL_STYLE) & ~LVS_SMALLVIEW & ~LVS_ICON)|Mode);
  if(SmallIcons) 
  {
    TWidthHeight widh=GetTextSize(Font,"Width of y Line in small icon view");
    widh.Width/=2;
    if(IconSpacing==1) widh.Width*=2;
    if(IconSpacing==2) widh.Width*=4;
    ListView_SetColumnWidth(DiskView,DWORD(-1),18+widh.Width);
  }
  else 
  {
    TWidthHeight widh=GetTextSize(Font,"8");
    ListView_SetIconSpacing(DiskView,32+24+IconSpacing*12,38+(widh.Height+2)*2);
  }
  SendMessage(DiskView,LVM_SORTITEMS,0,(LPARAM)CompareFunc);
}


void TDiskManager::SetDir(EasyStr NewFol,bool AddToHistory,EasyStr SelPath,
                          bool EditLabel,EasyStr SelLinkPath,int iSelItem) {
  EasyStr Fol=NewFol;
  if(Fol.RightChar()!='\\' && Fol.RightChar()!='/') 
    Fol+=SLASH;
  WIN32_FIND_DATA wfd;
  HANDLE Find=FindFirstFile(Fol+"*.*",&wfd);
  if(Find!=INVALID_HANDLE_VALUE) 
  {
    SetCursor(LoadCursor(NULL,IDC_WAIT));
    {
      SetWindowText(GetDlgItem(Handle,97),Fol.Lefts(Fol.Length()-1).Text
        +MIN(3,Fol.Length()-1));
      int idx=(int)SendMessage(GetDlgItem(Handle,90),CB_FINDSTRING,0xffffffff,
        (LPARAM)((Fol.Lefts(2)+SLASH).Text));
      if(idx>-1) 
        SendMessage(GetDlgItem(Handle,90),CB_SETCURSEL,idx,0);
    }
    SendMessage(DiskView,WM_SETREDRAW,0,0);
    int c=(int)SendMessage(DiskView,LVM_GETITEMCOUNT,0,0);
    for(int i=0;i<c;i++)
      SendMessage(DiskView,LVM_DELETEITEM,0,0);
    if(SmallIcons) 
    {
      SetDiskViewMode(LVS_ICON);
      SetDiskViewMode(LVS_SMALLVIEW);
    }
    IShellLink *LinkObj=NULL;
    IPersistFile *FileObj=NULL;
    HRESULT hres=CoCreateInstance(CLSID_ShellLink,NULL,
      CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&LinkObj);
    if(SUCCEEDED(hres)==0) 
      LinkObj=NULL;
    if(LinkObj) 
    {
      hres=LinkObj->QueryInterface(IID_IPersistFile,(void**)&FileObj);
      if(SUCCEEDED(hres)==0) 
        FileObj=NULL;
    }
    DynamicArray<TDiskManFileInfo*> Files;
    Files.Resize(512); // Assume for 512 items
    Files.SizeInc=128; // Increase by 128 items at a time
    EasyStr Name,Path,Extension,LinkPath;
    char *exts;
    TDiskManFileInfo *Inf;
    bool Link,Broken;
    do {
      bool Add=true;
      if((wfd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)&&!ShowHiddenFiles)
        Add=0;
      else if(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) 
      {
        if(wfd.cFileName[0]=='.' && wfd.cFileName[1]==0)
          Add=0;
       // else if(wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
       // keep those, eg Frontpage's website folder // Add=0; 
       // TRACE("%s attrib %x add %d\n",wfd.cFileName,wfd.dwFileAttributes,Add);
      }
      if(Add) 
      {
        Link=0;
        Name=wfd.cFileName;
        Path=Fol+Name;
        LinkPath="";
        Extension="";
        Broken=0;
        if((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0) 
        {
          exts=strrchr(Name,'.');
          if(exts!=NULL) {
            if(HideExtension)
              *exts=0; //Strip extension from Name
            Extension=exts+1;
            strupr(Extension);
            if(IsSameStr_I(Extension,"LNK")) 
            {
              Link=true;
              LinkPath=Path;
              Path=GetLinkDest(Fol+wfd.cFileName,&wfd,NULL,LinkObj,FileObj);
              NO_SLASH(Path);
              if(Path.NotEmpty()&&DoExtraShortcutCheck) 
              {
                HANDLE hFind=FindFirstFile(Path,&wfd);
                if(hFind!=INVALID_HANDLE_VALUE) 
                {
                  FindClose(hFind);
                  EasyStr DestFilePath=Path;
                  RemoveFileNameFromPath(DestFilePath,WITH_SLASH);
                  Path=DestFilePath+wfd.cFileName;
                }
              }
              if(Path.NotEmpty()) 
              {
                UINT HostDriveType=GetDriveType(Path.Lefts(2)+SLASH);
                if(HostDriveType==DRIVE_NO_ROOT_DIR)
                  Broken=true;
                else if(HostDriveType!=DRIVE_REMOVABLE 
                  && HostDriveType!=DRIVE_CDROM) 
                {
                  if(Path.Length()!=2) 
                    Broken=(GetFileAttributes(Path)==0xffffffff);
                }
                exts=strrchr(wfd.cFileName,'.');
                if(exts!=NULL) 
                {
                  Extension=exts+1;
                  strupr(Extension);
                }
              }
            }
          }
        }
        if(Path.NotEmpty()&&(Broken==0||HideBroken==0)) 
        {
          if((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            &&!(Broken && wfd.nFileSizeLow==1)) 
          {
            Inf=new TDiskManFileInfo;
            Inf->Folder=true;
            Inf->ReadOnly=0;
            Inf->BrokenLink=Broken;
            Inf->Zip=0;
            if(Name=="..") 
            {
              Inf->Image=2;
              Inf->Name=T("Parent Directory");
              Str HigherPath=Fol;
              NO_SLASH(HigherPath);
              RemoveFileNameFromPath(HigherPath,REMOVE_SLASH);
              Inf->Path=HigherPath;
              Inf->LinkPath="";
              Inf->UpFolder=true;
            }
            else 
            {
              Inf->Image=Link*3+Broken*3;
              Inf->Name=Name;
              Inf->Path=Path;
              Inf->LinkPath=LinkPath;
              Inf->UpFolder=0;
            }
            Files.Add(Inf);
          }
          else 
          {
            int Type=ExtensionIsDisk(Extension);
            if(Type==DISK_UNCOMPRESSED||Type==DISK_PASTI||Type==DISK_IS_CONFIG) 
            {
              Inf=new TDiskManFileInfo;
              Inf->Name=Name;
              Inf->Path=Path;
              Inf->LinkPath=LinkPath;
              Inf->Folder=0;
              Inf->UpFolder=0;
              if(Broken)
                Inf->ReadOnly=0;
              else
                Inf->ReadOnly=(wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY);
              Inf->BrokenLink=Broken;
              Inf->Zip=0;
              Inf->Image=1+Link*3+Broken*3;
              if(Inf->ReadOnly && Link==0)
                Inf->Image=5;
#if defined(SSE_TOS_PRG_AUTORUN)
              if(OPTION_PRG_SUPPORT
                && MatchesAnyString_I(Extension,DISK_EXT_PRG,DISK_EXT_TOS,NULL))
                Inf->Image=9;
#endif
              if(Type==DISK_IS_CONFIG)
                Inf->Image=10;
              Files.Add(Inf);
            }
            else if(Type==DISK_COMPRESSED && enable_zip) 
            {
              Inf=new TDiskManFileInfo;
              Inf->Name=Name;
              Inf->Path=Path;
              Inf->LinkPath=LinkPath;
              Inf->Folder=0;
              Inf->UpFolder=0;
              Inf->ReadOnly=true;
              Inf->BrokenLink=Broken;
              Inf->Zip=true;
              Inf->Image=int(Link?4+Broken*3:8);
              Files.Add(Inf);
            }
          }
        }
      }
    } while(FindNextFile(Find,&wfd));
    FindClose(Find);
    if(LinkObj) 
      LinkObj->Release();
    if(FileObj) 
      FileObj->Release();
    SendMessage(DiskView,LVM_SETITEMCOUNT,Files.NumItems+16,0);
    LV_ITEM lvi;
    lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
    lvi.iItem=0;
    lvi.iSubItem=0;
    lvi.pszText=LPSTR_TEXTCALLBACK;
    for(int n=0;n<Files.NumItems;n++) 
    {
      lvi.lParam=(LPARAM)(Files[n]);
      lvi.iImage=Files[n]->Image;
      SendMessage(DiskView,LVM_INSERTITEM,0,LPARAM(&lvi));
    }
    Files.DeleteAll();
    SendMessage(DiskView,LVM_SORTITEMS,0,LPARAM(CompareFunc));
    if(NotSameStr_I(NewFol,DisksFol)) 
    {
      if(AddToHistory) 
      {
        HistForward[0]="";
        for(int n=9;n>0;n--)
          HistBack[n]=HistBack[n-1];
        HistBack[0]=DisksFol;
        EnableWindow(GetDlgItem(Handle,82),true);
        if(GetFocus()==GetDlgItem(Handle,83)) SetFocus(GetDlgItem(Handle,82));
        EnableWindow(GetDlgItem(Handle,83),0);
      }
      DisksFol=NewFol;
      NO_SLASH(DisksFol);
    }
    if(IsSameStr_I(DisksFol,HomeFol)) 
    {
      if(GetFocus()==GetDlgItem(Handle,80)||GetFocus()==GetDlgItem(Handle,81))
        SetFocus(GetDlgItem(Handle,90));
      AtHome=true;
    }
    else
      AtHome=0;
    if(SelPath.NotEmpty()||SelLinkPath.NotEmpty()) 
    {
      if(SelectItemWithPath(SelPath,EditLabel,SelLinkPath)==0)
        SelPath="",SelLinkPath="";
    }
    if(SelPath.IsEmpty()&&SelLinkPath.IsEmpty()) 
    {
      lvi.stateMask=LVIS_SELECTED|LVIS_FOCUSED;
      lvi.state=LVIS_SELECTED|LVIS_FOCUSED;
      iSelItem=bound(iSelItem,0,MAX((long)SendMessage(DiskView,
        LVM_GETITEMCOUNT,0,0)-1,0L));
      SendMessage(DiskView,LVM_SETITEMSTATE,iSelItem,(LPARAM)&lvi);
      SendMessage(DiskView,LVM_ENSUREVISIBLE,iSelItem,1);
    }
    SendMessage(DiskView,WM_SETREDRAW,1,0);
    InvalidateRect(DiskView,NULL,true);
    UpdateWindow(DiskView);
    SetCursor(PCArrow);
  }
}


int CALLBACK TDiskManager::CompareFunc(LPARAM lPar1,LPARAM lPar2,LPARAM) {
  //SS the place to change how we sort items, but need to edit TDiskManFileInfo
  TDiskManFileInfo *Inf1=(TDiskManFileInfo*)lPar1;
  TDiskManFileInfo *Inf2=(TDiskManFileInfo*)lPar2;
  if(Inf1->UpFolder)
    return -1;
  else if(Inf2->UpFolder)
    return 1;
  else if(Inf1->Folder && Inf2->Folder==0)
    return -1;
  else if(Inf1->Folder==0&&Inf2->Folder)
    return 1;
  else
    return strcmpi(Inf1->Name.Text,Inf2->Name.Text);
}


bool TDiskManager::SelectItemWithPath(char *Path,bool EditLabel,char *LinkPath) {
  int c=(int)SendMessage(DiskView,LVM_GETITEMCOUNT,0,0);
  bool Match;
  LV_ITEM lvi;
  lvi.mask=LVIF_PARAM;
  lvi.iSubItem=0;
  for(lvi.iItem=0;lvi.iItem<c;lvi.iItem++) 
  {
    SendMessage(DiskView,LVM_GETITEM,0,(LPARAM)&lvi);
    Match=true;
    if(Path)     
      Match&=bool(Path[0] ? 
        IsSameStr_I(((TDiskManFileInfo*)lvi.lParam)->Path,Path):true);
    if(LinkPath) 
      Match&=bool(LinkPath[0] ?
        IsSameStr_I(((TDiskManFileInfo*)lvi.lParam)->LinkPath,LinkPath):true);
    if(Match) 
    {
      lvi.stateMask=LVIS_SELECTED|LVIS_FOCUSED;
      lvi.state=LVIS_SELECTED|LVIS_FOCUSED;
      SendMessage(DiskView,LVM_SETITEMSTATE,lvi.iItem,(LPARAM)&lvi);
      SendMessage(DiskView,LVM_ENSUREVISIBLE,lvi.iItem,1);
      if(EditLabel) 
        SendMessage(DiskView,LVM_EDITLABEL,lvi.iItem,0);
      return true;
    }
  }
  return 0;
}


int TDiskManager::GetSelectedItem() {
  int c=(int)SendMessage(DiskView,LVM_GETITEMCOUNT,0,0);
  LV_ITEM lvi;
  lvi.iSubItem=0;
  for(lvi.iItem=0;lvi.iItem<c;lvi.iItem++) 
  {
    if(SendMessage(DiskView,LVM_GETITEMSTATE,lvi.iItem,LVIS_SELECTED))
      return lvi.iItem;
  }
  return -1;
}


bool TDiskManager::HasHandledMessage(MSG *mess) {
  if(Handle!=NULL && Dragging==-1) 
  {
    if(VisibleDiag())
      return (IsDialogMessage(VisibleDiag(),mess)!=0);
    else
      return (IsDialogMessage(Handle,mess)!=0);
  }
  else
    return 0;
}


void TDiskManager::AddFoldersToMenu(HMENU Pop,int StartID,EasyStr NoAddFol,
                                    bool Setting) {
  int MaxWidth=GuiSM.cx_screen()/2;
  if(NotSameStr_I(HomeFol,NoAddFol)) 
  {
    InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,StartID,EasyStr(Setting?
      "(":"")+ShortenPath(HomeFol,Font,MaxWidth)+(Setting?")":""));
    InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,1999,NULL);
  }
  if(Setting)
    StartID+=5;
  else
    StartID++;
  for(int n=0;n<10;n++) 
  {
    if(Setting) 
    {
      HMENU OptionsPop=CreatePopupMenu();
      InsertMenu(OptionsPop,0xffffffff,MF_BYPOSITION|MF_STRING,StartID+0,
        T("Change to Current Folder"));
      InsertMenu(OptionsPop,0xffffffff,MF_BYPOSITION|MF_STRING,StartID+1,
        T("Change to..."));
      InsertMenu(OptionsPop,0xffffffff,MF_BYPOSITION|MF_STRING,StartID+2,
        T("Erase"));
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MF_POPUP,
        (UINT_PTR)OptionsPop,EasyStr(1+n)+": ("+ShortenPath(QuickFol[n],
          Font,MaxWidth)+")");
      StartID+=5;
    }
    else if(QuickFol[n].NotEmpty()) 
    {
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|
        (IsSameStr_I(QuickFol[n],NoAddFol)?(MF_DISABLED|MF_GRAYED):0),
        StartID++,EasyStr(1+n)+": "+ShortenPath(QuickFol[n],Font,MaxWidth));
    }
    else
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MF_DISABLED|MF_GRAYED,
        StartID++,EasyStr(1+n)+":");
  }
}


Str TDiskManager::GetMSAConverterPath() {
  if(MSAConvPath.NotEmpty())
  {
    if(Exists(MSAConvPath))
      return MSAConvPath;
  }
#if defined(_DEBUG)
  BRK(Auto MSA path wont work in _DEBUG);
#else
  // First check if msa.exe is on the steem path, if yes, no need to ask player
  EasyStr str(RunDir);
  str+=SLASH SSE_PLUGIN_DIR1 SLASH "msa.exe";
  if(Exists(str))
  {
    MSAConvPath=str;
    return MSAConvPath;
  }
  str=RunDir+SLASH SSE_PLUGIN_DIR1 SLASH "msa.exe";
  if(Exists(str))
  {
    MSAConvPath=str;
    return MSAConvPath;
  }
  str=RunDir+"\\msa.exe";
  if(Exists(str))
  {
    MSAConvPath=str;
    return MSAConvPath;
  }
#endif	// NDBG
  int i=Alert(T("Have you installed MSA Converter elsewhere on this computer?"),
    T("Run MSA Converter"),MB_ICONQUESTION|MB_YESNO);
  if(i==IDYES) 
  {
    Str Fol=MSAConvPath;
    if(Fol.NotEmpty())
      RemoveFileNameFromPath(Fol,REMOVE_SLASH);
    else 
    {
      Fol="C:\\Program Files"; //?
      ITEMIDLIST *idl;
      if(SHGetSpecialFolderLocation(NULL,CSIDL_PROGRAM_FILES,&idl)==NOERROR) 
      {
        IMalloc *Mal;SHGetMalloc(&Mal);
        Fol.SetLength(MAX_PATH);
        SHGetPathFromIDList(idl,Fol);
        Mal->Free(idl);
      }
      NO_SLASH(Fol);
    }
    EnableAllWindows(0,Handle);
    EasyStr NewMSA=FileSelect(HWND(FullScreen?StemWin:Handle),
      T("Run MSA Converter"),
      Fol,FSTypes(1,T("Executables").Text,"*.exe",NULL),1,true,"exe");
    if(NewMSA.NotEmpty()) 
      MSAConvPath=NewMSA;
    SetForegroundWindow(Handle);
    EnableAllWindows(true,Handle);
    return MSAConvPath;
  }
  else 
  {
    i=Alert(T("MSA Converter is a free Windows program to edit disk images and convert them between different formats.")+" "+
      T("It has great features like converting archives containing files into disk images.")+"\r\n\r\n"+
      T("Would you like to open the MSA Converter website now so you can find out more and download it?"),
      T("Run MSA Converter"),MB_ICONQUESTION|MB_YESNO);
    if(i==IDYES) ShellExecute(NULL,NULL,MSACONV_WEB,"","",SW_SHOWNORMAL);
  }
  return "";
}


void TDiskManager::AddFileOrFolderContextMenu(HMENU Pop,TDiskManFileInfo *Inf) {
  bool AddProperties=0;
  if(Inf->UpFolder==0) 
  {
    if(Inf->BrokenLink==0) 
    {
      if(Inf->Folder==0) 
      {
        int MultiDisk=0;
        HMENU IAPop=NULL,IBPop=NULL,IRRPop=NULL;
        MenuESL.DeleteAll();
        MenuESL.Sort=eslSortByNameI;
        if(Inf->Zip) 
        {
          zippy.list_contents(Inf->Path,&MenuESL,true);
          if(MenuESL.NumStrings>1) 
          {
            MultiDisk=MF_POPUP;
            IAPop=CreatePopupMenu(),IBPop=CreatePopupMenu(),
              IRRPop=CreatePopupMenu();
            for(int i=0;i<MIN(MenuESL.NumStrings,200);i++) 
            {
              InsertMenu(IAPop,0xffffffff,MF_BYPOSITION|MF_STRING,9000+i,
                MenuESL[i].String);
              InsertMenu(IBPop,0xffffffff,MF_BYPOSITION|MF_STRING,9200+i,
                MenuESL[i].String);
              InsertMenu(IRRPop,0xffffffff,MF_BYPOSITION|MF_STRING,9400+i,
                MenuESL[i].String);
            }
          }
        }
        AddProperties=true;
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MultiDisk,
          (UINT_PTR)((MultiDisk==0)?1010:UINT_PTR(IAPop)),T("Insert Into Drive &A"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MultiDisk,
          (UINT_PTR)((MultiDisk==0)?1011:UINT_PTR(IBPop)),T("Insert Into Drive &B"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MultiDisk,
          (UINT_PTR)((MultiDisk==0)?1012:UINT_PTR(IRRPop)),T("Insert, Reset and &Run"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1015,
          T("Get &Contents"));
        HMENU ContentsPop=CreatePopupMenu();
        AddFoldersToMenu(ContentsPop,7000,"",0);
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MF_POPUP,
          UINT_PTR(ContentsPop),T("Get Contents and Create Shortcuts In"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,0,NULL);
        if(Inf->LinkPath.NotEmpty()) 
        {
          InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1090,
            T("&Go To Disk"));
          InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1092,
            T("Open Disk's Folder in Explorer"));
          InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
        }
        if(Inf->Zip==0) 
        {
          Inf->ReadOnly=(access(Inf->Path,2)!=0);
          InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
            |UINT_PTR(Inf->ReadOnly?MF_CHECKED:0),1040,T("Read &Only"));
          InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
        }
        else
        {
          if(MenuESL.NumStrings)
          {    // There are disks in archive
            if(MultiDisk)
              InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1080,
                T("E&xtract Disks Here"));
            else
              InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1080,
                T("E&xtract Disk Here"));
            InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
          }
        }
        HMENU MSAPop=CreatePopupMenu();
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_POPUP,(UINT_PTR)MSAPop,
          "MSA Converter");
        int FileZip=0;
        if(Inf->Zip) 
        {
          //SS MSA Converter doesn't work with RAR files
          if(MenuESL.NumStrings==0&&has_extension(Inf->Path,"zip")) 
            FileZip=true;
        }
        if(FileZip)
          InsertMenu(MSAPop,0xffffffff,MF_BYPOSITION|MF_STRING,2034,T("Convert to Disk Image"));
        else 
        {
          InsertMenu(MSAPop,0xffffffff,MF_BYPOSITION|MF_STRING,2031,T("Open Disk Image"));
          bool AddedSep=0;
          for(int d=2;d<26;d++) 
          {
            if(mount_flag[d]) 
            {
              if(AddedSep==0) 
              {
                InsertMenu(MSAPop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
                AddedSep=true;
              }
              InsertMenu(MSAPop,0xffffffff,MF_BYPOSITION|MF_STRING,2040+d,T("Extract Contents to ST Hard Drive")+" "+char('A'+d)+":");
            }
          }
        }
#if defined(SSE_DISK_STW)
/*  To help our MFM disk image format, we add a right click option to convert
    regular images to STW.
    Works with archives too.
*/
        if(!Inf->Folder&&!Inf->UpFolder &&!pasti_active) // pasti_active would interfere in SetDisk
        {
          char *ext=NULL;
          char *dot=strrchr(Inf->Path,'.');
          if(dot) 
            ext=dot+1;
          if(ext&&(IsSameStr_I(ext,DISK_EXT_ST)||IsSameStr_I(ext,DISK_EXT_MSA)
            ||IsSameStr_I(ext,DISK_EXT_DIM)||Inf->Zip))
            InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1041,
              T("Convert to ST&W"));
        }
#endif
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
      }
      else 
      {
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,
          1060,T("Open in &Explorer"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,
          1061,EasyStr(T("&Find..."))+" \10F3");
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
      }
    }
    else 
    {
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1070,T("&Fix Shortcut"));
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
    }
    if(Inf->LinkPath.NotEmpty()) 
    {
      HMENU MoveLinkPop=CreatePopupMenu();
      AddFoldersToMenu(MoveLinkPop,6060,DisksFol,0);
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MF_POPUP,
        (UINT_PTR)MoveLinkPop,T("&Move Shortcut To"));
      HMENU CopyLinkPop=CreatePopupMenu();
      AddFoldersToMenu(CopyLinkPop,6080,DisksFol,0);
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MF_POPUP,
        (UINT_PTR)CopyLinkPop,T("&Copy Shortcut To"));
    }
    EasyStr MoveToText=T("&Move Disk To"),CopyToText=T("&Copy Disk To"),
      ShortcutToText=T("Create &Shortcut To Disk In");
    if(Inf->Folder) 
    {
      MoveToText=T("&Move Folder To"),CopyToText=T("&Copy Folder To"),
        ShortcutToText=T("Create &Shortcut To Folder In");
    }
    Str FolderContainingDisk=Inf->Path;
    RemoveFileNameFromPath(FolderContainingDisk,REMOVE_SLASH);
    HMENU MovePop=CreatePopupMenu();
    AddFoldersToMenu(MovePop,6000,FolderContainingDisk,0);
    InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MF_POPUP,
      (UINT_PTR)MovePop,MoveToText);
    HMENU CopyPop=CreatePopupMenu();
    AddFoldersToMenu(CopyPop,6020,FolderContainingDisk,0);
    InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MF_POPUP,
      (UINT_PTR)CopyPop,CopyToText);
    if(Inf->LinkPath.Empty()) 
    {
      HMENU LinkPop=CreatePopupMenu();
      AddFoldersToMenu(LinkPop,6040,"",0);
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MF_POPUP,
        (UINT_PTR)LinkPop,
        ShortcutToText);
    }
    InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
    InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1020,
      EasyStr(T("&Rename"))+" \10F2");
    InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1030,
      EasyStr(T("Delete"))+" \10DEL");
    InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
    if(AddProperties) {
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1099,T("Properties"));
    }
    InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
  }
}


void TDiskManager::GoToDisk(Str Path,bool Refresh) {
  EasyStr Fol=Path;
  char *slash=strrchr(Fol,SLASHCHAR);
  if(slash) 
    *slash=0;
  if(IsSameStr_I(Fol,DisksFol)==0)
    SetDir(Fol,true,Path);
  else if(Refresh)
    RefreshDiskView(Path);
  else
    SelectItemWithPath(Path);
  SetFocus(DiskView);
}


#if defined(SSE_DISK_STW) && !defined(SSE_DISK_AUTOSTW)
/*  Helper function for case 1041
    What counts here is code size, not performance
    write_mode 0 normal
         1 missing clock
         2 crc
*/

void wd1772_write_stw(BYTE data,TWD1772MFM* wd1772mfm,TWD1772Crc* wd1772crc,
  int& p,int write_mode) {
  if(!write_mode)
    wd1772crc->Add(data);
  wd1772mfm->data=data;
  wd1772mfm->Encode((write_mode==1)?(TWD1772MFM::FORMAT_CLOCK)
    :(TWD1772MFM::NORMAL_CLOCK));
  if(write_mode==1)
    wd1772crc->Reset();
  ImageSTW[1].SetMfmData(p++,wd1772mfm->encoded);
}

#define WD1772_WRITE(d) wd1772_write_stw(d,&wd1772mfm,&wd1772crc,p,0);
#define WD1772_WRITE_A1 wd1772_write_stw(0xA1,&wd1772mfm,&wd1772crc,p,1);
#define WD1772_WRITE_CRC(d) wd1772_write_stw(d,&wd1772mfm,&wd1772crc,p,2);

#endif


#define GET_THIS This=(TDiskManager*)GetWindowLongPtr(Win,GWLP_USERDATA);
//#pragma warning (disable: 4701) //lvi in case WM_USER//390

LRESULT CALLBACK TDiskManager::WndProc(HWND Win,UINT Mess,WPARAM wPar,
                                      LPARAM lPar) {
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if(StemDialog_RetDefVal) 
    return Ret;
  TDiskManager *This;
  WORD wpar_lo=LOWORD(wPar);
  WORD wpar_hi=HIWORD(wPar);
  switch(Mess) {
  case WM_COMMAND:
  {
    GET_THIS;
    switch(wpar_lo) {
    case IDCANCEL: //Esc
    {
      if(This->Dragging>-1) 
        break;
      int SelItem=This->GetSelectedItem();
      if(SelItem>-1) 
      {
        TDiskManFileInfo *Inf=This->GetItemInf(SelItem);
        if(Inf->LinkPath.NotEmpty())
          This->RefreshDiskView("",0,Inf->LinkPath);
        else
          This->RefreshDiskView(Inf->Path);
      }
      else
        This->RefreshDiskView();
      break;
    }
    case 10:  //Hard Drives
    {
      HWND icon_handle=GetDlgItem(Win,10);
      if(SendMessage(icon_handle,BM_GETCLICKBUTTON,0,0)==2)
      { //Right click on HD manager icon toggles HD
        HardDiskMan.DisableHardDrives=!HardDiskMan.DisableHardDrives;
        TRACE_INIT("Option GEMDOS HD %d\n",!HardDiskMan.DisableHardDrives);
        HardDiskMan.update_mount();
        REFRESH_STATUS_BAR;
        SendMessage(icon_handle,BM_SETCHECK,!HardDiskMan.DisableHardDrives,0);
      }
      else if(wpar_hi==BN_CLICKED)  //left
      {
        SendMessage(icon_handle,BM_SETCHECK,1,0);
        HardDiskMan.Show();
      }
      break;
    }
#if defined(SSE_ACSI)
    case 11:
    {
      HWND icon_handle=GetDlgItem(Win,wpar_lo);
      if(SendMessage(icon_handle,BM_GETCLICKBUTTON,0,0)==2) //right click
      {
        SSEOptions.Acsi=!SSEOptions.Acsi;
        TRACE_INIT("Option ACSI %d\n",SSEOptions.Acsi);
#if defined(SSE_ACSI)
        if(SSEOptions.Acsi)
          load_TOS(ROMFile); // for STE speedboot hack
#endif
        REFRESH_STATUS_BAR;
        SendMessage(icon_handle,BM_SETCHECK,SSEOptions.Acsi,0);
      }
      else if(wpar_hi==BN_CLICKED) 
      {
        SendMessage(icon_handle,BM_SETCHECK,1,0);
        AcsiHardDiskMan.Show();
      }
      break;
    }
#endif
    case 80:  // Go Home
      if(This->Dragging>-1) 
        break;
      if(wpar_hi==BN_CLICKED) 
      {
        bool InHome=IsSameStr_I(This->HomeFol,This->DisksFol);
        if(SendMessage(HWND(lPar),BM_GETCLICKBUTTON,0,0)==2||InHome) 
        {
          SendMessage(HWND(lPar),BM_SETCHECK,1,0);
          HMENU Pop=CreatePopupMenu();
          This->AddFoldersToMenu(Pop,5000,This->DisksFol,0);
          RECT rc;
          GetWindowRect(HWND(lPar),&rc);
          TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
            rc.left,rc.bottom,0,Win,NULL);
          DestroyMenu(Pop);
          SendMessage(HWND(lPar),BM_SETCHECK,0,0);
        }
        else 
          This->SetDir(This->HomeFol,true);
      }
      break;
    case 81:  // Set Home
      if(wpar_hi==BN_CLICKED) 
      {
        SendMessage(HWND(lPar),BM_SETCHECK,1,0);
        if(SendMessage(HWND(lPar),BM_GETCLICKBUTTON,0,0)==2
          ||IsSameStr_I(This->HomeFol,This->DisksFol)) 
        {
          HMENU Pop=CreatePopupMenu();
          This->AddFoldersToMenu(Pop,8000,This->DisksFol,true);
          RECT rc;
          GetWindowRect(HWND(lPar),&rc);
          TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
            rc.left,rc.bottom,0,Win,NULL);
          DestroyMenu(Pop);
        }
        else 
        {
          if(Alert(This->DisksFol+"\n\n"
            +T("Are you sure you want to make this folder your new home folder?"),
            T("Change Home Folder?"),MB_YESNO|MB_ICONQUESTION)==IDYES)
            This->HomeFol=This->DisksFol;
        }
        SendMessage(HWND(lPar),BM_SETCHECK,0,0);
      }
      break;
    case 82:  // Back
      if(This->Dragging>-1) 
        break;
      if(wpar_hi==BN_CLICKED) 
      {
        if(SendMessage(HWND(lPar),BM_GETCLICKBUTTON,0,0)==2) 
        {
          SendMessage(HWND(lPar),BM_SETCHECK,1,0);
          HMENU Pop=CreatePopupMenu();
          for(int n=0;n<10;n++) 
          {
            if(This->HistBack[n].NotEmpty()) 
            {
              EasyStr Name=GetFileNameFromPath(This->HistBack[n]);
              if(Name.Empty()) 
                Name=This->HistBack[n];
              InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,5040+n,Name);
            }
          }
          RECT rc;
          GetWindowRect(HWND(lPar),&rc);
          TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
            rc.left,rc.bottom,0,Win,NULL);
          DestroyMenu(Pop);
          SendMessage(HWND(lPar),BM_SETCHECK,0,0);
        }
        else 
        {
          for(int n=9;n>0;n--)
            This->HistForward[n]=This->HistForward[n-1];
          This->HistForward[0]=This->DisksFol;
          This->SetDir(This->HistBack[0],0);
          for(int n=0;n<9;n++) 
            This->HistBack[n]=This->HistBack[n+1];
          This->HistBack[9]="";
          EnableWindow(GetDlgItem(Win,83),true);
          if(This->HistBack[0].IsEmpty()) 
          {
            if(GetFocus()==HWND(lPar)) 
              SetFocus(GetDlgItem(Win,83));
            EnableWindow(HWND(lPar),0);
          }
        }
      }
      break;
    case 83:  // Forward
      if(This->Dragging>-1) 
        break;
      if(wpar_hi==BN_CLICKED) 
      {
        if(SendMessage(HWND(lPar),BM_GETCLICKBUTTON,0,0)==2) 
        {
          SendMessage(HWND(lPar),BM_SETCHECK,1,0);
          HMENU Pop=CreatePopupMenu();
          for(int n=0;n<10;n++) 
          {
            if(This->HistForward[n].NotEmpty()) 
            {
              EasyStr Name=GetFileNameFromPath(This->HistForward[n]);
              if(Name.Empty()) 
                Name=This->HistForward[n];
              InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,5060+n,Name);
            }
          }
          RECT rc;
          GetWindowRect(HWND(lPar),&rc);
          TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
            rc.left,rc.bottom,0,Win,NULL);
          DestroyMenu(Pop);
          SendMessage(HWND(lPar),BM_SETCHECK,0,0);
        }
        else 
        {
          for(int n=9;n>0;n--)
            This->HistBack[n]=This->HistBack[n-1];
          This->HistBack[0]=This->DisksFol;
          This->SetDir(This->HistForward[0],0);
          for(int n=0;n<9;n++)
            This->HistForward[n]=This->HistForward[n+1];
          This->HistForward[9]="";
          EnableWindow(GetDlgItem(Win,82),true);
          if(This->HistForward[0].IsEmpty()) 
          {
            if(GetFocus()==HWND(lPar)) SetFocus(GetDlgItem(Win,82));
            EnableWindow(HWND(lPar),0);
          }
        }
      }
      break;
    case 84:  // Options
      if(This->Dragging>-1) 
        break;
      if(wpar_hi==BN_CLICKED) 
      {
        SendMessage(HWND(lPar),BM_SETCHECK,1,0);
        HMENU Pop=CreatePopupMenu();
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
          |(!floppy_instant_sector_access ? MF_CHECKED : 0),2013,
          T("Accurate Disk Access Times (Slow)"));
        ADVANCED_BEGIN
          InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
            |(OPTION_COUNT_DMA_CYCLES ? MF_CHECKED : 0),2019,
            T("Count DMA transfer cycles"));
        ADVANCED_END
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
          |(floppy_access_ff ? MF_CHECKED : 0),2018,
          T("Automatic fast forward on disk access"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
          |(FloppyArchiveIsReadWrite ? MF_CHECKED:0),2014,
          T("Read/Write Archives (Changes Lost On Eject)"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
          |((num_connected_floppies==1) ? MF_CHECKED : 0),2012,
          T("Disconnect Drive B"));
#if defined(SSE_DISK_AUTOSTW)
        ADVANCED_BEGIN
          InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
            |(OPTION_AUTOSTW ? MF_CHECKED : 0),2025,T("MFM emulation"));
        ADVANCED_END
#endif
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,1999,NULL);
#if USE_PASTI
        if(hPasti) 
        {
          ADVANCED_BEGIN
            InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
              |(pasti_active ? MF_CHECKED : 0),2023,
              T("Use Pasti for all floppies and ACSI"));
          ADVANCED_END
          InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,2024,
            T("Pasti Configuration"));
          InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,1999,NULL);
        }
#endif
#if defined(SSE_DISK_GHOST)
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
          |(OPTION_GHOST_DISK ? MF_CHECKED : 0),2027,
          T("Enable ghost disks for protected disks"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,1999,NULL);
#endif
#if defined(SSE_TOS_PRG_AUTORUN)
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
          |(OPTION_PRG_SUPPORT ? MF_CHECKED : 0),2028,
          T("Run standalone PRG and TOS files"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,1999,NULL);
#endif
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
          |(This->AutoInsert2 ? MF_CHECKED : 0),2016,
          T("Automatically Insert &Second Disk"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
          |(This->EjectDisksWhenQuit ? MF_CHECKED : 0),2011,
          T("E&ject Disks When Quit"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,1999,NULL);
        HMENU DCActionPop=CreatePopupMenu();
        InsertMenu(DCActionPop,0xffffffff,MF_BYPOSITION|MF_STRING,
          2007,T("&None"));
        InsertMenu(DCActionPop,0xffffffff,MF_BYPOSITION|MF_STRING,
          2008,T("Insert In &Drive A"));
        InsertMenu(DCActionPop,0xffffffff,MF_BYPOSITION|MF_STRING,
          2009,T("Insert, &Reset and Run"));
        CheckMenuRadioItem(DCActionPop,2007,2009,
          2007+This->DoubleClickAction,MF_BYCOMMAND);
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MF_POPUP,
          (UINT_PTR)DCActionPop,T("Double Click Disk &Action"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,1999,NULL);
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|
          UINT_PTR(This->CloseAfterIRR?MF_CHECKED:0),
          2015,T("&Close Disk Manager After Insert, Reset and Run"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,1999,NULL);
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
          |(This->ShowHiddenFiles ? MF_CHECKED : 0),2029,
          T("Show &Hidden Files"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
          |(This->HideBroken ? MF_CHECKED : 0),2002,T("Hide &Broken Shortcuts"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
          |(This->HideExtension ? MF_CHECKED : 0),2017,T("Hide E&xtension"));
        HMENU IconSizePop=CreatePopupMenu();
        InsertMenu(IconSizePop,0xffffffff,MF_BYPOSITION|MF_STRING,
          2005,T("&Large"));
        InsertMenu(IconSizePop,0xffffffff,MF_BYPOSITION|MF_STRING,
          2006,T("&Small"));
        CheckMenuRadioItem(IconSizePop,2005,2006,
          2005+This->SmallIcons,MF_BYCOMMAND);
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MF_POPUP,
          (UINT_PTR)IconSizePop,T("&Icon Size"));
        HMENU SpacingPop=CreatePopupMenu();
        InsertMenu(SpacingPop,0xffffffff,MF_BYPOSITION|MF_STRING,2020,
          T("Thin"));
        InsertMenu(SpacingPop,0xffffffff,MF_BYPOSITION|MF_STRING,2021,
          T("Normal"));
        InsertMenu(SpacingPop,0xffffffff,MF_BYPOSITION|MF_STRING,2022,
          T("Wide"));        
        CheckMenuRadioItem(SpacingPop,2020,2022,2020+This->IconSpacing,
          MF_BYCOMMAND);
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|MF_POPUP,
          (UINT_PTR)SpacingPop,T("&Icon Spacing"));
        RECT rc;
        GetWindowRect(HWND(lPar),&rc);
        TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
          rc.left,rc.bottom,0,Win,NULL);
        DestroyMenu(Pop);
        SendMessage(HWND(lPar),BM_SETCHECK,0,0);
      }
      break;
    case 85:  // Options
      if(This->Dragging>-1) 
        break;
      if(wpar_hi==BN_CLICKED) 
      {
        SendMessage(HWND(lPar),BM_SETCHECK,1,0);
        HMENU Pop=CreatePopupMenu();
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,2026,
          T("Search Disk Image Database")+"\10F9");
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,0,NULL);
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,2003,
          T("Open Current Folder In &Explorer")+"\10F4");
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
          |(This->ExplorerFolders ? MF_CHECKED : 0),2004,
          T("&Folders Pane in Explorer"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,2010,
          T("Find In Current Folder"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,2030,
          T("Run MSA Converter")+"\10F6");
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,0,NULL);
#ifndef SSE_NO_WINSTON_IMPORT
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,2001,T("Import &WinSTon Favourites"));
#endif
        RECT rc;
        GetWindowRect(HWND(lPar),&rc);
        TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
          rc.left,rc.bottom,0,Win,NULL);
        DestroyMenu(Pop);
        SendMessage(HWND(lPar),BM_SETCHECK,0,0);
      }
      break;
    case IDOK:  //Return
      if(This->Dragging>-1) 
        break;
      if(GetFocus()==This->DiskView)
        PostMessage(Win,WM_USER,1234,0);
      break;
    case 90:  //Drive Combo
      if(This->Dragging>-1) 
        break;
      if(wpar_hi==CBN_SELENDOK) 
      {
        char Text[8];
        SendMessage((HWND)lPar,CB_GETLBTEXT,SendMessage((HWND)lPar,
          CB_GETCURSEL,0,0),(LPARAM)Text);
        if(Text[0]!=This->DisksFol[0]) 
        {
          This->SetDir(Text,true);
          if(Text[0]!=This->DisksFol[0]) 
          {
            int idx=(int)SendMessage((HWND)lPar,CB_FINDSTRING,0xffffffff,
              (LPARAM)((This->DisksFol.Lefts(2)+SLASH).Text));
            if(idx>-1) 
              SendMessage((HWND)lPar,CB_SETCURSEL,idx,0);
          }
        }
      }
      break;
    case 1000:
    {
      EasyStr FolName=This->DisksFol+SLASH+T("New Folder");
      int n=2;
      while(GetFileAttributes(FolName)<0xFFFFFFFF)
        FolName=This->DisksFol+SLASH+T("New Folder")+" ("+(n++)+")";
      CreateDirectory(FolName,NULL);
      This->RefreshDiskView(FolName,true);
      break;
    }
    case 1001:
#if defined(SSE_DISK_HD)
    case 1006: // HD ST disk image, 18 sectors instead of 9
#endif
    {
      EasyStr STName=This->DisksFol+SLASH+T("Blank Disk")+".st";
      int n=2;
      while(Exists(STName))
        STName=This->DisksFol+SLASH+T("Blank Disk")+" ("+(n++)+").st";
#if defined(SSE_DISK_HD)
      WORD sectors=(wpar_lo==1006) ? 18 : 9;
      if(This->CreateDiskImage(STName,sectors*2*80,sectors,2))
#else
      if(This->CreateDiskImage(STName,1440,9,2))
#endif
        This->RefreshDiskView(STName,true);
      else
        Alert(EasyStr(T("Could not create the disk image "))+STName,
          T("Error"),MB_ICONEXCLAMATION);
      return 0;
    }
    case 1002:  // Custom disk image
      This->ShowDiskDiag();
      break;
#if defined(SSE_DISK_STW) || defined(SSE_DISK_HFE)
    case 1003: // New STW Disk Image
    case 1004: // New HFE Disk Image
#if defined(SSE_DISK_HD)
    case 1007: // New HD STW Disk Image
#endif
    {
      char *extension=(wPar==1004)?DISK_EXT_HFE:DISK_EXT_STW;
      EasyStr STName=This->DisksFol+SLASH+extension+" Disk."+extension;
      int n=2;
      while(Exists(STName))
        STName=This->DisksFol+SLASH+extension+" Disk ("+(n++)+")."+extension;
      if((wPar==1003)// ugly C++
#if defined(SSE_DISK_STW)
        &&ImageSTW[0].Create(STName)
#if defined(SSE_DISK_HD)
        ||(wPar==1007) &&ImageSTW[0].Create(STName,2) // HD
#endif
#endif
        ||(wPar==1004)
#if defined(SSE_DISK_HFE)
        &&ImageHFE[0].Create(STName)
#endif
        ) 
        This->RefreshDiskView(STName,true);
      else
        Alert(EasyStr(T("Could not create the disk image "))+STName,
          T("Error"),MB_ICONEXCLAMATION);
      return 0;
    }
#endif
    case 1005:
      PostMessage(Win,WM_COMMAND,IDCANCEL,0);
      break;
    case 1010:case 1011:case 1012:  //Insert in A/B/RR
    {
      TDiskManFileInfo *Inf=This->GetItemInf(This->MenuTarget);
      if(Inf)
        This->PerformInsertAction(wpar_lo-1010,Inf->Name,Inf->Path,"");
      break;
    }
    case 1020:
      SendMessage(This->DiskView,LVM_EDITLABEL,This->MenuTarget,0);
      break;
    case 1030:
      PostMessage(Win,WM_USER,1234,2);
      break;
    // drive icon context menu
    case 1056:
    case 1057:
    {
      EasyStr path=FileSelect(NULL,T("Select Disk Image"),DiskMan.DisksFol,
        FSTypes(2,NULL),1,true,"");
      EasyStr name=GetFileNameFromPath(path);
      DiskMan.InsertDisk((wpar_lo-1056),name,path,0,0,"",true); // can be ""
      break;
    }
#if defined(SSE_DRIVE_SOUND)
    case 1054:
    case 1055:
    {
      int drive=(wpar_lo-1054);
      EasyStr NewFol=ChooseFolder(HWND(FullScreen?StemWin:Win),
        T("Pick a Folder"),DriveSoundDir[drive]);
      if(NewFol.NotEmpty()&&NotSameStr_I(NewFol,DriveSoundDir[drive]))
      {
        NO_SLASH(NewFol);
        DriveSoundDir[drive]=NewFol;
      }
      break;
    }
    case 1044:
    case 1045:
      OPTION_DRIVE_SOUND_SEEK_SAMPLE=!OPTION_DRIVE_SOUND_SEEK_SAMPLE;
      break;
#endif
    case 1046:
    case 1047: // hack, some programs leave the motor on
      FloppyDrive[(wpar_lo-1046)].motor=false;
      Fdc.str&=~FDC_STR_MO;
      agenda_delete(agenda_fdc_motor_flag_off);
      break;
#if defined(SSE_DRIVE_FREEBOOT)
/*   bit0 A bit 1 B set = single - we toggle bits
Toggling a bit

The XOR operator (^) can be used to toggle a bit.
number ^= 1 << x;

That will toggle bit x.
*/
    case 1048:
    case 1049:
      FloppyDrive[wpar_lo-1048].single_sided
        =!FloppyDrive[wpar_lo-1048].single_sided;
      InvalidateRect(GetDlgItem(This->Handle,(wpar_lo-98)),NULL,FALSE);
      break;
    case 1052:
    case 1053:
      FloppyDrive[wpar_lo-1052].freeboot
        =!FloppyDrive[wpar_lo-1052].freeboot;
      InvalidateRect(GetDlgItem(This->Handle,(wpar_lo-98)),NULL,FALSE);
      Psg.CheckFreeboot();
      break;
#endif
      //SS TODO: write-protect that doesn't change file properties
    case 1050:  // Disk in Drive 1
    case 1051:  // Disk in Drive 2
      This->DragLV=GetDlgItem(Win,int(wpar_lo==1050?100:101));
    case 1040:  // Toggle Read-Only
    {
      bool FromDV=0;
      if(wpar_lo==1040) 
      {
        This->DragLV=This->DiskView;
        FromDV=true;
      }
      LV_ITEM lvi;
      lvi.iItem=int(FromDV?This->MenuTarget:0);
      lvi.iSubItem=0;
      lvi.mask=LVIF_PARAM;
      SendMessage(This->DragLV,LVM_GETITEM,0,(LPARAM)&lvi);
      TDiskManFileInfo *Inf=(TDiskManFileInfo*)lvi.lParam;
      EasyStr DiskPath=Inf->Path;
      bool InDrive[2]={0,0};
      EasyStr OldName[2],DiskInZip[2];
      for(int d=0;d<2;d++) 
      {
        if(IsSameStr_I(FloppyDrive[d].GetDisk(),DiskPath)) 
        {
          InDrive[d]=true;
          OldName[d]=FloppyDisk[d].DiskName;
          DiskInZip[d]=FloppyDisk[d].DiskInZip;
          FloppyDrive[d].RemoveDisk();
        }
      }
      DWORD Attrib=GetFileAttributes(DiskPath);
      if(Inf->ReadOnly)
        SetFileAttributes(DiskPath,Attrib & ~FILE_ATTRIBUTE_READONLY);
      else
        SetFileAttributes(DiskPath,Attrib|FILE_ATTRIBUTE_READONLY);
      Inf->ReadOnly=bool(GetFileAttributes(DiskPath) & FILE_ATTRIBUTE_READONLY); // Just in case of failure
      int c=(int)SendMessage(This->DiskView,LVM_GETITEMCOUNT,0,0);
      if(Inf->LinkPath.NotEmpty()||FromDV==0) 
      {
        for(lvi.iItem=1;lvi.iItem<c;lvi.iItem++) 
        {
          lvi.mask=LVIF_PARAM;
          SendMessage(This->DiskView,LVM_GETITEM,0,(LPARAM)&lvi);
          if(((TDiskManFileInfo*)lvi.lParam)->LinkPath.IsEmpty()) {
            if(IsSameStr_I(((TDiskManFileInfo*)lvi.lParam)->Path,DiskPath)) 
            {
              ((TDiskManFileInfo*)lvi.lParam)->ReadOnly=Inf->ReadOnly;
              break;
            }
          }
        }
      }
      if(lvi.iItem<c) 
      {
        lvi.mask=LVIF_IMAGE;
        lvi.iImage=1+Inf->ReadOnly*4;
        SendMessage(This->DiskView,LVM_SETITEM,0,(LPARAM)&lvi);
      }
      if(InDrive[0]) 
        This->InsertDisk(0,OldName[0],DiskPath,0,0,DiskInZip[0]);
      if(InDrive[1]) 
        This->InsertDisk(1,OldName[1],DiskPath,0,0,DiskInZip[1]);
      SetFocus(This->DragLV);
      break;
    }
#if defined(SSE_DISK_STW)
/*  Create a new STW disk image, and copy the data from a DIM, MSA or ST
    image into it, using some facilities of our fdc emu.
*/
    case 1041:
    {
#if !defined(SSE_DISK_AUTOSTW)
      TWD1772MFM wd1772mfm;
      TWD1772Crc wd1772crc;
#endif
      TDiskManFileInfo *Inf=This->GetItemInf(This->MenuTarget);
      if(Inf)
      {
        char dest[MAX_PATH];
        strcpy(dest,Inf->Path.Text);
        char *dot=strrchr(dest,'.');
        if(dot)
          strcpy(dot+1,"STW");
        BYTE save1=OPTION_AUTOSTW;
        bool save2=FloppyDisk[0].ReadOnly;
        FloppyDisk[0].ReadOnly=false;
        OPTION_AUTOSTW=0; // don't want to autoconvert it!
        int Err=FloppyDrive[0].SetDisk(Inf->Path,"");
        if(!Err)
        {
          if(FloppyDrive[0].ImageType.Manager==MNGR_STEEM //DIM, MSA or ST
#if !defined(SSE_DISK_HD)
          && FloppyDisk[0].SectorsPerTrack<=11 //not "super" disks
#endif
          )
          {
            TRACE_LOG("Creating %s\n",dest);
#if defined(SSE_DISK_AUTOSTW)
            STtoSTW(0,dest);
            FloppyDisk[0].WrittenTo=true;
            ImageSTW[0].Close();
#else
            ImageSTW[1].Create(dest.Text);
            ImageSTW[1].Open(dest.Text);
            FloppyDisk[1].ReadOnly=false; // make sure we save the STW
            for(int track=0;track<FloppyDisk[0].TracksPerSide;track++)
            {
              for(int side=0;side<FloppyDisk[0].Sides;side++)
              {
                if(!ImageSTW[1].LoadTrack(side,track))
                  continue;
                int p=0;
                for(int i=0;i<FloppyDisk[0].PostIndexGap();i++)
                  WD1772_WRITE(0x4E)
                int sector; //used in trace
                for(int sector2=1;sector2<=FloppyDisk[0].SectorsPerTrack
                  ;sector2++)
                {
                  //  We must use interleave 6 for 11 sectors, eg Pang -EMP
                  sector=(FloppyDisk[0].SectorsPerTrack==11
                    ?((((sector2-1)*DISK_11SEC_INTERLEAVE)%11)+1)
                    :sector2);
                  if(FloppyDisk[0].SeekSector(side,track,sector,false))
                    break; // not in source, write nothing more
                  for(int i=0;i<(FloppyDisk[0].SectorsPerTrack==11?3:12);i++)
                    WD1772_WRITE(0)
                  for(int i=0;i<3;i++)
                    WD1772_WRITE_A1
                  // write sector ID
                  WD1772_WRITE(0xFE)
                  WD1772_WRITE(track)
                  WD1772_WRITE(side)
                  WD1772_WRITE(sector)
                  WD1772_WRITE(2) //512 bytes
                  WD1772_WRITE_CRC(wd1772crc.crc>>8)
                  WD1772_WRITE_CRC(wd1772crc.crc&0xFF)
                  for(int i=0;i<22;i++)
                    WD1772_WRITE(0x4E)
                  for(int i=0;i<12;i++) // 11 sectors too?
                    WD1772_WRITE(0)
                  for(int i=0;i<3;i++)
                    WD1772_WRITE_A1
                  // write sector data
                  WD1772_WRITE(0xFB)
                  BYTE b=0;
                  for(int i=0;i<512;i++)
                  {
                    fread(&b,1,1,FloppyDisk[0].f);
                    WD1772_WRITE(b)
                  }
                  WD1772_WRITE_CRC(wd1772crc.crc>>8)
                  WD1772_WRITE_CRC(wd1772crc.crc&0xFF)
                  WD1772_WRITE(0xFF) // so 1 byte gap fewer ?
                  for(int i=0;i<(FloppyDisk[0].SectorsPerTrack==11?
                    1-1:40-1);i++)
                    WD1772_WRITE(0x4E)
                }//sector
                // pre-index gap: the rest
                //ASSERT(FloppyDisk[1].TrackBytes>=6256);
                while(p<FloppyDisk[1].TrackBytes)
                  WD1772_WRITE(0x4E)
                TRACE_LOG("STW track %d/%d written %d sectors %d bytes\n",side,track,sector-1,p);
              }//side
            }//track
            ImageSTW[1].Close();
#endif
          }
          FloppyDrive[0].RemoveDisk(true);
          This->RefreshDiskView(dest,true);
        }//!err
        OPTION_AUTOSTW=save1;
        FloppyDisk[0].ReadOnly=save2;
      }//inf
      break;
    }
#endif
    case 1060:
    case 1061:
    {
      TDiskManFileInfo *Inf=This->GetItemInf(This->MenuTarget);
      if(Inf) ShellExecute(NULL,LPSTR((wpar_lo==1061)?"Find":
        LPSTR(This->ExplorerFolders?"explore":NULL)),Inf->Path,"","",
        SW_SHOWNORMAL);
      break;
    }
    case 1070:
    {
      TDiskManFileInfo *Inf=This->GetItemInf(This->MenuTarget);
      WIN32_FIND_DATA wfd;
      EasyStr NewPath=GetLinkDest(Inf->LinkPath,&wfd,HWND(FullScreen?StemWin:
        This->Handle));
      if(NewPath.NotEmpty()) 
      {
        if(GetFileAttributes(NewPath)!=0xffffffff) 
        {
          Inf->Path=NewPath;
          Inf->BrokenLink=0;
          LV_ITEM lvi;
          lvi.iItem=This->MenuTarget;
          lvi.iSubItem=0;
          lvi.mask=LVIF_IMAGE;
          lvi.iImage=int(Inf->Folder?3:4);
          SendMessage(This->DiskView,LVM_SETITEM,0,(LPARAM)&lvi);
        }
      }
      break;
    }
    case 1080:  //Extract
    {
      TDiskManFileInfo *Inf=This->GetItemInf(This->MenuTarget);
      This->ExtractDisks(Inf->Path);
      break;
    }
    case 1082:
    {//Put the name of the game in the clipboard when user clicks on it.
      GET_THIS;
      TDiskManFileInfo *Inf=This->GetItemInf(0,(HWND)GetDlgItem(Win,
        100+This->MenuTarget));
      SetClipboardText(Inf->Name.Text); // in acc.cpp
      break;
    }
    case 1090:  // Go to disk
    case 1091:  // Go to disk in drive
    {
      GET_THIS;
      TDiskManFileInfo *Inf=This->GetItemInf(int((wpar_lo==1090)?
        This->MenuTarget:0),HWND((wpar_lo==1090)?
        This->DiskView:GetDlgItem(Win,100+This->MenuTarget)));
      This->GoToDisk(Inf->Path,0);
      break;
    }
    case 1092: // Open Disk's Folder in Explorer
    {
      TDiskManFileInfo *Inf=This->GetItemInf(This->MenuTarget);
      if(Inf) 
      {
        Str Fol=Inf->Path;
        RemoveFileNameFromPath(Fol,REMOVE_SLASH);
        ShellExecute(NULL,LPSTR(This->ExplorerFolders?"explore":NULL),
          Fol,"","",SW_SHOWNORMAL);
      }
      break;
    }
    //SS can make case 1093 for copy-to-clipboard
    case 1099:
    {
      TDiskManFileInfo *Inf=This->GetItemInf(This->MenuTarget);
      if(Inf)
      {
        This->PropInf=*Inf;
        This->ShowPropDiag();
      }
      break;
    }
    case 1100: // Swap disks
    {
      This->SwapDisks(This->MenuTarget);
      break;
    }
    case 1101:
      if(SendMessage(GetDlgItem(Win,100+This->MenuTarget),LVM_GETITEMCOUNT,0,0))
        This->EjectDisk(This->MenuTarget);
      break;
    case 1102:
      if(SendMessage(GetDlgItem(Win,100+This->MenuTarget),LVM_GETITEMCOUNT,0,0))
        This->EjectDisk(This->MenuTarget,true); // lose changes
      break;
    case 2002:
      This->HideBroken=!This->HideBroken;
      PostMessage(Win,WM_COMMAND,IDCANCEL,0);
      break;
    case 2029:
      This->ShowHiddenFiles=!This->ShowHiddenFiles;
      PostMessage(Win,WM_COMMAND,IDCANCEL,0);
      break;
    case 2017:
      This->HideExtension=!This->HideExtension;
      PostMessage(Win,WM_COMMAND,IDCANCEL,0);
      break;
    case 2003:
      ShellExecute(NULL,LPSTR(This->ExplorerFolders?"explore":NULL),
        This->DisksFol,"","",SW_SHOWNORMAL);
      break;
    case 2004:
      This->ExplorerFolders=!This->ExplorerFolders;
      break;
    case 2005:
      if(This->SmallIcons) 
      {
        This->SmallIcons=0;
        SendMessage(This->DiskView,WM_SETREDRAW,0,0);
        This->SetDiskViewMode(LVS_ICON);
        This->RefreshDiskView();
      }
      break;
    case 2006:
      if(This->SmallIcons==0) 
      {
        This->SmallIcons=true;
        This->RefreshDiskView();
      }
      break;
    case 2007:case 2008:case 2009:
      This->DoubleClickAction=wpar_lo-2007;
      break;
#ifndef SSE_NO_WINSTON_IMPORT
    case 2001:  // Import
      This->ShowImportDiag();
      break;
#endif
    case 2010:
      ShellExecute(NULL,"Find",This->DisksFol,"","",SW_SHOWNORMAL);
      break;
    case 2011:
      This->EjectDisksWhenQuit=!This->EjectDisksWhenQuit;
      break;
    case 2012:
      SendDlgItemMessage(Win,99,WM_LBUTTONDOWN,0,0);
      break;
    case 2013:
      floppy_instant_sector_access=!floppy_instant_sector_access;
      InvalidateRect(GetDlgItem(Win,98),NULL,0);
      InvalidateRect(GetDlgItem(Win,99),NULL,0);
      CheckResetDisplay();
      FloppyDrive[0].UpdateAdat();
      FloppyDrive[1].UpdateAdat();
      break;
    case 2014:
      FloppyArchiveIsReadWrite=!FloppyArchiveIsReadWrite;
      This->LoadIcons();
      if(FloppyDisk[0].IsZip()) 
        FloppyDrive[0].ReinsertDisk();
      if(FloppyDisk[1].IsZip()) 
        FloppyDrive[1].ReinsertDisk();
      break;
    case 2015:
      This->CloseAfterIRR=!This->CloseAfterIRR;
      break;
    case 2016:
      This->AutoInsert2=!This->AutoInsert2;
      break;
    case 2018:
      floppy_access_ff=!floppy_access_ff;
      break;
    case 2019:
      OPTION_COUNT_DMA_CYCLES=!OPTION_COUNT_DMA_CYCLES;
      break;
    case 2020:case 2021:case 2022:
      if(This->IconSpacing!=(wpar_lo-2020)) 
      {
        This->IconSpacing=wpar_lo-2020;
        This->SetDiskViewMode(This->SmallIcons?LVS_SMALLVIEW:LVS_ICON);
        This->RefreshDiskView();
      }
      break;
#if USE_PASTI
    case 2023:case 2024:
      if(hPasti==NULL) 
        break;
      if(wpar_lo==2024) // Pasti configuration
        pasti->DlgConfig(HWND(FullScreen?StemWin:This->Handle),0,NULL);
      if(wpar_lo==2023) 
      { //SS option use pasti
        // no reset, just reinsert
        pasti_active=!pasti_active;
        //TRACE_LOG("pasti_active %d\n",pasti_active);
        for(int i=0;i<2;i++)
        {
          if(FloppyDrive[i].NotEmpty())
          {
            EasyStr name=FloppyDisk[i].DiskName;
            EasyStr path=FloppyDisk[i].GetImageFile();
            This->EjectDisk(i);
            This->InsertDisk(i,name,path,0,0,"",true);
          }
          else FloppyDrive[i].ImageType.Manager=(BYTE)((pasti_active)
            ? MNGR_PASTI : (OPTION_AUTOSTW ? MNGR_WD1772 : MNGR_STEEM));
        }
        This->RefreshDiskView();
      }//if(wpar_lo==2023
      break;
#endif//pasti
#if defined(SSE_DISK_AUTOSTW)
    case 2025:
      for(int i=0;i<2;i++) // reinsert disks, like for Pasti
      {
        if(FloppyDrive[i].NotEmpty())
        {
          EasyStr name=FloppyDisk[i].DiskName;
          EasyStr path=FloppyDisk[i].GetImageFile();
          This->EjectDisk(i);
          OPTION_AUTOSTW=!OPTION_AUTOSTW;
          This->InsertDisk(i,name,path,0,0,"",true);
          OPTION_AUTOSTW=!OPTION_AUTOSTW;
        }
        else FloppyDrive[i].ImageType.Manager=(BYTE)((pasti_active)
          ? MNGR_PASTI : (OPTION_AUTOSTW ? MNGR_WD1772 : MNGR_STEEM));
      }
      OPTION_AUTOSTW=!OPTION_AUTOSTW;
      TRACE_LOG("Option AutoSTW %d\n",OPTION_AUTOSTW);
      break;
#endif
    case 2026:
      This->ShowDatabaseDiag();
      break;
#if defined(SSE_DISK_GHOST)
    case 2027:
      OPTION_GHOST_DISK=!OPTION_GHOST_DISK;
      TRACE_LOG("Option Ghost disk %d\n",OPTION_GHOST_DISK);
      break;
#endif
#if defined(SSE_TOS_PRG_AUTORUN)
    case 2028:
      OPTION_PRG_SUPPORT=!OPTION_PRG_SUPPORT;
      TRACE_LOG("Option PRG support %d\n",OPTION_GHOST_DISK);
      This->RefreshDiskView();
      break;
#endif
    }
    if(wpar_lo>=4000&&wpar_lo<5000)
      This->MenuTarget=wpar_lo;
    else if(wpar_lo>=5000&&wpar_lo<5080) 
    {
      if(wpar_lo>=5060) 
      {
        int nGoForward=(wpar_lo-5060)+1;
        for(int i=0;i<nGoForward;i++)
          for(int n=9;n>0;n--)
            This->HistBack[n]=This->HistBack[n-1];
        int BackIdx=0,ForIdx=nGoForward-2;
        for(int n=0;n<nGoForward-1;n++) 
          This->HistBack[BackIdx++]=This->HistForward[ForIdx--];
        This->HistBack[BackIdx]=This->DisksFol;
        This->SetDir(This->HistForward[nGoForward-1],0);
        for(int i=0;i<nGoForward;i++) 
        {
          for(int n=0;n<9;n++)
            This->HistForward[n]=This->HistForward[n+1];
          This->HistForward[9]="";
        }
        EnableWindow(GetDlgItem(Win,82),true);
        if(This->HistForward[0].IsEmpty()) 
        {
          if(GetFocus()==GetDlgItem(Win,83)) 
            SetFocus(GetDlgItem(Win,82));
          EnableWindow(GetDlgItem(Win,83),0);
        }
      }
      else if(wpar_lo>=5040) 
      {
        int nGoBack=(wpar_lo-5040)+1;
        for(int i=0;i<nGoBack;i++)
          for(int n=9;n>0;n--)
            This->HistForward[n]=This->HistForward[n-1];
        int ForIdx=0,BackIdx=nGoBack-2;
        for(int n=0;n<nGoBack-1;n++) 
          This->HistForward[ForIdx++]=This->HistBack[BackIdx--];
        This->HistForward[ForIdx]=This->DisksFol;
        This->SetDir(This->HistBack[nGoBack-1],0);
        for(int i=0;i<nGoBack;i++) 
        {
          for(int n=0;n<9;n++)
            This->HistBack[n]=This->HistBack[n+1];
          This->HistBack[9]="";
        }
        EnableWindow(GetDlgItem(Win,83),true);
        if(This->HistBack[0].IsEmpty()) {
          if(GetFocus()==GetDlgItem(Win,82)) 
            SetFocus(GetDlgItem(Win,83));
          EnableWindow(GetDlgItem(Win,82),0);
        }
      }
      else 
      { // Go to quick folder
        if(wpar_lo==5000)
          This->SetDir(This->HomeFol,true);
        else 
        {
          if(This->QuickFol[wpar_lo-5001].NotEmpty())
            This->SetDir(This->QuickFol[wpar_lo-5001],true);
        }
      }
    }
    if(wpar_lo>=8000&&wpar_lo<8100) 
    {  // Change/erase quick folder
      if(wpar_lo==8000) 
      {
        if(Alert(This->DisksFol+"\n\n"+T("Are you sure you want to make this folder your new home folder?"),
          T("Change Home Folder?"),MB_YESNO|MB_ICONQUESTION)==IDYES) {
          This->HomeFol=This->DisksFol;
        }
      }
      else 
      {
        int n=(wpar_lo-8005)/5;
        int Action=(wpar_lo-8005)%5;
        switch(Action) {
        case 0:
          This->QuickFol[n]=This->DisksFol;
          break;
        case 1:
        {
          EnableAllWindows(0,Win);
          // SS: this allows player to select a network folder as well
          // (home network when it was supported by Windows)
          EasyStr NewFol=ChooseFolder(HWND(FullScreen?StemWin:Win),
            T("Pick a Folder"),This->DisksFol);
          if(NewFol.NotEmpty())
            This->QuickFol[n]=NewFol;
          SetForegroundWindow(Win);
          EnableAllWindows(true,Win);
          break;
        }
        case 2:
          This->QuickFol[n]="";
          break;
        }
      }
    }
    else if(wpar_lo>=6000&&wpar_lo<7000) 
    {
      int SelItem=This->GetSelectedItem();
      if(SelItem>-1) 
      {
        TDiskManFileInfo *Inf=This->GetItemInf(SelItem);
        int Action=(wpar_lo-6000)/20;
        EasyStr SrcFol,DestFol,To;
        char From[MAX_PATH+2];
        ZeroMemory(From,sizeof(From));
        if((wpar_lo%20)==0)
          DestFol=This->HomeFol;
        else
          DestFol=This->QuickFol[(wpar_lo%20)-1];
        To=DestFol+SLASH+GetFileNameFromPath((Action<2)?Inf->Path:Inf->LinkPath);
        bool Moving=0;
        switch(Action) {
        case 0: // Move Path
        case 3: // Move LinkPath
          Moving=true;
        case 1: // Copy Path
        case 4: // Copy LinkPath
          strcpy(From,(Action<2)?Inf->Path:Inf->LinkPath);
          SrcFol=From;
          RemoveFileNameFromPath(SrcFol,REMOVE_SLASH);
          if(NotSameStr_I(SrcFol,DestFol)||Moving==0) 
          {
            This->MoveOrCopyFile(Moving,From,To,(Action==0)?Inf->Path:Str(),
              IsSameStr_I(SrcFol,DestFol));
            bool Refresh=0;
            if(Action==0&&Inf->LinkPath.NotEmpty()) 
            {
              CreateLink(Inf->LinkPath,To);
              Inf->Path=To;
            }
            if(IsSameStr_I(SrcFol,This->DisksFol)&&Moving) Refresh=true;
            if(IsSameStr_I(DestFol,This->DisksFol)) Refresh=true;
            if(Refresh) This->RefreshDiskView("",0,"",SelItem);
          }
          break;
        case 2:  // Link to Path
        {
          EasyStr LinkName=DestFol+SLASH+Inf->Name+".lnk";
          int n=2;
          while(Exists(LinkName))
            LinkName=DestFol+SLASH+Inf->Name+" ("+(n++)+").lnk";
          CreateLink(LinkName,Inf->Path);
          if(IsSameStr_I(DestFol,This->DisksFol)) 
            This->RefreshDiskView("",true,LinkName);
          break;
        }
        }
      }
    }
    else if((wpar_lo>=7000&&wpar_lo<7020)||wpar_lo==1015) 
    {
      // Get contents [and create shortcuts in DestFol]
      Str DestFol;
      if(wpar_lo!=1015) 
      {
        if((wpar_lo%20)==0)
          DestFol=This->HomeFol;
        else
          DestFol=This->QuickFol[(wpar_lo%20)-1];
      }
      TDiskManFileInfo *Inf=This->GetItemInf(This->MenuTarget);
      This->GetContentsSL(Inf->Path);
      if(This->contents_sl.NumStrings) 
      {
        if(DestFol.NotEmpty()) 
        {
          // 0 = path, 1 = disk name, 2+ = contents
          Str ShortName=This->GetContentsGetAppendName(This->contents_sl[1].String);
          Str SelLink;
          int start_i=2;
          if(This->contents_sl.NumStrings==1) 
            start_i=1;
          for(int i=start_i;i<This->contents_sl.NumStrings;i++) 
          {
            SelLink=GetUniquePath(DestFol,Str(This->contents_sl[i].String)
              +" ("+ShortName+").lnk");
            CreateLink(SelLink,Inf->Path);
          }
          if(SelLink.NotEmpty()&&IsSameStr_I(DestFol,This->DisksFol))
            This->RefreshDiskView("",0,SelLink);
        }
        else 
        {
          This->ContentsLinksPath=This->DisksFol;
          This->ShowContentDiag();
        }
      }
    }
    else if(wpar_lo>=9000&&wpar_lo<10000) 
    {
      TDiskManFileInfo *Inf=This->GetItemInf(This->MenuTarget);
      if(Inf) 
      {
        int Action=(wpar_lo-9000)/200;
        EasyStr DiskInZip=This->MenuESL[(wpar_lo-9000)%200].String;
        This->PerformInsertAction(Action,Inf->Name,Inf->Path,DiskInZip);
      }
    }
    else if(wpar_lo>=2030&&wpar_lo<2100) 
    { // MSA Converter
      Str MSA=This->GetMSAConverterPath();
      if(This->MSAConvPath.NotEmpty()) 
      {
        Str Comline;
        TDiskManFileInfo *Inf=This->GetItemInf(This->MenuTarget);
        Str NewSel;
        switch(wpar_lo) {
        case 2031:  Comline=Str("\"")+Inf->Path+"\"";  break;
        case 2034: // Zip containing files to disk image
          NewSel=Inf->Path;
          *strchr(NewSel,'.')=0;
          NewSel+=".st";
          Comline=Str("\"convert\" \"")+Inf->Path+"\" \"st\" \"exit\"";
          break;
        default:
          if(wpar_lo>2040&&wpar_lo<=2040+26) 
          {
            NewSel=GetUniquePath(mount_path[wpar_lo-2040],Inf->Name);
            CreateDirectory(NewSel,NULL);
            Comline=Str("\"diskimg_to_hdisk\" \"")+Inf->Path+"\" \""
              +NewSel+"\" \"exit\"";
            NewSel=""; // Nothing to select
          }
          else
            Comline=Str("\"user_path\" \"")+This->DisksFol+"\"";
        }
        SHELLEXECUTEINFO sei;
        sei.cbSize=sizeof(SHELLEXECUTEINFO);
        sei.fMask=SEE_MASK_FLAG_NO_UI|SEE_MASK_NOCLOSEPROCESS;
        sei.hwnd=NULL;
        sei.lpVerb=NULL;
        sei.lpFile=MSA;
        sei.lpParameters=Comline;
        sei.lpDirectory=NULL;
        sei.nShow=SW_SHOWNORMAL;
        if(ShellExecuteEx(&sei)) 
        {
          if(NewSel.NotEmpty()) 
          {
            This->MSAConvProcess=sei.hProcess;
            This->MSAConvSel=NewSel;
            SetTimer(Win,MSACONV_TIMER_ID,1000,NULL);
          }
        }
      }
    }
    break;
  }
  case WM_CONTEXTMENU:
    GET_THIS;
    if(HWND(wPar)==This->DiskView) 
    {
      HMENU Pop=CreatePopupMenu();
      int c=(int)SendMessage(This->DiskView,LVM_GETITEMCOUNT,0,0);
      LV_ITEM lvi;
      lvi.mask=LVIF_PARAM|LVIF_STATE;
      lvi.iSubItem=0;
      lvi.stateMask=LVIS_SELECTED;
      for(lvi.iItem=0;lvi.iItem<c;lvi.iItem++) 
      {
        SendMessage(This->DiskView,LVM_GETITEM,0,(LPARAM)&lvi);
        if(lvi.state==LVIS_SELECTED) 
        {
          This->AddFileOrFolderContextMenu(Pop,(TDiskManFileInfo*)lvi.lParam);
          This->MenuTarget=lvi.iItem;
          break;
        }
      }
      HMENU FolOptionsPop=Pop;
      if(GetScreenHeight()<600) 
      {
        FolOptionsPop=CreatePopupMenu();
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_POPUP|MF_STRING,
          (UINT_PTR)FolOptionsPop,T("More Options"));
      }
      InsertMenu(FolOptionsPop,0xffffffff,MF_BYPOSITION|MF_STRING,1005,
        Str(T("Refresh"))+" \10ESC");
      InsertMenu(FolOptionsPop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
      InsertMenu(FolOptionsPop,0xffffffff,MF_BYPOSITION|MF_STRING,2003,
        T("Open Current Folder In &Explorer")+"\10F4");
      InsertMenu(FolOptionsPop,0xffffffff,MF_BYPOSITION|MF_STRING,2010,
        T("Find In Current Folder"));
      InsertMenu(FolOptionsPop,0xffffffff,MF_BYPOSITION|MF_STRING,2030,
        T("Run MSA Converter")+"\10F6");
      InsertMenu(FolOptionsPop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
      HMENU NewPop=CreatePopupMenu();
      InsertMenu(NewPop,0xffffffff,MF_BYPOSITION|MF_STRING,1000,
        T("&Folder"));
      InsertMenu(NewPop,0xffffffff,MF_BYPOSITION|MF_STRING,1001,
        //T("Standard &Disk Image"));
        T("&ST Disk Image"));
#if defined(SSE_DISK_HD)
      InsertMenu(NewPop,0xffffffff,MF_BYPOSITION|MF_STRING,1006,
        T("HD ST Disk Image"));
#endif
      InsertMenu(NewPop,0xffffffff,MF_BYPOSITION|MF_STRING,1002,
        //T("Custom Disk &Image"));
        T("&Custom Disk Image"));
#if defined(SSE_DISK_STW)
      InsertMenu(NewPop,0xffffffff,MF_BYPOSITION|MF_STRING,1003,
        T("ST&W Disk Image"));
#endif
#if defined(SSE_DISK_HD)
      InsertMenu(NewPop,0xffffffff,MF_BYPOSITION|MF_STRING,1007,
        T("HD STW Disk Image"));
#endif
#if defined(SSE_DISK_HFE)
      InsertMenu(NewPop,0xffffffff,MF_BYPOSITION|MF_STRING,1004,
        T("&HFE Disk Image"));
#endif
      InsertMenu(FolOptionsPop,0xffffffff,MF_BYPOSITION|MF_STRING|MF_POPUP,
        (UINT_PTR)NewPop,T("&New..."));
      POINT pt;
      GetCursorPos(&pt);
      TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
        pt.x,pt.y,0,Win,NULL);
      DestroyMenu(Pop);
    }
    else if(HWND(wPar)==GetDlgItem(Win,100)||HWND(wPar)==GetDlgItem(Win,101)) 
    {
      // right click in box on the right of disk icons
      if(SendMessage(HWND(wPar),LVM_GETITEMCOUNT,0,0)>0) 
      {
        HMENU Pop=CreatePopupMenu();
        LV_ITEM lvi;
        lvi.iItem=0;
        lvi.iSubItem=0;
        lvi.mask=LVIF_PARAM;
        SendMessage(HWND(wPar),LVM_GETITEM,0,(LPARAM)&lvi);
        TDiskManFileInfo *Inf=(TDiskManFileInfo*)lvi.lParam;
        if(HWND(wPar)==GetDlgItem(Win,100))
          This->MenuTarget=0;
        else
          This->MenuTarget=1;
        if(Inf->Zip==0) 
          InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|int(Inf->ReadOnly
          ?MF_CHECKED:0),1050+This->MenuTarget,T("Read &Only"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1100,T("&Swap"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR|MF_STRING,999,"-");
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1101,
          //T("Eject Disk")+" \10DEL");
          T("Eject Disk \10DEL"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1102,
          T("&Eject Disk (don't save changes)"));
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_SEPARATOR,999,NULL);
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1091,T("&Go To Disk"));
/*  This is so the player can read the full name of the disk without
    checking at the place of storage.
    If he clicks on it, it is copied in the clipboard, whatever use this
    then may have.
*/
        InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1082,Inf->Name.Text);
        POINT pt;
        GetCursorPos(&pt);
        TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
          pt.x,pt.y,0,Win,NULL);
        DestroyMenu(Pop);
      }
    }
    break;
  case WM_NOTIFY:
    if(wPar==102) 
    {  //DiskView Only
      switch(((NMHDR*)lPar)->code) {
      case LVN_GETDISPINFO:
      {
        LV_DISPINFO *DispInf=(LV_DISPINFO*)lPar;
        DispInf->item.mask=LVIF_TEXT;
        DispInf->item.pszText=((TDiskManFileInfo*)(DispInf->item.lParam))->
          Name.Text;
        break;
      }
      case LVN_BEGINLABELEDIT:
      {
        LV_DISPINFO *DispInf=(LV_DISPINFO*)lPar;
        if(DispInf->item.iItem==0) 
          return true;
        return 0;
      }
      case LVN_ENDLABELEDIT:
      {
        LV_DISPINFO *DispInf=(LV_DISPINFO*)lPar;
        if(DispInf->item.pszText==NULL) 
          return 0;
        GET_THIS;
        TDiskManFileInfo *Inf=(TDiskManFileInfo*)DispInf->item.lParam;
        bool Link=Inf->LinkPath.NotEmpty();
        bool InDrive[2]={0,0};
        EasyStr NewDiskName[2],OldDiskName[2],OldDiskPath[2],DiskInZip[2];
        EasyStr NewName=DispInf->item.pszText,OldName=Inf->Name,Extension;
        if(Link)
          Extension=".lnk";
        else 
        {
          if(Inf->Folder==0) 
          {
            Extension=strrchr(Inf->Path,'.');
            for(int disk=0;disk<2;disk++) 
            {
              OldDiskName[disk]=FloppyDisk[disk].DiskName;
              if(OldDiskName[disk]==OldName)
                NewDiskName[disk]=NewName;
              else
                NewDiskName[disk]=OldDiskName[disk];
              OldDiskPath[disk]=FloppyDrive[disk].GetDisk();
              DiskInZip[disk]=FloppyDisk[disk].DiskInZip;
              if(IsSameStr_I(OldDiskPath[disk],This->DisksFol
                +SLASH+OldName+Extension)) 
              {
                InDrive[disk]=true;
                FloppyDrive[disk].RemoveDisk();
              }
            }
          }
          else 
          {
            // Check for inserted disks being in renamed folder
          }
        }
        BOOL success;
        if(This->HideExtension)
          success=MoveFile(This->DisksFol+SLASH+OldName+Extension,This->DisksFol
            +SLASH+NewName+Extension);
        else
          success=MoveFile(This->DisksFol+SLASH+OldName,This->DisksFol
            +SLASH+NewName);
        if(success)
        {
          if(Link)
            Inf->LinkPath=This->DisksFol+SLASH+NewName+Extension;
          else 
          {
            Str NewPath=This->DisksFol+SLASH+NewName;
            if(This->HideExtension)
              NewPath+=Extension;
            if(Inf->Folder==0) 
              This->UpdateBPBFiles(Inf->Path,NewPath,true);
            Inf->Path=NewPath;
          }
          Inf->Name=NewName;
          for(int disk=0;disk<2;disk++) 
          {
            if(InDrive[disk]) 
            {
              This->InsertHistoryDelete(disk,OldDiskName[disk],This->DisksFol
                +SLASH+OldName+Extension,DiskInZip[disk]);
              This->InsertHistoryAdd(disk,NewDiskName[disk],This->DisksFol
                +SLASH+NewName+Extension,DiskInZip[disk]);
              FloppyDrive[disk].SetDisk(This->DisksFol+SLASH+NewName
                +Extension,DiskInZip[disk]);
              FloppyDisk[disk].DiskName=NewDiskName[disk];
              HWND LV=GetDlgItem(Win,100+disk);
              Inf=This->GetItemInf(0,LV);
              Inf->Path=FloppyDrive[disk].GetDisk();
              if(Inf->Name==OldDiskName[disk]) 
              {
                Inf->Name=NewDiskName[disk];
                LV_ITEM lvi;
                lvi.mask=LVIF_TEXT;
                lvi.iItem=0;
                lvi.iSubItem=0;
                lvi.pszText=NewDiskName[disk].Text;
                SendMessage(LV,LVM_SETITEM,0,(LPARAM)&lvi);
                CentreLVItem(LV,0);
              }
            }
          }
          return true;
        }
        else 
        {
          Alert(T("Unable to rename file (read-only? inserted?)"),T("Error"),
            MB_ICONEXCLAMATION);
          for(int disk=0;disk<2;disk++) 
          {
            if(InDrive[disk]) 
            {
              FloppyDrive[disk].SetDisk(This->DisksFol+SLASH+OldName+Extension,
                DiskInZip[disk]);
              FloppyDisk[disk].DiskName=OldDiskName[disk];
            }
          }
        }
        return 0;
      }
      case LVN_KEYDOWN:
      {
        GET_THIS;
        if(This->Dragging>-1) 
          break;
        LV_KEYDOWN *KeyInf=(LV_KEYDOWN*)lPar;
        switch(KeyInf->wVKey) {
        case VK_RETURN:case VK_SPACE:
          PostMessage(Win,WM_USER,1234,0);
          break;
        case VK_BACK:
          PostMessage(Win,WM_USER,1234,1);
          break;
        case VK_DELETE:
          PostMessage(Win,WM_USER,1234,2);
          break;
        case VK_F2:
        {
          GET_THIS;
          int SelItem=This->GetSelectedItem();
          if(SelItem>-1) 
            SendMessage(This->DiskView,LVM_EDITLABEL,SelItem,0);
          break;
        }
        case VK_F3:
        {
          GET_THIS;
          int SelItem=This->GetSelectedItem();
          if(SelItem>-1) 
          {
            TDiskManFileInfo *Inf=This->GetItemInf(SelItem);
            if(Inf->Folder)
              ShellExecute(NULL,"Find",Inf->Path,"","",SW_SHOWNORMAL);
            else
              SelItem=-1;
          }
          if(SelItem==-1)
            ShellExecute(NULL,"Find",This->DisksFol,"","",SW_SHOWNORMAL);
          break;
        }
        case VK_F4:
          PostMessage(Win,WM_COMMAND,2003,0);
          break;
        case VK_F5:
          PostMessage(Win,WM_COMMAND,IDCANCEL,0);
          break;
        case VK_F6:
          PostMessage(Win,WM_COMMAND,2030,0);
          break;
        case VK_F9:
          PostMessage(Win,WM_COMMAND,2026,0);
          break;
        }
        break;
      }
      case NM_DBLCLK:
        GET_THIS;
        if(This->Dragging>-1) 
          break;
        PostMessage(Win,WM_USER,1234,0);
        break;
      }
    }
    if(wPar>=100&&wPar<=102) 
    { //ListView
      switch(((NMHDR*)lPar)->code) {
      case LVN_DELETEITEM:
      {
        LV_ITEM lvi;
        lvi.mask=LVIF_PARAM;
        lvi.iItem=((NM_LISTVIEW*)lPar)->iItem;
        lvi.iSubItem=0;
        SendMessage(GetDlgItem(Win,(int)wPar),LVM_GETITEM,0,(LPARAM)&lvi);
        delete ((TDiskManFileInfo*)lvi.lParam);
        if(wPar<102) 
        {
          GET_THIS;
          This->SetDriveViewEnable((int)wPar-100,0);
        }
        break;
      }
      case LVN_BEGINDRAG:case LVN_BEGINRDRAG:
      {
        GET_THIS;
        This->BeginDrag(((NM_LISTVIEW*)lPar)->iItem,GetDlgItem(Win,(int)wPar));
        break;
      }
      }
      if(wPar<102) 
      {
        switch(((NMHDR*)lPar)->code) {
        case LVN_KEYDOWN:
          LV_KEYDOWN *KeyInf=(LV_KEYDOWN*)lPar;
          if(KeyInf->wVKey==VK_DELETE) 
          {
            GET_THIS;
            This->EjectDisk((int)wPar-100);
          }
          break;
        }
      }
    }
    break;
  case WM_MOUSEMOVE:
    GET_THIS;
    if(This->Dragging>-1) 
      This->MoveDrag();
    break;
  case WM_TIMER:
    GET_THIS;
    if(wPar==DISKVIEWSCROLL_TIMER_ID) 
    {
      if(This->DragLV==This->DiskView&&(This->LastOverID!=80||This->AtHome)) 
      {
        POINT spt;
        GetCursorPos(&spt);
        RECT rc;
        GetWindowRect(This->DiskView,&rc);
        if(spt.x>=rc.left && spt.y<=rc.right) 
        {
          int y=0;
          if(spt.y<=rc.top+2&&spt.y>=rc.top-20)
            y=-5;
          else if(spt.y>=rc.bottom-2&&spt.y<=rc.bottom+10)
            y=5;
          if(y) 
          {
            if(This->DragEntered) 
            {
              ImageList_DragLeave(Win);
              This->DragEntered=0;
            }
            SendMessage(This->DiskView,LVM_SCROLL,0,y);
            UpdateWindow(This->DiskView);
          }
        }
      }
    }
    else if(wPar==MSACONV_TIMER_ID) 
    {
      bool Kill=true;
      DWORD Code;
      if(GetExitCodeProcess(This->MSAConvProcess,&Code)) 
      {
        if(Code==STILL_ACTIVE)
          Kill=0;
        else 
        {
          This->GoToDisk(This->MSAConvSel,true);
          This->MSAConvSel="";
          This->MSAConvProcess=NULL;
        }
      }
      if(Kill) 
        KillTimer(Win,MSACONV_TIMER_ID);
    }
    break;
  case WM_LBUTTONUP:case WM_RBUTTONUP:
    GET_THIS;
    if(This->Dragging>-1) 
      This->EndDrag(LOWORD(lPar),HIWORD(lPar),(Mess==WM_RBUTTONUP));
    break;
  case WM_CAPTURECHANGED:
    GET_THIS;
    if(This->EndingDrag==0) 
    {
      if(This->Dragging>-1) 
      {
        if(This->DragEntered) 
        {
          ImageList_DragLeave(Win);
          This->DragEntered=0;
        }
        ImageList_EndDrag();
        SendMessage(GetDlgItem(Win,80),BM_SETCHECK,0,0);
        This->Dragging=-1;
        InvalidateRect(This->DiskView,NULL,true);
      }
    }
    break;
  case WM_USER:
    GET_THIS;
    if(wPar==1234) 
    {
      LV_ITEM lvi;
      int c=(int)SendMessage(This->DiskView,LVM_GETITEMCOUNT,0,0);
      lvi.mask=LVIF_PARAM|LVIF_STATE;
      lvi.iSubItem=0;
      lvi.stateMask=LVIS_SELECTED;
      for(lvi.iItem=0;lvi.iItem<c;lvi.iItem++) 
      {
        SendMessage(This->DiskView,LVM_GETITEM,0,(LPARAM)&lvi);
        if(lvi.state==LVIS_SELECTED) break;
      }
      TDiskManFileInfo *Inf=(TDiskManFileInfo*)lvi.lParam;
      if(lPar==0&&lvi.iItem<c) 
      {
        if(Inf->Folder) 
        {
          if(Inf->UpFolder)
            lPar=1;
          else
            This->SetDir(Inf->Path,true);
        }
        else 
        {
          TDiskManFileInfo *Inf2=This->GetItemInf(This->GetSelectedItem());
          if(Inf2) 
          {
            switch(This->DoubleClickAction) {
            case 1: This->PerformInsertAction(0,Inf2->Name,Inf->Path,""); break;
            case 2: This->PerformInsertAction(2,Inf2->Name,Inf->Path,""); break;
            }
          }
        }
      }
      else if(!lPar)
        lPar++; // double click on no icon: go up in directory tree
      if(lPar==1) 
      {  // Go Up
        EasyStr Fol=This->DisksFol;
        char *LastSlash=strrchr(Fol,'\\');
        if(LastSlash==NULL) 
        {
          LastSlash=strrchr(Fol,'/');
          if(LastSlash==NULL)
            LastSlash=strrchr(Fol,':');
        }
        if(LastSlash!=NULL) 
        {
          *LastSlash=0;
          if(NotSameStr_I(Fol,This->DisksFol)) 
            This->SetDir(Fol,true,This->DisksFol);
        }
      }
      else if(lPar==2&&Inf->UpFolder==0) 
      {  //Delete
        char Fol[MAX_PATH+2];
        ZeroMemory(Fol,MAX_PATH+2);
        if(Inf->LinkPath.IsEmpty())
          strcpy(Fol,Inf->Path);
        else
          strcpy(Fol,Inf->LinkPath);
        EasyStr OldDisk[2],OldName[2],DiskInZip[2];
        for(int disk=0;disk<2;disk++) 
        {
          if(IsSameStr_I(FloppyDrive[disk].GetDisk(),Fol)) 
          {
            OldDisk[disk]=Fol;
            OldName[disk]=FloppyDisk[disk].DiskName;
            DiskInZip[disk]=FloppyDisk[disk].DiskInZip;
            FloppyDrive[disk].RemoveDisk();
          }
        }
        SHFILEOPSTRUCT fos;
        fos.hwnd=HWND(FullScreen?StemWin:This->Handle);
        fos.wFunc=FO_DELETE;
        fos.pFrom=Fol;
        fos.pTo="\0\0";
        fos.fFlags=FILEOP_FLAGS(int((GetKeyState(VK_SHIFT)<0)?0:FOF_ALLOWUNDO)|int(FullScreen?FOF_SILENT:0));
        fos.hNameMappings=NULL;
        fos.lpszProgressTitle=StaticT("Deleting...");
        EnableWindow(This->Handle,0);
        SHFileOperation(&fos);
        EnableWindow(This->Handle,true);
        for(int disk=0;disk<2;disk++) 
        {
          if(OldDisk[disk].NotEmpty()) 
          {
            if(fos.fAnyOperationsAborted) 
            {
              FloppyDrive[disk].SetDisk(OldDisk[disk],DiskInZip[disk]);
              FloppyDisk[disk].DiskName=OldName[disk];
            }
            else
              SendMessage(GetDlgItem(Win,100+disk),LVM_DELETEITEM,0,0);
          }
        }
        if(fos.fAnyOperationsAborted==0) 
        {
          if(Inf->LinkPath.IsEmpty()&&Inf->Folder==0)  // Deleting disk
            This->UpdateBPBFiles(Inf->Path,"",0);
          This->RefreshDiskView("",0,"",lvi.iItem);
        }
      }
    }
    SendMessage(GetDlgItem(Win,10),BM_SETCHECK,
      !HardDiskMan.DisableHardDrives||HardDiskMan.IsVisible(),0);
#if defined(SSE_ACSI)
    SendMessage(GetDlgItem(Win,11),BM_SETCHECK,
      SSEOptions.Acsi||AcsiHardDiskMan.IsVisible(),0);
#endif
    break;
  case WM_SIZE:
    GET_THIS;
    if(GetDlgItem(Win,10)) 
    {
      SetWindowPos(GetDlgItem(Win,10),0,LOWORD(lPar)-70,10,0,0,
        SWP_NOSIZE|SWP_NOZORDER);
#if defined(SSE_ACSI)
      SetWindowPos(GetDlgItem(Win,11),0,LOWORD(lPar)-70*2,10,0,0,
        SWP_NOSIZE|SWP_NOZORDER);
#endif
      SetWindowPos(GetDlgItem(Win,97),0,0,0,LOWORD(lPar)-(125+26+50+10),21,
        SWP_NOMOVE|SWP_NOZORDER);
      SetWindowPos(GetDlgItem(Win,102),0,0,0,LOWORD(lPar)-20,HIWORD(lPar)-114,
        SWP_NOMOVE|SWP_NOZORDER);
      if(This->SmallIcons==0) 
        SendMessage(GetDlgItem(Win,102),LVM_ARRANGE,LVA_DEFAULT,0);
    }
    if(FullScreen) 
    {
      if(IsZoomed(Win)==0) 
      {
        This->FSMaximized=0;
        RECT rc;GetWindowRect(Win,&rc);
        This->FSWidth=rc.right-rc.left;This->FSHeight=rc.bottom-rc.top;
      }
      else 
        This->FSMaximized=true;
    }
    else 
    {
      if(IsIconic(Win)==0) 
      {
        if(IsZoomed(Win)==0) 
        {
          This->Maximized=0;
          RECT rc;GetWindowRect(Win,&rc);
          This->Width=rc.right-rc.left;This->Height=rc.bottom-rc.top;
        }
        else
          This->Maximized=true;
      }
    }
    break;
  case (WM_USER+1011):
  {
    GET_THIS;
    //ASSERT(This);
    if(This->VisibleDiag())
      SendMessage(This->VisibleDiag(),WM_COMMAND,IDCANCEL,0);
    HWND NewParent=(HWND)lPar;
    if(NewParent) 
    {
      //        SetWindowLong(Win,GWL_STYLE,GetWindowLong(Win,GWL_STYLE) & ~WS_MINIMIZEBOX);
      This->CheckFSPosition(NewParent);
      SetWindowPos(Win,NULL,This->FSLeft,This->FSTop,This->FSWidth,This->FSHeight,SWP_NOZORDER);
    }
    else
      SetWindowPos(Win,NULL,This->Left,This->Top,This->Width,This->Height,SWP_NOZORDER);
    This->ChangeParent(NewParent);
    break;
  }
  case WM_GETMINMAXINFO:
  {
    MINMAXINFO *mmi=(MINMAXINFO*)lPar;
#if defined(SSE_ACSI) //report Dima
    mmi->ptMinTrackSize.x=403+70+GuiSM.cx_frame()*2+GuiSM.cx_vscroll();
#else
    mmi->ptMinTrackSize.x=403+GuiSM.cx_frame()*2+GuiSM.cx_vscroll();
#endif
    mmi->ptMinTrackSize.y=186+GuiSM.cy_caption()+GuiSM.cy_frame()*2;
    if(FullScreen) 
    {
      mmi->ptMaxSize.x=GuiSM.cx_screen()+GuiSM.cx_frame()*2;
      mmi->ptMaxSize.y=GuiSM.cy_screen()+GuiSM.cy_frame()-MENUHEIGHT;
      mmi->ptMaxPosition.x=-GuiSM.cx_frame();
      mmi->ptMaxPosition.y=MENUHEIGHT;
    }
    else 
    {
      mmi->ptMaxPosition.x=-GuiSM.cx_frame();
      mmi->ptMaxPosition.y=-GuiSM.cy_frame();
    }
    break;
  }
  case WM_CLOSE:
    GET_THIS;
    This->Hide();
    return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}

//#pragma warning (default: 4701)


LRESULT CALLBACK TDiskManager::Drive_Icon_WndProc(HWND Win,UINT Mess,
                                              WPARAM wPar,LPARAM lPar) {
  TDiskManager *This;
  int disk=GetDlgCtrlID(Win)-98;
  WORD wpar_lo=LOWORD(wPar);
  switch(Mess) {
  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    RECT box;
    HBRUSH br;
    BeginPaint(Win,&ps);
    GetClientRect(Win,&box);
    br=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(ps.hdc,&box,br);
    if(disk==1&&num_connected_floppies==1)
      DrawIconEx(ps.hdc,0,0,hGUIIcon[RC_ICO_DRIVEB_DISABLED],
        64,64,0,NULL,DI_NORMAL);
    else
      DrawIconEx(ps.hdc,0,0,hGUIIcon[RC_ICO_DRIVEA+disk],
        64,64,0,br,DI_NORMAL);
    if(floppy_instant_sector_access==0) // SS draw snails!
      DrawIconEx(ps.hdc,24,48,hGUIIcon[RC_ICO_ACCURATEFDC],16,16,0,NULL,
        DI_NORMAL);
    DeleteObject(br);
    //SetBkMode(ps.hdc, TRANSPARENT); // unreadable
#if defined(SSE_DRIVE_FREEBOOT)
    SetTextColor(ps.hdc, RGB(255,0,0));
    if(FloppyDrive[disk].single_sided)
      TextOut(ps.hdc,0,0,T("SF354"),(int)T("SF354").Length());
    if(FloppyDrive[disk].freeboot)
      TextOut(ps.hdc,0,0,T("Freeboot"),(int)T("Freeboot").Length());
#endif
    EndPaint(Win,&ps);
    return 0;
  }
  case WM_LBUTTONDOWN:
  case WM_LBUTTONDBLCLK:
    GET_THIS;
    if(disk==1) 
      This->SetNumFloppies(3-num_connected_floppies);
    return 0;
  case WM_RBUTTONDOWN: // right click on drive, make context menu
  case WM_CONTEXTMENU:
  {
    GET_THIS;
    This->MenuTarget=disk;
    HMENU Pop=CreatePopupMenu();
    InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1056+disk,
      T("Choose with Windows file selector"));
#if defined(SSE_DRIVE_FREEBOOT)
/*  The first 520 ST were equipped with a single-sided drive unfortunately.
    The external model was called SF354 (double-sided was SF314).
    The first STF also had an internal single-sided drive.
    Because of that, almost all games were single-sided for a long time.
*/
    if(FloppyDrive[disk].ImageType.Manager!=MNGR_PASTI)
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING|
        ((FloppyDrive[disk].single_sided) ? MF_CHECKED : 0),1048+disk,
        T("Single-sided drive"));
/*  The Freeboot was a hardware mod that could force the drive side to 1 (B)
    with a switch. Using it fooled the ST into thinking it was reading
    side A (based on the YM2149 register) when in fact it was B, so you could
    for example have two single-sided games on one double sided disk, or disk B
    of a game on side B.
    It worked because the WD1772 doesn't check side (it couldn't, it doesn't
    know the side it's operating on).
*/
    InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
      |((FloppyDrive[disk].freeboot) ? MF_CHECKED : 0),1052+disk,
      T("Freeboot side B"));
#endif
    if(OPTION_HACKS && FloppyDrive[disk].motor)
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1046+disk,T("Stop motor"));
#if defined(SSE_DRIVE_SOUND)
    if(OPTION_DRIVE_SOUND)
    {
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING,1054+disk,
        T("Choose drive sound directory"));
      InsertMenu(Pop,0xffffffff,MF_BYPOSITION|MF_STRING
        |(OPTION_DRIVE_SOUND_SEEK_SAMPLE ? MF_CHECKED : 0),1044+disk,
        T("Sampled seek sound"));
    }
#endif
    POINT pt;
    GetCursorPos(&pt); // menu will appear at the mouse pointer
    TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
      pt.x,pt.y,0,This->Handle,NULL);
    DestroyMenu(Pop);
    return 0; // mimic
  }
  case WM_COMMAND:
    GET_THIS;
    if(wpar_lo==100) 
    {
      SendMessage(HWND(lPar),BM_SETCHECK,1,0);
      HMENU Pop=CreatePopupMenu();
      EasyStr CurrentDiskName=This->
        CreateDiskName(FloppyDisk[disk].DiskName,FloppyDisk[disk].DiskInZip);
      for(int n=0;n<10;n++) 
      {
        if(This->InsertHist[disk][n].Path.NotEmpty()) 
        {
          EasyStr MenuItemText=This->CreateDiskName(This->
            InsertHist[disk][n].Name,This->InsertHist[disk][n].DiskInZip);
          if(NotSameStr_I(CurrentDiskName,MenuItemText))
            AppendMenu(Pop,MF_STRING,200+n,MenuItemText);
        }
      }
      RECT rc;
      GetWindowRect(HWND(lPar),&rc);
      TrackPopupMenu(Pop,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
        rc.left,rc.bottom,0,Win,NULL);
      DestroyMenu(Pop);
      SendMessage(HWND(lPar),BM_SETCHECK,0,0);
    }
    else if(wpar_lo>=200&&wpar_lo<210) 
    {
      This->InsertDisk(disk,This->InsertHist[disk][wpar_lo-200].Name,
        This->InsertHist[disk][wpar_lo-200].Path,0,true,
        This->InsertHist[disk][wpar_lo-200].DiskInZip,0,true);
    }
    break;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}


LRESULT CALLBACK TDiskManager::DriveView_WndProc(HWND Win,UINT Mess,
                                            WPARAM wPar,LPARAM lPar) {
  TDiskManager *This;
  GET_THIS;
  if(Mess==WM_DROPFILES) 
  {
    int nFiles=DragQueryFile((HDROP)wPar,0xffffffff,NULL,0);
    for(int i=0;i<nFiles;i++) 
    {
      EasyStr File;
      File.SetLength(MAX_PATH);
      DragQueryFile((HDROP)wPar,i,File,MAX_PATH);
      char *dot=strrchr(GetFileNameFromPath(File),'.');
      if(dot && IsSameStr_I(dot,".LNK")) 
      {
        WIN32_FIND_DATA wfd;
        File=GetLinkDest(File,&wfd);
        dot=strrchr(GetFileNameFromPath(File),'.');
      }
      if(dot) 
      {
        if(ExtensionIsDisk(dot)) 
        {
          EasyStr Name=GetFileNameFromPath(File);
          *strrchr(Name,'.')=0;
          if(DiskMan.InsertDisk(GetDlgCtrlID(Win)-100,Name,File,0,0,"",0,true)) 
            break;
        }
      }
    }
    DragFinish((HDROP)wPar);
    SetForegroundWindow(This->Handle);
    return 0;
  }
  else if(Mess==WM_KEYDOWN && This->Dragging>-1)
    return 0;
  else if(Mess==WM_LBUTTONDOWN||Mess==WM_MBUTTONDOWN||Mess==WM_RBUTTONDOWN||
    Mess==WM_LBUTTONDBLCLK||Mess==WM_MBUTTONDBLCLK||Mess==WM_RBUTTONDBLCLK)
    if(SendMessage(Win,LVM_GETITEMCOUNT,0,0)==0) 
      return 0;
  return CallWindowProc(This->Old_ListView_WndProc,Win,Mess,wPar,lPar);
}


LRESULT CALLBACK TDiskManager::DiskView_WndProc(HWND Win,UINT Mess,
                                                WPARAM wPar,LPARAM lPar) {
  TDiskManager *This;
  GET_THIS;
  if(Mess==WM_DROPFILES) 
  {
    POINT pt;
    GetCursorPos(&pt);
    This->MenuTarget=0;
    HMENU OpMenu=CreatePopupMenu();
    AppendMenu(OpMenu,MF_STRING,4000,T("&Move Here"));
    AppendMenu(OpMenu,MF_STRING,4001,T("&Copy Here"));
    AppendMenu(OpMenu,MF_STRING,4002,T("Create &Shortcut(s) Here"));
    AppendMenu(OpMenu,MF_SEPARATOR,4099,NULL);
    AppendMenu(OpMenu,MF_STRING,4098,T("Cancel"));
    TrackPopupMenu(OpMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,0,
      This->Handle,NULL);
    DestroyMenu(OpMenu);
    MSG mess;
    while(PeekMessage(&mess,This->Handle,WM_COMMAND,WM_COMMAND,PM_REMOVE)) 
      DispatchMessage(&mess);
    EasyStr SelPath,SelLink;
    if(This->MenuTarget>=4000&&This->MenuTarget<=4002) 
    {
      int nFiles=DragQueryFile((HDROP)wPar,0xffffffff,NULL,0);
      if(This->MenuTarget==4002) 
      {
        EasyStr File,Name,LinkFile;
        for(int i=0;i<nFiles;i++) 
        {
          File.SetLength(MAX_PATH);
          DragQueryFile((HDROP)wPar,i,File,MAX_PATH);
          Name.SetLength(MAX_PATH);
          GetLongPathName(File,Name,MAX_PATH);
          Name=EasyStr(GetFileNameFromPath(Name));
          char *dot=strrchr(Name,'.');
          if(dot)
            if(ExtensionIsDisk(dot)||ExtensionIsPastiDisk(dot)) 
              *dot=0;
          LinkFile=This->DisksFol+SLASH+Name+".lnk";
          int n=2;
          while(Exists(LinkFile))
            LinkFile=This->DisksFol+SLASH+Name+" ("+(n++)+").lnk";
          CreateLink(LinkFile,File);
          SelLink=LinkFile;
        }
      }
      else 
      {
        char *From=new char[MAX_PATH*nFiles+2];
        ZeroMemory(From,MAX_PATH*nFiles+2);
        char *FromPtr=From;
        for(int i=0;i<nFiles;i++) 
        {
          DragQueryFile((HDROP)wPar,i,FromPtr,MAX_PATH);
          // support links
          if(FileIsDisk(FromPtr)) 
            SelPath=This->DisksFol+SLASH+GetFileNameFromPath(FromPtr);
          FromPtr+=strlen(FromPtr)+1;
        }
        SHFILEOPSTRUCT fos;
        fos.hwnd=This->Handle;
        fos.wFunc=int((This->MenuTarget==4000)?FO_MOVE:FO_COPY);
        fos.pFrom=From;
        fos.pTo=This->DisksFol;
        fos.fFlags=FILEOP_FLAGS(FOF_ALLOWUNDO)|FOF_RENAMEONCOLLISION;
        fos.hNameMappings=NULL;
        fos.lpszProgressTitle=LPSTR((This->MenuTarget==4000) 
          ? StaticT("Moving...") : StaticT("Copying..."));
        EnableWindow(This->Handle,0);
        SHFileOperation(&fos);
        EnableWindow(This->Handle,true);
      }
      This->RefreshDiskView(SelPath,0,SelLink);
      SetForegroundWindow(This->Handle);
    }
    DragFinish((HDROP)wPar);
    return 0;
  }
  else if(Mess==WM_KEYDOWN && This->Dragging>-1)
    return 0;
  else if(Mess==WM_VSCROLL && This->Dragging>-1)
  {
    ImageList_DragLeave(This->Handle);
    LRESULT Ret=CallWindowProc(This->Old_ListView_WndProc,Win,Mess,wPar,lPar);
    UpdateWindow(Win);
    POINT mpt;
    GetCursorPos(&mpt);
    ScreenToClient(This->Handle,&mpt);
    ImageList_DragEnter(This->Handle,mpt.x-This->DragWidth,
      mpt.y-This->DragHeight);
    This->MoveDrag();
    return Ret;
  }
  return CallWindowProc(This->Old_ListView_WndProc,Win,Mess,wPar,lPar);
}

#undef GET_THIS


void TDiskManager::SetDriveViewEnable(int drive,bool EnableIt) {
  HWND LV=GetDlgItem(Handle,100+drive);
  if(GetFocus()==LV) 
    SetFocus(DiskView);
  SendMessage(LV,LVM_SETBKCOLOR,0,
    (LPARAM)GetSysColor(int(EnableIt?COLOR_WINDOW:COLOR_BTNFACE)));
  if(EnableIt)
    SetWindowLong(LV,GWL_STYLE,GetWindowLong(LV,GWL_STYLE)|WS_TABSTOP);
  else
    SetWindowLong(LV,GWL_STYLE,GetWindowLong(LV,GWL_STYLE) & ~WS_TABSTOP);
  InvalidateRect(LV,NULL,true);
}


TDiskManFileInfo* TDiskManager::GetItemInf(int iItem,HWND LV/*=NULL*/) {
  LV_ITEM lvi;
  lvi.iItem=iItem;
  lvi.iSubItem=0;
  lvi.mask=LVIF_PARAM;
  lvi.lParam=0; // v4 anti-crash
  SendMessage(HWND(LV ? LV:DiskView),LVM_GETITEM,0,(LPARAM)&lvi);
  return (TDiskManFileInfo*)lvi.lParam;
}


#endif//WIN32


#ifdef UNIX

int TDiskManager::dir_lv_notify_handler(hxc_dir_lv *dlv,int mess,int i)
{
  TDiskManager *This=(TDiskManager*)dlv->owner;
  if (mess==DLVN_FOLDERCHANGE){
    This->set_path((char*)i,true,0);
    return 0;
  }else if (mess==DLVN_DOUBLECLICK || mess==DLVN_RETURN){
    if (This->DoubleClickAction==0 || i<0) return 0;

    EasyStr file=dlv->get_item_path(i);
    if ((GetFileAttributes(file) & FILE_ATTRIBUTE_DIRECTORY)==0){
      int action=2;
      if (This->DoubleClickAction==1) action=0;
      This->PerformInsertAction(action,dlv->get_item_name(i),file,"");
      return 1; // Don't focus the listview
    }
  }else if (mess==DLVN_DROP){
  	hxc_listview_drop_struct *ds=(hxc_listview_drop_struct*)i;
    EasyStr file=dlv->get_item_path(ds->dragged);
    int type=dlv->sl[ds->dragged].Data[DLVD_TYPE];

    if (dlv->lv.is_dropped_in(ds,&(This->HomeBut))){
    }else if (dlv->lv.is_dropped_in(ds,&(This->disk_name[0])) ||
              dlv->lv.is_dropped_in(ds,&(This->drive_icon[0]))){
      if (type>=2){
        This->PerformInsertAction(0,dlv->get_item_name(ds->dragged),file,"");
      }
    }else if (dlv->lv.is_dropped_in(ds,&(This->disk_name[1])) ||
              dlv->lv.is_dropped_in(ds,&(This->drive_icon[1]))){
      if (type>=2){
        This->PerformInsertAction(1,dlv->get_item_name(ds->dragged),file,"");
      }
    }
  }else if (mess==DLVN_CONTEXTMENU){
    dlv->pop.lpig=&Ico16;
    if (i>=0){
      EasyStr file=dlv->get_item_path(i);
      int is_link=dlv->sl[i].Data[DLVD_FLAGS] & DLVF_LINKMASK;
      int type=dlv->sl[i].Data[DLVD_TYPE];
      bool is_zip=type >= This->ArchiveTypeIdx;
      bool read_only=(dlv->sl[i].Data[DLVD_FLAGS] & DLVF_READONLY)!=0;
      int pos=0;
      if (is_link<2){ // 2 = dead link
        if (type>=2){ // File
          dlv->pop.menu.InsertAt(pos++,2,StripAndT("Insert Into Drive &A"),ICO16_INSERTDISK,1010);
          dlv->pop.menu.InsertAt(pos++,2,StripAndT("Insert Into Drive &B"),ICO16_INSERTDISK,1011);
          dlv->pop.menu.InsertAt(pos++,2,StripAndT("Insert, Reset and &Run"),ICO16_INSERTDISK,1012);
          dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);
          dlv->pop.menu.InsertAt(pos++,2,StripAndT("Get &Contents"),-1,1015);
          dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);
          if (is_link){
            dlv->pop.menu.InsertAt(pos++,2,StripAndT("&Go To Disk"),ICO16_FORWARD,1090);
            dlv->pop.menu.InsertAt(pos++,2,StripAndT("Open Disk's Folder in File Manager"),-1,1092);
            dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);
          }
          if (is_zip==0){
            dlv->pop.menu.InsertAt(pos++,2,StripAndT("Read &Only"),
                                    (read_only ? ICO16_TICKED:ICO16_UNTICKED),1040);
            dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);
          }else{
            dlv->pop.menu.InsertAt(pos++,2,StripAndT("E&xtract Disk Here"),ICO16_ZIP_RW,1080);
            dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);
          }
        }else{ // Folder
          int pos=0;
          dlv->pop.menu.InsertAt(pos++,2,StripAndT("Open in File Manager"),-1,1060);
          dlv->pop.menu.InsertAt(pos++,2,StripAndT("&Find..."),-1,1061);
          dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);
        }
      }
    }
    int pos=dlv->pop.menu.NumStrings-1; // New folder
    dlv->pop.menu[pos].Data[0]=ICO16_FOLDER;
    dlv->pop.menu.InsertAt(pos++,2,StripAndT("Open Current Folder In File Manager"),-1,2003);
    dlv->pop.menu.InsertAt(pos++,2,StripAndT("Find In Current Folder"),-1,2010);
    dlv->pop.menu.InsertAt(pos++,2,"-",-1,0);  
    dlv->pop.menu.Add(StripAndT("New Standard &Disk Image"),ICO16_DISK,1001);
    dlv->pop.menu.Add(StripAndT("New Custom Disk &Image"),ICO16_DISK,1002);
#if defined(SSE_GUI_DM_STW) //new context option
    dlv->pop.menu.Add(StripAndT("New ST&W Disk Image"),ICO16_DISK,1003);
#endif
#if defined(SSE_GUI_DM_HFE) //new context option
    dlv->pop.menu.Add(StripAndT("New &HFE Disk Image"),ICO16_DISK,1004);
#endif

  }else if (mess==DLVN_POPCHOOSE){
    if (dlv->pop.menu[i].NumData<2) return 0;

    int action=dlv->pop.menu[i].Data[1];
    switch (action){
      case 1010:
      case 1011:
      case 1012:
        This->PerformInsertAction(action-1010,
                    dlv->get_item_name(dlv->lv.sel),
                    dlv->get_item_path(dlv->lv.sel),"");
        return 0;
      case 1015:
        This->GetContentsSL(dlv->get_item_path(dlv->lv.sel));
        if (This->contents_sl.NumStrings){
          This->ContentsLinksPath=This->DisksFol;
          This->ShowContentDiag();
        }
        return 0;
      case 1040:
        This->ToggleReadOnly(dlv->lv.sel);
        return 0;
      case 1060:case 1092:
      {
        Str fol=dlv->get_item_path(dlv->lv.sel);
        if (action==1092) RemoveFileNameFromPath(fol,KEEP_SLASH);
        shell_execute(Comlines[COMLINE_FM],Str("[PATH]\n")+fol);
        return 0;
      }
      case 1061:
        shell_execute(Comlines[COMLINE_FIND],Str("[PATH]\n")+dlv->get_item_path(dlv->lv.sel));
        return 0;
      case 1080:
        This->ExtractDisks(dlv->get_item_path(dlv->lv.sel));
        return 0;
      case 1090:
      {
        Str File=dlv->get_item_path(dlv->lv.sel,true);
        Str DiskFol=File;
        RemoveFileNameFromPath(DiskFol,REMOVE_SLASH);
        This->set_path(DiskFol);
        dlv->select_item_by_name(GetFileNameFromPath(File));
        return 0;
      }
      case 1001:
      case 1002:
      {
        EasyStr STName;
        int sectors=1440,secs_per_track=9,sides=2;
        if (action==1002){
          STName=This->GetCustomDiskImage(&sectors,&secs_per_track,&sides);
        }else{
          hxc_prompt prompt;
          STName=prompt.ask(XD,T("Blank Disk"),T("Enter Name"));
        }
        if (STName.NotEmpty()){
          STName=GetUniquePath(This->DisksFol,STName+".st");
          if (This->CreateDiskImage(STName,sectors,secs_per_track,sides)){
            This->RefreshDiskView(STName);
          }else{
            Alert(Str(T("Could not create the disk image "))+STName,
                      T("Error"),MB_ICONEXCLAMATION);
          }
        }
        return 0;
      }
#if defined(SSE_GUI_DM_STW)
      case 1003:  // STW
      {
        hxc_prompt prompt;
        EasyStr STName=prompt.ask(XD,T("STW Disk"),T("Enter Name"));
        if (STName.NotEmpty()){
          STName=GetUniquePath(This->DisksFol,STName+".stw");
          if(ImageSTW[0].Create(STName)) {
            This->RefreshDiskView(STName);
          }else{
            Alert(Str(T("Could not create the disk image "))+STName,
                      T("Error"),MB_ICONEXCLAMATION);
          }
        }
        return 0;
      }
#endif
#if defined(SSE_GUI_DM_HFE)
      case 1004:  // HFE
      {
        hxc_prompt prompt;
        EasyStr STName=prompt.ask(XD,T("HFE Disk"),T("Enter Name"));
        if (STName.NotEmpty()){
          STName=GetUniquePath(This->DisksFol,STName+".hfe");
          if(ImageHFE[0].Create(STName)) {
            This->RefreshDiskView(STName);
          }else{
            Alert(Str(T("Could not create the disk image "))+STName,
                      T("Error"),MB_ICONEXCLAMATION);
          }
        }
        return 0;
      }
#endif
      case 2003:
        shell_execute(Comlines[COMLINE_FM],Str("[PATH]\n")+This->DisksFol);
        return 0;
      case 2010:
        shell_execute(Comlines[COMLINE_FIND],Str("[PATH]\n")+This->DisksFol);
        return 0;
#if defined(SSE_ACSI_OPTION)
      case 2029:
        SSEOption.Acsi=!SSEOption.Acsi;
        return 0;
#endif
    }
  }else if (mess==DLVN_CONTENTSCHANGE){
    dlv_ccn_struct *p_ccn=(dlv_ccn_struct*)i;
    if (p_ccn->time==DLVCCN_BEFORE){
      This->TempEject_InDrive[0]=0;
      This->TempEject_InDrive[1]=0;
      for (int d=0;d<2;d++){
        // Should check whether in folder being deleted too.
        if (IsSameStr_I(FloppyDrive[d].GetDisk(),p_ccn->path)){
          This->TempEject_InDrive[d]=true;
          This->TempEject_Name=FloppyDisk[d].DiskName;
          This->TempEject_DiskInZip[d]=FloppyDisk[d].DiskInZip;
          FloppyDrive[d].RemoveDisk();
        }
      }
    }else if (p_ccn->time==DLVCCN_AFTER){
      Str new_path=p_ccn->path;
      Str new_name=This->TempEject_Name;
      if (p_ccn->success){
        if (p_ccn->action==DLVCCN_DELETE){
          This->UpdateDiskNames(0);
          This->UpdateDiskNames(1);
          return 0;
        }else if (p_ccn->action==DLVCCN_MOVE || p_ccn->action==DLVCCN_RENAME){
          new_path=p_ccn->new_path;
          if (p_ccn->action==DLVCCN_RENAME){
            new_name=GetFileNameFromPath(new_path);
            if (p_ccn->flags & DLVF_EXTREMOVED){
              char *dot=strrchr(new_name,'.');
              if (dot) *dot=0;
            }
          }
        }
      }
      for (int d=0;d<2;d++){
        if (This->TempEject_InDrive[d]){
          This->InsertDisk(d,new_name,new_path,0,0,This->TempEject_DiskInZip[d]);
        }
      }
    }
    return 0;
  }
  return 0;
}
//---------------------------------------------------------------------------
void TDiskManager::UpdateDiskNames(int d)
{
  if (XD==NULL || Handle==0) return;

  if (FloppyDrive[d].DiskInDrive()){
    Str RO;
    if (FloppyDisk[d].ReadOnly) RO=" <RO>";
    disk_name[d].set_text(FloppyDisk[d].DiskName+RO);
  }else{
    disk_name[d].set_text("");
  }
}
//---------------------------------------------------------------------------
void TDiskManager::set_home(Str fol)
{
  if (Alert(fol+"/"+"\n\n"+
        T("Are you sure you want to make this folder your new home folder?"),
        T("Change Home Folder?"),MB_YESNO | MB_ICONQUESTION)==IDYES){
    HomeFol=fol;
  }
}
//---------------------------------------------------------------------------
int TDiskManager::button_notify_handler(hxc_button *But,int mess,int *Inf)
{
  TDiskManager *This=(TDiskManager*)GetProp(But->XD,But->parent,cWinThis);
  if (mess==BN_CLICKED){
    switch (But->id){
      case 2: //back
      	if (This->HistBackLength > 0){
          for (int n=9;n>0;n--){
            This->HistForward[n]=This->HistForward[n-1];
          }
          This->HistForward[0]=This->DisksFol;
          if((This->HistForwardLength)<10)This->HistForwardLength++;

          This->set_path(This->HistBack[0],0);
          for (int n=0;n<9;n++){
            This->HistBack[n]=This->HistBack[n+1];
          }
          This->HistBack[9]="";
          This->HistBackLength--;
				}
      	break;
      case 3: //forward
      	if (This->HistForwardLength > 0){
          for (int n=9;n>0;n--){
            This->HistBack[n]=This->HistBack[n-1];
          }
          if((This->HistBackLength)<10)This->HistBackLength++;
          This->HistBack[0]=This->DisksFol;
          This->set_path(This->HistForward[0],0);
          for (int n=0;n<9;n++){
            This->HistForward[n]=This->HistForward[n+1];
          }
          This->HistForward[9]="";
          This->HistForwardLength--;
				}
      	break;
      case 4: //go home
      {
        bool at_home=IsSameStr(This->HomeFol.Text,This->DisksFol.Text);
        if (Inf[0]!=Button1 || at_home){
          But->set_check(true);
          pop.lpig=&Ico16;
          pop.menu.DeleteAll();
          if (at_home==0){
            pop.menu.Add(This->HomeFol,ICO16_HOMEFOLDER,4000);
            pop.menu.Add("-",-1);
          }
          for (int i=0;i<10;i++){
            pop.menu.Add(Str(i+1)+": "+This->QuickFol[i],ICO16_FOLDER,4010+i);
          }
          pop.create(XD,But->handle,0,But->h,This->menu_popup_notifyproc,This);
        }else{
          This->set_path(This->HomeFol,true);
        }
        break;
      }
      case 5: //set home
      {
        bool at_home=IsSameStr(This->HomeFol.Text,This->DisksFol.Text);
        if (Inf[0]!=Button1 || at_home){
          But->set_check(true);
          pop.lpig=&Ico16;
          pop.menu.DeleteAll();
          if (at_home==0){
            pop.menu.Add(Str("(")+This->HomeFol+")",ICO16_HOMEFOLDER,4100);
            pop.menu.Add("-",-1);
          }
          for (int i=0;i<10;i++){
            pop.menu.Add(Str(i+1)+": ("+This->QuickFol[i]+")",ICO16_FOLDER,4110+i);
          }
          pop.create(XD,But->handle,0,But->h,This->menu_popup_notifyproc,This);
        }else{
          This->set_home(This->DisksFol);
        }
      	break;
			}
			case 6:
      {
    		But->set_check(true);
    		pop.lpig=&Ico16;
      	pop.menu.DeleteAll();
        pop.menu.Add(StripAndT("Disconnect Drive B"),
        	int((num_connected_floppies==1) ? ICO16_TICKED:ICO16_UNTICKED),2012);
        pop.menu.Add(StripAndT("Accurate Disk Access Times (Slow)"),
        	int((floppy_instant_sector_access==0) ? ICO16_TICKED:ICO16_UNTICKED),2013);
        pop.menu.Add(StripAndT("Read/Write Archives (Changes Lost On Eject)"),
        	int(FloppyArchiveIsReadWrite ? ICO16_TICKED:ICO16_UNTICKED),2014);

        pop.menu.Add("-",-1,0);
        pop.menu.Add(StripAndT("Search Disk Image Database"),-1,2025);
        pop.menu.Add(StripAndT("Open Current Folder In File Manager"),-1,2003);
        pop.menu.Add(StripAndT("Find In Current Folder"),-1,2010);
        pop.menu.Add("-",-1,0);
#if defined(SSE_ACSI_OPTION)  //v3.7.2
        pop.menu.Add(StripAndT("ACSI hard disk image"),
        	int(SSEOption.Acsi?ICO16_TICKED:ICO16_UNTICKED),2029);
        pop.menu.Add("-",-1,0);
#endif
        pop.menu.Add(StripAndT("Automatically Insert &Second Disk"),int(This->AutoInsert2 ? ICO16_TICKED:ICO16_UNTICKED),2016);
//        pop.menu.Add(StripAndT("Hide Dangling Links"),int(This->HideBroken),2002);
        pop.menu.Add(StripAndT("E&ject Disks When Quit"),
        	int(This->EjectDisksWhenQuit ? ICO16_TICKED:ICO16_UNTICKED),2011);
        pop.menu.Add("-",-1,0);
        int idx=pop.menu.NumStrings;
        pop.menu.Add(StripAndT("Double Click On Disk Does &Nothing"),ICO16_UNRADIOMARKED,2007);
        pop.menu.Add(StripAndT("Double Click On Disk Inserts In &Drive A"),ICO16_UNRADIOMARKED,2008);
        pop.menu.Add(StripAndT("Double Click On Disk Inserts, &Resets and Runs"),ICO16_UNRADIOMARKED,2009);
        pop.menu[idx+This->DoubleClickAction].Data[0]=ICO16_RADIOMARK;
        pop.menu.Add("-",-1,0);
        pop.menu.Add(StripAndT("&Close Disk Manager After Insert, Reset and Run"),
                  int(This->CloseAfterIRR ? ICO16_TICKED:ICO16_UNTICKED),2015);
        pop.create(XD,But->handle,0,But->h,This->menu_popup_notifyproc,This);
				break;
      }
			case 10:
				HardDiskMan.Show();
				break;
			case 101:
				This->SetNumFloppies(3-num_connected_floppies);
				break;
      case 200:case 201:
      {
        if (Inf[0]!=Button3 && Inf[0]!=Button2) break;

        pop.lpig=&Ico16;
        pop.menu.DeleteAll();

        int d=But->id-200;
        bool added_line=true;
        if (FloppyDrive[d].NotEmpty()){
          if (FloppyDisk[d].IsZip()==0){
            int ico=ICO16_UNTICKED;
            if (FloppyDisk[d].ReadOnly) ico=ICO16_TICKED;
            pop.menu.Add(StripAndT("Read &Only"),ico,1040,(-d)-1);
          }
        }
        if (FloppyDrive[0].NotEmpty() || FloppyDrive[1].NotEmpty()){
          pop.menu.Add(StripAndT("&Swap"),-1,1100);
          added_line=0;
        }
        if (FloppyDrive[d].NotEmpty()){
          pop.menu.Add(StripAndT("&Remove Disk From Drive"),ICO16_EJECTDISK,1101,d);
          pop.menu.Add(StripAndT("&Go To Disk"),-1,1090,d);
        }
        EasyStr CurrentDiskName=This->CreateDiskName(FloppyDisk[d].DiskName,
          FloppyDisk[d].DiskInZip);
        for (int n=0;n<10;n++){
          if (This->InsertHist[d][n].Path.NotEmpty()){
            EasyStr MenuItemText=This->CreateDiskName(This->InsertHist[d][n].Name,This->InsertHist[d][n].DiskInZip);
            if (NotSameStr_I(CurrentDiskName,MenuItemText)){
              if (added_line==0){
                pop.menu.Add("-",-1,0,0);
                added_line=true;
              }
              pop.menu.Add(MenuItemText,ICO16_INSERTDISK,3000+n,d);
            }
          }
        }
        if (pop.menu.NumStrings) pop.create(XD,0,POP_CURSORPOS,0,This->menu_popup_notifyproc,This);
        break;
      }
      case 302:case 303:
        This->EjectDisk(But->id & 1);
        break;
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
int TDiskManager::menu_popup_notifyproc(hxc_popup *pPop,int mess,int i)
{
  TDiskManager *This=(TDiskManager*)(pPop->owner);
	int id=pop.menu[i].Data[1];
	if (mess==POP_CHOOSE){
		switch (id){
      case 2002:
        This->HideBroken=!This->HideBroken;
        This->dir_lv.show_broken_links=(This->HideBroken==0);
        This->RefreshDiskView();
        break;
      case 2007:case 2008:case 2009:
        This->DoubleClickAction=id-2007;
        break;
      case 2011:
        This->EjectDisksWhenQuit=!This->EjectDisksWhenQuit;
        break;
      case 2012:
				This->SetNumFloppies(3-num_connected_floppies);
        break;
      case 2013:
        floppy_instant_sector_access=!floppy_instant_sector_access;
        CheckResetDisplay();
        FloppyDrive[0].UpdateAdat();
        FloppyDrive[1].UpdateAdat();        
        break;
      case 2014:
      {
        FloppyArchiveIsReadWrite=!FloppyArchiveIsReadWrite;
        if (FloppyDisk[0].IsZip()) FloppyDrive[0].ReinsertDisk();
        if (FloppyDisk[1].IsZip()) FloppyDrive[1].ReinsertDisk();
        int zipicon=ICO16_ZIP_RO;
        if (FloppyArchiveIsReadWrite) zipicon=ICO16_ZIP_RW;
        for (int i=This->ArchiveTypeIdx;i<This->dir_lv.ext_sl.NumStrings;i++){
          This->dir_lv.ext_sl[i].Data[0]=zipicon;
        }
        This->UpdateDiskNames(0);
        This->UpdateDiskNames(1);
        This->RefreshDiskView();
        break;
      }
      case 2015:
        This->CloseAfterIRR=!This->CloseAfterIRR;
        break;
      case 2016:
        This->AutoInsert2=!This->AutoInsert2;
        break;
      case 2025:
        This->ShowDatabaseDiag();
        break;
      case 1040:  // Toggle Read-Only
        This->ToggleReadOnly(pop.menu[i].Data[2]);
        break;
      case 1100:
        This->SwapDisks(0);
        break;
      case 1101:
        This->EjectDisk(pop.menu[i].Data[2]);
        break;
      case 1090:
      {
        int d=pop.menu[i].Data[2]; // Get index
        EasyStr DiskFol=FloppyDrive[d].GetDisk();
        RemoveFileNameFromPath(DiskFol,REMOVE_SLASH);
        This->set_path(DiskFol);
        This->dir_lv.select_item_by_name(GetFileNameFromPath(FloppyDrive[d].GetDisk()));
        break;
      }
      case 2003:
        shell_execute(Comlines[COMLINE_FM],Str("[PATH]\n")+This->DisksFol);
        break;
      case 2010:
        shell_execute(Comlines[COMLINE_FIND],Str("[PATH]\n")+This->DisksFol);
        break;
		}
    if (id>=3000 && id<3030){
      int disk=pop.menu[i].Data[2];
      int n=id-3000;
      This->InsertDisk(disk,This->InsertHist[disk][n].Name,This->InsertHist[disk][n].Path,
                        0,true,This->InsertHist[disk][n].DiskInZip,0,true);
    }else if (id>=4000 && id<4100){
      if (id==4000){
        This->set_path(This->HomeFol,true);
      }else{
        id-=4010;
        if (This->QuickFol[id].NotEmpty()) This->set_path(This->QuickFol[id],true);
      }
    }else if (id>=4100 && id<4200){
      if (id==4100){
        This->set_home(This->DisksFol);
      }else{
        id-=4110;
        This->QuickFol[id]=This->DisksFol;
      }
    }
	}
  This->MenuBut.set_check(0);
  This->HomeBut.set_check(0);
  This->SetHomeBut.set_check(0);
	return 0;
}
//---------------------------------------------------------------------------
void TDiskManager::ToggleReadOnly(int i)
{
  EasyStr DiskPath;
  if (i<0){
    DiskPath=FloppyDrive[-(i+1)].GetDisk();
  }else{
    DiskPath=dir_lv.get_item_path(i,true);
  }

  bool InDrive[2]={0,0};
  EasyStr OldName[2],DiskInZip[2];
  for (int d=0;d<2;d++){
    if (IsSameStr_I(FloppyDrive[d].GetDisk(),DiskPath)){
      InDrive[d]=true;
      OldName[d]=FloppyDisk[d].DiskName;
      DiskInZip[d]=FloppyDisk[d].DiskInZip;
      FloppyDrive[d].RemoveDisk();
    }
  }
  DWORD Attrib=GetFileAttributes(DiskPath);
  if (Attrib & FILE_ATTRIBUTE_READONLY){
    SetFileAttributes(DiskPath,Attrib & ~FILE_ATTRIBUTE_READONLY);
  }else{
    SetFileAttributes(DiskPath,Attrib | FILE_ATTRIBUTE_READONLY);
  }

  if (InDrive[0]) InsertDisk(0,OldName[0],DiskPath,0,0,DiskInZip[0]);
  if (InDrive[1]) InsertDisk(1,OldName[1],DiskPath,0,0,DiskInZip[1]);

  RemoveFileNameFromPath(DiskPath,REMOVE_SLASH);
  if (IsSameStr(DiskPath,DisksFol)) RefreshDiskView();
}
//---------------------------------------------------------------------------
void TDiskManager::RefreshDiskView(Str sel)
{
	set_path(DisksFol,0);
  if (sel.NotEmpty()){
    dir_lv.select_item_by_name(GetFileNameFromPath(sel));
  }
}
//---------------------------------------------------------------------------
void TDiskManager::set_path(EasyStr new_path,bool add_to_history,bool change_dir_lv)
{
  if (add_to_history){
    HistForward[0]="";
    HistForwardLength=0;
    for (int n=9;n>0;n--) HistBack[n]=HistBack[n-1];
    HistBack[0]=DisksFol;
    if (HistBackLength<10) HistBackLength++;
  }

	NO_SLASH(new_path);
  if (change_dir_lv){
    dir_lv.fol=new_path;
    dir_lv.refresh_fol();
  }
  DisksFol=new_path;
  DirOutput.set_text(DisksFol);
}

//---------------------------------------------------------------------------
int TDiskManager::WinProc(TDiskManager *This,Window Win,XEvent *Ev)
{
  switch (Ev->type){
    case ClientMessage:
      if (Ev->xclient.message_type==hxc::XA_WM_PROTOCOLS){
        if (Atom(Ev->xclient.data.l[0])==hxc::XA_WM_DELETE_WINDOW){
          This->Hide();
        }
      }
      break;
    case ConfigureNotify:
    {
      XWindowAttributes wa;
      XGetWindowAttributes(XD,Win,&wa);

      This->Width=wa.width;This->Height=wa.height;
      int w=This->Width;int h=This->Height;

      for (int d=0;d<2;d++){
  	    XResizeWindow(XD,This->disk_name[d].handle,MAX(w-(10+32 + 25+10+60+10),10),25);
     	}
      XResizeWindow(XD,This->DirOutput.handle,MAX(w-145,30),25);
      XResizeWindow(XD,This->dir_lv.lv.handle,MAX(w-20,10),MAX(h-120,10));

      XSync(XD,0);
      break;
    }
  }
  return PEEKED_MESSAGE;
}
//---------------------------------------------------------------------------
Str TDiskManager::GetCustomDiskImage(int *pSectors,int *pSecsPerTrack,int *pSides)
{
  int w=300,h=10+35+35+35+25+10;
  Window handle=hxc::create_modal_dialog(XD,w,h,T("Create Custom Disk Image"),true);
  if (handle==0) return "";

  hxc_edit *p_ed;
  hxc_dropdown *p_sides_dd,*p_tracks_dd,*p_secs_dd;

  int y=10,x=10,hw=(w-20)/2,tw;
  hxc_button *p_but=new hxc_button(XD,handle,x,y,0,25,NULL,this,BT_LABEL,T("Name"),0,hxc::col_bk);
  x+=p_but->w+5;

  p_ed=new hxc_edit(XD,handle,x,y,w-10-x,25,NULL,this);
  p_ed->set_text(T("Blank Disk"),true);
  y+=35;

  x=10;
  new hxc_button(XD,handle,x,y,0,25,NULL,this,BT_LABEL,T("Sides"),0,hxc::col_bk);
  x+=hw;

  p_sides_dd=new hxc_dropdown(XD,handle,x,y,hw,200,NULL,this);
  p_sides_dd->additem("1",1);
  p_sides_dd->additem("2",2);
  p_sides_dd->sel=1;
  p_sides_dd->changesel(SidesIdx);
  y+=35;

  x=10;
  new hxc_button(XD,handle,x,y,0,25,NULL,this,BT_LABEL,T("Tracks"),0,hxc::col_bk);
  x+=hw;

  tw=hxc::get_text_width(XD,T("0 to "));
  new hxc_button(XD,handle,x-tw,y,0,25,NULL,this,BT_LABEL,T("0 to "),0,hxc::col_bk);

  p_tracks_dd=new hxc_dropdown(XD,handle,x,y,hw,300,NULL,this);
  for (int n=75;n<=FLOPPY_MAX_TRACK_NUM;n++) p_tracks_dd->additem(Str(n),n);
  p_tracks_dd->sel=80-75;
  p_tracks_dd->changesel(TracksIdx);
  y+=35;

  x=10;
  new hxc_button(XD,handle,x,y,0,25,NULL,this,BT_LABEL,T("Sectors"),0,hxc::col_bk);
  x+=hw;

  tw=hxc::get_text_width(XD,T("1 to "));
  new hxc_button(XD,handle,x-tw,y,0,25,NULL,this,BT_LABEL,T("1 to "),0,hxc::col_bk);

  p_secs_dd=new hxc_dropdown(XD,handle,x,y,hw,300,NULL,this);
  for (int n=8;n<=FLOPPY_MAX_SECTOR_NUM;n++) p_secs_dd->additem(Str(n),n);
  p_secs_dd->sel=9-8;
  p_secs_dd->changesel(SecsPerTrackIdx);

  Str ret;
  if (hxc::show_modal_dialog(XD,handle,true,p_ed->handle)==1){
    ret=p_ed->text;

    // 0 to tracks_per_side inclusive! Choosing 80 gives you 81 tracks.
    int tracks_per_side=p_tracks_dd->sl[p_tracks_dd->sel].Data[0]+1;
    *pSecsPerTrack=p_secs_dd->sl[p_secs_dd->sel].Data[0];
    *pSides=p_sides_dd->sl[p_sides_dd->sel].Data[0];

    *pSectors=*pSecsPerTrack * tracks_per_side * *pSides;

    SidesIdx=p_sides_dd->sel;
    TracksIdx=p_tracks_dd->sel;
    SecsPerTrackIdx=p_secs_dd->sel;
  }

  hxc::destroy_modal_dialog(XD,handle);

  return ret;
}


int TDiskManager::diag_lv_np(hxc_listview *lv,int mess,int i)
{
  if (mess==LVN_ICONCLICK){
    int icon=lv->sl[i].Data[0]-101;
    if (icon==ICO16_TICKED){
      icon=ICO16_UNTICKED;
    }else{
      icon=ICO16_TICKED;
    }
    lv->sl[i].Data[0]=101+icon;
    lv->draw(0);
  }
  return 0;
}

#endif//UNIX


bool TDiskManager::CreateDiskImage(char *STName,WORD Sectors,WORD SecsPerTrack,
                                  WORD Sides) {
#if defined(SSE_DISK_CREATE_MSA_DIM)
#ifndef SSE_LEAN_AND_MEAN
  if(!SecsPerTrack||!Sides) 
    return false;
#endif
  WORD nTracks=(Sectors/SecsPerTrack)/Sides;
  FILE *f=fopen(STName,"wb");
  if(f==NULL)
    return false;
  // write header
  int header_size=0;
  WORD header_word;
  switch(SSEConfig.DiskImageCreated) {
  case EXT_MSA:
/*
Header:
  Word	ID marker, should be $0E0F
  Word	Sectors per track
  Word	Sides (0 or 1; add 1 to this to get correct number of sides)
  Word	Starting track (0-based)
  Word	Ending track (0-based)
*/
    header_size=10+2;
    header_word=0x0E0F;
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=SecsPerTrack;
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=Sides-1;
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=0;
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=nTracks-1;
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    break;
  case EXT_DIM:
/*
$00     $4242 ("BB") identifier
$02     low byte=1=get used sectors, hi byte=1=read disk conf
$04     seems to be always(?) zero

$06     hi byte=sides
$08     hi byte=sectors
$0a     hi byte=start track
$0c     hi byte=end track
0x000D	Byte		Double-Density(0) or High-Density (1)
The following information block can easily be identified as a Bios
Parameter Block (BPB). The contained data is encoded in Motorola manner.
$0e     RECSIZ sector size (bytes) 
$10     CLSIZ  sectors per cluster
$12     CLSIZB cluster size (bytes) 
$14     RDLEN  root dir size (sectors) 
$16     FSIZ   FAT size (sectors) 
$18     FATREC first sector if 2nd FAT (the one that is used by TOS)
$1a     DATREC Number of 1st data sector 
$1c     NUMCL  Total number of clusters minus DATREC
$1e     BFLAGS (Bit 0 is 0 for 12 bit FAT, 1 for 16 bit FAT)
*/
    header_size=32;
    header_word=0x4242;
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=0;
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=0;
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=Sides-1;
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=SecsPerTrack;
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=0;
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=nTracks-1;
    fwrite(&header_word,sizeof(WORD),1,f);
    //bpb - don't think Steem uses it anyway
    header_word=0x200; // RECSIZ sector size (bytes) 
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=0x2; // CLSIZ  sectors per cluster
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=0x400; // CLSIZB cluster size (bytes) 
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=0x7; // RDLEN  root dir size (sectors) 
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=0x3; // FSIZ   FAT size (sectors) 
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=0x4; // FATREC first sector of 2nd FAT (the one that is used by TOS)
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=0xE; // DATREC Number of 1st data sector FATREC + FSIZ + RDLEN
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=Sectors/2-0xE/2; // NUMCL  Total number of clusters minus DATREC (SEC - DATREC) / CLSIZ
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    header_word=0; // BFLAGS (Bit 0 is 0 for 12 bit FAT, 1 for 16 bit FAT)
    SWAP_BIG_ENDIAN_WORD(header_word);
    fwrite(&header_word,sizeof(WORD),1,f);
    break;
  }//sw
  {
    char zeros[512];
    ZeroMemory(zeros,sizeof(zeros));
    for(int track=0;track<nTracks;track++)
      for(int side=0;side<Sides;side++)
      {
        if(SSEConfig.DiskImageCreated==EXT_MSA)
        {
          WORD size=512*SecsPerTrack;
          SWAP_BIG_ENDIAN_WORD(size);
          fwrite(&size,sizeof(WORD),1,f);
        }
        for(int n=0;n<SecsPerTrack;n++) // zero disk data
          fwrite(zeros,1,512,f);
      }
  }
  WORD buf;
  fseek(f,header_size,SEEK_SET);
  fputc(0xeb,f);fputc(0x30,f);
  fseek(f,header_size+8,SEEK_SET); // Skip loader
  fputc(BYTE(rand()),f);fputc(BYTE(rand()),f);fputc(BYTE(rand()),f); //Serial number
  buf=512;
  SWAP_LITTLE_ENDIAN_WORD(buf);
  fwrite(&buf,2,1,f); //Bytes Per Sector
  buf=2;
  fwrite(&buf,1,1,f); //Sectors Per Cluster
  buf=1;
  SWAP_LITTLE_ENDIAN_WORD(buf);
  fwrite(&buf,2,1,f); //RES
  buf=2;
  fwrite(&buf,1,1,f); //FATs
  buf=112;
  SWAP_LITTLE_ENDIAN_WORD(buf);
  fwrite(&buf,2,1,f); //Dir Entries
  buf=Sectors;
  SWAP_LITTLE_ENDIAN_WORD(buf);
  fwrite(&buf,2,1,f);
  buf=249;
  fwrite(&buf,1,1,f); //Unused - MSDOS signo in fact
  // from Petari
  // 3 sectors per FAT is enough up to 1MB disk capacity
  // 5 sectors per FAT up to 1700 KB - so HD, 20 sectors per track too
  buf=(Sectors<2000)?3:5;
  SWAP_LITTLE_ENDIAN_WORD(buf);
  fwrite(&buf,2,1,f);    //Sectors Per FAT
  buf=SecsPerTrack;
  SWAP_LITTLE_ENDIAN_WORD(buf);
  fwrite(&buf,2,1,f);
  buf=Sides;
  SWAP_LITTLE_ENDIAN_WORD(buf);
  fwrite(&buf,2,1,f);
  buf=0;
  SWAP_LITTLE_ENDIAN_WORD(buf);
  fwrite(&buf,2,1,f); //Hidden Sectors
  fseek(f,header_size+510,SEEK_SET);
  fputc(0x97,f);fputc(0xc7,f); 
  fputc(0xf0,f);fputc(0xff,f);fputc(0xff,f);  // First FAT begin
  fseek(f,(Sectors<2000)?(header_size+2048):(header_size+3072),SEEK_SET); // from Petari
  fputc(0xf0,f);fputc(0xff,f);fputc(0xff,f);  // Second FAT begin
  fclose(f);
  DeleteFile(Str(STName)+".steembpb");
  return true;
#else
  FILE *f=fopen(STName,"wb");
  if(f) 
  {
    {//scope to free stack (3.6.1)
      char zeros[512];
      ZeroMemory(zeros,sizeof(zeros));
      for(int n=0;n<Sectors;n++) 
        fwrite(zeros,1,512,f);
    }//SS scope
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//LITTLE-ENDIAN ONLY             ******************************************************
    int buf;
    fseek(f,0,SEEK_SET);
    fputc(0xeb,f);fputc(0x30,f);
    fseek(f,8,SEEK_SET); // Skip loader
    fputc(BYTE(rand()),f);fputc(BYTE(rand()),f);fputc(BYTE(rand()),f); //Serial number
    buf=512;                          fwrite(&buf,2,1,f); //Bytes Per Sector
    buf=2;                            fwrite(&buf,1,1,f); //Sectors Per Cluster
    buf=1;                            fwrite(&buf,2,1,f); //RES
    buf=2;                            fwrite(&buf,1,1,f); //FATs
    buf=112;                          fwrite(&buf,2,1,f); //Dir Entries
    buf=Sectors;                      fwrite(&buf,2,1,f);
    buf=249;                          fwrite(&buf,1,1,f); //Unused - MSDOS signo in fact
    // from Petari
    // 3 sectors per FAT is enough up to 1MB disk capacity
    // 5 sectors per FAT up to 1700 KB - so HD, 20 sectors per track too
    buf=(Sectors<2000)?3:5;
    fwrite(&buf,2,1,f);    //Sectors Per FAT
    buf=SecsPerTrack;                 fwrite(&buf,2,1,f);
    buf=Sides;                        fwrite(&buf,2,1,f);
    buf=0;                            fwrite(&buf,2,1,f); //Hidden Sectors
    fseek(f,510,SEEK_SET);
    fputc(0x97,f);fputc(0xc7,f); 
    fputc(0xf0,f);fputc(0xff,f);fputc(0xff,f);  // First FAT begin
    fseek(f,(Sectors<2000)?2048:3072,SEEK_SET); // from Petari
    fputc(0xf0,f);fputc(0xff,f);fputc(0xff,f);  // Second FAT begin
    fclose(f);
    DeleteFile(Str(STName)+".steembpb");
    return true;
  }
  return 0;
#endif
}


bool TDiskManager::ToggleVisible() {
  if(HardDiskMan.IsVisible()) //SS don't quite understand, don't do it for ACSI
    HardDiskMan.Show();
  else
    IsVisible()?Hide():Show();
  return IsVisible();
}


void TDiskManager::SwapDisks(int FocusDrive) {
#ifdef WIN32
  HWND FocusTo=NULL;
  if(GetForegroundWindow()==Handle && Handle)
  {
    if(FocusDrive>-1)
      FocusTo=GetDlgItem(Handle,100+FocusDrive);
    else
      FocusTo=GetFocus();
    if(GetFocus()==GetDlgItem(Handle,100)||GetFocus()==GetDlgItem(Handle,101)) 
      SetFocus(NULL);
  }
#endif
  EasyStr DiskPath[2];
  DiskPath[0]=FloppyDrive[0].GetDisk();
  DiskPath[1]=FloppyDrive[1].GetDisk();
  EasyStr Name[2]={FloppyDisk[0].DiskName,FloppyDisk[1].DiskName};
  EasyStr DiskInZip[2]={FloppyDisk[0].DiskInZip,FloppyDisk[1].DiskInZip};
  bool HadDisk[2]={FloppyDrive[0].NotEmpty(),FloppyDrive[1].NotEmpty()};
  FloppyDrive[0].RemoveDisk();
  FloppyDrive[1].RemoveDisk();
#ifdef WIN32
  if(Handle) 
  {
    if(HadDisk[0]) 
      SendMessage(GetDlgItem(Handle,100),LVM_DELETEITEM,0,0);
    if(HadDisk[1]) 
      SendMessage(GetDlgItem(Handle,101),LVM_DELETEITEM,0,0);
  }
#endif
  if(HadDisk[1]) 
    InsertDisk(0,Name[1],DiskPath[1],0,0,DiskInZip[1]);
  if(HadDisk[0]) 
    InsertDisk(1,Name[0],DiskPath[0],0,0,DiskInZip[0]);
  WIN_ONLY(if(FocusTo) SetFocus(FocusTo); )
  UNIX_ONLY(UpdateDiskNames(0);UpdateDiskNames(1); )
}


bool TDiskManager::AreNewDisksInHistory(int d) {
  EasyStr CurrentDiskName=CreateDiskName(FloppyDisk[d].DiskName,
    FloppyDisk[d].DiskInZip);
  for(int n=0;n<10;n++) 
  {
    if(InsertHist[d][n].Path.NotEmpty()) 
    {
      EasyStr MenuItemText=CreateDiskName(InsertHist[d][n].Name,
        InsertHist[d][n].DiskInZip);
      if(NotSameStr_I(CurrentDiskName,MenuItemText))
        return true;
    }
  }
  return 0;
}


void TDiskManager::InsertHistoryAdd(int d,char *Name,char *Path,
                                    char *DiskInZip) {
  InsertHistoryDelete(d,Name,Path,DiskInZip);
  for(int n=9;n>0;n--)
    InsertHist[d][n]=InsertHist[d][n-1];
  InsertHist[d][0].Name=Name;
  InsertHist[d][0].Path=Path;
  InsertHist[d][0].DiskInZip=DiskInZip;
#ifdef WIN32
  if(Handle)
    EnableWindow(GetDlgItem(GetDlgItem(Handle,98+d),100),
      AreNewDisksInHistory(d));
#endif
}


void TDiskManager::InsertHistoryDelete(int d,char *Name,char *Path,
                                       char *DiskInZip) {
  for(int n=0;n<10;n++) 
  {
    if(IsSameStr_I(Name,InsertHist[d][n].Name)&&
      IsSameStr_I(Path,InsertHist[d][n].Path)&&
      IsSameStr_I(DiskInZip,InsertHist[d][n].DiskInZip))
      InsertHist[d][n].Path="";
  }
  for(int n=0;n<10;n++) 
  {
    bool More=0;
    for(int i=n;i<10;i++)
      if(InsertHist[d][i].Path.NotEmpty()) 
      {
        More=true;
        break;
      }
    if(More==0) 
      break;
    if(InsertHist[d][n].Path.Empty()) 
    {
      for(int i=n;i<9;i++)
        InsertHist[d][i]=InsertHist[d][i+1];
      n--;
    }
  }
}


bool TDiskManager::InsertDisk(int drive,EasyStr Name,EasyStr Path,
                        bool DontChangeDisk,bool MakeFocus,EasyStr DiskInZip,
                        bool SuppressErr,bool AllowInsert2) {
#ifdef SSE_DEBUG
  if(!DontChangeDisk)
    TRACE_LOG("%c: Inserting disk %s [%s]\n",drive+'A',Name.c_str(),Path.c_str());
#endif
  if(DontChangeDisk==0) 
  {
    if(Path.Empty())
      return 0;
    int Err=FloppyDrive[drive].SetDisk(Path,DiskInZip);
    if(Err) 
    {
      TRACE_LOG("Set Disk Error %d\n",Err);
      if(FloppyDrive[drive].Empty()) 
        EjectDisk(drive); // Update display
      if(!SuppressErr) 
      {
        switch(Err) {
        case FIMAGE_WRONGFORMAT:
          Alert(Path+": "+T("image not recognised!"),T("Disk Image Error"),MB_ICONEXCLAMATION);
          break;
        case FIMAGE_CANTOPEN:
          Alert(Path+" "+T("cannot be opened."),T("Disk Image Error"),MB_ICONEXCLAMATION);
          break;
        case FIMAGE_FILEDOESNTEXIST:
          Alert(Path+" "+T("doesn't exist!"),T("Disk Image Error"),MB_ICONEXCLAMATION);
          break;
        case FIMAGE_CORRUPTZIP:
          Alert(Path+" "+T("does not contain any files, it may be corrupt!"),T("Archive Error"),MB_ICONEXCLAMATION);
          break;
        case FIMAGE_NODISKSINZIP:
          ExtractArchiveToSTHardDrive(Path);
          break;
        case FIMAGE_DIMNOMAGIC:
          Alert(Path+" "+T("is not in the correct format, it may be corrupt!")+"\r\n\r\n"+
            T("This image has the extension DIM, unfortunately many different disk imaging programs use that extension for different disk image formats.")+" "+
            T("Sometimes DIM images are actually ST images with the incorrect extension.")+" "+
            T("You may find you can use this image by changing the extension to .st.")+"\r\n"+
            T("WARNING: Backup the disk image before you change the extension, inserting an image with the wrong extension could corrupt it."),
            T("Disk Image Error"),MB_ICONEXCLAMATION);
          break;
        case FIMAGE_DIMTYPENOTSUPPORTED:
          Alert(Path+" "+T("is in a version of the DIM format that Steem currently doesn't support.")+" "+
            T("If you have details for how to read this disk image please let us know and we'll support it in the next version."),
            T("Disk Image Error"),MB_ICONEXCLAMATION);
          break;
        case FIMAGE_IS_CONFIG:
          break;
        }
      }
      return 0;
    }
    FloppyDisk[drive].DiskName=Name;
    REFRESH_STATUS_BAR;
    InsertHistoryAdd(drive,Name,Path,DiskInZip);
    if(AllowInsert2&&!drive)
      AutoInsert2&=~2;
    if(AllowInsert2 && drive==0&&AutoInsert2) 
    {
      Err=1;
      Str NewPath=Path;
      Str NewDiskInZip=DiskInZip;
      char *dot=strrchr(NewPath,'.');
      if(dot)
      {
        // The last symbol before the . must be A and B, it will not 
        //work with (A) and (B)
        dot--;
        if(*dot=='1')
          *dot='2',Err=0;
        if(*dot=='a')
          *dot='b',Err=0;
        if(*dot=='A')
          *dot='B',Err=0;
        Str NewName=GetFileNameFromPath(NewPath);
        if(HideExtension)
        {
          dot=strrchr(NewName,'.');
          NewName=NewName.Lefts(dot-NewName.Text);
        }
        if(Err==0)
        {
          InsertDisk(1,NewName,NewPath,0,0,NewDiskInZip,true,0);
          AutoInsert2|=2; //TODO def
        }
      }
    }
  }
#ifdef WIN32
  if(Handle==NULL) 
    return true;
  HWND LV=GetDlgItem(Handle,100+drive);
  if(SendMessage(LV,LVM_GETITEMCOUNT,0,0)) 
    SendMessage(LV,LVM_DELETEITEM,0,0);
  SetDriveViewEnable(drive,true);
  if(GetForegroundWindow()==Handle && MakeFocus) 
    SetFocus(LV);
  Name=CreateDiskName(Name,DiskInZip);
  TDiskManFileInfo *Inf=new TDiskManFileInfo;
  Inf->Name=Name;
  Inf->Path=Path;
  Inf->Folder=0;
  Inf->UpFolder=0;
  Inf->ReadOnly=FloppyDisk[drive].ReadOnly;
  Inf->BrokenLink=0;
  Inf->Zip=FloppyDisk[drive].IsZip();
  LV_ITEM lvi;
  lvi.mask=LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE|LVIF_STATE;
  lvi.iItem=0;
  lvi.iSubItem=0;
  lvi.iImage=int(Inf->Zip?8:(1+FloppyDisk[drive].ReadOnly*4));
  lvi.stateMask=LVIS_SELECTED|LVIS_FOCUSED;
  lvi.state=LVIS_SELECTED|LVIS_FOCUSED;
  lvi.lParam=(LPARAM)Inf;
  lvi.pszText=Inf->Name;
  SendMessage(LV,LVM_INSERTITEM,0,(LPARAM)&lvi);
  CentreLVItem(LV,0,LVIS_SELECTED|LVIS_FOCUSED);
#endif
#ifdef UNIX
  UpdateDiskNames(drive);
#endif
  return true;
}


void TDiskManager::ExtractArchiveToSTHardDrive(Str Path) {
  if(Alert(Path+": "+T("Steem doesn't recognise any disk images.")+"\n\n"+
    T("Would you like to extract the contents of this archive to an ST hard drive?"),
    T("Extract Contents?"),MB_ICONQUESTION|MB_YESNO)==IDNO) 
    return;
  Str Name=GetFileNameFromPath(Path);
  char *dot=strrchr(Name,'.');
  if(dot) 
    *dot=0;
  Str ExtractPath;
  int d=2;
  for(;d<26;d++) 
  {
    if(mount_flag[d]) 
    {
      ExtractPath=mount_path[d];
      break;
    }
  }
  if(ExtractPath.Empty()) 
  {
    ExtractPath=WriteDir+SLASH+"st_c";
    CreateDirectory(ExtractPath,NULL);
    if(HardDiskMan.NewDrive(ExtractPath)) 
    {
      d=2;
      HardDiskMan.update_mount();
    }
    else 
    {
      Alert(T("Could not create a new hard drive."),T("Archive Error"),
        MB_ICONEXCLAMATION);
      return;
    }
  }
  Str Fol;
#ifdef WIN32
  Fol=GetUniquePath(ExtractPath,Name);
#else
  {
    if(Name.Length()>8) Name[8]=0;
    strupr(Name);
    struct stat s;
    bool first=true;
    for(;;) {
      Fol=ExtractPath+"/"+Name;
      if(stat(Fol,&s)==-1) break;
      if(first) {
        if(Name.Length()<7) Name+="_";
        if(Name.Length()<8) Name+="2";
        *Name.Right()='2';
        first=0;
      }
      else {
        (*Name.Right())++;
        if(Name.RightChar()==char('9'+1)) *Name.Right()='A';
      }
    }
  }
#endif
  CreateDirectory(Fol,NULL);
  EasyStringList sl;
  sl.Sort=eslNoSort;
  zippy.list_contents(Path,&sl,0);
  // If every file is in the same folder then strip it (stops annoying double folder)
  Str Temp=sl[0].String,FirstFol;
  for(int i=0;i<Temp.Length();i++) 
  {
    if(Temp[i]=='\\'||Temp[i]=='/') 
    {
      Temp[i+1]=0;
      FirstFol=Temp;
      break;
    }
  }
  if(FirstFol.NotEmpty()) 
  {
    for(int s=1;s<sl.NumStrings;s++) 
    {
      if(strstr(sl[s].String,FirstFol)!=sl[s].String) 
      {
        FirstFol="";
        break;
      }
    }
  }
  // Extract all files, make sure we create all necessary directories
  for(int s=0;s<sl.NumStrings;s++) 
  {
    Str Dest=Fol+SLASH+(sl[s].String+FirstFol.Length());
    UNIX_ONLY(while(strchr(Dest,'\\')) (*strchr(Dest,'\\'))='/'; )
    Str ContainingPath=sl[s].String+FirstFol.Length();
    for(int i=0;i<ContainingPath.Length();i++) 
    {
      if(ContainingPath[i]=='\\'||ContainingPath[i]=='/') 
      {
        char old=ContainingPath[i];
        ContainingPath[i]=0;
        if(GetFileAttributes(Fol+SLASH+ContainingPath)==0xffffffff)
          CreateDirectory(Fol+SLASH+ContainingPath,NULL);
        ContainingPath[i]=old;
      }
    }
    if(Dest.RightChar()!='/' && Dest.RightChar()!='\\') 
    {
      if(zippy.extract_file(Path,(DWORD)sl[s].Data[0],Dest,0,
        (DWORD)sl[s].Data[1])==ZIPPY_FAIL) {
        Alert(T("Could not extract files, this archive may be corrupt!"),
          T("Archive Error"),MB_ICONEXCLAMATION);
        return;
      }
    }
  }
  Str STFol=Str(char('A'+d))+":\\";
  DirSearch ds(Fol);
  STFol+=ds.ShortName;
  ds.Close();
  if(Alert(T("Files successfully extracted to:")+"\n\n"+
    T("PC folder")+": "+Fol+"\n"+
    T("ST folder")+": "+STFol+"\n"+
    T("Would you like to run Steem and go to the GEM desktop now?"),
    T("Files Extracted"),MB_ICONQUESTION|MB_YESNO)==IDYES)
    PerformInsertAction(2,"","","");
}


void TDiskManager::EjectDisk(int drive,bool losechanges/*=false*/) {
  FloppyDrive[drive].RemoveDisk(losechanges);
#ifdef WIN32
  if(Handle) 
  {
    SendMessage(GetDlgItem(Handle,100+drive),LVM_DELETEITEM,0,0);
    EnableWindow(GetDlgItem(GetDlgItem(Handle,98+drive),100),AreNewDisksInHistory(drive));
  }
  REFRESH_STATUS_BAR;
#endif
#ifdef UNIX
  UpdateDiskNames(drive);
#endif//win32
}


void TDiskManager::ExtractDisks(Str Path) {
  if(!enable_zip) 
    return;
  EasyStringList sl;
  sl.Sort=eslNoSort;
  zippy.list_contents(Path,&sl,true);
  if(sl.NumStrings==0)
    Alert(EasyStr(T("Cannot find a disk image in the archive"))+" "+Path,
      T("Archive Error"),MB_ICONEXCLAMATION);
  else 
  {
    EasyStr SelPath="";
    for(int s=0;s<sl.NumStrings;s++) 
    {
      int Choice=IDYES;
      EasyStr DestPath=DisksFol+SLASH+GetFileNameFromPath(sl[s].String);
      if(Exists(DestPath))
        Choice=Alert(Str(sl[s].String)+" "
          +T("already exists, do you want to overwrite it?"),
          T("Extract Disk?"),MB_ICONQUESTION|MB_YESNO);
      if(Choice==IDYES) 
      {
        if(zippy.extract_file(Path,(DWORD)sl[s].Data[0],DestPath,0,
          (DWORD)sl[s].Data[1])==ZIPPY_FAIL)
          Alert(EasyStr(T("There was an error extracting"))+" "+sl[s].String
            +" "+T("from")+" "+Path,T("ZIP Error"),MB_ICONEXCLAMATION);
        else 
          SelPath=DestPath;
      }
    }
    if(SelPath.NotEmpty()) 
      RefreshDiskView(SelPath);
  }
}


void TDiskManager::GCGetCRC(char *Path,DWORD *lpCRC,int nCRCs) {
  EasyStringList sl;
  zippy.list_contents(Path,&sl,true);
  for(int i=0;i<MIN(sl.NumStrings,nCRCs);i++)
    *(lpCRC++)=DWORD(sl[i].Data[2]);
}


BYTE* TDiskManager::GCConvertToST(char *Path,int Num,int *pLen) {
  char file[MAX_PATH];
  char real_name[MAX_PATH];
  bool del=0;
  *pLen=0;
  if(FileIsDisk(Path)==DISK_COMPRESSED) 
  {
    EasyStringList sl;
    zippy.list_contents(Path,&sl,true);
    if(Num>=sl.NumStrings) 
    {
      *pLen=-1;
      return NULL;
    }
    GetTempFileName(WriteDir,"TMP",0,file);
    zippy.extract_file(Path,(DWORD)sl[Num].Data[0],file,true,0);
    strcpy(real_name,sl[Num].String);
    del=true;
  }
  else 
  {
    if(Num>0) 
    {
      *pLen=-1;
      return NULL;
    }
    strcpy(file,Path);
    strcpy(real_name,Path);
  }
  char *ext=strrchr(real_name,'.');
  if(ext==NULL) 
  {
    if(del) 
      DeleteFile(file);
    return NULL;
  }
  BYTE *mem=NULL;
  int len;
  if(IsSameStr_I(ext,dot_ext(EXT_MSA))) 
  {
    FILE *nf=fopen(file,"rb");
    if(nf==NULL) 
    {
      if(del) 
        DeleteFile(file);
      return NULL;
    }
    WORD ID,MSA_SecsPerTrack,MSA_Sides,StartTrack,MSA_EndTrack;
    // Read header
    fread(&ID,2,1,nf);               SWAPBYTES(ID);
    fread(&MSA_SecsPerTrack,2,1,nf); SWAPBYTES(MSA_SecsPerTrack);
    fread(&MSA_Sides,2,1,nf);        SWAPBYTES(MSA_Sides);
    fread(&StartTrack,2,1,nf);       SWAPBYTES(StartTrack);
    fread(&MSA_EndTrack,2,1,nf);     SWAPBYTES(MSA_EndTrack);
    bool Err=(MSA_SecsPerTrack<1||MSA_SecsPerTrack>FLOPPY_MAX_SECTOR_NUM||
      MSA_Sides>1||StartTrack!=0||MSA_EndTrack<1
      ||MSA_EndTrack>FLOPPY_MAX_TRACK_NUM);
    if(!Err) 
    {
      *pLen=(MSA_SecsPerTrack*512)*(MSA_EndTrack+1)*(MSA_Sides+1);
      mem=(BYTE*)malloc(*pLen+16);
      // Read data
      WORD Len,NumRepeats;
      BYTE *TrackData=new BYTE[(MSA_SecsPerTrack*512)+16];
      BYTE *pDat,*pEndDat,dat;
      BYTE *pSTBuf=mem;
      for(int n=0;n<=MSA_EndTrack;n++) 
      {
        for(int s=0;s<=MSA_Sides;s++) 
        {
          Len=0;
          fread(&Len,1,2,nf); SWAPBYTES(Len);
          if(Len>MSA_SecsPerTrack*512||Len==0)
          {
            Err=true;
            break;
          }
          if(WORD(fread(TrackData,1,Len,nf))<Len)
          {
            Err=true;
            break;
          }
          if(Len==(MSA_SecsPerTrack*512)) 
          {
            memcpy(pSTBuf,TrackData,Len);
            pSTBuf+=Len;
          }
          else 
          {
            // Convert compressed MSA format track in TrackData to ST format in STBuf
            BYTE *pSTBufEnd=pSTBuf+(MSA_SecsPerTrack*512);
            pDat=TrackData;
            pEndDat=TrackData+Len;
            while(pDat<pEndDat && pSTBuf<pSTBufEnd) 
            {
              dat=*(pDat++);
              if(dat==0xE5) 
              {
                dat=*(pDat++);
                NumRepeats=*LPWORD(pDat);pDat+=2;
                SWAPBYTES(NumRepeats);
                for(int s2=0;s2<NumRepeats && pSTBuf<pSTBufEnd;s2++) 
                  *(pSTBuf++)=dat;
              }
              else
                *(pSTBuf++)=dat;
            }
          }
        }
        if(Err) 
          break;
      }
      delete[] TrackData;
    }
    fclose(nf);
    if(Err) 
    {
      free(mem);
      mem=NULL;
      *pLen=0;
    }
  }
  else if(IsSameStr_I(ext,dot_ext(EXT_DIM))) 
  {
    FILE *f=fopen(file,"rb");
    if(f) 
    {
      len=GetFileLength(f)-32;
      mem=(BYTE*)malloc(len);
      fseek(f,32,SEEK_SET);
      fread(mem,1,len,f);
      fclose(f);
      *pLen=len;
    }
  }
  if(del) 
    DeleteFile(file);
  return mem;
}


void TDiskManager::InitGetContents() { // called by Initialise()
  GetContents_GetZipCRCsProc=GCGetCRC;
  GetContents_ConvertToSTProc=GCConvertToST;
  strcpy(GetContents_ListFile,RunDir+SLASH+SSE_PLUGIN_DIR1+SLASH+DISK_IMAGE_DB);
  if(!Exists(GetContents_ListFile)) 
    strcpy(GetContents_ListFile,RunDir+SLASH+SSE_PLUGIN_DIR2+SLASH+DISK_IMAGE_DB);
  if(!Exists(GetContents_ListFile)) 
    strcpy(GetContents_ListFile,RunDir+SLASH+DISK_IMAGE_DB);
}


bool TDiskManager::GetContentsCheckExist() {
  if(Exists(GetContents_ListFile)) 
    return true;
  int i=Alert(
    T("Steem cannot find the ST disk image database, would you like to open the disk image database website now?"),
    T("Cannot Find Database"),MB_ICONQUESTION|MB_YESNO);
  if(i==IDYES) 
  {
    WIN_ONLY(ShellExecute(NULL,NULL,DIDATABASE_WEB,"","",SW_SHOWNORMAL); )
    UNIX_ONLY(shell_execute(Comlines[COMLINE_HTTP],Str("[URL]\n")+DIDATABASE_WEB); )
  }
  return 0;
}


void TDiskManager::GetContentsSL(Str Path) {
  contents_sl.DeleteAll();
  if(GetContentsCheckExist()==0) 
    return;
  char buf[1024];
  int nLinks=GetContentsFromDiskImage(Path,buf,1024,GC_ONAMBIGUITY_GUESS);
  if(nLinks>0) 
  {
    contents_sl.Sort=eslNoSort;
    contents_sl.Add(Path);
    char *p=buf;
    for(int i=0;i<nLinks;i++) 
    {
      if(p[0]==0) break;
      contents_sl.Add(p);
      p+=strlen(p)+1;
    }
  }
  else
    Alert(T("Sorry this disk image was not recognised"),
      T("Unrecognised Disk Image"),MB_ICONINFORMATION);
}


Str TDiskManager::GetContentsGetAppendName(Str TOSECName) {
  Str FirstName=TOSECName;
  char *spc=strchr(FirstName,' ');
  if(spc) 
    *spc=0;
  Str TOSEC=TOSECName;
  Str Letter;
  spc=strstr(TOSEC," of ");
  if(spc) 
  {
    while(strstr(spc+1," of ")) 
      spc=strstr(spc+1," of "); // find last " of "
    *(spc--)=0;
    for(;spc>TOSEC.Text;spc--) 
      if(spc[0]<'0'||spc[0]>'9') 
      {
        *(spc++)=0;
        break;
      }
    Letter=char('a'+atoi(spc)-1);
  }
  else 
  {
    for(int i=0;i<10;i++)
      if(strstr(TOSEC,Str("Part ")+char('A'+i))==TOSEC.Right()-5) 
        Letter=char('a'+i);
  }
  Str Number;
  for(spc=TOSEC.Text+TOSEC.Length()-1;spc>TOSEC.Text;spc--) 
  {
    if(spc[0]>='0' && spc[0]<='9') 
    {
      *(spc+1)=0;
      for(;spc>TOSEC.Text;spc--) 
        if(spc[0]<'0'||spc[0]>'9') 
        {
          Number=atoi(spc);
          break;
        }
      break;
    }
  }
  if(FirstName=="Automation") 
    FirstName="Auto";
  if(strstr(TOSEC,"Pompey Pirates")==TOSEC.Text)
    FirstName="PP";
  if(strstr(TOSEC,"Sewer Doc")==TOSEC.Text) 
    FirstName="Sewer Doc";
  if(strstr(TOSEC,"Flame of Finland")==TOSEC.Text) 
    FirstName="FOF";
  if(strstr(TOSEC,"Persistance of Vision")==TOSEC.Text) 
    FirstName="POV";
  if(strstr(TOSEC,"ST Format")==TOSEC.Text) 
    FirstName="STF";
  if(strstr(TOSEC,"Bad Brew Crew")==TOSEC.Text) 
    FirstName="BBC";
  Str ShortName=FirstName;
  if(Number.NotEmpty()) 
    ShortName+=Str(" ")+Number+Letter;
  return ShortName;
}
