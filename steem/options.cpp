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
FILE: options.cpp
DESCRIPTION: The code for Steem's option dialog (TOptionBox) that allows
the user to change Steem's many options to their heart's delight.
Also TOption and TConfig.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <easystr.h>
#include <conditions.h>
#include <draw.h>
#include <gui.h>
#include <loadsave.h>
#include <options.h> 
#include <stports.h>
#include <emulator.h>
#include <options.h>
#include <debug.h>
#include <display.h>
#include <computer.h>
#include <osd.h>
#include <translate.h>
#include <mymisc.h>
#include <dirsearch.h>
#include <dataloadsave.h>
#include <hd_gemdos.h>
#include <macros.h>
#include <stjoy.h>
#include <palette.h>
#include <choosefolder.h>
#include <diskman.h>
#include <key_table.h>
#include <harddiskman.h>
#include <infobox.h>
#include <reset.h>
#include <notifyinit.h>
#if defined(UNIX)
#include <x/hxc_prompt.h>
#endif


#if defined(SSE_MEGASTE)
char* st_model_name[]={"STE","STF","Mega ST","STFM","Mega STE"};
#else
char* st_model_name[]={"STE","STF","Mega ST","STFM"};
#endif



TOption::TOption() {
  Init();
}


void TOption::Init() {
  ZeroMemory(this,sizeof(TOption));
  Hacks=true;
#if defined(SSE_HD6301_LL)
  Chipset1=TRUE;
#endif
  Chipset2=true; // not used
  VideoLogicEmu=1; // high level by default + Linux
  OsdDriveInfo=1;
  StatusBar=1;
  DriveSound=1;
#ifdef SSE_VID_D3D
  Direct3D=1;
#endif
  SampledYM=true;
  Microwire=1;
  EmuDetect=1;
  PastiJustSTX=1;
  //FakeFullScreen=1; // no: legacy
  KeyboardClick=1;
  //StPreselect=1; // no: legacy
  Spurious=1;
  //Warnings=1; // no: legacy
  CountDmaCycles=1;
  RandomWakeup=1; // choosing is for the expert
  YmLowLevel=1;
  low_pass_frequency=YM_LOW_PASS_FREQ;
}


/*  Restore advanced settings.
    All when player pressed the 'Reset Advanced Settings' button.
    Not all when player unchecked the 'Advanced Settings' option.
*/

void TOption::Restore(bool all) {
#if defined(SSE_VID_DD)
  draw_fs_blit_mode=DFSM_FAKEFULLSCREEN;
#else
  FakeFullScreen=true; // other settings are all hidden
#endif
  FinetuneCPUclock=false;
  if(all) // player pressed 'Reset Advanced Settings'
  {
    run_speed_ticks_per_second=1000;
    floppy_access_ff=false; // slow but compatible
    SSEConfig.OverscanOn=false;
    SSEConfig.SwitchSTModel(ST_MODEL); // updates cpu/mfp ratio
    n_cpu_cycles_per_second=CpuNormalHz;
#ifndef NO_CRAZY_MONITOR
    extended_monitor=0;
#endif
    // display size
    WinSizeForRes[0]=1; // double low
    WinSizeForRes[1]=1; // double height med
    WinSizeForRes[2]=0; // normal high
#ifdef WIN32
    #ifdef STEEM_CRT
        draw_win_mode[0]=draw_win_mode[1]=0;
    #else
        draw_win_mode[0]=draw_win_mode[1]=1;
    #endif
#endif
    StemWinResize();
#if USE_PASTI
    pasti_active=false;
#endif
    SampledYM=1;
    Psg.LoadFixedVolTable();
    YmLowLevel=1;
    Microwire=1;
    sound_freq=44100;
#if !defined(SSE_SOUND_ENFORCE_RECOM_OPT)
    sound_write_primary=false;
    sound_time_method=1;
#endif
    OptionBox.ChangeSoundFormat(16,2); // needs lots of declarations but proper
    OPTION_SHIFTER_WU=SHIFTER_DEFAULT_WAKEUP;
    UnstableShifter=false;
    Chipset1=TRUE;
    Chipset2=true;
    VideoLogicEmu=1; // high level by default
    VMMouse=false;
    floppy_instant_sector_access=false;
    GhostDisk=false;
    EmuDetect=false;
    Hacks=true;
    Str NewCart="";
    LoadSnapShotChangeCart(NewCart); // remove cartridge
#if defined(SSE_DONGLE_PORT) 
    DONGLE_ID=0;
#endif
    WriteCSFStr("Options","NoDirectDraw","0",globalINIFile);
    WriteCSFStr("Options","NoDirectSound","0",globalINIFile);
    NoDsp=false;
    low_pass_frequency=YM_LOW_PASS_FREQ;
    AutoSTW=false;
    SSEConfig.DiskImageCreated=EXT_ST;
    Warnings=1; //?
    OsdDebugInfo=0;
    mouse_speed=10;
    KeyboardClick=1; // most compatible
    SSEConfig.YmSoundOn=SSEConfig.SteSoundOn=1;
    FloppyDrive[0].single_sided=FloppyDrive[1].single_sided=false;
    FloppyDrive[0].freeboot=FloppyDrive[1].freeboot=false;
    Shifter.Preload=0;
  }
}


TConfig::TConfig() {
  ZeroMemory(this,sizeof(TConfig));
  CpuBoost=1;
  DiskImageCreated=YmSoundOn=SteSoundOn=ShowNotify=1;
}


TConfig::~TConfig() {
#if defined(SSE_GUI_FONT_FIX)
  if(hSteemGuiFont)
  {
    if(hSteemGuiFont!=(HFONT)GetStockObject(DEFAULT_GUI_FONT)) //v402
      DeleteObject(hSteemGuiFont); // free Windows resource
#ifndef LEAN_AND_MEAN
    hSteemGuiFont=NULL;
#endif
  }
#endif
}


#if defined(SSE_GUI_FONT_FIX)
/*  Steem uses DEFAULT_GUI_FONT at several places to get the GUI font.
    With this mod, we create a logical font with the parameters of
    DEFAULT_GUI_FONT that we expect.
    If it is different, it can seriously mess the GUI.
    Only if it fails do we use DEFAULT_GUI_FONT.
    Notice that this function is called as soon as needed, which is
    in a constructor. That means we can't reliably use TRACE() here.
*/

HFONT TConfig::GuiFont() {
  if(hSteemGuiFont==NULL)
  {
    hSteemGuiFont=CreateFont(-11, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 
      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE,
      "MS Shell Dlg");
    //ASSERT(hSteemGuiFont);
    if(hSteemGuiFont==NULL) // fall back
      hSteemGuiFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
  }
  return hSteemGuiFont;
}

#endif


void TConfig::make_Mem(BYTE conf0,BYTE conf1) {
  if(STMem!=NULL)
    delete[] STMem;
  Mmu.MemConfig=BYTE((conf0 << 2) | conf1);
  Mmu.bank_length[0]=bank_length[0]=mmu_bank_length_from_config[conf0];
  Mmu.bank_length[1]=bank_length[1]=mmu_bank_length_from_config[conf1];
  mem_len=bank_length[0]+bank_length[1];
  STMem=new BYTE[mem_len+MEM_EXTRA_BYTES];
  //memset(STMem,0xFF,MEM_EXTRA_BYTES);
  Mem_End=STMem+mem_len+MEM_EXTRA_BYTES;
  Mem_End_minus_1=Mem_End-1;
  Mem_End_minus_2=Mem_End-2;
  Mem_End_minus_4=Mem_End-4;
  ZeroMemory(palette_exec_mem,64+PAL_EXTRA_BYTES);
  himem=mem_len;
#if defined(SSE_MMU_MONSTER_ALT_RAM)
  if(himem==0xC00000) //12MB
    himem=FOUR_MEGS; //alt-RAM needs to be activated
#endif
  Mmu.Confused=false;
  //TRACE("make_Mem %X %X mmu %X len %X himem %X\n",conf0,conf1,Mmu.MemConfig,mem_len,himem);
}


int TConfig::SwitchSTModel(BYTE new_type) { 
  ASSERT(new_type<N_ST_MODELS);
  if(new_type>=N_ST_MODELS)
    new_type=0;
  bool change_ws=(new_type!=STE && ST_MODEL==STE && OPTION_WS==4);
  ST_MODEL=new_type;
  BYTE &tos_language=SSEConfig.TosLanguage; // note Steem won't start without a TOS
#if defined(SSE_MEGASTE)
  MegaSte.MemCache.Ready(false);
#endif
  switch(ST_MODEL) {
  case MEGA_ST:
    CpuNormalHz=CPU_CLOCK_MEGA_ST;
    Blitter=true;
    Ste=false;
    Mega=true;
    if(change_ws)
      OPTION_WS=(OPTION_RANDOM_WU)?(rand()%4):3;
    break;
  case STF:
  case STFM:
    CpuNormalHz=(tos_language) ? CPU_CLOCK_STF_PAL : CPU_CLOCK_STF_NTSC;
    Blitter=false;
    Ste=false;
    Mega=false;
    if(change_ws)
      OPTION_WS=(OPTION_RANDOM_WU)?(rand()%4):3;
    break;
  case STE:
  default:
    CpuNormalHz=(tos_language) ? CPU_CLOCK_STE_PAL : CPU_CLOCK_STE_NTSC;
    Blitter=true;
    Ste=true;
    Mega=false;
    OPTION_WS=4; // = WS1
#if defined(SSE_HARDWARE_OVERSCAN)
    OPTION_HWOVERSCAN=0;
#endif
    break;
#if defined(SSE_MEGASTE)
  case MEGA_STE:
    CpuNormalHz=(tos_language)?CPU_CLOCK_STE_PAL:CPU_CLOCK_STE_NTSC;
    Blitter=true;
    Ste=true;
    Mega=true;
    OPTION_WS=4; // = WS1
#if defined(SSE_HARDWARE_OVERSCAN)
    OPTION_HWOVERSCAN=0;
#endif
    MegaSte.MemCache.Ready(true); // allocate memory
    break;
#endif
  }//sw
#ifdef SSE_CPU_MFP_RATIO_OPTION
  if(OPTION_CPU_CLOCK)
    CpuMfpRatio=(double)CpuCustomHz/(double)MFP_XTAL;
  else
#endif
    CpuMfpRatio=(double)CpuNormalHz/(double)MFP_XTAL;
  TRACE_INIT("%s CPU~%d Hz\n",st_model_name[new_type],CpuNormalHz);
  if(n_cpu_cycles_per_second<9000000) // avoid interference with ST CPU Speed option
    n_cpu_cycles_per_second=CpuNormalHz; // no wrong CPU speed icon in OSD (3.5.1)
  Glue.Update();
  ChangeTimingFunctions();
  if(OPTION_SAMPLED_YM)
    Psg.LoadFixedVolTable(); // reload to adapt
  return ST_MODEL;
}


// TODO use this everywhere
#ifdef WIN32
void TOptionBox::EnableControl(int nIDDlgItem,BOOL enabled) {
  if(Handle)
    EnableWindow(GetDlgItem(Handle,nIDDlgItem),enabled);
}
#endif


bool TOptionBox::USDateFormat=0;
WIN_ONLY( DirectoryTree TOptionBox::DTree; )
UNIX_ONLY( hxc_dir_lv TOptionBox::dir_lv; )
EasyStr WAVOutputFile;
EasyStringList DSDriverModuleList;

UINT extmon_res[EXTMON_RESOLUTIONS][3]={ 
  {800,600,1},{1024,720,1},{1024,768,1},{1280,960,1},
  {640,400,4},{800,600,4},{1024,720,4},{1024,768,4},{1280,960,4},
  {0,0,1},{0,0,4} //max screen, must be init
};


TOptionBox::TOptionBox() {
  Section="Options";
  Page=9; // Machine
  NewMemConf0=-1,NewMemConf1=-1,NewMonitorSel=-1;
  RecordWarnOverwrite=true;
  eslTOS.Sort=eslSortByData0;
  eslTOS_Descend=0;
#ifdef WIN32
  eslTOS.Sort2=eslSortByData0;
  page_l=150;page_w=320;
  Left=(GuiSM.cx_screen()-(3+page_l+page_w+10+3))/2;
  Top=(GuiSM.cy_screen()-(OPTIONS_HEIGHT+6+GuiSM.cy_caption()))/2;
  FSLeft=(640-(3+page_l+page_w+10+3))/2;
  FSTop=(480-(OPTIONS_HEIGHT+6+GuiSM.cy_caption()))/2;
  hBrightBmp=NULL;
  BorderOption=NULL;
  il=NULL;
#endif
}


TOptionBox::~TOptionBox() { 
  Hide(); 
}


#define LOGSECTION LOGSECTION_OPTIONS

bool TOptionBox::ChangeBorderModeRequest(int newborder) {
  int newval=newborder;
  if(Disp.BorderPossible()==0&&(FullScreen==0)) 
    newval=0;
  bool proceed=true;
  if(MIN(border,(BYTE)BIGGEST_DISPLAY)==MIN(newval,BIGGEST_DISPLAY)) 
    proceed=false;
  if(proceed) 
    border_last_chosen=(BYTE)newborder;
  return proceed;
}


void TOptionBox::ChangeOSDDisable(bool disable) {
  osd_disable=disable;
  osd_init_run(0);
#ifdef WIN32
  if(Handle)
  {
    if(GetDlgItem(Handle,12030))
      SendMessage(GetDlgItem(Handle,12030),BM_SETCHECK,osd_disable,0);
  }
  CheckMenuItem(StemWin_SysMenu,113,MF_BYCOMMAND|int(osd_disable?MF_CHECKED:MF_UNCHECKED));
#endif
#ifdef UNIX
  osd_disable_but.set_check(osd_disable);
#endif
  draw(true);
  CheckResetDisplay();
}


void TOptionBox::SetRecord(bool On) {
  if(On==0&&sound_record) 
  {
    sound_record_close_file();
    sound_record=false;
  }
  else if(On && sound_record==0) 
  {
    WIN_ONLY(if(Handle) if(GetDlgItem(Handle,7201)) SendDlgItemMessage(Handle,7201,BM_SETCHECK,true,0); )
    UNIX_ONLY(record_but.set_check(true); )
    int Ret=IDYES;
    if(RecordWarnOverwrite) 
    {
      if(Exists(WAVOutputFile)) 
      {
        Ret=Alert(WAVOutputFile+"\n\n"+T("This file already exists, would you like to overwrite it?"),
          T("Record Over?"),MB_ICONQUESTION|MB_YESNO);
      }
    }
    if(Ret==IDYES) 
    {
      timer=timeGetTime();
      sound_record_start_time=timer+100; //start recorfing in 100ms' time
      sound_record=true;
      sound_record_open_file();
    }
  }
#ifdef WIN32
  if(Handle)
    if(GetDlgItem(Handle,7201))
      SendDlgItemMessage(Handle,7201,BM_SETCHECK,sound_record,0);
#endif
#ifdef UNIX
  record_but.set_check(sound_record);
#endif
}


void TOptionBox::SoundMute(BYTE muting) {
  if(muting!=OPTION_SOUNDMUTE)
  {
#if defined(SSE_EMU_THREAD)
    if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING)
      SoundLock.Lock();
#endif
    if(muting)
      Sound_Stop();
    OPTION_SOUNDMUTE=muting;
#ifdef WIN32
    if(IsVisible()&&Page==PAGE_SOUND)
      SendDlgItemMessage(Handle,IDC_SOUNDMUTE,BM_SETCHECK,OPTION_SOUNDMUTE,0);
#endif
    if(!muting)
      Sound_Start();
#if defined(SSE_EMU_THREAD)
    SoundLock.Unlock();
#endif
  }
}


void TOptionBox::UpdateRecordBut() {
  WIN_ONLY(if(Handle) if(GetDlgItem(Handle,7201)) 
  SendDlgItemMessage(Handle,7201,BM_SETCHECK,sound_record,0); )
  UNIX_ONLY(record_but.set_check(sound_record); )
}


void TOptionBox::UpdateSoundFreq() {
#if defined(SSE_EMU_THREAD)
  if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING)
    SoundLock.Lock();
#endif
  Sound_Stop();
#ifdef WIN32
  if(Handle) 
    if(GetDlgItem(Handle,7101)) 
      CBSelectItemWithData(GetDlgItem(Handle,7101),sound_chosen_freq);
#endif
#if defined(SSE_EMU_THREAD)
  SoundLock.Unlock();
#endif
  Sound_Start();
}


void TOptionBox::ChangeSoundFormat(BYTE bits,BYTE channels) {
#if defined(SSE_EMU_THREAD)
  if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING)
    SoundLock.Lock();
#endif
  Sound_Stop();
  sound_num_bits=bits;
  sound_num_channels=channels;
  sound_bytes_per_sample=(sound_num_bits/8)*sound_num_channels;
#if defined(SSE_EMU_THREAD)
  SoundLock.Unlock();
#endif
  Sound_Start();
}


Str TOptionBox::CreateMacroFile(bool Edit) {
  Str Path="";
#ifdef WIN32
  if(Handle) 
  {
    if(GetDlgItem(Handle,10000)) 
    {
      HTREEITEM Item=DTree.NewItem(T("New Macro"),DTree.RootItem,1,Edit);
      if(Item) return DTree.GetItemPath(Item);
      return "";
    }
  }
  Path=GetUniquePath(MacroDir,T("New Macro")+".stmac");
#endif
#ifdef UNIX
  EasyStr name=T("New Macro");
  if (Edit){
    hxc_prompt prompt;
    name=prompt.ask(XD,name,T("Enter Name"));
    if (name.Empty()) return "";
  }
  // Put in current folder
  EasyStr fol=MacroSel;
  RemoveFileNameFromPath(fol,REMOVE_SLASH);
  if (fol.Empty()) fol=MacroDir;
  Path=GetUniquePath(fol,name+".stmac");
#endif
  FILE *f=fopen(Path,"wb");
  if(f==NULL)
    return "";
  fclose(f);
#ifdef UNIX
  if (dir_lv.lv.handle) dir_lv.refresh_fol();
#endif
  return Path;
}


int TOptionBox::GetCurrentMonitorSel() {
  int monitor_sel=bool(MONO);
  if(extended_monitor) 
  {
    monitor_sel=2;
    for(int n=0;n<EXTMON_RESOLUTIONS;n++) 
    {
      if(em_width==extmon_res[n][0]&&em_height==extmon_res[n][1]
        &&em_planes==extmon_res[n][2]) 
        monitor_sel=n+2;
    }
  }
  return monitor_sel;
}


int TOptionBox::TOSLangToFlagIdx(int Lang) {
  switch(Lang) {
  case 7: return 0;  //UK
  case 5: return 2;  //French
  case 0: return 1;  //US
  case 9: return 3;  //Spanish
  case 3: return 4;  //German
  case 11: return 5; //Italian
  case 13: return 6; //Swedish
  case 17: case 0x0F: return 7; //Swiss German + French
  case 27: return 8; //Dutch
/*  Adding support of more countries due to Atari compendium & EMUTOS project
    We have flags for Czech, Finland, Norway and Greece (thx avtandil)
    ID '0F' for 'Swiss French';
    ID '13' for 'Turkey';
    ID '15' for 'Finland';
    ID '17' for 'Norway';
    ID '19' for 'Denmark';
    ID '1D' for 'Nederland';
    ID '1F' for 'Czech';
    ID '21' for 'Hungary';
    ID '23' for 'Slovak';
    ID '25' for 'Greece';
    ID '27' for 'Russia';
    ID 'FF' for 'Multilanguage';
    ID '3F' for 'Greece';
*/
  case 39: return 9; //Russian
  case 0x1F: return 10;
  case 0x15: return 11;
  case 0x17: return 12;
  case 0x3F: return 13;
  }
  return -1;
}


