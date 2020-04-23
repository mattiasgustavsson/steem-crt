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
FILE: gui.cpp
DESCRIPTION: This is a core file that has lots and lots of miscellaneous
GUI (Graphic User Interface) functions. 
It creates the main window in MakeGUI.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <computer.h>
#include <draw.h>
#include <palette.h>
#include <osd.h>
#include <gui.h>
#include <screen_saver.h>
#include <mem_browser.h>
#include <diskman.h>
#include <harddiskman.h>
#include <options.h>
#include <stjoy.h>
#include <shortcutbox.h>
#include <patchesbox.h>
#include <infobox.h>
#include <translate.h>
#include <macros.h>
#include <debugger.h>
#include <debugger_trace.h>
#include <dir_id.h>
#include <mymisc.h>
#include <loadsave.h>
#include <mr_static.h>
#include <dwin_edit.h>
#include <key_table.h>
#include <stdarg.h>

int DoSaveScreenShot=(0);
bool ResChangeResize=(true),CanUse_400=(0);
bool bAppActive=(true),bAppMinimized=(0);
DWORD DisableDiskLightAfter=(3000);
LANGID KeyboardLangID=(0);
int stem_mousemode=(STEM_MOUSEMODE_DISABLED);
int window_mouse_centre_x,window_mouse_centre_y;
bool TaskSwitchDisabled=(0);
Str ROMFile,CartFile;
bool FSQuitAskFirst=(true),Quitting=(0);
bool FSDoVsync=(0);
int ExternalModDown=(0);
bool comline_allow_LPT_input=(0);
BYTE KeyDownModifierState[256];
int PasteVBLCount=0,PasteSpeed=2;
Str PasteText;
bool StartEmuOnClick=0;

TStatusInfo StatusInfo;

#ifdef WIN32

bool RunMessagePosted=false;

TSystemMetrics GuiSM;

void TSystemMetrics::Update() {
#if defined(SSE_VID_2SCREENS)
   Disp.CheckCurrentMonitorConfig(); // Update monitor rectangle
   m_cx_screen=Disp.rcMonitor.right-Disp.rcMonitor.left; // maybe!
   m_cy_screen=Disp.rcMonitor.bottom-Disp.rcMonitor.top;
#else
  m_cx_screen=GetSystemMetrics(SM_CXSCREEN);
  m_cy_screen=GetSystemMetrics(SM_CYSCREEN);
#endif
  m_cx_frame=GetSystemMetrics(SM_CXFRAME);
  m_cy_frame=GetSystemMetrics(SM_CYFRAME);
#ifdef SSE_X64_GUI //?
  m_cy_frame+=4;
  m_cx_frame+=4;
#endif
  m_cy_caption=GetSystemMetrics(SM_CYCAPTION);
  m_cx_vscroll=GetSystemMetrics(SM_CXVSCROLL);
}


HICON hGUIIcon[RC_NUM_ICONS],hGUIIconSmall[RC_NUM_ICONS];
HWND StemWin=NULL,ParentWin=NULL,ToolTip=NULL,DisableFocusWin=NULL,UpdateWin=NULL;
HMENU StemWin_SysMenu=NULL;
#if defined(SSE_GUI_ALT_MENU)
HMENU StemWin_AltMenu=NULL;
HMENU StemWin_AltMenuFile=NULL;
HMENU StemWin_AltMenuEmu=NULL;
HMENU StemWin_AltMenuTools=NULL;
#endif

#if defined(SSE_GUI_MENU)
HMENU StemWin_AltMenuStatus=NULL;
#endif

HFONT fnt;

HFONT hSteemGuiFont=NULL;

HCURSOR PCArrow;
COLORREF MidGUIRGB,DkMidGUIRGB;
HANDLE SteemRunningMutex=NULL;
bool WinNT=0;
bool AllowTaskSwitch = NOT_ONEGAME(true) ONEGAME_ONLY(0);
HHOOK hNTTaskSwitchHook=NULL;
HWND NextClipboardViewerWin=NULL;
char status_bar_text[256];

#endif//WIN32


#ifdef UNIX

XErrorEvent XError;
hxc_popup pop;
hxc_popuphints hints;
Window StemWin=0;
GC DispGC=0;
Cursor EmptyCursor=0;
Atom RunSteemAtom,LoadSnapShotAtom;
XID SteemWindowGroup = 0;
DWORD BlackCol=0,WhiteCol=0,BkCol=0,BorderLightCol,BorderDarkCol;
hxc_alert alert;
short KeyState[256]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
extern "C" LPBYTE Get_icon16_bmp(),Get_icon32_bmp(),Get_icon64_bmp(),Get_tos_flags_bmp();
IconGroup Ico16,Ico32,Ico64,IcoTOSFlags;
Pixmap StemWinIconPixmap=0,StemWinIconMaskPixmap=0;
hxc_button RunBut,FastBut,ResetBut,SnapShotBut,ScreenShotBut,PasteBut,FullScreenBut;
hxc_button InfBut,PatBut,CutBut,OptBut,JoyBut,DiskBut;
DWORD ff_doubleclick_time=0;
hxc_fileselect fileselect;
char* Comlines_Default[NUM_COMLINES][8]={
        {"netscape \"[URL]\"","konqueror \"[URL]\"","galeon \"[URL]\"","opera \"[URL]\"","firefox \"[URL]\"","mozilla \"[URL]\"",NULL},
        {"netscape \"[URL]\"","konqueror \"[URL]\"","galeon \"[URL]\"","opera \"[URL]\"","firefox \"[URL]\"","mozilla \"[URL]\"",NULL},
        {"netscape \"mailto:[ADDRESS]\"","mozilla \"mailto:[ADDRESS]\"","kmail \"[ADDRESS]\"","galeon \"mailto:[ADDRESS]\"",NULL},
        {"konqueror \"[PATH]\"","nautilus \"[PATH]\"","xfm \"[PATH]\"",NULL},
        {"kfind \"[PATH]\"","gnome-search-tool \"[PATH]\"",NULL}
        };
Str Comlines[NUM_COMLINES]={Comlines_Default[0][0],Comlines_Default[1][0],Comlines_Default[2][0],Comlines_Default[3][0],Comlines_Default[4][0]};


int romfile_parse_routine(char*fn,struct stat*s)
{
  if (S_ISDIR(s->st_mode)) return FS_FTYPE_FOLDER;
  if (has_extension_list(fn,".IMG",".ROM",NULL)){
    return FS_FTYPE_FILE_ICON+ICO16_STCONFIG;
  }
  return FS_FTYPE_REJECT;
}


int diskfile_parse_routine(char *fn,struct stat *s)
{
  if (S_ISDIR(s->st_mode)) return FS_FTYPE_FOLDER;
  if (FileIsDisk(fn)){
	  return FS_FTYPE_FILE_ICON+ICO16_DISKMAN;	
  }
  return FS_FTYPE_REJECT;
}


int wavfile_parse_routine(char *fn,struct stat *s)
{
  if (S_ISDIR(s->st_mode)) return FS_FTYPE_FOLDER;
  if (has_extension(fn,".WAV")){
	  return FS_FTYPE_FILE_ICON+ICO16_SOUND;	
  }
  return FS_FTYPE_REJECT;
}


int folder_parse_routine(char *fn,struct stat *s)
{
  if (S_ISDIR(s->st_mode)) return FS_FTYPE_FOLDER;
  return FS_FTYPE_REJECT;
}


int cartfile_parse_routine(char *fn,struct stat*s)
{
  if (S_ISDIR(s->st_mode)) return FS_FTYPE_FOLDER;
  if (has_extension(fn,".STC")){
    if ((s->st_size)==128*1024+4){
      return FS_FTYPE_FILE_ICON+ICO16_CART;
    }
  }
  return FS_FTYPE_REJECT;
}


hxc_listview hxc_buttonpicker::lv;
DWORD hxc_buttonpicker::old_joy_axis_down[MAX_PC_JOYS],hxc_buttonpicker::old_joy_button_down[MAX_PC_JOYS];

#endif//UNIX


bool StepByStepInit=0;
EasyStr RunDir,WriteDir,globalINIFile,ScreenShotFol,DocDir;
EasyStr LastSnapShot,BootStateFile,StateHist[10];
#ifdef SSE_X64
EasyStr AutoSnapShotName="auto64";
#else
EasyStr AutoSnapShotName="auto32";
#endif
EasyStr DefaultSnapshotFile;
Str BootDisk[2];
int BootPasti=BOOT_PASTI_DEFAULT;
bool PauseWhenInactive=0,BootTOSImage=0;
BYTE MuteWhenInactive=0;
bool bAOT=0,bAppMaximized=0;
#ifndef ONEGAME
// autoload is convenient but crash-prone especially when switching Steem versions
bool AutoLoadSnapShot=false,ShowTips=true;
#else
bool AutoLoadSnapShot=0,ShowTips=0;
#endif
bool AllowLPT=true,AllowCOM=true;
bool HighPriority=0;
int BootInMode=BOOT_MODE_DEFAULT;

bool NoINI; // NoINI means there's no ini file, so there should be intro!

const POINT WinSize[4][5]={ {{320,200},{640,400},{960, 600},{1280,800},{-1,-1}},
                            {{640,200},{640,400},{1280,400},{1280,800},{-1,-1}},
                            {{640,400},{1280,800},{-1,-1}},
                            {{800,600},{-1,-1}}};

int WinSizeForRes[4]={1,1,0,0}; // first run: double size


#ifdef WIN32

RECT rcPreFS;

bool HandleMessage(MSG *mess) {
#if defined(SSE_GUI_TOOLBAR)
  if(ToolBar.Handle)
    if(ToolBar.HasHandledMessage(mess))
      return 0;
#endif
  if(DiskMan.Handle)
    if(DiskMan.HasHandledMessage(mess))     
      return 0;
  if(OptionBox.Handle)
    if(OptionBox.HasHandledMessage(mess))   
      return 0;
  if(JoyConfig.Handle)
    if(JoyConfig.HasHandledMessage(mess))
      return 0;
  if(InfoBox.Handle)
    if(InfoBox.HasHandledMessage(mess))
      return 0;
  if(ShortcutBox.Handle)
    if(ShortcutBox.HasHandledMessage(mess))
      return 0;
  if(HardDiskMan.Handle)
    if(HardDiskMan.HasHandledMessage(mess))
      return 0;
  if(AcsiHardDiskMan.Handle)
    if(AcsiHardDiskMan.HasHandledMessage(mess))
      return 0;
  if(PatchesBox.Handle)
    if(PatchesBox.HasHandledMessage(mess))
      return 0;
  return true;
}


int RCGetSizeOfIcon(INT_PTR n) {
  switch(n) {
  case RC_ICO_DRIVE:case RC_ICO_DRIVELINK:
  case RC_ICO_DRIVEBROKEN:case RC_ICO_DRIVEREADONLY:
  case RC_ICO_DRIVEZIPPED_RO:case RC_ICO_DRIVEZIPPED_RW:
  case RC_ICO_FOLDER:case RC_ICO_FOLDERLINK:
  case RC_ICO_FOLDERBROKEN:case RC_ICO_PARENTDIR:
  case RC_ICO_PRGFILEICO:
    return 33;
  case RC_ICO_RECORD:case RC_ICO_PLAY_BIG:
    return 32;
  case RC_ICO_HARDDRIVES:case RC_ICO_HARDDRIVES_FR:
  case RC_ICO_HARDDRIVES_ACSI:
  case RC_ICO_DRIVEA:case RC_ICO_DRIVEB:case RC_ICO_DRIVEB_DISABLED:
    return 64;
  case RC_ICO_SNAPSHOTFILEICO:
  //case RC_ICO_PRGFILEICO:
  case RC_ICO_APP256:
#ifndef DEBUG_BUILD
  case RC_ICO_BOMB:
  case RC_ICO_STCLOSE:
#endif
    return 0;
  }
  return 16;
}

#endif//WIN32

#if defined(SSE_GUI_STATUS_BAR)

