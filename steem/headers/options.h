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
FILE: options.h
DESCRIPTION: Declarations for Steem's option dialog (TOptionBox) that allows
the user to change Steem's many options to their heart's delight.
struct TOptionBox, TOption, TConfig
---------------------------------------------------------------------------*/

#pragma once
#ifndef SSEOPTION_H
#define SSEOPTION_H
/*  options defines structures TOption (player choices) and TConfig 
    (internal use).
    This file is seen by 6301.c
*/

#include "SSE.h" 


#ifdef __cplusplus // 6301.c

#include <easystr.h>
#include <stemdialogs.h>
#ifdef WIN32
#include <scrollingcontrolswin.h>
#define OPTIONS_HEIGHT 395
#endif
#ifdef UNIX
#include <x/hxc_dir_lv.h>
#define OPTIONS_HEIGHT 400
#endif


#pragma pack(push, 8)

// we finally create an enum for control handles, still need to cast!
//TODO
enum EOptionHandles {
  IDSYS_NORMAL=101,
  IDSYS_TOP=102,
  IDSYS_ASPECT=103,
  IDSYS_BIGGER=104,
  IDSYS_SMALLER=105,
  IDSYS_BORDEROFF=110,
  IDSYS_BORDERON=111,
  IDSYS_NOOSD=113,

  IDC_DISK_MANAGER=100, //should be in gui.h
  IDC_PLAY=101,
  IDC_RESET=102,
  IDC_JOYSTICKS=103,
  IDC_INFO=105,
  IDC_OPTIONS=107,
  IDC_SNAPSHOT=108,
  IDC_FASTFORWARD=109,
  IDC_SHORTCUTS=112,
  IDC_PATCHES=113,
  IDC_PASTE=114,
  IDC_SCREENSHOT=115,
  IDC_STATUS_BAR=120,
  IDC_CONFIGS=121,
  IDC_DEBUGGER=199,

  IDC_GLU_WAKEUP=212,
  IDC_TOGGLE_FULLSCREEN=215,
  IDC_GLU_WAKEUP0=7347, // buddy edit must have different number!

  IDC_CPU_SPEED=404,

  IDC_6301=1029,
  IDC_SCANLINES=1032,
  IDC_VMMOUSE=1035,
  IDC_ADVANCED_SETTINGS=1038,
  IDC_ST_AR=1042,

  IDC_EMU_THREAD=1047,
  IDC_UNSTABLE_SHIFTER=1049,
  IDC_LEGACY_TOOLBAR=1050,
  IDC_SCREENSHOT_FORMAT,
  IDC_WARNINGS=1053,
  IDC_OSD_DEBUGINFO,
  IDC_VID_FREQUENCY,
  IDC_OSD_FPSINFO,
  IDC_YM2149_ON,
  IDC_STESOUND_ON,
  IDC_RANDOM_WU,
  IDC_OSD_NONEONSTOP,
  ID_TOOLBAR=1061,
  IDC_FLIPEX=1062,
  IDC_RESET_DISPLAY=1063,
  IDC_TOOLBAR_TASKBAR=1064,
  IDC_TOOLBAR_VERTICAL,
  IDC_SOUNDMUTE,
  IDC_FASTBLITTER,
  //free=1068,

  ID_BRIGHTNESS_MAP=2010,

  IDC_SHIFTER_WU=7330,
  IDC_SHIFTER_WU0=7346, // buddy edit must have different number!

  IDC_RADIO_STSCREEN=7341,
  IDC_RADIO_SWOVERSCAN=7326,
  IDC_RADIO_HWOVERSCAN=7335,
  IDC_RADIO_BORDER=17360, //->17363

  IDC_MEMORY_SIZE=8100,
  IDC_EXTENDED_MONITOR=8200,

  IDC_RADIO_CAPTURE_MOUSE=17380, //-> 17382 // off on auto
  IDC_RADIO_ST_MODEL=17340,
};

extern EasyStr WAVOutputFile;
extern EasyStringList DSDriverModuleList;
#define EXTMON_RESOLUTIONS (7+2+2)
extern UINT extmon_res[EXTMON_RESOLUTIONS][3];