void TOptionBox::TOSRefreshBox(EasyStr Sel) {
  //SS this creates the TOS list in the option page
#ifdef WIN32
  HWND Win=GetDlgItem(Handle,8300);
  if(Win==NULL) 
    return;
  EnumDateFormats(EnumDateFormatsProc,LOCALE_USER_DEFAULT,DATE_SHORTDATE);
  SendMessage(Win,LB_RESETCONTENT,0,0);
  UpdateWindow(Win);
  SendMessage(Win,WM_SETREDRAW,0,0);
#endif
#ifdef UNIX
  if(tos_lv.handle==0)
    return;
  tos_lv.sl.DeleteAll(); //clear out the box
  tos_lv.display_mode=1;
  tos_lv.sl.Sort=eslNoSort;	
  tos_lv.lpig=&IcoTOSFlags;
  tos_lv.columns.DeleteAll();
  tos_lv.columns.Add(5+8+5+hxc::get_text_width(XD,"8.88")+15);
  tos_lv.columns.Add(page_w-hxc::get_text_width(XD,"12/12/2000")-15);
  tos_lv.text_trunc_mode=LVTTM_CUT;
//  EasyStringList eslTOS; // it's in the h...
  eslTOS.Sort2=eslSortByData0;
  //char LinkPath[MAX_PATH+1];
#endif
  EasyStr Fol=RunDir; // SS links are in rundir
  EasyStr VersionPath; // The first TOS found which matches the current TOS version
  eslTOS.DeleteAll();
  eslTOS.Sort=eslTOS_Sort;
  if(Sel.Empty())
    Sel=(NewROMFile.Empty()) ? ROMFile : NewROMFile;
  DirSearch ds;
  if(ds.Find(Fol+SLASH+"*.*")) 
  {
    EasyStr Path;
    do {
      Path=Tos.GetNextTos(ds);
      if(has_extension_list(Path,"IMG","ROM",NULL)) 
      {
        WORD Ver,Date;
        BYTE Country,Recognised;
        Tos.GetTosProperties(Path,Ver,Country,Date,Recognised);
        eslTOS.Add(3,Str(GetFileNameFromPath(Path))+"\01"+Path,
          Ver,Country|(Recognised<<16),Date);
        if(Ver==tos_version && VersionPath.Empty())
          VersionPath=Path;
      }
    } while(ds.Next());
    ds.Close();
  }
  int Selected=-1,VersionSel=-1,ROMFileSel=-1;
  int i=0,dir=1;
  if(eslTOS_Descend) 
  {
    i=eslTOS.NumStrings-1;
    dir=-1;
  }
  for(int idx=0;idx<eslTOS.NumStrings;idx++) 
  {
    char *FullPath=strrchr(eslTOS[i].String,'\01')+1;
#ifdef WIN32
    SendMessage(Win,LB_INSERTSTRING,idx,LPARAM(""));
#endif
#ifdef UNIX
    Str t;
    if(eslTOS[i].Data[0])
      t=HEXSl(eslTOS[i].Data[0],3).Insert(".",1);
    t+="\01";

    BYTE Recognised=(BYTE)(eslTOS[i].Data[1]>>16);
    if(Recognised)
      t+=(Recognised==1) ? "(V) " : "(x) "; // TODO colours
    t+=Str(GetFileNameFromPath(eslTOS[i].String));
    
    t+="\01";
    if(eslTOS[i].Data[0])
    {
      t+=Str(eslTOS[i].Data[2]&0x1f)+"/";
      t+=Str((eslTOS[i].Data[2]>>5)&0xf)+"/";
      t+=Str((eslTOS[i].Data[2]>>9)+1980);
    }
    t+="\01";
    t+=FullPath;
    tos_lv.sl.Add(t,101+TOSLangToFlagIdx(eslTOS[i].Data[1]));
#endif
    if(IsSameStr_I(FullPath,Sel)) 
      Selected=idx;
    if(IsSameStr_I(FullPath,ROMFile)) 
      ROMFileSel=idx;
    if(IsSameStr_I(FullPath,VersionPath)) 
      VersionSel=idx;
    i+=dir;
  }
  static bool Recursing=0;
  if(Selected<0 && ROMFileSel<0 && Exists(ROMFile)) 
  {
    if(!Recursing) 
    {
#ifdef WIN32
      EasyStr LinkName=WriteDir+SLASH +GetFileNameFromPath(ROMFile)+".lnk";
      int n=2;
      while(Exists(LinkName))
        LinkName=WriteDir+SLASH+GetFileNameFromPath(ROMFile)+" ("+(n++)+")"
          +".lnk";
      CreateLink(LinkName,ROMFile,T("TOS Image"));
#endif
#ifdef UNIX
      Str Name=GetFileNameFromPath(ROMFile),Ext;
      char *dot=strrchr(Name,'.');
      if(dot)
      {
        Ext=dot;
        *dot=0;
      }
      EasyStr LinkName=WriteDir+SLASH+Name+Ext;
      int n=2;
      while(Exists(LinkName))
        LinkName=WriteDir+SLASH+Name+"("+(n++)+")"+Ext;
      symlink(ROMFile,LinkName);
#endif
      Recursing=true;
      TOSRefreshBox(ROMFile);
      Recursing=false;
    }
  }
  else 
  {
    int iSel=Selected;
    if(iSel<0) 
      iSel=VersionSel;
    if(iSel<0) 
      iSel=MAX(ROMFileSel,0);
#ifdef WIN32
    SendMessage(Win,LB_SETCURSEL,iSel,0);
    SendMessage(Win,LB_SETCARETINDEX,iSel,0);
#endif
#ifdef UNIX
    tos_lv.contents_change();
    tos_lv.changesel(iSel);
#endif
  }
#ifdef WIN32
  SendMessage(Win,WM_SETREDRAW,1,0);
#endif
}


void TOptionBox::LoadProfile(char *File) {
#ifdef WIN32
  TNotify myNotify(T("Loading configuration"));
  // it can take some time, remove the possibility to act too fast!
  DestroyCurrentPage();
#endif
  TConfigStoreFile CSF(File);
  bool DisableSections[PSEC_NSECT];
  for(int i=0;i<PSEC_NSECT;i++) 
  {
    if(ProfileSection[i].ID>=0&&ProfileSection[i].ID<PSEC_NSECT)
      DisableSections[ProfileSection[i].ID]=(CSF.GetInt("ProfileSections",
        ProfileSection[i].Name,LVI_SI_CHECKED)==LVI_SI_UNCHECKED);
  }
  LoadAllDialogData(0,File,DisableSections,&CSF);
  // Get current settings
  BYTE CurMemConf[2];
  GetCurrentMemConf(CurMemConf);
  int CurMonSel=GetCurrentMonitorSel();
  Str ProfileROM=CSF.GetStr("Machine","ROM_File",ROMFile);
#if defined(SSE_GUI_CONFIG_FILE)
  if(strchr(ProfileROM.Text,SLASHCHAR)==NULL)
    ProfileROM=TOSBrowseDir+SLASH+ProfileROM;
#endif
  BYTE ProfileMemConf[2]={(BYTE)CSF.GetInt("Machine","Mem_Bank_1",
    CurMemConf[0]),(BYTE)CSF.GetInt("Machine","Mem_Bank_2",CurMemConf[1])};
  int ProfileMonSel=(CSF.GetInt("Machine","Colour_Monitor",
    mfp_gpip_no_interrupt & MFP_GPIP_COLOUR))==0;
#ifndef NO_CRAZY_MONITOR
  if(CSF.GetInt("Machine","ExMon",extended_monitor))
  {
    UINT pro_em_width=CSF.GetInt("Machine","ExMonWidth",em_width);
    UINT pro_em_height=CSF.GetInt("Machine","ExMonHeight",em_height);
    UINT pro_em_planes=CSF.GetInt("Machine","ExMonPlanes",em_planes);
    ProfileMonSel=2;
    for(int n=0;n<EXTMON_RESOLUTIONS;n++) 
    {
      if(pro_em_width==extmon_res[n][0]&&pro_em_height==extmon_res[n][1]&&
        pro_em_planes==extmon_res[n][2])
        ProfileMonSel=n+2;
    }
  }
#endif
  if(NewROMFile.Empty()) 
    if(NotSameStr_I(ROMFile,ProfileROM)) 
      NewROMFile=ProfileROM;
  if(NewMemConf0==-1) 
  {
    if(ProfileMemConf[0]!=CurMemConf[0]||ProfileMemConf[1]!=CurMemConf[1]) 
    {
      NewMemConf0=ProfileMemConf[0];
      NewMemConf1=ProfileMemConf[1];
    }
  }
  if(NewMonitorSel==-1)
    if(ProfileMonSel!=CurMonSel)
      NewMonitorSel=ProfileMonSel;
  // If profile was saved with settings pending, check they still need to pend
  if(IsSameStr_I(NewROMFile,ROMFile)) 
    NewROMFile="";
  if(NewMemConf0==CurMemConf[0]&&NewMemConf1==CurMemConf[1]) 
    NewMemConf0=-1;
  if(NewMonitorSel==CurMonSel) 
    NewMonitorSel=-1;
  CSF.Close();
  //if(Handle) 
    //SetForegroundWindow(Handle);
  CheckResetIcon();
  CheckResetDisplay();
#ifdef WIN32
  CreatePage(Page);
#endif
}


void TOptionBox::UpdateMacroRecordAndPlay(Str SelPath,int Type) {
  if(Handle==NULL) 
    return;
  TMacroFileOptions MFO;
#ifdef WIN32
  if(GetDlgItem(Handle,10000)==NULL) 
    return;
  if(SelPath.Empty()) 
  {
    HTREEITEM SelItem=(HTREEITEM)SendMessage(DTree.hTree,TVM_GETNEXTITEM,
      TVGN_CARET,0);
    SelPath=DTree.GetItemPath(SelItem);
    Type=DTree.GetItem(SelItem,TVIF_IMAGE).iImage;
  }
  bool CheckRec=0,CheckPlay=0;
  if(Type==1) 
  {
    if(macro_record && IsSameStr_I(macro_record_file,SelPath)) 
      CheckRec=true;
    if(macro_play && IsSameStr_I(macro_play_file,SelPath)) 
      CheckPlay=true;
  }
  SendDlgItemMessage(Handle,10011,BM_SETCHECK,CheckRec,0);
  SendDlgItemMessage(Handle,10012,BM_SETCHECK,CheckPlay,0);
  macro_file_options(MACRO_FILE_GET,SelPath,&MFO);
  CBSelectItemWithData(GetDlgItem(Handle,10016),MFO.allow_same_vbls);
  CBSelectItemWithData(GetDlgItem(Handle,10014),MFO.max_mouse_speed);
#endif//WIN32
#ifdef UNIX
  hxc_button *p_grp=(hxc_button*)hxc::find(page_p,2009);
  if (p_grp==NULL) return;
  hxc_button *p_rec=(hxc_button*)hxc::find(p_grp->handle,2010);
  hxc_button *p_play=(hxc_button*)hxc::find(p_grp->handle,2011);
  hxc_dropdown *p_ms=(hxc_dropdown*)hxc::find(p_grp->handle,2012);
  hxc_dropdown *p_ped=(hxc_dropdown*)hxc::find(p_grp->handle,2013);
  if (SelPath.Empty()){
    Type=-1;
    if (dir_lv.lv.sel>=0){
      SelPath=dir_lv.get_item_path(dir_lv.lv.sel);
      Type=dir_lv.sl[dir_lv.lv.sel].Data[DLVD_TYPE];
    }
  }
  ShowHideWindow(XD,p_grp->handle,Type==2);
  bool CheckRec=0,CheckPlay=0;
  if (Type==2){
    if (macro_record && IsSameStr_I(macro_record_file,SelPath)) CheckRec=true;
    if (macro_play && IsSameStr_I(macro_play_file,SelPath)) CheckPlay=true;
  }
  p_play->set_check(CheckPlay);
  p_rec->set_check(CheckRec);
  macro_file_options(MACRO_FILE_GET,SelPath,&MFO);
  p_ms->select_item_by_data(MFO.max_mouse_speed);
  p_ped->select_item_by_data(MFO.allow_same_vbls);
  p_ms->draw();
  p_ped->draw();
#endif
}


void TOptionBox::UpdateCaption() { // because some don't know
#ifdef WIN32
  if(Handle)
    SetWindowText(Handle,
    OPTION_ADVANCED ? T("Options") : T("Options (simplified)"));
#endif    
}


void TOptionBox::Show() {
  if(Handle!=NULL) 
  {
#ifdef WIN32
    ShowWindow(Handle,SW_SHOWNORMAL);
    SetForegroundWindow(Handle);
#endif
    return;
  }

#ifdef WIN32
  if(FullScreen) 
    Top=MAX(Top,MENUHEIGHT);
  ManageWindowClasses(SD_REGISTER);
  Handle=CreateWindowEx(WS_EX_CONTROLPARENT,"Steem Options","",    
    WS_CAPTION|WS_SYSMENU,Left,Top,400,320,ParentWin,NULL,Inst,NULL);
  if(HandleIsInvalid()) 
  {
    ManageWindowClasses(SD_UNREGISTER);
    return;
  }
  UpdateCaption();
  SetWindowLongPtr(Handle,GWLP_USERDATA,(LONG_PTR)this);
  MakeParent(HWND(FullScreen?StemWin:NULL));
  LoadIcons();
  PageTree=CreateWindowEx(512,WC_TREEVIEW,"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|
    TVS_HASLINES|TVS_SHOWSELALWAYS|TVS_HASBUTTONS|TVS_DISABLEDRAGDROP,
    0,0,100,OPTIONS_HEIGHT,Handle,(HMENU)60000,Inst,NULL);
  SendMessage(PageTree,TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)il);
  AddPageLabel(T("Machine"),9);
  AddPageLabel(T("ST Video"),18); // "machine 2"
  AddPageLabel("TOS",10);
  AddPageLabel("Keyboard/Mouse",17);
  AddPageLabel(T("I/O Ports"),12); // was Ports, confusing because it includes MIDI ports
  AddPageLabel(T("MIDI"),4);
  AddPageLabel(T("General"),0);
  AddPageLabel(T("Sound"),5);
  AddPageLabel(T("Display"),1);
  AddPageLabel(T("On Screen Display"),15);
  AddPageLabel(T("Fullscreen Mode"),3);
  AddPageLabel(T("Colour Control"),2);
  AddPageLabel(T("Configurations"),11); // was Profiles
  AddPageLabel(T("Record Input"),13); // was Macros
  AddPageLabel(T("Startup"),6);
  AddPageLabel(T("Icons"),14);
#ifndef SSE_NO_UPDATE
  AddPageLabel(T("Auto Update"),7);
#endif
  AddPageLabel(T("File Associations"),8);
  AddPageLabel("Misc.",16); // was SSE
  page_l=MIN(2+TreeGetMaxItemWidth(PageTree)+5+2+10,630-(page_w+10));
#if defined(SSE_X64_GUI)//quick dirty fix
  SetWindowPos(Handle,NULL,0,0,3+page_l+page_w+10+3+12,OPTIONS_HEIGHT+6+12
    +GuiSM.cy_caption(),SWP_NOZORDER|SWP_NOMOVE);
#else
  SetWindowPos(Handle,NULL,0,0,3+page_l+page_w+10+3,OPTIONS_HEIGHT+6
    +GuiSM.cy_caption(),SWP_NOZORDER|SWP_NOMOVE);
#endif
  SetWindowPos(PageTree,NULL,0,0,page_l-10,OPTIONS_HEIGHT,SWP_NOZORDER
    |SWP_NOMOVE);
  Focus=NULL;
  TreeSelectItemWithData(PageTree,Page);
  ShowWindow(Handle,SW_SHOW);
  SetFocus(Focus);
  if(StemWin!=NULL) 
    PostMessage(StemWin,WM_USER,1234,0);
#endif//WIN32

#ifdef UNIX
  page_lv.sl.DeleteAll();
  page_lv.sl.Sort=eslNoSort;

  page_lv.sl.Add(T("Machine"),101+ICO16_ST,9);
  page_lv.sl.Add("TOS",101+ICO16_CHIP,10);
  page_lv.sl.Add(T("Macros"),101+ICO16_MACROS,13);
  page_lv.sl.Add(T("Ports"),101+ICO16_PORTS,12);

  page_lv.sl.Add(T("General"),101+ICO16_TOOLS,0);
  page_lv.sl.Add(T("Sound"),101+ICO16_SOUND,5);
  page_lv.sl.Add(T("Display"),101+ICO16_DISPLAY,1);
  page_lv.sl.Add(T("On Screen Display"),101+ICO16_OSD,14);
  page_lv.sl.Add(T("Brightness")+"/"+T("Contrast"),101+ICO16_BRIGHTCON,2);
  page_lv.sl.Add(T("Profiles"),101+ICO16_PROFILE,11);
  page_lv.sl.Add(T("Startup"),101+ICO16_FUJI16,6);
  page_lv.sl.Add(T("Paths"),101+ICO16_FUJI16,15);

#if 1//defined(SSE_GUI_OPTION_PAGE) && defined(SSE_UNIX)
  page_lv.sl.Add(T("Misc."),101+ICO16_SSE_OPTION,16);
#endif
  
  page_lv.lpig=&Ico16;
  page_lv.display_mode=1;

  int lv_w=page_lv.get_max_width(XD);
  page_w=380;
  int w=lv_w+10+page_w+10;

  if (StandardShow(w,OPTIONS_HEIGHT,T("Options"),
      ICO16_OPTIONS,ButtonPressMask,(LPWINDOWPROC)WinProc)) return;

  control_parent.create(XD,Handle,lv_w,0,page_w+20,OPTIONS_HEIGHT,
                    NULL,this,BT_STATIC,"",0,hxc::col_bk);
  page_p=control_parent.handle;
  page_l=10;

  page_lv.select_item_by_data(Page,1);
  page_lv.id=60000;
  page_lv.create(XD,Handle,0,0,lv_w,OPTIONS_HEIGHT,listview_notify_proc,this);

  CreatePage(Page);

  if (StemWin) OptBut.set_check(true);

  XMapWindow(XD,Handle);
  XFlush(XD);
#endif
}


void TOptionBox::Hide() {
  if(Handle==NULL) 
    return;
#ifdef WIN32
  ShowWindow(Handle,SW_HIDE);
  if(FullScreen) 
    SetFocus(StemWin);
  DestroyCurrentPage();
  DestroyWindow(Handle);Handle=NULL;
  ImageList_Destroy(il);il=NULL;
  if(StemWin) 
    PostMessage(StemWin,WM_USER,1234,0);
  ManageWindowClasses(SD_UNREGISTER);
#endif
#ifdef UNIX
  if(XD==NULL)
    return;
  hints.remove_all_children(page_p);
  StandardHide();
  if(StemWin)
    OptBut.set_check(0);
#endif
}


void TOptionBox::EnableBorderOptions(bool WIN_ONLY(enable)) {
#ifdef WIN32
  if(enable==0)
    border=0;
  else
    border=border_last_chosen;
  CheckMenuRadioItem(StemWin_SysMenu,110,112,110+(border!=0),MF_BYCOMMAND);
  if(Handle==NULL||BorderOption==NULL) 
    return;
  EnableWindow(BorderOption,enable);
  SendMessage(BorderOption,CB_SETCURSEL,MIN(border,(BYTE)BIGGEST_DISPLAY),0);
#endif
}


#ifdef WIN32

void TOptionBox::ChangeScreenShotFormat(int NewFormat,Str Ext) {
  Disp.ScreenShotFormat=NewFormat;
  char *dot=strrchr(Ext,'.');
  if(dot) 
  {
    Ext=dot+1;
    dot=strrchr(Ext,')');
    if(dot) 
      *dot=0;
  }
  Disp.ScreenShotExt=Ext.LowerCase();
#if !defined(SSE_VID_NO_FREEIMAGE)
  Disp.ScreenShotFormatOpts=0;
  //Disp.FreeImageLoad();
  FillScreenShotFormatOptsCombo();
#endif
  if(Handle)
    if(GetDlgItem(Handle,IDC_SCREENSHOT_FORMAT)) 
      CBSelectItemWithData(GetDlgItem(Handle,IDC_SCREENSHOT_FORMAT),NewFormat);
}


#if !defined(SSE_VID_NO_FREEIMAGE)

void TOptionBox::ChangeScreenShotFormatOpts(int NewOpt)
{
  Disp.ScreenShotFormatOpts=NewOpt;
  //Disp.FreeImageLoad();
  if(Handle) 
    if(GetDlgItem(Handle,1052)) 
      CBSelectItemWithData(GetDlgItem(Handle,1052),NewOpt);
}

#endif


void TOptionBox::ChooseScreenShotFolder(HWND Win) {
  EnableAllWindows(0,Win);
  EasyStr NewFol=ChooseFolder(HWND(FullScreen?StemWin:Win),T("Pick a Folder"),
    ScreenShotFol);
  if(NewFol.NotEmpty()) 
  {
    NO_SLASH(NewFol);
    if(Handle) 
      if(GetDlgItem(Handle,1021)) 
        SendDlgItemMessage(Handle,1021,WM_SETTEXT,0,(LPARAM)(NewFol.Text));
    ScreenShotFol=NewFol;
  }
  SetForegroundWindow(Win);
  EnableAllWindows(true,Win);
}


void TOptionBox::UpdateForDSError() // DirectSound
{
  if(Handle==NULL) 
    return;
  for(int n=7099;n<7110;n++) 
    if(GetDlgItem(Handle,n)) 
      EnableWindow(GetDlgItem(Handle,n),0);
  for(int n=7049;n<7062;n++) 
    if(GetDlgItem(Handle,n)) 
      EnableWindow(GetDlgItem(Handle,n),0);
  for(int n=7200;n<7210;n++) 
    if(GetDlgItem(Handle,n)) 
      EnableWindow(GetDlgItem(Handle,n),0);
  if(GetDlgItem(Handle,7010)) 
    SendDlgItemMessage(Handle,7010,WM_SETTEXT,0,
      LPARAM((T("Current driver")+": None").Text));
}


void TOptionBox::ManageWindowClasses(bool Unreg) {
  char *ClassName="Steem Options";
  if(Unreg)
    UnregisterClass(ClassName,Inst);
  else
    RegisterMainClass(WndProc,ClassName,RC_ICO_OPTIONS);
}


