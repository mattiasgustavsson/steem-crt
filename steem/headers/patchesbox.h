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
FILE: patchesbox.h
DESCRIPTION: Declarations for Steem's patches dialog that allows the user to
apply patches to fix ST programs that don't work or are incompatible with
Steem. Patches can also be used to defeat a game protection.
class TPatchesBox
---------------------------------------------------------------------------*/

#pragma once
#ifndef PATCHES_BOX_H
#define PATCHES_BOX_H

#include <stemdialogs.h>
#include <parameters.h>

#pragma pack(push, 8)

class TPatchesBox : public TStemDialog {
private:
  void GetPatchText(char*,Str[4]);
#ifdef WIN32
  static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
  void ManageWindowClasses(bool);
#endif
#ifdef UNIX
  static int WinProc(TPatchesBox*,Window,XEvent*);
	static int ListviewNotifyHandler(hxc_listview*,int,int);
	static int ButtonNotifyHandler(hxc_button*,int,int[]);

  hxc_button PatchLabel,DescLabel,ApplyWhenLabel,VersionLabel,AuthorLabel;
  hxc_listview PatchList;
  hxc_button ApplyBut;
	hxc_textdisplay DescText,ApplyWhenText,VersionText,AuthorText;
  hxc_button PatchDirLabel,PatchDirText,PatchDirBut;
#endif
public:
  TPatchesBox();
  ~TPatchesBox() { Hide(); }
  void Show(),Hide();
  bool ToggleVisible(){ IsVisible() ? Hide():Show();return IsVisible(); }
  bool LoadData(bool,TConfigStoreFile*,bool* = NULL),SaveData(bool,TConfigStoreFile*);

  EasyStr SelPatch,PatchDir;

  void PatchPoke(MEM_ADDRESS &,int,DWORD);
  void RefreshPatchList();
  void ShowPatchFile();
  void ApplyPatch();

#ifdef WIN32
  EasyStr GetPatchVersion();
  void SetButtonIcon();
#endif
};

extern TPatchesBox PatchesBox;

#pragma pack(pop)

#endif//#ifndef PATCHES_BOX_H