struct TOptionBox : public TStemDialog {
  enum EOptionBox {PAGE_SOUND=5,PAGE_MACHINE=9}; //...
  TOptionBox();
  ~TOptionBox();
  void Show(),Hide();
  bool ToggleVisible() { 
    IsVisible() ? Hide() : Show();
    return IsVisible(); 
  }
  void EnableBorderOptions(bool);
  bool ChangeBorderModeRequest(int);
  void ChangeOSDDisable(bool);
  bool LoadData(bool,TConfigStoreFile*,bool* = NULL),SaveData(bool,TConfigStoreFile*);
  void CreatePage(int);
  void CreateResetButton(int y,EasyStr protip);
  void CreateMachinePage(),CreateTOSPage(),CreateGeneralPage(),CreatePortsPage();
  void CreateSoundPage(),CreateDisplayPage(),CreateBrightnessPage();
  void CreateMacrosPage(),CreateProfilesPage(),CreateStartupPage(),CreateOSDPage();
  void CreateSSEPage();
  void CreateInputPage();
  void CreateSTVideoPage(),UpdateSTVideoPage();
  void FullscreenBrightnessBitmap();
  void UpdateSoundFreq();
  void ChangeSoundFormat(BYTE,BYTE);
  void UpdateRecordBut();
  void SetRecord(bool);
  void SoundMute(BYTE muting);
  void UpdateMacroRecordAndPlay(Str="",int=0);
  Str CreateMacroFile(bool);
  void LoadProfile(char*);
  void UpdateParallel();
  void MachineUpdateIfVisible();
  void SSEUpdateIfVisible();
  void UpdateCaption();
  void TOSRefreshBox(EasyStr="");
  bool NeedReset() { 
    return NewMemConf0>=0 || NewMonitorSel>=0 || NewROMFile.NotEmpty(); 
  }
  int GetCurrentMonitorSel();
	int TOSLangToFlagIdx(int);
  void EnableControl(int nIDDlgItem,BOOL enabled);

#ifdef WIN32
  static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT CALLBACK Fullscreen_WndProc(HWND,UINT,WPARAM,LPARAM);
  static LRESULT CALLBACK GroupBox_WndProc(HWND,UINT,WPARAM,LPARAM);
  static int DTreeNotifyProc(DirectoryTree*,void*,int,INT_PTR,INT_PTR);
  void DestroyCurrentPage();
  void ManageWindowClasses(bool);
  void AssAddToExtensionsLV(char *Ext,char *Desc,int Num);
  void DrawBrightnessBitmap(HBITMAP);
  void CreateBrightnessBitmap();
  void PortsMakeTypeVisible(int);
  HWND CreateButton(EasyStr caption,int hMenu,int X,int Y,int &Wid,int nHeight
                    =25,DWORD dwStyle=WS_CHILD|WS_TABSTOP|BS_CHECKBOX);
  void CreateFullscreenPage(),CreateMIDIPage(),CreateAssocPage();
  void CreateIconsPage();
  void IconsAddToScroller();
  bool HasHandledMessage(MSG *);
  void LoadIcons();
  void ChangeScreenShotFormat(int,Str);
  void ChooseScreenShotFolder(HWND);
  static BOOL CALLBACK EnumDateFormatsProc(char *);
  void UpdateWindowSizeAndBorder();
  void SetBorder(int);
  void UpdateForDSError();
#if defined(SSE_VID_DD)
  void UpdateFullscreen();
#endif
#if !defined(SSE_VID_NO_FREEIMAGE)
  void ChangeScreenShotFormatOpts(int);
  void FillScreenShotFormatOptsCombo();
#endif
#ifndef SSE_NO_UPDATE
  void CreateUpdatePage();
#endif
  HIMAGELIST il;
  HBITMAP hBrightBmp;
  WNDPROC Old_GroupBox_WndProc;
  HWND BorderOption;
  ScrollControlWin Scroller;
  static DirectoryTree DTree;
#endif//WIN32  

#ifdef UNIX
  static int WinProc(TOptionBox*,Window,XEvent*);
	static int listview_notify_proc(hxc_listview*,int,int);
  static int dd_notify_proc(hxc_dropdown*,int,int);
  static int button_notify_proc(hxc_button*,int,int*);
	static int edit_notify_proc(hxc_edit *,int,int);
	static int scrollbar_notify_proc(hxc_scrollbar*,int,int);
  static int dir_lv_notify_proc(hxc_dir_lv*,int,int);
  void DrawBrightnessBitmap(XImage*);
  void UpdateProfileDisplay(Str="",int=-1);
  void FillSoundDevicesDD();
  void CreatePathsPage();
  void UpdatePortDisplay(int);
  int page_p;
  hxc_listview page_lv;
  hxc_button control_parent;
  hxc_button cpu_boost_label,pause_inactive_but;
  hxc_dropdown cpu_boost_dd;
	hxc_button memory_label,monitor_label,tos_group;
  hxc_dropdown memory_dd,monitor_dd;
	hxc_button cart_group,cart_display,cart_change_but,cart_remove_but;
	hxc_button keyboard_language_label,keyboard_sc_but;
  hxc_dropdown keyboard_language_dd;
	hxc_button coldreset_but;
  hxc_textdisplay mustreset_td;
  hxc_dropdown tos_sort_dd;
  hxc_listview tos_lv;
	hxc_button tosadd_but,tosrefresh_but;
  hxc_button PortGroup[3],ConnectLabel[3];
  hxc_dropdown ConnectDD[3];
  hxc_button IOGroup[3],IOChooseBut[3],IOAllowIOBut[3][2],IOOpenBut[3];
  hxc_edit IODevEd[3];
  hxc_button LANGroup[3];
  hxc_button FileGroup[3],FileDisplay[3],FileChooseBut[3],FileEmptyBut[3];
  hxc_button high_priority_but,start_click_but;
  hxc_button FFMaxSpeedLabel,SMSpeedLabel,RunSpeedLabel;
  hxc_scrollbar FFMaxSpeedSB,SMSpeedSB,RunSpeedSB;
  hxc_button ff_on_fdc_but;
  hxc_button fs_label;hxc_dropdown frameskip_dd;
  hxc_button bo_label;hxc_dropdown border_dd;
  hxc_button size_group,reschangeresize_but;
  hxc_button lowres_doublesize_but,medres_doublesize_but;
  hxc_button screenshots_group,screenshots_fol_display;
  hxc_button screenshots_fol_label,screenshots_fol_but;
  hxc_button sound_group,sound_mode_label,sound_freq_label,sound_format_label;
  hxc_dropdown sound_mode_dd,sound_freq_dd,sound_format_dd;
	hxc_button device_label,record_group,record_but;
	hxc_button wav_output_label,wav_choose_but,overwrite_ask_but;
  hxc_edit device_ed;
  hxc_listview profile_sect_lv;
  IconGroup brightness_ig;
  XImage *brightness_image;
  hxc_button brightness_picture,brightness_picture_label;
  hxc_button brightness_label;
  hxc_scrollbar brightness_sb;
  hxc_button contrast_label;
  hxc_scrollbar contrast_sb;
  hxc_button auto_sts_but;
  hxc_button auto_sts_filename_label;
  hxc_edit auto_sts_filename_edit;
  hxc_button no_shm_but;
  hxc_button osd_disable_but;
  hxc_listview drop_lv;
  static hxc_dir_lv dir_lv;
  hxc_button internal_speaker_but; // changed in Sound_Start
  hxc_button border_size_label; 
  hxc_dropdown border_size_dd;
  hxc_button capture_mouse_but;
  hxc_button specific_hacks_but;
  hxc_button stealth_mode_but;
  hxc_button st_type_label;
  hxc_dropdown st_type_dd;
  hxc_button wake_up_label; 
  hxc_dropdown wake_up_dd;
#if defined(SSE_HD6301_LL) 
  hxc_button hd6301emu_but;
#endif
#if defined(SSE_INT_MFP_OPTION)
  hxc_button mc68901_but;
#endif
  hxc_button keyboard_click_but; 
#if defined(SSE_YM2149_FIXED_VOL_TABLE)
  hxc_button psg_samples_but;
#endif
  hxc_button ste_microwire_but;
  hxc_button vm_mouse_but;
#endif//UNIX