void TOptionBox::LoadIcons() {
  if(Handle==NULL) 
    return;
  HIMAGELIST old_il=il;
  il=ImageList_Create(18,20,BPPToILC[BytesPerPixel]|ILC_MASK,10,10);
  if(il) 
  {
    ImageList_AddPaddedIcons(il,PAD_ALIGN_RIGHT,hGUIIcon[RC_ICO_OPS_GENERAL],
      hGUIIcon[RC_ICO_OPS_DISPLAY],hGUIIcon[RC_ICO_OPS_BRIGHTCON],
      hGUIIcon[RC_ICO_OPS_FULLSCREEN],hGUIIcon[RC_ICO_OPS_MIDI],
      hGUIIcon[RC_ICO_OPS_SOUND],hGUIIcon[RC_ICO_OPS_STARTUP],
      hGUIIcon[RC_ICO_OPS_UPDATE],
      hGUIIcon[RC_ICO_OPS_ASSOC],hGUIIcon[RC_ICO_OPS_MACHINE],
      hGUIIcon[RC_ICO_CHIP],
      hGUIIcon[RC_ICO_CFG],
      hGUIIcon[RC_ICO_EXTERNAL],hGUIIcon[RC_ICO_OPS_MACROS],
      hGUIIcon[RC_ICO_OPS_ICONS],hGUIIcon[RC_ICO_OPS_OSD],
      hGUIIcon[RC_ICO_OPS_SSE],
      hGUIIcon[RC_ICO_OPS_KBDMOUSE],
      hGUIIcon[RC_ICO_OPS_DISPLAY], 0);
  }
  if(GetDlgItem(Handle,60000)) 
    SendMessage(PageTree,TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)il);
  if(old_il) 
    ImageList_Destroy(old_il);
  if(GetDlgItem(Handle,7201)) 
    SendDlgItemMessage(Handle,7201,BM_RELOADICON,0,0); // Record
  if(GetDlgItem(Handle,10011)) 
    SendDlgItemMessage(Handle,10011,BM_RELOADICON,0,0); // Record macro
  if(GetDlgItem(Handle,10012)) 
    SendDlgItemMessage(Handle,10012,BM_RELOADICON,0,0); // Play macro
  if(Scroller.GetControlPage()) 
  {
    for(int i=14100;i<14100+RC_NUM_ICONS;i++) 
      if(GetDlgItem(Scroller.GetControlPage(),i)) 
        SendDlgItemMessage(Scroller.GetControlPage(),i,BM_RELOADICON,0,0);
  }
  UpdateDirectoryTreeIcons(&DTree);
  CreateBrightnessBitmap();
}


void TOptionBox::DestroyCurrentPage() {
  ToolsDeleteAllChildren(ToolTip,Handle);
  // Stop profiles saving out all check states when close
  if(GetDlgItem(Handle,11013)) 
    EnableWindow(GetDlgItem(Handle,11013),0);
  TStemDialog::DestroyCurrentPage();
  BorderOption=NULL;
  if(hBrightBmp) 
    DeleteObject(hBrightBmp);
  hBrightBmp=NULL;
}


bool TOptionBox::HasHandledMessage(MSG *mess) {
  if(Handle!=NULL) 
  {
    if(mess->message==WM_KEYDOWN)
    {
#if defined(SSE_GUI_KBD)
      // ???
      // arrows for radio buttons -> optionbox
      // return, esc for label edit -> system
      if(mess->wParam!=VK_RETURN && mess->wParam!=VK_ESCAPE)
#else
      if(mess->wParam==VK_TAB)
#endif
        return (IsDialogMessage(Handle,mess)!=0);
    }
    return 0;
  }
  else
    return 0;
}


void TOptionBox::SetBorder(int newborder) {


#if defined(SSE_EMU_THREAD)
  if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING)
    VideoLock.Lock();
  if(OPTION_EMUTHREAD&&runstate==RUNSTATE_RUNNING)
    SuspendRendering=true;
  VideoLock.Unlock();
#endif
  BYTE oldborder=border;
  TRACE_LOG("Option Border %d->%d\n",oldborder,newborder);
  if(!newborder)
    ChangeBorderSize(0);
  if(ChangeBorderModeRequest(newborder))
  {
    border=(BYTE)newborder;
    ChangeBorderSize(newborder);
    if(FullScreen) 
      change_fullscreen_display_mode(true);
    //change_window_size_for_border_change(oldborder,newborder);
    StemWinResize();
    if(newborder>=oldborder) //?
      draw(false);
    InvalidateRect(StemWin,NULL,0);
#if defined(SSE_VID_DD)
    if(Handle)
      if(GetDlgItem(Handle,210))
        EnableWindow(GetDlgItem(Handle,210),
          border==0&&draw_fs_blit_mode!=DFSM_FAKEFULLSCREEN);
#endif
  }
  else 
  {
    if(Handle) 
      SendMessage(GetDlgItem(Handle,17360+oldborder),BM_SETCHECK,TRUE,0);
      //if(GetDlgItem(Handle,207)) 
        //SendDlgItemMessage(Handle,207,CB_SETCURSEL,oldborder,0);
    border=oldborder;
  }
  CheckMenuRadioItem(StemWin_SysMenu,110,112,110+MIN((int)border,1),MF_BYCOMMAND);
  REFRESH_STATUS_BAR;
#if defined(SSE_EMU_THREAD)
  SuspendRendering=false;
#endif
}


#define GET_THIS This=(TOptionBox*)GetWindowLongPtr(Win,GWLP_USERDATA);

LRESULT CALLBACK TOptionBox::WndProc(HWND Win,UINT Mess,WPARAM wPar,
                                     LPARAM lPar) {
  if(DTree.ProcessMessage(Mess,wPar,lPar)) 
    return DTree.WndProcRet;
  LRESULT Ret=DefStemDialogProc(Win,Mess,wPar,lPar);
  if(StemDialog_RetDefVal) 
    return Ret;
  TOptionBox *This;
  WORD wpar_lo=LOWORD(wPar);
  WORD wpar_hi=HIWORD(wPar);
  switch(Mess) {
  case WM_COMMAND:
    GET_THIS;
    switch(wpar_lo) {
    case IDC_CPU_SPEED:
      if(wpar_hi==CBN_SELENDOK)
      {
        n_cpu_cycles_per_second=MAX(MIN((int)SendMessage(HWND(lPar),
          CB_GETITEMDATA,SendMessage(HWND(lPar),CB_GETCURSEL,0,0),0),
          (int)CPU_MAX_HERTZ),(int)CpuNormalHz);
        if(runstate==RUNSTATE_RUNNING) 
          osd_init_run(0);
        prepare_cpu_boosted_event_plans();
      }
      break;
    case IDC_VID_FREQUENCY: // hack
      if(wpar_hi==CBN_SELENDOK)
      {
        BYTE newfreq=(BYTE)SendMessage(HWND(lPar),CB_GETITEMDATA,
          SendMessage(HWND(lPar),CB_GETCURSEL,0,0),0);
        if(newfreq)
        {
          switch(newfreq) {
          case 50:
            if(Glue.m_ShiftMode&2)
              Glue.SetShiftMode(0);
            Glue.SetSyncMode(2);
            break;
          case 60:
            if(Glue.m_ShiftMode&2)
              Glue.SetShiftMode(0);
            Glue.SetSyncMode(0);
            break;
          case MONO_HZ:
            Glue.SetShiftMode(2);
            break;
          }//sw
          Glue.Update();
          Glue.previous_video_freq=newfreq;
          REFRESH_STATUS_BAR;
        }
      }
      break;
    case 700:
      if(wpar_hi==BN_CLICKED) 
      {
        AllowTaskSwitch=!AllowTaskSwitch;
        SendMessage(HWND(lPar),BM_SETCHECK,(AllowTaskSwitch==0),0);
      }
      break;
    case 800:
      if(wpar_hi==BN_CLICKED) 
      {
        PauseWhenInactive=!PauseWhenInactive;
        SendMessage(HWND(lPar),BM_SETCHECK,PauseWhenInactive,0);
      }
      break;
    case 801:
      if(wpar_hi==BN_CLICKED) 
      {
        MuteWhenInactive=!(MuteWhenInactive!=0);
        SendMessage(HWND(lPar),BM_SETCHECK,MuteWhenInactive,0);
      }
      break;
#if 0
    case 900:
      if(wpar_hi==BN_CLICKED) 
      {
        floppy_access_ff=!floppy_access_ff;
        SendMessage(HWND(lPar),BM_SETCHECK,floppy_access_ff,0);
      }
      break;
#endif
    case 901:
      if(wpar_hi==BN_CLICKED) 
      {
        StartEmuOnClick=!StartEmuOnClick;
        SendMessage(HWND(lPar),BM_SETCHECK,StartEmuOnClick,0);
      }
      break;
    case 201:
      if(wpar_hi==CBN_SELENDOK) 
      {
        frameskip=(int)SendMessage(HWND(lPar),CB_GETCURSEL,0,0)+1;
        if(frameskip==5)
          frameskip=AUTO_FRAMESKIP;
      }
      break;
    case 203:
      if(wpar_hi==BN_CLICKED) {
        //            This->ChangeOSD(!osd_on);
      }
      break;
#if defined(SSE_VID_DD)
    case 204:
      if(wpar_hi==CBN_SELENDOK) 
      {
        draw_fs_blit_mode=(BYTE)SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
        This->UpdateFullscreen();
        if(draw_grille_black<4) 
          draw_grille_black=4;
      }
      break;
#endif
    case 206:
      if(wpar_hi==BN_CLICKED) 
      {
        FSDoVsync=!FSDoVsync;
        SendMessage(HWND(lPar),BM_SETCHECK,FSDoVsync,0);
      }
      break;
#if defined(SSE_VID_D3D)
    case 209:
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_FULLSCREEN_DEFAULT_HZ=!OPTION_FULLSCREEN_DEFAULT_HZ;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_FULLSCREEN_DEFAULT_HZ,0);
        TRACE_LOG("Option FullScreenDefaultHz = %d\n",OPTION_FULLSCREEN_DEFAULT_HZ);
        if(FullScreen && D3D9_OK)
          Disp.ScreenChange();
      }
      break;
#endif
#if defined(SSE_VID_D3D_FAKE_FULLSCREEN)
    case 213:
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_FAKE_FULLSCREEN=!OPTION_FAKE_FULLSCREEN;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_FAKE_FULLSCREEN,0);
        TRACE_LOG("Option FakeFullScreen = %d\n",OPTION_FAKE_FULLSCREEN);
        if(FullScreen&&D3D9_OK)
          Disp.ScreenChange();
        else
        {
          //if(!OPTION_FAKE_FULLSCREEN)
          //  OPTION_FULLSCREEN_GUI=0;
          This->DestroyCurrentPage(); // rough
          This->CreatePage(This->Page);
        }
      }
      break;
#endif
    case 214:
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_MAX_FS=!OPTION_MAX_FS;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_MAX_FS,0);
        TRACE_LOG("Option FullScreen on Maximize = %d\n",OPTION_MAX_FS);
      }
      break;
    case IDC_TOGGLE_FULLSCREEN: //Go Windowed now, Go Fullscreen now
      if(wpar_hi==BN_CLICKED) 
      {
        if(FullScreen)
        {
          Disp.ChangeToWindowedMode(StatusInfo.MessageIndex
            ==TStatusInfo::BLIT_ERROR);
        }
        else
          PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,2);
      }
      break;
    case IDC_RESET_DISPLAY:
      if(wpar_hi==BN_CLICKED) 
      {
        Disp.nUseMethod=0;
#if defined(SSE_EMU_THREAD)
        if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING)
          VideoLock.Lock();
#endif
        Disp.Init();
#if defined(SSE_EMU_THREAD)
        VideoLock.Unlock();
#endif
        Disp.ScreenChange(); // surfaces
      }
      break;
#if defined(SSE_GUI_TOOLBAR)
    case IDC_TOOLBAR_TASKBAR:
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_TOOLBAR_TASKBAR=!OPTION_TOOLBAR_TASKBAR;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_TOOLBAR_TASKBAR,0);
        if(ToolBar.IsVisible())
        {
          ToolBar.Hide();
          ToolBar.Show();
        }
      }
      break;
    case IDC_TOOLBAR_VERTICAL:
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_TOOLBAR_VERTICAL=!OPTION_TOOLBAR_VERTICAL;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_TOOLBAR_VERTICAL,0);
        if(ToolBar.IsVisible())
        {
          ToolBar.Hide();
          ToolBar.Show();
        }
      }
      break;
#endif
#if defined(SSE_VID_DD)
    case 208:
      if(wpar_hi==CBN_SELENDOK) 
      {
        INT_PTR i=SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
        if(i<NFSRES && Disp.fs_res[i].x)
        {
          Disp.fs_res_choice=(BYTE)i;
          TRACE_LOG("fs customp %d:%dx%d\n",i,Disp.fs_res[i].x,Disp.fs_res[i].y);
        }
      }
      break;
#endif
#if defined(SSE_VID_DD)
    case 210:
      if(wpar_hi==BN_CLICKED) 
      {
        prefer_res_640_400=!prefer_res_640_400;
        //This->DestroyCurrentPage(); // rough
        //This->CreatePage(This->Page);
        SendMessage(HWND(lPar),BM_SETCHECK,prefer_res_640_400,0);
      }
      break;
#endif
    case (IDC_RADIO_ST_MODEL+STE): // option ST model
    case (IDC_RADIO_ST_MODEL+STF):
    case (IDC_RADIO_ST_MODEL+MEGA_ST):
    case (IDC_RADIO_ST_MODEL+STFM):
#if defined(SSE_MEGASTE)
    case (IDC_RADIO_ST_MODEL+MEGA_STE):
#endif
      if(wpar_hi==BN_CLICKED
        && SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED)
      {
        BYTE new_st_model=(BYTE)(wpar_lo-17340);
        TRACE_LOG("Option ST type = %d\n",new_st_model);
        SSEConfig.SwitchSTModel(new_st_model);
        if(OPTION_ST_PRESELECT && runstate==RUNSTATE_STOPPED)
        { // try to load corresponding config (not if changing live)
          Str Cfg=This->ProfileDir;
          Cfg+=SLASH;
          Cfg+=st_model_name[ST_MODEL];
          Cfg+=".";
          Cfg+=CONFIG_FILE_EXT;
          //TRACE("%s\n",Cfg.Text);
          if(Exists(Cfg.Text))
            This->LoadProfile(Cfg.Text);
        }
      }
      This->MachineUpdateIfVisible(); //anyway
      if(DiskMan.IsVisible())
      {
        SendMessage(GetDlgItem(DiskMan.Handle,10),BM_SETCHECK,
          !HardDiskMan.DisableHardDrives||HardDiskMan.IsVisible(),0);
      }
      break;
    case IDC_GLU_WAKEUP0:
      if(wpar_hi==EN_CHANGE)
      {
        char buf[12];
        SendMessage(HWND(lPar),WM_GETTEXT,3,(LPARAM)buf);
        int wished_ws=atoi(buf); // order of option is different for historical reasons
        switch(wished_ws) {
        case 1: OPTION_WS=4; break;
        case 2: OPTION_WS=1; break;
        case 3: OPTION_WS=3; break;
        case 4: OPTION_WS=2; break;
        }
        TRACE_LOG("Option WS = %d\n",OPTION_WS);
        Shifter.Preload=0; // reset the thing!
        Glue.Update();
      }
      break;
#if defined(SSE_VID_DD)
    case 220:case 222:case 224:case 226:
      if(wpar_hi==CBN_SELENDOK) 
      {
        int i=(wpar_lo-220)/2;
        int new_hz=HzIdxToHz[SendMessage(HWND(lPar),CB_GETCURSEL,0,0)];
        if(prefer_pc_hz[i]!=new_hz) 
        {
          prefer_pc_hz[i]=new_hz;
          int current_i=int((border)?2:1);
          if(FullScreen && current_i==i) 
          {
            if(IDYES==Alert(T("Do you want to test this video frequency now?"),
              T("Change Monitor Frequency"),MB_YESNO|MB_DEFBUTTON1
              |MB_ICONQUESTION)) {
              change_fullscreen_display_mode(false);
              palette_convert_all();
              draw(false);
              InvalidateRect(StemWin,NULL,0);
            }
          }
          This->UpdateFullscreen();
        }
      }
      break;
#endif
    case 230:
      if(wpar_hi==BN_CLICKED) 
      {
        FSQuitAskFirst=!FSQuitAskFirst;
        SendMessage(HWND(lPar),BM_SETCHECK,FSQuitAskFirst,0);
      }
      break;
#if defined(SSE_VID_DD)
    case 280:
      if(wpar_hi==BN_CLICKED) 
      {
        draw_fs_fx=(BYTE)((SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED
          ? DFSFX_GRILLE : DFSFX_NONE));
        if(draw_grille_black<4) 
          draw_grille_black=4;
        if(runstate!=RUNSTATE_RUNNING && FullScreen) 
          draw(false);
      }
      break;
#endif
    case 300:
      if(wpar_hi==BN_CLICKED) 
      {
        ResChangeResize=!ResChangeResize;
        SendMessage(HWND(lPar),BM_SETCHECK,ResChangeResize,0);
      }
      break;
    case 302:case 304:case 306:   //SS Window Size Low Medium High
      if(wpar_hi==CBN_SELENDOK) 
      {
#if defined(SSE_EMU_THREAD)
        if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING)
          VideoLock.Lock();
#endif
        int Res=(wpar_lo-302)/2;
        bool redraw=0;
        DWORD dat=(DWORD)CBGetSelectedItemData(HWND(lPar));
        WinSizeForRes[Res]=LOWORD(dat);
        if(Res<2) 
        {
          if(draw_win_mode[Res]!=HIWORD(dat)) 
          {
            #ifdef STEEM_CRT
              draw_win_mode[Res]=0;
            #else
              draw_win_mode[Res]=HIWORD(dat);
            #endif
            redraw=true;
          }
        }
        if(Res==int(video_mixed_output?1:screen_res)) 
        {
          if(redraw && FullScreen==0) 
          {
            if(draw_grille_black<4)
              draw_grille_black=4;
            draw(false);
          }
          if(ResChangeResize) 
            StemWinResize();
        }
#if defined(SSE_EMU_THREAD)
        VideoLock.Unlock();
#endif
      }
      break;
    case 400:
      if(wpar_hi==BN_CLICKED) 
      {
        ShowTips=!ShowTips;
        SendMessage(HWND(lPar),BM_SETCHECK,ShowTips,0);
        SendMessage(ToolTip,TTM_ACTIVATE,ShowTips,0);
      }
      break;
    case 1022:
      if(wpar_hi==BN_CLICKED) 
      {
        SendMessage(HWND(lPar),BM_SETCHECK,1,true);
        This->ChooseScreenShotFolder(Win);
        SetFocus(HWND(lPar));
        SendMessage(HWND(lPar),BM_SETCHECK,0,true);
      }
      break;
    case 1023:
      if(wpar_hi==BN_CLICKED) 
        ShellExecute(NULL,NULL,ScreenShotFol,"","",SW_SHOWNORMAL);
      break;
    case 1024:
      if(wpar_hi==BN_CLICKED) 
      {
        Disp.ScreenShotMinSize=!Disp.ScreenShotMinSize;
        SendMessage(HWND(lPar),BM_SETCHECK,Disp.ScreenShotMinSize,0);
      }
      break;
/*  Reset colour controls.
    We set the sliders to centre and send a message for each.
    TBM_SETPOSNOTIFY would do it in one line but it's only for Windows 7 and up.
*/
    case 1025:
      for(int i=0;i<5;i++) 
      {
        HWND handle=GetDlgItem(This->Handle,2000+1+i*2);
        SendMessage(handle,TBM_SETPOS,1,128);
        SendMessage(This->Handle,WM_HSCROLL,0,(LPARAM)handle);
      }
      break;
    case 1027:
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_HACKS=!OPTION_HACKS;
        TRACE_LOG("Option Hacks %d\n",OPTION_HACKS);
        This->DestroyCurrentPage(); // rough
        This->CreatePage(This->Page);
        //SendMessage(HWND(lPar),BM_SETCHECK,OPTION_HACKS,0);
      }
      break;
#if defined(SSE_HD6301_LL) // Option 6301 emu
    case IDC_6301:
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_C1=!OPTION_C1;
        if(!HD6301_OK)
          OPTION_C1=0;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_C1,0);
        TRACE_LOG("Option HD6301 emu: %d\n",OPTION_C1);
      }
      break;
#endif
    case 1031:
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_EMU_DETECT=!OPTION_EMU_DETECT;
        TRACE_LOG("Emu detect: %d\n",OPTION_EMU_DETECT);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_EMU_DETECT,0);
        emudetect_reset();
      }
      break;
    case IDC_ST_AR:
      if(wpar_hi==BN_CLICKED)
      {
#if defined(SSE_EMU_THREAD)
        if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING)
          VideoLock.Lock();
#endif
        OPTION_ST_ASPECT_RATIO=!OPTION_ST_ASPECT_RATIO;
        TRACE_LOG("ST Aspect Ratio: %d\n",OPTION_ST_ASPECT_RATIO);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_ST_ASPECT_RATIO,0);
        StemWinResize();
#if defined(SSE_EMU_THREAD)
        VideoLock.Unlock();
#endif
      }
      break;
    case IDC_SCANLINES: // option scanlines
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_SCANLINES=!OPTION_SCANLINES;
        TRACE_LOG("Scanlines: %d\n",OPTION_SCANLINES);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_SCANLINES,0);
        Disp.ScreenChange(); // create new surfaces
      }
      break;
#if defined(SSE_VID_VSYNC_WINDOW) // Option Window VSync
    case 1033:
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_WIN_VSYNC=!OPTION_WIN_VSYNC;
        TRACE_LOG("Option Window VSync: %d\n",OPTION_WIN_VSYNC);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_WIN_VSYNC,0);
#if defined(SSE_VID_D3D)
        Disp.ScreenChange(); // create new surfaces
#endif
      }
      break;
#endif
#if defined(SSE_VID_3BUFFER_FS) 
    case 1037: // Option Triple Buffer FS
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_3BUFFER_FS=!OPTION_3BUFFER_FS;
        TRACE_LOG("Option Triple Buffer FS: %d\n",OPTION_3BUFFER_FS);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_3BUFFER_FS,0);
        Disp.ScreenChange(); // must delete and create surface
      }
      break;
#endif
#if defined(SSE_VID_DD_3BUFFER_WIN)
    case 1034: // Option Triple Buffer Win
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_3BUFFER_WIN=!OPTION_3BUFFER_WIN;
        TRACE_LOG("Option Triple Buffer Win: %d\n",OPTION_3BUFFER_WIN);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_3BUFFER_WIN,0);
        Disp.ScreenChange(); // must delete and create surface
      }
      break;
