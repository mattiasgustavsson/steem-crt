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
FILE: directory_tree.h
DESCRIPTION: Declarations for the directory tree feature of Steem.
class DirectoryTree
---------------------------------------------------------------------------*/

#pragma once
#ifndef DIRECTORY_TREE_H
#define DIRECTORY_TREE_H

#include <Windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <easystr.h>
#include <easystringlist.h>
#include <mymisc.h>


#define DTM_SELCHANGED 0
#define DTM_NAMECHANGED 1
#define DTM_TYPECHANGED 2
#define DTM_GETTYPE 3
#define DTM_FOLDERMOVED 4
#define DTM_ITEMDELETED 5

class DirectoryTree;

typedef int(*PDTNOTIFYPROC)(DirectoryTree*,void*,int,INT_PTR,INT_PTR);

class DirectoryTree {
private:
  static LPARAM CALLBACK TVWndProc(HWND,UINT,WPARAM,LPARAM);
  bool FindFilesAndFolders(EasyStr,EasyStringList&,EasyStringList&,bool);
  void PutDirectoryInTree(HTREEITEM,EasyStr);
  bool ItemIsChild(HTREEITEM,HTREEITEM);
  HTREEITEM InsertItem(char *,HTREEITEM,HTREEITEM,int,bool=0,LPARAM=0,UINT=0);
  bool DeleteItem(HTREEITEM);
  bool DeleteChildrenOfItem(HTREEITEM);
  HTREEITEM MoveItems(HTREEITEM,HTREEITEM,bool,EasyStr="");
  HTREEITEM CopyItemAndChildren(HTREEITEM,HTREEITEM,HTREEITEM,HTREEITEM *);
  void DragEnd(bool),DragMove();
  HTREEITEM MenuItem,DragItem,DropTarget,CurrentItem;
  HIMAGELIST DragIL,il;
  bool DragEntered;
  int DragButton;
  bool DoNotifyOnChange,ExpandTimerActive;
  int DropPos;
  WNDPROC OldTVWndProc;
public:
  DirectoryTree();
  ~DirectoryTree();
  bool Create(HWND Par,int x,int y,int w,int h,HMENU ID,DWORD Flags,
      PDTNOTIFYPROC NP,void* That,EasyStr RF,EasyStr RN,bool CO=0);
  void Destroy();
  bool ProcessMessage(UINT,WPARAM,LPARAM);
  void RefreshDirectory();
  TV_ITEM GetItem(HTREEITEM,int,char* = NULL,int=0);
  EasyStr GetItemPath(HTREEITEM);
  HTREEITEM NewItem(EasyStr,HTREEITEM,int,bool=true);
  HTREEITEM SelectItemByPath(EasyStr);
  LRESULT SendTreeMessage(UINT m,WPARAM w,LPARAM l);
  void EnsureItemExpanded(HTREEITEM Item);
  void ReloadIcons(UINT);
  HWND hTree,hParent;
  LRESULT WndProcRet;
  HMENU TreeID;
  PDTNOTIFYPROC NotifyProc;
  void *NotifyThis;
  int IDBase;
  bool ChooseOnly,AllowTypeChange;
  HTREEITEM RootItem;
  EasyStr RootFol;
  EasyStringList FileMasksESL;
  static HWND PopupParent;
};

#endif//#ifndef DIRECTORY_TREE_H