  EasyStringList eslTOS;
  ESLSortEnum eslTOS_Sort;
  EasyStr WAVOutputDir;
  Str NewROMFile;
  Str TOSBrowseDir,LastCartFile;
  Str LastIconPath,LastIconSchemePath;
  Str MacroDir,MacroSel;
  Str ProfileDir,ProfileSel;
  int Page;
  int page_l,page_w;
  int NewMemConf0,NewMemConf1,NewMonitorSel;
  bool eslTOS_Descend;
  bool RecordWarnOverwrite;
  static bool USDateFormat;
};

#pragma pack(pop)

extern TOptionBox OptionBox;

#endif//#ifdef __cplusplus


/////////////////////
// (extra) Options //
/////////////////////

struct TOption {
  WORD low_pass_frequency; //in Hz
  BYTE STModel;
  BYTE DisplaySize;
  BYTE WakeUpState; 
  BYTE Hacks;
  BYTE Chipset1;
  BYTE Microwire;
  BYTE PSGFilter;
  BYTE CaptureMouse;
  BYTE EmuDetect;
  BYTE TraceFileLimit;
  BYTE UseSDL;
  BYTE OsdDriveInfo;
  BYTE NoDsp;
  BYTE OsdImageName;
  BYTE PastiJustSTX;
  BYTE Scanlines;
  BYTE StatusBar;
  BYTE WinVSync;
  BYTE TripleBufferWin;
  BYTE StatusBarGameName;
  BYTE DriveSound;
  BYTE SampledYM;
  BYTE GhostDisk;
  BYTE Direct3D;
  BYTE STAspectRatio;
  BYTE DriveSoundSeekSample;
  BYTE TestingNewFeatures;
  BYTE BlockResize;
  BYTE LockAspectRatio;
  BYTE FinetuneCPUclock;
  BYTE Chipset2;
  BYTE PRG_support;
  BYTE FullscreenAR;
  BYTE FullscreenCrisp; //d3d
  BYTE Acsi;
  BYTE KeyboardClick;
  BYTE MonochromeDisableBorder;
  BYTE FullScreenGui;
  BYTE VMMouse;
  BYTE OsdTime;
  BYTE CartidgeOff;
  BYTE FullScreenDefaultHz;
  BYTE TripleBufferFS;
  BYTE FakeFullScreen;
  BYTE Advanced;
  BYTE YmLowLevel;
  BYTE Chipset3; // low-level video logic emulation
  BYTE FullscreenOnMaximize;
  BYTE Battery6301;
  BYTE RtcHack;
  BYTE HwOverscan;
  BYTE VideoLogicEmu;
  BYTE StPreselect;
  BYTE YM12db;
  BYTE EmuThread;
  BYTE Spurious;
  BYTE UnstableShifter;
  BYTE AutoSTW;
  BYTE LegacyToolbar;
  BYTE Warnings;
  BYTE OsdDebugInfo;
  BYTE OsdFpsInfo;
  BYTE CountDmaCycles;
  BYTE RandomWakeup;
  BYTE OsdNoneOnStop;
  BYTE FlipEx;
  BYTE ToolbarTaskbar;
  BYTE ToolbarVertical;
  BYTE SoundMute;
  BYTE FastBlitter;
#if defined(__cplusplus)
  BYTE SoundRecordFormat;
  enum EOption {SoundFormatWav,SoundFormatYm};
  TOption();
  void Init();
  void Restore(bool all=false);
#endif
};