#endif
#if defined(SSE_VID_D3D_FLIPEX)
    case IDC_FLIPEX: // Option Triple Buffer Win
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_FLIPEX=!OPTION_FLIPEX;
        TRACE_LOG("Option FlipEx: %d\n",OPTION_FLIPEX);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_FLIPEX,0);
        if(!OPTION_FLIPEX && OPTION_WARNINGS)
          Alert(T("When you disable FlipEx, the system may somehow think that it\
 is still enabled, you may have to leave and restart Steem"),T("Warning"),
          MB_ICONEXCLAMATION);
        Disp.ScreenChange();
      }
      break;
#endif
    case IDC_VMMOUSE:
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_VMMOUSE=!OPTION_VMMOUSE;
        TRACE_LOG("Option VMMouse: %d\n",OPTION_VMMOUSE);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_VMMOUSE,0);
      }
      break;
#if defined(SSE_OSD_SHOW_TIME)
    case 1036:
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_OSD_TIME=!OPTION_OSD_TIME;
        OsdControl.StartingTime=timeGetTime(); //reset
        OsdControl.StoppingTime=0; //reset
        TRACE_LOG("Option OsdTime: %d\n",OPTION_OSD_TIME);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_OSD_TIME,0);
      }
      break;
#endif
#if defined(SSE_OSD_DEBUGINFO)
    case IDC_OSD_DEBUGINFO:
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_OSD_DEBUGINFO=!OPTION_OSD_DEBUGINFO;
        TRACE_LOG("OPTION_OSD_DEBUGINFO: %d\n",OPTION_OSD_DEBUGINFO);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_OSD_DEBUGINFO,0);
      }
      break;
#endif
#if defined(SSE_OSD_FPS_INFO)
    case IDC_OSD_FPSINFO:
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_OSD_FPSINFO=!OPTION_OSD_FPSINFO;
        TRACE_LOG("OPTION_OSD_FPSINFO: %d\n",OPTION_OSD_FPSINFO);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_OSD_FPSINFO,0);
      }
      break;
#endif
    case IDC_ADVANCED_SETTINGS: // toggle Advanced Settings
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_ADVANCED=!OPTION_ADVANCED;
        TRACE_LOG("Option Advanced Settings: %d\n",OPTION_ADVANCED);
        if(!OPTION_ADVANCED)
          SSEOptions.Restore();
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_ADVANCED,0);
        This->UpdateCaption();
      }
      break;
    case 1039: // reset Advanced Settings - so it's player's choice
      if(wpar_hi==BN_CLICKED)
      {
        SSEOptions.Restore(true);
        This->DestroyCurrentPage(); // rough
        This->CreatePage(This->Page);
      }
      break;
    case IDC_SCREENSHOT_FORMAT:
      if(wpar_hi==CBN_SELENDOK) 
      {
        Str Ext;
        Ext.SetLength(200);
        SendMessage(HWND(lPar),CB_GETLBTEXT,SendMessage(HWND(lPar),CB_GETCURSEL,
          0,0),LPARAM(Ext.Text));
        This->ChangeScreenShotFormat((int)CBGetSelectedItemData(HWND(lPar)),Ext);
      }
      break;
#if !defined(SSE_VID_NO_FREEIMAGE)
    case 1052:
      if(wpar_hi==CBN_SELENDOK)
        This->ChangeScreenShotFormatOpts((int)CBGetSelectedItemData(HWND(lPar)));
      break;
#endif
    case 1030:
      if(wpar_hi==BN_CLICKED) 
      {
        HighPriority=!HighPriority;
        SendMessage(HWND(lPar),BM_SETCHECK,HighPriority,0);
        if(runstate==RUNSTATE_RUNNING)
          SetPriorityClass(GetCurrentProcess(),
          (HighPriority?HIGH_PRIORITY_CLASS:NORMAL_PRIORITY_CLASS));
      }
      break;
#if defined(SSE_EMU_THREAD)
    case IDC_EMU_THREAD:
      if(wpar_hi==BN_CLICKED) 
      {
        if(runstate==RUNSTATE_STOPPED)
          OPTION_EMUTHREAD=!OPTION_EMUTHREAD;
        else // should be disabled when running
          EnableWindow(HWND(lPar),FALSE);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_EMUTHREAD,0);
        TRACE_LOG("Option Emu Thread: %d\n",OPTION_EMUTHREAD);
      }
      break;
#endif
    case 1048:
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_SPURIOUS=!OPTION_SPURIOUS;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_SPURIOUS,0);
        TRACE_LOG("Option Spurious: %d\n",OPTION_SPURIOUS);
      }
      break;
    case IDC_UNSTABLE_SHIFTER:
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_UNSTABLE_SHIFTER=!OPTION_UNSTABLE_SHIFTER;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_UNSTABLE_SHIFTER,0);
        TRACE_LOG("UnstableShifter: %d\n",OPTION_UNSTABLE_SHIFTER);
      }
      break;
#if defined(SSE_GUI_LEGACY_TOOLBAR)
    case IDC_LEGACY_TOOLBAR:
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_LEGACY_TOOLBAR=!OPTION_LEGACY_TOOLBAR;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_LEGACY_TOOLBAR,0);
        TRACE_LOG("LegacyToolbar: %d\n",OPTION_LEGACY_TOOLBAR);
        RECT rc;
        GetClientRect(StemWin,&rc);
        GUIToolbarArrangeIcons(rc.right-rc.left);
        InvalidateRect(StemWin,NULL,TRUE);
      }
      break;
#endif
    case IDC_WARNINGS:
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_WARNINGS=!((OPTION_WARNINGS&1)!=0);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_WARNINGS,0);
        TRACE_LOG("Warnings: %d\n",OPTION_WARNINGS);
      }
      break;
    case IDC_YM2149_ON:
      if(wpar_hi==BN_CLICKED) 
      {
        SSEConfig.YmSoundOn=!SSEConfig.YmSoundOn;
        SendMessage(HWND(lPar),BM_SETCHECK,SSEConfig.YmSoundOn,0);
      }
      break;
    case IDC_STESOUND_ON:
      if(wpar_hi==BN_CLICKED) 
      {
        SSEConfig.SteSoundOn=!SSEConfig.SteSoundOn;
        SendMessage(HWND(lPar),BM_SETCHECK,SSEConfig.SteSoundOn,0);
      }
      break;
    case IDC_RANDOM_WU:
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_RANDOM_WU=!OPTION_RANDOM_WU;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_RANDOM_WU,0);
      }
      break;
    case IDC_OSD_NONEONSTOP:
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_NO_OSD_ON_STOP=!OPTION_NO_OSD_ON_STOP;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_NO_OSD_ON_STOP,0);
      }
      break;
    case 3001:
      if(wpar_hi==CBN_SELENDOK) 
      {
        INT_PTR CurSel=SendDlgItemMessage(Win,3001,CB_GETCURSEL,0,0);
        if(CurSel==0)
          WriteCSFStr("Options","DSDriverName","",globalINIFile);
        else if(CurSel>0) 
        {
          EasyStr DrivName;
          DrivName.SetLength(SendDlgItemMessage(Win,3001,CB_GETLBTEXTLEN,CurSel,
            0));
          SendDlgItemMessage(Win,3001,CB_GETLBTEXT,CurSel,LPARAM(DrivName.Text));
          WriteCSFStr("Options","DSDriverName",DrivName,globalINIFile);
        }
        InitSound();
      }
      break;
    case 3303:
      if(wpar_hi==BN_CLICKED) 
      {
        AutoLoadSnapShot=!AutoLoadSnapShot;
        SendMessage(HWND(lPar),BM_SETCHECK,AutoLoadSnapShot,0);
      }
      break;
    case 3311:
      if(wpar_hi==EN_UPDATE) 
      {
        EasyStr NewName;
        int Len=(int)SendMessage(HWND(lPar),WM_GETTEXTLENGTH,0,0)+1;
        AutoSnapShotName.SetLength(Len);
        SendMessage(HWND(lPar),WM_GETTEXT,Len,LPARAM(AutoSnapShotName.Text));
        bool SetText=0;
        for(int i=0;i<AutoSnapShotName.Length();i++) 
        {
          switch(AutoSnapShotName[i]) {
          case ':':case '/':
          case '"':case '<':
          case '>':case '|':
          case '*':case '?':
          case '\\':
            AutoSnapShotName[i]='-';
            SetText=true;
            break;
          }
        }
        if(SetText) 
        {
          DWORD Start,End;
          SendMessage(HWND(lPar),EM_GETSEL,WPARAM(&Start),LPARAM(&End));
          SendMessage(HWND(lPar),WM_SETTEXT,0,LPARAM(AutoSnapShotName.Text));
          SendMessage(HWND(lPar),EM_SETSEL,Start,End);
        }
      }
      break;
    case 3300:case 3301:case 3302:case 3304:case 3305:case 3307:case 3308:
      if(wpar_hi==BN_CLICKED) 
      {
        char *key;
        switch(wpar_lo) {
        //case 3300:
        default:
          key="NoDirectDraw";
          EnableWindow(GetDlgItem(Win,3302),!SendMessage(HWND(lPar),
            BM_GETCHECK,0,0));
          EnableWindow(GetDlgItem(Win,3304),!SendMessage(HWND(lPar),
            BM_GETCHECK,0,0));
          EnableWindow(GetDlgItem(Win,3305),!SendMessage(HWND(lPar),
            BM_GETCHECK,0,0));
          break;
#if !defined(SSE_SOUND_NO_NOSOUND_OPTION)
        case 3301:
          key="NoDirectSound";
          break;
#endif
        case 3302:
          key="StartFullscreen";
          break;
        case 3304:
          key="DrawToVidMem";
          if(OPTION_HACKS) // because it's supposed to be startup, + in case of bug
          {
            Disp.DrawToVidMem=(SendMessage(HWND(lPar),BM_GETCHECK,0,0)==TRUE); // immediate effect!
            TRACE_LOG("DrawToVidMem %d\n",Disp.DrawToVidMem);
            Disp.ScreenChange(); // -> create surfaces
          }
          break;
        case 3305:
          key="BlitHideMouse";
          break;
        case 3307:
          key="TraceFile";
          break;
        case 3308:
          key="RunOnStart";
          break;
        }
        WriteCSFStr("Options",key,EasyStr(SendMessage(HWND(lPar),
          BM_GETCHECK,0,0)),globalINIFile);
      }
      break;
#if defined(SSE_SOUND_OPTION_DISABLE_DSP)
    case 3306:
      DSP_DISABLED=!DSP_DISABLED;
      break;
#endif
#if !defined(SSE_NO_UPDATE)
    case 4200:case 4201:case 4202:case 4203:
      if(wpar_hi==BN_CLICKED) {
        TConfigStoreFile CSF(globalINIFile);
        CSF.SetStr("Update","AutoUpdateEnabled",
          LPSTR(SendMessage(GetDlgItem(Win,4200),BM_GETCHECK,0,0)==0?"1":"0"));
        CSF.SetStr("Update","AlwaysOnline",
          LPSTR(SendMessage(GetDlgItem(Win,4201),BM_GETCHECK,0,0)?"1":"0"));
        CSF.SetStr("Update","PatchDownload",
          LPSTR(SendMessage(GetDlgItem(Win,4202),BM_GETCHECK,0,0)?"1":"0"));
        CSF.SetStr("Update","AskPatchInstall",
          LPSTR(SendMessage(GetDlgItem(Win,4203),BM_GETCHECK,0,0)?"1":"0"));
        CSF.Close();
      }
      break;
    case 4400:
      if(wpar_hi==BN_CLICKED) {
        EasyStr Online=LPSTR(SendMessage(GetDlgItem(Win,4201),BM_GETCHECK,0,0)?" online":"");
        EasyStr NoPatch=LPSTR(SendMessage(GetDlgItem(Win,4202),BM_GETCHECK,0,0)==0?" nopatchcheck":"");
        EasyStr AskPatch=LPSTR(SendMessage(GetDlgItem(Win,4203),BM_GETCHECK,0,0)?" askpatchinstall":"");
        WinExec(EasyStr("\"")+RunDir+"\\SteemUpdate.exe\""+Online+NoPatch+AskPatch,SW_SHOW);
      }
      break;
#endif
    case 5100:case 5101:case 5102:case 5103:case 5104:case 5105:case 5106:
    case 5107:case 5108:case 5109:
      if(wpar_hi==BN_CLICKED) 
      {
        EasyStr Ext;
        switch(wpar_lo) {
        case 5100: Ext=dot_ext(EXT_ST);AssociateSteem(Ext,"st_disk_image"); 
          break;
        case 5101: Ext=dot_ext(EXT_STT);AssociateSteem(Ext,"st_disk_image"); 
          break;
        case 5102: Ext=dot_ext(EXT_MSA);AssociateSteem(Ext,"st_disk_image"); 
          break;
#if defined(SSE_DISK_STW) // STW instead because it's less likely to be zipped
        case 5103: Ext=dot_ext(EXT_STW);AssociateSteem(Ext,"stw_disk_image"); 
#elif USE_PASTI
        case 5103: Ext=dot_ext(EXT_STX);AssociateSteem(Ext,"st_pasti_disk_image");
#endif
          break;
        case 5104: Ext=dot_ext(EXT_DIM);AssociateSteem(Ext,"st_disk_image"); 
          break;
        case 5105: Ext=".STZ";AssociateSteem(Ext,"st_disk_image"); break;
        case 5106: Ext=".STS";AssociateSteem(Ext,"steem_memory_snapshot"); 
          break;
#if defined(SSE_DISK_HFE) // more useful
        case 5107: Ext=dot_ext(EXT_HFE);AssociateSteem(Ext,"st_hfe_disk_image");
#else
        case 5107: Ext=".STC";AssociateSteem(Ext,"st_cartridge");
#endif
           break;
#if defined(SSE_TOS_PRG_AUTORUN)
        case 5108: Ext=dot_ext(EXT_PRG);AssociateSteem(Ext,
          "st_atari_prg_executable"); break;
        case 5109:Ext=dot_ext(EXT_TOS); AssociateSteem(Ext,
          "st_atari_prg_executable"); break;
#endif
        }
        HWND But=HWND(lPar);
        if(IsSteemAssociated(Ext)) 
          SendMessage(But,WM_SETTEXT,0,LPARAM(T("Associated").Text));
        else
          SendMessage(But,WM_SETTEXT,0,LPARAM(T("Associate").Text));
        EnableWindow(But,true);
        }
      break;
    case 5502:
    {
      if(wpar_hi!=BN_CLICKED) 
        break;
      TConfigStoreFile CSF(globalINIFile);
      bool OpenFilesInNew=!CSF.GetInt("Options","OpenFilesInNew",true);
      CSF.SetInt("Options","OpenFilesInNew",OpenFilesInNew);
      CSF.Close();
      SendMessage(HWND(lPar),BM_SETCHECK,OpenFilesInNew,0);
      break;
    }
    case 6010:
      if(wpar_hi==BN_CLICKED) 
      {
        MIDI_out_running_status_flag=!MIDI_out_running_status_flag;
        SendMessage(HWND(lPar),BM_SETCHECK,int((MIDI_out_running_status_flag==MIDI_ALLOW_RUNNING_STATUS)?BST_CHECKED:BST_UNCHECKED),0);
      }
      break;
    case 6011:
      if(wpar_hi==BN_CLICKED) 
      {
        MIDI_in_running_status_flag=!MIDI_in_running_status_flag;
        SendMessage(HWND(lPar),BM_SETCHECK,int((MIDI_in_running_status_flag==MIDI_ALLOW_RUNNING_STATUS)?BST_CHECKED:BST_UNCHECKED),0);
      }
      break;
    case 6021:case 6023:
      if(wpar_hi==CBN_SELENDOK) 
      {
        MIDI_out_n_sysex=(int)SendDlgItemMessage(Win,6021,CB_GETCURSEL,0,0)+2;
        MIDI_out_sysex_max=(16<<SendDlgItemMessage(Win,6023,CB_GETCURSEL,0,0))
          *1024;
        for(int i=0;i<3;i++)
          if(STPort[i].MIDI_Out) 
            STPort[i].MIDI_Out->ReInitSysEx();
      }
      break;
    case 6031:case 6033:
      if(wpar_hi==CBN_SELENDOK) 
      {
        MIDI_in_n_sysex=(int)SendDlgItemMessage(Win,6031,CB_GETCURSEL,0,0)+2;
        MIDI_in_sysex_max=(16<<SendDlgItemMessage(Win,6033,CB_GETCURSEL,0,0))
          *1024;
        for(int i=0;i<3;i++)
          if(STPort[i].MIDI_In) 
            STPort[i].MIDI_In->ReInitSysEx();
      }
      break;
    case ID_BRIGHTNESS_MAP:
      if(wpar_hi==BN_CLICKED) 
        This->FullscreenBrightnessBitmap();
      break;
    case 7101:
      if(wpar_hi==CBN_SELENDOK)
      {
        LRESULT freq=CBGetSelectedItemData(HWND(lPar));
        if(freq) 
          sound_chosen_freq=(int)freq;
        This->UpdateSoundFreq();
      }
      break;
    case 7061:
      if(wpar_hi==CBN_SELENDOK) 
      {
        WORD dat=(WORD)CBGetSelectedItemData(HWND(lPar));
        This->ChangeSoundFormat(LOBYTE(dat),HIBYTE(dat));
      }
      break;
#if !defined(SSE_SOUND_ENFORCE_RECOM_OPT)
    case 7102:
      if(wpar_hi==BN_CLICKED) {
        Sound_Stop();
        sound_write_primary=!sound_write_primary;
        SendMessage(HWND(lPar),BM_SETCHECK,sound_write_primary,true);
        if(runstate==RUNSTATE_RUNNING) {
          Sleep(50);
          Sound_Start();
        }
      }
      break;
    case 7103:
    {
      if(wpar_hi==CBN_SELENDOK) {
        int nstm=SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
        if(nstm!=sound_time_method) {
          Sound_Stop();
          sound_time_method=nstm;
          if(runstate==RUNSTATE_RUNNING) {
            Sleep(50);
            Sound_Start();
          }
        }
      }
      break;
    }
#endif
    case 7104:
      if(wpar_hi==CBN_SELENDOK)
        psg_write_n_screens_ahead=(int)SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
      break;
    case 7105:
      if(wpar_hi==CBN_SELENDOK) 
        OPTION_SOUND_RECORD_FORMAT=(BYTE)SendMessage(HWND(lPar),
          CB_GETCURSEL,0,0);
      break;
    case 7099: //SS sound output type
      if(wpar_hi==CBN_SELENDOK)
        psg_hl_filter=(BYTE)SendMessage(HWND(lPar),CB_GETCURSEL,0,0)+1;
      break;
    case IDC_SOUNDMUTE:
      if(wpar_hi==BN_CLICKED)
        This->SoundMute(!OPTION_SOUNDMUTE);
      break;
#if defined(SSE_OPTION_FASTBLITTER)
    case IDC_FASTBLITTER:
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_FASTBLITTER=!OPTION_FASTBLITTER;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_FASTBLITTER,0);
        ChangeTimingFunctions();
      }
      break;
#endif
    case 7201:
      if(wpar_hi==BN_CLICKED)
        This->SetRecord(!sound_record);
      break;
    case 7203:
      if(wpar_hi==BN_CLICKED)
      {
        SendMessage(HWND(lPar),BM_SETCHECK,1,true);
        EnableAllWindows(0,Win);
        char wildcard[6]="*.wav";
        if(OPTION_SOUND_RECORD_FORMAT)
          strcpy(wildcard,"*.ym");
        char filetype[6+5];
        sprintf(filetype,"%s File",wildcard+2);
        EasyStr NewWAV=FileSelect(HWND(FullScreen?StemWin:Win),
          T("Choose Sound Output File"),
          This->WAVOutputDir,FSTypes(1,filetype,wildcard,NULL),1,0,wildcard+2);
        if(NewWAV.NotEmpty()) 
        {
          WAVOutputFile=NewWAV;
          This->WAVOutputDir=NewWAV;
          RemoveFileNameFromPath(This->WAVOutputDir,REMOVE_SLASH);
          SendMessage(GetDlgItem(Win,7202),WM_SETTEXT,0,
            LPARAM(WAVOutputFile.Text));
        }
        SetForegroundWindow(Win);
        EnableAllWindows(true,Win);
        SendMessage(HWND(lPar),BM_SETCHECK,0,true);
        SetFocus(HWND(lPar));
      }
      break;
    case 7204:
      if(wpar_hi==BN_CLICKED) 
      {
        This->RecordWarnOverwrite=!This->RecordWarnOverwrite;
        SendMessage(HWND(lPar),BM_SETCHECK,This->RecordWarnOverwrite,0);
      }
      break;
#if !defined(SSE_NO_INTERNAL_SPEAKER)
    case 7300:
      if(wpar_hi==BN_CLICKED) {
        if(sound_internal_speaker) SoundStopInternalSpeaker();

        sound_internal_speaker=!sound_internal_speaker;
        SendMessage(HWND(lPar),BM_SETCHECK,sound_internal_speaker,0);
      }
      break;
#endif
#if defined(SSE_TOS_KEYBOARD_CLICK)
    case 7301: // Keyboard click on/off
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_KEYBOARD_CLICK=!OPTION_KEYBOARD_CLICK;
        TRACE_LOG("Option Keyboard click %d\n",OPTION_KEYBOARD_CLICK);
        Tos.CheckKeyboardClick();
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_KEYBOARD_CLICK,0);
      }
      break;
