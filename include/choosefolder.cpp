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
FILE: choosefolder.cpp
DESCRIPTION: Code for letting the user pick a folder.
---------------------------------------------------------------------------*/

#include <shlobj.h>
#include "mymisc.h"

#ifndef NO_SLASH
#define CHOOSEFOLDER_UNDEF_NO_SLASH
#define NO_SLASH(c) if(c[0]) if (c[strlen(c)-1]=='\\' || c[strlen(c)-1]=='/') c[strlen(c)-1]=0;
#endif

#pragma warning (disable : 4996)

int CALLBACK ChooseFolder_BrowseCallbackProc(HWND Win,UINT Mess,LPARAM,
                                        LPARAM lParUser) {
  switch(Mess) {
  case BFFM_INITIALIZED:
  {
    RECT box;
    POINT pt;
    HWND TreeView;
    char Path[MAX_PATH+1];
    int Len;
    // Centre dialog
    GetWindowRect(Win,&box);
    box.right-=box.left;
    box.bottom-=box.top;
    SetWindowPos(Win,0,(GetSystemMetrics(SM_CXSCREEN)/2)-(box.right/2),
      (GetSystemMetrics(SM_CYSCREEN)/2)-(box.bottom/2),
      0,0,SWP_NOSIZE|SWP_NOZORDER);
    // Select passed folder
    strcpy(Path,(char*)lParUser);
    NO_SLASH(Path);
    Len=(int)strlen(Path);
    if(Path[Len-1]==':')
    {
      Path[Len++]=SLASHCHAR;
      Path[Len]=0;
    }
    SendMessage(Win,BFFM_SETSELECTION,TRUE,(LPARAM)Path);
    // Change Tree to make it work better
    pt.x=box.right/2;
    pt.y=box.bottom/2;
    TreeView=ChildWindowFromPoint(Win,pt);
    SetWindowLong(TreeView,GWL_STYLE,GetWindowLong(TreeView,GWL_STYLE)
      |TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP);
    break;
  }//case
  }//sw
  return 0;
}


EasyStr ChooseFolder(HWND Win,char *Title,char *FolToSel) {
  //ITEMIDLIST *RetIDL,*RootIDL=NULL;//warning C4090: '=': different '__unaligned' qualifiers
  LPITEMIDLIST RetIDL,RootIDL=NULL;
  char FolName[MAX_PATH+1];
  IMalloc *Mal;
  BROWSEINFO bi;
  /* TODO
  SHGetMalloc was introduced in Windows 95 and Microsoft Windows NT 4.0, but 
  as of Windows 2000 it is no longer necessary. In its place, programs can 
  call the equivalent (and easier to use) CoTaskMemAlloc and CoTaskMemFree.
  If you find an older reference document that suggests or even requires the
  use of SHGetMalloc, it is acceptable and encouraged to use CoTaskMemAlloc
  and CoTaskMemFree instead.
  
  https://docs.microsoft.com/en-us/windows/desktop/api/shlobj_core/nf-shlobj_core-shgetmalloc
  
  */
  SHGetMalloc(&Mal);
  bi.hwndOwner=Win;
  bi.pidlRoot=RootIDL;              //Directory to start from (NULL=desktop)
  bi.pszDisplayName=FolName;        //Doesn't return full path
  bi.lpszTitle=Title;
  bi.ulFlags=BIF_RETURNONLYFSDIRS;  //No false folders (like DUN)
  bi.lpfn=ChooseFolder_BrowseCallbackProc; // Function to handle various notification
  bi.lParam=(LPARAM)FolToSel;         // What to call that func with
  bi.iImage=0;
  RetIDL=SHBrowseForFolder(&bi);
  if(RetIDL==NULL) return "";
  EasyStr SelFol;
  SelFol.SetLength(MAX_PATH);
  SHGetPathFromIDList(RetIDL,SelFol);
  NO_SLASH(SelFol);
  Mal->Free(RetIDL);
  return SelFol;
}

#ifdef CHOOSEFOLDER_UNDEF_NO_SLASH
#undef CHOOSEFOLDER_UNDEF_NO_SLASH
#undef NO_SLASH
#endif