// C linkage to be accessible by 6301 emu
#ifdef __cplusplus
extern "C" TOption SSEOptions;
#else
extern struct TOption SSEOptions;
#endif

// if an option is defined as always true or always false, a smart compiler
// can optimise

#define OPTION_HACKS (SSEOptions.Hacks)

#if !defined(SSE_HD6301_LL)
#define OPTION_C1 (FALSE) 
#else
#define OPTION_C1 (SSEOptions.Chipset1)
#endif

#if defined(SSE_INT_MFP_OPTION)
#define OPTION_68901 (SSEOptions.Chipset2)
#else
#define OPTION_68901 (TRUE)
#endif

#define OPTION_C2 (OPTION_VLE==1)

#if defined(SSE_VID_STVL1)
#define OPTION_C3 (OPTION_VLE==2)
#else
#define OPTION_C3 (FALSE)
#endif
#if !defined(SSE_GUI_OPTIONS_MICROWIRE)
#define OPTION_MICROWIRE (TRUE)
#else
#define OPTION_MICROWIRE (SSEOptions.Microwire)
#endif
#define ST_MODEL (SSEOptions.STModel) // was 'ST_TYPE'

#define OPTION_CAPTURE_MOUSE (SSEOptions.CaptureMouse)
#define OPTION_VMMOUSE (SSEOptions.VMMouse)

#define DISPLAY_SIZE (border)
#define OPTION_EMU_DETECT SSEOptions.EmuDetect
#define TRACE_FILE_REWIND (SSEOptions.TraceFileLimit)
#define OPTION_WS (SSEOptions.WakeUpState)

