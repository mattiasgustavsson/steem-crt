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

DOMAIN: Various
FILE: mymisc.h
DESCRIPTION: Many miscellaneous functions from all areas of programming that
just refuse to be categorized.
struct TWinPositionData, TWidthHeight, TModifierState
---------------------------------------------------------------------------*/

#pragma once
#ifndef MYMISC_H
#define MYMISC_H

#include <stdio.h>
#ifdef WIN32
#include <Windows.h>
#include <io.h>
#define WIN_ONLY(a) a
#define UNIX_ONLY(a)
#define SLASH "\\"
#define SLASHCHAR '\\'

#include <time.h>
#include <CommCtrl.h>
//#include <ShObjIdl.h>
#include <ShlObj.h>
//#include <ShlGuid.h>

#endif
#ifdef UNIX
#ifndef WIN_ONLY
#define WIN_ONLY(a)
#endif
#define UNIX_ONLY(a) a
#define SLASH "/"
#define SLASHCHAR '/'
typedef Window WINDOWTYPE;
#endif

#ifdef MINGW_BUILD
#define NULL 0
#endif

//#include <ShObjIdl.h>
//#include 
//#define _INC_TIME
//#define _INC_COMMCTRL
//#define _SHLOBJ_H_



#include "easystr.h"
/*
#ifndef SLASH
#ifdef WIN32
#define SLASH "\\"
#else
#define SLASH "/"
#endif
#endif
*/

#define NO_SLASH(path) if (path[0]) \
                         if (path[strlen(path)-1]=='/' \
                   WIN_ONLY( || path[strlen(path)-1]=='\\' )  \
                            ) path[strlen(path)-1]=0;

#define CenterWindow CentreWindow

#define bound(Val,Min,Max) ( ((Val)<(Min))?(Min):( ((Val)>(Max))?(Max):(Val) ) )

#define REMOVE_LAST_SLASH true
#define REMOVE_SLASH true
#define WITH_SLASH 0
#define KEEP_SLASH 0
#define WITHOUT_SLASH true

#pragma pack(push, 8)

struct TWinPositionData {
  int Left,Top;
  int Width,Height;
  bool Maximized,Minimized;
};


struct TWidthHeight {
  LONG Width;
  LONG Height;
};

#pragma pack(pop)


unsigned long HexToVal(char *);
#ifdef _WIN64
unsigned long long HexToVall(char *);
#endif

bool LoadBool(FILE *f);
void SaveBool(bool b,FILE *f);
//int LoadInt(FILE *f);
//void SaveInt(int i,FILE *f);
void LoadChars(char *buf,FILE *f);
void SaveChars(char *buf,FILE *f);
long GetFileLength(FILE *f);
char *GetFileNameFromPath(char *);
void RemoveFileNameFromPath(char *fil,bool rem);
bool has_extension_list(char *Filename,char *Ext,...);
bool has_extension(char *Filename,char *Ext);
bool MatchesAnyString(char *StrToCompare,char *Str,...);
bool MatchesAnyString_I(char *StrToCompare,char *Str,...);
#if defined(_INC_TIME) || defined(_SYS_TIME_H)
DWORD TMToDOSDateTime(struct tm *lpTime);
#endif
extern int log_to_base_2(unsigned long x);

#ifdef EASYSTR_H
extern EasyStr GetUniquePath(EasyStr path,EasyStr name);
#endif

#pragma pack(push, 8)

struct TModifierState {
  bool LShift,RShift;
  bool LCtrl,RCtrl;
  bool LAlt,RAlt;
};

#pragma pack(pop)

#ifdef WIN32
#define SetPropI(w,s,dw) SetProp(w,s,(HANDLE)(dw))
#define GetPropI(w,s) DWORD(GetProp(w,s))
void RemoveProps(HWND Win,char *Prop1,...);
void Border3D(HDC dc,int x,int y,int w,int h,
              DWORD col0,DWORD col1,DWORD col2,DWORD col3);
void Box3D(HDC dc,int x,int y,int w,int h,bool d);
void CentreTextOut(HDC dc,int x,int y,int w,int h,
    char *text,int len);
#ifdef MINGW_BUILD
#undef GetLongPathName
#endif
void GetLongPathName(char *src,char *dest,int maxlen);
void SetWindowAndChildrensFont(HWND Win,HFONT fnt);
void RemoveAllMenuItems(HMENU),DeleteAllMenuItems(HMENU);
void CentreWindow(HWND Win,bool Redraw);
#define RegValueExists(Key,Name) (RegQueryValueEx(Key,Name,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
bool RegKeyExists(HKEY Key,char *Name);
bool WindowIconsAre256();
void DisplayLastError(char *TitleText=NULL);
HFONT MakeFont(char *Typeface,int Height,int Width=0,int Boldness=FW_NORMAL,
                        bool Italic=0,bool Underline=0,bool Strikeout=0);