void GUIRefreshStatusBar() {
  HWND status_bar_win=GetDlgItem(StemWin,IDC_STATUS_BAR); // get handle
  char *status_bar_text=StatusInfo.text;
  // should we show or hide that "status bar" (centre of icon bar at the top)
  bool should_we_show=(OPTION_STATUS_BAR!=0
#ifdef SSE_GUI_STATUS_BAR_GAME_NAME
    ||OPTION_STATUS_BAR_GAME_NAME!=0
#endif
    ); 
  // build text of "status bar", only if we're to show it
  // and it's no special string
  if(Ikbd.Crashed)
    StatusInfo.MessageIndex=TStatusInfo::HD6301_CRASH;
  if(should_we_show && StatusInfo.MessageIndex==TStatusInfo::MESSAGE_NONE)
  {
    status_bar_text[0]='\0';
    if(OPTION_STATUS_BAR)
    {
      // basic ST/TOS/RAM
      char sb_st_model[10],sb_tos[5],sb_ram[7];
      sprintf(sb_st_model,"%s",st_model_name[ST_MODEL]);
      sprintf(sb_tos,"T%03x",tos_version);
      sprintf(sb_ram,"%dK",mem_len/1024);
      // make room for flag after TXXX
#if defined(SSE_GUI_TOOLBAR)
      sprintf(status_bar_text,"%s %s        %s %dHz",
#else
      sprintf(status_bar_text,"%s %s       %s %dHz",
#endif
        sb_st_model,sb_tos,sb_ram,Glue.previous_video_freq);

#if defined(SSE_PRIVATE_BUILD)
      if(SSEOptions.TestingNewFeatures)
        strcat(status_bar," ##");
#endif
      if(OPTION_WIN_VSYNC)
        strcat(status_bar_text,"V"); // right after Hz to show it's synced
    }
#ifdef SSE_GUI_STATUS_BAR_GAME_NAME // not in v4
/*  We try to take advantage of all space.
    Font is proportional so we need a margin.
    TODO: precise computing
*/
    if(OPTION_STATUS_BAR_GAME_NAME 
      && (FloppyDrive[floppy_current_drive()].NotEmpty() ))
    {
#define MAX_TEXT_LENGTH_BORDER_ON (30+62+10) 
#define MAX_TEXT_LENGTH_BORDER_OFF (30+42+10) 
      size_t max_text_length=(border!=0)?MAX_TEXT_LENGTH_BORDER_ON:
        MAX_TEXT_LENGTH_BORDER_OFF;
      if(OPTION_STATUS_BAR)
        max_text_length-=30-5;
      if(SideBorderSizeWin<VERY_LARGE_BORDER_SIDE)
        max_text_length-=5;
      if(SideBorderSizeWin==ORIGINAL_BORDER_SIDE)
        max_text_length-=5;
      char tmp[MAX_TEXT_LENGTH_BORDER_ON+2+1]=" ";
      if(strlen(FloppyDisk[floppy_current_drive()].DiskName.Text)
        <=max_text_length)
      {
        strncpy(tmp+1,FloppyDisk[floppy_current_drive()].DiskName.Text,
          max_text_length);
#if defined(SSE_TOS_PRG_AUTORUN)
        if(FloppyDrive[0].ImageType.Extension==EXT_PRG)
          strcat(tmp,dot_ext(EXT_PRG)); // TODO
        else if(FloppyDrive[0].ImageType.Extension==EXT_TOS)
          strcat(tmp,dot_ext(EXT_TOS));
#endif
      }
      else
      {
        strncpy(tmp+1,FloppyDisk[floppy_current_drive()].DiskName.Text,
          max_text_length-3);
        strcat(tmp,"...");
      }
      strcat(status_bar_text,tmp);
    }
#undef MAX_TEXT_LENGTH_BORDER_ON
#undef MAX_TEXT_LENGTH_BORDER_OFF
/*  If the game in A: isn't displayed on status bar, then we
    show what kind of file is in A: and B:. v3.7.2
*/
    else
#endif//#ifdef SSE_GUI_STATUS_BAR_GAME_NAME
    {
      char disk_type[13]=""; // " A:MSA B:STW"
#if defined(SSE_DISK_AUTOSTW)
      if(FloppyDrive[0].ImageType.RealExtension>=NUM_EXT)
        ; // corrupt data
      else if(num_connected_floppies==1)
        sprintf(disk_type," A:%s",extension_list[FloppyDrive[0].ImageType.
          RealExtension]);
      else if(FloppyDrive[1].ImageType.RealExtension>=NUM_EXT)
        ; // corrupt data
      else
        sprintf(disk_type," A:%s B:%s",extension_list[FloppyDrive[0].ImageType.
          RealExtension],extension_list[FloppyDrive[1].ImageType.RealExtension]);
#else
      if(num_connected_floppies==1)
        sprintf(disk_type," A:%s",extension_list[FloppyDrive[0].ImageType.
          Extension]);
      else
        sprintf(disk_type," A:%s B:%s",extension_list[FloppyDrive[0].ImageType.
          Extension],extension_list[FloppyDrive[1].ImageType.Extension]);
#endif
      strcat(status_bar_text,disk_type);
    }
    if(Disp.Method==DISPMETHOD_GDI)
      strcat(status_bar_text," GDI");
#if defined(SSE_GUI_TOOLBAR) // because the arrow may be out of sight
    if(runstate==RUNSTATE_RUNNING)
      strcat(status_bar_text," Run");
#endif
  }
  switch(StatusInfo.MessageIndex) {
  case TStatusInfo::MOTOROLA_CRASH:
    strcpy(status_bar_text,T("HALT"));
    break;
  case TStatusInfo::BLIT_ERROR:
    strcpy(status_bar_text,T("BLIT ERROR"));
    break;
  case TStatusInfo::INTEL_CRASH:
    strcpy(status_bar_text,T("STEEM CRASHED AGAIN"));
    break;
  case TStatusInfo::HD6301_CRASH:
    strcpy(status_bar_text,T("6301 CRASH"));
    break;
  default:
    break;
  }
#if !defined(SSE_GUI_MENU)
  if(should_we_show)
  {
    // compute free width
    RECT window_rect1,window_rect2;
    // last icon on the left
#if defined(SSE_GUI_LEGACY_TOOLBAR)
    HWND previous_icon=GetDlgItem(StemWin,
      (OPTION_LEGACY_TOOLBAR) ? (IDC_PASTE) : (IDC_CONFIGS));
#else
    HWND previous_icon=GetDlgItem(StemWin,IDC_CONFIGS);
#endif
    GetWindowRect(previous_icon,&window_rect1); //absolute
    // first icon on the right
    HWND next_icon=GetDlgItem(StemWin,
#if defined(SSE_GUI_LEGACY_TOOLBAR)
      (OPTION_LEGACY_TOOLBAR) ? (IDC_INFO):
#endif
      (IDC_DISK_MANAGER));
    GetWindowRect(next_icon,&window_rect2); //absolute
    //TRACE_RECT(window_rect1); TRACE_RECT(window_rect2);
    int w=window_rect2.left-window_rect1.right;
    if(w<200)
      should_we_show=false;
    // resize status bar without trashing other icons
    POINT mypoint;
    mypoint.x=window_rect1.right;
    mypoint.y=window_rect1.top;
    ScreenToClient(StemWin,&mypoint);
    //TRACE("move satus bar %d %d %d %d\n",mypoint.x,0,w,window_rect1.bottom-window_rect1.top);
    MoveWindow(status_bar_win,mypoint.x,0,w,window_rect1.bottom-window_rect1.top,TRUE);
  }
  // show or hide
  ShowWindow(status_bar_win, (should_we_show) ? SW_SHOW : SW_HIDE);
#else
  static WORD old_hash=0;
  WORD new_hash=0;
  for(size_t i=0;i<strlen(status_bar_text);i++)
    new_hash+=status_bar_text[i];
  new_hash+=(ADAT)+(OPTION_C1)+(OPTION_VLE>0)+(OPTION_HACKS)
          +(!HardDiskMan.DisableHardDrives||ACSI_EMU_ON);
  bool something_changed=new_hash!=old_hash;
  old_hash=new_hash;
  if(something_changed) // because the effect is large (many Windows redraw)
#endif
    InvalidateRect(status_bar_win,NULL,FALSE); // to get message WM_DRAWITEM
}

#endif


#if defined(SSE_GUI_LEGACY_TOOLBAR)
/*  We changed the order of play, reset, etc. icons for the better but player
    may prefer the old way so there's an option for that.
*/

void GUIToolbarArrangeIcons(int cw) {
  HWND handle;
  int x=0,y=0,w=20,h=20;
  BOOL repaint=FALSE;
  UINT mask=(SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS);
  int offset=20+5;
  if(OPTION_LEGACY_TOOLBAR)
  {
    //left
    handle=GetDlgItem(StemWin,IDC_PLAY);
    MoveWindow(handle,x,y,w,h,repaint);
    x+=23;
    handle=GetDlgItem(StemWin,IDC_FASTFORWARD);
    MoveWindow(handle,x,y,w,h,repaint);
    x+=23;
    handle=GetDlgItem(StemWin,IDC_RESET);
    MoveWindow(handle,x,y,w,h,repaint);
    //right (from right to left)
#if defined(SSE_DEBUGGER_TOGGLE)
    SetWindowPos(GetDlgItem(StemWin,IDC_DEBUGGER),0,cw-offset,0,0,0,mask);
    offset+=23;
#endif
    SetWindowPos(GetDlgItem(StemWin,IDC_DISK_MANAGER),0,cw-offset,0,0,0,mask);
    offset+=23;
    SetWindowPos(GetDlgItem(StemWin,IDC_JOYSTICKS),0,cw-offset,0,0,0,mask);
    offset+=23;
    SetWindowPos(GetDlgItem(StemWin,IDC_OPTIONS),0,cw-offset,0,0,0,mask);
    offset+=23;
    SetWindowPos(GetDlgItem(StemWin,IDC_SHORTCUTS),0,cw-offset,0,0,0,mask);
    offset+=23;
    SetWindowPos(GetDlgItem(StemWin,IDC_PATCHES),0,cw-offset,0,0,0,mask);
    offset+=23;
    SetWindowPos(GetDlgItem(StemWin,IDC_INFO),0,cw-offset,0,0,0,mask);
    handle=GetDlgItem(StemWin,IDC_CONFIGS);
    ShowWindow(handle,SW_HIDE);
  }
  else // SSE
  {
    //left
    handle=GetDlgItem(StemWin,IDC_RESET);
    MoveWindow(handle,x,y,w,h,repaint);
    x+=23;
    handle=GetDlgItem(StemWin,IDC_PLAY);
    MoveWindow(handle,x,y,w,h,repaint);
    x+=23;
    handle=GetDlgItem(StemWin,IDC_FASTFORWARD);
    MoveWindow(handle,x,y,w,h,repaint);
    x+=23+23+23+23;
    handle=GetDlgItem(StemWin,IDC_CONFIGS);
    MoveWindow(handle,x,y,w,h,repaint);
    //right (from right to left)
    SetWindowPos(GetDlgItem(StemWin,IDC_INFO),0,cw-offset,0,0,0,mask);
    offset+=23;
#if defined(SSE_DEBUGGER_TOGGLE)
    SetWindowPos(GetDlgItem(StemWin,IDC_DEBUGGER),0,cw-offset,0,0,0,mask);
    offset+=23;
#endif
    SetWindowPos(GetDlgItem(StemWin,IDC_OPTIONS),0,cw-offset,0,0,0,mask);
    offset+=23;
    SetWindowPos(GetDlgItem(StemWin,IDC_SHORTCUTS),0,cw-offset,0,0,0,mask);
    offset+=23;
    SetWindowPos(GetDlgItem(StemWin,IDC_PATCHES),0,cw-offset,0,0,0,mask);
    offset+=23;
    SetWindowPos(GetDlgItem(StemWin,IDC_JOYSTICKS),0,cw-offset,0,0,0,mask);
    offset+=23;
    SetWindowPos(GetDlgItem(StemWin,IDC_DISK_MANAGER),0,cw-offset,0,0,0,mask);
    handle=GetDlgItem(StemWin,IDC_CONFIGS);
    ShowWindow(handle,SW_SHOW);
  }
}

#endif


void GUIRunStart() {
#ifdef WIN32
  KillTimer(StemWin,SHORTCUTS_TIMER_ID);
  if(AllowTaskSwitch==0) 
    DisableTaskSwitch();
  if(HighPriority) 
    SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);

#elif defined(PRIO_PROCESS) && defined(PRIO_MAX)
  hxc::kill_timer(StemWin,SHORTCUTS_TIMER_ID);
  if(HighPriority) setpriority(PRIO_PROCESS,0,PRIO_MAX);
#endif
  CheckResetDisplay(true);
  WIN_ONLY(if(FullScreen) TScreenSaver::killTimer(); )
}


bool GUIPauseWhenInactive() { // called by event_vbl_interrupt()
  if((PauseWhenInactive && bAppActive==0)||timer<CutPauseUntilSysEx_Time) 
  {
    bool MouseWasCaptured=(stem_mousemode==STEM_MOUSEMODE_WINDOW);
    if(FullScreen==0) 
      SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
    Sound_Stop();

#ifdef WIN32
    SetWindowText(StemWin,Str("Steem - ")+T("Suspended"));
    MSG mess;
    SetTimer(StemWin,MIDISYSEX_TIMER_ID,100,NULL);
#if defined(SSE_EMU_THREAD)
    if(OPTION_EMUTHREAD && hEmuThread && runstate==RUNSTATE_RUNNING)
    {
      while(bAppActive==0)
        Sleep(200);
    }
    else
#endif
    {
      while(GetMessage(&mess,NULL,0,0)) {
        if(HandleMessage(&mess)) {
          TranslateMessage(&mess);
          DispatchMessage(&mess);
        }
        if(timeGetTime()>CutPauseUntilSysEx_Time&&(PauseWhenInactive==0||bAppActive)) 
          break;
        if(runstate!=RUNSTATE_RUNNING) 
          break;
      }
      if(mess.message==WM_QUIT) 
        QuitSteem();
    }
    KillTimer(StemWin,MIDISYSEX_TIMER_ID);
    SetWindowText(StemWin,stem_window_title);
#endif//WIN32

#ifdef UNIX
    XEvent Ev;
    do {
      if(hxc::wait_for_event(XD,&Ev,200)) ProcessEvent(&Ev);

      if(timeGetTime()>CutPauseUntilSysEx_Time&&(PauseWhenInactive==0||bAppActive)) break;
    } while(runstate==RUNSTATE_RUNNING && Quitting==0);
#endif

    if(FullScreen==0&&MouseWasCaptured && GetForegroundWindow()==StemWin)
      SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
    Sound_Start();
    return true;
  }
  return 0;
}


void GUIRunEnd() {
  slow_motion=0;
  if(fast_forward) 
  {
    fast_forward=0;
    flashlight(false);
#ifdef WIN32
    SendMessage(GetDlgItem(StemWin,109),BM_SETCHECK,0,1);
#endif
#ifdef UNIX
    FastBut.set_check(0);
#endif
  }
  WIN_ONLY(if(HighPriority) SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS); )
#ifdef PRIO_PROCESS
  UNIX_ONLY(setpriority(PRIO_PROCESS,0,0); )
#endif
  SetStemMouseMode(STEM_MOUSEMODE_DISABLED);
  WIN_ONLY(EnableTaskSwitch(); )
  WIN_ONLY(SetTimer(StemWin,SHORTCUTS_TIMER_ID,50,NULL); )
  UNIX_ONLY(hxc::set_timer(StemWin,SHORTCUTS_TIMER_ID,50,timerproc,NULL); )
  WIN_ONLY(if(FullScreen) TScreenSaver::prepareTimer(); )
}


void GUIColdResetChangeSettings() {
  if(OptionBox.NewMemConf0==-1) 
  {
    if((SSEConfig.bank_length[0]+SSEConfig.bank_length[1])==(KB512+KB128))
    {  // Old 512Kb
      OptionBox.NewMemConf0=MEMCONF_512;
      OptionBox.NewMemConf1=MEMCONF_512K_BANK1_CONF;
    }
#if !defined(SSE_MMU_2560K)
    else if((SSEConfig.bank_length[0]+SSEConfig.bank_length[1])==(MB2+KB512))
    {  // Old 2Mb
      OptionBox.NewMemConf0=MEMCONF_2MB;
      OptionBox.NewMemConf1=MEMCONF_2MB_BANK1_CONF;
    }
#endif
  }
  if(OptionBox.NewMemConf0>=0) 
  {
    SSEConfig.make_Mem(BYTE(OptionBox.NewMemConf0),BYTE(OptionBox.NewMemConf1));
    OptionBox.NewMemConf0=-1;
  }
  if(OptionBox.NewMonitorSel>=0) 
  {
#ifndef NO_CRAZY_MONITOR
    bool old_em=(extended_monitor!=0); //would need type_of() 
    extended_monitor=0;
#endif
    if(OptionBox.NewMonitorSel==1)
    {
      mfp_gpip_no_interrupt&=MFP_GPIP_NOT_COLOUR;
      SSEConfig.ColourMonitor=false;
    }
    else if(OptionBox.NewMonitorSel==0)
    {
      mfp_gpip_no_interrupt|=MFP_GPIP_COLOUR;
      SSEConfig.ColourMonitor=true;
    }
    else
    { //crazy monitor
#ifndef NO_CRAZY_MONITOR
      int m=OptionBox.NewMonitorSel-2;
      if(extmon_res[m][2]==1) 
      {
        mfp_gpip_no_interrupt&=MFP_GPIP_NOT_COLOUR;
        SSEConfig.ColourMonitor=false;
        screen_res=2;
      }
      else
      {
        mfp_gpip_no_interrupt|=MFP_GPIP_COLOUR;
        SSEConfig.ColourMonitor=true;
        screen_res=0;
      }
      extended_monitor=1;
      em_width=extmon_res[m][0];
      em_height=extmon_res[m][1];
      em_planes=(BYTE)extmon_res[m][2];
      DISPLAY_SIZE=0;
#endif
    }
    //if(extended_monitor!=old_em||extended_monitor)
    if(old_em||extended_monitor)
    {
      if(FullScreen)
        change_fullscreen_display_mode(true);
      else
        Disp.ScreenChange(); // For extended monitor
    }
    else
      ChangeBorderSize(DISPLAY_SIZE);
    OptionBox.NewMonitorSel=-1;
  }
  if(OPTION_HACKS&&(tos_version==0x106||tos_version==0x162)
    && OptionBox.NewROMFile.IsEmpty())
    OptionBox.NewROMFile=ROMFile;
  if(OptionBox.NewROMFile.NotEmpty()) 
  {
    if(load_TOS(OptionBox.NewROMFile))
      Alert(T("The selected TOS file")+" "+OptionBox.NewROMFile+" "
        +T("is not in the correct format or may be corrupt."),T("Cannot Load TOS"),MB_ICONEXCLAMATION);
    else
      ROMFile=OptionBox.NewROMFile;
    OptionBox.NewROMFile="";
  }
}


