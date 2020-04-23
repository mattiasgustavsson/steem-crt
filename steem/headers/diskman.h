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
FILE: diskman.h
DESCRIPTION: Declarations for Steem's Disk Manager.
struct TDiskManager, TDiskManFileInfo
---------------------------------------------------------------------------*/

#pragma once
#ifndef DISKMAN_DECLA_H
#define DISKMAN_DECLA_H

#include <stemdialogs.h>
#include <floppy_disk.h>
#ifdef SSE_UNIX
#include <x/hxc_dir_lv.h>
#endif


#define FileIsDisk(s) ExtensionIsDisk(strrchr(s,'.'))

int ExtensionIsDisk(char *TestedExt);
bool ExtensionIsPastiDisk(char *Ext);


#pragma pack(push, 8)

struct TDiskManFileInfo {
  EasyStr Name,Path,LinkPath;  
  int Image;
  bool UpFolder,Folder,ReadOnly,BrokenLink,Zip;

// Could implement the next few, in a million years!
//
//  EasyStr IconPath;int IconIdx;
//  EasyStr Description
};

#define DISKVIEWSCROLL_TIMER_ID 1
#define MSACONV_TIMER_ID 2

struct TDiskManager : public TStemDialog {
  // FUNCTION
  void PerformInsertAction(int,EasyStr,EasyStr,EasyStr);
  void ExtractArchiveToSTHardDrive(Str);
  static void GCGetCRC(char*,DWORD*,int);
  static BYTE* GCConvertToST(char *,int,int *);
  void GetContentsSL(Str);
  bool GetContentsCheckExist();
  Str GetContentsGetAppendName(Str);
#ifdef WIN32
  static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT CALLBACK Drive_Icon_WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT CALLBACK DiskView_WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT CALLBACK DriveView_WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT CALLBACK Dialog_WndProc(HWND,UINT,WPARAM,LPARAM);
  static int CALLBACK CompareFunc(LPARAM,LPARAM,LPARAM);
  void BeginDrag(int,HWND),MoveDrag(),EndDrag(int,int,bool);
  bool DoCreateMultiLinks();
  void AddFoldersToMenu(HMENU,int,EasyStr,bool);
  bool MoveOrCopyFile(bool,char*,char*,char*,bool);
  void PropShowFileInfo(int);
  void AddFileOrFolderContextMenu(HMENU,TDiskManFileInfo *);
  void UpdateBPBFiles(Str,Str,bool);
  void ManageWindowClasses(bool);
  Str GetMSAConverterPath();
  void GoToDisk(Str,bool);
#ifndef SSE_NO_WINSTON_IMPORT
  HRESULT CreateLinkCheckForOverwrite(char *,char *,IShellLink *,IPersistFile *);
  bool ImportDiskExists(char *,EasyStr &),DoImport();
  void ShowImportDiag(); //public
#endif
#endif 
#ifdef UNIX
  static int WinProc(TDiskManager*,Window,XEvent*);
  void set_path(EasyStr,bool=true,bool=true);
  void UpdateDiskNames(int);
  void ToggleReadOnly(int);
  Str GetCustomDiskImage(int*,int*,int*);
  void set_home(Str);
  static int dir_lv_notify_handler(hxc_dir_lv*,int,int);
  static int button_notify_handler(hxc_button*,int,int*);
	static int menu_popup_notifyproc(hxc_popup*,int,int);
  static int diag_lv_np(hxc_listview *,int,int);
  static int diag_but_np(hxc_button *,int,int*);
  static int diag_ed_np(hxc_edit *,int,int);
  void RefreshDiskView(Str=""); //public
#endif
  TDiskManager();
  ~TDiskManager();
  void Show(),Hide();
  bool ToggleVisible();
  bool LoadData(bool,TConfigStoreFile*,bool* = NULL),SaveData(bool,TConfigStoreFile*);
  void SwapDisks(int);
  bool InsertDisk(int drive,EasyStr Name,EasyStr Path,bool DontChangeDisk=false,
    bool MakeFocus=true,EasyStr DiskInZip="",bool SuppressErr=false,
    bool AllowInsert2=false);
  void EjectDisk(int drive,bool losechanges=false);
  bool AreNewDisksInHistory(int);
  void InsertHistoryAdd(int,char *,char *,char* = "");
  void InsertHistoryDelete(int,char *,char *,char* = "");
  bool CreateDiskImage(char *STName,WORD Sectors,WORD SecsPerTrack,WORD Sides);
  EasyStr CreateDiskName(char *,char *);
  void SetNumFloppies(BYTE NewNum);
  void ExtractDisks(Str);
  void InitGetContents();
  void ShowDatabaseDiag(),ShowContentDiag();

#ifdef WIN32
  bool HasHandledMessage(MSG*); // public
  void SetDir(EasyStr,bool,EasyStr="",bool=0,EasyStr="",int=0);
  bool SelectItemWithPath(char *,bool=0,char* = NULL);
  bool SelectItemWithLinkPath(char *LinkPath,bool EditLabel=false) {
    return SelectItemWithPath(NULL,EditLabel,LinkPath);
  }
  void RefreshDiskView(EasyStr SelPath="",bool EditLabel=0,
    EasyStr SelLinkPath="",int iItem=0);
  int GetSelectedItem();
  TDiskManFileInfo *GetItemInf(int iItem,HWND LV=NULL); //v402 not inline
  void ShowLinksDiag(),ShowPropDiag(),ShowDiskDiag();
  int GetDiskSelectionSize();
  void SetDiskViewMode(int);
  void LoadIcons();
  void SetDriveViewEnable(int,bool);
  HWND VisibleDiag() { // used by HasHandledMessage()
    return (HWND)((DWORD_PTR)DiskDiag|(DWORD_PTR)LinksDiag|(DWORD_PTR)PropDiag
#if !defined(SSE_NO_WINSTON_IMPORT)
      |(DWORD_PTR)ImportDiag
#endif
      |(DWORD_PTR)ContentDiag|(DWORD_PTR)DatabaseDiag);
  }
#endif
  //DATA
  EasyStr HistBack[10],HistForward[10];
  EasyStr SaveSelPath;
  EasyStr ContentsLinksPath;
  EasyStr DisksFol,HomeFol,ContentListsFol;
  EasyStr QuickFol[10];
  struct {
    EasyStr Name,Path,DiskInZip;
  }InsertHist[2][10];
  TDiskManFileInfo PropInf;
  bool HideBroken,CloseAfterIRR,HideExtension,ShowHiddenFiles;
  bool Maximized,FSMaximized;
  BYTE SmallIcons,AutoInsert2;
  bool EjectDisksWhenQuit;
  EasyStringList contents_sl;
  TBpbInfo bpbi,file_bpbi,final_bpbi;
  int IconSpacing, DoubleClickAction,ContentConflictAction,SaveScroll;
  int Width,Height,FSWidth,FSHeight;
  WORD SecsPerTrackIdx,TracksIdx,SidesIdx; // Idx = actual numbers now
#ifdef WIN32
  EasyStr MultipleLinksPath,LinksTargetPath;
  WNDPROC Old_ListView_WndProc;
  HIMAGELIST il[2];
  HWND DragLV;
  HIMAGELIST DragIL;
  HWND DiskView;
  HICON DriveIcon[2],AccurateFDCIcon,DisableDiskIcon;
  HWND DatabaseDiag,ContentDiag,DiskDiag,LinksDiag,PropDiag,DiagFocus;
  HANDLE MSAConvProcess;
  int Dragging,DragWidth,DragHeight,DropTarget;
  int LastOverID;
  int MenuTarget;  
  Str MSAConvPath,MSAConvSel;
  Str DatabaseFind;
  EasyStringList MenuESL;
  bool DragEntered,EndingDrag;
  bool AtHome;
  bool ExplorerFolders;
  bool DoExtraShortcutCheck;
#if !defined(SSE_NO_WINSTON_IMPORT)
  HWND ImportDiag;
  bool Importing;
  EasyStr WinSTonPath,WinSTonDiskPath,ImportPath;
  bool ImportOnlyIfExist;
  int ImportConflictAction;
#endif
#endif//WIN32
#ifdef UNIX
  int HistBackLength,HistForwardLength;
  int ArchiveTypeIdx;
  bool TempEject_InDrive[2];
  Str TempEject_Name,TempEject_DiskInZip[2];
  hxc_dir_lv dir_lv;
  hxc_button UpBut,BackBut,ForwardBut,eject_but[2];
  hxc_button DirOutput,disk_name[2],drive_icon[2];
  hxc_button HomeBut,SetHomeBut,MenuBut;
  hxc_button HardBut;
#endif//UNIX
};

extern TDiskManager DiskMan; // singleton

#pragma pack(pop)

#endif//DISKMAN_DECLA_H