COLORREF GetMidColour(COLORREF RGB1,COLORREF RGB2);
COLORREF DimColour(COLORREF Col,double DimAmount);
#ifndef DSTRING_H
//#ifdef EASYSTR_H
EasyStr GetCurrentDir();
EasyStr GetEXEDir();
EasyStr GetEXEFileName();
//#endif
#endif
bool GetWindowPositionData(HWND Win,TWinPositionData *wpd);
//#ifdef EASYSTR_H
EasyStr GetPPEasyStr(char *SectionName,char *KeyName,char *Default,char *FileName);
EasyStr FileSelect(HWND Owner,char *Title,char *DefaultDir,char *Types,int InitType,int LoadFlag,EasyStr DefExt="",char *DefFile="");
#ifdef _SHLOBJ_H_
EasyStr GetLinkDest(EasyStr LinkFileName,WIN32_FIND_DATA *wfd,HWND UIParent=NULL,
                     IShellLink *Link=NULL,IPersistFile* File=NULL);
HRESULT CreateLink(char *LinkFileName,char *TargetFileName,char *Description=NULL,
                    IShellLink *Link=NULL,IPersistFile* File=NULL,
                    char *IconPath=NULL,int IconIdx=0,
                    bool NoOverwrite=0);
#endif
//#endif
void DeleteDirAndContents(char *Dir);
#if defined(_INC_COMMCTRL) || defined(MINGW_BUILD)
void CentreLVItem(HWND LV,int iItem,LRESULT State=-1);
void GetTabControlPageSize(HWND Tabs,RECT *rc);
#endif


TWidthHeight GetTextSize(HFONT Font,char *Text);
TWidthHeight GetCheckBoxSize(HFONT Font=NULL,char *Text=NULL);
#ifdef _INC_TOOLHELP32
typedef bool (WINAPI *LPTOOLHELPMODULEWALK)(HANDLE,LPMODULEENTRY32);
typedef HANDLE (WINAPI *LPTOOLHELPCREATESNAPSHOT)(DWORD,DWORD);
void GetWindowExePaths(HWND Win,char *Buf,int BufLen);
#endif
char *RemoveIllegalFromPath(char *Path,bool DriveIncluded,bool RemoveWild=true,char ReplaceChar='-',bool STPath=0);
char *RemoveIllegalFromName(char *Name,bool RemoveWild=true,char ReplaceChar='-');
LPARAM lParamPointsToParent(HWND Win,LPARAM lPar);
LRESULT CBAddString(HWND Combo,char *String);
LRESULT CBAddString(HWND Combo,char *String,LONG_PTR Data);
LRESULT CBFindItemWithData(HWND Combo,LONG_PTR Data);
LRESULT CBSelectItemWithData(HWND Combo,LONG_PTR Data);
LRESULT CBGetSelectedItemData(HWND Combo);
void MoveWindowClient(HWND Win,int x,int y,int w,int h);
void GetWindowRectRelativeToParent(HWND Win,RECT *pRc);
#define IsCachedPrivateProfile() (0)
#define UnCachePrivateProfile()
#define CachePrivateProfile(a)


#ifdef _INC_COMMCTRL
extern void ToolsDeleteWithIDs(HWND,HWND,DWORD,...);
extern void ToolsDeleteAllChildren(HWND,HWND);
extern void ToolAddWindow(HWND,HWND,char*);

extern HTREEITEM TreeSelectItemWithData(HWND,long,HTREEITEM=TVI_ROOT);
extern int TreeGetMaxItemWidth(HWND,HTREEITEM=TVI_ROOT,int=0);

#define PAD_ALIGN_CENTRE b0000
#define PAD_ALIGN_LEFT   b0001
#define PAD_ALIGN_RIGHT  b0010
#define PAD_ALIGN_TOP    b0100
#define PAD_ALIGN_BOTTOM b1000
#define PAD_NAMES    b10000000

extern void ImageList_AddPaddedIcons(HIMAGELIST,int,char *,...);
extern void ImageList_AddPaddedIcons(HIMAGELIST,int,int,...);
extern void ImageList_AddPaddedIcons(HIMAGELIST,int,HICON,...);

extern int LVGetSelItem(HWND);
#ifdef EASYSTR_H
EasyStr LVGetItemText(HWND,int);
#endif

#endif

#ifdef EASYSTR_H
extern EasyStr GetWindowTextStr(HWND Win);
extern EasyStr LoadWholeFileIntoStr(char *File);
extern bool SaveStrAsFile(EasyStr &s,char *File);
extern EasyStr ShortenPath(EasyStr,HFONT,int);
#endif

extern TModifierState GetLRModifierStates();

extern HDC CreateScreenCompatibleDC();
extern HBITMAP CreateScreenCompatibleBitmap(int,int);

#else

#include "notwin_mymisc.h"

#endif//WIN32

#ifdef UNIX
#include "x/x_mymisc.h"
#endif

#ifdef BEOS
#include "beos/be_mymisc.h"
#endif

extern bool no_ints;

inline int abs_quicki(int i) {
  if(i>=0) 
    return i;
  return -i;
}


inline COUNTER_VAR abs_quick(COUNTER_VAR i) {
  if(i>=0) 
    return i;
  return -i;
}

#endif//#ifndef MYMISC_H