void GUISaveResetBackup() {
  DeleteFile(WriteDir+SLASH+"auto_loadsnapshot_backup.sts");
  SaveSnapShot(WriteDir+SLASH+"auto_reset_backup.sts",-1,0); // Don't add to history
}


void GUIDiskErrorEject(int f) {
  DiskMan.EjectDisk(f);
}


void GUIEmudetectCreateDisk(Str Name,WORD Sides,WORD TracksPerSide,
                            WORD SectorsPerTrack) {
  Str DiskPath=DiskMan.HomeFol+SLASH+Name+".st";
  DiskMan.CreateDiskImage(Name,TracksPerSide*Sides*SectorsPerTrack,
    SectorsPerTrack,Sides);
  DiskMan.InsertDisk(0,Name,DiskPath);
}


bool GUICanGetKeys() {
  return (GetForegroundWindow()==StemWin);
}


void LoadAllIcons(TConfigStoreFile *pCSF,bool FirstCall) {
#ifdef WIN32
#ifdef ONEGAME
  for(int n=1;n<RC_NUM_ICONS;n++) 
    hGUIIcon[n]=NULL;
#else
  HICON hOld[RC_NUM_ICONS],hOldSmall[RC_NUM_ICONS];
  for(int n=1;n<RC_NUM_ICONS;n++) 
  {
    hOld[n]=hGUIIcon[n];
    hOldSmall[n]=hGUIIconSmall[n];
  }
  bool UseDefault=0;
  HDC dc=GetDC(NULL);
  if(GetDeviceCaps(dc,BITSPIXEL)<=8)
    UseDefault=(pCSF->GetInt("Icons","UseDefaultIn256",0)!=0);
  ReleaseDC(NULL,dc);
  Str File;
  for(WORD n=1;n<RC_NUM_ICONS;n++) 
  {
    int size=RCGetSizeOfIcon(n);
    bool load16too=size & 1;
    size&=~1;
    hGUIIcon[n]=NULL;
    hGUIIconSmall[n]=NULL;
    if(size)
    {
      if(UseDefault==0) 
        File=pCSF->GetStr("Icons",Str("Icon")+n,"");
      if(File.NotEmpty()) 
        hGUIIcon[n]=(HICON)LoadImage(Inst,File,IMAGE_ICON,size,size,LR_LOADFROMFILE);
      if(hGUIIcon[n]==NULL) 
        hGUIIcon[n]=(HICON)LoadImage(Inst,RCNUM(n),IMAGE_ICON,size,size,0);
      if(load16too) 
      {
        if(File.NotEmpty()) 
          hGUIIconSmall[n]=(HICON)LoadImage(Inst,File,IMAGE_ICON,16,16,LR_LOADFROMFILE);
        if(hGUIIconSmall[n]==NULL) 
          hGUIIconSmall[n]=(HICON)LoadImage(Inst,RCNUM(n),IMAGE_ICON,16,16,0);
      }
    }
  }
  if(FirstCall==0)
  {
    // Update all window classes, buttons and other icon thingies
    SetClassLongPtr(StemWin,GCLP_HICON,(LONG_PTR)hGUIIcon[RC_ICO_APP]);
#ifdef DEBUG_BUILD
    SetClassLongPtr(DWin,GCLP_HICON,(LONG_PTR)hGUIIcon[RC_ICO_BOMB]);
    SetClassLongPtr(trace_window_handle,GCLP_HICON,(LONG_PTR)hGUIIcon[RC_ICO_STCLOSE]);
    for(int n=0;n<MAX_MEMORY_BROWSERS;n++)
      if(m_b[n])
        m_b[n]->update_icon();
#endif
    for(int n=0;n<nStemDialogs;n++)
      DialogList[n]->UpdateMainWindowIcon();
    for(int id=IDC_DISK_MANAGER;id<=IDC_CONFIGS;id++)
    {
      if(GetDlgItem(StemWin,id))
        PostMessage(GetDlgItem(StemWin,id),BM_RELOADICON,0,0);
    }
    DiskMan.LoadIcons();
    OptionBox.LoadIcons();
    InfoBox.LoadIcons();
    ShortcutBox.UpdateDirectoryTreeIcons(&(ShortcutBox.DTree));
    if(ShortcutBox.pChooseMacroTree)
      ShortcutBox.UpdateDirectoryTreeIcons(ShortcutBox.pChooseMacroTree);
    for(int n=1;n<RC_NUM_ICONS;n++) 
    {
      if(hOld[n]) 
        DestroyIcon(hOld[n]);
      if(hOldSmall[n]) 
        DestroyIcon(hOldSmall[n]);
    }
  }
#endif
#endif//WIN32
}


#define LOGSECTION LOGSECTION_INIT

BOOL MakeGUI() {
#ifdef WIN32
  GuiSM.Update();
#if defined(SSE_GUI_FONT_FIX)
  fnt=SSEConfig.GuiFont();
#else
	fnt=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
#endif
  MidGUIRGB=GetMidColour(GetSysColor(COLOR_3DFACE),GetSysColor(COLOR_WINDOW));
  DkMidGUIRGB=GetMidColour(GetSysColor(COLOR_3DFACE),MidGUIRGB);
  PCArrow=LoadCursor(NULL,IDC_ARROW);
  ParentWin=GetDesktopWindow();
  WNDCLASS wc={0,WndProc,0,0,Inst,hGUIIcon[RC_ICO_APP],
                PCArrow,NULL,NULL,"Steem Window"};
  RegisterClass(&wc);
  wc.lpfnWndProc=FSClipWndProc;
  wc.hIcon=NULL;
  wc.hCursor=NULL;
  wc.lpszClassName="Steem Fullscreen Clip Window";
  RegisterClass(&wc);
  wc.lpfnWndProc=FSQuitWndProc;
  wc.lpszClassName="Steem Fullscreen Quit Button";
  RegisterClass(&wc);
  wc.lpfnWndProc=ResetInfoWndProc;
  wc.lpszClassName="Steem Reset Info Window";
  RegisterClass(&wc);
  RegisterSteemControls();
  RegisterButtonPicker();
  // Main Steem window
  StemWin=CreateWindowEx(WS_EX_ACCEPTFILES,"Steem Window",stem_window_title,
    WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_SIZEBOX|WS_MAXIMIZEBOX
    |WS_CLIPSIBLINGS,180,180,324+GuiSM.cx_frame()*2,204+MENUHEIGHT+GuiSM.cy_frame()*2
    +GuiSM.cy_caption(),ParentWin,NULL,Inst,NULL);
  if(StemWin==NULL) 
    return 0;
  if(IsWindow(StemWin)==0) 
  {
    StemWin=NULL;
    return 0;
  }
  StemWin_SysMenu=GetSystemMenu(StemWin,0); // Window menu
  int pos=GetMenuItemCount(StemWin_SysMenu)-2;
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION|MF_STRING,IDSYS_SMALLER,
    T("Smaller Window"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION|MF_STRING,IDSYS_BIGGER,
    T("Bigger Window"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION|MF_SEPARATOR,0,NULL);
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION|MF_STRING,IDSYS_BORDEROFF,
    T("Borders Off"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION|MF_STRING,IDSYS_BORDERON,
    T("Borders On"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION|MF_SEPARATOR,0,NULL);
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION|MF_STRING,IDSYS_NOOSD,
    T("Disable On Screen Display"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION|MF_SEPARATOR,0,NULL);
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION|MF_STRING,IDSYS_TOP,
    T("Always On Top"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION|MF_STRING,IDSYS_ASPECT,
    T("Restore Aspect Ratio"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION|MF_STRING,IDSYS_NORMAL,
    T("Normal Size"));
  InsertMenu(StemWin_SysMenu,pos,MF_BYPOSITION|MF_SEPARATOR,0,NULL);
#if defined(SSE_GUI_ALT_MENU)
  StemWin_AltMenu=CreateMenu(); // Menu when press Alt
  StemWin_AltMenuFile=CreatePopupMenu();
  StemWin_AltMenuEmu=CreatePopupMenu();
  StemWin_AltMenuTools=CreatePopupMenu();
  AppendMenu(StemWin_AltMenu,MF_POPUP|MF_STRING,(UINT_PTR)StemWin_AltMenuFile,
    "&File");
  AppendMenu(StemWin_AltMenu,MF_POPUP|MF_STRING,(UINT_PTR)StemWin_AltMenuEmu,
    "&Emu");
  AppendMenu(StemWin_AltMenu,MF_POPUP|MF_STRING,(UINT_PTR)StemWin_AltMenuTools,
    "&Tools");
  // File
  AppendMenu(StemWin_AltMenuFile,MF_STRING,602,"&Disk Manager");
  AppendMenu(StemWin_AltMenuFile,MF_STRING,613,T("Insert Disk &A"));
  AppendMenu(StemWin_AltMenuFile,MF_STRING,614,T("Insert Disk &B"));
  AppendMenu(StemWin_AltMenuFile,MF_STRING,615,T("GEMDOS Hard discs"));
#if defined(SSE_ACSI)
  AppendMenu(StemWin_AltMenuFile,MF_STRING,616,T("ACSI Hard discs"));
#endif
  AppendMenu(StemWin_AltMenuFile,MF_STRING,443,T("&Load configuration file"));
  AppendMenu(StemWin_AltMenuFile,MF_STRING,444,T("&Save configuration file"));
  AppendMenu(StemWin_AltMenuFile,MF_STRING,200,T("L&oad snapshot file"));
  AppendMenu(StemWin_AltMenuFile,MF_STRING,201,T("Sa&ve snapshot file"));
  AppendMenu(StemWin_AltMenuFile,MF_STRING,609,"E&xit");
  // Emu
  AppendMenu(StemWin_AltMenuEmu,MF_STRING,601,"&Run/Stop (F12)");
  AppendMenu(StemWin_AltMenuEmu,MF_STRING,611,"Re&boot");
  AppendMenu(StemWin_AltMenuEmu,MF_STRING,612,"Re&set");
  AppendMenu(StemWin_AltMenuEmu,MF_STRING,207,"&Undo last reset");
  AppendMenu(StemWin_AltMenuEmu,MF_STRING,605,"Toggle &Fullsceen (Alt-Enter)");
  AppendMenu(StemWin_AltMenuEmu,MF_STRING,603,"&Patches");
#if defined(SSE_EMU_THREAD)
  AppendMenu(StemWin_AltMenuEmu,MF_STRING|MF_DISABLED,604,"&Kill emu thread");
#endif
  // Tools
#if defined(SSE_GUI_TOOLBAR)
  AppendMenu(StemWin_AltMenuTools,MF_STRING,656,"&Tool bar");
#endif
#if defined(SSE_DEBUGGER_TOGGLE)
  AppendMenu(StemWin_AltMenuTools,MF_STRING,655,"&Debugger");
#endif
  AppendMenu(StemWin_AltMenuTools,MF_STRING,651,"&Options");
  AppendMenu(StemWin_AltMenuTools,MF_STRING,652,"&Shortcuts");
  AppendMenu(StemWin_AltMenuTools,MF_STRING,653,"&Joysticks");
  AppendMenu(StemWin_AltMenuTools,MF_STRING,654,"&Info");
#endif
  ToolTip=CreateWindowEx(WS_EX_TOPMOST,TOOLTIPS_CLASS,NULL,TTS_ALWAYSTIP
    |TTS_NOPREFIX,0,0,100,100,NULL,NULL,Inst,NULL);
  SendMessage(ToolTip,TTM_SETDELAYTIME,TTDT_AUTOPOP,20000); // 20 seconds before disappear
  SendMessage(ToolTip,TTM_SETDELAYTIME,TTDT_INITIAL,400);   // 0.4 second before appear
  SendMessage(ToolTip,TTM_SETDELAYTIME,TTDT_RESHOW,200);     // 0.2 moving from one tool to next
  SendMessage(ToolTip,TTM_SETMAXTIPWIDTH,0,400);
#if defined(SSE_GUI_MENU)
  StemWin_AltMenuStatus=CreatePopupMenu();   // was fake Status bar, now fake menu!
  VERIFY(AppendMenu(StemWin_AltMenu,MFT_OWNERDRAW,(UINT_PTR)IDC_STATUS_BAR,""));
  SetMenu(StemWin,StemWin_AltMenu);
#endif
#if !defined(SSE_GUI_TOOLBAR)
#ifndef ONEGAME
  int x=0;
#if 1
  HWND Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_RESET),WS_CHILDWINDOW
    |WS_VISIBLE|PBS_RIGHTCLICK,x,0,20,20,StemWin,(HMENU)IDC_RESET,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Reset (Left Click = Warm, Right Click = Cold)"));
  x+=23;
#else
  HWND
#endif
    Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_PLAY),WS_CHILDWINDOW|
    WS_VISIBLE|WS_TABSTOP|PBS_RIGHTCLICK,x,0,20,20,StemWin,(HMENU)IDC_PLAY,Inst,
    NULL);
#endif
  ToolAddWindow(ToolTip,Win,
    T("Run (Left Click = Run/Stop, Right Click = Slow Motion)"));
  x+=23;
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_FF),WS_CHILDWINDOW
    |WS_VISIBLE|PBS_RIGHTCLICK|PBS_DBLCLK,x,0,20,20,StemWin,
    (HMENU)IDC_FASTFORWARD,Inst,NULL);
  ToolAddWindow(ToolTip,Win,
    T("Fast Forward (Right Click = Searchlight, Double Click = Sticky)"));
  x+=23;
#if 0
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_RESET),WS_CHILDWINDOW
    |WS_VISIBLE|PBS_RIGHTCLICK,x,0,20,20,StemWin,(HMENU)IDC_RESET,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Reset (Left Click = Warm, Right Click = Cold)"));
  x+=23;
#endif
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_SNAPSHOTBUT),WS_CHILDWINDOW
    |WS_VISIBLE,x,0,20,20,StemWin,(HMENU)IDC_SNAPSHOT,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Memory Snapshot Menu"));
  x+=23;
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_TAKESCREENSHOTBUT),
    WS_CHILDWINDOW|WS_VISIBLE|PBS_RIGHTCLICK,x,0,20,20,StemWin,
    (HMENU)IDC_SCREENSHOT,Inst,NULL);
  ToolAddWindow(ToolTip,Win,
    T("Take Screenshot")+" ("+T("Right Click = Options")+")");
  x+=23;
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_PASTE),WS_CHILD|WS_VISIBLE
    |PBS_RIGHTCLICK,x,0,20,20,StemWin,(HMENU)IDC_PASTE,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Paste Text Into ST (Right Click = Options)"));
  x+=23;