#endif
#if defined(SSE_GUI_OPTIONS_MICROWIRE)
    case 7302: // STE Microwire on/off 
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_MICROWIRE=!OPTION_MICROWIRE; 
        TRACE_LOG("Option Microwire %d\n",OPTION_MICROWIRE);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_MICROWIRE,0);
      }
      break;
#endif
#if defined(SSE_SOUND_MICROWIRE_12DB_HACK) 
    case 7345: // STE YM-12db
      if(wpar_hi==BN_CLICKED) 
      {
        OPTION_YM_12DB=!OPTION_YM_12DB;
        TRACE_LOG("Option STE YM-12db %d\n",OPTION_YM_12DB);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_YM_12DB,0);
        if(OPTION_SAMPLED_YM)
          Psg.LoadFixedVolTable(); // reload to adapt
      }
      break;
#endif
#if defined(SSE_YM2149_LL)
    case 7312: // option Low-level YM
      if(wpar_hi==BN_CLICKED) 
      {
#if defined(SSE_EMU_THREAD)
        if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING)
          SoundLock.Lock();
#endif
        //Sound_Stop();
        OPTION_MAME_YM=!OPTION_MAME_YM;
        OPTION_SAMPLED_YM=OPTION_MAME_YM;
        TRACE_LOG("Option Low-level YM %d\n",OPTION_MAME_YM);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_MAME_YM,0);
        if(OPTION_SAMPLED_YM)
          Psg.LoadFixedVolTable();
        else
          Psg.FreeFixedVolTable();
        This->DestroyCurrentPage();
        This->CreatePage(This->Page);
        //Sound_Start();
#if defined(SSE_EMU_THREAD)
        SoundLock.Unlock();
#endif
      }
      break;
#endif
#if defined(SSE_GUI_STATUS_BAR) && !defined(SSE_GUI_LEGACY_TOOLBAR)
    case 7307: // status bar
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_STATUS_BAR=!OPTION_STATUS_BAR;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_STATUS_BAR,0);
        TRACE_LOG("Option status bar %d\n",OPTION_STATUS_BAR);
        GUIRefreshStatusBar();
#if defined(SSE_GUI_TOOLBAR) // provoke redraw
        //SetMenu(StemWin,NULL);
        SetMenu(StemWin,StemWin_AltMenu);
#endif
      }
      break;
#endif
#ifdef SSE_GUI_STATUS_BAR_GAME_NAME
    case 7309: //  status bar game name
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_STATUS_BAR_GAME_NAME=!OPTION_STATUS_BAR_GAME_NAME;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_STATUS_BAR_GAME_NAME,0);
        TRACE_LOG("Option status bar game name %d\n",OPTION_STATUS_BAR_GAME_NAME);
      }
      break;
#endif
#if defined(SSE_DRIVE_SOUND)
    case 7310: //  option Drive Sound  - also see 7311 for volume slider
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_DRIVE_SOUND=!OPTION_DRIVE_SOUND;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_DRIVE_SOUND,0);
        TRACE_LOG("Option Drive Sound %d\n",OPTION_DRIVE_SOUND);
      }
      break;
#endif
#if defined(SSE_GUI_OPTION_FOR_TESTS)
    case 7316: // Option Beta Tests
      if(wpar_hi==BN_CLICKED)
      {
        SSEOptions.TestingNewFeatures=!SSEOptions.TestingNewFeatures;
        SendMessage(HWND(lPar),BM_SETCHECK,SSEOptions.TestingNewFeatures,0);
        TRACE_LOG("Option Beta Tests %d\n",SSEOptions.TestingNewFeatures);
      }
      break;
#endif
    case 7317: // Option No resize
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_BLOCK_RESIZE=!OPTION_BLOCK_RESIZE;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_BLOCK_RESIZE,0);
        EnableWindow(GetDlgItem(Win,7318),!OPTION_BLOCK_RESIZE);
        TRACE_LOG("Option No resize %d\n",OPTION_BLOCK_RESIZE);
      }
      break;
    case 7318: // Option Lock aspect ratio of the window
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_LOCK_ASPECT_RATIO=!OPTION_LOCK_ASPECT_RATIO;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_LOCK_ASPECT_RATIO,0);
        TRACE_LOG("Option Lock aspect ratio %d\n",OPTION_LOCK_ASPECT_RATIO);
      }
      break;
#if defined(SSE_VID_D3D)
    case 7319: // Option D3D mode
      if(wpar_hi==CBN_SELENDOK)
      {
        UINT old_mode=Disp.D3DMode;
        Disp.D3DMode=(UINT)SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
        Disp.D3DUpdateWH(Disp.D3DMode);
        TRACE_LOG("Option D3D mode = %d %dx%d\n",Disp.D3DMode,Disp.D3DFsW,Disp.D3DFsH);
        if(FullScreen && old_mode!=Disp.D3DMode)
        {
          Disp.ScreenChange();
#if !defined(SSE_VID_2SCREENS) // done in D3DCreateSurfaces()
          SetWindowPos(StemWin,HWND_TOPMOST,0,0,Disp.D3DFsW,Disp.D3DFsH,
            SWP_FRAMECHANGED);
          InvalidateRect(StemWin,NULL,FALSE);
#endif
        }
      }
      break;
#endif
#if defined(SSE_CPU_MFP_RATIO_OPTION) 
    // user can fine tune CPU clock, it counts only for MFP timers
    case 7322:
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_CPU_CLOCK=!OPTION_CPU_CLOCK;
        TRACE_LOG("Option CPU clock = %d\n",OPTION_CPU_CLOCK);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_CPU_CLOCK,0);
        CpuMfpRatio=(OPTION_CPU_CLOCK?(double)CpuCustomHz
          :(double)CpuNormalHz)/(double)MFP_XTAL;
      }
      break;
#endif
#if defined(SSE_INT_MFP_OPTION)
    case 7323: // Option mfp
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_68901=!OPTION_68901;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_68901,0);
      }
      break;
#endif
#if defined(SSE_VID_D3D) // Option Crisp D3D
    case 7324:
      if(HIWORD(wPar)==BN_CLICKED)
      {
        OPTION_CRISP_FULLSCREEN=!OPTION_CRISP_FULLSCREEN;
        TRACE_LOG("Option Crisp Fullscreen = %d\n",OPTION_CRISP_FULLSCREEN);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_CRISP_FULLSCREEN,0);
        if(FullScreen && D3D9_OK)
          Disp.D3DSpriteInit();
      }
      break;
#endif
    case 7326: // Software Overscan = Shifter tricks
    case 7327:
    case 7328:
      if(wpar_hi==BN_CLICKED)
      {
        if(SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED)
        {
#if defined(SSE_EMU_THREAD)
          if(OPTION_EMUTHREAD && runstate==RUNSTATE_RUNNING)
            VideoLock.Lock();
#endif
          OPTION_VLE=(BYTE)(wpar_lo-7326); // 0=none, 1=high-level, 2=low-level
          if(OPTION_C2 && (LINECYCLE0&1) && SSEConfig.Stvl) // when going C3->C2
            LINECYCLE0-=1; // high-level video emu wants even cycles
          ChangeTimingFunctions();
          Glue.Update();
#if defined(SSE_EMU_THREAD)
          VideoLock.Unlock();
#endif
        }
      }
      break;
    case IDC_SHIFTER_WU0:
      if(wpar_hi==EN_CHANGE)
      {
        char buf[12];
        SendMessage(HWND(lPar),WM_GETTEXT,3,(LPARAM)buf);
        OPTION_SHIFTER_WU=(char)atoi(buf);
        TRACE_LOG("Option Shifter WU = %d\n",OPTION_SHIFTER_WU);
        Glue.Update(); // which will also update the Shifter...
      }
      break;

    case 17350:
    case 17351:
    case 17352:
      if(wpar_hi==BN_CLICKED
        && SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED)
      {
        OPTION_FULLSCREEN_AR=(BYTE)(wpar_lo-17350);
        TRACE_LOG("OPTION_FULLSCREEN_AR = %d\n",OPTION_FULLSCREEN_AR);
#if defined(SSE_VID_D3D)
        if(FullScreen && D3D9_OK)
          Disp.D3DSpriteInit();
#endif
      }
      break;

    case 17360:
    case 17361:
    case 17362:
    case 17363:
      if(wpar_hi==BN_CLICKED
        && SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED)
      {
        This->SetBorder(wpar_lo-17360);
        if(draw_grille_black<4) 
          draw_grille_black=4;
      }
      break;
    case IDC_RADIO_CAPTURE_MOUSE:
    case (IDC_RADIO_CAPTURE_MOUSE+1):
    case (IDC_RADIO_CAPTURE_MOUSE+2):
      if(wpar_hi==BN_CLICKED
        && SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED)
      {
        OPTION_CAPTURE_MOUSE=(BYTE)(wpar_lo-IDC_RADIO_CAPTURE_MOUSE);
        TRACE_LOG("OPTION_CAPTURE_MOUSE %d\n",OPTION_CAPTURE_MOUSE);
      }
      break;
    case 7325:
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_FULLSCREEN_GUI=!OPTION_FULLSCREEN_GUI;
        TRACE_LOG("Option FullScreen GUI = %d\n",OPTION_FULLSCREEN_GUI);
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_FULLSCREEN_GUI,0);
        EnableWindow(GetDlgItem(Win,226),OPTION_FULLSCREEN_GUI); // class A interface!
      }
      break;
#if defined(SSE_IKBD_RTC)
    case 7331:
    case 7332:
    case 7333:
      if(wpar_hi==BN_CLICKED)
      {
        if(SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED)
          OPTION_BATTERY6301=(BYTE)(wpar_lo-7331);
      }
      break;
    case 7334:
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_RTC_HACK=!OPTION_RTC_HACK;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_RTC_HACK,0);
        TRACE_LOG("Option RTC hack = %d\n",OPTION_RTC_HACK);
      }
      break;
#endif
#if defined(SSE_HARDWARE_OVERSCAN)
    case IDC_RADIO_HWOVERSCAN:
    case (IDC_RADIO_HWOVERSCAN+1):
    case (IDC_RADIO_HWOVERSCAN+2):
      if(wpar_hi==BN_CLICKED)
      {
        if(SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED)
          OPTION_HWOVERSCAN=(BYTE)(wpar_lo-IDC_RADIO_HWOVERSCAN);
      }
      break;
#endif
    case IDC_RADIO_STSCREEN://colour
    case (IDC_RADIO_STSCREEN+1): //mono
      if(wpar_hi==BN_CLICKED)
      {
        if(SendMessage(HWND(lPar),BM_GETCHECK,0,0)==BST_CHECKED)
        {
          This->NewMonitorSel=wpar_lo-IDC_RADIO_STSCREEN;
          extended_monitor=0;
          This->UpdateSTVideoPage();
          // can change live, funny effects (in general crash)
          switch(This->NewMonitorSel) {
          case 0:
            screen_res=0;
            init_screen();
            mfp_gpip_no_interrupt|=MFP_GPIP_COLOUR;
            SSEConfig.ColourMonitor=true;
            mfp_gpip_set_bit(MFP_GPIP_MONO_BIT,1);
            EnableWindow(GetDlgItem((HWND)lPar,17362),TRUE);
            break;
          case 1:
            screen_res=2;
            init_screen();
            mfp_gpip_no_interrupt&=MFP_GPIP_NOT_COLOUR;
            SSEConfig.ColourMonitor=false;
            mfp_gpip_set_bit(MFP_GPIP_MONO_BIT,0);
            break;
          }//sw
          This->UpdateSTVideoPage();
        }
      }
      break;
    case 7343://extended
      //extended_monitor=1;
      break;
#if defined(SSE_GUI_DEFAULT_ST_CONFIG)
    case 7344: // ST Preselect
      if(wpar_hi==BN_CLICKED)
      {
        OPTION_ST_PRESELECT=!OPTION_ST_PRESELECT;
        SendMessage(HWND(lPar),BM_SETCHECK,OPTION_ST_PRESELECT,0);
        TRACE_LOG("Option ST Preselect = %d\n",OPTION_ST_PRESELECT);
      }
      break;
#endif
    case IDC_MEMORY_SIZE: // Memory size
      if(wpar_hi==CBN_SELENDOK)
      {
        DWORD Conf=(DWORD)CBGetSelectedItemData(HWND(lPar));
        This->NewMemConf0=LOWORD(Conf);This->NewMemConf1=HIWORD(Conf);
        if(SSEConfig.bank_length[0]==mmu_bank_length_from_config[This->NewMemConf0]&&
          SSEConfig.bank_length[1]==mmu_bank_length_from_config[This->NewMemConf1])
        {
          This->NewMemConf0=-1;
        }
        CheckResetIcon();
      }
      break;
    case IDC_EXTENDED_MONITOR:
      if(wpar_hi==CBN_SELENDOK)
      {
        // if Extended button checked, get selection
        if(SendMessage(GetDlgItem(This->Handle,(IDC_RADIO_STSCREEN+2)),
          BM_GETCHECK,0,0)==BST_CHECKED)
          This->NewMonitorSel=(int)SendMessage(HWND(lPar),CB_GETCURSEL,0,0)+2;
        if(This->NewMonitorSel==This->GetCurrentMonitorSel()) 
          This->NewMonitorSel=-1;
        CheckResetIcon();
      }
      break;
    case 8501: // Choose cart
      if(wpar_hi==BN_CLICKED)
      {
        SendMessage(HWND(lPar),BM_SETCHECK,1,true);
        EnableAllWindows(0,Win);
        Str LastCartFol=This->LastCartFile;
        RemoveFileNameFromPath(LastCartFol,REMOVE_SLASH);
        EasyStr NewCart=FileSelect(HWND(FullScreen?StemWin:Win),
          T("Find a Cartridge"),
          LastCartFol,FSTypes(0,T("ST Cartridge Images").Text,
            "*.stc",NULL),1,true,"stc",
          GetFileNameFromPath(This->LastCartFile));
        if(NewCart.NotEmpty())
        {
          SetWindowText(GetDlgItem(Win,8500),NewCart);
          EnableWindow(GetDlgItem(Win,8502),true);
          EnableWindow(GetDlgItem(Win,8503),true);
          EnableWindow(GetDlgItem(Win,8504),true);
          This->LastCartFile=NewCart;
          if(load_cart(NewCart))
            Alert(T("There was an error loading the cartridge."),
              T("Cannot Load Cartridge"),MB_ICONEXCLAMATION);
          else
            CartFile=NewCart;
          CheckResetDisplay();
        }
        SetForegroundWindow(Win);
        EnableAllWindows(true,Win);
        SetFocus(HWND(lPar));
        SendMessage(HWND(lPar),BM_SETCHECK,0,true);
      }
      break;
    case 8502: // Remove cart
      if(wpar_hi==BN_CLICKED)
      {
        SetWindowText(GetDlgItem(Win,8500),"");
        SetFocus(GetDlgItem(Win,8501));
        EnableWindow(GetDlgItem(Win,wpar_lo),0);
        EnableWindow(GetDlgItem(Win,wpar_lo+1),0);
        EnableWindow(GetDlgItem(Win,wpar_lo+2),0); //8504
        if(cart_save)
          cart=cart_save;
        cart_save=NULL;
        delete[] cart; cart=NULL;
#if defined(SSE_SOUND_CARTRIDGE)
        SSEConfig.mv16=SSEConfig.mr16=false;
#endif
        CartFile="";
        CheckResetDisplay();
      }
      break;
#if defined(SSE_DONGLE)
/*  The Multiface ST cartridge features a 'freeze' button that messes with the
    monochrome monitor detection interrupt. That's the purpose of the cable
    that intercepts the monitor connection.
    The bit is cleared only as long as the player is pressing the button.
    Same idea on the Ultimate Ripper cartridge, but the button hits a line of
    the serial port instead.
    The switches are considered as dongles to simplify the GUI.
*/
    case 8503:
      switch(DONGLE_ID) {
#if defined(SSE_DONGLE_URC)
      case TDongle::URC:
        mfp_gpip_set_bit(MFP_GPIP_RING_BIT,false); // Ultimate Ripper
        break;
#endif
#if defined(SSE_DONGLE_MULTIFACE)
      case TDongle::MULTIFACE:
        mfp_gpip_set_bit(MFP_GPIP_MONO_BIT,false); // Multiface
        break;
#endif
      }//sw
      break;