//#define USE_SDL (options.UseSDL)
#define OPTION_DRIVE_INFO (SSEOptions.OsdDriveInfo)
#define OSD_IMAGE_NAME (SSEOptions.OsdImageName)
#define OPTION_PASTI_JUST_STX (SSEOptions.PastiJustSTX)
#define OPTION_SCANLINES (SSEOptions.Scanlines)
#define OPTION_ST_ASPECT_RATIO (SSEOptions.STAspectRatio)
#if defined(SSE_GUI_LEGACY_TOOLBAR)
#define OPTION_STATUS_BAR (!SSEOptions.LegacyToolbar)
#else
#define OPTION_STATUS_BAR (SSEOptions.StatusBar)
#endif
#define OPTION_STATUS_BAR_GAME_NAME (SSEOptions.StatusBarGameName)
#define OPTION_WIN_VSYNC (SSEOptions.WinVSync)
#define OPTION_3BUFFER (SSEOptions.TripleBufferWin)

#if defined(SSE_VID_DD_3BUFFER_WIN)
#define OPTION_3BUFFER_WIN (SSEOptions.TripleBufferWin)
#else
#define OPTION_3BUFFER_WIN (FALSE)
#endif

#if defined(SSE_VID_3BUFFER_FS)
#define OPTION_3BUFFER_FS (SSEOptions.TripleBufferFS)
#else
#define OPTION_3BUFFER_FS (FALSE)
#endif
#define OPTION_DRIVE_SOUND (SSEOptions.DriveSound)
#define OPTION_GHOST_DISK (SSEOptions.GhostDisk)
#define OPTION_SAMPLED_YM (SSEOptions.SampledYM)
#define OPTION_SOUND_RECORD_FORMAT (SSEOptions.SoundRecordFormat)
#if defined(SSE_SOUND_OPTION_DISABLE_DSP)
#define DSP_DISABLED (SSEOptions.NoDsp)
#else
#define DSP_DISABLED (FALSE)
#endif
#define OPTION_DRIVE_SOUND_SEEK_SAMPLE (SSEOptions.DriveSoundSeekSample)
#define SSE_TEST_ON (SSEOptions.TestingNewFeatures) //use macro only for actual tests
#define OPTION_BLOCK_RESIZE (SSEOptions.BlockResize)
#define OPTION_LOCK_ASPECT_RATIO (SSEOptions.LockAspectRatio)
#define OPTION_CPU_CLOCK (SSEOptions.FinetuneCPUclock)
#define OPTION_PRG_SUPPORT (SSEOptions.PRG_support)
#define OPTION_FULLSCREEN_AR (SSEOptions.FullscreenAR)
#define OPTION_CRISP_FULLSCREEN (SSEOptions.FullscreenCrisp) //d3d
#define OPTION_KEYBOARD_CLICK (SSEOptions.KeyboardClick)
#define OPTION_FULLSCREEN_GUI (SSEOptions.FullScreenGui)
#define OPTION_OSD_TIME (SSEOptions.OsdTime)
#define OPTION_FULLSCREEN_DEFAULT_HZ (SSEOptions.FullScreenDefaultHz)

#ifdef SSE_VID_DD
#define OPTION_FAKE_FULLSCREEN (draw_fs_blit_mode==DFSM_FAKEFULLSCREEN)
#else
#define OPTION_FAKE_FULLSCREEN (SSEOptions.FakeFullScreen)
#endif

#define OPTION_MAME_YM (SSEOptions.YmLowLevel)
#define RENDER_SIGNED_SAMPLES (sound_num_bits==16)
#define OPTION_ADVANCED (SSEOptions.Advanced)
#define OPTION_MAX_FS (SSEOptions.FullscreenOnMaximize)
#define OPTION_RTC_HACK (SSEOptions.RtcHack)
#define OPTION_VLE (SSEOptions.VideoLogicEmu)
#define OPTION_HWOVERSCAN (SSEOptions.HwOverscan)
#define LACESCAN 1
#define AUTOSWITCH 2
#define OPTION_BATTERY6301 (SSEOptions.Battery6301)
#define OPTION_ST_PRESELECT (SSEOptions.StPreselect)
#define OPTION_YM_12DB (SSEOptions.YM12db)
#define OPTION_EMUTHREAD (SSEOptions.EmuThread)
#define OPTION_SPURIOUS (SSEOptions.Spurious)
#define OPTION_UNSTABLE_SHIFTER (SSEOptions.UnstableShifter)
#define OPTION_AUTOSTW (SSEOptions.AutoSTW)
#define OPTION_LEGACY_TOOLBAR (SSEOptions.LegacyToolbar)
#define OPTION_WARNINGS (SSEOptions.Warnings)
#define OPTION_OSD_DEBUGINFO (SSEOptions.OsdDebugInfo)
#define OPTION_OSD_FPSINFO (SSEOptions.OsdFpsInfo)
#define OPTION_COUNT_DMA_CYCLES (SSEOptions.CountDmaCycles)
#define OPTION_RANDOM_WU (SSEOptions.RandomWakeup)
#define OPTION_NO_OSD_ON_STOP (SSEOptions.OsdNoneOnStop)
#define OPTION_FLIPEX (SSEOptions.FlipEx)
#define OPTION_TOOLBAR_TASKBAR (SSEOptions.ToolbarTaskbar)
#define OPTION_TOOLBAR_VERTICAL (SSEOptions.ToolbarVertical)
#define OPTION_SOUNDMUTE (SSEOptions.SoundMute)
#define OPTION_FASTBLITTER (SSEOptions.FastBlitter)