#ifdef RELEASE_BUILD 
  // This causes freeze up if tracing in debugger, so only do it in final build
  NextClipboardViewerWin=SetClipboardViewer(StemWin);
#endif
  UpdatePasteButton();
#if !defined(SSE_NO_UPDATE)
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_UPDATE),WS_CHILD,
                          x,0,20,20,StemWin,(HMENU)120,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Steem Update Available! Click Here For Details!"));
#endif
#if defined(SSE_GUI_CONFIG_FILE)
  // 'wrench' icon for config files, popup menu when left click
  // it only doubles the feature in Option box but I didn't know!
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_CFG),WS_CHILDWINDOW|
    WS_VISIBLE,x,0,20,20,StemWin,(HMENU)IDC_CONFIGS,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Load/save configuration file"));
  x+=23;
#endif
/*  Create an owner drawn static control as status bar. We take the undef update
    icon's  number.
    WINDOW_TITLE is dummy, the field will be updated later, its size too.
*/
  Win=CreateWindowEx(0,"Static",WINDOW_TITLE,WS_CHILD|WS_VISIBLE|SS_OWNERDRAW,
    x,0,50,20,StemWin,(HMENU)IDC_STATUS_BAR,Inst,NULL);
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_PATCHES),WS_CHILD|WS_VISIBLE,
    100,0,20,20,StemWin,(HMENU)IDC_PATCHES,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Patches"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_SHORTCUT),WS_CHILD|WS_VISIBLE,
    100,0,20,20,StemWin,(HMENU)IDC_SHORTCUTS,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Shortcuts"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_OPTIONS),WS_CHILD|WS_VISIBLE,
    100,0,20,20,StemWin,(HMENU)IDC_OPTIONS,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Options"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_JOY),WS_CHILDWINDOW|WS_VISIBLE,
    100,0,20,20,StemWin,(HMENU)IDC_JOYSTICKS,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Joystick Configuration"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_DISKMAN),WS_CHILDWINDOW|WS_VISIBLE
    ,100,0,20,20,StemWin,(HMENU)IDC_DISK_MANAGER,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Disk Manager"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_INFO),WS_CHILD|WS_VISIBLE,
    100,0,20,20,StemWin,(HMENU)IDC_INFO,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("General Info"));
#if defined(SSE_DEBUGGER_TOGGLE)
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_BOMB),WS_CHILD|WS_VISIBLE,
    100,0,20,20,StemWin,(HMENU)IDC_DEBUGGER,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Debugger"));
#endif
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_TOWINDOW),WS_CHILD,
    120,0,20,20,StemWin,(HMENU)106,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Windowed Mode"));
  Win=CreateWindow("Steem Fullscreen Quit Button","",WS_CHILD,
    120,0,20,20,StemWin,(HMENU)116,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Quit Steem"));
  SetWindowAndChildrensFont(StemWin,fnt);
#endif//#if !defined(SSE_GUI_TOOLBAR)
#ifndef ONEGAME
  CentreWindow(StemWin,0);
#else
  MoveWindow(StemWin,GetScreenWidth(),0,100,100,0);
#endif
#ifdef DEBUG_BUILD
  DBG_LOG("STARTUP: DWin_init Called");
  DWin_init();
#endif
#endif//WIN32

#ifdef UNIX
  if (XD==NULL) return 0;

  UNIX_get_fake_VKs();
  RunSteemAtom=XInternAtom(XD,"SteemRun",0);
  LoadSnapShotAtom=XInternAtom(XD,"SteemLoadSnapShot",0);
#ifdef ALLOW_XALLOCID
  SteemWindowGroup=XAllocID(XD);
#endif

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.colormap=colormap;
  StemWin=XCreateWindow(XD,
  				XDefaultRootWindow(XD),
                200,
                200,
                2+320+2,
                MENUHEIGHT+2+200+2,
                0,
                CopyFromParent,
                InputOutput,
                CopyFromParent, 
                CWBackingStore | int(colormap ? CWColormap:0),
                &swa);
  if (StemWin==0) return 0;

  hxc::load_res(XD);

  Atom Prots[1]={hxc::XA_WM_DELETE_WINDOW};
  XSetWMProtocols(XD,StemWin,Prots,1);

  DispGC=XCreateGC(XD,StemWin,0,NULL);

#if defined(SSE_UNIX) //&& defined(SSE_GUI_WINDOW_TITLE)
  XSetStandardProperties(XD,StemWin,WINDOW_TITLE,"Steem",None,_argv,_argc,NULL);
#else
  XSetStandardProperties(XD,StemWin,"Steem Engine","Steem",None,_argv,_argc,NULL);
#endif

  StemWinIconPixmap=Ico16.CreateIconPixmap(ICO16_STEEM,DispGC);
  StemWinIconMaskPixmap=Ico16.CreateMaskBitmap(ICO16_STEEM);
  SetWindowHints(XD,StemWin,True,NormalState,StemWinIconPixmap,StemWinIconMaskPixmap,SteemWindowGroup,0);

  XSelectInput(XD,StemWin,KeyPressMask | KeyReleaseMask |
                    ButtonPressMask | ButtonReleaseMask |
                    ExposureMask | StructureNotifyMask |
                    VisibilityChangeMask | FocusChangeMask);

  SetProp(StemWin,cWinProc,(DWORD)StemWinProc);

  int x=0;
  RunBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                  BT_ICON | BT_UPDOWNNOTIFY,"",101,BkCol);
  RunBut.set_icon(&Ico16,ICO16_RUN);
  hints.add(RunBut.handle,T("Run (Right Click = Slow Motion)"),StemWin);
  x+=23;

  FastBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                  BT_ICON | BT_UPDOWNNOTIFY,"",109,BkCol);
  FastBut.set_icon(&Ico16,ICO16_FF);
  hints.add(FastBut.handle,T("Fast Forward (Right Click = Searchlight, Double Click = Sticky)"),StemWin);
  x+=23;

  ResetBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",102,BkCol);
  ResetBut.set_icon(&Ico16,ICO16_RESET);

#if defined(SSE_GUI_RESET_BUTTON)
  hints.add(ResetBut.handle,T("Reset (Left Click) - Switch off (Right Click)"),StemWin);
#else
  hints.add(ResetBut.handle,T("Reset (Left Click = Cold, Right Click = Warm)"),StemWin);