#endif
    case 8504:
      if(SSEOptions.CartidgeOff)
      {
        if(cart_save)
          cart=cart_save;
        cart_save=NULL;
      }
      else
      {
        if(cart)
          cart_save=cart;
        cart=NULL;
      }
      SSEOptions.CartidgeOff=!SSEOptions.CartidgeOff;
      OptionBox.MachineUpdateIfVisible();
      break;
    case 8601: // Cold reset
      if(wpar_hi==BN_CLICKED)
        reset_st(RESET_COLD|RESET_STOP|RESET_CHANGESETTINGS|RESET_BACKUP);
      break;
    case 8401: // Keyboard language
      if(wpar_hi==CBN_SELENDOK)
      {
#if defined(SSE_IKBD_MAPPINGFILE)
        int ix=KeyboardLangID=(LANGID)SendMessage(HWND(lPar),CB_GETITEMDATA,
          SendMessage(HWND(lPar),CB_GETCURSEL,0,0),0);
        if(ix==LANG_CUSTOM)
        {
          EasyStr path=FileSelect(NULL,T("Select Keyboard Mapping"),KeyboardMappingPath,
            FSTypes(0,T("Configuration files").Text,"*." CONFIG_FILE_EXT,NULL),1,true,
            "ini",KeyboardMappingPath);
          if(path.IsNotEmpty())
          {
            KeyboardMappingPath=path;
            KeyboardLangID=(LANGID)ix;
          }
        }
        else
          KeyboardLangID=(LANGID)ix;
#else
        KeyboardLangID=(LANGID)SendMessage(HWND(lPar),CB_GETITEMDATA,
          SendMessage(HWND(lPar),CB_GETCURSEL,0,0),0);
#endif
        InitKeyTable();
        EnableWindow(GetDlgItem(Win,8402),ShiftSwitchingAvailable);
      }
      break;
    case 8402: // Keyboard alt and shift correction
      if(wpar_hi==BN_CLICKED)
      {
        EnableShiftSwitching=(SendMessage(HWND(lPar),BM_GETCHECK,0,0)!=0);
        InitKeyTable();
      }
      break;
    case 8300: // TOS
      if(wpar_hi==LBN_SELCHANGE)
      {
        int i=(int)SendMessage(HWND(lPar),LB_GETCURSEL,0,0);
        if(i!=LB_ERR)
        {
          int idx=i;
          if(This->eslTOS_Descend) 
            idx=This->eslTOS.NumStrings-1-i;
          This->NewROMFile=strchr(This->eslTOS[idx].String,'\01')+1;
          if(IsSameStr_I(ROMFile,This->NewROMFile)) 
            This->NewROMFile="";
        }
        CheckResetIcon();
      }
      break;
    case 8301:
      if(wpar_hi==BN_CLICKED)
      {
        SendMessage(HWND(lPar),BM_SETCHECK,1,true);
        EnableAllWindows(0,Win);
        OPENFILENAME ofn;
        ZeroMemory(&ofn,sizeof(OPENFILENAME));
        char *files=new char[MAX_PATH*80+1];
        ZeroMemory(files,MAX_PATH*80+1);
        ofn.lStructSize=sizeof(OPENFILENAME);
        ofn.hwndOwner=HWND(FullScreen?StemWin:Win);
        ofn.hInstance=(HINSTANCE)GetModuleHandle(NULL);
        ofn.lpstrFilter=FSTypes(3,NULL);
        ofn.lpstrCustomFilter=NULL;
        ofn.nMaxCustFilter=0;
        ofn.nFilterIndex=1;
        ofn.lpstrFile=files;
        ofn.nMaxFile=MAX_PATH*80;
        ofn.lpstrFileTitle=NULL;
        ofn.nMaxFileTitle=0;
        ofn.lpstrInitialDir=This->TOSBrowseDir;
        ofn.lpstrTitle=StaticT("Select One or More TOS Images");
        ofn.Flags=OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_FILEMUSTEXIST
          |OFN_PATHMUSTEXIST|OFN_ALLOWMULTISELECT|OFN_EXPLORER;
        ofn.lpstrDefExt="IMG";
        ofn.lpfnHook=0;
        ofn.lpTemplateName=0;
        if(GetOpenFileName(&ofn))
        {
          int nfiles=0;
          char *f=files;
          while(f[0])
          {
            f+=strlen(f)+1;
            nfiles++;
          }
          if(nfiles==1)
          {
            This->TOSBrowseDir=files;
            RemoveFileNameFromPath(This->TOSBrowseDir,REMOVE_SLASH);
            files+=strlen(This->TOSBrowseDir)+1; // only want name
          }
          else
          {
            This->TOSBrowseDir=files;
            files+=strlen(files)+1; // skip to files
          }
          HWND TOSBox=GetDlgItem(Win,8300);
          Str file;
          char *cur_file=files;
          while(cur_file[0])
          {
            file=This->TOSBrowseDir+SLASH+cur_file;
            int c=(int)SendMessage(TOSBox,LB_GETCOUNT,0,0);
            for(int i=0;i<c;i++)
            {
              if(IsSameStr_I(file,strrchr(This->eslTOS[i].String,'\01')+1))
              {
                if(nfiles==1)
                {
                  SendMessage(TOSBox,LB_SETCARETINDEX,i,0);
                  SendMessage(TOSBox,LB_SETCURSEL,i,0);
                }
                file=""; // skip this file
                break;
              }
            }
            if(file[0])
            {
              WIN32_FIND_DATA wfd;
              HANDLE hFind=FindFirstFile(file,&wfd);
              if(hFind!=INVALID_HANDLE_VALUE)
              {
                FindClose(hFind);
                if(get_TOS_address(file))
                {
                  int n=2;
                  EasyStr LinkFileName=WriteDir+SLASH+GetFileNameFromPath(file)
                    +".lnk";
                  while(Exists(LinkFileName))
                    LinkFileName=WriteDir+SLASH+GetFileNameFromPath(file)
                      +" ("+(n++)+").lnk";
                  CreateLink(LinkFileName,file,T("TOS Image"));
                  if(nfiles==1) 
                    This->NewROMFile=file;
                }
                else if(nfiles==1)
                  Alert(Str(file)+" "+T("is not a valid TOS image."),
                    T("Cannot use TOS"),MB_ICONEXCLAMATION);
              }
            }
            cur_file+=strlen(cur_file)+1;
          }
          This->TOSRefreshBox();
        }
        SetForegroundWindow(Win);
        EnableAllWindows(true,Win);
        SendMessage(HWND(lPar),BM_SETCHECK,0,true);
        SetFocus(HWND(lPar));
        CheckResetIcon();
      }
      break;
    case 8302:
      if(wpar_hi==BN_CLICKED)
      {
        SendMessage(HWND(lPar),BM_SETCHECK,1,true);
        HWND TOSBox=GetDlgItem(Win,8300);
        int idx=(int)SendMessage(TOSBox,LB_GETCURSEL,0,0);
        if(This->eslTOS_Descend) 
          idx=This->eslTOS.NumStrings-1-idx;
        char *RemovePath=strrchr(This->eslTOS[idx].String,'\01')+1;
        int idx1=idx+int(This->eslTOS_Descend?1:-1);
        if(idx1<0) 
          idx1=1;
        if(idx1>=This->eslTOS.NumStrings) 
          idx1=This->eslTOS.NumStrings-2;
        Str NewSel=strrchr(This->eslTOS[idx1].String,'\01')+1;
        DirSearch ds;
        EasyStringList dellist;
        if(ds.Find(RunDir+SLASH+"*.*"))
        {
          Str Path,RealPath;
          do
          {
            if((ds.Attrib & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN))==0)
            {
              Path=RunDir+SLASH+ds.Name;
              if(has_extension(Path,"LNK"))
              {
                WIN32_FIND_DATA wfd;
                RealPath=GetLinkDest(Path,&wfd);
              }
              else
                RealPath=Path;
              if(IsSameStr_I(RealPath,RemovePath)) 
                dellist.Add(Path,IsSameStr_I(RealPath,Path));
            }
          } while(ds.Next());
          ds.Close();
        }
        for(int i=0;i<dellist.NumStrings;i++)
        {
          if(dellist[i].Data[0])
            // Not link
            SetFileAttributes(dellist[i].String,
              GetFileAttributes(dellist[i].String)|FILE_ATTRIBUTE_HIDDEN);
          else
            DeleteFile(dellist[i].String);
        }
        This->NewROMFile=NewSel;
        This->TOSRefreshBox();
        SendMessage(HWND(lPar),BM_SETCHECK,0,true);
      }
      break;
    case 8311:
      if(wpar_hi==CBN_SELENDOK)
      {
        This->eslTOS_Sort=(ESLSortEnum)(signed short)
          LOWORD(CBGetSelectedItemData(HWND(lPar)));
        This->eslTOS_Descend=(HIWORD(CBGetSelectedItemData(HWND(lPar)))!=0);
        This->TOSRefreshBox("");
      }
      break;
    case 10001:
      if(wpar_hi!=BN_CLICKED) 
        break;
      DTree.NewItem(T("New Macro"),DTree.RootItem,1);
      break;
    case 10002:case 11002:
      if(wpar_hi==BN_CLICKED)
      {
        SendMessage(HWND(lPar),BM_SETCHECK,1,true);
        EnableAllWindows(0,Win);
        Str *Dir=(Str*)((wpar_lo==10002)?&(This->MacroDir):&(This->ProfileDir));
        EasyStr NewFol=ChooseFolder(HWND(FullScreen?StemWin:Win),
          T("Pick a Folder"),Dir->Text);
        if(NewFol.NotEmpty())
        {
          NO_SLASH(NewFol);
          *Dir=NewFol;
          DTree.RootFol=NewFol;
          DTree.RefreshDirectory();
        }
        SetForegroundWindow(Win);
        EnableAllWindows(true,Win);
        SetFocus(HWND(lPar));
        SendMessage(HWND(lPar),BM_SETCHECK,0,true);
      }
      break;
    case 10011:
      if(wpar_hi!=BN_CLICKED) 
        break;
      if(macro_record==0)
      {
        macro_record_file=This->MacroSel;
        macro_advance(MACRO_STARTRECORD);
      }
      else
        macro_end(MACRO_ENDRECORD);
      break;
    case 10012:
      if(wpar_hi!=BN_CLICKED)
        break;
      if(macro_play==0)
      {
        macro_play_file=This->MacroSel;
        macro_advance(MACRO_STARTPLAY);
      }
      else
        macro_end(MACRO_ENDPLAY);
      break;
    case 10016:case 10017:case 10014:
    {
      TMacroFileOptions MFO;
      macro_file_options(MACRO_FILE_GET,This->MacroSel,&MFO);
      switch(wpar_lo) {
      case 10016:
        if(wpar_hi==CBN_SELENDOK) 
          MFO.allow_same_vbls=(int)CBGetSelectedItemData(HWND(lPar));
        break;
      case 10014:
        if(wpar_hi==CBN_SELENDOK) 
          MFO.max_mouse_speed=(int)CBGetSelectedItemData(HWND(lPar));
        break;
      }//sw
      macro_file_options(MACRO_FILE_SET,This->MacroSel,&MFO);
      break;
    }
    case 11001:
    case 11012:
      if(wpar_hi==BN_CLICKED)
      {
        Str Path;
        if(wpar_lo==11012)
        {
          Path=This->ProfileSel;
          if(IDNO==Alert(
            //T("Are you sure?"),T("Overwrite File"),
            T("Overwrite file?"),GetFileNameFromPath(Path),
            MB_ICONQUESTION|MB_YESNO)) 
            break;
          //Path=This->ProfileSel;
        }
        else
        {
          HTREEITEM Item=DTree.NewItem(T("New Profile"),DTree.RootItem,1);
          if(Item==NULL) 
            break;
          Path=DTree.GetItemPath(Item);
        }
        SaveAllDialogData(0,Path);
      }
      break;
    case 11011:
      if(wpar_hi==BN_CLICKED) 
        This->LoadProfile(This->ProfileSel);
      break;
    case 14020:
      if(wpar_hi==BN_CLICKED)
      {
        SendMessage(HWND(lPar),BM_SETCHECK,1,true);
        EnableAllWindows(0,Win);
        Str LastIconSchemeFol=This->LastIconSchemePath;
        RemoveFileNameFromPath(LastIconSchemeFol,REMOVE_SLASH);
        Str NewFile=FileSelect(HWND(FullScreen?StemWin:Win),T("Load Icon"),
          LastIconSchemeFol,FSTypes(1,T("Steem Icon Scheme").Text,"*.stico",
            NULL),1,true,"stico",GetFileNameFromPath(This->LastIconSchemePath));
        if(NewFile.NotEmpty())
        {
          This->LastIconSchemePath=NewFile;
          TConfigStoreFile SchemeCSF(NewFile);
          TConfigStoreFile CSF(globalINIFile);
          Str TransSect=T("Patch Text Section=");
          if(TransSect=="Patch Text Section=") 
            TransSect="Text";
          Str Desc;
          for(int i=0;i<2;i++)
          {
            Desc=SchemeCSF.GetStr(TransSect,"Description","");
            if(Desc.NotEmpty()) 
              break;
            TransSect="Text";
          }
          if(Desc.NotEmpty()) Desc+="\r\n\r\n";
          Desc+=T("Are you sure you want to load this icon scheme?");
          if(Alert(Desc,T("Load Icon Scheme?"),MB_ICONQUESTION|MB_YESNO)==IDYES)
          {
            Str File;
            EasyStringList Fols(eslNoSort);
            Str Fol=This->LastIconSchemePath;
            RemoveFileNameFromPath(Fol,REMOVE_SLASH);
            CSF.SetInt("Icons","UseDefaultIn256",
              SchemeCSF.GetInt("Options","UseDefaultIn256",0));
            int icn=1;
            for(;;)
            {
              File=SchemeCSF.GetStr("Options",Str("SearchFolder")+(icn++),"");
              if(File.Empty()) break;
              Fols.Add(Fol+SLASH+File);
            }
            Fols.Add(Fol);
            for(int n=1;n<RC_NUM_ICONS;n++)
            {
              File=SchemeCSF.GetStr("Icons",Str("Icon")+n,".");
              if(File!=".")
              {
                Fol="";
                for(int i=0;i<Fols.NumStrings;i++)
                {
                  if(Exists(Str(Fols[i].String)+SLASH+File))
                  {
                    Fol=Fols[i].String;
                    break;
                  }
                }
                if(Fol.NotEmpty()) 
                  CSF.SetStr("Icons",Str("Icon")+n,Fol+SLASH+File);
              }
            }
            LoadAllIcons(&CSF,0);
          }
          SchemeCSF.Close();
          CSF.Close();
        }
        SetForegroundWindow(Win);
        EnableAllWindows(true,Win);
        SendMessage(HWND(lPar),BM_SETCHECK,0,true);
      }
      break;
    case 14021:
      if(wpar_hi==BN_CLICKED)
      {
        TConfigStoreFile CSF(globalINIFile);
        CSF.SetInt("Icons","UseDefaultIn256",0);
        for(int n=1;n<RC_NUM_ICONS;n++) 
          CSF.SetStr("Icons",Str("Icon")+n,"");
        LoadAllIcons(&CSF,0);
        CSF.Close();
      }
      break;
    }//sw(wpar_lo) - we're still handling WM_COMMAND
    REFRESH_STATUS_BAR; // overkill
    if(wpar_lo>=14100&&wpar_lo<14100+RC_NUM_ICONS)
    { // Icons
      if(wpar_hi==BN_CLICKED)
      {
        bool AllowDefault=true;
        Str NewFile;
        SendMessage(HWND(lPar),BM_SETCHECK,1,true);
        if(SendMessage(HWND(lPar),BM_GETCLICKBUTTON,0,0)==1)
        {
          AllowDefault=0;
          EnableAllWindows(0,Win);
          Str LastIconFol=This->LastIconPath;
          RemoveFileNameFromPath(LastIconFol,REMOVE_SLASH);
          NewFile=FileSelect(HWND(FullScreen?StemWin:Win),T("Load Icon"),
            LastIconFol,FSTypes(1,T("Icon File").Text,"*.ico",NULL),1,true,
            "ico",GetFileNameFromPath(This->LastIconPath));
          if(NewFile.NotEmpty()) 
            This->LastIconPath=NewFile;
          SetForegroundWindow(Win);
          EnableAllWindows(true,Win);
        }
        if(NewFile.NotEmpty()||AllowDefault)
        {
          TConfigStoreFile CSF(globalINIFile);
          CSF.SetStr("Icons",Str("Icon")+(wpar_lo-14100),NewFile);
          LoadAllIcons(&CSF,0);
          CSF.Close();
        }
        SendMessage(HWND(lPar),BM_SETCHECK,0,true);
      }

    }
    else if(wpar_lo>=9000&&wpar_lo<9300
#if defined(SSE_DONGLE_PORT)
      +100
#endif
      )
    { // Ports
      Str ErrorText,ErrorTitle;
      BYTE Port=(BYTE)((wpar_lo-9000)/100);
      ASSERT(Port<=3);
      int Control=(wpar_lo%100);
#if defined(SSE_DONGLE_PORT)
      if(Port==3)
      {
        STPort[Port].Type=(int)SendMessage(HWND(lPar),CB_GETITEMDATA,
          SendMessage(HWND(lPar),CB_GETCURSEL,0,0),0);
      }
      else
#endif
      switch(Control) {
      case 2:
        if(wpar_hi==CBN_SELENDOK)
        {
          STPort[Port].Type=(int)SendMessage(HWND(lPar),CB_GETITEMDATA,
            SendMessage(HWND(lPar),CB_GETCURSEL,0,0),0);
          This->PortsMakeTypeVisible(Port);
          STPort[Port].Create(Port,ErrorText,ErrorTitle,true);
        }
        break;
      case 11:
        if(wpar_hi==CBN_SELENDOK)
        {
          int NewDevice=(int)SendMessage(HWND(lPar),CB_GETCURSEL,0,0)-2;
          if(NewDevice!=STPort[Port].GetMIDIOutDeviceID())
          {
            STPort[Port].MIDIOutDevice=NewDevice;
            STPort[Port].Create(Port,ErrorText,ErrorTitle,true);
          }
        }
        break;
      case 13:
        if(wpar_hi==CBN_SELENDOK)
        {
          int NewDevice=(int)SendMessage(HWND(lPar),CB_GETCURSEL,0,0)-1;
          if(NewDevice!=STPort[Port].GetMIDIInDeviceID())
          {
            STPort[Port].MIDIInDevice=NewDevice;
            STPort[Port].Create(Port,ErrorText,ErrorTitle,true);
          }
        }
        break;
      case 21:
        if(wpar_hi==CBN_SELENDOK)
        {
          STPort[Port].LPTNum=(int)SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
          STPort[Port].Create(Port,ErrorText,ErrorTitle,true);
        }
        break;
      case 31:
        if(wpar_hi==CBN_SELENDOK)
        {
          STPort[Port].COMNum=(int)SendMessage(HWND(lPar),CB_GETCURSEL,0,0);
          STPort[Port].Create(Port,ErrorText,ErrorTitle,true);
        }
        break;
      case 41:
        if(wpar_hi==BN_CLICKED)
        {
          SendMessage(HWND(lPar),BM_SETCHECK,BST_CHECKED,0);
          EasyStr CurFol=STPort[Port].File;
          EnableAllWindows(0,Win);
          char *CurName=GetFileNameFromPath(CurFol);
          if(CurName>CurFol.Text) 
            *(CurName-1)=0;
          EasyStr FileName=FileSelect(HWND(FullScreen?StemWin:Win),
            T("Select Output File"),CurFol,FSTypes(1,NULL),1,2,
            Port==1?"txt":"bin",CurName);
          if(FileName.NotEmpty())
          {
            STPort[Port].File=FileName;
            SendDlgItemMessage(GetDlgItem(Win,9000+Port*100),9000+Port*100+40,
              WM_SETTEXT,0,LPARAM(FileName.Text));
            STPort[Port].Create(Port,ErrorText,ErrorTitle,true);
          }
          SetForegroundWindow(Win);
          EnableAllWindows(true,Win);
          SetFocus(HWND(lPar));
          SendMessage(HWND(lPar),BM_SETCHECK,0,0);
        }
        break;
      case 42:
        if(wpar_hi==BN_CLICKED)
        {
          SendMessage(HWND(lPar),BM_SETCHECK,BST_CHECKED,0);
          Ret=Alert(T("Are you sure? This will permanently delete the\
 contents of the file."),T("Delete Contents?"),MB_ICONQUESTION|MB_YESNO);
          if(Ret==IDYES)
          {
            STPort[Port].Close();
            DeleteFile(STPort[Port].File);
            STPort[Port].Create(Port,ErrorText,ErrorTitle,true);
          }
          SendMessage(HWND(lPar),BM_SETCHECK,0,0);
        }
        break;
      }
    }
    else if(wpar_lo>=12000&&wpar_lo<12100)
    {
      int i=wpar_lo-12000;
      if(i==0)
      {
        if(wpar_hi==BN_CLICKED)
        {
          osd_show_disk_light=!osd_show_disk_light;
          SendMessage(HWND(lPar),BM_SETCHECK,osd_show_disk_light,0);
        }
      }
      else if(i==1)
      {
        if(wpar_hi==BN_CLICKED)
        {
          OPTION_DRIVE_INFO=!OPTION_DRIVE_INFO;
          SendMessage(HWND(lPar),BM_SETCHECK,OPTION_DRIVE_INFO,0);
        }
      }
      else if(i>=10&&i<20)
      {
        if(wpar_hi==CBN_SELENDOK)
        {
          BYTE *p_element[4]={&osd_show_plasma,&osd_show_speed,&osd_show_icons,
            &osd_show_cpu};
          *(p_element[i-10])=(BYTE)CBGetSelectedItemData(HWND(lPar));
          //TRACE("i %d osd_show_plasma %d osd_show_speed %d osd_show_icons %d osd_show_cpu %d\n",osd_show_plasma,osd_show_speed,osd_show_icons,osd_show_cpu);
          osd_init_run(0);
        }
      }
      else if(i==20)
      {
        if(wpar_hi==BN_CLICKED)
        {
          osd_show_scrollers=!osd_show_scrollers;
          SendMessage(HWND(lPar),BM_SETCHECK,osd_show_scrollers,0);
        }
      }
      else if(i==30)
        if(wpar_hi==BN_CLICKED) 
          This->ChangeOSDDisable(!osd_disable);
      else if(i==50)
      {
        if(wpar_hi==BN_CLICKED)
        {
          osd_old_pos=!osd_old_pos;
          SendMessage(HWND(lPar),BM_SETCHECK,osd_old_pos,0);
          osd_init_run(0);
        }
      }
    }
    break;
  case WM_NOTIFY:
  {
    NMHDR *pnmh=(NMHDR*)lPar;
    if(wPar==60000)
    {
      GET_THIS;
      switch(pnmh->code) {
      case TVN_SELCHANGING:
      {
        NM_TREEVIEW *Inf=(NM_TREEVIEW*)lPar;
        if(Inf->action==4096)
        { //DODGY!!!!!! Undocumented!!!!!
          return true;
        }
        return 0;
      }
      case TVN_SELCHANGED:
      {
        NM_TREEVIEW *Inf=(NM_TREEVIEW*)lPar;
        if(Inf->itemNew.hItem)
        {
          TV_ITEM tvi;
          tvi.mask=TVIF_PARAM;
          tvi.hItem=(HTREEITEM)Inf->itemNew.hItem;
          SendMessage(This->PageTree,TVM_GETITEM,0,(LPARAM)&tvi);
          This->DestroyCurrentPage();
          This->Page=(int)tvi.lParam;
          This->CreatePage(This->Page);
        }
        break;
      }
      }//sw
    }
    else if(wPar==11013)
    {
      if(IsWindowEnabled(pnmh->hwndFrom)==0) 
        break;
      GET_THIS;
      if(pnmh->code==LVN_ITEMCHANGED)
      {
        NM_LISTVIEW *pLV=(NM_LISTVIEW*)lPar;
        if(pLV->uChanged & LVIF_STATE)
        {
          LV_ITEM lvi;
          lvi.iItem=pLV->iItem;
          lvi.iSubItem=0;
          lvi.mask=LVIF_PARAM|LVIF_STATE;
          lvi.stateMask=LVIS_STATEIMAGEMASK;
          SendMessage(pnmh->hwndFrom,LVM_GETITEM,0,(LPARAM)&lvi);
          WriteCSFInt("ProfileSections",ProfileSection[pLV->iItem].Name,
            lvi.state,This->ProfileSel);
        }
      }
    }
    break;
  }
  case WM_HSCROLL:
  {
    int ID=GetDlgCtrlID(HWND(lPar));
    switch(ID) {
    case 2001:
    case 2003:
    case 2005:
    case 2007:
    case 2009:
    {
      char tmp[30]; // also avoid memory allocation
      Brightness=(short)SendDlgItemMessage(Win,2001,TBM_GETPOS,0,0)-128;
      sprintf(tmp,"Brightness:%d",Brightness);
      SendDlgItemMessage(Win,2000,WM_SETTEXT,0,LPARAM(tmp));
      Contrast=(short)SendDlgItemMessage(Win,2003,TBM_GETPOS,0,0)-128;
      sprintf(tmp,"Contrast:%d",Contrast);
      SendDlgItemMessage(Win,2002,WM_SETTEXT,0,LPARAM(tmp));
      for(int i=0;i<3;i++)
      {
        col_gamma[i]=(short)SendDlgItemMessage(Win,2005+i*2,TBM_GETPOS,0,0)-128;
        sprintf(tmp,"Gamma %s:%d",rgb_txt[i],col_gamma[i]);
        SendDlgItemMessage(Win,2004+i*2,WM_SETTEXT,0,LPARAM(tmp));
      }
    }
      make_palette_table(Brightness,Contrast);
      if(!flashlight_flag) 
        palette_convert_all();
      GET_THIS;
      This->DrawBrightnessBitmap(This->hBrightBmp);
      InvalidateRect(GetDlgItem(Win,ID_BRIGHTNESS_MAP),NULL,true);
      UpdateWindow(GetDlgItem(Win,ID_BRIGHTNESS_MAP));
      break;
    case 1001:
      slow_motion_speed=(int)(SendDlgItemMessage(Win,1001,TBM_GETPOS,0,0)+1)*10;
      SendDlgItemMessage(Win,1000,WM_SETTEXT,0,LPARAM((T("Slow motion speed")
        +": "+(slow_motion_speed/10)+"%").Text));
      break;
    case 1000:
      mouse_speed=(BYTE)SendMessage(HWND(lPar),TBM_GETPOS,0,0);
      break;
#if defined(SSE_YM2149_LL)
    case 1002:
    {
      SSEOptions.low_pass_frequency=(WORD)SendMessage(HWND(lPar),TBM_GETPOS,0,0);
      char tmp[40];
      sprintf(tmp,"%dHz",SSEOptions.low_pass_frequency);
      SendDlgItemMessage(Win,995,WM_SETTEXT,0,LPARAM(tmp));
#if defined(SSE_EMU_THREAD)
      if(!DSP_DISABLED && runstate==RUNSTATE_RUNNING) // change live!
      {
        if(OPTION_EMUTHREAD)
          SoundLock.Lock();
        if(Psg.AntiAlias)
          delete Psg.AntiAlias;
        Psg.AntiAlias=NULL;
        if(SSEOptions.low_pass_frequency<ym_low_pass_max)
          Psg.AntiAlias=new Filter(LPF,51,250.0,(double)SSEOptions.
            low_pass_frequency/1000);
        SoundLock.Unlock();
      }
#endif
      break;
    }
#endif
    case 1011:
    {
      fast_forward_max_speed=1000/(int)(SendDlgItemMessage(Win,
        1011,TBM_GETPOS,0,0)+2);
      Str Text=T("Maximum fast forward speed")+": ";
      if(fast_forward_max_speed>50)
        Text+=Str((1000/fast_forward_max_speed)*100)+"%";
      else
      {
        Text+=T("Unlimited");
        fast_forward_max_speed=0;
      }
      SendDlgItemMessage(Win,1010,WM_SETTEXT,0,LPARAM(Text.Text));
      break;
    }
    case 1041:
    {
      run_speed_ticks_per_second=100000/(50+(int)SendDlgItemMessage(Win,
        1041,TBM_GETPOS,0,0)*5);
      SendDlgItemMessage(Win,1040,WM_SETTEXT,0,LPARAM((T("Run speed")+": "
        +(100000/run_speed_ticks_per_second)+"%").Text));
      REFRESH_STATUS_BAR;
      break;
    }
    case 7100:
    {
      int position=(int)SendMessage(HWND(lPar),TBM_GETPOS,0,0);
      int db=(int)-(10000-10000*log10((float)(position+1))/log10((float)101));
      MaxVolume=db;
      SoundChangeVolume();
      break;
    }
#if defined(SSE_DRIVE_SOUND)
    case 7311:
    {
      int position=(int)SendMessage(HWND(lPar),TBM_GETPOS,0,0);
      int db=(int)-(10000-10000*log10((float)(position+1))/log10((float)101));
      FloppyDrive[0].Sound_Volume=FloppyDrive[1].Sound_Volume=db;
      FloppyDrive[0].Sound_ChangeVolume();
      FloppyDrive[1].Sound_ChangeVolume();
      break;
    }
#endif
#if defined(SSE_CPU_MFP_RATIO_OPTION)
    case 7320:
    {
      CpuCustomHz=(DWORD)SendDlgItemMessage(Win,7320,TBM_GETPOS,0,0)*10+8000000;
      char tmp[8];
      itoa(CpuCustomHz,tmp,10);
      SendDlgItemMessage(Win,7321,WM_SETTEXT,0,LPARAM(tmp));
      if(OPTION_CPU_CLOCK)
        CpuMfpRatio=(double)CpuCustomHz/(double)MFP_XTAL;
      break;
    }
#endif
    case 6001:
      MIDI_out_volume=(WORD)SendMessage(HWND(lPar),TBM_GETPOS,0,0);
      if(runstate==RUNSTATE_RUNNING)
      {
        for(int i=0;i<3;i++)
          if(STPort[i].MIDI_Out) 
            STPort[i].MIDI_Out->SetVolume(MIDI_out_volume);
      }
      break;
    case 6041:
      MIDI_in_speed=(int)SendMessage(HWND(lPar),TBM_GETPOS,0,0)+1;
      SendDlgItemMessage(Win,6040,WM_SETTEXT,0,LPARAM((T("Input speed")+": "
        +Str(MIDI_in_speed)+"%").Text));
      break;
    }
    break;
  }
  case WM_DRAWITEM:
    if(wPar==8300)
    {
      GET_THIS;
      DRAWITEMSTRUCT *di=(DRAWITEMSTRUCT*)lPar;
      HBRUSH br;
      int oldtextcol=GetTextColor(di->hDC),oldmode=GetBkMode(di->hDC);
      if(di->itemState & ODS_SELECTED)
      {
        br=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
        SetTextColor(di->hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
      }
      else
      {
        br=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
        SetTextColor(di->hDC,GetSysColor(COLOR_WINDOWTEXT));
      }
      SetBkMode(di->hDC,TRANSPARENT);
      FillRect(di->hDC,&(di->rcItem),br);
      DeleteObject(br);
      if(di->itemID<0xffffffff)
      {
        int idx=di->itemID;
        if(This->eslTOS_Descend) 
          idx=This->eslTOS.NumStrings-1-di->itemID;
        WORD Ver=(WORD)This->eslTOS[idx].Data[0];
        WORD Lang=(WORD)This->eslTOS[idx].Data[1];
        BYTE Recognised=(BYTE)(This->eslTOS[idx].Data[1]>>16); //!
        WORD Date=(WORD)This->eslTOS[idx].Data[2];
        Str Text=This->eslTOS[idx].String;
        char *NameEnd=strchr(Text,'\01');
        if(NameEnd) 
          *NameEnd=0;
        RECT shiftrect=di->rcItem;shiftrect.left+=2;
        int Right=shiftrect.right;
        shiftrect.right=shiftrect.left+60;
        Str szVer=HEXSl(Ver,3).Insert(".",1);
        DrawText(di->hDC,szVer,(int)strlen(szVer),&shiftrect,
          DT_LEFT|DT_SINGLELINE|DT_VCENTER);
        shiftrect.right=shiftrect.left+GetTextSize((HFONT)GetCurrentObject
          (di->hDC,OBJ_FONT),szVer).Width+2;
        HDC TempDC=CreateCompatibleDC(di->hDC);
        HANDLE OldBmp=SelectObject(TempDC,LoadBitmap(Inst,"TOSFLAGS"));
        int FlagIdx=This->TOSLangToFlagIdx(Lang);
        if(FlagIdx>=0)
        {
          BitBlt(di->hDC,shiftrect.right,shiftrect.top+((shiftrect.bottom-
            shiftrect.top)/2)-RC_FLAG_HEIGHT/2,RC_FLAG_WIDTH,RC_FLAG_HEIGHT,
            TempDC,FlagIdx*RC_FLAG_WIDTH,0,SRCCOPY);
        }
        DeleteObject(SelectObject(TempDC,OldBmp));
        DeleteDC(TempDC);
        shiftrect.left+=60;shiftrect.right=Right-105;
        if(Recognised) // bad TOS in red, good in green, don't know in black
          SetTextColor(di->hDC, (Recognised==1) ? RGB(0,110,0) : RGB(190,0,0) );
        DrawText(di->hDC,Text,(int)strlen(Text),&shiftrect,DT_LEFT|DT_SINGLELINE
          |DT_VCENTER);
        if(Recognised)
          SetTextColor(di->hDC,oldtextcol);
        FILETIME ft;
        if(DosDateTimeToFileTime(Date,WORD(0),&ft))
        {
          SYSTEMTIME st;
          FileTimeToSystemTime(&ft,&st);
          Str szDate;
          if(USDateFormat)
            szDate=Str(st.wDay)+"/"+st.wMonth+"/"+st.wYear;
          else
            szDate=Str(st.wMonth)+"/"+st.wDay+"/"+st.wYear;
          shiftrect.left=Right-100;shiftrect.right=Right;
          DrawText(di->hDC,szDate,(int)strlen(szDate),&shiftrect,DT_LEFT|
            DT_SINGLELINE|DT_VCENTER);
        }
      }
      SetTextColor(di->hDC,oldtextcol);
      SetBkMode(di->hDC,oldmode);
      if(di->itemState & ODS_FOCUS) 
        DrawFocusRect(di->hDC,&(di->rcItem));
      return 0;
    }
    break;
  case WM_DELETEITEM:
    //      if (wPar==8300) delete (Str*)(((DELETEITEMSTRUCT*)lPar)->itemData);
    break;
  case WM_ACTIVATE:
    if(wPar!=WA_INACTIVE)
    {
      GET_THIS;
      This->DrawBrightnessBitmap(This->hBrightBmp);
    }
    break;
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
    return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}


