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
FILE: stemdialogs.h
DESCRIPTION: Declarations for Steem's Dialog base class.
class TStemDialog
---------------------------------------------------------------------------*/

#pragma once
#ifndef STEMDIALOGS_DECLA_H
#define STEMDIALOGS_DECLA_H

#include <configstorefile.h>

#ifdef WIN32
#include <CommCtrl.h> //HTREEITEM
#include <directory_tree.h>
#endif
#ifdef UNIX
#include <x/hxc.h>
#endif

#include <parameters.h>

#define MAX_DIALOGS 20
#define SD_REGISTER 0
#define SD_UNREGISTER 1


#pragma pack(push, 8)

class TStemDialog {
private:
public:
  TStemDialog();
  ~TStemDialog(){};
  void LoadPosition(TConfigStoreFile*),SavePosition(bool,TConfigStoreFile*),SaveVisible(TConfigStoreFile*);
  EasyStr Section;
#ifdef WIN32
  void CheckFSPosition(HWND);
  void RegisterMainClass(WNDPROC,char*,int);
  static LRESULT DefStemDialogProc(HWND,UINT,WPARAM,LPARAM);
  void MakeParent(HWND),UpdateMainWindowIcon();
  void ChangeParent(HWND);
  bool IsVisible(){return Handle!=NULL;}
  bool HasHandledMessage(MSG *);
  bool HandleIsInvalid();

  HTREEITEM AddPageLabel(char *,int);
  void DestroyCurrentPage();
  void GetPageControlList(DynamicArray<HWND> &);
  void ShowPageControls(),SetPageControlsFont();

  void UpdateDirectoryTreeIcons(DirectoryTree*);

  HWND Handle,Focus,PageTree;
  HFONT Font;
  int nMainClassIcon;
#endif
#ifdef UNIX
  bool IsVisible(){ return Handle!=0;}
  Pixmap IconPixmap;
  Pixmap IconMaskPixmap;
  void StandardHide();
  bool StandardShow(int w,int h,char* name,
      int icon_index,long input_mask,LPWINDOWPROC WinProc,bool=false);
  Window Handle;
#endif
  int Left,Top;
  int FSLeft,FSTop;
};

#pragma pack(pop)


extern TStemDialog *DialogList[MAX_DIALOGS];
extern int nStemDialogs;

#ifdef WIN32
extern bool StemDialog_RetDefVal;
// For some reason in 24-bit and 32-bit screen modes on XP ILC_COLOR24 and
// ILC_COLOR32 icons don't highlight properly, have to be 16-bit.
extern const UINT BPPToILC[5];
#endif

#endif//#ifndef STEMDIALOGS_DECLA_H