#endif

  x+=23;

  SnapShotBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                      BT_ICON,"",108,BkCol);
  SnapShotBut.set_icon(&Ico16,ICO16_SNAPSHOTS);
  hints.add(SnapShotBut.handle,T("Memory Snapshot Menu"),StemWin);
  x+=23;

  ScreenShotBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                      BT_ICON,"",116,BkCol);
  ScreenShotBut.set_icon(&Ico16,ICO16_TAKESCREENSHOTBUT);
  hints.add(ScreenShotBut.handle,T("Take Screenshot"),StemWin);
  x+=23;

  PasteBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                      BT_ICON,"",114,BkCol);
  PasteBut.set_icon(&Ico16,ICO16_PASTE);
  hints.add(PasteBut.handle,T("Paste Text Into ST (Right Click = Options)"),StemWin);
  x+=23;

  if (Disp.CanGoToFullScreen()){
    FullScreenBut.create(XD,StemWin,x,0,20,20,StemWinButtonNotifyProc,NULL,
                        BT_ICON | BT_TOGGLE,"",115,BkCol);
    FullScreenBut.set_icon(&Ico16,ICO16_FULLSCREEN);
    hints.add(FullScreenBut.handle,T("Fullscreen"),StemWin);
  }

  InfBut.create(XD,StemWin,0,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",105,BkCol);
  InfBut.set_icon(&Ico16,ICO16_GENERALINFO);
  hints.add(InfBut.handle,T("General Info"),StemWin);

  PatBut.create(XD,StemWin,0,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",113,BkCol);
  PatBut.set_icon(&Ico16,ICO16_PATCHES);
  hints.add(PatBut.handle,T("Patches"),StemWin);

  CutBut.create(XD,StemWin,0,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",112,BkCol);
  CutBut.set_icon(&Ico16,ICO16_CUT);
  hints.add(CutBut.handle,T("Shortcuts"),StemWin);

  OptBut.create(XD,StemWin,0,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",107,BkCol);
  OptBut.set_icon(&Ico16,ICO16_OPTIONS);
  hints.add(OptBut.handle,T("Options"),StemWin);

  JoyBut.create(XD,StemWin,0,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",103,BkCol);
  JoyBut.set_icon(&Ico16,ICO16_JOY);
  hints.add(JoyBut.handle,T("Joystick Configuration"),StemWin);

  DiskBut.create(XD,StemWin,0,0,20,20,StemWinButtonNotifyProc,NULL,
                    BT_ICON,"",100,BkCol);
  DiskBut.set_icon(&Ico16,ICO16_DISKMAN);
  hints.add(DiskBut.handle,T("Disk Manager"),StemWin);


#ifndef CYGWIN
  // Create empty 1 bit per pixel bitmap
  char *Dat=new char[16*16/8];
  ZeroMemory(Dat,16*16/8);
  Pixmap EmptyPix=XCreatePixmapFromBitmapData(XD,StemWin,
                    Dat,16,16,0,0,1);
  XColor ccols[2];
  ccols[0].pixel=0;
  ccols[1].pixel=0;
  EmptyCursor=XCreatePixmapCursor(XD,EmptyPix,EmptyPix,
                    &ccols[0],&ccols[1],8,8);
  XFreePixmap(XD,EmptyPix);
  delete[] Dat;
#else
  EmptyCursor=XCreateFontCursor(XD,XC_cross);
#endif
#endif//UNIX
  return TRUE;
}


void CheckResetIcon() {
#ifdef WIN32
  if(StemWin==NULL)
    return;
  HWND ResetBut=GetDlgItem(StemWin,IDC_RESET);
  if(ResetBut==NULL) 
    return;
  int new_icon=int(OptionBox.NeedReset() ? RC_ICO_RESETGLOW:RC_ICO_RESET);
  Str CurNumText;
  CurNumText.SetLength(20);
  GetWindowText(ResetBut,CurNumText,20);
  if(atoi(CurNumText)!=new_icon) 
    SetWindowText(ResetBut,Str(new_icon));
#endif

#ifdef UNIX
  if (ResetBut.handle==0) return;
  int new_icon=int(OptionBox.NeedReset() ? ICO16_RESETGLOW:ICO16_RESET);
  if (ResetBut.icon_index!=new_icon){
    ResetBut.icon_index=new_icon;
    ResetBut.draw();
  }
#endif
}


void CheckResetDisplay(bool NOT_ONEGAME(AlwaysHide)) {
  if(!SSEConfig.IsInit) // hdman constructor calls, pc isn't referenced
    return;
#ifndef ONEGAME
#ifdef WIN32
  if(LITTLE_PC==rom_addr && StemWin && runstate==RUNSTATE_STOPPED && !AlwaysHide)
  {
    if(ResetInfoWin==NULL) 
    {
      if(FullScreen==0) 
        SetWindowLong(StemWin,GWL_STYLE,GetWindowLong(StemWin,GWL_STYLE)
          |WS_CLIPCHILDREN);
#if defined(SSE_VID_D3D) || defined(SSE_VID_DD_MISC)
      ResetInfoWin=CreateWindow("Steem Reset Info Window","",WS_CHILD,
        0,0,0,0,HWND(StemWin),(HMENU)9876,Inst,NULL);
#else
      ResetInfoWin=CreateWindow("Steem Reset Info Window","",WS_CHILD,
        0,0,0,0,HWND(FullScreen?ClipWin:StemWin),(HMENU)9876,Inst,NULL);
#endif
      SendMessage(ResetInfoWin,WM_USER,1789,0);
      ShowWindow(ResetInfoWin,SW_SHOWNA);
    }
    else
    {
      SendMessage(ResetInfoWin,WM_USER,1789,0);
      InvalidateRect(ResetInfoWin,NULL,0);
    }
  }
  else if(ResetInfoWin)
  {
    HWND Win=ResetInfoWin;
    ResetInfoWin=NULL;
    DestroyWindow(Win);
    SetWindowLong(StemWin,GWL_STYLE,GetWindowLong(StemWin,GWL_STYLE) 
      & ~WS_CLIPCHILDREN);
  }
#endif//WIN32

#ifdef UNIX
  if (pc==rom_addr && StemWin){
		XWindowAttributes wa;
	  XGetWindowAttributes(XD,StemWin,&wa);
  	XClearArea(XD,StemWin,2,MENUHEIGHT,wa.width-4,
            wa.height-4-MENUHEIGHT,True); // Make StemWin redraw
  }
#endif
#endif
}

#undef LOGSECTION
#define LOGSECTION LOGSECTION_SHUTDOWN

void CleanupGUI() {
#ifdef WIN32
  WNDCLASS wc;
#ifdef DEBUG_BUILD
  DWin_edit_is_being_temporarily_defocussed=true;
  DBG_LOG("SHUTDOWN: Destroying debug-build menus");
  if(insp_menu) 
    DestroyMenu(insp_menu);
  if(trace_window_handle)
    DestroyWindow(trace_window_handle);
  trace_window_handle=NULL;
  DBG_LOG("SHUTDOWN: Destroying Debugger Mr Statics");
  if(DWin) 
  {
    mr_static_delete_children_of(DWin);
    mr_static_delete_children_of(DWin_timings_scroller.GetControlPage());
  }
  DBG_LOG("SHUTDOWN: Destroying debug-build Debugger window");
  if(DWin) 
    DestroyWindow(DWin);
  DBG_LOG("SHUTDOWN: Destroying debug-build memory browsers");
  for(int n=0;n<MAX_MEMORY_BROWSERS;n++)
  {
    if(m_b[n]!=NULL && m_b[n]->owner!=NULL)
    {
      if(IsWindow(m_b[n]->owner))
      {
        DestroyWindow(m_b[n]->owner);
        n--;
      }
    }
  }
  debug_plugin_free();
  if(HiddenParent) 
    DestroyWindow(HiddenParent);
  if(GetClassInfo(Inst,"Steem Debug Window",&wc))
    UnregisterClass("Steem Debug Window",Inst);
  if(GetClassInfo(Inst,"Steem Trace Window",&wc)) 
  {
    UnregisterClass("Steem Mem Browser Window",Inst);
    UnregisterClass("Steem Trace Window",Inst);
  }
  if(mem_browser::icons_bmp) 
  {
    DeleteDC(mem_browser::icons_dc);
    DeleteObject(mem_browser::icons_bmp);
  }
#endif
#if defined(SSE_GUI_ALT_MENU)
  DestroyMenu(StemWin_AltMenu);
  DestroyMenu(StemWin_AltMenuFile);
  DestroyMenu(StemWin_AltMenuEmu);
  DestroyMenu(StemWin_AltMenuTools);
#endif
  DBG_LOG("SHUTDOWN: Destroying StemWin");
  if(StemWin) 
  {
    CheckResetDisplay(true);
    DestroyWindow(StemWin);
#if !defined(SSE_LEAN_AND_MEAN)
    StemWin=NULL;
#endif
  }
  DBG_LOG("SHUTDOWN: Destroying ToolTip");
  if(ToolTip) 
    DestroyWindow(ToolTip);
  if(GetClassInfo(Inst,"Steem Window",&wc)) 
  {
    UnregisterSteemControls();
    UnregisterButtonPicker();
    UnregisterClass("Steem Window",Inst);
    UnregisterClass("Steem Fullscreen Clip Window",Inst);
  }
  DBG_LOG("SHUTDOWN: Calling CoUninitialize()");
  CoUninitialize();
  for(int n=1;n<RC_NUM_ICONS;n++) 
    if(hGUIIcon[n]) 
      DestroyIcon(hGUIIcon[n]);
#endif//WIN32

#ifdef UNIX
// NOTE: Everything in this function MUST be checked for existance,
//       it can be called *before* MakeGUI!
  if (XD==NULL) return;

  if (StemWin){
		XAutoRepeatOn(XD);
    hxc::destroy_children_of(StemWin);
    XDestroyWindow(XD,StemWin);StemWin=0;
    hxc::free_res(XD);
  }
  if (DispGC){
    XFreeGC(XD,DispGC);DispGC=0;
  }
  if (EmptyCursor){
    XFreeCursor(XD,EmptyCursor);EmptyCursor=0;
  }
  Ico16.FreeIcons();
  Ico32.FreeIcons();
  Ico64.FreeIcons();
  IcoTOSFlags.FreeIcons();
  if (StemWinIconPixmap){
  	XFreePixmap(XD,StemWinIconPixmap);StemWinIconPixmap=0;
  }
  if (StemWinIconMaskPixmap){
	  XFreePixmap(XD,StemWinIconMaskPixmap);StemWinIconMaskPixmap=0;
	}
  if (colormap){
    XFreeColormap(XD,colormap);
    colormap=0;
  }
  hints.stop();
#endif//UNIX
}

#undef LOGSECTION




int GetScreenWidth() {
#ifdef WIN32
  return GuiSM.cx_screen(); // main screen
#endif

#ifdef UNIX
  static int Wid;
  if (XD){
    return (Wid=XDisplayWidth(XD,XDefaultScreen(XD)));
  }else{
    return Wid;
  }
#endif
}


int GetScreenHeight() {
#ifdef WIN32
  return GuiSM.cy_screen(); // main screen
#endif

#ifdef UNIX
  static int Height;
  if (XD){
    return (Height=XDisplayHeight(XD,XDefaultScreen(XD)));
  }else{
    return Height;
  }
#endif
}


char *FSTypes(int Type,...) {
  char *FileTypes=ansi_string;
  char *tp=FileTypes;
  ZeroMemory(FileTypes,256);
  if(Type==2)
  {
    strcpy(tp,T("Disk Images"));tp+=strlen(tp)+1;
    strcpy(tp,"*.st;*.stt;*.msa;*.dim;*.zip;*.stz");tp+=strlen(tp);
#ifdef RARLIB_SUPPORT
    strcpy(tp,";*.rar");tp+=strlen(tp);
#endif
#if defined(SSE_UNRAR_SUPPORT)
    if(UNRAR_OK)
      strcpy(tp,";*.rar");tp+=strlen(tp);
#endif
#if defined(SSE_ARCHIVEACCESS_SUPPORT) || defined(SSE_7Z_SUPPORT_UNIX)
#if defined(SSE_ARCHIVEACCESS_SUPPORT)
    if(ARCHIVEACCESS_OK) // or test at startup?
#endif
      strcpy(tp,";*.7z;*.bz2;*.gz;*.tar;*.arj");tp+=strlen(tp);
#endif
#if USE_PASTI
    if(hPasti) 
    {
      tp[0]=';';tp++;
      pasti->GetFileExtensions(tp,160,TRUE); // will add "*.st;*.stx"
      tp+=strlen(tp);
    }
#endif
    tp++;
  }
  else if(Type==3)
  {
    strcpy(tp,T("TOS Images"));tp+=strlen(tp)+1;
    strcpy(tp,"*.img;*.rom");tp+=strlen(tp)+1;
  }
  else
  {
    va_list vl;  
    va_start(vl,Type);
    char* arg;
    do
    {
      arg=va_arg(vl,char*);
      if(arg!=NULL)
      { // each time 2 strings
        strcpy(tp,arg);
        tp+=strlen(arg)+1;
        arg=va_arg(vl,char*);
        ASSERT(arg!=NULL);
        strcpy(tp,arg);
        tp+=strlen(arg)+1;
      }
    } while(arg!=NULL);
    va_end(vl);
  }
  if(Type) 
  {
    strcpy(tp,T("All Files"));tp+=strlen(tp)+1;
    strcpy(tp,"*.*");
  }
  //ASSERT(strlen(FileTypes)<256);
  return FileTypes;
}


bool CheckForSteemRunning() {
#ifdef WIN32
  SteemRunningMutex=CreateMutex(NULL,0,"Steem_Running");
  return GetLastError()==ERROR_ALREADY_EXISTS;
#endif
#ifdef UNIX
  return 0;
#endif
}


bool CleanupTempFiles() {
  bool SteemHasCrashed=0;
  for(int n=0;n<4;n++) {
    char *prefix=extension_list[EXT_MSA];
    switch(n) {
    case 1: prefix="ZIP"; break;
    case 2: prefix="FMT"; break;
    case 3: prefix="CRA"; break;
    }
    DirSearch ds;
    if(ds.Find(WriteDir+SLASH+prefix+"*.TMP")) 
    {
      EasyStringList FileESL;
      do {
        FileESL.Add(WriteDir+SLASH+ds.Name);
      } while(ds.Next());
      ds.Close();
      for(int i=0;i<FileESL.NumStrings;i++) 
        DeleteFile(FileESL[i].String);
      if(n==3) 
        SteemHasCrashed=true;
    }
  }
  return SteemHasCrashed;
}


int PeekEvent() {
#ifdef WIN32
  //ASSERT(!OPTION_EMUTHREAD);
  static MSG mess;
  if(PeekMessage(&mess,NULL,0,0,PM_REMOVE)==0) 
    return PEEKED_NOTHING;
  if(mess.message==WM_QUIT) 
  {
    QuitSteem();
    return PEEKED_QUIT;
  }
  if(HandleMessage(&mess)) 
  {
    TranslateMessage(&mess);
    DispatchMessage(&mess);
  }
  return PEEKED_MESSAGE;
#endif

#ifdef UNIX
  if (XD==NULL) return PEEKED_NOTHING;

  hxc::check_timers();
  if (XPending(XD)==0) return PEEKED_NOTHING;
  XEvent Ev;
  XNextEvent(XD,&Ev);
  
//XSync(XD,False);//SS  // ?
  
  return ProcessEvent(&Ev);
#endif
}


void SetStemMouseMode(int NewMM) {
#ifdef WIN32
  static POINT OldMousePos={-1,0};
  if(stem_mousemode!=STEM_MOUSEMODE_WINDOW && NewMM==STEM_MOUSEMODE_WINDOW) 
    GetCursorPos(&OldMousePos);
  stem_mousemode=NewMM;
  if(NewMM==STEM_MOUSEMODE_WINDOW) 
  {
    if(no_set_cursor_pos||OPTION_VMMOUSE)
    {
      SetCursor((no_set_cursor_pos)?LoadCursor(NULL,IDC_CROSS):NULL);
      POINT pt;
      GetCursorPos(&pt);
      window_mouse_centre_x=pt.x;
      window_mouse_centre_y=pt.y;
    }
    else 
    {
      SetCursor(NULL);
      RECT rc;
      GetWindowRect(StemWin,&rc);
      window_mouse_centre_x=rc.left+164+GuiSM.cx_frame();
      window_mouse_centre_y=rc.top+104+MENUHEIGHT+GuiSM.cy_frame()
        +GuiSM.cy_caption();
      SetCursorPos(window_mouse_centre_x,window_mouse_centre_y);
    }
#ifndef DEBUG_BUILD
    if(!OPTION_VMMOUSE)// we don't clip, mouse can exit window
    {
      if(FullScreen)
        ClipCursor(NULL);
      else
      {
        RECT rc;
        POINT pt={0,0};
        GetClientRect(StemWin,&rc);
        rc.right-=6;
        rc.bottom-=6+MENUHEIGHT;
        ClientToScreen(StemWin,&pt);
        OffsetRect(&rc,pt.x+3,pt.y+3+MENUHEIGHT);
        ClipCursor(&rc);
      }
    }
#endif
  }
  else 
  {
    SetCursor(PCArrow);
    if(FullScreen && runstate==RUNSTATE_RUNNING) 
      runstate=RUNSTATE_STOPPING;
#ifndef DEBUG_BUILD
    ClipCursor(NULL);
#endif
    if(!OPTION_VMMOUSE && OldMousePos.x>=0 && no_set_cursor_pos==0) 
    {
      SetCursorPos(OldMousePos.x,OldMousePos.y);
      OldMousePos.x=-1;
    }
  }
#endif

#ifdef UNIX
//  static POINT OldMousePos={-1,0};

//  if (stem_mousemode!=STEM_MOUSEMODE_WINDOW && NewMM==STEM_MOUSEMODE_WINDOW) GetCursorPos(&OldMousePos);
  stem_mousemode=NewMM;

  if (XD==NULL) return;

  if (NewMM==STEM_MOUSEMODE_WINDOW){
#ifdef CYGWIN
    POINT pt;
    GetCursorPos(&pt);
    window_mouse_centre_x=pt.x;
    window_mouse_centre_y=pt.y;
    XGrabPointer(XD,StemWin,0,0,GrabModeAsync,GrabModeAsync,
                  None,EmptyCursor,CurrentTime);
#else
    if(OPTION_VMMOUSE)
    {
      POINT pt;
      GetCursorPos(&pt);
      window_mouse_centre_x=pt.x;
      window_mouse_centre_y=pt.y;
      XGrabPointer(XD,StemWin,0,0,GrabModeAsync,GrabModeAsync,None,EmptyCursor,
        CurrentTime);
    }
    else
    {
      window_mouse_centre_x=164;
      window_mouse_centre_y=MENUHEIGHT+104;
      XGrabPointer(XD,StemWin,0,0,GrabModeAsync,GrabModeAsync,StemWin,EmptyCursor,
        CurrentTime);
      XWarpPointer(XD,None,StemWin,0,0,0,0,window_mouse_centre_x,
        window_mouse_centre_y);
    }
#endif
  }else{
    if (FullScreen && runstate==RUNSTATE_RUNNING) runstate=RUNSTATE_STOPPING;
    XUngrabPointer(XD,CurrentTime);
/*
    if (OldMousePos.x>=0){
      SetCursorPos(OldMousePos.x,OldMousePos.y);
      OldMousePos.x=-1;
    }
*/
  }
#endif
  mouse_move_since_last_interrupt_x=0;
  mouse_move_since_last_interrupt_y=0;
  mouse_change_since_last_interrupt=false;
}


void ShowAllDialogs(bool Show) {
#ifdef WIN32
  if(FullScreen==0) 
    return;
  static bool DiskManWasMaximized=0;
  int PosChange=int(Show?-3000:3000);
  if(DiskMan.Handle) 
  {
    if(DiskMan.FSMaximized && Show==0) 
      DiskManWasMaximized=true;
    if(DiskManWasMaximized && Show) 
    {
      SetWindowPos(DiskMan.Handle,NULL,-GuiSM.cx_frame(),MENUHEIGHT,
        Disp.SurfaceWidth+GuiSM.cx_frame()*2,Disp.SurfaceHeight+GuiSM.cy_frame()-MENUHEIGHT,
        SWP_NOZORDER|SWP_NOACTIVATE);
      DiskManWasMaximized=0;
    }
    else 
    {
      DiskMan.FSLeft+=PosChange;
      SetWindowPos(DiskMan.Handle,NULL,DiskMan.FSLeft,DiskMan.FSTop,0,0,
        SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }
  }
  for(int n=0;n<nStemDialogs;n++) 
  {
    if(DialogList[n]!=&DiskMan) 
    {
      if(DialogList[n]->Handle) 
      {
        DialogList[n]->FSLeft+=PosChange;
        SetWindowPos(DialogList[n]->Handle,NULL,DialogList[n]->FSLeft,
          DialogList[n]->FSTop,0,0,SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
      }
    }
  }
#endif
}


//401R5 ref. unix/win
void HandleKeyPress(UINT VKCode,DWORD Up,int Extended) { 
  if(disable_input_vbl_count) 
    return;
  if(!OPTION_C1 && ikbd_keys_disabled())
    return; //in duration mode
  BYTE STCode=0;
  bool DidShiftSwitching=false;

#ifdef WIN32
  if(macro_play_has_keys) 
    return;
  if((Extended&3)==1) 
  {
    switch(LOBYTE(VKCode)) {
    case VK_RETURN: 
      STCode=STKEY_PAD_ENTER;
      break;
    case VK_DIVIDE:
      STCode=STKEY_PAD_DIVIDE;
      break;
    }
  }
#endif

  int ModifierRestoreArray[3];
  if(EnableShiftSwitching&&shift_key_table[0]&&(Extended & NO_SHIFT_SWITCH)==0)
  {
    if(STCode==0) // can be different in Win32
    {
      HandleShiftSwitching(VKCode,Up,STCode,ModifierRestoreArray);
      if(STCode) 
        DidShiftSwitching=true;
    }
  }
  if(STCode==0)
    STCode=key_table[BYTE(VKCode)];

  if(STCode)
  {
    // If we're sending key combinations, we don't want 6301 to see the key
    if(!DidShiftSwitching||!OPTION_C1)
      ST_Key_Down[STCode]=!Up; // this is used by ikbd.cpp & ireg.c

#ifdef WIN32
#if defined(SSE_DEBUG)
#undef LOGSECTION
#define LOGSECTION LOGSECTION_IKBD
    TRACE_LOG("%d %d Key PC $%X ST $%X ",FRAME,scan_y,VKCode,STCode);
    TRACE_LOG((Up)?"-\n":"+\n");
#undef LOGSECTION
#endif
#if defined(SSE_OSD_DEBUGINFO)
    if(OPTION_OSD_DEBUGINFO && (acia[0].overrun) && !Up)
      TRACE_OSD2("OVR");
#endif
#endif//WIN32

    if(Up) // The break code for each key is obtained by ORing 0x80 with the make code:
    {
      STCode|=MSB_B; // MSB_B = $80
#if defined(SSE_STATS)
      Stats.nKeyIn++;
#endif
    }
    if(OPTION_C1&&!DidShiftSwitching)
    {
      //We don't write in a buffer, 6301 emu will do it after having scanned
      //ST_Key_Down.
      if(macro_record)
        macro_record_key(STCode);
      //if(DidShiftSwitching)
      //  keyboard_buffer_write(STCode); //must send ourselves to ACIA
    }
    else
      keyboard_buffer_write_n_record(STCode);

#ifndef DISABLE_STEMDOS //control-C
    if(VKCode==WIN_ONLY('C') UNIX_ONLY(XK_c) && ST_Key_Down[key_table[VK_CONTROL]])
      stemdos_control_c();
#endif

  }//if(STCode)
  if(DidShiftSwitching)
    ShiftSwitchRestoreModifiers(ModifierRestoreArray);
}


#ifdef WIN32

void EnableWindow2(HWND Win,bool Enable,HWND NoDisable) {
  if(Win!=NoDisable) 
    SetWindowLong(Win,GWL_STYLE,(GetWindowLong(Win,GWL_STYLE) 
      & int(Enable ? ~WS_DISABLED:0xffffffff)) | int(Enable==0 ? WS_DISABLED:0));
}


void EnableAllWindows(bool Enable,HWND NoDisable) {
  if(Enable)
    DisableFocusWin=NULL;
  else
    DisableFocusWin=NoDisable;
  DEBUG_ONLY( EnableWindow2(DWin,Enable,NoDisable) );
  DEBUG_ONLY(if(trace_window_handle) EnableWindow2(trace_window_handle,Enable,NoDisable));
  EnableWindow2(StemWin,Enable,NoDisable);
  if(DiskMan.Handle) 
  {
    if(HardDiskMan.Handle)
      EnableWindow2(HardDiskMan.Handle,Enable,NoDisable);
    else if(DiskMan.VisibleDiag()==NULL)
      EnableWindow2(DiskMan.Handle,Enable,NoDisable);
    else
      EnableWindow2(DiskMan.VisibleDiag(),Enable,NoDisable);
  }
  for(int n=0;n<nStemDialogs;n++) 
  {
    if(DialogList[n]!=&DiskMan) 
      if(DialogList[n]->Handle) 
        EnableWindow2(DialogList[n]->Handle,Enable,NoDisable);
  }
}


#define WH_KEYBOARD_LL 13
#if (_WIN32_WINNT < 0x0400)
#define LLKHF_ALTDOWN 0x00000020
#endif
#if !defined(MINGW_BUILD) && (_WIN32_WINNT < 0x0400) // well well
typedef struct{
  DWORD vkCode;
  DWORD scanCode;
  DWORD flags;
  DWORD time;
  DWORD dwExtraInfo;
}KBDLLHOOKSTRUCT, *LPKBDLLHOOKSTRUCT;
#endif

LRESULT CALLBACK NTKeyboardProc(INT nCode,WPARAM wParam,LPARAM lParam) {
  KBDLLHOOKSTRUCT *pkbhs=LPKBDLLHOOKSTRUCT(lParam);
  if(nCode==HC_ACTION)
  {
    bool ControlDown=(GetAsyncKeyState(VK_CONTROL)<0),
      AltDown=(pkbhs->flags & LLKHF_ALTDOWN)!=0;
    bool ShiftDown=(GetAsyncKeyState(VK_SHIFT) < 0);
    if(pkbhs->vkCode==VK_TAB && AltDown) return 1;
    if(pkbhs->vkCode==VK_ESCAPE&&(AltDown||ShiftDown||ControlDown)) return 1;
    if(pkbhs->vkCode==VK_DELETE && AltDown && ControlDown) return 1;
#ifdef ONEGAME
    if(pkbhs->vkCode==VK_LWIN||pkbhs->vkCode==VK_RWIN) return 1;
#endif
  }
  return CallNextHookEx(hNTTaskSwitchHook,nCode,wParam,lParam);
}


void DisableTaskSwitch() {
  if(TaskSwitchDisabled)
    return;
  if(WinNT) 
  {
    hNTTaskSwitchHook=SetWindowsHookEx(WH_KEYBOARD_LL,HOOKPROC(NTKeyboardProc),NULL,GetCurrentThreadId());
    if(hNTTaskSwitchHook==NULL) 
    {
      int Base=1400;
      RegisterHotKey(StemWin,Base++,MOD_ALT,VK_TAB);
      RegisterHotKey(StemWin,Base++,MOD_ALT|MOD_SHIFT,VK_TAB);
      RegisterHotKey(StemWin,Base++,MOD_ALT,VK_ESCAPE);
      RegisterHotKey(StemWin,Base++,MOD_ALT|MOD_SHIFT,VK_ESCAPE);
      RegisterHotKey(StemWin,Base++,MOD_CONTROL,VK_ESCAPE);
      RegisterHotKey(StemWin,Base++,MOD_CONTROL|MOD_ALT,VK_ESCAPE);
      RegisterHotKey(StemWin,Base++,MOD_CONTROL|MOD_SHIFT,VK_ESCAPE);
      RegisterHotKey(StemWin,Base++,MOD_CONTROL|MOD_ALT|MOD_SHIFT,VK_ESCAPE);
      RegisterHotKey(StemWin,Base++,MOD_CONTROL|MOD_ALT,VK_DELETE);
      RegisterHotKey(StemWin,Base++,MOD_CONTROL|MOD_ALT|MOD_SHIFT,VK_DELETE);
      RegisterHotKey(StemWin,Base++,MOD_SHIFT,VK_ESCAPE);
#ifdef ONEGAME
      RegisterHotKey(StemWin,Base++,0,VK_LWIN);
      RegisterHotKey(StemWin,Base++,0,VK_RWIN);
#endif
    }
  }
  else 
  {
    UINT PrevSS;
    SystemParametersInfo(SPI_SETSCREENSAVERRUNNING,TRUE,&PrevSS,0);
  }
  TaskSwitchDisabled=true;
}


void EnableTaskSwitch() {
  if(TaskSwitchDisabled==0) 
    return;
  if(WinNT) 
  {
    if(hNTTaskSwitchHook==NULL)
      for(int n=1400;n<1411 ONEGAME_ONLY(+2);n++) 
        UnregisterHotKey(StemWin,n);
  }
  else 
  {
    UINT PrevSS;
    SystemParametersInfo(SPI_SETSCREENSAVERRUNNING,FALSE,&PrevSS,0);
  }
  if(hNTTaskSwitchHook) 
  {
    UnhookWindowsHookEx(hNTTaskSwitchHook);
    hNTTaskSwitchHook=NULL;
  }
  TaskSwitchDisabled=0;
}


void UpdatePasteButton() {
#ifdef RELEASE_BUILD
  // Only have automatic updating of paste button if final build
  if(PasteText.Empty())
    EnableWindow(GetDlgItem(StemWin,114),IsClipboardFormatAvailable(CF_TEXT));
#else
  EnableWindow(GetDlgItem(StemWin,114),true);
#endif
}


#if !defined(RELEASE_BUILD) && defined(DEBUG_BUILD)
bool HWNDNotValid(HWND Win,char *File,int Line) {
  bool Err=0;
  if(Win==NULL)
    Err=true;
  else if(IsWindow(Win)==0)
    Err=true;
  if(Err)
  {
    Alert(Str("WINDOWS: Arrghh, using ")+long(Win)+" as HWND in file "+File+" at line "+Line,"Window Handle Error",MB_ICONEXCLAMATION);
    return true;
  }
  return 0;
}

LRESULT SendMessage_checkforbugs(HWND Win,UINT Mess,WPARAM wPar,
                                LPARAM lPar,char *File,int Line) {
  if(HWNDNotValid(Win,File,Line)) 
    return 0;
  return SendMessageA(Win,Mess,wPar,lPar);
}

BOOL PostMessage_checkforbugs(HWND Win,UINT Mess,WPARAM wPar,
                              LPARAM lPar,char *File,int Line) {
  if(HWNDNotValid(Win,File,Line)) 
    return 0;
  return PostMessageA(Win,Mess,wPar,lPar);
}

#endif

#endif //WIN32


int Alert(char *Mess,char *Title,UINT Flags) {
  WIN_ONLY( HWND Win=GetActiveWindow(); )
  Disp.FlipToDialogsScreen();
  log_write(EasyStr(Title)+": "+Mess);
  int Ret=MessageBox(WINDOWTYPE(FullScreen ? StemWin:0),Mess,Title,
    Flags | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
  WIN_ONLY( SetActiveWindow(Win); )
  return Ret;
}


// Shift and alt switching, use 4 tables of 256 WORDs:
//  table 0 = no shift, no alt
//  table 1 = shift, no alt
//  table 2 = no shift, alt
//  table 3 = shift, alt
// Look up our key (VKCode) in the table, if it isn't 0 then shift/alt switching
// must be performed. The LOBYTE contains the ST key code, the high byte has
// bit 0 set if shift should be down when the key is sent and bit 1 for alt to
// be down. Before the key is pressed we must change the ST shift and alt states
// to what we need by sending releasing/press IKBD messages for those keys.
// After the key has been sent we then send more messages to restore them to
// their former glory. Surprisingly it works great, except if you try to do any
// key repeat.

void ShiftSwitchChangeModifiers(bool ShiftShouldBePressed,bool AltShouldBePressed,
                                int ModifierRestoreArray[3]) {
  // Get current states
  BYTE STLShiftDown=(ST_Key_Down[key_table[VK_LSHIFT]]);
  BYTE STRShiftDown=(ST_Key_Down[key_table[VK_RSHIFT]]);
  int STAltDown=(ST_Key_Down[key_table[VK_MENU]] ? BIT_1:0);
  if((STLShiftDown||STRShiftDown)&&ShiftShouldBePressed==0) 
  {
    // Send Shift Up Messages
    if(STLShiftDown) 
    {
      keyboard_buffer_write_n_record(key_table[VK_LSHIFT]|MSB_B);
      ModifierRestoreArray[0]=1; //Lshift down
    }
    if(STRShiftDown) {
      keyboard_buffer_write_n_record(key_table[VK_RSHIFT]|MSB_B);
      ModifierRestoreArray[1]=1; //Rshift down
    }
  }
  else if((STLShiftDown||STRShiftDown)==0&&ShiftShouldBePressed) 
  {
    // Send Shift Down Message
    keyboard_buffer_write_n_record(key_table[VK_LSHIFT]);
    ModifierRestoreArray[0]=2; //Lshift up
  }
  if(STAltDown && AltShouldBePressed==0) 
  {
    // Send Alt Up Messages
    keyboard_buffer_write_n_record(key_table[VK_MENU]|MSB_B);
    ModifierRestoreArray[2]=1; //Alt down
  }
  else if(STAltDown==0&&AltShouldBePressed) 
  {
    // Send Alt Down Message
    keyboard_buffer_write_n_record(key_table[VK_MENU]);
    ModifierRestoreArray[2]=2; //Alt up
  }
}


void ShiftSwitchRestoreModifiers(int ModifierRestoreArray[3]) {
  if(ModifierRestoreArray[0]==1) 
    keyboard_buffer_write_n_record(key_table[VK_LSHIFT]);
  if(ModifierRestoreArray[0]==2) 
    keyboard_buffer_write_n_record(key_table[VK_LSHIFT]|MSB_B);
  if(ModifierRestoreArray[1]==1) 
    keyboard_buffer_write_n_record(key_table[VK_RSHIFT]);
  if(ModifierRestoreArray[2]==1) 
    keyboard_buffer_write_n_record(key_table[VK_MENU]);
  if(ModifierRestoreArray[2]==2) 
    keyboard_buffer_write_n_record(key_table[VK_MENU]|MSB_B);
}


void HandleShiftSwitching(UINT VKCode,DWORD Up,BYTE &STCode,
                          int ModifierRestoreArray[3]) {
  //ASSERT(VKCode==VK_MENU);
  // These are set to tell the HandleKeyPress routine what to do after it has
  // sent the key.
  ModifierRestoreArray[0]=0;  // LShift
  ModifierRestoreArray[1]=0;  // RShift
  ModifierRestoreArray[2]=0;  // Alt (only one on ST)
  // Don't need to do anything when you release the key
  if(shift_key_table[0]==NULL) 
    return;
  // Get ST code and required modifier states
  int Shift,Alt;
  if(Up==0) 
  { // Pressing key
    // Get current state of modifiers
    Shift=int((ST_Key_Down[key_table[VK_LSHIFT]]||ST_Key_Down[key_table[VK_RSHIFT]])?BIT_0:0);
    Alt=int(ST_Key_Down[key_table[VK_MENU]] ? BIT_1 : 0);
  }
  else 
  { // Releasing key
   // Get state of modifiers when key was pressed
    Shift=(KeyDownModifierState[BYTE(VKCode)]&BIT_0);
    Alt=(KeyDownModifierState[BYTE(VKCode)]&BIT_1);
  }
  WORD KeyEntry=shift_key_table[Shift | Alt][BYTE(VKCode)];
  STCode=LOBYTE(KeyEntry);
  KeyDownModifierState[BYTE(VKCode)]=BYTE(Shift | Alt);
  if(STCode && Up==0) 
  {
    bool ShiftShouldBePressed=(HIBYTE(KeyEntry) & BIT_0)!=0;
    bool AltShouldBePressed=(HIBYTE(KeyEntry) & BIT_1)!=0;
    ShiftSwitchChangeModifiers(ShiftShouldBePressed,AltShouldBePressed,ModifierRestoreArray);
  }
}


void PasteIntoSTAction(int Action) {
  if(Action==STPASTE_STOP||Action==STPASTE_TOGGLE) 
  {
    if(PasteText.NotEmpty()) 
    {
      PasteText="";
      PasteVBLCount=0;
      WIN_ONLY(SendDlgItemMessage(StemWin,114,BM_SETCHECK,0,0); )
      UNIX_ONLY(PasteBut.set_check(0); )
      return;
    }
    else if(Action==STPASTE_STOP) 
      return;
  }
#ifdef WIN32
  if(IsClipboardFormatAvailable(CF_TEXT)==0) 
    return;
  if(OpenClipboard(StemWin)==0) 
    return;
  HGLOBAL hGbl=GetClipboardData(CF_TEXT);
  if(hGbl) 
  {
    PasteText=(char*)GlobalLock(hGbl);
    PasteVBLCount=PasteSpeed;
    SendDlgItemMessage(StemWin,114,BM_SETCHECK,1,0);
    GlobalUnlock(hGbl);
  }
  CloseClipboard();
#endif
#ifdef UNIX
  Window SelectionOwner=XGetSelectionOwner(XD,XA_PRIMARY);
  if(SelectionOwner!=None) {
    XEvent SendEv;
    SendEv.type=SelectionRequest;
    SendEv.xselectionrequest.requestor=StemWin;
    SendEv.xselectionrequest.owner=SelectionOwner;
    SendEv.xselectionrequest.selection=XA_PRIMARY;
    SendEv.xselectionrequest.target=XA_STRING;
    SendEv.xselectionrequest.property=XA_CUT_BUFFER0;
    SendEv.xselectionrequest.time=CurrentTime;
    XSendEvent(XD,SelectionOwner,0,0,&SendEv);
    // PasteText,PasteVBLCount and PasteBut are set up
    // in SelectionNotify event handler
	}
#endif
}


void PasteVBL() {
  if(PasteText.NotEmpty()) 
  {
    if((--PasteVBLCount)<=0) 
    {
      // Convert to ST Ascii
      BYTE c=BYTE(PasteText[0]);
      if(c>127) 
      {
        c=STCharToPCChar[c-128];
        if(c) PasteText[0]=char(c);
      }
      // Go through every character TOS can produce to find it
      switch(c) {
      case '\r': break; // Only need line feeds
      case '\n':
        keyboard_buffer_write_n_record(0x1c);
        keyboard_buffer_write_n_record(BYTE(0x1c|BIT_7));
        break;
      case '\t':
        keyboard_buffer_write_n_record(0x0f);
        keyboard_buffer_write_n_record(BYTE(0x0f|BIT_7));
        break;
      case ' ':
        keyboard_buffer_write_n_record(0x39);
        keyboard_buffer_write_n_record(BYTE(0x39|BIT_7));
        break;
      default:
        DynamicArray<DWORD> Chars;
        GetAvailablePressChars(&Chars);
        for(int n=0;n<Chars.NumItems;n++) 
        {
          if(HIWORD(Chars[n])==c) 
          {
            // Now fix shift/alt and press the key
            int ModifierRestoreArray[3]={0,0,0};
            BYTE STCode=LOBYTE(LOWORD(Chars[n]));
            BYTE Modifiers=HIBYTE(LOWORD(Chars[n]));
            ShiftSwitchChangeModifiers((Modifiers & BIT_0)!=0,
              (Modifiers & BIT_1)!=0,ModifierRestoreArray);
            keyboard_buffer_write_n_record(STCode);
            keyboard_buffer_write_n_record(BYTE(STCode|BIT_7));
            ShiftSwitchRestoreModifiers(ModifierRestoreArray);
            break;
          }
        }
      }
      PasteText.Delete(0,1);
      if(PasteText.NotEmpty())
        PasteVBLCount=PasteSpeed;
      else 
      {
        PasteText=""; // Release some memory
        WIN_ONLY(SendDlgItemMessage(StemWin,114,BM_SETCHECK,0,0); )
        UNIX_ONLY(PasteBut.set_check(0); )
      }
    }
  }
}


void UpdateSTKeys() {
  for(int n=0;n<128;n++) 
  {
    if(ST_Key_Down[n]) 
    {
      if(!OPTION_C1)
        keyboard_buffer_write(BYTE(n|BIT_7));
      ST_Key_Down[n]=0;
    }
  }
}


#if defined(SSE_GUI_TOOLBAR)
/*  A floating toolbar instead of being part of the client area.
    This is only enabled for the D3D FlipEx build.
    We derive from TStemDialog.
*/

TToolbar::TToolbar() {
  Handle=NULL;
  Section="Toolbar";
}


TToolbar::~TToolbar() {
  Hide();
}


void TToolbar::ManageWindowClasses(bool Unreg) {
  char *ClassName="Steem Toolbar";
  if(Unreg)
    UnregisterClass(ClassName,Inst);
  else
    RegisterMainClass(WndProc,ClassName,RC_ICO_APP);
}


void TToolbar::Show() {
  if(Handle!=NULL) 
  {
    if(IsIconic(Handle)) 
      ShowWindow(Handle,SW_SHOWNORMAL);
    SetForegroundWindow(Handle);
    return;
  }
  ManageWindowClasses(SD_REGISTER);
  int Width,Height;
  const BYTE spacing=23, size=20;
#if defined(SSE_DEBUGGER_TOGGLE)
  const BYTE extra=1;
#else
  const BYTE extra=0;
#endif
  int dx,dy;
  if(OPTION_TOOLBAR_VERTICAL)
    Width=25,Height=GuiSM.cy_caption()+(13+extra)*25,dx=0,dy=spacing;
  else
    Width=(13+extra)*25,Height=50,dx=spacing,dy=0;
  Handle=CreateWindowEx((OPTION_TOOLBAR_TASKBAR ? 0: WS_EX_TOOLWINDOW),
    "Steem toolbar",OPTION_TOOLBAR_VERTICAL ? "" : "Steem tool bar",
    0,Left,Top, Width,Height,ParentWin,NULL,Inst,NULL);
  if(HandleIsInvalid())
  {
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }
  SetWindowLongPtr(Handle,GWLP_USERDATA,(LONG_PTR)this);
#define StemWin Handle
#ifndef ONEGAME
  int x=0,y=0;
#if 1
  HWND Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_RESET),WS_CHILDWINDOW
    |WS_VISIBLE|PBS_RIGHTCLICK,x,y,size,size,StemWin,(HMENU)IDC_RESET,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Reset (Left Click = Warm, Right Click = Cold)"));
  x+=dx,y+=dy;
#else
  HWND
#endif
    Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_PLAY),WS_CHILDWINDOW|
    WS_VISIBLE|WS_TABSTOP|PBS_RIGHTCLICK,x,y,size,size,StemWin,(HMENU)IDC_PLAY,Inst,
    NULL);
#endif
  ToolAddWindow(ToolTip,Win,
    T("Run (Left Click = Run/Stop, Right Click = Slow Motion)"));
  x+=dx,y+=dy;
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_FF),WS_CHILDWINDOW
    |WS_VISIBLE|PBS_RIGHTCLICK|PBS_DBLCLK,x,y,size,size,StemWin,
    (HMENU)IDC_FASTFORWARD,Inst,NULL);
  ToolAddWindow(ToolTip,Win,
    T("Fast Forward (Right Click = Searchlight, Double Click = Sticky)"));
  x+=dx,y+=dy;
#if 0
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_RESET),WS_CHILDWINDOW
    |WS_VISIBLE|PBS_RIGHTCLICK,x,y,size,size,StemWin,(HMENU)IDC_RESET,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Reset (Left Click = Warm, Right Click = Cold)"));
  x+=dx,y+=dy;
#endif
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_SNAPSHOTBUT),WS_CHILDWINDOW
    |WS_VISIBLE,x,y,size,size,StemWin,(HMENU)IDC_SNAPSHOT,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Memory Snapshot Menu"));
  x+=dx,y+=dy;
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_TAKESCREENSHOTBUT),
    WS_CHILDWINDOW|WS_VISIBLE|PBS_RIGHTCLICK,x,y,size,size,StemWin,
    (HMENU)IDC_SCREENSHOT,Inst,NULL);
  ToolAddWindow(ToolTip,Win,
    T("Take Screenshot")+" ("+T("Right Click = Options")+")");
  x+=dx,y+=dy;
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_PASTE),WS_CHILD|WS_VISIBLE
    |PBS_RIGHTCLICK,x,y,size,size,StemWin,(HMENU)IDC_PASTE,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Paste Text Into ST (Right Click = Options)"));
  x+=dx,y+=dy;