////////////
// Config //
////////////

struct TConfig {
  int WindowsVersion;
  int CpuBoost; // coarse
  BYTE UnrarDll;
  //BYTE SdlDll;
  BYTE Hd6301v1Img;
  BYTE unzipd32Dll;
  BYTE CapsImgDll;
  BYTE PastiDll;
  BYTE FreeImageDll;
  BYTE Direct3d9;
  BYTE ArchiveAccess;
  BYTE AcsiImg;
  BYTE VideoCard8bit;
  BYTE VideoCard16bit;
  BYTE VideoCard32bit;
  BYTE ym2149_fixed_vol;
  BYTE mv16; // B.A.T cartridge
  BYTE mr16; // Microdeal Replay 16 cartridge
  BYTE Port0Joy;
  BYTE TraceFile;
  BYTE OverscanOn;
  BYTE CpuBoosted;
  BYTE ColourMonitor;
  BYTE old_DisableHardDrives;
  BYTE HiresColourMonitor;
  BYTE Blitter;
  BYTE Ste;
  BYTE Mega;
  BYTE CurrentWs;
  WORD Stvl; //0 or version
  BYTE TosRecognised;
  BYTE TosLanguage;
  BYTE IsInit;
  BYTE DiskImageCreated; //1 ST, 2 MSA, 3 DIM
  BYTE YmSoundOn;
  BYTE SteSoundOn;
  BYTE ShowNotify;
#ifdef __cplusplus // visible only to C++ objects
  MEM_ADDRESS bank_length[2]; //ref. v402
  TConfig();
  ~TConfig();
#if defined(SSE_GUI_FONT_FIX)
  HFONT GuiFont();
#endif
  void make_Mem(BYTE conf0,BYTE conf1); //ref. v402
  int SwitchSTModel(BYTE new_type); // was SwitchSTType
#endif
};

// C linkage to be accessible by 6301 emu
#ifdef __cplusplus
extern "C" TConfig SSEConfig;
#else
extern struct TConfig SSEConfig;
#endif

#define CAPSIMG_OK (SSEConfig.CapsImgDll)
#define DX_FULLSCREEN (SSEConfig.FullscreenMask)
#define HD6301_OK (SSEConfig.Hd6301v1Img)
#define SDL_OK (SSEConfig.SdlDll)
#ifdef SSE_UNRAR_SUPPORT_UNIX
#define UNRAR_OK (true)
#else
#define UNRAR_OK (SSEConfig.UnrarDll)
#endif
#define D3D9_OK (SSEConfig.Direct3d9)
#define ARCHIVEACCESS_OK (SSEConfig.ArchiveAccess)
#if defined(SSE_HIRES_COLOUR)
#define HIRES_COLOUR (SSEConfig.HiresColourMonitor)
#else
#define HIRES_COLOUR (FALSE)
#endif
#if defined(SSE_ACSI)
#define ACSI_EMU_ON (SSEConfig.AcsiImg && SSEOptions.Acsi)
#else
#define ACSI_EMU_ON (FALSE)
#endif

#define IS_STE (SSEConfig.Ste)
#define IS_STF (!SSEConfig.Ste)

#if defined(SSE_MEGASTE)

#define IS_MEGASTE (ST_MODEL==MEGA_STE)

enum ESTModels {STE,STF,MEGA_ST,STFM,MEGA_STE,N_ST_MODELS}; // we keep but don't use STFM

#else

enum ESTModels {STE,STF,MEGA_ST,STFM,N_ST_MODELS}; 

#endif

extern char* st_model_name[];

#endif//#ifndef SSEOPTION_H