int TOptionBox::DTreeNotifyProc(DirectoryTree*,void *t,int Mess,INT_PTR i1,
  INT_PTR) {
  DTREE_LOG(EasyStr("DTree: Options DTreeNotifyProc Mess=")+Mess);
  TOptionBox *This=(TOptionBox*)t;
  if(Mess==DTM_SELCHANGED||Mess==DTM_NAMECHANGED)
  {
    DTREE_LOG(EasyStr("DTree: Getting item path and type for ")+DWORD(i1));
    Str NewSel=DTree.GetItemPath((HTREEITEM)i1);
    int Type=DTree.GetItem((HTREEITEM)i1,TVIF_IMAGE).iImage;
    DTREE_LOG(EasyStr("DTree: NewSel=")+NewSel+" Type="+Type);
    int DisableLo=0,DisableHi=0;
    if(GetDlgCtrlID(DTree.hTree)==10000)
    {
      This->MacroSel=NewSel;
      DTREE_LOG(EasyStr("DTree: Calling UpdateMacroRecordAndPlay"));
      This->UpdateMacroRecordAndPlay(NewSel,Type);
      DisableLo=10010;DisableHi=10030;
    }
    else if(GetDlgCtrlID(DTree.hTree)==11000)
    {
      This->ProfileSel=NewSel;
      DisableLo=11010;DisableHi=11030;
      DTREE_LOG(EasyStr("DTree: Updating profile sections"));
      int i=0;
      HWND LV=GetDlgItem(This->Handle,11013);
      EnableWindow(LV,0);
      TConfigStoreFile CSF;
      if(Type==1) 
        CSF.Open(This->ProfileSel);
      while(ProfileSection[i].Name)
      {
        int Check=LVI_SI_CHECKED;
        if(Type==1) 
          Check=CSF.GetInt("ProfileSections",ProfileSection[i].Name,
            LVI_SI_CHECKED);
        ListView_SetItemState(LV,i++,Check,LVIS_STATEIMAGEMASK);
      }
      if(Type==1) 
        CSF.Close();
    }
    DTREE_LOG(EasyStr("DTree: Enabling/Disabling sections"));
    for(int n=DisableLo;n<DisableHi;n++)
      if(GetDlgItem(This->Handle,n)) 
        EnableWindow(GetDlgItem(This->Handle,n),Type);
  }
  DTREE_LOG(EasyStr("DTree: Finished processing Mess=")+Mess);
  DTREE_LOG("");
  return 0;
}

#undef GET_THIS

#endif//WIN32


#ifdef UNIX

/*
for unix we need to create unix objects, it's duplication
if we change an option, we must change it in the unix build too
ids are different from windows build
*/


void TOptionBox::UpdateProfileDisplay(Str Sel,int Type)
{
  if (Handle==0) return;

  hxc_button *p_grp=(hxc_button*)hxc::find(page_p,2109);
  if (p_grp==NULL) return;

  if (Sel.Empty()){
    Type=-1;
    if (dir_lv.lv.sel>=0){
      Sel=dir_lv.get_item_path(dir_lv.lv.sel);
      Type=dir_lv.sl[dir_lv.lv.sel].Data[DLVD_TYPE];
    }
  }
  ShowHideWindow(XD,p_grp->handle,Type==2);

  if (Type==2){
    TConfigStoreFile CSF;
    CSF.Open(Sel);
    int i=0;
    while (ProfileSection[i].Name){
      int icon=101+ICO16_TICKED;
      if (CSF.GetInt("ProfileSections",ProfileSection[i].Name,PROFILESECT_ON)==PROFILESECT_OFF){
        icon=101+ICO16_UNTICKED;
      }
      profile_sect_lv.sl[i++].Data[0]=icon;
    }
    CSF.Close();
    profile_sect_lv.draw(true);
  }
}
//---------------------------------------------------------------------------
void TOptionBox::UpdatePortDisplay(int p)
{
	if (PortGroup[0].handle==0) return;

	int PortIOType=GetPortIOType(STPort[p].Type);
	if (PortIOType>=0){
    if (STPort[p].Type==PORTTYPE_LAN){
      int Base=1200+p*20;
    	hxc_edit *p_ed_out=(hxc_edit*)hxc::find(LANGroup[p].handle,Base+10);
    	hxc_edit *p_ed_in=(hxc_edit*)hxc::find(LANGroup[p].handle,Base+12);
    	hxc_button *p_but_open=(hxc_button*)hxc::find(LANGroup[p].handle,Base+14);
      if (p_ed_out->text!=STPort[p].PortDev[PortIOType]){
        p_ed_out->set_text(STPort[p].PortDev[PortIOType]);
      }
      if (p_ed_in->text!=STPort[p].LANPipeIn){
        p_ed_in->set_text(STPort[p].LANPipeIn);
      }
      p_but_open->set_check(STPort[p].IsPCPort());
  	  XUnmapWindow(XD,FileGroup[p].handle);
  	  XUnmapWindow(XD,IOGroup[p].handle);
      XMapWindow(XD,LANGroup[p].handle);
    }else{
      if (IODevEd[p].text!=STPort[p].PortDev[PortIOType]){
        IODevEd[p].set_text(STPort[p].PortDev[PortIOType]);
      }
      IOAllowIOBut[p][0].set_check(STPort[p].AllowIO[PortIOType][0]);
      IOAllowIOBut[p][1].set_check(STPort[p].AllowIO[PortIOType][1]);
      IOOpenBut[p].set_check(STPort[p].IsPCPort());
      XUnmapWindow(XD,LANGroup[p].handle);
      XUnmapWindow(XD,FileGroup[p].handle);
      XMapWindow(XD,IOGroup[p].handle);
    }
	}else if (STPort[p].Type==PORTTYPE_FILE){
    XUnmapWindow(XD,LANGroup[p].handle);
	  XUnmapWindow(XD,IOGroup[p].handle);
	  XMapWindow(XD,FileGroup[p].handle);
	}else{
    XUnmapWindow(XD,LANGroup[p].handle);
	  XUnmapWindow(XD,FileGroup[p].handle);
	  XUnmapWindow(XD,IOGroup[p].handle);
	}
}


//---------------------------------------------------------------------------
int TOptionBox::WinProc(TOptionBox *This,Window Win,XEvent *Ev)
{
  switch (Ev->type){
    case ClientMessage:
      if (Ev->xclient.message_type==hxc::XA_WM_PROTOCOLS){
        if (Atom(Ev->xclient.data.l[0])==hxc::XA_WM_DELETE_WINDOW){
          This->Hide();
        }
      }
  }
  return PEEKED_MESSAGE;
}