#ifdef RELEASE_BUILD //(Steem authors probably used Borland debugger)
  // This causes freeze up if tracing in debugger, so only do it in final build
  NextClipboardViewerWin=SetClipboardViewer(StemWin);
#endif
  UpdatePasteButton();
#if !defined(SSE_NO_UPDATE)
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_UPDATE),WS_CHILD,
                          x,y,size,size,StemWin,(HMENU)120,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Steem Update Available! Click Here For Details!"));
#endif
#if defined(SSE_GUI_CONFIG_FILE)
  // 'wrench' icon for config files, popup menu when left click
  // it only doubles the feature in Option box but I didn't know!
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_CFG),WS_CHILDWINDOW|
    WS_VISIBLE,x,y,size,size,StemWin,(HMENU)IDC_CONFIGS,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Load/save configuration file"));
  x+=dx,y+=dy;
#endif
#if 0 // status bar is on menu bar in this build
  Win=CreateWindowEx(0,"Static",WINDOW_TITLE,WS_CHILD|WS_VISIBLE|SS_OWNERDRAW,
    x,y,50,size,StemWin,(HMENU)IDC_STATUS_BAR,Inst,NULL); 
#endif
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_PATCHES),WS_CHILD|WS_VISIBLE,
    x,y,size,size,StemWin,(HMENU)IDC_PATCHES,Inst,NULL);
  x+=dx,y+=dy;
  ToolAddWindow(ToolTip,Win,T("Patches"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_SHORTCUT),WS_CHILD|WS_VISIBLE,
    x,y,size,size,StemWin,(HMENU)IDC_SHORTCUTS,Inst,NULL);
  x+=dx,y+=dy;
  ToolAddWindow(ToolTip,Win,T("Shortcuts"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_OPTIONS),WS_CHILD|WS_VISIBLE,
    x,y,size,size,StemWin,(HMENU)IDC_OPTIONS,Inst,NULL);
  x+=dx,y+=dy;
  ToolAddWindow(ToolTip,Win,T("Options"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_JOY),WS_CHILDWINDOW|WS_VISIBLE,
    x,y,size,size,StemWin,(HMENU)IDC_JOYSTICKS,Inst,NULL);
  x+=dx,y+=dy;
  ToolAddWindow(ToolTip,Win,T("Joystick Configuration"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_DISKMAN),WS_CHILDWINDOW
    |WS_VISIBLE,x,y,size,size,StemWin,(HMENU)IDC_DISK_MANAGER,Inst,NULL);
  x+=dx,y+=dy;
  ToolAddWindow(ToolTip,Win,T("Disk Manager"));
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_INFO),WS_CHILD|WS_VISIBLE,
    x,y,size,size,StemWin,(HMENU)IDC_INFO,Inst,NULL);
  x+=dx,y+=dy;
  ToolAddWindow(ToolTip,Win,T("General Info"));
#if defined(SSE_DEBUGGER_TOGGLE)
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_BOMB),WS_CHILD|WS_VISIBLE,
    x,y,size,size,StemWin,(HMENU)IDC_DEBUGGER,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Debugger"));
  x+=dx,y+=dy;
#endif
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_FULLQUIT),WS_CHILD
    |WS_VISIBLE,x,y,size,size,StemWin,(HMENU)116,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Close tool bar"));
#if 0
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_TOWINDOW),WS_CHILD,
    120,0,size,size,StemWin,(HMENU)106,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Windowed Mode"));
  Win=CreateWindow("Steem Fullscreen Quit Button","",WS_CHILD,
    120,0,size,size,StemWin,(HMENU)116,Inst,NULL);
  ToolAddWindow(ToolTip,Win,T("Quit Steem"));
#endif
  VERIFY(!ShowWindow(Handle,SW_SHOW));
  SetWindowAndChildrensFont(StemWin,fnt);
#undef StemWin
  VERIFY(PostMessage(StemWin,WM_USER,1234,0)); // -> update button state
}


void TToolbar::Hide() {
  if(Handle)
  {
    VERIFY(ShowWindow(Handle,SW_HIDE));
    VERIFY(DestroyWindow(Handle));
    Handle=NULL;
  }
  ManageWindowClasses(SD_UNREGISTER);
}


#define GET_THIS This=(TToolbar*)GetWindowLongPtr(Win,GWLP_USERDATA);

LRESULT CALLBACK TToolbar::WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar) {
  TToolbar *This;
  GET_THIS;
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if(StemDialog_RetDefVal) 
    return Ret;
  switch(Mess) {
  case WM_COMMAND:
    if(LOWORD(wPar)==116 && HIWORD(wPar)==BN_CLICKED)
    {
      This->Hide();
      return 0;
    }
    // we can't move code, there are various builds, so we send the message
    // to where it will be interpreted
    return SendMessage(StemWin,Mess,wPar,lPar);
    break;
  case WM_PAINT:
  {
    RECT dest;
    GetClientRect(Win,&dest);
    PAINTSTRUCT ps;
    BeginPaint(Win,&ps);
    HBRUSH br=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(ps.hdc,&dest,br);
    DeleteObject(br);
    return 0;
  }
  case WM_GETMINMAXINFO:
    ((MINMAXINFO*)lPar)->ptMinTrackSize.x=30;//50;
    ((MINMAXINFO*)lPar)->ptMinTrackSize.y=50;
    break;
  case WM_CLOSE:
    This->Hide();
    return 0;
  }//sw
  return DefWindowProc(Win,Mess,wPar,lPar);
}


TToolbar ToolBar;

#endif


#ifdef UNIX

//---------------------------------------------------------------------------
int HandleXError(Display *XD,XErrorEvent *pXErr)
{
  XError=*pXErr;
  log_write("X Error, oh dear, things may be going wrong.");
  char ErrText[300]={0};
  XGetErrorText(XD,XError.type,ErrText,299);
  if (ErrText[0]) log_write(ErrText);
#if defined(SSE_UNIX_TRACE)
  if (ErrText[0]) 
    TRACE(ErrText); // newline?
#endif
  return 0;
}
//---------------------------------------------------------------------------
void InitColoursAndIcons()
{
  int Scr=XDefaultScreen(XD);
  if (XDefaultDepth(XD,Scr)==8){ // Oh no! 8-bit!
    XVisualInfo vith;
    //I want to set the member called "class".  But gcc
    //doesn't like members with reserved words for names!
    *((&(vith.depth))+1)=PseudoColor;
    int how_many=0;
    XVisualInfo *vi=XGetVisualInfo(XD,VisualClassMask,&vith,&how_many);
    if (how_many){
      colormap=XCreateColormap(XD,XDefaultRootWindow(XD),vi->visual,AllocAll);
      XFree(vi);

      for (int n=0;n<257;n++){
        logpal[n]=0;
        new_pal[n].pixel=n;
        if (n<256){
        	XQueryColor(XD,XDefaultColormap(XD,XDefaultScreen(XD)),new_pal+n);
  	      XStoreColor(XD,colormap,new_pal+n);
  	    }
        new_pal[n].flags=DoRed | DoGreen | DoBlue;
      }
  		
      for(int n=0;n<8;n++){
      	logpal[n]=0xffffffff;
      }
  		XColor c;
  		for (int n=0;n<18;n++){
  			int nn=standard_palette[n][0];
  			int col=standard_palette[n][1];
  			logpal[nn]=0xffffffff;
    	  c.flags=DoRed | DoGreen | DoBlue;
  			c.pixel=nn;
  			c.red=(col & 0xff0000) >> 8;
  			c.green=(col & 0xff00);
  			c.blue=(col & 0xff) << 8;
  	    XStoreColor(XD,colormap,&c);
  	    new_pal[nn]=c;
  		}
  		IconGroup::ColList=(long(*)[2])standard_palette;
  		IconGroup::ColListLen=18;
			hxc::alloc_colours_vector=steem_hxc_alloc_colours;
			hxc::free_colours_vector=steem_hxc_free_colours;
  	}
  	
    WhiteCol=255;
    BlackCol=0;
    BkCol=13;
    BorderLightCol=14;
    BorderDarkCol=254;
	}else{
    WhiteCol=WhitePixel(XD,Scr);
    BlackCol=BlackPixel(XD,Scr);
    BkCol=GetColourValue(XD,192 << 8,192 << 8,192 << 8,WhiteCol);
    BorderLightCol=GetColourValue(XD,224 << 8,224 << 8,224 << 8,WhiteCol);;
    BorderDarkCol=GetColourValue(XD,128 << 8,128 << 8,128 << 8,BlackCol);
  }//SS 8bit

  cWinProc=XUniqueContext();
  cWinThis=XUniqueContext();

  Ico16.LoadIconsFromMemory(XD,Get_icon16_bmp(),16);
  Ico32.LoadIconsFromMemory(XD,Get_icon32_bmp(),32);
  Ico64.LoadIconsFromMemory(XD,Get_icon64_bmp(),64);
  IcoTOSFlags.LoadIconsFromMemory(XD,Get_tos_flags_bmp(),RC_FLAG_WIDTH);
  hxc_button::pcheck_ig=&Ico16;
  hxc_button::check_on_icon=ICO16_TICKED;
  hxc_button::check_off_icon=ICO16_UNTICKED;

  fileselect.set_alert_box_icons(&Ico32,&Ico16);
  fileselect.lpig=&Ico16;

  hints.XD=XD;
}