//---------------------------------------------------------------------------
int TOptionBox::listview_notify_proc(hxc_listview* LV,int Mess,int i)
{
  TOptionBox *This=(TOptionBox*)(LV->owner);
  if (LV->id==60000){
    if (Mess==LVN_SELCHANGE || Mess==LVN_SINGLECLICK){
      int NewPage=LV->sl[LV->sel].Data[1];
      if (This->Page!=NewPage){
        XUnmapWindow(XD,This->page_p);
        hxc::destroy_children_of(This->page_p);
        hints.remove_all_children(This->page_p);
			  This->brightness_ig.FreeIcons();
        This->Page=NewPage;
        This->CreatePage(This->Page);
        XMapWindow(XD,This->page_p);
      }
    }
  }else if (LV->id==1000){
    if (Mess==LVN_SELCHANGE || Mess==LVN_SINGLECLICK){
      This->NewROMFile=strrchr(LV->sl[LV->sel].String,'\01')+1;
      if (This->NewROMFile[0]!='/') This->NewROMFile.Insert(RunDir+"/",0);
      if (IsSameStr_I(ROMFile,This->NewROMFile)) This->NewROMFile="";
      CheckResetIcon();
    }
  }else if (LV->id==2112){
    if (Mess==LVN_ICONCLICK){
      int icon=LV->sl[i].Data[0]-101;
      if (icon==ICO16_TICKED){
        icon=ICO16_UNTICKED;
      }else{
        icon=ICO16_TICKED;
      }
      WriteCSFInt("ProfileSections",ProfileSection[i].Name,
                  int((icon==ICO16_TICKED) ? PROFILESECT_ON:PROFILESECT_OFF),
                  This->ProfileSel);
      LV->sl[i].Data[0]=101+icon;
      LV->draw(0);
      return 1;
    }
  }else if (LV->id>=15000 && LV->id<15100){
    if (Mess==LVN_RETURN || Mess==LVN_CB_RETRACT){
      int n=(LV->id-15000)/10;
      hxc_edit *p_ed=(hxc_edit*)hxc::find(This->page_p,15000+n*10);
      hxc_button *p_but=(hxc_button*)hxc::find(This->page_p,15001+n*10);

      LV->destroy(LV);

      if (i>=0){
        p_ed->set_text(Comlines_Default[n][i]);
        p_ed->notifyproc(p_ed,EDN_CHANGE,0);
      }
      p_but->set_check(0);
    }

  }
  return 0;
}
//---------------------------------------------------------------------------
int TOptionBox::button_notify_proc(hxc_button*b,int mess,int* ip)
{
  TOptionBox *This=(TOptionBox*)(b->owner);
  if (mess==BN_CLICKED){
    if (b->id==100){ //auto load snapshot
      AutoLoadSnapShot=b->checked;
    }else if (b->id==101){ //never use MIT Shared Memory Extension
      WriteCSFStr("Options","NoSHM",EasyStr(b->checked),globalINIFile);
    }else if (b->id==110){
      PauseWhenInactive=b->checked;
    }else if (b->id==120){
      HighPriority=b->checked;
    }else if (b->id==121){
      ShowTips=b->checked;
      if (ShowTips){
        hints.start();
      }else{
        hints.stop();
      }
    }else if (b->id==130){
      floppy_access_ff=b->checked;
    }else if (b->id==140){
      StartEmuOnClick=b->checked;
    }else if (b->id==210){
      draw_fs_fx=(b->checked ? DFSFX_GRILLE:DFSFX_NONE);
      if (draw_grille_black<4) draw_grille_black=4;
      if (runstate!=RUNSTATE_RUNNING) draw(false);
    }else if (b->id==220){
      Disp.DoAsyncBlit=b->checked;
    }else if (b->id==230){
      ResChangeResize=b->checked;
    }else if (b->id==122){
      This->FullscreenBrightnessBitmap();
    }else if (b->id==250 || b->id==251){
      int res=(b->id)-250;
      if (b->checked){
        WinSizeForRes[res]=1;
      }else{
        WinSizeForRes[res]=0;
      }
      if (ResChangeResize && res==screen_res){
        StemWinResize();
      }
    }else if (b->id==252){
      fileselect.set_corner_icon(&Ico16,ICO16_FOLDER);
      EasyStr Path=fileselect.choose(XD,ScreenShotFol,"",T("Pick a Folder"),
          FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");
      if (Path.NotEmpty()){
        NO_SLASH(Path);
        ScreenShotFol=Path;
        CreateDirectory(ScreenShotFol,NULL);
        This->screenshots_fol_display.set_text(ScreenShotFol);
      }
    }else if (b->id==253){
      prefer_res_640_400=b->checked;
    }else if (b->id==5100){
      This->SetRecord(!sound_record);
    }else if (b->id==5101){
      fileselect.set_corner_icon(&Ico16,ICO16_SOUND);
      EasyStr NewFile=fileselect.choose(XD,This->WAVOutputDir,
                GetFileNameFromPath(WAVOutputFile),T("Choose WAV Output File"),FSM_OK,
                wavfile_parse_routine,".wav");
      if (NewFile.NotEmpty()){
        WAVOutputFile=NewFile;
        This->WAVOutputDir=NewFile;
        RemoveFileNameFromPath(This->WAVOutputDir,REMOVE_SLASH);
        This->wav_output_label.set_text(NewFile);
      }
    }else if (b->id==5102){
      This->RecordWarnOverwrite=b->checked;
#if !defined(SSE_NO_INTERNAL_SPEAKER)
    }else if (b->id==5300){
      if (sound_internal_speaker) internal_speaker_sound_by_period(0);

      sound_internal_speaker=!sound_internal_speaker;
      b->set_check(sound_internal_speaker);
#endif
    }else if (b->id==737){ // Choose cart
    	b->set_check(true);
      Str LastCartFol=This->LastCartFile;
      RemoveFileNameFromPath(LastCartFol,REMOVE_SLASH);

      fileselect.set_corner_icon(&Ico16,ICO16_CART);
      EasyStr fn=fileselect.choose(XD,LastCartFol,GetFileNameFromPath(This->LastCartFile),
        T("Find a Cartridge"),FSM_LOAD | FSM_LOADMUSTEXIST,cartfile_parse_routine,".stc");
      if (fn[0]){
        This->LastCartFile=fn;
        if (load_cart(fn)){
          Alert(T("There was an error loading the cartridge."),T("Cannot Load Cartridge"),MB_ICONEXCLAMATION);
        }else{
          CartFile=fn;
          This->cart_display.set_text(fn);
        }
        CheckResetDisplay();
      }
    	b->set_check(0);
    }else if (b->id==747){ // Remove cart
      This->cart_display.set_text("");

      delete[] cart;
      cart=NULL;
#if defined(SSE_SOUND_CARTRIDGE)
      SSEConfig.mv16=SSEConfig.mr16=false;
#endif
      CartFile="";
      //if (pc>=MEM_EXPANSION_CARTRIDGE && pc<0xfc0000){
      	//SET_PC(PC32);
      ///}

      CheckResetDisplay();
    }else if (b->id==960){
    	EnableShiftSwitching=b->checked;
      InitKeyTable();
    }else if (b->id==1000){
      reset_st(RESET_COLD | RESET_STOP | RESET_CHANGESETTINGS | RESET_BACKUP);
    }else if (b->id==1010){
    	b->set_check(true);
      fileselect.set_corner_icon(&Ico16,ICO16_CHIP);
      EasyStr fn=fileselect.choose(XD,This->TOSBrowseDir,"",
        		T("Find a TOS"),FSM_LOAD | FSM_LOADMUSTEXIST,
		        romfile_parse_routine,".img");
      if (fn[0]){
        This->TOSBrowseDir=fn;
        RemoveFileNameFromPath(This->TOSBrowseDir,true);

        bool Found=0;
        for (int i=0;i<This->tos_lv.sl.NumStrings;i++){
        	if (IsSameStr_I(strrchr(This->tos_lv.sl[i].String,'\01')+1,fn)){
        		Found=true;
        		This->tos_lv.changesel(i);
        	}
        }
        if (Found==0){
          if (get_TOS_address(fn)){
            Str Name=GetFileNameFromPath(fn),Ext;
            char *dot=strrchr(Name,'.');
            if (dot){
              Ext=dot;
              *dot=0;
            }
            EasyStr LinkName=WriteDir+SLASH+Name+Ext;
            int n=2;
            while (Exists(LinkName)){
              LinkName=WriteDir+SLASH+Name+"("+(n++)+")"+Ext;
            }
            symlink(fn,LinkName);
            This->TOSRefreshBox(fn);
            This->NewROMFile=fn;
          }else{
            Alert(fn+" "+T("is not a valid TOS image."),T("Cannot use TOS"),MB_ICONEXCLAMATION);
          }
        }
	      CheckResetIcon();
      }
    	b->set_check(0);
    }else if (b->id==1011){
      b->set_check(true);
      This->TOSRefreshBox();
      b->set_check(0);
    }else if (b->id>=1200 && b->id<1300){
      int p=(b->id-1200)/20;
      int i=b->id % 20;
      int IOType=GetPortIOType(STPort[p].Type);
      bool ClosePort=0,UpdateDisplay=0;
      switch (i){
        case 1:case 11:case 13:	// Choose device
        {
          Str CurDev=STPort[p].PortDev[IOType];
          if (i==13) CurDev=STPort[p].LANPipeIn;
          b->set_check(true);
          fileselect.set_corner_icon(&Ico16,ICO16_PORTS);
          Str CurFol=CurDev;
          RemoveFileNameFromPath(CurFol,REMOVE_SLASH);
          EasyStr fn=fileselect.choose(XD,CurFol,GetFileNameFromPath(CurDev),
                        T("Choose Device"),FSM_OK | FSM_LOADMUSTEXIST,NULL,"");
          if (fileselect.chose_option==FSM_OK){
            if (i!=13){
              STPort[p].PortDev[IOType]=fn;
            }else{
              STPort[p].LANPipeIn=fn;
            }
            UpdateDisplay=true;
            ClosePort=true;
          }
          b->set_check(0);
          break;
        }
        case 3: STPort[p].AllowIO[IOType][0]=b->checked;ClosePort=true; break;
        case 4: STPort[p].AllowIO[IOType][1]=b->checked;ClosePort=true; break;
        case 5:case 14: // Open device
        {
          if (STPort[p].IsPCPort()){
            ClosePort=true;
          }else{
            Str ErrText,ErrTitle;
            b->set_check(STPort[p].Create(p,ErrText,ErrTitle,true));
          }
          break;
        }
        case 6: // Choose file
        {
          b->set_check(true);
          fileselect.set_corner_icon(&Ico16,ICO16_PORTS);
          Str CurFol=STPort[p].File;
          RemoveFileNameFromPath(CurFol,REMOVE_SLASH);
          EasyStr fn=fileselect.choose(XD,CurFol,GetFileNameFromPath(STPort[p].File),
                        T("Select Output File"),FSM_OK | FSM_LOADMUSTEXIST,NULL,".dmp");
          if (fileselect.chose_option==FSM_OK){
            STPort[p].File=fn;
            This->FileDisplay[p].set_text(fn);
          }
          b->set_check(0);
          break;
        }
        case 7: // Empty file
        {
          b->set_check(true);
          int Ret=Alert(T("Are you sure? This will permanently delete the contents of the file."),
                          T("Delete Contents?"),MB_ICONQUESTION | MB_YESNO);
          if (Ret==IDYES){
            STPort[p].Close();
            DeleteFile(STPort[p].File);
            Str ErrText,ErrTitle;
            STPort[p].Create(p,ErrText,ErrTitle,true);
          }
          b->set_check(0);
          break;
        }
      }
      if (ClosePort){
        if (STPort[p].IsPCPort()){
          STPort[p].Close();
          UpdateDisplay=true;
        }
      }
      if (UpdateDisplay) This->UpdatePortDisplay(p);
    }else if (b->id==2001){
      b->set_check(true);
      Str Path=This->CreateMacroFile(true);
      dir_lv.select_item_by_name(GetFileNameFromPath(Path));
      This->MacroSel=Path;
      This->UpdateMacroRecordAndPlay();
      b->set_check(0);
    }else if (b->id==2002 || b->id==2102){ // Change store folder
      bool Macro=(b->id==2002);
      char *Current=This->ProfileDir;
      if (Macro) Current=This->MacroDir;
      fileselect.set_corner_icon(&Ico16,ICO16_FOLDER);
      EasyStr new_path=fileselect.choose(XD,Current,"",T("Pick a Folder"),
          FSM_CHOOSE_FOLDER | FSM_CONFIRMCREATE,folder_parse_routine,"");
      if (new_path.NotEmpty()){
        NO_SLASH(new_path);
        if (Macro){
          This->MacroDir=new_path;
          This->MacroSel="";
        }else{
          This->ProfileDir=new_path;
          This->ProfileSel="";
        }

        This->dir_lv.base_fol=new_path;
        This->dir_lv.fol=new_path;
        This->dir_lv.lv.sel=-1;
        This->dir_lv.refresh_fol();

        This->UpdateMacroRecordAndPlay();
        This->UpdateProfileDisplay();
      }
    }else if (b->id==2010){
      if (macro_record==0){
        macro_record_file=This->MacroSel;
        macro_advance(MACRO_STARTRECORD);
      }else{
        macro_end(MACRO_ENDRECORD);
      }
    }else if (b->id==2011){
      if (macro_play==0){
        macro_play_file=This->MacroSel;
        macro_advance(MACRO_STARTPLAY);
      }else{
        macro_end(MACRO_ENDPLAY);
      }
    }else if (b->id==2101 || b->id==2111){
      b->set_check(true);
      Str Path;
      if (b->id==2111){ // Save over
        if (IDYES==Alert(T("Are you sure?"),T("Overwrite File"),MB_ICONQUESTION | MB_YESNO)){
          Path=This->ProfileSel;
        }
      }else{ // Save new
        hxc_prompt prompt;
        Path=prompt.ask(XD,T("New Profile"),T("Enter Name"));
        if (Path.NotEmpty()){
          Path=GetUniquePath(dir_lv.fol,Path+".ini");
        }
      }
      if (Path.NotEmpty()){
        SaveAllDialogData(0,Path);
        dir_lv.refresh_fol();
        dir_lv.select_item_by_name(GetFileNameFromPath(Path));
        This->ProfileSel=Path;
        This->UpdateProfileDisplay();
      }
      b->set_check(0);
    }else if (b->id==2110){
      b->set_check(true);
      This->LoadProfile(This->ProfileSel);
      b->set_check(0);

    }else if (b->id==12000){
      osd_show_disk_light=b->checked;
    }else if (b->id==12001){
      OPTION_DRIVE_INFO=b->checked;
    }else if (b->id==12020){
      osd_show_scrollers=b->checked;
    }else if (b->id==12030){
      This->ChangeOSDDisable(b->checked);
    }else if (b->id>=15000 && b->id<15100){
      int i=(b->id-15000)/10;
      hxc_listview *p_lv=&(This->drop_lv);
      hxc_edit *p_ed=(hxc_edit*)hxc::find(b->parent,15000+i*10);

      b->set_check(true);

      p_lv->sl.DeleteAll();
      for (int cl=0;cl<16;cl++){
        if (Comlines_Default[i][cl]==NULL) break;
        p_lv->additem(Comlines_Default[i][cl]);
      }
      p_lv->itemheight=(b->font->ascent)+(b->font->descent)+2; //use the listview's font!
      p_lv->in_combo=true;
      p_lv->sel=0;
      p_lv->id=15000+i*10;
      p_lv->create(XD,p_ed->handle,0,p_ed->h+1,p_ed->w + b->w,
                p_lv->itemheight*p_lv->sl.NumStrings + p_lv->border*2,listview_notify_proc,This);
      XSetInputFocus(XD,p_lv->handle,RevertToParent,CurrentTime);
      XFlush(XD);
    }

#if defined(SSE_UNIX)
    else if(b->id==4002)
      OPTION_CAPTURE_MOUSE=b->checked;
    else if(b->id==4003)
      OPTION_HACKS=b->checked;
    else if(b->id==4004)
      OPTION_EMU_DETECT=b->checked;
#if defined(SSE_HD6301_LL)
    else if(b->id==4006)
      b->checked=OPTION_C1=b->checked&HD6301_OK;
#endif
#if defined(SSE_TOS_KEYBOARD_CLICK)
    else if(b->id==4007)
    {
      OPTION_KEYBOARD_CLICK=b->checked;
      Tos.CheckKeyboardClick(); // immediate effect
    }
#endif
    else if(b->id==4013)
    {
      //SSEOptions.PSGFixedVolume=b->checked;
    }
#ifdef SSE_GUI_OPTIONS_MICROWIRE
    else if(b->id==4009)
      OPTION_MICROWIRE=b->checked;
#endif
#if defined(SSE_INT_MFP_OPTION)
    else if(b->id==4014)
      OPTION_68901=b->checked;
#endif
    else if(b->id==4015)
      OPTION_VMMOUSE=b->checked;
#endif//SSE_UNIX
  }
  return 0;
}
//---------------------------------------------------------------------------
int TOptionBox::dd_notify_proc(hxc_dropdown*dd,int mess,int i)
{
  TOptionBox*This=(TOptionBox*)(dd->owner);
	if (mess!=DDN_SELCHANGE) return 0;

  if (dd->id==8){
#if defined(SSE_CPU_4GHZ)
    n_cpu_cycles_per_second=MAX(MIN(dd->sl[dd->sel].Data[0],4096000000l),8000000l);
#elif defined(SSE_CPU_3GHZ)
    n_cpu_cycles_per_second=MAX(MIN(dd->sl[dd->sel].Data[0],3072000000l),8000000l);
#elif defined(SSE_CPU_2GHZ)
    n_cpu_cycles_per_second=MAX(MIN(dd->sl[dd->sel].Data[0],2048000000l),8000000l);
#elif defined(SSE_CPU_1GHZ)
    n_cpu_cycles_per_second=MAX(MIN(dd->sl[dd->sel].Data[0],1024000000l),8000000l);
#elif defined(SSE_CPU_512MHZ)
    n_cpu_cycles_per_second=MAX(MIN(dd->sl[dd->sel].Data[0],512000000l),8000000l);
#elif defined(SSE_CPU_256MHZ)
    n_cpu_cycles_per_second=MAX(MIN(dd->sl[dd->sel].Data[0],256000000l),8000000l);
#else
    n_cpu_cycles_per_second=MAX(MIN(dd->sl[dd->sel].Data[0],128000000l),8000000l);
#endif
    if (runstate==RUNSTATE_RUNNING) osd_init_run(0);
    prepare_cpu_boosted_event_plans();
  }else if (dd->id==5001){ //sound mode
    psg_hl_filter=i+1;
  }else if (dd->id==5067){ //sound lib
    Sound_Stop();
    SoundRelease();
    x_sound_lib=dd->sl[dd->sel].Data[0];
    InitSound();
    This->FillSoundDevicesDD();
    Sound_Start();
  }else if (dd->id==5000){ //sound device
    sound_device_name=dd->sl[dd->sel].String;
  }else if (dd->id==5004){ //sound delay
    psg_write_n_screens_ahead=dd->sl[dd->sel].Data[0];
#if !defined(SSE_SOUND_ENFORCE_RECOM_OPT)
  }else if (dd->id==5005){ //sound timing method
    sound_time_method=dd->sl[dd->sel].Data[0];
#endif
  }else if (dd->id==5002){ //sound freq
    sound_chosen_freq=dd->sl[i].Data[0];
    This->UpdateSoundFreq();
  }else if (dd->id==5003){ //sound format
#ifndef NO_RTAUDIO
    if (HIWORD(dd->sl[dd->sel].Data[0])){
      rt_unsigned_8bit=1;
    }else if (x_sound_lib==XS_RT){
      rt_unsigned_8bit=0;
    }
#endif
    This->ChangeSoundFormat(LOBYTE(dd->sl[dd->sel].Data[0]),HIBYTE(dd->sl[dd->sel].Data[0]));
  }else if (dd->id>=12010 && dd->id<12020){
    BYTE *p_element[4]={&osd_show_plasma,&osd_show_speed,&osd_show_icons,&osd_show_cpu};
    *(p_element[dd->id - 12010])=dd->sl[dd->sel].Data[0];
  }
  if (dd==&(This->frameskip_dd)){
    if (i==4){
      frameskip=AUTO_FRAMESKIP;
    }else{
      frameskip=i+1;
    }
  }else if (dd==&(This->border_dd)){
    int newborder=dd->sel,oldborder=border;
    if (This->ChangeBorderModeRequest(newborder)){
      //DISPLAY_SIZE= // see def
      border=newborder;
     // ChangeBorderSize(newborder);
      if (FullScreen) change_fullscreen_display_mode(true);
      //change_window_size_for_border_change(oldborder,newborder);
      else
        StemWinResize();
      draw(false);
    }else{
#if 0
      dd->changesel(oldborder);dd->draw();
      dd->lv.changesel(oldborder);dd->lv.draw(true,true);
#elif defined(SSE_VID_DISABLE_AUTOBORDER)
      dd->changesel(MIN(oldborder,1));dd->draw();
      dd->lv.changesel(MIN(oldborder,1));dd->lv.draw(true,true);
#else
      dd->changesel(MIN(oldborder,2));dd->draw();
      dd->lv.changesel(MIN(oldborder,2));dd->lv.draw(true,true);
#endif
      border=oldborder;
    }
  }else if (dd->id==910){
    This->NewMemConf0=dd->lv.sl[dd->sel].Data[0];
    This->NewMemConf1=dd->lv.sl[dd->sel].Data[1];
    if (SSEConfig.bank_length[0]==mmu_bank_length_from_config[This->NewMemConf0] &&
        SSEConfig.bank_length[1]==mmu_bank_length_from_config[This->NewMemConf1]){
      This->NewMemConf0=-1;
    }
    CheckResetIcon();
  }else if (dd->id==920){
    This->NewMonitorSel=dd->sel;
    if (This->NewMonitorSel==This->GetCurrentMonitorSel()) This->NewMonitorSel=-1;
    CheckResetIcon();
  }else if (dd->id==940){
    KeyboardLangID=(LANGID)(This->keyboard_language_dd.lv.sl[This->keyboard_language_dd.sel].Data[0]);
    InitKeyTable();
  }else if (dd->id==1020){
    This->eslTOS_Sort=(ESLSortEnum)dd->sl[dd->sel].Data[0];
    This->eslTOS_Descend=(bool)dd->sl[dd->sel].Data[1];
    This->TOSRefreshBox();
  }else if (dd->id>=1200 && dd->id<1300){
    int p=(dd->id-1200)/20;
    int NewType=dd->sl[dd->sel].Data[0];
    if (STPort[p].Type!=NewType){
      STPort[p].Close();
      STPort[p].Type=NewType;
      This->UpdatePortDisplay(p);

      // Don't open devices straight away, everything else is okay
      if (GetPortIOType(NewType)==-1){
        Str ErrText,ErrTitle;
        STPort[p].Create(p,ErrText,ErrTitle,true);
      }
    }
  }else if (dd->id==2012 || dd->id==2013){
    TMacroFileOptions MFO;
    macro_file_options(MACRO_FILE_GET,This->MacroSel,&MFO);
    if (dd->id==2012) MFO.max_mouse_speed=dd->sl[dd->sel].Data[0];
    if (dd->id==2013) MFO.allow_same_vbls=dd->sl[dd->sel].Data[0];
    macro_file_options(MACRO_FILE_SET,This->MacroSel,&MFO);
  }


  // SSE
/*
  else if(dd->id==4001) // display size
  {
    DISPLAY_SIZE=dd->sel;
    ChangeBorderSize(DISPLAY_SIZE);
  }
*/
  else if(dd->id==4005) // ST model
  {
    ST_MODEL=dd->sel;
    SSEConfig.SwitchSTModel(ST_MODEL);
  }
/*
  else if(dd->id==4006) // wake-up state
  {
    OPTION_WS=dd->sel;
  }
*/


  return 0;
}
//---------------------------------------------------------------------------
int TOptionBox::edit_notify_proc(hxc_edit *ed,int Mess,int Inf)
{
  switch (ed->id){
    case 5000:
      if (Mess==EDN_CHANGE) sound_device_name=ed->text;
      if (Mess==EDN_RETURN || Mess==EDN_LOSTFOCUS){
        Sound_Stop();
        Sound_Start();
      }
      break;
    case 100:
      if (Mess==EDN_CHANGE) AutoSnapShotName=ed->text;
      break;
  }
  if (ed->id>=1200 && ed->id<1300){
    if (Mess!=EDN_CHANGE) return 0;

    TOptionBox *This=(TOptionBox*)(ed->owner);
    int p=(ed->id-1200)/20;
    int i=ed->id % 20;
    switch (i){
      case 2:
      {
        int IOType=GetPortIOType(STPort[p].Type);
        if (IOType>=0){
          STPort[p].PortDev[IOType]=ed->text;
          if (STPort[p].IsPCPort()){
            STPort[p].Close();
            This->IOOpenBut[p].set_check(0);
          }
        }
        break;
      }
      case 10:
        STPort[p].PortDev[TPORTIO_TYPE_PIPE]=ed->text;
      case 12:
        if (i==12) STPort[p].LANPipeIn=ed->text;
        if (STPort[p].IsPCPort()){
          STPort[p].Close();
          This->UpdatePortDisplay(p);
        }
        break;
    }
  }else if (ed->id>=15000 && ed->id<16000){
    if (Mess==EDN_CHANGE) Comlines[(ed->id-15000)/10]=ed->text;
  }
	return 0;
}
//---------------------------------------------------------------------------
int TOptionBox::scrollbar_notify_proc(hxc_scrollbar *SB,int Mess,int I)
{
	TOptionBox *This=(TOptionBox*)(SB->owner);
	if (Mess==SBN_SCROLLBYONE){
		SB->pos+=I;
		SB->rangecheck();
	}else if (Mess==SBN_SCROLL){
		SB->pos=I;
	}else{
		return 0;
	}
	bool UpdatePalette=0;
	switch (SB->id){
		case 0: slow_motion_speed=SB->pos*10 + 10; break;
    case 1: fast_forward_max_speed=1000/(SB->pos+2); break;
    case 2: run_speed_ticks_per_second=100000/(50 + SB->pos*5); break;
    case 10: Brightness=SB->pos - 128;UpdatePalette=true; break;
    case 11: Contrast=SB->pos - 128;UpdatePalette=true; break;
	}
	SB->draw();
	if (UpdatePalette){
		make_palette_table(Brightness,Contrast);
		if (!flashlight_flag) palette_convert_all();
		This->DrawBrightnessBitmap(This->brightness_image);
		This->brightness_picture.draw();
	}

	This->RunSpeedLabel.set_text(T("Run speed")+": "+(100000/run_speed_ticks_per_second)+"%");
	This->SMSpeedLabel.set_text(T("Slow motion speed")+": "+(slow_motion_speed/10)+"%");

  Str Text=T("Maximum fast forward speed")+": ";
  if (fast_forward_max_speed>50){
    Text+=Str((1000/fast_forward_max_speed)*100)+"%";
  }else{
    Text+=T("Unlimited");
    fast_forward_max_speed=0;
  }
	This->FFMaxSpeedLabel.set_text(Text);

	This->brightness_label.set_text(T("Brightness")+": "+Brightness);
	This->contrast_label.set_text(T("Contrast")+": "+Contrast);

	return 0;
}
//---------------------------------------------------------------------------
int TOptionBox::dir_lv_notify_proc(hxc_dir_lv *lv,int Mess,int i)
{
	TOptionBox *This=(TOptionBox*)(lv->owner);
  switch (Mess){
    case DLVN_SELCHANGE:
    {
      Str new_sel;
      if (i>=0){
        if (lv->sl[i].Data[DLVD_TYPE]==0){ // Up folder
          new_sel=lv->fol+"/..";
        }else{
          new_sel=lv->get_item_path(i);
        }
      }
      if (lv->id==2000){ // Macro
        if (new_sel==This->MacroSel) break;
        This->MacroSel=new_sel;
        This->UpdateMacroRecordAndPlay();
      }else{
        if (new_sel==This->ProfileSel) break;
        This->ProfileSel=new_sel;
        This->UpdateProfileDisplay();
      }
      break;
    }
  }
  return 0;
}
//---------------------------------------------------------------------------

#endif//UNIX