void steem_hxc_alloc_colours(Display*)
{
  hxc::col_black=0;
  hxc::col_white=255;
  hxc::col_grey=13;
  hxc::col_border_dark=254;
  hxc::col_border_light=14;
  hxc::col_sel_back=247;
  hxc::col_sel_fore=255;
  hxc::col_bk=13;
  hxc::colormap=colormap;
}

void steem_hxc_free_colours(Display*){
}



//---------------------------------------------------------------------------
void steem_hxc_modal_notify(bool going)
{
  // Warning: This could be called at any time! As we only need to do anything
  // when running it is safe.
  if (runstate!=RUNSTATE_RUNNING) return;

  if (going){
    Sound_Stop();
  }else{
    Sound_Start();
  }
}


void XGUIUpdatePortDisplay()
{
  if (OptionBox.IsVisible()){
    // Update open buttons
    for (int p=0;p<3;p++) OptionBox.UpdatePortDisplay(p);
  }
}
//---------------------------------------------------------------------------
void PostRunMessage()
{
  if (XD==NULL || StemWin==0) return;

  XEvent SendEv;
  SendEv.type=ClientMessage;
  SendEv.xclient.window=StemWin;
  SendEv.xclient.message_type=RunSteemAtom;
  SendEv.xclient.format=32;
  XSendEvent(XD,StemWin,0,0,&SendEv);
}


void PrintHelpToStdout()
{
  printf(" \nsteem: run XSteem, the Atari STE emulator for X \n");
  printf("Written by Anthony and Russell Hayward.   \n \n");

  printf("Usage:  steem [options] [disk_image_a [disk_image_b]] [cartridge]\n");
  printf("        steem [options] [state_file]\n \n");

  printf("  disk image: name of disk image (extension ST/MSA/DIM/STT/ZIP/RAR) ");
  printf("for Steem to load.  If 2 disks are specified, the first ");
  printf("will be ST drive A: and the second drive B:.\n \n");

  printf("  cartridge:  name of a cartridge image (.STC) to be loaded.\n \n");

  printf("  state file: previously saved state file (.STS) to load.  If none ");
  printf("is specified, Steem will load \"auto.sts\" provided ");
  printf("the relevant option is checked in the Options dialog.\n \n");

  printf("  tos image:  name of TOS image to use (.IMG or .ROM).\n \n");

  printf("  options:    list of options separated by spaces.  Options are case-");
  printf("independent and can be prefixed by -, --, / or nothing.\n");
  printf("              NOSHM: disable use of Shared Memory.\n");
  printf("              NOSOUND: no sound output.\n");
  printf("              SOF=<n>: set sound output frequency to <n> Hz.\n");
  printf("              PABUFSIZE=<n>: set PortAudio buffer size to <n> samples.\n");
  printf("              FONT=<string>: use a different font.\n");
  printf("              HELP: print this message and quit.\n");
  printf("              INI=<file>: use <file> instead of steem.ini to ");
  printf("initialise options.\n");
  printf("              TRANS=<file>: use <file> instead of searching for ");
  printf("Translate.txt or Translate_*.txt to ");
  printf("translate the GUI text.\n \n");
  
  printf("All of these options (except INI= and TRANS=) can be changed ");
  printf("from the GUI once Steem is running.  It is easiest just to run ");
  printf("Steem and play with the GUI.\n\n");
}


bool GetWindowPositionData(Window Win,TWinPositionData *wpd)
{
  if (Win==0 || XD==NULL) return 1;

  XWindowAttributes wa;
  XGetWindowAttributes(XD,Win,&wa);
  wpd->Left=wa.x;
  wpd->Top=wa.y;
  wpd->Width=wa.width;
  wpd->Height=wa.height;

  wpd->Maximized=0;
  wpd->Minimized=0;

  return 0;
}
//---------------------------------------------------------------------------
void CentreWindow(Window Win,bool)
{
  if (XD==NULL) return;

  XWindowAttributes wa;
  XGetWindowAttributes(XD,Win,&wa);
  XMoveWindow(XD,Win,(GetScreenWidth()-wa.width)/2,
               (GetScreenHeight()-wa.height)/2);
}
//---------------------------------------------------------------------------
bool SetForegroundWindow(Window Win,Time TimeStamp)
{
  if (XD==NULL || Win==0) return 0;

  XRaiseWindow(XD,Win);
  XSync(XD,False);
  XError.display=NULL;
  XSetInputFocus(XD,Win,RevertToNone,TimeStamp);
  XFlush(XD);
  return (XError.display==NULL);
}
//---------------------------------------------------------------------------
Window GetForegroundWindow()
{
  if (XD==NULL) return 0;

	Window Foc;
	int Revert;
	XGetInputFocus(XD,&Foc,&Revert);
  return Foc;
}
//---------------------------------------------------------------------------
/*
NoEventMask            No events wanted
KeyPressMask           Keyboard down events wanted
KeyReleaseMask         Keyboard up events wanted
ButtonPressMask        Pointer button down events wanted
ButtonReleaseMask      Pointer button up events wanted
EnterWindowMask        Pointer window entry events wanted
LeaveWindowMask        Pointer window leave events wanted
PointerMotionMask      Pointer motion events wanted
PointerMotionHint-     Pointer motion hints wanted
Mask
Button1MotionMask      Pointer motion while button 1 down
Button2MotionMask      Pointer motion while button 2 down
Button3MotionMask      Pointer motion while button 3 down
Button4MotionMask      Pointer motion while button 4 down
Button5MotionMask      Pointer motion while button 5 down
ButtonMotionMask       Pointer motion while any button
                       down
KeymapStateMask        Keyboard state wanted at window
                       entry and focus in
ExposureMask           Any exposure wanted
VisibilityChangeMask   Any change in visibility wanted
StructureNotifyMask    Any change in window structure
                       wanted
ResizeRedirectMask     Redirect resize of this window
SubstructureNotify-    Substructure notification wanted
Mask
SubstructureRedi-      Redirect structure requests on
rectMask               children
FocusChangeMask        Any change in input focus wanted
PropertyChangeMask     Any change in property wanted
ColormapChangeMask     Any change in colormap wanted
OwnerGrabButtonMask    Automatic grabs should activate
                       with owner_events set to True
-------------------------------------------------------------
Event Category           Event Type
-------------------------------------------------------------
Keyboard events          KeyPress, KeyRelease
Pointer events           ButtonPress, ButtonRelease, Motion-
                         Notify

Window crossing events   EnterNotify, LeaveNotify
Input focus events       FocusIn, FocusOut
Keymap state notifica-   KeymapNotify
tion event
Exposure events          Expose, GraphicsExpose, NoExpose
Structure control        CirculateRequest, ConfigureRequest,
events                   MapRequest, ResizeRequest
Window state notifica-   CirculateNotify, ConfigureNotify,
tion events              CreateNotify, DestroyNotify,
                         GravityNotify, MapNotify,
                         MappingNotify, ReparentNotify,
                         UnmapNotify, VisibilityNotify
Colormap state notifi-   ColormapNotify
cation event
Client communication     ClientMessage, PropertyNotify,
events                   SelectionClear, SelectionNotify,
                         SelectionRequest
-------------------------------------------------------------
*/
//---------------------------------------------------------------------------
typedef int WNDPROC(void*,Window,XEvent*);
typedef WNDPROC* LPWINDOWPROC;

int ProcessEvent(XEvent *Ev)
{
  if (XD==NULL) return PEEKED_NOTHING;

  LPWINDOWPROC WinProc=(LPWINDOWPROC)GetProp(Ev->xany.window,cWinProc);
  if (WinProc) return WinProc((void*)GetProp(Ev->xany.window,cWinThis),Ev->xany.window,Ev);

  return PEEKED_MESSAGE;
}



void GUIUpdateInternalSpeakerBut()
{
  OptionBox.internal_speaker_but.set_check(false);
}


//---------------------------------------------------------------------------
short GetKeyState(int Key)
{
  if (Key==VK_LBUTTON || Key==VK_RBUTTON || Key==VK_MBUTTON){
    Window InWin,InChild;
    int RootX,RootY,WinX,WinY;
    UINT Mask;
    XQueryPointer(XD,StemWin,&InWin,&InChild,
                  &RootX,&RootY,&WinX,&WinY,&Mask);
    if (Key==VK_LBUTTON) return short((Mask & Button1Mask) ? -1:0);
    if (Key==VK_MBUTTON) return short((Mask & Button2Mask) ? -1:0);
    if (Key==VK_RBUTTON) return short((Mask & Button3Mask) ? -1:0);
  }else if (Key==VK_SHIFT){
    if (KeyState[VK_LSHIFT]<0 || KeyState[VK_RSHIFT]<0) return -1;
  }else if (Key==VK_CONTROL){
    if (KeyState[VK_LCONTROL]<0 || KeyState[VK_RCONTROL]<0) return -1;
  }else if (Key==VK_MENU){
    if (KeyState[VK_LMENU]<0 || KeyState[VK_RMENU]<0) return -1;
  }
  return KeyState[BYTE(Key)];
}
//---------------------------------------------------------------------------
void SetKeyState(int Key,bool Down,bool Toggled)
{
  KeyState[BYTE(Key)]=short((Toggled ? 1:0) | (Down ? 0x8000:0));
}
//---------------------------------------------------------------------------
short GetKeyStateSym(KeySym Sym)
{
  return KeyState[XKeysymToKeycode(XD,Sym)];
}
//---------------------------------------------------------------------------
TModifierState GetLRModifierStates()
{
  TModifierState mss;
  mss.LShift=(GetKeyState(VK_LSHIFT)<0);
  mss.RShift=(GetKeyState(VK_RSHIFT)<0);
  mss.LCtrl=(GetKeyState(VK_LCONTROL)<0);
  mss.RCtrl=(GetKeyState(VK_RCONTROL)<0);
  mss.LAlt=(GetKeyState(VK_LMENU)<0);
  mss.RAlt=(GetKeyState(VK_RMENU)<0);
  return mss;
}
//---------------------------------------------------------------------------
int MessageBox(WINDOWTYPE,char *Text,char *Caption,UINT Flags)
{
  int icon_index=-1;
  int mb_ico=(Flags&MB_ICONMASK);
  if(mb_ico==MB_ICONEXCLAMATION){
    icon_index=ICO32_EXCLAM;
  }else if(mb_ico==MB_ICONQUESTION){
    icon_index=ICO32_QUESTION;
  }else if(mb_ico==MB_ICONSTOP){
    icon_index=ICO32_STOP;
  }else if(mb_ico==MB_ICONINFORMATION){
    icon_index=ICO32_INFO;
  }
  if(icon_index==-1){
    alert.set_icons(NULL,0);
  }else{
    alert.set_icons(&Ico32,icon_index,&Ico16,icon_index);
  }
  int default_option=-1;
  switch(Flags&MB_DEFMASK){
  case MB_DEFBUTTON1:default_option=1;break;
  case MB_DEFBUTTON2:default_option=2;break;
  case MB_DEFBUTTON3:default_option=3;break;
  case MB_DEFBUTTON4:default_option=4;break;
  }
  int choice;

  switch (Flags & MB_TYPEMASK){
    case MB_OKCANCEL:
      choice=alert.ask(XD,Text,Caption,T("Okay")+"|"+T("Cancel"),default_option,1);
      if(choice==0)return IDOK;
      else return IDCANCEL;
    case MB_ABORTRETRYIGNORE:
      choice=alert.ask(XD,Text,Caption,T("Abort")+"|"+T("Retry")+"|"+T("Ignore"),default_option,0);
      if(choice==0)return IDABORT;
      else if(choice==1)return IDRETRY;
      else return IDRETRY;
    case MB_YESNOCANCEL:
      choice=alert.ask(XD,Text,Caption,T("Yes")+"|"+T("No")+"|"+T("Cancel"),default_option,2);
      if(choice==0)return IDYES;
      else if(choice==1)return IDNO;
      else return IDCANCEL;
    case MB_YESNO:
      choice=alert.ask(XD,Text,Caption,T("Yes")+"|"+T("No"),default_option,1);
      if(choice==0)return IDYES;
      else return IDNO;
    case MB_RETRYCANCEL:
      choice=alert.ask(XD,Text,Caption,T("Retry")+"|"+T("Cancel"),default_option,1);
      if(choice==0)return IDRETRY;
      else return IDCANCEL;
    default:
      alert.ask(XD,Text,Caption,T("Okay"),default_option,0);
      return IDOK;
  }
}
//---------------------------------------------------------------------------
void GetCursorPos(POINT *pPt)
{
  Window InWin,InChild;
  int RootX,RootY;
  UINT Mask;
  if (XQueryPointer(XD,Window(FullScreen ? Disp.XVM_FullWin:StemWin),&InWin,&InChild,
                    &RootX,&RootY,(int*)&(pPt->x),(int*)&(pPt->y),&Mask)==0){
    pPt->x=window_mouse_centre_x;pPt->y=window_mouse_centre_y;
  }
}
//---------------------------------------------------------------------------
void SetCursorPos(int x,int y)
{
  XWarpPointer(XD,None,Window(FullScreen ? Disp.XVM_FullWin:StemWin),0,0,0,0,x,y);
}
//---------------------------------------------------------------------------
int hyperlink_np(hxc_button *b,int mess,int *pi)
{
  if (mess!=BN_CLICKED) return 0;

  EasyStr Text=b->text;
  char *pipe=strchr(Text,'|');
  if (pipe) Text=pipe+1;
  bool web=IsSameStr_I(Text.Lefts(7),"http://");
  bool ftp=IsSameStr_I(Text.Lefts(6),"ftp://");
  bool email=IsSameStr_I(Text.Lefts(7),"mailto:");
  if (web || ftp || email){
    if (email) Text=Text.Text+7; // strip "mailto:"
    // Shell browser
    Str comline=Comlines[COMLINE_HTTP];
    if (ftp) comline=Comlines[COMLINE_FTP];
    if (email) comline=Comlines[COMLINE_MAILTO];
    shell_execute(comline,Str("[URL]\n")+Text+"\n[ADDRESS]\n"+Text);
  }
  return 0;
}
//---------------------------------------------------------------------------


#endif//UNIX

