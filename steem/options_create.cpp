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
FILE: options_create.cpp
DESCRIPTION: Functions to create the pages of the options dialog box.
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
#include <midi.h>
#include <notifyinit.h>

const int HorizontalSeparation=5;
const int LineHeight=30;

void TOptionBox::CreatePage(int n) {
  switch(n) {
  case PAGE_MACHINE: CreateMachinePage();break;
  case 10:CreateTOSPage();break;
  case 13:CreateMacrosPage();break;
  case 12:CreatePortsPage();break;
#ifdef WIN32
  case 4:CreateMIDIPage();break;
#endif
  case 0:CreateGeneralPage();break;
  case PAGE_SOUND: CreateSoundPage();break;
  case 1:CreateDisplayPage();break;
#ifdef WIN32
  case 15:CreateOSDPage();break;
#endif
#ifdef UNIX
  case 15:CreatePathsPage();break;
#endif
#ifdef WIN32
  case 3:CreateFullscreenPage();break;
#endif
  case 2:CreateBrightnessPage();break;
  case 11:CreateProfilesPage();break;
  case 6:CreateStartupPage();break;
#ifdef WIN32
  case 14:CreateIconsPage();break;
#endif
#ifdef UNIX
  case 14:CreateOSDPage();break;
#endif
#ifdef WIN32
  case 8:CreateAssocPage();break;
#endif
#if !defined(SSE_NO_UPDATE)
  case 7:CreateUpdatePage();break;
#endif
  case 16:CreateSSEPage();break;
  case 17:CreateInputPage(); break;
  case 18:CreateSTVideoPage(); break;
  }
#ifdef WIN32
  Focus=PageTree;
  SetPageControlsFont();
  ShowPageControls();
#endif
#ifdef UNIX
  XFlush(XD);
#endif
}


void TOptionBox::CreateMachinePage() {
  int y=10,Wid;

#ifdef WIN32
  HWND Win;
  int Offset,mask;
  Wid=get_text_width(T("ST model"));
  CreateWindow("Static",T("ST model"),WS_CHILD,
    page_l,y+4,Wid,21,Handle,(HMENU)209,HInstance,NULL);
  Offset=Wid+HorizontalSeparation;
  mask=WS_CHILD|BS_AUTORADIOBUTTON;
  Wid=GetCheckBoxSize(Font,st_model_name[STF]).Width;
  Win=CreateWindow("Button",st_model_name[STF],mask|WS_GROUP,page_l+Offset,
    y,Wid,25,Handle,(HMENU)(IDC_RADIO_ST_MODEL+STF),HInstance,NULL);
  EasyStr hint=T("The STE was more elaborated than the older STF but some programs are \
compatible only with the STF");
  ToolAddWindow(ToolTip,Win,hint);
#if !defined(SSE_MEGASTE) // if Mega STE model available, useless STFM disappears from options (no room!)
    Offset+=Wid+HorizontalSeparation;
    Wid=GetCheckBoxSize(Font,st_model_name[STFM]).Width;
    Win=CreateWindow("Button",st_model_name[STFM],mask,page_l+Offset,
      y,Wid,25,Handle,(HMENU)(IDC_RADIO_ST_MODEL+STFM),HInstance,NULL);
    ToolAddWindow(ToolTip,Win,hint);
#endif
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,st_model_name[MEGA_ST]).Width;
  Win=CreateWindow("Button",st_model_name[MEGA_ST],mask,page_l+Offset,
    y,Wid,25,Handle,(HMENU)(IDC_RADIO_ST_MODEL+MEGA_ST),HInstance,NULL);
  ToolAddWindow(ToolTip,Win,hint);
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,st_model_name[STE]).Width;
  Win=CreateWindow("Button",st_model_name[STE],mask,page_l+Offset,
    y,Wid,25,Handle,(HMENU)(IDC_RADIO_ST_MODEL+STE),HInstance,NULL);
  ToolAddWindow(ToolTip,Win,hint);
#if defined(SSE_MEGASTE)
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,st_model_name[MEGA_STE]).Width;
  Win=CreateWindow("Button",st_model_name[MEGA_STE],mask,page_l+Offset,
    y,Wid,25,Handle,(HMENU)(IDC_RADIO_ST_MODEL+MEGA_STE),HInstance,NULL);
  ToolAddWindow(ToolTip,Win,hint);
#endif
  SendMessage(GetDlgItem(Handle,IDC_RADIO_ST_MODEL+ST_MODEL),BM_SETCHECK,TRUE,0);
  y+=LineHeight;
  ADVANCED_HACK_BEGIN
  Wid=get_text_width(T("ST CPU speed"));
  CreateWindow("Static",T("ST CPU speed"),WS_CHILD,page_l,y+4,Wid,23,Handle,(HMENU)403,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,
    page_l+5+Wid,y,page_w-(5+Wid),400,Handle,(HMENU)IDC_CPU_SPEED,HInstance,NULL);
  EasyStr Mhz=T("Mhz");
  CBAddString(Win,EasyStr("8 ")+Mhz+" ("+T("ST standard")+")",CpuNormalHz);
  CBAddString(Win,EasyStr("9 ")+Mhz,9000000);
  CBAddString(Win,EasyStr("10 ")+Mhz,10000000);
  CBAddString(Win,EasyStr("11 ")+Mhz,11000000);
  CBAddString(Win,EasyStr("12 ")+Mhz,12000000);
  CBAddString(Win,EasyStr("14 ")+Mhz,14000000);
  CBAddString(Win,EasyStr("16 ")+Mhz,16000000);
  CBAddString(Win,EasyStr("20 ")+Mhz,20000000);
  CBAddString(Win,EasyStr("24 ")+Mhz,24000000);
  CBAddString(Win,EasyStr("28 ")+Mhz,28000000);
  CBAddString(Win,EasyStr("32 ")+Mhz,32000000);
  CBAddString(Win,EasyStr("36 ")+Mhz,36000000);
  CBAddString(Win,EasyStr("40 ")+Mhz,40000000);
  CBAddString(Win,EasyStr("44 ")+Mhz,44000000);
  CBAddString(Win,EasyStr("48 ")+Mhz,48000000);
  CBAddString(Win,EasyStr("56 ")+Mhz,56000000);
  CBAddString(Win,EasyStr("64 ")+Mhz,64000000);
  CBAddString(Win,EasyStr("80 ")+Mhz,80000000);
  CBAddString(Win,EasyStr("96 ")+Mhz,96000000);
  CBAddString(Win,EasyStr("128 ")+Mhz,128000000);
  CBAddString(Win,EasyStr("256 ")+Mhz,256000000);
  CBAddString(Win,EasyStr("512 ")+Mhz,512000000);
  for(COUNTER_VAR i=1000000000,j=1;i>0&&i<=CPU_MAX_HERTZ;i+=1000000000,j++)
  {
    char not_easystr[40];
    sprintf(not_easystr,"%d Ghz",j);
    CBAddString(Win,not_easystr,i);
  }
  if(CBSelectItemWithData(Win,n_cpu_cycles_per_second)<0) 
  {
    n_cpu_cycles_per_second=CpuNormalHz;
    CBSelectItemWithData(Win,n_cpu_cycles_per_second);
  }
  y+=LineHeight;
  ADVANCED_END
  Wid=GetTextSize(Font,T("Memory size")).Width;
  CreateWindow("Static",T("Memory size"),WS_CHILD,page_l,y+4,Wid,20,Handle,HMENU(8090),HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
    page_l+5+Wid,y,page_w-(5+Wid),200,Handle,(HMENU)IDC_MEMORY_SIZE,HInstance,NULL);
  ADVANCED_BEGIN // curiosity
    CBAddString(Win,"256Kb",MAKELONG(MEMCONF_128,MEMCONF_128));
  ADVANCED_END
  CBAddString(Win,"512Kb",MAKELONG(MEMCONF_512,MEMCONF_0));
  CBAddString(Win,"1 MB",MAKELONG(MEMCONF_512,MEMCONF_512));
  CBAddString(Win,"2 MB",MAKELONG(MEMCONF_2MB,MEMCONF_0));
  ADVANCED_BEGIN // curiosity
    CBAddString(Win,"2.5 MB",MAKELONG(MEMCONF_512,MEMCONF_2MB));
  ADVANCED_END
  CBAddString(Win,"4 MB",MAKELONG(MEMCONF_2MB,MEMCONF_2MB));
  ADVANCED_BEGIN
#if defined(SSE_MMU_MONSTER_ALT_RAM)
    CBAddString(Win,"12 MB (MonSTer alt-RAM)",MAKELONG(MEMCONF_6MB,MEMCONF_6MB));
#endif
  ADVANCED_END
#if !defined(SSE_GUI_NO14MB)
  ADVANCED_HACK_BEGIN
    CBAddString(Win, "14 MB (hack)", MAKELONG(MEMCONF_7MB, MEMCONF_7MB));
  ADVANCED_END
#endif
  y+=LineHeight;
  ADVANCED_BEGIN
    CreateWindow("Button",T("Cartridge"),WS_CHILD|BS_GROUPBOX,
      page_l,y,page_w,80,Handle,(HMENU)8093,HInstance,NULL);
    y+=20;
    CreateWindowEx(512,"Steem Path Display","",WS_CHILD,
      page_l+10,y,page_w-20,22,Handle,(HMENU)8500,HInstance,NULL);
    y+=LineHeight;
    CreateWindow("Button",T("Choose"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
      page_l+10,y,(page_w-20)/4-5,23,Handle,(HMENU)8501,HInstance,NULL);
    CreateWindow("Button",T("Remove"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
      page_l+10+(page_w-20)/4,y,(page_w-20)/4-5,23,Handle,(HMENU)8502,HInstance,NULL);
    Win=CreateWindow("Button","",WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
      page_l+10+2*(page_w-20)/4,y,(page_w-20)/4-5,23,Handle,(HMENU)8504,HInstance,NULL);
    CreateWindow("Button",T("Freeze"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
      page_l+10+3*(page_w-20)/4,y,(page_w-20)/4-5,23,Handle,(HMENU)8503,HInstance,NULL);
    y+=40;
  ADVANCED_END
#if defined(SSE_INT_MFP_OPTION) // no option in v4
  ADVANCED_BEGIN
    mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX;
    Wid=GetCheckBoxSize(Font,T("Precise interrupt emulation")).Width;
    Win=CreateWindow("Button",T("Precise interrupt emulation"),mask,
      page_l,y,Wid,25,Handle,(HMENU)7323,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,OPTION_68901,0);
    ToolAddWindow(ToolTip,Win,
      T("Check for a more precise emulation of interrupt handling by the CPU and the MFP."));
    y+=LineHeight;
  ADVANCED_END
#endif
  mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX;
#if 1
  ADVANCED_BEGIN
    Offset=0;
    Wid=GetCheckBoxSize(Font,T("Spurious interrupts")).Width;
    Win=CreateWindow("Button",T("Spurious interrupts"),mask,
      page_l+Offset,y,Wid,25,Handle,(HMENU)1048,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,OPTION_SPURIOUS,0);
    ToolAddWindow(ToolTip,Win,
      T("Emulation of this is tricky, that's why you can disable it.\
 Especially if you suddenly see 24 bombs on your screen."));
    y+=LineHeight;
  ADVANCED_END
#endif
#if defined(SSE_CPU_MFP_RATIO_OPTION)
  ADVANCED_HACK_BEGIN
    // user can fine tune CPU timer XTAL, more or less legit hack
#define ST_CYCLES_TO_CONTROL(c) ((c-8000000)/10)
    Wid=GetCheckBoxSize(Font,T("Fine tune CPU clock")).Width;
    Win=CreateWindow("Button",T("Fine tune CPU clock"),mask,
      page_l,y,Wid,25,Handle,(HMENU)7322,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,OPTION_CPU_CLOCK,0);
    ToolAddWindow(ToolTip,Win,
      T("This parameter affects the CPU/MFP timer crystal ratio, it is a hack."));
    CreateWindow("Static","",WS_CHILD|SS_CENTER,page_l+115,y+5,100,20,Handle,(HMENU)7321,HInstance,NULL);
    y+=20;
    Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD|WS_TABSTOP|TBS_HORZ,
      page_l,y,page_w,18,Handle,(HMENU)7320,HInstance,NULL);
    SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(ST_CYCLES_TO_CONTROL(8000000),ST_CYCLES_TO_CONTROL(8030000)));
    SendMessage(Win,TBM_SETLINESIZE,0,1);
    SendMessage(Win,TBM_SETTIC,0,ST_CYCLES_TO_CONTROL(CPU_CLOCK_MEGA_ST));
    SendMessage(Win,TBM_SETTIC,0,ST_CYCLES_TO_CONTROL(CPU_CLOCK_STE_PAL)); //we know it should be the same...
    SendMessage(Win,TBM_SETTIC,0,ST_CYCLES_TO_CONTROL(CPU_CLOCK_STF_PAL));
    SendMessage(Win,TBM_SETPOS,1,ST_CYCLES_TO_CONTROL(CpuCustomHz));
    SendMessage(Win,TBM_SETPAGESIZE,0,1);
    SendMessage(Handle,WM_HSCROLL,0,LPARAM(Win));
    y+=LineHeight;
#undef ST_CYCLES_TO_CONTROL
  ADVANCED_END
#endif
  EasyStr protip=T("Memory changes don't take effect until the next cold reset of \
the ST. ST model changes are immediate!");
  CreateResetButton(y,protip);
#endif//WIN32

#ifdef UNIX

  Wid=hxc::get_text_width(XD,T("ST model"));
  st_type_label.create(XD,page_p,page_l,y,Wid,25,NULL,this,BT_STATIC 
    | BT_TEXT,T("ST Model"),0,BkCol);

  st_type_dd.id=4005;
  st_type_dd.make_empty();

  for(int m=0;m<N_ST_MODELS;m++)
    st_type_dd.additem(st_model_name[m],m);

  st_type_dd.select_item_by_data(ST_MODEL);

  st_type_dd.create(XD,page_p,page_l+5+Wid,y,180-(15+Wid+10),350,
    dd_notify_proc,this);

#if 0
  Wid=hxc::get_text_width(XD,T("Wake-up state"));
  wake_up_label.create(XD,page_p,page_l+160,y,Wid,25,NULL,this,BT_STATIC 
    | BT_TEXT,T("Wake-up state"),0,BkCol);
  wake_up_dd.id=4006;
  wake_up_dd.make_empty();
  wake_up_dd.additem("Ignore",0);

  wake_up_dd.additem("DL3 WU2 WS2",1);
  wake_up_dd.additem("DL4 WU2 WS4",2);
  wake_up_dd.additem("DL5 WU1 WS3",3);
  wake_up_dd.additem("DL6 WU1 WS1",4);

  wake_up_dd.select_item_by_data(OPTION_WS);

  hints.add(wake_up_dd.handle, // works?
    "Very technical - Some demos will display correctly only in one of those states."
    ,page_p);

  wake_up_dd.create(XD,page_p,page_l+160+Wid,y,250-(15+Wid+10),350,
    dd_notify_proc,this);
#endif

#if 1
  memory_label.create(XD,page_p,page_l+160+5,y,0,25,NULL,this,BT_LABEL,
    T("Memory size"),0,BkCol);
  memory_dd.id=910;
  memory_dd.make_empty();
  memory_dd.lv.sl.Add("512Kb",MEMCONF_512,MEMCONF_0);
  memory_dd.lv.sl.Add("1 MB",MEMCONF_512,MEMCONF_512);
  memory_dd.lv.sl.Add("2 MB",MEMCONF_2MB,MEMCONF_0);
  memory_dd.lv.sl.Add("4 MB",MEMCONF_2MB,MEMCONF_2MB);
  memory_dd.lv.sl.Add("14 MB",MEMCONF_7MB,MEMCONF_7MB);
	memory_dd.create(XD,page_p,page_l+5+memory_label.w+160,y,
    page_w-(5+memory_label.w+160),200,dd_notify_proc,this);
#endif


  y+=LineHeight;

  Wid=hxc::get_text_width(XD,T("ST CPU speed"));
	cpu_boost_label.create(XD,page_p,page_l,y,Wid,25,NULL,this,BT_STATIC | BT_TEXT,T("ST CPU speed"),0,BkCol);
  cpu_boost_dd.id=8;

  cpu_boost_dd.make_empty();
  EasyStr Mhz=T("Megahertz");
  cpu_boost_dd.additem(EasyStr("8 ")+Mhz+" ("+T("ST standard")+")",CpuNormalHz);
  cpu_boost_dd.additem(EasyStr("9 ")+Mhz,9000000);
  cpu_boost_dd.additem(EasyStr("10 ")+Mhz,10000000);
  cpu_boost_dd.additem(EasyStr("11 ")+Mhz,11000000);
  cpu_boost_dd.additem(EasyStr("12 ")+Mhz,12000000);
  cpu_boost_dd.additem(EasyStr("14 ")+Mhz,14000000);
  cpu_boost_dd.additem(EasyStr("16 ")+Mhz,16000000);
  cpu_boost_dd.additem(EasyStr("20 ")+Mhz,20000000);
  cpu_boost_dd.additem(EasyStr("24 ")+Mhz,24000000);
  cpu_boost_dd.additem(EasyStr("28 ")+Mhz,28000000);
  cpu_boost_dd.additem(EasyStr("32 ")+Mhz,32000000);
  cpu_boost_dd.additem(EasyStr("36 ")+Mhz,36000000);
  cpu_boost_dd.additem(EasyStr("40 ")+Mhz,40000000);
  cpu_boost_dd.additem(EasyStr("44 ")+Mhz,44000000);
  cpu_boost_dd.additem(EasyStr("48 ")+Mhz,48000000);
  cpu_boost_dd.additem(EasyStr("56 ")+Mhz,56000000);
  cpu_boost_dd.additem(EasyStr("64 ")+Mhz,64000000);
  cpu_boost_dd.additem(EasyStr("80 ")+Mhz,80000000);
  cpu_boost_dd.additem(EasyStr("96 ")+Mhz,96000000);
  cpu_boost_dd.additem(EasyStr("128 ")+Mhz,128000000);
  cpu_boost_dd.additem(EasyStr("256 ")+Mhz,256000000);
  cpu_boost_dd.additem(EasyStr("512 ")+Mhz,512000000);
#if 1 // O2 does something terrible with the loop
  cpu_boost_dd.additem("1 GHz",1000000000);
  cpu_boost_dd.additem("2 GHz",2000000000);
#else
  for(COUNTER_VAR i=1000000000,j=1;(i>0&&i<=CPU_MAX_HERTZ);i+=1000000000,j++)
  {
    char not_easystr[40];
    sprintf(not_easystr,"%d Ghz",j);
    cpu_boost_dd.additem(not_easystr,i);
  }
#endif
  if (cpu_boost_dd.select_item_by_data(n_cpu_cycles_per_second)<0){
    EasyStr Cycles=n_cpu_cycles_per_second;
    Cycles=Cycles.Lefts(Cycles.Length()-6);
    cpu_boost_dd.additem(Cycles+" "+Mhz,n_cpu_cycles_per_second);
    cpu_boost_dd.changesel(cpu_boost_dd.lv.sl.NumStrings-1);
  }

  cpu_boost_dd.create(XD,page_p,page_l+5+Wid,y,400-(15+Wid+10),350,dd_notify_proc,this);
  y+=35;

#if 0
  memory_label.create(XD,page_p,page_l,y,0,25,NULL,this,BT_LABEL,T("Memory size"),0,BkCol);

  memory_dd.id=910;
  memory_dd.make_empty();
  memory_dd.lv.sl.Add("512Kb",MEMCONF_512,MEMCONF_0);
  memory_dd.lv.sl.Add("1 MB",MEMCONF_512,MEMCONF_512);
  memory_dd.lv.sl.Add("2 MB",MEMCONF_2MB,MEMCONF_0);
  memory_dd.lv.sl.Add("4 MB",MEMCONF_2MB,MEMCONF_2MB);
  memory_dd.lv.sl.Add("14 MB",MEMCONF_7MB,MEMCONF_7MB);
	memory_dd.create(XD,page_p,page_l+5+memory_label.w,y,page_w-(5+memory_label.w),200,dd_notify_proc,this);
	y+=35;
#endif
	monitor_label.create(XD,page_p,page_l,y,0,25,NULL,this,BT_LABEL,
													T("Monitor"),0,BkCol);

  monitor_dd.id=920;
  monitor_dd.make_empty();
  monitor_dd.additem(T("Colour")+" ("+T("Low/Med Resolution")+")");
  monitor_dd.additem(T("Monochrome")+" ("+T("High Resolution")+")");
#ifndef NO_CRAZY_MONITOR
  for(int n=0;n<EXTMON_RESOLUTIONS;n++){
    monitor_dd.additem(T("Extended Monitor At")+" "+extmon_res[n][0]+"x"+extmon_res[n][1]+"x"+extmon_res[n][2]);
  }
#endif

  monitor_dd.create(XD,page_p,page_l+5+monitor_label.w,y,page_w-(5+monitor_label.w),200,dd_notify_proc,this);
  y+=35;

  hxc_button *kg=new hxc_button(XD,page_p,page_l,y,page_w,85,NULL,this,
					BT_GROUPBOX,T("Keyboard"),0,hxc::col_bk);

  keyboard_language_dd.id=940;
  keyboard_language_dd.make_empty();
  keyboard_language_dd.additem(T("United States"),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US));
  keyboard_language_dd.additem(T("United Kingdom"),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_UK));
  keyboard_language_dd.additem(T("Australia (UK TOS)"),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_AUS));
  keyboard_language_dd.additem(T("German"),MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN));
  keyboard_language_dd.additem(T("French"),MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH));
  keyboard_language_dd.additem(T("Spanish"),MAKELANGID(LANG_SPANISH,SUBLANG_SPANISH));
  keyboard_language_dd.additem(T("Italian"),MAKELANGID(LANG_ITALIAN,SUBLANG_ITALIAN));
  keyboard_language_dd.additem(T("Swedish"),MAKELANGID(LANG_SWEDISH,SUBLANG_SWEDISH));
  keyboard_language_dd.additem(T("Norwegian"),MAKELANGID(LANG_NORWEGIAN,SUBLANG_NEUTRAL));
  keyboard_language_dd.additem(T("Belgian (French TOS)"),MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_BELGIAN));
#if defined(SSE_IKBD_MAPPINGFILE)
  keyboard_language_dd.additem(T("Mapping File"),MAKELANGID(LANG_CUSTOM,SUBLANG_NEUTRAL));
#endif
  if (keyboard_language_dd.select_item_by_data(KeyboardLangID)<0){
    // if can't find the language
    keyboard_language_dd.sel=0;
    KeyboardLangID=MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US);
  }
  keyboard_language_dd.grandfather=page_p;
  Wid=hxc::get_text_width(XD,T("Language"));
	keyboard_language_label.create(XD,kg->handle,10,20,Wid,25,NULL,this,
                                  BT_TEXT | BT_STATIC | BT_BORDER_NONE,
																	T("Language"),0,BkCol);
  keyboard_language_dd.create(XD,kg->handle,15+Wid,20,page_w-20-(5+Wid),200,dd_notify_proc,this);

  keyboard_sc_but.set_check(EnableShiftSwitching);
  keyboard_sc_but.create(XD,kg->handle,10,50,0,25,button_notify_proc,this,
    BT_CHECKBOX,T("Shift and alternate correction"),960,BkCol);
  hints.add(keyboard_sc_but.handle,T("When checked this allows Steem to emulate all keys correctly, it does this by changing the shift and alternate state of the ST when you press them.")+" "+
                              T("This could interfere with games and other programs, only use it if you are doing lots of typing.")+" "+
                              T("Please note that instead of pressing Alt-Gr or Control to access characters on the right-hand side of a key, you have to press Alt or Alt+Shift (this is how it was done on an ST)."),
                              page_p);
  y+=95;

  cart_group.create(XD,page_p,page_l,y,page_w,90,NULL,this,BT_GROUPBOX,T("Cartridge"),0,BkCol);

  cart_display.create(XD,cart_group.handle,10,25,
    page_w-20,25,NULL,this,BT_STATIC|BT_TEXT|BT_BORDER_INDENT|BT_TEXT_PATH,
    CartFile.Text,0,WhiteCol);

  cart_change_but.create(XD,cart_group.handle,10,55,
    page_w/2-10-5,25,button_notify_proc,this,BT_TEXT,T("Choose"),737,BkCol);

  cart_remove_but.create(XD,cart_group.handle,page_w/2+5,55,
    page_w/2-10-5,25,button_notify_proc,this,BT_TEXT,T("Remove"),747,BkCol);
  y+=100;

  

  mustreset_td.text=T("Memory and monitor changes don't take effect until the next cold reset of the ST");
  mustreset_td.sy=0;
  mustreset_td.wordwrapped=false;
	mustreset_td.create(XD,page_p,page_l,y,page_w,45,hxc::col_white,0);
  y+=55;
  
  coldreset_but.create(XD,page_p,page_l,y,page_w,25,button_notify_proc,this,
            BT_TEXT,T("Perform Cold Reset Now"),1000,hxc::col_bk);

#endif//UNIX

  MachineUpdateIfVisible();
}


void TOptionBox::MachineUpdateIfVisible() {

#ifdef WIN32
  TOSRefreshBox("");
  if(Handle==NULL)
    return;
  if(GetDlgItem(Handle,IDC_ST_AR)!=NULL) // it's ST Video Page instead
    UpdateSTVideoPage(); // then update it
  HWND cb_mem=GetDlgItem(Handle,IDC_MEMORY_SIZE);
  if(cb_mem==NULL) // not machine page
    return;
  SendMessage(GetDlgItem(Handle,IDC_RADIO_ST_MODEL+ST_MODEL),BM_SETCHECK,TRUE,0);
  // should work with different builds (w/wo 256KB, 2.5MB...)
  BYTE MemConf[2]={MEMCONF_512,MEMCONF_512};
  if(NewMemConf0==-1) // no new memory config selected
    GetCurrentMemConf(MemConf);
  else
  {
    MemConf[0]=(BYTE)NewMemConf0;
    MemConf[1]=(BYTE)NewMemConf1;
  }
  DWORD dwMemConf=MAKELONG(MemConf[0],MemConf[1]);
  // by Steem authors - could be used more?
  LRESULT curs_index=CBFindItemWithData(cb_mem,dwMemConf); 
  SendMessage(cb_mem,CB_SETCURSEL,curs_index,0);
  SetWindowText(GetDlgItem(Handle,8500),CartFile);
  EnableWindow(GetDlgItem(Handle,8502),CartFile.NotEmpty());
  EnableWindow(GetDlgItem(Handle,8503),CartFile.NotEmpty());
  SendMessage(GetDlgItem(Handle,8504),WM_SETTEXT,0,SSEOptions.CartidgeOff
    ? (LPARAM)T("Switch on").Text : (LPARAM)T("Switch off").Text);
  EnableWindow(GetDlgItem(Handle,8504),CartFile.NotEmpty());
#endif

#ifdef UNIX
  TOSRefreshBox();

  int memconf=4;
  if (NewMemConf0<0){
    if (mem_len<1024*1024){
      memconf=0;
    }else if (mem_len<2048*1024){
      memconf=1;
    }else if (mem_len<4096*1024){
      memconf=2;
    }else if (mem_len<14*1024*1024){
      memconf=3;
    }
  }else{
    if (NewMemConf0==MEMCONF_512) memconf=int((NewMemConf1==MEMCONF_512) ? 1:0); // 1Mb:512Kb
    if (NewMemConf0==MEMCONF_2MB) memconf=int((NewMemConf1==MEMCONF_2MB) ? 3:2); // 4Mb:2Mb
  }

  memory_dd.changesel(memconf);

  int monitor_sel=NewMonitorSel;
  if (monitor_sel<0) monitor_sel=GetCurrentMonitorSel();
  monitor_dd.changesel(monitor_sel);

  cart_display.set_text(CartFile.Text);
#endif//UNIX
}


void TOptionBox::CreateTOSPage() {
  int y=10;

#ifdef WIN32
  int Wid;
  HWND Win;
  TNotify myNotify(T("Checking TOS files"));
#if !defined(SSE_GUI_TOS_NOSORTCHOICE)
  ADVANCED_BEGIN // because it's a bit over the top
    Wid=GetTextSize(Font,T("Sort by")).Width;
    CreateWindow("Static",T("Sort by"),WS_CHILD,page_l,y+4,Wid,25,
      Handle,HMENU(8310),HInstance,NULL);
    Win=CreateWindow("Combobox","",WS_CHILD|CBS_DROPDOWNLIST|WS_TABSTOP,
      page_l+Wid+5,y,page_w-(Wid+5),200,Handle,HMENU(8311),HInstance,NULL);
    CBAddString(Win,T("Version (Ascending)"),MAKELONG((WORD)eslSortByData0,0));
    CBAddString(Win,T("Version (Descending)"),MAKELONG((WORD)eslSortByData0,1));
    CBAddString(Win,T("Language"),MAKELONG((WORD)eslSortByData1,0));
    CBAddString(Win,T("Date (Ascending)"),MAKELONG((WORD)eslSortByData2,0));
    CBAddString(Win,T("Date (Descending)"),MAKELONG((WORD)eslSortByData2,1));
    CBAddString(Win,T("Name (Ascending)"),MAKELONG((WORD)(signed short)eslSortByNameI,0));
    CBAddString(Win,T("Name (Descending)"),MAKELONG((WORD)(signed short)eslSortByNameI,1));
    if(CBSelectItemWithData(Win,MAKELONG(eslTOS_Sort,eslTOS_Descend))<0)
    {
      SendMessage(Win,CB_SETCURSEL,0,0);
      eslTOS_Sort=eslSortByData0;
      eslTOS_Descend=0;
    }
    y+=LineHeight;
  ADVANCED_END
#endif
  TWidthHeight wh=GetTextSize(Font,T("TOS changes don't take effect until the next cold reset of the ST. \
Be advised that STF and STE need different TOS. e.g. STF: 1.02 STE: 1.62"));
  if(wh.Width>=page_w) 
    wh.Height=(wh.Height+1)*2;
  int TOSBoxHeight=(OPTIONS_HEIGHT-20)-(10+30+30+wh.Height+5+23+10);
  Win=CreateWindowEx(512,"ListBox","",WS_CHILD|WS_VSCROLL|
    WS_TABSTOP|LBS_NOINTEGRALHEIGHT|LBS_OWNERDRAWFIXED|LBS_NOTIFY|LBS_SORT,
    page_l,y,page_w,TOSBoxHeight,Handle,(HMENU)8300,HInstance,NULL);
  SendMessage(Win,LB_SETITEMHEIGHT,0,MAX((int)GetTextSize(Font,"HyITljq").Height+4,RC_FLAG_HEIGHT+4));
  y+=TOSBoxHeight+10;
  CreateWindow("Button",T("Add"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
    page_l,y,page_w/2-5,23,Handle,(HMENU)8301,HInstance,NULL);
  CreateWindow("Button",T("Remove"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
    page_l+page_w/2+5,y,page_w/2-5,23,Handle,(HMENU)8302,HInstance,NULL);
  y+=LineHeight;
  CreateResetButton(y,T("TOS changes don't take effect until the next cold reset of the ST. \
Be advised that STF and STE need different TOS."));
  MachineUpdateIfVisible();
#endif//WIN32

#ifdef UNIX
	int tosbox_h=OPTIONS_HEIGHT-10-35-10-35-55-25-10;

	hxc_button *label=new hxc_button(XD,page_p,page_l,y,0,25,NULL,this,
                      BT_TEXT | BT_STATIC | BT_BORDER_NONE,
											T("Sort by"),0,BkCol);

  tos_sort_dd.make_empty();
  tos_sort_dd.lv.sl.Add(T("Version (Ascending)"),eslSortByData0,0);
  tos_sort_dd.lv.sl.Add(T("Version (Descending)"),eslSortByData0,1);
  tos_sort_dd.lv.sl.Add(T("Language"),eslSortByData1,0);
  tos_sort_dd.lv.sl.Add(T("Date (Ascending)"),eslSortByData2,0);
  tos_sort_dd.lv.sl.Add(T("Date (Descending)"),eslSortByData2,1);
  tos_sort_dd.lv.sl.Add(T("Name (Ascending)"),eslSortByNameI,0);
  tos_sort_dd.lv.sl.Add(T("Name (Descending)"),eslSortByNameI,1);
  bool Found=0;
	for (int i=0;i<tos_sort_dd.lv.sl.NumStrings;i++){
		if (tos_sort_dd.lv.sl[i].Data[0]==(long)eslTOS_Sort){
			if (tos_sort_dd.lv.sl[i].Data[1]==(long)eslTOS_Descend){
				Found=true;
				tos_sort_dd.sel=i;
				break;
			}
		}
	}
	if (Found==0){
		tos_sort_dd.sel=0;
		eslTOS_Sort=eslSortByData0;
		eslTOS_Descend=0;
	}
	tos_sort_dd.create(XD,page_p,page_l+label->w+5,y,page_w-(label->w+5),200,
									dd_notify_proc,this);
  tos_sort_dd.id=1020;
  y+=35;

  tos_lv.id=1000;
  tos_lv.create(XD,page_p,page_l,y,page_w,tosbox_h,listview_notify_proc,this);
  y+=tosbox_h+5;


	tosadd_but.create(XD,page_p,page_l,y,page_w/2-5,25,button_notify_proc,this,
            BT_TEXT,T("Add To List"),1010,hxc::col_bk);

  tosrefresh_but.create(XD,page_p,page_l+page_w/2+5,y,page_w/2-5,25,button_notify_proc,this,
            BT_TEXT,T("Refresh"),1011,hxc::col_bk);
  y+=35;

  mustreset_td.text=T("TOS changes don't take effect until the next cold reset of the ST");
  mustreset_td.sy=0;
  mustreset_td.wordwrapped=false;
	mustreset_td.create(XD,page_p,page_l,y,page_w,45,hxc::col_white,0);
  y+=55;

  coldreset_but.create(XD,page_p,page_l,y,page_w,25,button_notify_proc,this,
            BT_TEXT,T("Perform Cold Reset Now"),1000,hxc::col_bk);



  TOSRefreshBox();

#endif//UNIX
}


void TOptionBox::CreateGeneralPage() {
  int y=10;

#ifdef WIN32
  HWND Win;
  int Wid;
  CreateWindow("Static","",WS_CHILD|SS_CENTER,page_l,y,page_w,20,Handle,
    (HMENU)1040,HInstance,NULL);
  y+=20;
  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD|WS_TABSTOP|TBS_HORZ,
    page_l,y,page_w,28,Handle,(HMENU)1041,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,190)); // Each tick worth 5
  SendMessage(Win,TBM_SETPOS,1,((100000/run_speed_ticks_per_second)-50)/5);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  for(int n=0;n<190;n+=10) 
    SendMessage(Win,TBM_SETTIC,0,n);
  SendMessage(Win,TBM_SETPAGESIZE,0,5);
  SendMessage(Handle,WM_HSCROLL,0,LPARAM(Win));
  y+=35;
  CreateWindow("Static",T("Slow motion speed")+": "+(slow_motion_speed/10)+"%",
    WS_CHILD|SS_CENTER,page_l,y,page_w,20,Handle,(HMENU)1000,HInstance,NULL);
  y+=20;
  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD|WS_TABSTOP|TBS_HORZ,
    page_l,y,page_w,28,Handle,(HMENU)1001,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,79));
  SendMessage(Win,TBM_SETPOS,1,(slow_motion_speed-10)/10);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  for (int n=4;n<79;n+=5) SendMessage(Win,TBM_SETTIC,0,n);
  SendMessage(Win,TBM_SETPAGESIZE,0,10);
  y+=35;
  CreateWindow("Static","",WS_CHILD | SS_CENTER,page_l,y,page_w,20,Handle,
    (HMENU)1010,HInstance,NULL);
  y+=20;
  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD|WS_TABSTOP|TBS_HORZ|TBS_AUTOTICKS,
    page_l,y,page_w,28,Handle,(HMENU)1011,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,18));
  SendMessage(Win,TBM_SETPOS,1,(1000/MAX(fast_forward_max_speed,50))-2);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  SendMessage(Win,TBM_SETTICFREQ,1,0);
  SendMessage(Win,TBM_SETPAGESIZE,0,3);
  SendMessage(Handle,WM_HSCROLL,0,LPARAM(Win));
  y+=35;
  int mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX;
  Wid=GetCheckBoxSize(Font,T("Show pop-up hints")).Width;
  Win=CreateWindow("Button",T("Show pop-up hints"),mask,page_l,y,Wid,25,Handle,
    (HMENU)400,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,ShowTips,0);
  y+=LineHeight;
  ADVANCED_BEGIN
    Wid=GetCheckBoxSize(Font,T("Make Steem high priority")).Width;
    Win=CreateWindow("Button",T("Make Steem high priority"),mask,page_l,y,Wid,25,
      Handle,(HMENU)1030,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,HighPriority,0);
    ToolAddWindow(ToolTip,Win,T("When this option is ticked Steem will get first use of the CPU ahead of other applications, this means Steem will still run smoothly even if you start doing something else at the same time, but everything else will run slower."));
    y+=LineHeight;
  ADVANCED_END
#if defined(SSE_EMU_THREAD)
  ADVANCED_BEGIN
    Wid=GetCheckBoxSize(Font,T("Emulation thread")).Width;
    Win=CreateWindow("Button",T("Emulation thread"),
      mask | ((runstate!=RUNSTATE_STOPPED)?WS_DISABLED:0),
      page_l,y,Wid,25,Handle,(HMENU)IDC_EMU_THREAD,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,OPTION_EMUTHREAD,0);
    ToolAddWindow(ToolTip,Win,
#ifdef DEBUG_BUILD
      T("Debug build: Not recommended! Always stop emulation before messing with Debugger windows!"));
#else
      T("This could make Steem more responsive. A bit experimental."));
#endif
    y+=LineHeight;
  ADVANCED_END
#endif
  Wid=GetCheckBoxSize(Font,T("Pause emulation when inactive")).Width;
  Win=CreateWindow("Button",T("Pause emulation when inactive"),mask,page_l,y,
    Wid,25,Handle,(HMENU)800,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,PauseWhenInactive,0);
  y+=LineHeight;
  Wid=GetCheckBoxSize(Font,T("Disable system keys when running")).Width;
  Win=CreateWindow("Button",T("Disable system keys when running"),mask,page_l,y,
    Wid,25,Handle,(HMENU)700,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,(AllowTaskSwitch==0),0);
  ToolAddWindow(ToolTip,Win,T("When this option is ticked Steem will disable the Alt-Tab, Alt-Esc and Ctrl-Esc key combinations when it is running, this allows the ST to receive those keys. This option doesn't work in fullscreen mode."));
  y+=LineHeight;
#if 0 // moved to disk manager
  ADVANCED_BEGIN
    Wid=GetCheckBoxSize(Font,T("Automatic fast forward on disk access")).Width;
    Win=CreateWindow("Button",T("Automatic fast forward on disk access"),WS_CHILD|
      WS_TABSTOP|BS_CHECKBOX,page_l,y,Wid,25,Handle,(HMENU)900,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,floppy_access_ff,0);
    y+=LineHeight;
  ADVANCED_END
#endif
  Wid=GetCheckBoxSize(Font,T("Start emulation on mouse click")).Width;
  Win=CreateWindow("Button",T("Start emulation on mouse click"),mask,page_l,y,
    Wid,25,Handle,(HMENU)901,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,StartEmuOnClick,0);
  ToolAddWindow(ToolTip,Win,T("When this option is ticked clicking a mouse button on Steem's main window will start emulation."));
#endif//WIN32

#ifdef UNIX
  RunSpeedLabel.create(XD,page_p,page_l,y,page_w,25,NULL,this,
      BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_CENTRE,"",0,BkCol);
  y+=25;

  RunSpeedSB.horizontal=true;
  RunSpeedSB.init(189+10,10,((100000/run_speed_ticks_per_second)-50) / 5); // 1 tick per 5%
  RunSpeedSB.create(XD,page_p,page_l,y,page_w,25,scrollbar_notify_proc,this);
  RunSpeedSB.id=2;
  y+=35;

  SMSpeedLabel.create(XD,page_p,page_l,y,page_w,25,NULL,this,
      BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_CENTRE,"",0,BkCol);
  y+=25;

  SMSpeedSB.horizontal=true;
  SMSpeedSB.init(79+5,5,(slow_motion_speed-10)/10);
  SMSpeedSB.create(XD,page_p,page_l,y,page_w,25,scrollbar_notify_proc,this);
  SMSpeedSB.id=0;
  y+=35;

  FFMaxSpeedLabel.create(XD,page_p,page_l,y,page_w,25,NULL,this,
        BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_CENTRE,"",0,BkCol);
  y+=25;

  FFMaxSpeedSB.horizontal=true;
  FFMaxSpeedSB.init(18+4,4,(1000/MAX(fast_forward_max_speed,50))-2);
  FFMaxSpeedSB.create(XD,page_p,page_l,y,page_w,25,scrollbar_notify_proc,this);
  FFMaxSpeedSB.id=1;
  y+=35;
  scrollbar_notify_proc(&SMSpeedSB,SBN_SCROLL,SMSpeedSB.pos);

  hxc_button *p_but=new hxc_button(XD,page_p,page_l,y,0,25,button_notify_proc,this,
        BT_CHECKBOX,T("Show pop-up hints"),121,BkCol);
  p_but->set_check(ShowTips);
  y+=35;

  high_priority_but.create(XD,page_p,page_l,y,0,25,button_notify_proc,this,
        BT_CHECKBOX,T("Make Steem high priority"),120,BkCol);
  high_priority_but.set_check(HighPriority);
  hints.add(high_priority_but.handle,T("When this option is ticked Steem will get first use of the CPU ahead of other applications, this means Steem will still run smoothly even if you start doing something else at the same time, but everything else will run slower."),
              page_p);
  y+=35;

  pause_inactive_but.create(XD,page_p,page_l,y,0,25,button_notify_proc,this,
        BT_CHECKBOX,T("Pause emulation when inactive"),110,BkCol);
  pause_inactive_but.set_check(PauseWhenInactive);
  y+=35;

  ff_on_fdc_but.create(XD,page_p,page_l,y,0,25,
          button_notify_proc,this,BT_CHECKBOX,
          T("Automatic fast forward on disk access"),130,BkCol);
  ff_on_fdc_but.set_check(floppy_access_ff);
  y+=35;

  start_click_but.create(XD,page_p,page_l,y,0,25,
          button_notify_proc,this,BT_CHECKBOX,
          T("Start emulation on mouse click"),140,BkCol);
  start_click_but.set_check(StartEmuOnClick);
  hints.add(start_click_but.handle,T("When this option is ticked clicking a mouse button on Steem's main window will start emulation."),
              page_p);

  
#endif//UNIX
}


void TOptionBox::CreatePortsPage() {
  int y=10;
#ifdef WIN32
  HWND Win;
  int Wid;
  
#if defined(SSE_DONGLE_PORT)
  int GroupHeight=(OPTIONS_HEIGHT-10)/3-10-15;
  int nPorts=(OPTION_ADVANCED)?4:3;
#else
  int GroupHeight=(OPTIONS_HEIGHT-10)/3-10;
  const int nPorts=3;
#endif
  int GroupMiddle=20+30+(GroupHeight-20-30)/2;
  for(int p=0;p<nPorts;p++)
  {
    if(p==3)
      GroupHeight-=45;
    HWND CtrlParent;
    INT_PTR base=9000+p*100;
    CtrlParent=CreateWindowEx(WS_EX_CONTROLPARENT,"Button",STPort[p].Name,WS_CHILD|
      BS_GROUPBOX,page_l,y,page_w,GroupHeight,Handle,HMENU(base),HInstance,NULL);
    SetWindowLongPtr(CtrlParent, GWLP_USERDATA,(LONG_PTR)this);
    Old_GroupBox_WndProc=(WNDPROC)SetWindowLongPtr(CtrlParent,GWLP_WNDPROC,
      (LONG_PTR)GroupBox_WndProc);
    y+=GroupHeight;
    Wid=get_text_width(T("Connect to"));
    Win=CreateWindow("Combobox","",WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL
      |CBS_DROPDOWNLIST,15+Wid,20,page_w-10-(15+Wid),200,CtrlParent,
      HMENU(base+2),HInstance,NULL);
    CreateWindow("Static",T("Connect to"),WS_CHILD|WS_VISIBLE,10,24,Wid,23,
      CtrlParent, HMENU(base+1),HInstance,NULL);
    CBAddString(Win,T("None"),PORTTYPE_NONE);
#if defined(SSE_DONGLE_PORT)
    if(p==3)
    {
#if defined(SSE_DONGLE_LEADERBOARD)
      CBAddString(Win,T("10th Frame dongle"),TDongle::TENTHFRAME);
#endif
#if defined(SSE_DONGLE_BAT2)
      CBAddString(Win,T("B.A.T II dongle"),TDongle::BAT2);
#endif
#if defined(SSE_DONGLE_CRICKET)
      CBAddString(Win,T("Cricket Captain dongle"),TDongle::CRICKET);
#endif
#if defined(SSE_DONGLE_LEADERBOARD)
      CBAddString(Win,T("Leader Board dongle"),TDongle::LEADERBOARD);
#endif
#if defined(SSE_DONGLE_JEANNEDARC)
      CBAddString(Win,T("Jeanne d'Arc dongle"),TDongle::JEANNEDARC);
#endif
#if defined(SSE_DONGLE_CRICKET)
      CBAddString(Win,T("Rugby Coach dongle"),TDongle::RUGBY);
#endif
#if defined(SSE_DONGLE_CRICKET)
      CBAddString(Win,T("Multi Player Soccer Manager dongle"),TDongle::SOCCER);
#endif
#if defined(SSE_DONGLE_MUSIC_MASTER)
      CBAddString(Win,T("Music Master dongle"),TDongle::MUSIC_MASTER);
#endif
#if defined(SSE_DONGLE_PROSOUND)
      CBAddString(Win,T("Pro Sound Designer (WOD/LXS)"),TDongle::PROSOUND);
#endif
#if defined(SSE_DONGLE_MULTIFACE)
      CBAddString(Win,T("Multiface Cartridge switch"),TDongle::MULTIFACE);
#endif
#if defined(SSE_DONGLE_URC)
      CBAddString(Win,T("Ultimate Ripper Cartridge switch"),TDongle::URC);
#endif
    }
    else
#endif
    {
      CBAddString(Win,T("MIDI Device"),PORTTYPE_MIDI);
      if(AllowLPT)
        CBAddString(Win,T("Parallel Port (LPT)"),PORTTYPE_PARALLEL);
      if(AllowCOM)
        CBAddString(Win,T("COM Port"),PORTTYPE_COM);
      CBAddString(Win,T("File"),PORTTYPE_FILE);
      CBAddString(Win,T("Loopback (Output->Input)"),PORTTYPE_LOOP);
    }
    if(CBSelectItemWithData(Win,STPort[p].Type)<0)
      SendMessage(Win,CB_SETCURSEL,0,0);
    // MIDI
    Wid=get_text_width(T("Output device"));
#if defined(SSE_DONGLE_PORT)
    CreateWindow("Static",T("Output device"),WS_CHILD,
      10,54-5,Wid,23,CtrlParent,HMENU(base+10),HInstance,NULL);
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,
      15+Wid,50-5,page_w-10-(15+Wid),200,CtrlParent,HMENU(base+11),HInstance,NULL);
#else
    CreateWindow("Static",T("Output device"),WS_CHILD,
      10,54,Wid,23,CtrlParent,HMENU(base+10),HInstance,NULL);
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,
      15+Wid,50,page_w-10-(15+Wid),200,CtrlParent,HMENU(base+11),HInstance,NULL);
#endif
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("None"));
    int c=midiOutGetNumDevs();
    MIDIOUTCAPS moc;
    ZeroMemory(&moc,sizeof(MIDIOUTCAPS)); //just in case really
    // MIDI_MAPPER is (UINT)-1 = $FFFFFFFF, same in VS2015, bad for x64 build
    for(INT_PTR n=-1;n<c;n++) // use -1 instead of MIDI_MAPPER
    {
      midiOutGetDevCaps(n,&moc,sizeof(MIDIOUTCAPS));
      SendMessage(Win,CB_ADDSTRING,0,(LPARAM)moc.szPname);
    }
    SendMessage(Win,CB_SETCURSEL,STPort[p].MIDIOutDevice+2,0);
    Wid=get_text_width(T("Input device"));
#if defined(SSE_DONGLE_PORT)
    CreateWindow("Static",T("Input device"),WS_CHILD,
      10,80+4-10,Wid,23,CtrlParent,HMENU(base+12),HInstance,NULL);
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,
      15+Wid,80-10,page_w-10-(15+Wid),200,CtrlParent,HMENU(base+13),HInstance,NULL);
#else
    CreateWindow("Static",T("Input device"),WS_CHILD,
      10,80+4,Wid,23,CtrlParent,HMENU(base+12),HInstance,NULL);
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,
      15+Wid,80,page_w-10-(15+Wid),200,CtrlParent,HMENU(base+13),HInstance,NULL);
#endif
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("None"));
    c=midiInGetNumDevs();
    MIDIINCAPS mic;
    ZeroMemory(&mic,sizeof(MIDIINCAPS));
    for(INT_PTR n=0;n<c;n++)
    {
      midiInGetDevCaps(n,&mic,sizeof(MIDIINCAPS));
      SendMessage(Win,CB_ADDSTRING,0,(LPARAM)mic.szPname);
    }
    SendMessage(Win,CB_SETCURSEL,STPort[p].MIDIInDevice+1,0);
    //Parallel
    Wid=get_text_width(T("Select port"));
#if defined(SSE_DONGLE_PORT)
    CreateWindow("Static",T("Select port"),WS_CHILD,
      page_w/2-(Wid+105)/2,GroupMiddle-15+4-5,Wid,23,CtrlParent,HMENU(base+20),HInstance,NULL);
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
      page_w/2-(Wid+105)/2+Wid+5,GroupMiddle-15-5,100,200,CtrlParent,HMENU(base+21),HInstance,NULL);
#else
    CreateWindow("Static",T("Select port"),WS_CHILD,
      page_w/2-(Wid+105)/2,GroupMiddle-15+4,Wid,23,CtrlParent,HMENU(base+20),HInstance,NULL);
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
      page_w/2-(Wid+105)/2+Wid+5,GroupMiddle-15,100,200,CtrlParent,HMENU(base+21),HInstance,NULL);
#endif
    for(int n=1;n<10;n++) 
      SendMessage(Win,CB_ADDSTRING,0,(LPARAM)((EasyStr("LPT")+n).Text));
    SendMessage(Win,CB_SETCURSEL,STPort[p].LPTNum,0);
    //COM
    Wid=get_text_width(T("Select port"));
#if defined(SSE_DONGLE_PORT)
    CreateWindow("Static",T("Select port"),WS_CHILD,
      page_w/2-(Wid+105)/2,GroupMiddle-15+4-5,Wid,23,CtrlParent,HMENU(base+30),HInstance,NULL);
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
      page_w/2-(Wid+105)/2+Wid+5,GroupMiddle-15-5,100,200,CtrlParent,HMENU(base+31),HInstance,NULL);
#else
    CreateWindow("Static",T("Select port"),WS_CHILD,
      page_w/2-(Wid+105)/2,GroupMiddle-15+4,Wid,23,CtrlParent,HMENU(base+30),HInstance,NULL);
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
      page_w/2-(Wid+105)/2+Wid+5,GroupMiddle-15,100,200,CtrlParent,HMENU(base+31),HInstance,NULL);
#endif
    for(int n=1;n<10;n++) 
      SendMessage(Win,CB_ADDSTRING,0,(LPARAM)((EasyStr("COM")+n).Text));
    SendMessage(Win,CB_SETCURSEL,STPort[p].COMNum,0);
    //File
#if defined(SSE_DONGLE_PORT)
    CreateWindowEx(512,"Steem Path Display",STPort[p].File,WS_CHILD,
      10,GroupMiddle-30-2,page_w-20,22,CtrlParent,HMENU(base+40),HInstance,NULL);
    CreateWindow("Button",T("Change File"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
      10,GroupMiddle-5-2,page_w/2-15,23,CtrlParent,HMENU(base+41),HInstance,NULL);
    CreateWindow("Button",T("Reset Current File"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
      page_w/2+5,GroupMiddle-5-2,page_w/2-15,23,CtrlParent,HMENU(base+42),HInstance,NULL);
#else
    CreateWindowEx(512,"Steem Path Display",STPort[p].File,WS_CHILD,
      10,GroupMiddle-30,page_w-20,22,CtrlParent,HMENU(base+40),HInstance,NULL);
    CreateWindow("Button",T("Change File"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
      10,GroupMiddle,page_w/2-15,23,CtrlParent,HMENU(base+41),HInstance,NULL);
    CreateWindow("Button",T("Reset Current File"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
      page_w/2+5,GroupMiddle,page_w/2-15,23,CtrlParent,HMENU(base+42),HInstance,NULL);
#endif
    // Disabled (parallel only)
    if(p==1) 
    {
#if defined(SSE_DONGLE_PORT)
      CreateWindow("Steem Path Display",T("Disabled due to parallel joystick"),
        WS_CHILD|PDS_VCENTRESTATIC,
        10,20,page_w-20,GroupHeight-30-5,CtrlParent,HMENU(99),HInstance,NULL);
#else
      CreateWindow("Steem Path Display",T("Disabled due to parallel joystick"),
        WS_CHILD|PDS_VCENTRESTATIC,
        10,20,page_w-20,GroupHeight-30,CtrlParent,HMENU(99),HInstance,NULL);
#endif
    }
    SetWindowAndChildrensFont(CtrlParent,Font);
  }
  for(int p=0;p<3;p++)
    PortsMakeTypeVisible(p);
#endif//WIN32

#ifdef UNIX
	PortGroup[0].create(XD,page_p,page_l,y,page_w,25+90+5,NULL,this,BT_GROUPBOX,
												T("MIDI Port"),0,BkCol);
	y+=25+90+5+10;
	PortGroup[1].create(XD,page_p,page_l,y,page_w,25+90+5,NULL,this,BT_GROUPBOX,
												T("Parallel Port"),0,BkCol);
	y+=25+90+5+10;
	PortGroup[2].create(XD,page_p,page_l,y,page_w,25+90+5,NULL,this,BT_GROUPBOX,
												T("Serial Port"),0,BkCol);

	for (int p=0;p<3;p++){
    int IDBase=1200+p*20;
		y=25;
		ConnectLabel[p].create(XD,PortGroup[p].handle,10,y,0,25,NULL,this,BT_LABEL,
														T("Connect to"),0,BkCol);

		ConnectDD[p].make_empty();
    ConnectDD[p].additem(T("None"),PORTTYPE_NONE);
    ConnectDD[p].additem(T("MIDI Port Device"),PORTTYPE_MIDI);
//    ConnectDD[p].additem(T("MIDI Sequencer Device"),PORTTYPE_UNIX_SEQUENCER);
    if (AllowLPT) ConnectDD[p].additem(T("Parallel Port Device"),PORTTYPE_PARALLEL);
    if (AllowCOM) ConnectDD[p].additem(T("Serial Port Device"),PORTTYPE_COM);
    ConnectDD[p].additem(T("Named Pipes"),PORTTYPE_LAN);
    ConnectDD[p].additem(T("Other Device"),PORTTYPE_UNIX_OTHER);
    ConnectDD[p].additem(T("File"),PORTTYPE_FILE);
    ConnectDD[p].additem(T("Loopback (Output->Input)"),PORTTYPE_LOOP);
		ConnectDD[p].select_item_by_data(STPort[p].Type);
		ConnectDD[p].grandfather=page_p;
		ConnectDD[p].id=IDBase+0;
		ConnectDD[p].create(XD,PortGroup[p].handle,15+ConnectLabel[p].w,
														y,page_w-10-(15+ConnectLabel[p].w),200,dd_notify_proc,this);
		y+=LineHeight;

    //---------------------------------------------------------------------------
		IOGroup[p].create(XD,PortGroup[p].handle,10,y,PortGroup[p].w-20,60,NULL,this,
														BT_STATIC,"",0,BkCol);

		IOChooseBut[p].create(XD,IOGroup[p].handle,IOGroup[p].w,0,0,25,
												button_notify_proc,this,BT_TEXT,T("Choose"),IDBase+1,BkCol);
		IOChooseBut[p].x-=IOChooseBut[p].w;
		XMoveResizeWindow(XD,IOChooseBut[p].handle,IOChooseBut[p].x,IOChooseBut[p].y,
																IOChooseBut[p].w,IOChooseBut[p].h);

		IODevEd[p].create(XD,IOGroup[p].handle,0,0,IOChooseBut[p].x-10,25,edit_notify_proc,this);
		IODevEd[p].id=IDBase+2;

		IOAllowIOBut[p][0].create(XD,IOGroup[p].handle,0,30,0,25,
												button_notify_proc,this,BT_CHECKBOX,
												T("Output"),IDBase+3,BkCol);

		IOAllowIOBut[p][1].create(XD,IOGroup[p].handle,IOGroup[p].w/3,30,
												0,25,button_notify_proc,this,
												BT_CHECKBOX,T("Input"),IDBase+4,BkCol);

		IOOpenBut[p].create(XD,IOGroup[p].handle,(IOGroup[p].w/3)*2,30,
												(IOGroup[p].w/3),25,button_notify_proc,this,
												BT_TEXT,T("Open"),IDBase+5,BkCol);

    //---------------------------------------------------------------------------
		LANGroup[p].create(XD,PortGroup[p].handle,10,y,PortGroup[p].w-20,60,NULL,this,
														BT_STATIC,"",0,BkCol);

		hxc_button *p_but=new hxc_button(XD,LANGroup[p].handle,LANGroup[p].w,0,0,55,
												button_notify_proc,this,BT_TEXT,T("Open"),IDBase+14,BkCol);
		p_but->x-=p_but->w;
		XMoveResizeWindow(XD,p_but->handle,p_but->x,p_but->y,p_but->w,p_but->h);
    int lan_wid=p_but->x-5;

		p_but=new hxc_button(XD,LANGroup[p].handle,lan_wid,0,0,25,
												button_notify_proc,this,BT_TEXT,T("Choose"),IDBase+11,BkCol);
		p_but->x-=p_but->w;
		XMoveResizeWindow(XD,p_but->handle,p_but->x,p_but->y,p_but->w,p_but->h);

		hxc_button *p_lab=new hxc_button(XD,LANGroup[p].handle,0,0,0,25,NULL,this,
                                    BT_LABEL,T("Output"),0,BkCol);

		hxc_edit *p_ed=new hxc_edit(XD,LANGroup[p].handle,p_lab->w+5,0,p_but->x-5-(p_lab->w+5),25,
                                  edit_notify_proc,this);
    p_ed->id=IDBase+10;

		p_but=new hxc_button(XD,LANGroup[p].handle,lan_wid,30,0,25,
												button_notify_proc,this,BT_TEXT,T("Choose"),IDBase+13,BkCol);
		p_but->x-=p_but->w;
		XMoveResizeWindow(XD,p_but->handle,p_but->x,p_but->y,p_but->w,p_but->h);

		p_lab=new hxc_button(XD,LANGroup[p].handle,0,30,0,25,NULL,this,
                                    BT_LABEL,T("Input"),0,BkCol);

		p_ed=new hxc_edit(XD,LANGroup[p].handle,p_lab->w+5,30,p_but->x-5-(p_lab->w+5),25,
                                  edit_notify_proc,this);
    p_ed->id=IDBase+12;

    //---------------------------------------------------------------------------
		FileGroup[p].create(XD,PortGroup[p].handle,10,y,PortGroup[p].w-20,60,NULL,this,
														BT_STATIC,"",0,BkCol);



		FileDisplay[p].create(XD,FileGroup[p].handle,0,0,FileGroup[p].w,25,
												NULL,this,BT_TEXT | BT_BORDER_INDENT | BT_STATIC | BT_TEXT_PATH,
                        STPort[p].File,0,WhiteCol);

		FileChooseBut[p].create(XD,FileGroup[p].handle,0,30,FileGroup[p].w/2-5,25,
												button_notify_proc,this,BT_TEXT,T("Choose"),IDBase+6,BkCol);

		FileEmptyBut[p].create(XD,FileGroup[p].handle,FileGroup[p].w/2+5,30,FileGroup[p].w/2-5,25,
												button_notify_proc,this,BT_TEXT,T("Empty"),IDBase+7,BkCol);

		UpdatePortDisplay(p);
	}

  

#endif//UNIX
}


void TOptionBox::FullscreenBrightnessBitmap() {

#ifdef WIN32
  int w=GuiSM.cx_screen(),h=GuiSM.cy_screen();
  WNDCLASS wc;
  wc.style=CS_DBLCLKS;
  wc.lpfnWndProc=Fullscreen_WndProc;
  wc.cbClsExtra=0;
  wc.cbWndExtra=0;
  wc.hInstance=HInstance;
  wc.hIcon=NULL;
  wc.hCursor=LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground=NULL;
  wc.lpszMenuName=NULL;
  wc.lpszClassName="Steem Temp Fullscreen Window";
  RegisterClass(&wc);
  HWND Win=CreateWindow("Steem Temp Fullscreen Window","",0,
    0,0,w,h,Handle,NULL,HInstance,NULL);
  SetWindowLong(Win,GWL_STYLE,0);
  HDC ScrDC=GetDC(NULL);
  HBITMAP hBmp=CreateCompatibleBitmap(ScrDC,w,h);
  ReleaseDC(NULL,ScrDC);
  DrawBrightnessBitmap(hBmp);
  SetProp(Win,"Bitmap",hBmp);
  ShowWindow(Win,SW_SHOW);
  SetWindowPos(Win,HWND_TOPMOST,0,0,w,h,0);
  UpdateWindow(Win);
  bool DoneMouseUp=0;
  MSG mess;
  for(;;)
  {
    PeekMessage(&mess,Win,0,0,PM_REMOVE);
    DispatchMessage(&mess);
    short MouseBut=(GetKeyState(VK_LBUTTON)|GetKeyState(VK_RBUTTON)
      |GetKeyState(VK_MBUTTON));
    if(MouseBut>=0) 
      DoneMouseUp=true;
    if(MouseBut<0&&DoneMouseUp) 
      break;
  }
  RemoveProp(Win,"Bitmap");
  DestroyWindow(Win);
  DeleteObject(hBmp);
  UnregisterClass("Steem Temp Fullscreen Window",HInstance);
#endif

#ifdef UNIX
  int sw=XDisplayWidth(XD,XDefaultScreen(XD));
  int sh=XDisplayHeight(XD,XDefaultScreen(XD));

  XSetWindowAttributes swa;
  swa.backing_store=NotUseful;
  swa.override_redirect=True;
  Window handle=XCreateWindow(XD,XDefaultRootWindow(XD),0,0,sw,sh,0,
                           CopyFromParent,InputOutput,CopyFromParent,
                           CWBackingStore | CWOverrideRedirect,&swa);

  SetProp(XD,handle,cWinProc,(DWORD)WinProc);
  SetProp(XD,handle,cWinThis,(DWORD)this);
  SetProp(XD,handle,hxc::cModal,(DWORD)0xffffffff);

  XSelectInput(XD,handle,KeyPressMask | KeyReleaseMask |
                            ButtonPressMask | ButtonReleaseMask |
                            FocusChangeMask | LeaveWindowMask);

  brightness_image=brightness_ig.NewIconImage(XD,sw,sh);
	DrawBrightnessBitmap(brightness_image);

  hxc_button *p_but=new hxc_button(XD,handle,0,0,sw,sh,NULL,this,
                      BT_STATIC | BT_ICON | BT_BORDER_NONE | BT_NOBACKGROUND,"",0,BkCol);
  p_but->set_icon(&brightness_ig,0);

  XMapWindow(XD,handle);
  XGrabPointer(XD,handle,False,ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync,GrabModeAsync,None,None,CurrentTime);
  XEvent Ev;
  while(1){
    if (hxc::wait_for_event(XD,&Ev)){
      if (Ev.xany.window==handle){
        break;
      }else{
        ProcessEvent(&Ev);
      }
    }
  }
  hxc::destroy_children_of(handle);

  hxc::RemoveProp(XD,handle,cWinProc);
  hxc::RemoveProp(XD,handle,cWinThis);
 	hxc::RemoveProp(XD,handle,hxc::cModal);
  hxc::kill_timer(handle,HXC_TIMER_ALL_IDS);
  XDestroyWindow(XD,handle);

	brightness_image=brightness_ig.NewIconImage(XD,136+136,120);
	DrawBrightnessBitmap(brightness_image);

#endif//UNIX
}


void TOptionBox::CreateBrightnessPage() {

#ifdef WIN32
  int mid=page_l+page_w/2;
  RECT rc={mid-136,12,mid+136,12+160};
  AdjustWindowRectEx(&rc,WS_CHILD|SS_BITMAP,0,512);
  HWND Win=CreateWindowEx(512,"Static","",WS_CHILD|SS_BITMAP|SS_NOTIFY,
    rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,
    Handle,(HMENU)ID_BRIGHTNESS_MAP,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Click to view fullscreen"));
  CreateBrightnessBitmap();
  GetWindowRect(Win,&rc);
  POINT pt={0,0};
  ClientToScreen(Handle,&pt);
  int y=(rc.bottom-pt.y)+5;
  char tmp[30];
  CreateWindow("Static",T("There should be 16 vertical strips (one black)"),
    WS_CHILD|SS_CENTER,page_l,y,page_w,40-20,Handle,(HMENU)2011,HInstance,NULL);
  y+=40-20;
  sprintf(tmp,"Brightness:%d",Brightness);
  CreateWindow("Static",tmp,WS_CHILD|SS_CENTER,
    page_l,y,page_w,25-10,Handle,(HMENU)2000,HInstance,NULL);
  y+=25-10;
  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD|WS_TABSTOP|TBS_HORZ,
    page_l,y,page_w,28-10,Handle,(HMENU)2001,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,256));
  SendMessage(Win,TBM_SETPOS,1,Brightness+128);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  SendMessage(Win,TBM_SETPAGESIZE,0,10);
  SendMessage(Win,TBM_SETTIC,0,128);
  y+=40-20;
  sprintf(tmp,"Contrast:%d",Contrast);
  CreateWindow("Static",tmp,WS_CHILD|SS_CENTER,
    page_l,y,page_w,25-10,Handle,(HMENU)2002,HInstance,NULL);
  y+=25-10;
  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD|WS_TABSTOP|TBS_HORZ,
    page_l,y,page_w,28-10,Handle,(HMENU)2003,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,256));
  SendMessage(Win,TBM_SETPOS,1,Contrast+128);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  SendMessage(Win,TBM_SETPAGESIZE,0,10);
  SendMessage(Win,TBM_SETTIC,0,128);
  for(INT_PTR i=0;i<3;i++)
  {
    y+=40-20;
    sprintf(tmp,"Gamma %s:%d",rgb_txt[i],col_gamma[i]); // red, green, blue
    CreateWindow("Static",tmp,WS_CHILD|SS_CENTER,
      page_l,y,page_w,25-10,Handle,(HMENU)(2004+i*2),HInstance,NULL);
    y+=25-10;
    Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD|WS_TABSTOP|TBS_HORZ,
      page_l,y,page_w,28-10,Handle,(HMENU)(2005+i*2),HInstance,NULL);
    SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(1,256));
    SendMessage(Win,TBM_SETPOS,1,col_gamma[i]+128);
    SendMessage(Win,TBM_SETLINESIZE,0,1);
    SendMessage(Win,TBM_SETPAGESIZE,0,10);
    SendMessage(Win,TBM_SETTIC,0,128);
  }
  CreateWindow("Button",T("Reset"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
    page_l,y+20,50,20,Handle,(HMENU)1025,HInstance,NULL);
#endif

#ifdef UNIX
	int y=10;

  make_palette_table(Brightness,Contrast);
	brightness_image=brightness_ig.NewIconImage(XD,136+136,120);
	DrawBrightnessBitmap(brightness_image);

  brightness_picture.set_icon(&brightness_ig,0);
	brightness_picture.create(XD,page_p,page_l + page_w/2 - 137,y,
														137+137,122,button_notify_proc,this,
                            BT_STATIC | BT_ICON | BT_BORDER_INDENT | BT_NOBACKGROUND,
														"",122,BkCol);
  hints.add(brightness_picture.handle,T("Click to view fullscreen"),page_p);
	y+=125;

  brightness_picture_label.create(XD,page_p,page_l,y,page_w,25,NULL,this,BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_CENTRE,
											  T("There should be 16 vertical strips (one black)"),0,BkCol);
  y+=25;

  brightness_label.create(XD,page_p,page_l,y,page_w,25,NULL,this,BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_CENTRE,"",0,BkCol);
  y+=LineHeight;

  brightness_sb.horizontal=true;
  brightness_sb.init(256+10,10,Brightness+128);

  brightness_sb.create(XD,page_p,page_l,y,page_w,25,scrollbar_notify_proc,this);
  brightness_sb.id=10;
  y+=35;

  contrast_label.create(XD,page_p,page_l,y,page_w,25,NULL,this,
        BT_STATIC | BT_TEXT | BT_BORDER_NONE | BT_TEXT_CENTRE,"",0,BkCol);
  y+=LineHeight;

  contrast_sb.horizontal=true;
  contrast_sb.init(256+10,10,Contrast+128);
  contrast_sb.create(XD,page_p,page_l,y,page_w,25,scrollbar_notify_proc,this);
  contrast_sb.id=11;

  y+=35;

  scrollbar_notify_proc(&contrast_sb,SBN_SCROLL,contrast_sb.pos); // update the label text

  

#endif//UNIX
}


void TOptionBox::CreateDisplayPage() {
  int y=10;

#ifdef WIN32
  HWND Win;
  int Wid,Offset,mask;
  Wid=get_text_width(T("Frameskip"));
  CreateWindow("Static",T("Frameskip"),WS_CHILD,page_l,y+4,Wid,20,Handle,
    (HMENU)200,HInstance,NULL);
  Offset=Wid+HorizontalSeparation;
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
    page_l+Offset,y,130,200,Handle,(HMENU)201,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("None"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Draw 1/2"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Draw 1/3"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Draw 1/4"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Auto"));
  SendMessage(Win,CB_SETCURSEL,MIN(frameskip-1,4),0);
  Win=CreateWindow("Button",T("Reset Video"),WS_CHILD|WS_TABSTOP
    |BS_CHECKBOX|BS_PUSHLIKE,page_l+200,y,90,23,Handle,(HMENU)IDC_RESET_DISPLAY,
    HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("It's hopeless, better restart Steem"));
  y+=LineHeight;
  mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX;
#if defined(SSE_VID_VSYNC_WINDOW)
  ADVANCED_BEGIN
    Win=CreateButton("VSync",1033,0,y,Wid);
     SendMessage(Win,BM_SETCHECK,OPTION_WIN_VSYNC,0);
    ToolAddWindow(ToolTip,Win,
      T("For the window. This can change emulation speed"));
    Offset=Wid+HorizontalSeparation;
  ADVANCED_END
#endif
#if defined(SSE_VID_DD_3BUFFER_WIN) // DirectDraw-only
  ADVANCED_BEGIN
  Win=CreateButton("Triple Buffering",1034,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_3BUFFER_WIN,0);
  ToolAddWindow(ToolTip,Win,T("For the window. High CPU use."));
  ADVANCED_END
#endif
#if defined(SSE_VID_D3D_FLIPEX)
  ADVANCED_BEGIN
  Win=CreateButton("FlipEx",IDC_FLIPEX,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_FLIPEX,0);
  ToolAddWindow(ToolTip,Win,
    T("Need Windows 7, can be useful for monochrome emulation"));
  ADVANCED_END
#endif
  Offset=10;
  y+=LineHeight+10;
  Wid=GetCheckBoxSize(Font,T("Lock window size")).Width;
  Win=CreateWindow("Button",T("Lock window size"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX,
    page_l+Offset,y,Wid,23,Handle,(HMENU)7317,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_BLOCK_RESIZE,0);
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Lock aspect ratio")).Width;
  mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX;
  if(OPTION_BLOCK_RESIZE)
    mask|=WS_DISABLED;
  Win=CreateWindow("Button",T("Lock aspect ratio"),mask,
    page_l+Offset,y,Wid,23,Handle,(HMENU)7318,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_LOCK_ASPECT_RATIO,0);
  y+=10;
  CreateWindow("Button",T("Window Size"),WS_CHILD|BS_GROUPBOX,
    page_l,y-25,page_w,20+30+30+30+30+2+25,Handle,(HMENU)99,HInstance,NULL);
  y+=20;
  Wid=GetCheckBoxSize(Font,T("Automatic resize on resolution change")).Width;
  Win=CreateWindow("Button",T("Automatic resize on resolution change"),WS_CHILD
    |WS_TABSTOP|BS_CHECKBOX, page_l+10,y,Wid,23,Handle,(HMENU)300,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,ResChangeResize,0);
  y+=LineHeight;
  Wid=get_text_width(T("Low resolution"));
  CreateWindow("Static",T("Low resolution"),WS_CHILD,
    page_l+10,y+4,Wid,23,Handle,(HMENU)301,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
    page_l+15+Wid,y,page_w-10-(15+Wid),200,Handle,(HMENU)302,HInstance,NULL);
  CBAddString(Win,T("Small (1:1)"),0);
  CBAddString(Win,T("Double Size")+" - "+T("Stretch"),1);
  CBAddString(Win,T("Double Size")+" - "+T("No Stretch"),MAKELONG(1,DWM_NOSTRETCH));
  CBAddString(Win,T("Treble Size"),2);
  CBAddString(Win,T("Quadruple Size"),3);
  y+=LineHeight;
  Wid=get_text_width(T("Medium resolution"));
  CreateWindow("Static",T("Medium resolution"),WS_CHILD,
    page_l+10,y+4,Wid,23,Handle,(HMENU)303,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
    page_l+15+Wid,y,page_w-10-(15+Wid),200,Handle,(HMENU)304,HInstance,NULL);
  CBAddString(Win,T("Small (1:1)"),0);
  CBAddString(Win,T("Double Height")+" - "+T("Stretch"),1);
  CBAddString(Win,T("Double Height")+" - "+T("No Stretch"),MAKELONG(1,DWM_NOSTRETCH));
  CBAddString(Win,T("Double (2:2)"),2);
  CBAddString(Win,T("Quadruple Height (2:4)"),3);
  y+=LineHeight;
  Wid=get_text_width(T("High resolution"));
  CreateWindow("Static",T("High resolution"),WS_CHILD,
    page_l+10,y+4,Wid,23,Handle,(HMENU)305,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
    page_l+15+Wid,y,page_w-10-(15+Wid),200,Handle,(HMENU)306,HInstance,NULL);
  CBAddString(Win,T("Normal Size"),0);
  CBAddString(Win,T("Double Size"),1);
  y+=LineHeight;
  y+=10;
#if !defined(SSE_VID_NO_FREEIMAGE) //?
  EasyStringList format_sl;
  Disp.ScreenShotGetFormats(&format_sl);
  bool FIAvailable=format_sl.NumStrings>2;
  int h=20+30+30+30+25+3;
  CreateWindow("Button",T("Screenshots"),WS_CHILD|BS_GROUPBOX,
    page_l,y,page_w,h,Handle,(HMENU)99,HInstance,NULL);
  y+=20;
  Wid=get_text_width(T("Folder"));
  CreateWindow("Static",T("Folder"),WS_CHILD,page_l+10,y+4,Wid,23,Handle,
    (HMENU)1020,HInstance,NULL);
  CreateWindowEx(512,"Steem Path Display",ScreenShotFol,WS_CHILD,
    page_l+15+Wid,y,page_w-10-(15+Wid),25,Handle,(HMENU)1021,HInstance,NULL);
  y+=LineHeight;
  CreateWindow("Button",T("Choose"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
    page_l+10,y,(page_w-20)/2-5,23,Handle,(HMENU)1022,HInstance,NULL);
  CreateWindow("Button",T("Open"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
    page_l+10+(page_w-20)/2+5,y,(page_w-20)/2-5,23,Handle,(HMENU)1023,HInstance,
    NULL);
  y+=LineHeight;
  Wid=get_text_width(T("Format"));
  CreateWindow("Static",T("Format"),WS_CHILD,page_l+10,y+4,Wid,23,Handle,
    (HMENU)1050,HInstance,NULL);
  int l=page_l+10+Wid+5;
  if(FIAvailable)
    Wid=(page_w-10-(10+Wid+5))/2-5;
  else
    Wid=page_w-10-(10+Wid+5);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST|WS_VSCROLL,
    l,y,Wid,300,Handle,(HMENU)IDC_SCREENSHOT_FORMAT,HInstance,NULL);
  for(int i=0;i<format_sl.NumStrings;i++)
    CBAddString(Win,format_sl[i].String,format_sl[i].Data[0]);
  INT_PTR n,c=SendMessage(Win,CB_GETCOUNT,0,0);
  for(n=0;n<c;n++)
    if(SendMessage(Win,CB_GETITEMDATA,n,0)==Disp.ScreenShotFormat) 
      break;
  if(n>=c)
  {
    Disp.ScreenShotFormat=FIF_BMP;
#if !defined(SSE_VID_NO_FREEIMAGE)
    Disp.ScreenShotFormatOpts=0;
#endif
    Disp.ScreenShotExt="bmp";
    n=1;    
  }
  SendMessage(Win,CB_SETCURSEL,n,0);
#if !defined(SSE_VID_NO_FREEIMAGE)
  if(FIAvailable)
  {
    CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
      l+Wid+5,y,Wid,200,Handle,(HMENU)1052,HInstance,NULL);
    FillScreenShotFormatOptsCombo();
  }
#endif
  y+=LineHeight;
  Wid=GetCheckBoxSize(Font,T("Minimum size screenshots")).Width;
  Win=CreateWindow("Button",T("Minimum size screenshots"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX,
    page_l+10,y,Wid,23,Handle,(HMENU)1024,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,Disp.ScreenShotMinSize,0);
  ToolAddWindow(ToolTip,Win,T("This option, when checked, ensures all screenshots will be taken at the smallest size possible for the resolution.")+" "+
    T("WARNING: Some video cards may cause the screenshots to look terrible in certain drawing modes."));
#endif //#if !defined(SSE_GUI_NO2SCREENSHOT_SETTINGS)
  UpdateWindowSizeAndBorder();
#endif//WIN32

#ifdef UNIX
  fs_label.create(XD,page_p,page_l,y,0,25,
    NULL,this,BT_LABEL,T("Frameskip"),0,BkCol);

  frameskip_dd.make_empty();
  frameskip_dd.additem(T("Draw Every Frame"));
  frameskip_dd.additem(T("Draw Every Second Frame"));
  frameskip_dd.additem(T("Draw Every Third Frame"));
  frameskip_dd.additem(T("Draw Every Fourth Frame"));
  frameskip_dd.additem(T("Auto Frameskip"));
  frameskip_dd.changesel(MIN(frameskip-1,4));

  frameskip_dd.create(XD,page_p,page_l+5+fs_label.w,y,page_w-(5+fs_label.w),
	  200,dd_notify_proc,this);
  y+=35;

  bo_label.create(XD,page_p,page_l,y,0,25,NULL,this,
    BT_LABEL,T("Borders"),0,BkCol);

  border_dd.make_empty();
#if defined(SSE_VID_DISABLE_AUTOBORDER)
  border_dd.additem(T("Off"));
  border_dd.additem(T("On"));
  //border_dd.additem(T("Normal")); //TODO
  //border_dd.additem(T("Large"));
  //border_dd.additem(T("Max"));
#else
  border_dd.additem(T("Never Show Borders"));
  border_dd.additem(T("Always Show Borders"));
  border_dd.additem(T("Auto Borders"));
#endif
#if defined(SSE_VID_DISABLE_AUTOBORDER)
  border_dd.changesel(MIN((int)border,1));
#elif defined(SSE_BUILD)
  border_dd.changesel(MIN((int)border,2));
#else
  border_dd.changesel(MIN(border,2));
#endif
  border_dd.create(XD,page_p,page_l+5+bo_label.w,y,
    page_w-(5+bo_label.w),210,dd_notify_proc,this);
  y+=35;

  hxc_button *p_but=new hxc_button(XD,page_p,page_l,y,0,25,button_notify_proc,this,
    BT_CHECKBOX,T("Scanline Grille"),210,BkCol);
  p_but->set_check(draw_fs_fx==DFSFX_GRILLE);
  y+=35;

  p_but=new hxc_button(XD,page_p,page_l,y,0,25,button_notify_proc,this,
    BT_CHECKBOX,T("Asynchronous blitting (can be faster)"),220,BkCol);
  p_but->set_check(Disp.DoAsyncBlit);
  y+=35;

  {
    size_group.create(XD,page_p,page_l,y,page_w,120,
      NULL,this,BT_STATIC | BT_TEXT | BT_BORDER_OUTDENT |
    BT_TEXT_VTOP,T("Window Size"),0,BkCol);

    lowres_doublesize_but.create(XD,size_group.handle,10,25,0,25,
            button_notify_proc,this,BT_CHECKBOX,T("Low-res double size"),
            250,BkCol);
    lowres_doublesize_but.set_check(WinSizeForRes[0]);

    medres_doublesize_but.create(XD,size_group.handle,10,55,0,25,
            button_notify_proc,this,BT_CHECKBOX,T("Med-res double height"),
            251,BkCol);
    medres_doublesize_but.set_check(WinSizeForRes[1]);

    hxc_button *p_but=new hxc_button(XD,size_group.handle,10,85,0,25,
            button_notify_proc,this,BT_CHECKBOX,T("Fullscreen 640x400 (never show borders only)"),
            253,BkCol);
    p_but->set_check(prefer_res_640_400);
    hints.add(p_but->handle,T("When this option is ticked Steem will use the 600x400 PC screen resolution in fullscreen if it can"),page_p);

    y+=130;
  }


	screenshots_group.create(XD,page_p,page_l,y,page_w,60,
												NULL,this,BT_STATIC | BT_TEXT | BT_BORDER_OUTDENT |
												BT_TEXT_VTOP,T("Screenshots"),0,BkCol);

  screenshots_fol_label.create(XD,screenshots_group.handle,10,25,0,25,NULL,this,
                    BT_LABEL,T("Folder"),0,BkCol);

  screenshots_fol_but.create(XD,screenshots_group.handle,screenshots_group.w-10,
                    25,0,25,button_notify_proc,this,BT_TEXT,T("Choose"),252,BkCol);
  screenshots_fol_but.x-=screenshots_fol_but.w;
  XMoveWindow(XD,screenshots_fol_but.handle,
              screenshots_fol_but.x,screenshots_fol_but.y);

  screenshots_fol_display.create(XD,screenshots_group.handle,
                    15+screenshots_fol_label.w,25,
                    screenshots_fol_but.x-10-(15+screenshots_fol_label.w),25,NULL,this,
                    BT_STATIC | BT_BORDER_INDENT | BT_TEXT_PATH | BT_TEXT,
                    ScreenShotFol,0,WhiteCol);

  
#endif//UNIX
}


void TOptionBox::CreateStartupPage() {
  int y=10;

#ifdef WIN32
  TConfigStoreFile CSF(globalINIFile);
  bool NoDD=(CSF.GetInt("Options","NoDirectDraw",0)!=0);
  int Wid;
  HWND Win;
  Wid=GetCheckBoxSize(Font,T("Restore previous state")).Width;
  Win=CreateWindow("Button",T("Restore previous state"),WS_CHILD|WS_TABSTOP
    |BS_CHECKBOX,page_l,y,Wid,23,Handle,(HMENU)3303,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,AutoLoadSnapShot,0);
  ToolAddWindow(ToolTip,Win,
    T("When this is checked, Steem saves the state when leaving and loads it\
 when starting. Without a hiccup."));
  y+=LineHeight;
  Wid=get_text_width(T("Filename"));
  CreateWindow("Static",T("Filename"),WS_CHILD,page_l,y+4,Wid,25,Handle,
    (HMENU)3310,HInstance,NULL);
  Win=CreateWindowEx(512,"Edit",AutoSnapShotName,WS_CHILD|WS_TABSTOP
    |ES_AUTOHSCROLL,page_l+Wid+5,y,page_w-(Wid+5),23,Handle,(HMENU)3311,
    HInstance,NULL);
  SendMessage(Win,WM_SETFONT,(UINT_PTR)Font,0);
  SendMessage(Win,EM_LIMITTEXT,100,0);
  INT_PTR Len=SendMessage(Win,WM_GETTEXTLENGTH,0,0);
  SendMessage(Win,EM_SETSEL,Len,Len);
  SendMessage(Win,EM_SCROLLCARET,0,0);
  y+=LineHeight;
  Wid=GetCheckBoxSize(Font,T("Start in fullscreen mode")).Width;
  Win=CreateWindow("Button",T("Start in fullscreen mode"),WS_CHILD|WS_TABSTOP
    |BS_AUTOCHECKBOX|int(NoDD?WS_DISABLED:0),page_l,y,Wid,23,Handle,
    (HMENU)3302,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,GetCSFInt("Options","StartFullscreen",0,globalINIFile),
    0);
  y+=LineHeight;
  ADVANCED_BEGIN
    Wid=GetCheckBoxSize(Font,T("Draw direct to video memory")).Width;
    Win=CreateWindow("Button",T("Draw direct to video memory"),WS_CHILD
      |WS_TABSTOP|BS_AUTOCHECKBOX|int(NoDD?WS_DISABLED:0),page_l,y,Wid,23,
      Handle,(HMENU)3304,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,GetCSFInt("Options","DrawToVidMem",
      Disp.DrawToVidMem,globalINIFile),0);
    ToolAddWindow(ToolTip,Win,
      T("Drawing direct to video memory is generally very fast but in some situations on some PCs it might cause Steem to slow down a lot.")+" "+
      T("If you're having problems with speed try turning this option off and restarting Steem."));
    y+=LineHeight;
    Wid=GetCheckBoxSize(Font,T("Hide mouse pointer when blit")).Width;
    Win=CreateWindow("Button",T("Hide mouse pointer when blit"),WS_CHILD
      |WS_TABSTOP|BS_AUTOCHECKBOX|int(NoDD?WS_DISABLED:0),
      page_l,y,Wid,23,Handle,(HMENU)3305,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,GetCSFInt("Options","BlitHideMouse",
      Disp.BlitHideMouse,globalINIFile),0);
    ToolAddWindow(ToolTip,Win,
      T("On some video cards, it makes a mess if the mouse pointer is over the area where the card is trying to draw.")+" "+
      T("This option, when checked, makes Steem hide the mouse before it draws to the screen.")+" "+
      T("Unfortunately this can make the mouse pointer flicker when Steem is running."));
    y+=LineHeight;
#if defined(SSE_VID_D3D)
    Wid=GetCheckBoxSize(Font,T("Never use Direct3D")).Width;
    Win=CreateWindow("Button",T("Never use Direct3D"),WS_CHILD|WS_TABSTOP
      |BS_AUTOCHECKBOX,
      page_l,y,Wid,23,Handle,(HMENU)3300,HInstance,NULL);
#else
    Wid=GetCheckBoxSize(Font,T("Never use DirectDraw")).Width;
    Win=CreateWindow("Button",T("Never use DirectDraw"),WS_CHILD|WS_TABSTOP
      |BS_AUTOCHECKBOX,
      page_l,y,Wid,23,Handle,(HMENU)3300,HInstance,NULL);
#endif
    SendMessage(Win,BM_SETCHECK,NoDD,0);
    y+=LineHeight;
#if !defined(SSE_SOUND_NO_NOSOUND_OPTION)
    Win=CreateWindow("Button",T("Never use DirectSound"),WS_CHILD|WS_TABSTOP
      |BS_AUTOCHECKBOX,page_l,y,GetCheckBoxSize(Font,
      T("Never use DirectSound")).Width,20,Handle,(HMENU)3301,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,CSF.GetInt("Options","NoDirectSound",0),0);
    y+=LineHeight;
#endif
#if defined(SSE_SOUND_OPTION_DISABLE_DSP)
    Wid=GetCheckBoxSize(Font,T("Disable DSP")).Width;
    Win=CreateWindow("Button",T("Disable DSP"),WS_CHILD|WS_TABSTOP
      |BS_AUTOCHECKBOX,page_l,y,Wid,23,Handle,(HMENU)3306,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,DSP_DISABLED,0);
    ToolAddWindow(ToolTip,Win,
      T("If you have some odd crashes, checking this may help. DSP code uses the math coprocessor and exceptions are almost impossible to catch"));
    y+=LineHeight;
#endif
  ADVANCED_END
  Wid=GetCheckBoxSize(Font,T("Run on startup")).Width;
  Win=CreateWindow("Button",T("Run on startup"),WS_CHILD|WS_TABSTOP
    |BS_AUTOCHECKBOX,page_l,y,Wid,23,Handle,(HMENU)3308,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,CSF.GetInt("Options","RunOnStart",0),0);
  ToolAddWindow(ToolTip,Win,
    T("No need to press play"));
  y+=LineHeight;
  Wid=GetCheckBoxSize(Font,T(SSE_TRACE_FILE_NAME)).Width;
  Win=CreateWindow("Button",T(SSE_TRACE_FILE_NAME),WS_CHILD|WS_TABSTOP
    |BS_AUTOCHECKBOX,page_l,y,Wid,23,Handle,(HMENU)3307,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,SSEConfig.TraceFile,0);
  ToolAddWindow(ToolTip,Win,
    T("Steem can produce a trace file with some debugging info"));
  CSF.Close();
#endif

#ifdef UNIX
  auto_sts_but.create(XD,page_p,page_l,y,0,25,
          button_notify_proc,this,BT_CHECKBOX,T("Restore previous state"),100,BkCol);
  auto_sts_but.set_check(AutoLoadSnapShot);
  y+=35;

  auto_sts_filename_label.create(XD,page_p,page_l,y,0,25,NULL,this,
                    BT_LABEL,T("Filename"),0,BkCol);

  auto_sts_filename_edit.create(XD,page_p,page_l+5+auto_sts_filename_label.w,y,
  									page_w-(5+auto_sts_filename_label.w),25,edit_notify_proc,this);
  auto_sts_filename_edit.set_text(AutoSnapShotName);
  auto_sts_filename_edit.id=100;

  y+=40;

  no_shm_but.create(XD,page_p,page_l,y,0,25,
              button_notify_proc,this,BT_CHECKBOX,
              T("Never use shared memory extension"),101,BkCol);
  no_shm_but.set_check(GetCSFInt("Options","NoSHM",0,globalINIFile));
  y+=35;

#endif//UNIX
}


void TOptionBox::CreateMacrosPage() {
  int y=10;

#ifdef WIN32
  int Wid,x;
  int ctrl_h=10+20+30+30+10;
  HWND Win;
  DTree.FileMasksESL.DeleteAll();
  DTree.FileMasksESL.Add("",0,RC_ICO_PCFOLDER);
  DTree.FileMasksESL.Add("stmac",0,RC_ICO_OPS_MACROS);
  UpdateDirectoryTreeIcons(&DTree);
  DTree.Create(Handle,page_l,y,page_w,OPTIONS_HEIGHT-ctrl_h-10-30,(HMENU)10000,
    WS_TABSTOP,DTreeNotifyProc,this,MacroDir,T("Macros"));
  y+=OPTIONS_HEIGHT-ctrl_h-30;
  CreateWindow("Button",T("New Macro"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX
    |BS_PUSHLIKE,page_l,y,page_w/2-5,23,Handle,(HMENU)10001,HInstance,NULL);
  CreateWindow("Button",T("Change Store Folder"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX
    |BS_PUSHLIKE,page_l+page_w/2+5,y,page_w/2-5,23,Handle,(HMENU)10002,
    HInstance,NULL);
  y=10+OPTIONS_HEIGHT-ctrl_h;
  CreateWindow("Button",T("Controls"),WS_CHILD|BS_GROUPBOX,page_l,y,page_w,
    ctrl_h-10-10,Handle,(HMENU)10010,HInstance,NULL);
  y+=20;
  x=page_l+10;
  CreateWindow("Steem Flat PicButton",Str(RC_ICO_RECORD),WS_CHILD|WS_TABSTOP,
    x,y,25,25,Handle,(HMENU)10011,HInstance,NULL);
  x+=30;
  CreateWindow("Steem Flat PicButton",Str(RC_ICO_PLAY_BIG),WS_CHILD|WS_TABSTOP,
    x,y,25,25,Handle,(HMENU)10012,HInstance,NULL);
  x+=30;
  ADVANCED_BEGIN
    Wid=get_text_width(T("Mouse speed"));
    CreateWindow("Static",T("Mouse speed"),WS_CHILD,x,y+4,Wid,23,Handle,
      (HMENU)10013,HInstance,NULL);
    x+=Wid+5;
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,
      x,y,page_l+page_w-10-x,400,Handle,(HMENU)10014,HInstance,NULL);
    CBAddString(Win,T("Safe"),15);
    CBAddString(Win,T("Slow"),32);
    CBAddString(Win,T("Medium"),64);
    CBAddString(Win,T("Fast"),96);
    CBAddString(Win,T("V.Fast"),127);
    CBSelectItemWithData(Win,127);
    y+=LineHeight;
    x=page_l+10;
    Wid=get_text_width(T("Playback event delay"));
    CreateWindow("Static",T("Playback event delay"),WS_CHILD,x,y+4,Wid,23,Handle,
      (HMENU)10015,HInstance,NULL);
    x+=Wid+5;
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,
      x,y,page_l+page_w-10-x,400,Handle,(HMENU)10016,HInstance,NULL);
    // Number of VBLs that input is allowed to be the same
    CBAddString(Win,T("As Recorded"),0);
    EasyStr Ms=Str(" ")+T("Milliseconds");
    for(int n=1;n<=25;n++) 
      CBAddString(Win,Str(n*20)+Ms,n);
    CBSelectItemWithData(Win,0); // 'As Recorded' works best with 'C1'
  ADVANCED_END
  DTree.SelectItemByPath(MacroSel);
#endif//WIN32

#ifdef UNIX
  dir_lv.ext_sl.DeleteAll();
  dir_lv.ext_sl.Add(3,T("Parent Directory"),1,1,0);
  dir_lv.ext_sl.Add(3,"",ICO16_FOLDER,ICO16_FOLDERLINK,0);
  dir_lv.ext_sl.Add(3,"stmac",ICO16_MACROS,ICO16_MACROLINK,0);
  dir_lv.lpig=&Ico16;
  dir_lv.base_fol=MacroDir;
  dir_lv.fol=MacroDir;
  dir_lv.allow_type_change=0;
  dir_lv.show_broken_links=0;
  dir_lv.lv.sel=-1;
  if (MacroSel.NotEmpty()){
		dir_lv.fol=MacroSel;
		RemoveFileNameFromPath(dir_lv.fol,REMOVE_SLASH);
	}
  dir_lv.id=2000;
  int dir_lv_h=OPTIONS_HEIGHT-10-10-30-20-30-30;
  dir_lv.create(XD,page_p,page_l,y,page_w,dir_lv_h-5,
                dir_lv_notify_proc,this);
  y+=dir_lv_h;

  hxc_button *p_but,*p_grp;

  new hxc_button(XD,page_p,page_l,y,page_w/2-5,25,button_notify_proc,this,
  										BT_TEXT,T("New Macro"),2001,BkCol);

  new hxc_button(XD,page_p,page_l+page_w/2+5,y,page_w/2-5,25,
                      button_notify_proc,this,BT_TEXT,
                      T("Change Store Folder"),2002,BkCol);
  y+=LineHeight;

  p_grp=new hxc_button(XD,page_p,page_l,y,page_w,20+30+30,NULL,this,
                      BT_GROUPBOX,T("Controls"),2009,BkCol);
  y=20;

  int x=10;
  p_but=new hxc_button(XD,p_grp->handle,x,y,25,25,
                      button_notify_proc,this,BT_ICON,"",2010,BkCol);
  p_but->set_icon(&Ico32,ICO32_RECORD);
  x+=p_but->w+5;

  p_but=new hxc_button(XD,p_grp->handle,x,y,25,25,
                      button_notify_proc,this,BT_ICON,"",2011,BkCol);
  p_but->set_icon(&Ico32,ICO32_PLAY);
  x+=p_but->w+5;

  p_but=new hxc_button(XD,p_grp->handle,x,y,0,25,NULL,this,
                      BT_LABEL,T("Mouse speed"),0,BkCol);
  x+=p_but->w+5;

  hxc_dropdown *p_dd=new hxc_dropdown(XD,p_grp->handle,x,y,
                          page_w-10-x,300,dd_notify_proc,this);
	p_dd->id=2012;
	p_dd->make_empty();
  p_dd->additem(T("Safe"),15);
  p_dd->additem(T("Slow"),32);
  p_dd->additem(T("Medium"),64);
  p_dd->additem(T("Fast"),96);
  p_dd->additem(T("V.Fast"),127);
  p_dd->select_item_by_data(127);
  y+=LineHeight;

  x=10;
  p_but=new hxc_button(XD,p_grp->handle,x,y,0,25,NULL,this,
                      BT_LABEL,T("Playback event delay"),0,BkCol);
  x+=p_but->w+5;

  p_dd=new hxc_dropdown(XD,p_grp->handle,x,y,page_w-10-x,300,
                    dd_notify_proc,this);
	p_dd->id=2013;
	p_dd->make_empty();
  p_dd->additem(T("As Recorded"),0);
  EasyStr Ms=Str(" ")+T("Milliseconds");
  for (int n=1;n<=25;n++) p_dd->additem(Str(n*20)+Ms,n);
  p_dd->select_item_by_data(1);

	if (MacroSel.NotEmpty()){
		dir_lv.select_item_by_name(GetFileNameFromPath(MacroSel));
	}
  UpdateMacroRecordAndPlay();

  

#endif//UNIX
}


void TOptionBox::CreateProfilesPage() {
  int y=10;

#ifdef WIN32
  HWND Win;
  int ctrl_h=OPTIONS_HEIGHT/2-30;
  DTree.FileMasksESL.DeleteAll();
  DTree.FileMasksESL.Add("",0,RC_ICO_PCFOLDER);
  DTree.FileMasksESL.Add("ini",0,RC_ICO_CFG);
  UpdateDirectoryTreeIcons(&DTree);
  DTree.Create(Handle,page_l,y,page_w,OPTIONS_HEIGHT-ctrl_h-40,(HMENU)11000,
    WS_TABSTOP,DTreeNotifyProc,this,ProfileDir,T("Configurations"));
  y+=OPTIONS_HEIGHT-ctrl_h-30;
  CreateWindow("Button",T("Save New Configuration"),WS_CHILD|WS_TABSTOP|
    BS_CHECKBOX|BS_PUSHLIKE,page_l,y,page_w/2-5,23,Handle,(HMENU)11001,
    HInstance,NULL);
  CreateWindow("Button",T("Change Configuration Folder"),WS_CHILD|WS_TABSTOP
    |BS_CHECKBOX|BS_PUSHLIKE,page_l+page_w/2+5,y,page_w/2-5,23,Handle,
    (HMENU)11002,HInstance,NULL);
  y=10+OPTIONS_HEIGHT-ctrl_h;
  CreateWindow("Button",T("Controls"),WS_CHILD|BS_GROUPBOX,page_l,y,page_w,
    ctrl_h-10-10,Handle,(HMENU)11010,HInstance,NULL);
  y+=20;
  CreateWindow("Button",T("Load Configuration"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX
    |BS_PUSHLIKE,page_l+10,y,(page_w-20)/2-5,23,Handle,(HMENU)11011,
    HInstance,NULL);
  CreateWindow("Button",T("Save Over Configuration"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
    page_l+10+(page_w-20)/2+5,y,(page_w-20)/2-5,23,Handle,(HMENU)11012,HInstance,NULL);
  y+=LineHeight;
  Win=CreateWindowEx(512,WC_LISTVIEW,"",WS_CHILD|WS_VISIBLE|WS_TABSTOP
    |WS_DISABLED|LVS_SINGLESEL|LVS_REPORT|LVS_NOCOLUMNHEADER,page_l+10,y,page_w
    -20,OPTIONS_HEIGHT-y-15,Handle,(HMENU)11013,HInstance,NULL);
  ListView_SetExtendedListViewStyle(Win,LVS_EX_CHECKBOXES);
  RECT rc;
  GetClientRect(Win,&rc);
  LV_COLUMN lvc;
  lvc.mask=LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;
  lvc.fmt=LVCFMT_LEFT;
  lvc.cx=rc.right-GuiSM.cx_vscroll();
  lvc.pszText="";
  lvc.iSubItem=0;
  SendMessage(Win,LVM_INSERTCOLUMN,0,LPARAM(&lvc));
  LV_ITEM lvi;
  lvi.mask=LVIF_TEXT|LVIF_PARAM;
  int i=0;
  for(;;) {
    if(ProfileSection[i].Name==NULL) break;
    lvi.iSubItem=0;
    lvi.pszText=StaticT(ProfileSection[i].Name);
    lvi.lParam=DWORD(ProfileSection[i].ID);
    lvi.iItem=i++;
    SendMessage(Win,LVM_INSERTITEM,0,(LPARAM)&lvi);
  }
  DTree.SelectItemByPath(ProfileSel);
#endif

#ifdef UNIX
  dir_lv.ext_sl.DeleteAll();
  dir_lv.ext_sl.Add(3,T("Parent Directory"),1,1,0);
  dir_lv.ext_sl.Add(3,"",ICO16_FOLDER,ICO16_FOLDERLINK,0);
  dir_lv.ext_sl.Add(3,"ini",ICO16_PROFILE,ICO16_PROFILELINK,0);
  dir_lv.lpig=&Ico16;
  dir_lv.base_fol=ProfileDir;
  dir_lv.fol=ProfileDir;
  dir_lv.allow_type_change=0;
  dir_lv.show_broken_links=0;
  dir_lv.lv.sel=-1;
	if (ProfileSel.NotEmpty()){
		dir_lv.fol=ProfileSel;
		RemoveFileNameFromPath(dir_lv.fol,REMOVE_SLASH);
	}
  dir_lv.id=2100;
  int dir_lv_h=OPTIONS_HEIGHT-10-10-30-20-30-130;
  dir_lv.create(XD,page_p,page_l,y,page_w,dir_lv_h-5,
                dir_lv_notify_proc,this);
  y+=dir_lv_h;

  hxc_button *p_grp;

  new hxc_button(XD,page_p,page_l,y,page_w/2-5,25,button_notify_proc,this,
  										BT_TEXT,T("Save New Profile"),2101,BkCol);

  new hxc_button(XD,page_p,page_l+page_w/2+5,y,page_w/2-5,25,
                      button_notify_proc,this,BT_TEXT,
                      T("Change Store Folder"),2102,BkCol);
  y+=LineHeight;

  p_grp=new hxc_button(XD,page_p,page_l,y,page_w,20+30+130,NULL,this,
                      BT_GROUPBOX,T("Controls"),2109,BkCol);
  y=20;
  Window par=p_grp->handle;
  int par_l=10,par_w=page_w-20;

  new hxc_button(XD,par,par_l,y,par_w/2-5,25,button_notify_proc,this,
  										BT_TEXT,T("Load Profile"),2110,BkCol);

  new hxc_button(XD,par,par_l+par_w/2+5,y,par_w/2-5,25,
                      button_notify_proc,this,BT_TEXT,
                      T("Save Over Profile"),2111,BkCol);
  y+=LineHeight;

  profile_sect_lv.lpig=&Ico16;
  profile_sect_lv.display_mode=1;
  profile_sect_lv.checkbox_mode=true;
  profile_sect_lv.id=2112;
  profile_sect_lv.sl.DeleteAll();
  profile_sect_lv.sl.Sort=eslNoSort;
  for (int i=0;ProfileSection[i].Name!=NULL;i++){
    profile_sect_lv.sl.Add(T(ProfileSection[i].Name),0,ProfileSection[i].ID);
  }
  profile_sect_lv.create(XD,par,par_l,y,par_w,125,listview_notify_proc,this);

	if (ProfileSel.NotEmpty()){
		dir_lv.select_item_by_name(GetFileNameFromPath(ProfileSel));
	}
  UpdateProfileDisplay();

  

#endif
}



#ifdef WIN32

// helper compute width and create checkbox control
// todo convert all CreateWindow("Button"...

HWND TOptionBox::CreateButton(EasyStr caption,int hMenu,int X,int Y,int &Wid,
     int nHeight/*=25*/,DWORD dwStyle/*=WS_CHILD|WS_TABSTOP|BS_CHECKBOX*/) {
  Wid=GetCheckBoxSize(Font,caption.Text).Width;
  HWND Win=CreateWindow("Button",caption,dwStyle,page_l+X,Y,Wid,nHeight,Handle,
    (HMENU)hMenu,HInstance,NULL);
  return Win; 
}

// helper
void TOptionBox::CreateResetButton(int y,EasyStr protip) {
  if(protip!=NULL)
  {
    TWidthHeight wh=GetTextSize(Font,protip);
    if(wh.Width>=page_w) wh.Height=(wh.Height+1)*2;
    CreateWindow("Static",protip,WS_CHILD,page_l,y,page_w,wh.Height,Handle,HMENU(8600),
      HInstance,NULL);
    y+=wh.Height+5;
  }
  CreateWindow("Button",T("Perform cold reset now"),WS_CHILD|WS_TABSTOP
    |BS_CHECKBOX|BS_PUSHLIKE,page_l,y,page_w,23,Handle,(HMENU)8601,HInstance,NULL);
}


BOOL CALLBACK TOptionBox::EnumDateFormatsProc(char *DateFormat) {
  USDateFormat=(strchr(DateFormat,'m')<strchr(DateFormat,'d'));
  return 0;
}


void TOptionBox::PortsMakeTypeVisible(int p) {
  int base=9000+p*100;
  HWND CtrlParent=GetDlgItem(Handle,base);
  if(CtrlParent==NULL)
    return;
  bool Disabled=(p==1&&(Joy[N_JOY_PARALLEL_0].ToggleKey
    ||Joy[N_JOY_PARALLEL_1].ToggleKey));
  for(int n=base+10;n<base+100;n++)
    if(GetDlgItem(CtrlParent,n)) 
      ShowWindow(GetDlgItem(CtrlParent,n),SW_HIDE);
  if(Disabled==0)
    for(int n=base+STPort[p].Type*10;n<base+STPort[p].Type*10+10;n++) 
    {
      if(GetDlgItem(CtrlParent,n)) 
        ShowWindow(GetDlgItem(CtrlParent,n),SW_SHOW);
    }
  if(p==1) 
  {
    if(Disabled) 
    {
      ShowWindow(GetDlgItem(CtrlParent,base+1),SW_HIDE);
      ShowWindow(GetDlgItem(CtrlParent,base+2),SW_HIDE);
      ShowWindow(GetDlgItem(CtrlParent,99),SW_SHOW);
    }
    else 
    {
      ShowWindow(GetDlgItem(CtrlParent,99),SW_HIDE);
      ShowWindow(GetDlgItem(CtrlParent,base+1),SW_SHOW);
      ShowWindow(GetDlgItem(CtrlParent,base+2),SW_SHOW);
    }
  }
  // Redraw the groupbox
  RECT rc;
  GetWindowRect(CtrlParent,&rc);
#if defined(SSE_DONGLE_PORT)
  rc.left+=8;rc.right-=8;rc.top+=20+25-5;rc.bottom-=5;
#else
  rc.left+=8;rc.right-=8;rc.top+=20+25;rc.bottom-=5;
#endif
  POINT pt={0,0};
  ClientToScreen(Handle,&pt);
  OffsetRect(&rc,-pt.x,-pt.y);
  InvalidateRect(Handle,&rc,true);
}


LRESULT CALLBACK TOptionBox::GroupBox_WndProc(HWND Win,UINT Mess,WPARAM wPar,
                                              LPARAM lPar) {
  TOptionBox *This=(TOptionBox*)GetWindowLongPtr(Win,GWLP_USERDATA);
  switch(Mess) {
  case WM_COMMAND:
  case WM_HSCROLL:
    return SendMessage(This->Handle,Mess,wPar,lPar);
  }
  return CallWindowProc(This->Old_GroupBox_WndProc,Win,Mess,wPar,lPar);
}


void TOptionBox::DrawBrightnessBitmap(HBITMAP hBmp) {
  if(hBmp==NULL)
    return;
  BITMAP bi;
  GetObject(hBmp,sizeof(BITMAP),&bi);
  int w=bi.bmWidth,h=bi.bmHeight,bpp=bi.bmBitsPixel;
  int text_h=h/8;
  int band_w=w/16;
  int col_h=(h-text_h)/4;
  int BytesPP=(bpp+7)/8;
  BYTE *PicMem=new BYTE[w*h*BytesPP+16];
  ZeroMemory(PicMem,w*h*BytesPP);
  BYTE *pMem=PicMem;
  int pc_pal_start_idx=10+118+(118-65); // End of the second half of the palette
  PALETTEENTRY *pbuf=(PALETTEENTRY*)&logpal[pc_pal_start_idx];
  for(int y=0;y<h-text_h;y++)
  {
    for(int i=0;i<w;i++)
    {
      int red=((i/band_w)>>1)+(((i/band_w)&1)<<3),green=red,blue=red;
      int pal_offset=0;
      if(y>col_h*3)
      {
        green=0,blue=0;
        pal_offset=48;
      }
      else if(y>col_h*2)
      {
        red=0,blue=0;
        pal_offset=32;
      }
      else if(y>col_h)
      {
        red=0,green=0;
        pal_offset=16;
      }
      long Col=palette_table[red|(green<<4)|(blue<<8)];
      switch(BytesPP) {
      case 1:
      {
        int ncol=pal_offset+(i/band_w);
        pbuf[ncol].peFlags=PC_RESERVED;
        pbuf[ncol].peRed=BYTE((Col&0xff0000)>>16);
        pbuf[ncol].peGreen=BYTE((Col&0x00ff00)>>8);
        pbuf[ncol].peBlue=BYTE((Col&0x0000ff));
        *pMem=BYTE(1+pc_pal_start_idx+ncol);
        break;
      }
      case 2:
        *LPWORD(pMem)=WORD(Col);
        break;
      case 3:case 4:
        *LPDWORD(pMem)=DWORD(Col);
        break;
      }
      pMem+=BytesPP;
    }
  }
  SetBitmapBits(hBmp,w*h*BytesPP,PicMem);
  delete[] PicMem;
  if(BytesPP==1)
    AnimatePalette(winpal,pc_pal_start_idx,64,pbuf);
  int gap_w=band_w/4,gap_h=text_h/8;
  HFONT f=MakeFont("Arial",-(text_h-gap_h),band_w/2-gap_w);
  HDC ScrDC=GetDC(NULL);
  HDC BmpDC=CreateCompatibleDC(ScrDC);
  ReleaseDC(NULL,ScrDC);
  SelectObject(BmpDC,hBmp);
  SelectObject(BmpDC,f);
  SetTextColor(BmpDC,RGB(224,224,224));
  SetBkMode(BmpDC,TRANSPARENT);
  for(int i=0;i<16;i++)
    TextOut(BmpDC,i*band_w+(band_w-GetTextSize(f,EasyStr(i+1)).Width)/2,
      h-text_h-1+gap_h/2,EasyStr(i+1),(int)EasyStr(i+1).Length());
  DeleteDC(BmpDC);
  DeleteObject(f);
}


LRESULT CALLBACK TOptionBox::Fullscreen_WndProc(HWND Win,UINT Mess,
                                              WPARAM wPar,LPARAM lPar) {
  if(Mess==WM_PAINT||Mess==WM_NCPAINT)
  {
    HDC WinDC=GetWindowDC(Win);
    HDC BmpDC=CreateCompatibleDC(WinDC);
    SelectObject(BmpDC,GetProp(Win,"Bitmap"));
    BitBlt(WinDC,0,0,GuiSM.cx_screen(),GuiSM.cy_screen(),BmpDC,0,0,SRCCOPY);
    DeleteDC(BmpDC);
    ReleaseDC(Win,WinDC);
    ValidateRect(Win,NULL);
    return 0;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}


void TOptionBox::CreateBrightnessBitmap() {
  if(Handle==NULL)
    return;
  HWND Win=GetDlgItem(Handle,ID_BRIGHTNESS_MAP);
  if(Win==NULL)
    return;
  if(hBrightBmp)
    DeleteObject(hBrightBmp);
  HDC ScrDC=GetDC(NULL);
  hBrightBmp=CreateCompatibleBitmap(ScrDC,136+136,160);
  ReleaseDC(NULL,ScrDC);
  make_palette_table(Brightness,Contrast);
  DrawBrightnessBitmap(hBrightBmp);
  SendMessage(Win,STM_SETIMAGE,IMAGE_BITMAP,LPARAM(hBrightBmp));
}

#endif//WIN32


#ifdef UNIX

void TOptionBox::DrawBrightnessBitmap(XImage *Img)
{
  if (Img==NULL) return;

  int w=Img->width,h=Img->height;

  int band_w=w/16;
  int col_h=h/4;
  int BytesPP=(Img->bits_per_pixel+7)/8;
  ZeroMemory(Img->data,w*h*BytesPP);
  if (BytesPP>1){
    BYTE *pMem=LPBYTE(Img->data);
    for (int y=0;y<h;y++){
      for (int i=0;i<w;i++){
        int r=((i/band_w) >> 1)+(((i/band_w) & 1) << 3),g=r,b=r;
        if (y>col_h*3){
          g=0,b=0;
        }else if (y>col_h*2){
          r=0,b=0;
        }else if (y>col_h){
          r=0,g=0;
        }
        long Col=palette_table[r | (g << 4) | (b << 8)];
        switch (BytesPP){
          case 1:
            *pMem=BYTE(Col);
            break;
          case 2:
            *LPWORD(pMem)=WORD(Col);
            break;
          case 3:case 4:
            *LPDWORD(pMem)=DWORD(Col);
            break;
        }
        pMem+=BytesPP;
      }
    }
  }
}

#endif//UNIX


void TOptionBox::UpdateParallel() {
#ifdef WIN32
  if(Handle)
    PortsMakeTypeVisible(1);
#endif
}


#if !defined(SSE_VID_NO_FREEIMAGE)

void TOptionBox::FillScreenShotFormatOptsCombo() {
  HWND Win=GetDlgItem(Handle,1052);
  if(Win==NULL)
    return;
  EasyStringList sl;
  sl.Sort=eslNoSort;
  Disp.ScreenShotGetFormatOpts(&sl);
  SendMessage(Win,CB_RESETCONTENT,0,0);
  if(sl.NumStrings)
  {
    EnableWindow(Win,true);
    for(int i=0;i<sl.NumStrings;i++)
      CBAddString(Win,sl[i].String,sl[i].Data[0]);
  }
  else
  {
    EnableWindow(Win,0);
    CBAddString(Win,T("Normal"),0);
  }
  if(CBSelectItemWithData(Win,Disp.ScreenShotFormatOpts)<0)
    SendMessage(Win,CB_SETCURSEL,0,0);
}

#endif


void TOptionBox::CreateOSDPage() {
  int y=10;

#ifdef WIN32
  HWND Win;
  int Wid,Wid2;
  Wid=GetCheckBoxSize(Font,T("Disk access light")).Width;
  Win=CreateWindow("Button",T("Disk access light"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX,
    page_l,y,Wid,23,Handle,(HMENU)12000,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,osd_show_disk_light,0);
  Wid2=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Disk drive track info")).Width;
  Win=CreateWindow("Button",T("Disk drive track info"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX,
    page_l+Wid2,y,Wid,23,Handle,(HMENU)12001,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_DRIVE_INFO,0);
  ToolAddWindow(ToolTip,Win,
    T("See what the floppy drives are doing with this option"));
  y+=LineHeight;
  //TRACE("osd_show_plasma %d\n",osd_show_plasma);
  BYTE *p_element[4]={&osd_show_plasma,&osd_show_speed,&osd_show_icons,&osd_show_cpu};
  Str osd_name[4];
  osd_name[0]=T("Logo");
  osd_name[1]=T("Speed bar");
  osd_name[2]=T("State icons");
  osd_name[3]=T("CPU speed indicator");
  for(INT_PTR i=0;i<4;i++)
  {
    Wid=GetTextSize(Font,osd_name[i]).Width;
    CreateWindow("Static",osd_name[i],WS_CHILD,
      page_l,y+4,Wid,23,Handle,(HMENU)0,HInstance,NULL);
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
      page_l+Wid+5,y,page_w-(Wid+5),250,Handle,HMENU(12010+i),HInstance,NULL);
    CBAddString(Win,T("Off"),0);
    CBAddString(Win,Str("1 ")+T("Second"),1); // as seem in ini?
    CBAddString(Win,Str("2 ")+T("Seconds"),2);
    CBAddString(Win,Str("3 ")+T("Seconds"),3);
    CBAddString(Win,Str("4 ")+T("Seconds"),4);
    CBAddString(Win,Str("5 ")+T("Seconds"),5);
    CBAddString(Win,Str("6 ")+T("Seconds"),6);
    CBAddString(Win,Str("8 ")+T("Seconds"),8);
    CBAddString(Win,Str("10 ")+T("Seconds"),10);
    CBAddString(Win,Str("12 ")+T("Seconds"),12);
    CBAddString(Win,Str("15 ")+T("Seconds"),15);
    CBAddString(Win,Str("20 ")+T("Seconds"),20);
    CBAddString(Win,Str("30 ")+T("Seconds"),30);
    CBAddString(Win,T("Always Shown"),OSD_SHOW_ALWAYS);
    if(CBSelectItemWithData(Win,*(p_element[i]))<0)
      SendMessage(Win,CB_SETCURSEL,0,0);
    y+=LineHeight;
  }
  Wid=GetCheckBoxSize(Font,T("Scrolling messages")).Width;
  Win=CreateWindow("Button",T("Scrolling messages"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX,
    page_l,y,Wid,23,Handle,(HMENU)12020,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,osd_show_scrollers,0);
#if defined(SSE_OSD_SHOW_TIME)
  y+=LineHeight;
  Wid=GetCheckBoxSize(Font,T("Time")).Width;
  Win=CreateWindow("Button",T("Time"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX,
    page_l,y,Wid,23,Handle,(HMENU)1036,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_OSD_TIME,0);
#endif
#if defined(SSE_OSD_DEBUGINFO)
  int Offset=Wid+HorizontalSeparation;
  Win=CreateButton(T("Debug info"),IDC_OSD_DEBUGINFO,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_OSD_DEBUGINFO,0);
  ToolAddWindow(ToolTip,Win,T("See manual for the meaning of symbols"));
#endif
#if defined(SSE_OSD_FPS_INFO)
  Offset+=Wid+HorizontalSeparation;
  Win=CreateButton(T("FPS"),IDC_OSD_FPSINFO,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_OSD_FPSINFO,0);
  ToolAddWindow(ToolTip,Win,T("ST Frame counter\nDoesn't work on everything!")); 
    //  in particular not with option C3, or STE programs that manipulate VCOUNT
#endif
  y+=LineHeight;
  Wid=GetCheckBoxSize(Font,T("Disable on screen display")).Width;
  Win=CreateWindow("Button",T("Disable on screen display"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX,
    page_l,y,Wid,23,Handle,(HMENU)12030,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,osd_disable,0);
  Offset=Wid+HorizontalSeparation;
  Win=CreateButton(T("No OSD on stop"),IDC_OSD_NONEONSTOP,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_NO_OSD_ON_STOP,0);
  ToolAddWindow(ToolTip,Win,T("One frame delay on stop")); 
#endif//WIN32

#ifdef UNIX
  hxc_button *p_but;
  hxc_dropdown *p_dd;

  p_but=new hxc_button(XD,page_p,page_l,y,0,25,button_notify_proc,this,
                          BT_CHECKBOX,T("Disk access light"),12000,BkCol);
  p_but->set_check(osd_show_disk_light);
  y+=35;

  p_but=new hxc_button(XD,page_p,page_l,y,0,25,button_notify_proc,this,
                          BT_CHECKBOX,T("Disk drive track info"),12001,BkCol);
  p_but->set_check(OPTION_DRIVE_INFO);
  y+=35;

  BYTE *p_element[4]={&osd_show_plasma,&osd_show_speed,&osd_show_icons,&osd_show_cpu};
  Str osd_name[4];
  osd_name[0]=T("Logo");
  osd_name[1]=T("Speed bar");
  osd_name[2]=T("State icons");
  osd_name[3]=T("CPU speed indicator");
  for (int i=0;i<4;i++){
    p_but=new hxc_button(XD,page_p,page_l,y,0,25,NULL,NULL,BT_LABEL,osd_name[i],0,BkCol);

    p_dd=new hxc_dropdown(XD,page_p,page_l+p_but->w+5,y,page_w-(p_but->w+5),200,dd_notify_proc,this);
    p_dd->id=12010+i;
    p_dd->additem(T("Off"),0);
    p_dd->additem(Str("2 ")+T("Seconds"),2);
    p_dd->additem(Str("3 ")+T("Seconds"),3);
    p_dd->additem(Str("4 ")+T("Seconds"),4);
    p_dd->additem(Str("5 ")+T("Seconds"),5);
    p_dd->additem(Str("6 ")+T("Seconds"),6);
    p_dd->additem(Str("8 ")+T("Seconds"),8);
    p_dd->additem(Str("10 ")+T("Seconds"),10);
    p_dd->additem(Str("12 ")+T("Seconds"),12);
    p_dd->additem(Str("15 ")+T("Seconds"),15);
    p_dd->additem(Str("20 ")+T("Seconds"),20);
    p_dd->additem(Str("30 ")+T("Seconds"),30);
    p_dd->additem(T("Always Shown"),OSD_SHOW_ALWAYS);
    if (p_dd->select_item_by_data(*(p_element[i]),0)<0) p_dd->changesel(0);
    y+=35;
  }

  p_but=new hxc_button(XD,page_p,page_l,y,0,25,button_notify_proc,this,
                          BT_CHECKBOX,T("Scrolling messages"),12020,BkCol);
  p_but->set_check(osd_show_scrollers);
  y+=35;

#if defined(SSE_OSD_SCROLLER_DISK_IMAGE)
  p_but=new hxc_button(XD,page_p,page_l,y,0,25,button_notify_proc,this,
                          BT_CHECKBOX,T("Disk image names"),12002,BkCol);
  p_but->set_check(OSD_IMAGE_NAME);
  y+=35;
#endif

  osd_disable_but.create(XD,page_p,page_l,y,0,25,button_notify_proc,this,
                          BT_CHECKBOX,T("Disable on screen display"),12030,BkCol);
  osd_disable_but.set_check(osd_disable);

#endif//UNIX
}


// 2 versions, one for DD, one for D3D

#if defined(SSE_VID_DD)

void TOptionBox::CreateFullscreenPage() {
  HWND Win;
  int y=10,Offset,mask,w,Wid;
  mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE;
  if(runstate!=RUNSTATE_STOPPED)
    mask|=WS_DISABLED;
  EasyStr caption=FullScreen?T("Go Windowed now"):T("Go Fullscreen now");
  Win=CreateWindow("Button",caption,mask,page_l+page_w/2-64,y,128,23,Handle,
    (HMENU)IDC_TOGGLE_FULLSCREEN,HInstance,NULL);
  y+=LineHeight;
  mask=WS_CHILD|WS_TABSTOP|BS_AUTOCHECKBOX;
  w=GetCheckBoxSize(Font,T("Fullscreen GUI")).Width;
  Win=CreateWindow("Button",T("Fullscreen GUI"),mask,
    page_l,y,w,23,Handle,(HMENU)7325,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_FULLSCREEN_GUI,0);
  ToolAddWindow(ToolTip,Win,T("Depends on your system, leaving this unchecked\
 is safer but if it works it's quite handy"));
  Offset=w+HorizontalSeparation;
  if(!OPTION_FULLSCREEN_GUI)
    mask|=WS_DISABLED;
  w=GetCheckBoxSize(Font,T("Confirm Before Quit")).Width;
  Win=CreateWindow("Button",T("Confirm Before Quit"),mask,page_l+Offset,y,w,23,
    Handle,(HMENU)230,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,FSQuitAskFirst,0);
  y+=LineHeight;
  w=GetCheckBoxSize(Font,T("Fullscreen on Maximize Window")).Width;
#if !defined(SSE_VID_2SCREENS) // not in bcc build
  OPTION_MAX_FS=TRUE;
  Win=CreateWindow("Button",T("Fullscreen on Maximize Window"),WS_CHILD|WS_DISABLED
    |WS_TABSTOP|BS_CHECKBOX,page_l,y,w,23,Handle,(HMENU)214,HInstance,NULL);
#else
  Win=CreateWindow("Button",T("Fullscreen on Maximize Window"),WS_CHILD
    |WS_TABSTOP|BS_CHECKBOX,page_l,y,w,23,Handle,(HMENU)214,HInstance,NULL);
#endif
  ToolAddWindow(ToolTip, Win,T("It was default behaviour in old Steem"));
  SendMessage(Win,BM_SETCHECK,OPTION_MAX_FS,0);
  Offset=w+HorizontalSeparation;
  w=GetCheckBoxSize(Font,T("Scanline Grille")).Width;
  Win=CreateWindow("Button",T("Scanline Grille"),WS_CHILD|WS_TABSTOP
    |BS_AUTOCHECKBOX,page_l+Offset,y,w,23,Handle,(HMENU)280,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,
    (draw_fs_fx==DFSFX_GRILLE ? BST_CHECKED:BST_UNCHECKED),0);
  ADVANCED_BEGIN
  if(Disp.DDDisplayModePossible[2][int(BytesPerPixel==1?0:1)])
  { // available on some systems
    y+=LineHeight; Offset=0;
    w=GetCheckBoxSize(Font,T("Use 640x400 (no borders only)")).Width;
    Win=CreateWindow("Button",T("Use 640x400 (no borders only)"),WS_CHILD
      |WS_TABSTOP|BS_CHECKBOX,page_l+Offset,y,w,23,Handle,(HMENU)210,HInstance,NULL);
    ToolAddWindow(ToolTip,Win,
      T("When this option is ticked Steem will use the 600x400 PC screen\
        resolution in fullscreen if it can"));
    SendMessage(Win,BM_SETCHECK,prefer_res_640_400,0);
  }
  y+=LineHeight;
  w=get_text_width(T("Drawing mode"));
  CreateWindow("Static",T("Drawing mode"),WS_CHILD,
    page_l,y+4,w,23,Handle,(HMENU)205,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
    page_l+5+w,y,127,200,Handle,(HMENU)204,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Screen Flip"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Straight Blit"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Stretch Blit"));
#if defined(SSE_VID_2SCREENS)
  //SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Fake fullscreen"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Windowed Borderless")); // same name as in D3D
#endif
  ToolAddWindow(ToolTip, Win,T("First two options draw double pixels (in low\
 res), Stretch adapts to your chosen resolution, fake uses your desktop screen"));
  SendMessage(Win,CB_SETCURSEL,draw_fs_blit_mode,0);
  Offset=203;
  w=get_text_width(T("Stretch"));
  CreateWindow("Static",T("Stretch"),WS_CHILD,page_l+Offset,y+4,w,23,Handle,
    (HMENU)205,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
    page_l+5+w+Offset,y,78,208,Handle,(HMENU)208,HInstance,NULL);
  for(int i=0;i<NFSRES&&Disp.fs_res[i].x;i++)
  {
    char res_txt[40];
    sprintf((char*)res_txt,"%dx%d",Disp.fs_res[i].x,Disp.fs_res[i].y);
    SendMessage(Win,CB_ADDSTRING,i,(LPARAM)res_txt);
  }
  SendMessage(Win,CB_SETCURSEL,Disp.fs_res_choice,0);
  ADVANCED_END
  y+=LineHeight;
  Wid=get_text_width(T("Aspect ratio"));
  CreateWindow("Static",T("Aspect ratio"),WS_CHILD,
    page_l,y+4,Wid,21,Handle,(HMENU)209,HInstance,NULL);
  Offset=Wid+HorizontalSeparation;
  mask=WS_CHILD|BS_AUTORADIOBUTTON;
  Wid=GetCheckBoxSize(Font,T("Screen")).Width;
  Win=CreateWindow("Button",T("Screen"),mask|WS_GROUP,page_l+Offset,
    y,Wid,25,Handle,(HMENU)17350,HInstance,NULL);
  EasyStr hint=T("Screen: resolution of your PC monitor, correct: resolution of\
 the ST, crisp: could be quite smaller but no distortion");
  ToolAddWindow(ToolTip,Win,hint);
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Correct")).Width;
  Win=CreateWindow("Button",T("Correct"),mask,page_l+Offset,
    y,Wid,25,Handle,(HMENU)17351,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,hint);
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Crisp")).Width;
  Win=CreateWindow("Button",T("Crisp"),mask,page_l+Offset,
    y,Wid,25,Handle,(HMENU)17352,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,hint);
  SendMessage(GetDlgItem(Handle,17350+OPTION_FULLSCREEN_AR),BM_SETCHECK,TRUE,0);
  ADVANCED_BEGIN
  y+=LineHeight;
  CreateWindow("Button",T("Synchronisation"),WS_CHILD|BS_GROUPBOX,
    page_l,y,page_w,170+30,Handle,(HMENU)99,HInstance,NULL);
  y+=20;
  w=GetCheckBoxSize(Font,T("VSync")).Width;
  Offset=10;
  Win=CreateWindow("Button",T("VSync"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX,
    page_l+Offset,y,w,23,Handle,(HMENU)206,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,FSDoVsync,0);
  ToolAddWindow(ToolTip,Win,T("When this option is ticked Steem will synchronise\
 the PC monitor with the ST in fullscreen mode, this makes some things look a\
 lot smoother but can be very slow.")+" "+T("The ST used 50Hz (PAL), 60Hz (NTSC)\
 and 70Hz (Mono), for good synchronisation you should set the PC refresh rate to\
 the same or double the ST refresh rate."));
#if defined(SSE_VID_3BUFFER_FS) // DD + D3D
  Offset+=w+HorizontalSeparation;
  w=GetCheckBoxSize(Font,T("Triple Buffering")).Width;
  Win=CreateWindow("Button",T("Triple Buffering"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX,
    page_l+Offset,y,w,25,Handle,(HMENU)1037,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_3BUFFER_FS,0);
  ToolAddWindow(ToolTip,Win,T("Yes, we add one buffer :) You decide if it's\
 better or not."));
#endif
  y+=LineHeight;
  CreateWindow("Static",T("Preferred PC refresh rates:"),WS_CHILD,
    page_l+10,y,page_w-20,25,Handle,(HMENU)99,HInstance,NULL);
  y+=25;
  if(Disp.DDDisplayModePossible[2][int(BytesPerPixel==1?0:1)])
  {
    w=get_text_width("640x400");
    CreateWindow("Static","640x400",WS_CHILD,
      page_l+10,y+4,w,25,Handle,(HMENU)99,HInstance,NULL);
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,page_l+
      15+w,y,page_w-10-(105+w),200,Handle,(HMENU)220,HInstance,NULL);
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Default"));
    for(int n=1;n<NUM_HZ;n++) 
      SendMessage(Win,CB_ADDSTRING,0,LPARAM((Str(HzIdxToHz[n])+"Hz").Text));
    CreateWindowEx(512,"Steem Path Display","",WS_CHILD|PDS_VCENTRESTATIC,
      page_l+page_w-90,y,80,23,Handle,(HMENU)221,HInstance,NULL);
    y+=LineHeight;
  }
  w=get_text_width("640x480");
  CreateWindow("Static","640x480",WS_CHILD,page_l+10,y+4,w,25,Handle,
    (HMENU)99,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
    page_l+15+w,y,page_w-10-(105+w),200,Handle,(HMENU)222,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Default"));
  for(int n=1;n<NUM_HZ;n++)
    SendMessage(Win,CB_ADDSTRING,0,LPARAM((Str(HzIdxToHz[n])+"Hz").Text));
  CreateWindowEx(512,"Steem Path Display","",WS_CHILD|PDS_VCENTRESTATIC,
    page_l+page_w-90,y,80,23,Handle,(HMENU)223,HInstance,NULL);
  y+=LineHeight;
  w=get_text_width("800x600");
  CreateWindow("Static","800x600",WS_CHILD,page_l+10,y+4,w,25,Handle,
    (HMENU)99,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
    page_l+15+w,y,page_w-10-(105+w),200,Handle,(HMENU)224,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Default"));
  for(int n=1;n<NUM_HZ;n++)
    SendMessage(Win,CB_ADDSTRING,0,LPARAM((Str(HzIdxToHz[n])+"Hz").Text));
  CreateWindowEx(512,"Steem Path Display","",WS_CHILD|PDS_VCENTRESTATIC,
    page_l+page_w-90,y,80,23,Handle,(HMENU)225,HInstance,NULL);
  y+=LineHeight;
  w=get_text_width(T("Stretch"));
  CreateWindow("Static",T("Stretch"), WS_CHILD,page_l+10,y+4,w,25,
    Handle,(HMENU)99,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
    page_l+15+w,y,page_w-10-(105+w),200,Handle,(HMENU)226,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Default"));
  for(int n=1;n<NUM_HZ;n++)
    SendMessage(Win,CB_ADDSTRING,0,LPARAM((Str(HzIdxToHz[n])+"Hz").Text));
  CreateWindowEx(512,"Steem Path Display","",WS_CHILD|PDS_VCENTRESTATIC,
    page_l+page_w-90,y,80,23,Handle,(HMENU)227,HInstance,NULL);
  ADVANCED_END
  UpdateFullscreen();
}

#endif


#if defined(SSE_VID_D3D)

void TOptionBox::CreateFullscreenPage() {
  HWND Win;
  int y=10,Offset,mask,w;
  mask=WS_CHILD|WS_TABSTOP|BS_AUTOCHECKBOX;
  int disable=(OPTION_FAKE_FULLSCREEN)?WS_DISABLED:0;
  Offset=0;
  long Wid=0;
  ADVANCED_BEGIN
    D3DFORMAT DisplayFormat=D3DFMT_X8R8G8B8;
/*  We do some D3D here, listing all modes only when necessary to save memory.
    Or we could have another function in display, but then code bloat...
*/
    UINT nD3Dmodes=(Disp.pD3D)
      ? Disp.pD3D->GetAdapterModeCount(Disp.m_Adapter,DisplayFormat) : 0;
    w=get_text_width(T("Mode"));
    CreateWindow("Static",T("Mode"),WS_CHILD,
      page_l+Offset,y+4,w,23,Handle,(HMENU)205,HInstance,NULL);
    Wid=110; // manual...
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST
      |WS_VSCROLL|disable,page_l+5+w+Offset,y,Wid,200,Handle,(HMENU)7319,
      HInstance,NULL);
    ToolAddWindow(ToolTip,Win,
      T("You have the choice between all the 32bit modes your video card can\
 handle. Try to be realistic."));
    D3DDISPLAYMODE Mode;
    for(UINT i=0;i<nD3Dmodes;i++)
    {
      Disp.pD3D->EnumAdapterModes(Disp.m_Adapter,DisplayFormat,i,&Mode);
      char tmp[20];
      sprintf(tmp,"%dx%d %dHz",Mode.Width,Mode.Height,Mode.RefreshRate);
      SendMessage(Win,CB_ADDSTRING,0,(LPARAM)tmp);
    }
    SendMessage(Win,CB_SETCURSEL,Disp.D3DMode,0);
    y+=LineHeight;
    Wid+=w;
  ADVANCED_END
  Wid=GetCheckBoxSize(Font,T("Crisp Rendering")).Width;
  mask=WS_CHILD | WS_TABSTOP | BS_CHECKBOX;
  Win=CreateWindow("Button",T("Crisp Rendering"),mask,
    page_l,y-1,Wid,25,Handle,(HMENU)7324,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_CRISP_FULLSCREEN,0);
  ToolAddWindow(ToolTip,Win,
    T("You like those big pixels? That's the option for you"));
  y+=LineHeight;
  ADVANCED_BEGIN
/*  This is handy if it works. Blame Microsoft and video card drivers 
    if it doesn't.
*/
  w=GetCheckBoxSize(Font,T("Fullscreen GUI")).Width;
  Win=CreateWindow("Button",T("Fullscreen GUI"),mask
 //   | (OPTION_FAKE_FULLSCREEN?0:WS_DISABLED)
    ,page_l,y,w,23,Handle,(HMENU)7325,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_FULLSCREEN_GUI,0);
  ToolAddWindow(ToolTip,Win,T("CAUTION! Liable to fail, especially true fullscreen!"));
  if(!OPTION_FULLSCREEN_GUI)
    mask|=WS_DISABLED;
  w=GetCheckBoxSize(Font,T("Confirm Before Quit")).Width;
  Win=CreateWindow("Button",T("Confirm Before Quit"),mask,page_l+130,y,w,23,
    Handle,(HMENU)230,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,FSQuitAskFirst,0);
  y+=LineHeight;
  ADVANCED_END
  int old_y=y;
  y=10; 
  y=old_y;
  ADVANCED_BEGIN
    w=GetCheckBoxSize(Font,T("VSync")).Width;
    Win=CreateWindow("Button",T("VSync"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|disable,
      page_l,y,w,23,Handle,(HMENU)206,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,FSDoVsync,0);
    ToolAddWindow(ToolTip,Win,T("When this option is ticked Steem will\
 synchronise the PC monitor with the ST in fullscreen mode, this makes some\
 things look a lot smoother but can be very slow.")+" "+T("The ST used 50Hz\
 (PAL), 60Hz (NTSC) and 70Hz (Mono), for good synchronisation you should set\
 the PC refresh rate to the same or double the ST refresh rate."));
#if defined(SSE_VID_3BUFFER_FS) // DD + D3D
    w=GetCheckBoxSize(Font,T("Triple Buffering")).Width;
    Win=CreateWindow("Button",T("Triple Buffering"),WS_CHILD|WS_TABSTOP
      |BS_CHECKBOX|disable,page_l+130,y,w,25,Handle,(HMENU)1037,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,OPTION_3BUFFER_FS,0);
    ToolAddWindow(ToolTip,Win,T("Yes, we add a buffer :) You decide if it's\
 better or not."));
#endif
    y+=LineHeight;
    w=GetCheckBoxSize(Font,T("Use Desktop Refresh Rate")).Width;
    Win=CreateWindow("Button",T("Use Desktop Refresh Rate"),
      WS_CHILD|WS_TABSTOP|BS_CHECKBOX|disable,
      page_l+10-10,y,w,23,Handle,(HMENU)209,HInstance,NULL);
    ToolAddWindow(ToolTip,Win,T("This will bypass the Hz setting in Mode, useful\
 for some video cards"));
    SendMessage(Win,BM_SETCHECK,OPTION_FULLSCREEN_DEFAULT_HZ,0);
#if defined(SSE_VID_D3D_FAKE_FULLSCREEN)
    y+=LineHeight;
    w=GetCheckBoxSize(Font,T("Windowed Borderless Mode")).Width;
    Win=CreateWindow("Button",T("Windowed Borderless Mode"),
      WS_CHILD|WS_TABSTOP|BS_CHECKBOX|(FullScreen?WS_DISABLED:0),
      page_l,y,w,23,Handle,(HMENU)213,HInstance,NULL); //395 210->213
    ToolAddWindow(ToolTip,Win,T("Safer fullscreen mode, like a big window\
 without frame"));
    SendMessage(Win,BM_SETCHECK,OPTION_FAKE_FULLSCREEN,0);
#endif
    y+=LineHeight;
  ADVANCED_END
  Win=CreateButton(T("Fullscreen on Maximize Window"),214,0,y,w);
  SendMessage(Win,BM_SETCHECK,OPTION_MAX_FS,0);
  if(FullScreen)
  {
    Win=CreateWindow("Button",T("Go Windowed now"),WS_CHILD|WS_TABSTOP
      |BS_CHECKBOX|BS_PUSHLIKE,page_l+w+20,y,128,23,Handle,
      (HMENU)IDC_TOGGLE_FULLSCREEN,HInstance,NULL);
  }
  else
  {
    Win=CreateWindow("Button",T("Go Fullscreen now"),WS_CHILD|WS_TABSTOP
      |BS_CHECKBOX|BS_PUSHLIKE,page_l+w+20,y,128,23,Handle,
      (HMENU)IDC_TOGGLE_FULLSCREEN,HInstance,NULL);
  }
}

#endif


#if defined(SSE_VID_DD)

void TOptionBox::UpdateFullscreen() {
  if(Handle==NULL) 
    return;
  EnableWindow(GetDlgItem(Handle,280),(draw_fs_blit_mode<DFSM_STRETCHBLIT));
  EnableWindow(GetDlgItem(Handle,281),(draw_fs_blit_mode<DFSM_STRETCHBLIT));
  EnableWindow(GetDlgItem(Handle,210),(border==0 
    && draw_fs_blit_mode!=DFSM_FAKEFULLSCREEN));
  EnableWindow(GetDlgItem(Handle,204),(!FullScreen));
  EnableWindow(GetDlgItem(Handle,208),
    (draw_fs_blit_mode==DFSM_STRETCHBLIT&&!FullScreen));
  BOOL can_change_ar=(draw_fs_blit_mode>=DFSM_STRETCHBLIT);
  EnableWindow(GetDlgItem(Handle,17350),can_change_ar);
  EnableWindow(GetDlgItem(Handle,17351),can_change_ar);
  EnableWindow(GetDlgItem(Handle,17352),can_change_ar);
  for(int i=0;i<NPC_HZ_CHOICES;i++) 
  {
    for(int n=0;n<NUM_HZ;n++) 
    {
      if(HzIdxToHz[n]==prefer_pc_hz[i]) 
      {
        SendDlgItemMessage(Handle,220+i*2,CB_SETCURSEL,n,0);
        break;
      }
    }
    EasyStr Text=T("UNTESTED");
    if(prefer_pc_hz[i]) 
    {
      if(LOBYTE(tested_pc_hz[i])==prefer_pc_hz[i]) 
      {
        if(HIBYTE(tested_pc_hz[i])) 
        {
          if(real_pc_hz[i]>0)
            Text=EasyStr(T("OK"))+" ("+real_pc_hz[i]+"Hz)";
          else
            Text=T("OK");
        }
        else
          Text=T("FAILED");
      }
    }
    else
      Text=T("OK");
    SendDlgItemMessage(Handle,221+i*2,WM_SETTEXT,0,LPARAM(Text.Text));
  }
}

#endif


void TOptionBox::CreateSoundPage() {
  int y=10;

#ifdef WIN32
  HWND Win;
  int Wid,Wid2,Offset,mask;
  DWORD DisableIfNoSound=DWORD((UseSound==0)?WS_DISABLED:0);
  Win=CreateButton(T("Mute sound"),IDC_SOUNDMUTE,0,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_SOUNDMUTE,0);
  Offset=Wid+HorizontalSeparation;
  Win=CreateButton(T("Mute sound when inactive"),801,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,MuteWhenInactive,0);
  y+=LineHeight;
  DWORD& DisableIfMute=DisableIfNoSound;

#if !defined(SSE_NO_INTERNAL_SPEAKER)
  DWORD DisableIfNT=DWORD(WinNT?WS_DISABLED:0);
#endif

  Str DrivStr=T("Device");
#if defined(SSE_SOUND_ENFORCE_RECOM_OPT)
  CreateWindow("Button",DrivStr.Text,WS_CHILD|BS_GROUPBOX|DisableIfMute,
    page_l,y,page_w,135,Handle,(HMENU)7105,HInstance,NULL);
#else
  CreateWindow("Button",DrivStr.Text,WS_CHILD|BS_GROUPBOX|DisableIfMute,
    page_l,y,page_w,230-30-30,Handle,(HMENU)7105,HInstance,NULL);
#endif
  y+=15;
  TConfigStoreFile CSF(globalINIFile);
  Wid=get_text_width(T("Sound driver"));
  Offset=10;
  CreateWindow("Static",T("Sound driver"),WS_CHILD,
    page_l+Offset,y+4,Wid,20,Handle,(HMENU)3000,HInstance,NULL);
  Offset+=Wid+HorizontalSeparation;
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST|DisableIfMute,
    page_l+Offset,y,page_w-Offset-10,200,Handle,(HMENU)3001,HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Default"));
  for(int i=0;i<DSDriverModuleList.NumStrings;i++)
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)DSDriverModuleList[i].String);
  SendMessage(Win,CB_SETCURSEL,0,0);
  EasyStr DSDriverModName=CSF.GetStr("Options","DSDriverName","");
  if(DSDriverModName.NotEmpty()) 
  {
    for(int i=0;i<DSDriverModuleList.NumStrings;i++)
      if(IsSameStr_I(DSDriverModuleList[i].String,DSDriverModName)) 
      {
        SendMessage(Win,CB_SETCURSEL,1+i,0);
        break;
      }
  }
  CSF.Close();
  y+=LineHeight;
  Wid=GetTextSize(Font,T("Volume")+": "+T("Min")).Width;
  CreateWindow("Static",T("Volume")+": "+T("Min"),WS_CHILD|DisableIfMute,
    page_l+10,y+4,Wid,23,Handle,(HMENU)7050,HInstance,NULL);
  Wid2=GetTextSize(Font,T("Max")).Width;
  CreateWindow("Static",T("Max"),WS_CHILD|DisableIfMute,
    page_l+page_w-10-Wid2,y+4,Wid2,23,Handle,(HMENU)7051,HInstance,NULL);
  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD|WS_TABSTOP|
    DisableIfMute|TBS_HORZ,
    page_l+15+Wid,y,(page_w-10-(Wid2+5))-(Wid+15),20,Handle,(HMENU)7100,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,100));
  int db=MaxVolume;
  int position=(int)pow(10.0,log10(101.0)*(db+10000)/10000)-1;
  SendMessage(Win,TBM_SETPOS,1,position);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  SendMessage(Win,TBM_SETPAGESIZE,0,10);
  y+=LineHeight;
  Wid=GetTextSize(Font,T("Frequency")).Width;
  CreateWindow("Static",T("Frequency"),WS_CHILD|DisableIfMute,page_l+10,y+4,
    Wid,23,Handle,(HMENU)7052,HInstance,NULL);
  Offset=70;
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|DisableIfMute
    |CBS_DROPDOWNLIST,page_l+15+Wid,y,Offset,200,Handle,(HMENU)7101,
    HInstance,NULL);
  Offset+=Wid;
  if(sound_comline_freq)
    CBAddString(Win,Str(sound_comline_freq)+"Hz",sound_comline_freq);
  CBAddString(Win,"384Khz",384000);
  CBAddString(Win,"250Khz",250000);
  CBAddString(Win,"192Khz",192000);
  CBAddString(Win,"96Khz",96000);
  CBAddString(Win,"50Khz",50066);
  CBAddString(Win,"48Khz",48000);
  CBAddString(Win,"44.1 Khz",44100);
  CBAddString(Win,"25Khz",25033);
  CBAddString(Win,"22Khz",22050);
  if(CBSelectItemWithData(Win,sound_chosen_freq)==-1)
    SendMessage(Win,CB_SETCURSEL,CBAddString(Win,Str(sound_chosen_freq)+"Hz",
      sound_chosen_freq),0);
  Wid=GetTextSize(Font,T("Format")).Width;
  Offset+=25;
  CreateWindow("Static",T("Format"),WS_CHILD|DisableIfMute,page_l+Offset,y+4,
    Wid,23,Handle,(HMENU)7060,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|DisableIfMute
    |CBS_DROPDOWNLIST,page_l+Offset+Wid,y,95,200,Handle,(HMENU)7061,
    HInstance,NULL);
  CBAddString(Win,T("8-Bit Mono"),MAKEWORD(8,1));
  CBAddString(Win,T("8-Bit Stereo"),MAKEWORD(8,2));
  CBAddString(Win,T("16-Bit Mono"),MAKEWORD(16,1));
  CBAddString(Win,T("16-Bit Stereo"),MAKEWORD(16,2));
  SendMessage(Win,CB_SETCURSEL,(sound_num_bits-8)/4+(sound_num_channels-1),0);
  y+=LineHeight;
#if !defined(SSE_SOUND_ENFORCE_RECOM_OPT)
  ADVANCED_BEGIN
    Wid=GetCheckBoxSize(Font,T("Write to primary buffer (not recommended)")).Width;
    Win=CreateWindow("Button",T("Write to primary buffer (not recommended)"),WS_CHILD|WS_TABSTOP|
    BS_CHECKBOX|DisableIfMute,
    page_l+10,y,Wid,23,Handle,(HMENU)7102,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,sound_write_primary,0);
    ToolAddWindow(ToolTip,Win,T("Steem tries to output sound in a way that is friendly to other programs.")+" "+
    T("Check this option ONLY if you have problems with your soundcard."));
    y+=LineHeight;
    Wid=GetTextSize(Font,T("Timing method")).Width;
    CreateWindow("Static",T("Timing method"),WS_CHILD|DisableIfMute,
      page_l+10,y+4,Wid,23,Handle,(HMENU)7053,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|
    DisableIfMute|CBS_DROPDOWNLIST,
    page_l+15+Wid,y,page_w-10-(15+Wid),200,Handle,(HMENU)7103,HInstance,NULL),
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Play Cursor"));
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Write Cursor (recommended)"));
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Milliseconds"));
    SendMessage(Win,CB_SETCURSEL,sound_time_method,0);
    y+=LineHeight;
  ADVANCED_END
#endif//#if !defined(SSE_SOUND_ENFORCE_RECOM_OPT)
  Wid=GetTextSize(Font,T("Delay")).Width;
  CreateWindow("Static",T("Delay"),WS_CHILD|DisableIfMute,page_l+10,y+4,Wid,
    23,Handle,(HMENU)7054,HInstance,NULL);
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|DisableIfMute|WS_VSCROLL
    |CBS_DROPDOWNLIST,page_l+15+Wid,y,page_w-10-(15+Wid),300,Handle,
    (HMENU)7104,HInstance,NULL);
  EasyStr Ms=T("Milliseconds");
  for(int i=0;i<=300;i+=20) 
    CBAddString(Win,Str(i)+" "+Ms);
  SendMessage(Win,CB_SETCURSEL,psg_write_n_screens_ahead,0);
  y+=LineHeight;
#if defined(SSE_YM2149_LL)
  CreateWindow("Button","YM2149 Sound",WS_CHILD|BS_GROUPBOX|DisableIfMute,
    page_l,y,page_w,45,Handle,(HMENU)7105,HInstance,NULL);
  y+=15;
  Offset=HorizontalSeparation;
  if(OPTION_HACKS)
  {
    Win=CreateButton(T("On"),IDC_YM2149_ON,Offset,y,Wid);
    SendMessage(Win,BM_SETCHECK,SSEConfig.YmSoundOn,0);
    Offset+=Wid+HorizontalSeparation;
  }
  Wid=GetCheckBoxSize(Font,T("Low-level")).Width;
  mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX|DisableIfMute;
  Win=CreateWindow("Button",T("Low-level"),mask,page_l+Offset,y,Wid,25,Handle,
    (HMENU)7312,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_MAME_YM,0);
  ToolAddWindow(ToolTip,Win,
    T("Using MAME's AY8910 emu (thx Couriersud) and LJBK's YM samples"));
  Offset+=Wid+HorizontalSeparation*2;
  Wid=GetTextSize(Font,"Filter").Width;
  CreateWindow("Static","Filter",WS_VISIBLE|WS_CHILD|DisableIfMute,page_l+
    Offset,y+HorizontalSeparation,Wid,23,Handle,(HMENU)3000,HInstance,NULL);
  Offset+=Wid+HorizontalSeparation;
  if(!OPTION_MAME_YM)
  {
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|DisableIfNoSound
      |CBS_DROPDOWNLIST,page_l+Offset,y,120,200,Handle,(HMENU)7099,
      HInstance,NULL),
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Line (direct)"));
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("TV (coaxial)"));
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Monitor (SCART)"));
    SendMessage(Win,CB_SETCURSEL,psg_hl_filter+1,0);
  }
  else
  {
    char tmp[40];
    sprintf(tmp,"%dHz",SSEOptions.low_pass_frequency);
    Wid=GetTextSize(Font,"22222hz").Width;
    CreateWindow("Static",tmp,WS_VISIBLE|WS_CHILD|DisableIfMute,
      page_l+Offset,y+HorizontalSeparation,Wid,23,Handle,(HMENU)995,HInstance,
      NULL);
    Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|TBS_HORZ
      |DisableIfMute,page_l+Offset+Wid+HorizontalSeparation,y,100,27,Handle,
      (HMENU)1002,HInstance,NULL);
    ToolAddWindow(ToolTip,Win,T("Kick-ass anti-aliasing filter, thx Mike Perkins"));
    SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,ym_low_pass_max));
    SendMessage(Win,TBM_SETPOS,1,SSEOptions.low_pass_frequency);
    SendMessage(Win,TBM_SETLINESIZE,0,1);
    SendMessage(Win,TBM_SETPAGESIZE,0,1);
    SendMessage(Win,TBM_SETTIC,0,YM_LOW_PASS_FREQ);
  }
  y+=LineHeight;
#endif//#if defined(SSE_YM2149_LL)
#if defined(SSE_GUI_OPTIONS_MICROWIRE)
  Offset=HorizontalSeparation;
  CreateWindow("Button",T("Digital Sound"),WS_CHILD|BS_GROUPBOX|DisableIfMute,
    page_l,y,page_w,45,Handle,(HMENU)7105,HInstance,NULL);
  y+=15;
  if(OPTION_HACKS)
  {
    Win=CreateButton(T("On"),IDC_STESOUND_ON,Offset,y,Wid);
    SendMessage(Win,BM_SETCHECK,SSEConfig.SteSoundOn,0);
    Offset+=Wid+HorizontalSeparation;
  }
  Wid=GetCheckBoxSize(Font,T("Microwire")).Width;
  mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX|DisableIfMute;
  if(DSP_DISABLED||(!IS_STE))
    mask|=WS_DISABLED;
  Win=CreateWindow("Button",T("Microwire"),mask,
    page_l+Offset,y,Wid,25,Handle,(HMENU)7302,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,SSEOptions.Microwire,0);
  ToolAddWindow(ToolTip,Win,T("Incomplete emulation"));
#endif
#if defined(SSE_SOUND_MICROWIRE_12DB_HACK)
  // Hey, now we have room for another option!
  if(OPTION_HACKS)
  {
    Offset+=Wid+HorizontalSeparation;
    Wid=GetCheckBoxSize(Font,T("YM-12db")).Width;
    Win=CreateWindow("Button",T("YM-12db"),mask,page_l+Offset,y,Wid,25,
      Handle,(HMENU)7345,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,OPTION_YM_12DB,0);
    ToolAddWindow(ToolTip,Win,T("Hack, not available on regular STE"));
  }
#endif
#if defined(SSE_DRIVE_SOUND)
  y+=LineHeight;
  CreateWindow("Button","Drive Sound",WS_CHILD|BS_GROUPBOX|DisableIfMute,
    page_l,y,page_w,45,Handle,(HMENU)7105,HInstance,NULL);
  y+=15;
  mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX|DisableIfMute;
  Wid=GetCheckBoxSize(Font,T("On")).Width;
  Win=CreateWindow("Button",T("On"),mask,page_l+HorizontalSeparation,y,Wid,25,
    Handle,(HMENU)7310,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_DRIVE_SOUND,0);
  ToolAddWindow(ToolTip,Win,T("Bad imitation of a SainT feature. You can choose\
 the sounds directory in the disk manager"));  
  mask&=~BS_CHECKBOX;
  Offset=Wid+HorizontalSeparation*2;
  Wid=GetTextSize(Font,T("Volume")+": "+T("Min")).Width;
  CreateWindow("Static",T("Volume")+": "+T("Min"),WS_CHILD|DisableIfMute,page_l
    +Offset,y+HorizontalSeparation,Wid,23,Handle,(HMENU)7050,HInstance,NULL);
  Wid2=GetTextSize(Font,T("Max")).Width;
  CreateWindow("Static",T("Max"),WS_CHILD|DisableIfMute,page_l+page_w-10-Wid2,
    y+HorizontalSeparation,Wid2,23,Handle,(HMENU)7051,HInstance,NULL);
  Offset+=Wid+HorizontalSeparation;
  Win=CreateWindow(TRACKBAR_CLASS,"",mask|TBS_HORZ,
    page_l+Offset,y,page_w-10-Offset-Wid2,20,Handle,(HMENU)7311,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(0,100));
  db=FloppyDrive[0].Sound_Volume;
  position=(int)pow(10.0,log10(101.0)*(db+10000)/10000)-1;
  SendMessage(Win,TBM_SETPOS,1,position);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  SendMessage(Win,TBM_SETPAGESIZE,0,10);
  y+=LineHeight;
#endif
  CreateWindow("Button",T("Record"),WS_CHILD|BS_GROUPBOX|DisableIfMute,
    page_l,y,page_w,80,Handle,(HMENU)7200,HInstance,NULL);
  y+=15;
  Win=CreateWindow("Steem Flat PicButton",Str(RC_ICO_RECORD),WS_CHILD
    |DisableIfMute,page_l+10,y,25,25,Handle,(HMENU)7201,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,sound_record,0);
  if(WAVOutputFile.Empty()) 
    WAVOutputFile=WriteDir+((OPTION_SOUND_RECORD_FORMAT)?"\\ST.wav":"\\ST.ym");
  CreateWindowEx(512,"Steem Path Display",WAVOutputFile,WS_CHILD|DisableIfMute,
    page_l+40,y,page_w-10-75-40,25,Handle,(HMENU)7202,HInstance,NULL);
  CreateWindow("Button",T("Choose"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE
    |DisableIfMute,page_l+page_w-10-70,y,70,23,Handle,(HMENU)7203,HInstance,NULL);
  y+=LineHeight;
/*  Add record to YM functionality using the same GUI elements as for WAV.
    We add a combobox to select format rather than radio buttons, this way
    we can add more formats. TODO either way
*/
  Wid=GetTextSize(Font,T("Format")).Width;
  CreateWindow("Static",T("Format"),WS_CHILD,page_l+10,y+4,Wid,23,Handle,
    (HMENU)7053,HInstance,NULL);
  Offset=page_l+15+Wid;
  mask=WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST|DisableIfMute;
  Win=CreateWindow("Combobox","",mask,
    Offset,y,60,200,Handle,(HMENU)7105,HInstance,NULL);
  Offset+=80;
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("Wav"));
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)CStrT("YM"));
  SendMessage(Win,CB_SETCURSEL,OPTION_SOUND_RECORD_FORMAT,0);
  Wid=GetCheckBoxSize(Font,T("Warn before overwrite")).Width;
  Win=CreateWindow("Button",T("Warn before overwrite"),WS_CHILD|WS_TABSTOP|
    BS_CHECKBOX|DisableIfMute,Offset,y,Wid,25,Handle,(HMENU)7204,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,RecordWarnOverwrite,0);
  y+=LineHeight;
  y+=5;
#if !defined(SSE_NO_INTERNAL_SPEAKER)
  Wid=GetCheckBoxSize(Font,T("Internal speaker sound")).Width;
  Win=CreateWindow("Button",T("Internal speaker sound"),WS_CHILD|WS_TABSTOP|
    BS_CHECKBOX|DisableIfNT,
    page_l,y,Wid,25,Handle,(HMENU)7300,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,sound_internal_speaker,0);
#endif
#endif//WIN32

#ifdef UNIX
  if(Psg.LoadFixedVolTable()==false)
  {
  sound_mode_label.create(XD,page_p,page_l,y,0,25,NULL,this,BT_LABEL,
    T("Output type"),0,BkCol);

  sound_mode_dd.create(XD,page_p,page_l+5+sound_mode_label.w,y,
    page_w-(5+sound_mode_label.w),200,dd_notify_proc,this);
  sound_mode_dd.id=5001;
  sound_mode_dd.make_empty();

    sound_mode_dd.additem(T("Line (direct)"));
    sound_mode_dd.additem(T("TV (coaxial)"));
    sound_mode_dd.additem(T("Monitor (SCART)"));
  sound_mode_dd.changesel(psg_hl_filter+1);
  y+=35;
  }

  sound_group.create(XD,page_p,page_l,y,page_w,210,NULL,this,BT_GROUPBOX,
    T("Configuration"),0,BkCol);

  int sgy=25;

  hxc_button *but=new hxc_button(XD,sound_group.handle,10,sgy,0,25,NULL,this,BT_LABEL,T("Library"),0,BkCol);

  hxc_dropdown *dd=new hxc_dropdown(XD,sound_group.handle,15+but->w,sgy,(sound_group.w-10-(15+but->w)),300,dd_notify_proc,this);
  dd->id=5067;
  dd->additem("None",0);
#ifndef NO_RTAUDIO
  dd->additem("RtAudio",XS_RT);
#endif
#ifndef NO_PORTAUDIO
  dd->additem("PortAudio",XS_PA);
#endif
  dd->select_item_by_data(x_sound_lib);
  sgy+=LineHeight;


  device_label.create(XD,sound_group.handle,10,sgy,0,25,NULL,this,
											BT_LABEL,T("Device"),0,BkCol);

  dd=new hxc_dropdown(XD,sound_group.handle,15+device_label.w,sgy,
                        (sound_group.w-10-(15+device_label.w)),300,dd_notify_proc,this);
  dd->id=5000;
  sgy+=LineHeight;

  but=new hxc_button(XD,sound_group.handle,10,sgy,0,25,NULL,this,BT_LABEL,T("Timing Method"),0,BkCol);

  dd=new hxc_dropdown(XD,sound_group.handle,15+but->w,sgy,(sound_group.w-10-(15+but->w)),300,dd_notify_proc,this);
  dd->id=5005;
  dd->additem("Write Cursor",0);
  dd->additem("Clock",2);
  dd->select_item_by_data(sound_time_method);
  sgy+=LineHeight;

  but=new hxc_button(XD,sound_group.handle,10,sgy,0,25,NULL,this,BT_LABEL,T("Delay"),0,BkCol);

  dd=new hxc_dropdown(XD,sound_group.handle,15+but->w,sgy,(sound_group.w-10-(15+but->w)),300,dd_notify_proc,this);
  dd->id=5004;
  EasyStr Ms=T("Milliseconds");
  for (int i=20;i<=800;i+=20) dd->additem(Str(i)+" "+Ms,i/20);
  dd->sel=0;
  dd->select_item_by_data(psg_write_n_screens_ahead);
  sgy+=LineHeight;

  sound_freq_label.create(XD,sound_group.handle,10,sgy,0,25,NULL,this,
    BT_LABEL,T("Frequency"),0,BkCol);

  sound_freq_dd.create(XD,sound_group.handle,15+sound_freq_label.w,
    sgy,370-(15+sound_freq_label.w),200,dd_notify_proc,this);
  sound_freq_dd.id=5002;
  sound_freq_dd.make_empty();
  if (sound_comline_freq){
    sound_freq_dd.additem(Str(sound_comline_freq)+"Hz",sound_comline_freq);
  }
  sound_freq_dd.additem("50066Hz",50066);
  sound_freq_dd.additem("44100Hz",44100);
  sound_freq_dd.additem("25033Hz",25033);
  sound_freq_dd.additem("22050Hz",22050);
  sound_freq_dd.select_item_by_data(sound_chosen_freq);
  sound_freq_dd.grandfather=page_p;
  sgy+=LineHeight;

  sound_format_label.create(XD,sound_group.handle,10,sgy,0,25,NULL,this,
    BT_LABEL,T("Format"),0,BkCol);

  sound_format_dd.create(XD,sound_group.handle,15+sound_format_label.w,
    sgy,370-(15+sound_format_label.w),200,dd_notify_proc,this);
  sound_format_dd.id=5003;

  FillSoundDevicesDD();

  y+=180+50;

#ifdef SSE_GUI_OPTIONS_MICROWIRE
///  y+=LineHeight;
  ste_microwire_but.create(XD,page_p,page_l+148,y,0,25,
    button_notify_proc,this,BT_CHECKBOX,T("Microwire"),4009,BkCol);
  ste_microwire_but.set_check(OPTION_MICROWIRE);
  hints.add(ste_microwire_but.handle,
//  T("This enables primitive DSP (based on code by Maverick aka Fabio Bizzetti, thx dude!) to emulate a rarely used STE feature."),
  T("Microwire (STE sound), incomplete emulation"),
    page_p);
//  y+=LineHeight;
#endif

#if 0
  keyboard_click_but.create(XD,page_p,page_l+240,y,0,25,
    button_notify_proc,this,BT_CHECKBOX,T("Keyboard click"),4007,BkCol);
  int keyboard_click=( PEEK(0x484)&1 ); // get current setting
  keyboard_click_but.set_check(keyboard_click);
  hints.add(keyboard_click_but.handle,
  T("This gives you direct access to bit1 of address $484, which enables or disables the annoying keyboard click (in TOS/GEM programs only)."),
    page_p);
  y+=LineHeight-10+2;
#endif

  record_group.create(XD,page_p,page_l,y,page_w,90,NULL,this,
    BT_GROUPBOX,T("Record"),0,BkCol);

  record_but.set_icon(&Ico32,ICO32_RECORD);
  record_but.create(XD,record_group.handle,10,25,25,25,button_notify_proc,this,
    BT_ICON,"",5100,BkCol);
  record_but.set_check(sound_record);

  wav_choose_but.create(XD,record_group.handle,record_group.w-10,25,0,25,button_notify_proc,this,
    BT_TEXT,T("Choose"),5101,BkCol);
  wav_choose_but.x-=wav_choose_but.w;
  XMoveWindow(XD,wav_choose_but.handle,wav_choose_but.x,wav_choose_but.y);

  wav_output_label.create(XD,record_group.handle,40,25,wav_choose_but.x-10-40,25,NULL,this,
    BT_TEXT | BT_BORDER_INDENT | BT_STATIC | BT_TEXT_PATH,
    WAVOutputFile,0,WhiteCol);

  overwrite_ask_but.create(XD,record_group.handle,10,55,0,25,button_notify_proc,this,
    BT_CHECKBOX,T("Warn before overwrite"),5102,BkCol);
  overwrite_ask_but.set_check(RecordWarnOverwrite);
  y+=100;
#if !defined(SSE_NO_INTERNAL_SPEAKER)
  internal_speaker_but.create(XD,page_p,page_l,y,0,25,button_notify_proc,this,
    BT_CHECKBOX,T("Internal speaker sound"),5300,BkCol);
  internal_speaker_but.set_check(sound_internal_speaker);
#endif

#endif//UNIX
}


#ifdef WIN32

void TOptionBox::CreateMIDIPage() {
  HWND Win;
  int y=10,Wid,Wid2,w,x;
  Wid=GetTextSize(Font,T("Volume")+": "+T("Min")).Width;
  CreateWindow("Static",T("Volume")+": "+T("Min"),WS_CHILD,
    page_l,y+4,Wid,23,Handle,HMENU(6000),HInstance,NULL);
  Wid2=GetTextSize(Font,T("Max")).Width;
  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD|WS_TABSTOP|TBS_HORZ,page_l+Wid
    +5,y,page_w-(Wid2+5)-(Wid+5),27,Handle,HMENU(6001),HInstance,NULL);
  SendMessage(Win,TBM_SETRANGEMAX,0,0xffff);
  SendMessage(Win,TBM_SETPOS,true,MIDI_out_volume);
  SendMessage(Win,TBM_SETLINESIZE,0,0xff);
  SendMessage(Win,TBM_SETPAGESIZE,0,0xfff);
  CreateWindow("Static",T("Max"),WS_CHILD,page_l+page_w-Wid2,y+4,Wid2,23,
    Handle,HMENU(6002),HInstance,NULL);
  y+=35;
  w=GetCheckBoxSize(Font,T("Allow running status for output")).Width;
  Win=CreateWindow("Button",T("Allow running status for output"),WS_CHILD
    |WS_TABSTOP|BS_CHECKBOX,page_l,y,w,23,Handle,(HMENU)6010,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,int((MIDI_out_running_status_flag==
    MIDI_ALLOW_RUNNING_STATUS)?BST_CHECKED:BST_UNCHECKED),0);
  y+=LineHeight;
  w=GetCheckBoxSize(Font,T("Allow running status for input")).Width;
  Win=CreateWindow("Button",T("Allow running status for input"),WS_CHILD
    |WS_TABSTOP|BS_CHECKBOX,page_l,y,w,23,Handle,(HMENU)6011,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,int((MIDI_in_running_status_flag==
    MIDI_ALLOW_RUNNING_STATUS)?BST_CHECKED:BST_UNCHECKED),0);
  y+=LineHeight;
  CreateWindow("Button",T("System Exclusive Buffers"),WS_CHILD|BS_GROUPBOX,
    page_l,y,page_w,85,Handle,(HMENU)99,HInstance,NULL);
  y+=20;
  x=page_l+10;
  Wid=GetTextSize(Font,T("Available for output")).Width;
  CreateWindow("Static",T("Available for output"),WS_CHILD,
    x,y+4,Wid,20,Handle,HMENU(6020),HInstance,NULL);
  x+=Wid+5;
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|WS_VSCROLL
    |CBS_DROPDOWNLIST,x,y,40,200,Handle,HMENU(6021),HInstance,NULL);
  for(int n=2;n<MAX_SYSEX_BUFS;n++)
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)Str(n).Text);
  SendMessage(Win,CB_SETCURSEL,MIDI_out_n_sysex-2,0);
  x+=45;
  Wid=GetTextSize(Font,T("size")).Width;
  CreateWindow("Static",T("size"),WS_CHILD,x,y+4,Wid,20,Handle,HMENU(6022),
    HInstance,NULL);
  x+=Wid+5;
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|WS_VSCROLL
    |CBS_DROPDOWNLIST,x,y,page_w-10-(x-page_l),200,Handle,HMENU(6023),HInstance,
    NULL);
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"16Kb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"32Kb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"64Kb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"128Kb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"256Kb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"512Kb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"1Mb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"2Mb");
  SendMessage(Win,CB_SETCURSEL,log_to_base_2(MIDI_out_sysex_max/1024)-4,0);
  y+=LineHeight;
  x=page_l+10;
  Wid=GetTextSize(Font,T("Available for input")).Width;
  CreateWindow("Static",T("Available for input"),WS_CHILD,x,y+4,Wid,20,Handle,
    HMENU(6030),HInstance,NULL);
  x+=Wid+5;
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|WS_VSCROLL
    |CBS_DROPDOWNLIST, x,y,40,200,Handle,HMENU(6031),HInstance,NULL);
  for(int n=2;n<MAX_SYSEX_BUFS;n++)
    SendMessage(Win,CB_ADDSTRING,0,(LPARAM)Str(n).Text);
  SendMessage(Win,CB_SETCURSEL,MIDI_in_n_sysex-2,0);
  x+=45;
  Wid=GetTextSize(Font,T("size")).Width;
  CreateWindow("Static",T("size"),WS_CHILD,x,y+4,Wid,20,Handle,HMENU(6032),
    HInstance,NULL);
  x+=Wid+5;
  Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|WS_VSCROLL
    |CBS_DROPDOWNLIST,x,y,page_w-10-(x-page_l),200,Handle,HMENU(6033),
    HInstance,NULL);
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"16Kb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"32Kb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"64Kb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"128Kb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"256Kb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"512Kb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"1Mb");
  SendMessage(Win,CB_ADDSTRING,0,(LPARAM)"2Mb");
  SendMessage(Win,CB_SETCURSEL,log_to_base_2(MIDI_in_sysex_max/1024)-4,0);
  y+=LineHeight;
  y+=13;
  CreateWindow("Static",T("Input speed")+": "+Str(MIDI_in_speed)+"%",WS_CHILD
    |SS_CENTER,page_l,y,page_w,20,Handle,HMENU(6040),HInstance,NULL);
  y+=20;
  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD|WS_TABSTOP|TBS_HORZ,
    page_l,y,page_w,27,Handle,HMENU(6041),HInstance,NULL);
  SendMessage(Win,TBM_SETRANGEMAX,0,99);
  SendMessage(Win,TBM_SETPOS,true,MIDI_in_speed-1);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  SendMessage(Win,TBM_SETPAGESIZE,0,5);
  for(int n=4;n<99;n+=5) 
    SendMessage(Win,TBM_SETTIC,0,n);
  y+=40;
  Win=CreateWindowEx(512,"Edit",
    T("The Steem MIDI interface is only suitable for programs that communicate using MIDI messages.")+"\r\n\r\n"+
    T("Any program that attempts to send raw data over the MIDI ports (for example a MIDI network game) will not work.")+"\r\n\r\n"+
    T("Keyboark option low-level emulation (HD6301, MC6250) has an impact on MIDI emulation"),
    WS_VISIBLE|WS_CHILD|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL,
    page_l,y,page_w,OPTIONS_HEIGHT-y-10,Handle,(HMENU)0,HInstance,NULL);
  MakeEditNoCaret(Win);
}


void TOptionBox::UpdateWindowSizeAndBorder() {
  SendMessage(BorderOption,CB_SETCURSEL,MIN((int)border,BIGGEST_DISPLAY),0);
  for(int res=0;res<3;res++) 
  {
    DWORD dat=WinSizeForRes[res];
    if(res<2)
      dat=MAKELONG(dat,draw_win_mode[res]);
    CBSelectItemWithData(GetDlgItem(Handle,302+res*2),dat);
  }
}


void TOptionBox::AssAddToExtensionsLV(char *Ext,char *Desc,int Num) {
  EasyStr Text=Str(Ext)+" ("+Desc+")";
  int y=5+30*Num;
  int ButWid=MAX(GetTextSize(Font,T("Associated")).Width,
    GetTextSize(Font,T("Associate")).Width)+16;
  int hoff=12-GetTextSize(Font,Text).Height/2;
  HWND But=CreateWindow("Button","",WS_CHILD|WS_TABSTOP|BS_CHECKBOX|BS_PUSHLIKE,
    5,y,ButWid,23,Scroller.GetControlPage(),HMENU(5100+Num),HInstance,NULL);
  HWND Stat=CreateWindow("Steem HyperLink",Text,WS_CHILD|HL_STATIC|HL_WINDOWBK,
    ButWid+10,y+hoff,300,25,Scroller.GetControlPage(),(HMENU)5000,HInstance,NULL);
  SendMessage(Stat,WM_SETFONT,WPARAM(Font),0);
  SendMessage(But,WM_SETFONT,WPARAM(Font),0);
  if(IsSteemAssociated(Ext))
    SendMessage(But,WM_SETTEXT,0,LPARAM(T("Associated").Text));
  else
    SendMessage(But,WM_SETTEXT,0,LPARAM(T("Associate").Text));
  ShowWindow(Stat,SW_SHOW);
  ShowWindow(But,SW_SHOW);
}


void TOptionBox::CreateAssocPage() {
  HWND Win;
#if defined(SSE_GUI_KBD)
  Scroller.CreateEx(WS_EX_CONTROLPARENT|512,WS_CHILD|WS_VSCROLL|WS_HSCROLL,
    page_l,10,page_w,OPTIONS_HEIGHT-10-10-25-10,Handle,(HMENU)5500,HInstance);
#else
  Scroller.CreateEx(512,WS_CHILD|WS_VSCROLL|WS_HSCROLL,
    page_l,10,page_w,OPTIONS_HEIGHT-10-10-25-10,Handle,5500,HInstance);
#endif
  Scroller.SetBkColour(GetSysColor(COLOR_WINDOW));
  AssAddToExtensionsLV(dot_ext(EXT_ST),T("Disk Image"),0);
  AssAddToExtensionsLV(dot_ext(EXT_STT),T("Disk Image"),1);
  AssAddToExtensionsLV(dot_ext(EXT_MSA),T("Disk Image"),2);
#if defined(SSE_DISK_STW) // STW instead because it's less likely to be zipped
  AssAddToExtensionsLV(dot_ext(EXT_STW),T("STW Disk Image"),3);
#elif USE_PASTI
  if(hPasti) AssAddToExtensionsLV(dot_ext(EXT_STX),T("Pasti Disk Image"),3);
#endif
  AssAddToExtensionsLV(dot_ext(EXT_DIM),T("Disk Image"),4);
  AssAddToExtensionsLV(".STZ",T("Zipped Disk Image"),5); // anybody uses that?
  AssAddToExtensionsLV(".STS",T("Memory Snapshot"),6);
#if defined(SSE_DISK_HFE) // more useful
  AssAddToExtensionsLV(dot_ext(EXT_HFE),T("ST/HxC Disk Image"),7);
#else
  AssAddToExtensionsLV(".STC",T("Cartridge Image"),7); //too rare
#endif
#if defined(SSE_TOS_PRG_AUTORUN)
  AssAddToExtensionsLV(dot_ext(EXT_PRG),T("Atari PRG executable"),8);
  AssAddToExtensionsLV(dot_ext(EXT_TOS),T("Atari TOS executable"),9);
#endif
  Scroller.AutoSize(5,5);
  int Wid=GetCheckBoxSize(Font,T("Always open files in new window")).Width;
  Win=CreateWindow("Button",T("Always open files in new window"),WS_CHILD
    |WS_TABSTOP|BS_CHECKBOX,page_l,OPTIONS_HEIGHT-35,Wid,25,Handle,
    (HMENU)5502,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,GetCSFInt("Options","OpenFilesInNew",true,globalINIFile),0);
}


void TOptionBox::IconsAddToScroller() {
  for(int n=14100;n<14100+RC_NUM_ICONS;n++)
    if(GetDlgItem(Scroller.GetControlPage(),n)) 
      DestroyWindow(GetDlgItem(Scroller.GetControlPage(),n));
  int x=3,y=3;
  for(int want_size=16;want_size;want_size<<=1) 
  {
    for(INT_PTR n=1;n<RC_NUM_ICONS;n++) 
    {
      int size=RCGetSizeOfIcon(n) & ~1;
      switch(n) {
      case RC_ICO_HARDDRIVES:
      case RC_ICO_HARDDRIVES_FR:
        int want_ico=RC_ICO_HARDDRIVES;
        if(IsSameStr_I(T("File"),"Fichier")) want_ico=RC_ICO_HARDDRIVES_FR;
        if(n!=want_ico) size=0;
        break;
      }
      if(size==want_size) 
      {
#if defined(SSE_GUI_KBD)
        CreateWindow("Steem Flat PicButton",Str(n),WS_CHILD|PBS_RIGHTCLICK
          |WS_TABSTOP,x,y,size+4,size+4,Scroller.GetControlPage(),
          HMENU(14100+n),HInstance,NULL);
#else
        CreateWindow("Steem Flat PicButton",Str(n),WS_CHILD|PBS_RIGHTCLICK,
          x,y,size+4,size+4,Scroller.GetControlPage(),HMENU(14100+n),HInstance,NULL);
#endif
        x+=size+4+3;
      }
      if(x+want_size+4+3>=page_w-GuiSM.cx_vscroll()||n==RC_NUM_ICONS-1) 
      {
        x=3;
        y+=want_size+4+3;
      }
    }
  }
  for(int n=14100;n<14100+RC_NUM_ICONS;n++)
    if(GetDlgItem(Scroller.GetControlPage(),n)) 
      ShowWindow(GetDlgItem(Scroller.GetControlPage(),n),SW_SHOWNA);
  Scroller.AutoSize(0,5);
}


void TOptionBox::CreateIconsPage() {
  int th=GetTextSize(Font,T("Left click to change")).Height;
  int y=10,scroller_h=OPTIONS_HEIGHT-10-th-2-10-25-10;
  CreateWindow("Static",T("Left click to change, right to reset"),WS_CHILD,
    page_l,y,page_w,th,Handle,(HMENU)14002,HInstance,NULL);
  y+=th+2;
#if defined(SSE_GUI_KBD)
  Scroller.CreateEx(WS_EX_CONTROLPARENT|512,WS_CHILD|WS_VSCROLL|WS_HSCROLL,
    page_l,y,page_w,scroller_h,Handle,(HMENU)14010,HInstance);
#else
  Scroller.CreateEx(512,WS_CHILD|WS_VSCROLL|WS_HSCROLL,page_l,y,
    page_w,scroller_h,Handle,14010,HInstance);
#endif
  Scroller.SetBkColour(GetSysColor(COLOR_BTNFACE));
  IconsAddToScroller();
  y+=scroller_h+10;
  CreateWindow("Button",T("Load Icon Scheme"),WS_CHILD|WS_TABSTOP|BS_CHECKBOX
    |BS_PUSHLIKE,page_l,y,page_w/2-5,23,Handle,(HMENU)14020,HInstance,NULL);
  CreateWindow("Button",T("All Icons To Default"),WS_CHILD|WS_TABSTOP
    |BS_CHECKBOX|BS_PUSHLIKE,page_l+page_w/2+5,y,page_w/2-5,23,Handle,
    (HMENU)14021,HInstance,NULL);
}

#endif//WIN32


#ifdef UNIX

void TOptionBox::FillSoundDevicesDD()
{
  hxc_dropdown *dd=(hxc_dropdown*)hxc::find(sound_group.handle,5000);
  dd->make_empty();
#ifndef NO_PORTAUDIO  //NO_PORTAUDIO
  if (UseSound==XS_PA){
    int c=Pa_GetDeviceCount(),isel=Pa_GetDefaultOutputDevice();
    for (PaDeviceIndex i=0;i<c;i++){
      const PaDeviceInfo *pdev=Pa_GetDeviceInfo(i);
      if (pdev->maxOutputChannels>0){
        dd->additem((char*)(pdev->name),i);
        if (IsSameStr_I(pdev->name,sound_device_name)) isel=i;
      }
    }
    dd->select_item_by_data(MAX(isel,0));
    sound_device_name=dd->sl[dd->sel].String; // Just in case changed to default
  }
#endif
#ifndef NO_RTAUDIO
  if (UseSound==XS_RT){
    RtAudio::DeviceInfo radi;
    int c=rt_audio->getDeviceCount(),isel=0; //isel=default device, find while walking through list
#if defined(SSE_UNIX)
    for (int i=0;i<c;i++){ // odd bug, the system must have changed
#else
    for (int i=1;i<=c;i++){
#endif
      radi=rt_audio->getDeviceInfo(i);
      if (radi.outputChannels>0){
        dd->additem((char*)(radi.name.c_str()),i);
        if (radi.isDefaultOutput) isel=i;
      }
    }
    for (int i=1;i<=c;i++){
      if (IsSameStr_I(radi.name.c_str(),sound_device_name)) isel=i;
    }
    dd->select_item_by_data(MAX(isel,0));
    sound_device_name=dd->sl[dd->sel].String; // Just in case changed to default
  }
#endif
  if (UseSound==0) dd->additem(T("None"));

#ifdef NO_RTAUDIO
  int rt_unsigned_8bit=0;
#endif
  sound_format_dd.make_empty();
  sound_format_dd.additem(T("8-Bit Mono"),MAKEWORD(8,1));
  sound_format_dd.additem(T("8-Bit Stereo"),MAKEWORD(8,2));
  if (x_sound_lib==XS_RT){
    sound_format_dd.additem(T("8-Bit Mono Unsigned"),MAKELONG(MAKEWORD(8,1),1));
    sound_format_dd.additem(T("8-Bit Stereo Unsigned"),MAKELONG(MAKEWORD(8,2),1));
  }
  sound_format_dd.additem(T("16-Bit Mono"),MAKEWORD(16,1));
  sound_format_dd.additem(T("16-Bit Stereo"),MAKEWORD(16,2));
  sound_format_dd.sel=-1;
	sound_format_dd.select_item_by_data(MAKELONG(MAKEWORD(sound_num_bits,sound_num_channels),rt_unsigned_8bit));
  if (sound_format_dd.sel==-1){
  	sound_format_dd.select_item_by_data(MAKEWORD(sound_num_bits,sound_num_channels));
  }
}


void TOptionBox::CreatePathsPage()
{
	int y=10;
  hxc_edit *p_ed;
  hxc_button *p_but;
  
  Str Comline_Desc[NUM_COMLINES];
  Comline_Desc[COMLINE_HTTP]=T("Web");
  Comline_Desc[COMLINE_FTP]=T("FTP");
  Comline_Desc[COMLINE_MAILTO]=T("E-mail");
  Comline_Desc[COMLINE_FM]=T("File Manager");
  Comline_Desc[COMLINE_FIND]=T("Find File(s)");

  for (int i=0;i<NUM_COMLINES;i++){
    p_but=new hxc_button(XD,page_p,page_l,y,0,25,NULL,this,BT_LABEL,Comline_Desc[i],0,BkCol);

    p_ed=new hxc_edit(XD,page_p,page_l+p_but->w+5,y,page_w-p_but->w-5-20,25,edit_notify_proc,this);
    p_ed->set_text(Comlines[i]);
    p_ed->id=15000+i*10;
  
    p_but=new hxc_button(XD,page_p,page_l+page_w-20,y,20,25,button_notify_proc,this,
    										BT_ICON,"",15001+i*10,BkCol);
    p_but->set_icon(NULL,1);
    y+=LineHeight;
  }
  
  
}

#endif//UNIX


#if !defined(SSE_NO_UPDATE)

void TOptionBox::CreateUpdatePage() {
  int Wid;

  TConfigStoreFile CSF(globalINIFile);
  DWORD Disable=DWORD(Exists(RunDir+"\\SteemUpdate.exe") ? 0:WS_DISABLED);
  int Runs=CSF.GetInt("Update","Runs",0),
      Offline=CSF.GetInt("Update","Offline",0),
      WSError=CSF.GetInt("Update","WSError",0),
      y=10;

  EasyStr Info=EasyStr(" ");
  Info+=T("Update has checked for a new Steem")+" "+Runs+" "+time_or_times(Runs)+"\n ";
  Info+=T("It thought you were off-line")+" "+Offline+" "+time_or_times(Offline)+"\n ";
  Info+=T("It encountered an error")+" "+WSError+" "+time_or_times(WSError)+"\n ";
  CreateWindowEx(512,"Static",Info,WS_CHILD | Disable,
                  page_l,y,page_w,80,Handle,(HMENU)4100,HInstance,NULL);
  y+=90;


  Wid=GetCheckBoxSize(Font,T("Disable automatic update checking")).Width;
  HWND ChildWin=CreateWindow("Button",T("Disable automatic update checking"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | Disable,
                          page_l,y,Wid,20,Handle,(HMENU)4200,HInstance,NULL);
  SendMessage(ChildWin,BM_SETCHECK,!CSF.GetInt("Update","AutoUpdateEnabled",true),0);
  y+=LineHeight;

  Wid=GetCheckBoxSize(Font,T("This computer is never off-line")).Width;
  ChildWin=CreateWindow("Button",T("This computer is never off-line"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | Disable,
                          page_l,y,Wid,20,Handle,(HMENU)4201,HInstance,NULL);
  SendMessage(ChildWin,BM_SETCHECK,CSF.GetInt("Update","AlwaysOnline",0),0);
  y+=LineHeight;

  Wid=GetCheckBoxSize(Font,T("Download new patches")).Width;
  ChildWin=CreateWindow("Button",T("Download new patches"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | Disable,
                          page_l,y,Wid,20,Handle,(HMENU)4202,HInstance,NULL);
  SendMessage(ChildWin,BM_SETCHECK,CSF.GetInt("Update","PatchDownload",1),0);
  y+=LineHeight;

  Wid=GetCheckBoxSize(Font,T("Ask before installing new patches")).Width;
  ChildWin=CreateWindow("Button",T("Ask before installing new patches"),WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | Disable,
                          page_l,y,Wid,20,Handle,(HMENU)4203,HInstance,NULL);
  SendMessage(ChildWin,BM_SETCHECK,CSF.GetInt("Update","AskPatchInstall",0),0);
  y+=LineHeight;

  HANDLE UpdateMutex=OpenMutex(MUTEX_ALL_ACCESS,0,"SteemUpdate_Running");
  if (UpdateMutex){
    CloseHandle(UpdateMutex);
    Disable=WS_DISABLED;
  }else if (UpdateWin || FullScreen){
    Disable=WS_DISABLED;
  }
  CreateWindow("Button",T("Check For Update Now"),WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | int(UpdateWin || FullScreen ? WS_DISABLED:0) | Disable,
                    page_l,y,page_w,23,Handle,(HMENU)4400,HInstance,NULL);

  CSF.Close();
}

#endif


void TOptionBox::CreateSSEPage() {
  int y=10; // top

#ifdef WIN32
  HWND Win;
  int Wid,Offset=0,mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX;
  EasyStr tip_text;
  // Title
  Wid=get_text_width(T("Steem Extra Options\n==========================="))/2;
  CreateWindow("Static",T("Steem Extra Options\n==========================="),
    WS_CHILD,page_l,y,Wid,21,Handle,(HMENU)209,HInstance,NULL);
#if defined(SSE_GUI_OPTION_FOR_TESTS)
  Offset=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Beta tests")).Width;
  Win=CreateWindow("Button",T("Beta tests"),WS_CHILD|WS_TABSTOP|
    BS_CHECKBOX,page_l+Offset,y,Wid,25,Handle,(HMENU)7316,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,SSEOptions.TestingNewFeatures,0);
  Offset=0;
#else
  //ASSERT(!SSEOptions.TestingNewFeatures);
#endif
  y+=LineHeight+10;
  Win=CreateButton(T("Advanced settings"),IDC_ADVANCED_SETTINGS,0,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_ADVANCED,0);
  ToolAddWindow(ToolTip,Win,
    T("For those who know what they're doing and don't report fake bugs!"));
  Win=CreateWindow("Button",T("Reset advanced settings"),WS_CHILD|WS_TABSTOP
    |BS_CHECKBOX|BS_PUSHLIKE,page_l+Wid+20,y,130,23,Handle,(HMENU)1039,
    HInstance,NULL);
  ToolAddWindow(ToolTip,Win,
    T("So you played the expert and now it's all messed up? This might help"));
    //T("Restore all advanced settings to compatible defaults"));
  y+=LineHeight;
#if defined(SSE_GUI_LEGACY_TOOLBAR)
  Win=CreateButton(T("Legacy toolbar"),IDC_LEGACY_TOOLBAR,0,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_LEGACY_TOOLBAR,0);
  ToolAddWindow(ToolTip,Win,T("As in Steem 3.2"));
  Offset+=Wid+HorizontalSeparation;
#elif defined(SSE_GUI_STATUS_BAR) // now it's all or nothing (v4.0.1)
  Win=CreateButton(T("Status bar"),7307,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_STATUS_BAR,0);
  ToolAddWindow(ToolTip,Win,T("Displays some info in the tool bar."));
#ifdef SSE_GUI_STATUS_BAR_GAME_NAME
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Disk name")).Width;
  Win=CreateWindow("Button",T("Disk name"),WS_CHILD|WS_TABSTOP|
    BS_CHECKBOX,page_l+Offset,y,Wid,25,Handle,(HMENU)7309,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_STATUS_BAR_GAME_NAME,0);
  ToolAddWindow(ToolTip,Win,T("Also the name of the current disk."));
#endif
#endif
  Offset+=Wid+HorizontalSeparation;
#if defined(SSE_GUI_TOOLBAR)
  Win=CreateButton(T("Tool bar in task bar"),IDC_TOOLBAR_TASKBAR,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_TOOLBAR_TASKBAR,0);
  Offset+=Wid+HorizontalSeparation;
  Win=CreateButton(T("Vertical tool bar"),IDC_TOOLBAR_VERTICAL,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_TOOLBAR_VERTICAL,0);
#endif
  y+=LineHeight;
  Offset=0;
  Win=CreateButton(T("Hacks"),1027,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_HACKS,0);
  //ToolAddWindow(ToolTip,Win,T("For an edgier emulation, recommended!"));
  ToolAddWindow(ToolTip,Win,
T("Some options not available on a real ST, conveniences for the player or the programmer"));
  Offset+=Wid+HorizontalSeparation;
  Win=CreateButton(T("Emu detect"),1031,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_EMU_DETECT,0);
  ToolAddWindow(ToolTip,Win,T("Enable communication between Steem and ST programs."));
  mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX;
#if defined(SSE_IKBD_RTC)
  if(OPTION_HACKS)
  {
    Offset+=Wid+HorizontalSeparation;
    Win=CreateButton(T("Clock always correct"),7334,Offset,y,Wid);
    SendMessage(Win,BM_SETCHECK,OPTION_RTC_HACK,0);
    ToolAddWindow(ToolTip,Win,T("Handy hack voiding any RTC emulation, but can\
 interfere with some programs."));
  }
#endif
  y+=LineHeight;
  Offset=0;
  Win=CreateButton(T("Warnings"),IDC_WARNINGS,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_WARNINGS,0);
  ToolAddWindow(ToolTip,Win,T("Uncheck this if you're a pro!"));
  Offset+=Wid+HorizontalSeparation;
#if defined(SSE_GUI_DEFAULT_ST_CONFIG)
  Win=CreateButton(T("Default ST configs"),7344,Offset,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_ST_PRESELECT,0);
  ToolAddWindow(ToolTip,Win,
    T("Look into the current configuration folder for a config with the same\
 name as the ST model."));
#endif
#endif//WIN32

#ifdef UNIX
  int Wid;
  const int LineHeight=30;
  const int HorizontalSeparation=10;

#if defined(SSE_VID_BORDERS)
  Wid=hxc::get_text_width(XD,T("Display size"));
  border_size_label.create(XD,page_p,page_l,y,Wid,25,NULL,this,BT_STATIC 
    | BT_TEXT,T("Display Size"),0,BkCol);
  border_size_dd.id=4001; //TODO
  border_size_dd.make_empty();
  border_size_dd.additem("384 x 270",0);
//  border_size_dd.additem("Large (400x275)",1);
  border_size_dd.additem("400 x 278 (buggy)",1);
#if defined(SSE_VID_BORDERS_412)
#if defined(SSE_VID_BORDERS_413) // !
#if defined(SSE_VID_BORDERS_BIGTOP) && !defined(SSE_VID_BORDERS_416)
  border_size_dd.additem("413 x 286",2);  
#else
  border_size_dd.additem("413 x 280 (buggy)",2);  
#endif
#else
#if defined(SSE_VID_BORDERS_BIGTOP) && !defined(SSE_VID_BORDERS_416)
  border_size_dd.additem("412 x 286",2);  
#else
  border_size_dd.additem("412 x 280",2);  
#endif
#endif//413
#endif//412

#if defined(SSE_VID_BORDERS_416)
#if defined(SSE_VID_BORDERS_BIGTOP)
  border_size_dd.additem("416 x 286",3);  
#else
  border_size_dd.additem("416 x 280",3);  
#endif
#endif
  border_size_dd.select_item_by_data(DISPLAY_SIZE);

  hints.add(border_size_dd.handle, // works?
    "Some border effects (demos) will look better in one of the sizes, it really depends on the program, generally smallest is fine for games."
    ,page_p);

  border_size_dd.create(XD,page_p,page_l+5+Wid,y,400-(15+Wid+10),350,
    dd_notify_proc,this);
  y+=LineHeight;
#endif

  capture_mouse_but.create(XD,page_p,page_l,y,0,25,
    button_notify_proc,this,BT_CHECKBOX,T("Capture mouse"),4002,BkCol);
  capture_mouse_but.set_check(OPTION_CAPTURE_MOUSE);
  hints.add(capture_mouse_but.handle,
    T("If unchecked, Steem will leave mouse control to X-Windows until you click in the window"),
    page_p);
  y+=LineHeight;

#if 1//defined(SSE_HACKS)
  specific_hacks_but.create(XD,page_p,page_l,y,0,25,
    button_notify_proc,this,BT_CHECKBOX,T("Hacks"),4003,BkCol);
  specific_hacks_but.set_check(OPTION_HACKS);
  hints.add(specific_hacks_but.handle,
  T("For an edgier emulation, recommended!"),page_p);
  y+=LineHeight;
#endif

#if 1//defined(SSE_VAR_EMU_DETECT) 
  stealth_mode_but.create(XD,page_p,page_l,y,0,25,
    button_notify_proc,this,BT_CHECKBOX,T("Emu detect"),4004,BkCol);
  stealth_mode_but.set_check(OPTION_EMU_DETECT);
  hints.add(stealth_mode_but.handle,
  T("Enable easy detection of Steem by ST programs."),page_p);
  y+=LineHeight;
#endif


#if defined(SSE_HD6301_LL) 
  hd6301emu_but.create(XD,page_p,page_l,y,0,25,
    button_notify_proc,this,BT_CHECKBOX,T("C1: 6850/6301/MIDI/E-Clock"),4006,BkCol);
  hd6301emu_but.set_check(OPTION_C1);
  hints.add(hd6301emu_but.handle,
  T("Chipset 1 - This enables a low level emulation of the IKBD keyboard chip (using\
 the Sim6xxx code by Arne Riiber, thx dude!), precise E-Clock, as well as ACIA and MIDI \
 improvements or bugs"),
    page_p);
  y+=LineHeight;
#endif

#if defined(SSE_INT_MFP_OPTION)
#if defined(SSE_HD6301_LL) 
  y-=LineHeight;
#endif
  mc68901_but.create(XD,page_p,page_l+220,y,0,25,
    button_notify_proc,this,BT_CHECKBOX,T("C2: 68901"),4014,BkCol);
  mc68901_but.set_check(OPTION_68901);
  hints.add(mc68901_but.handle,
  T("Chipset 2 - Check for a more precise emulation of the MFP."),page_p);
  y+=LineHeight;
#endif

#if defined(SSE_TOS_KEYBOARD_CLICK)
  keyboard_click_but.create(XD,page_p,page_l,y,0,25,
    button_notify_proc,this,BT_CHECKBOX,T("Keyboard click"),4007,BkCol);
  int keyboard_click=( PEEK(0x484)&1 ); // get current setting
  keyboard_click_but.set_check(keyboard_click);
  hints.add(keyboard_click_but.handle,
  T("This gives you direct access to bit1 of address $484, which enables or disables the annoying keyboard click (in TOS/GEM programs only)."),
    page_p);
  y+=LineHeight;
#endif

  vm_mouse_but.create(XD,page_p,page_l,y,0,25,
    button_notify_proc,this,BT_CHECKBOX,T("VM-friendly mouse"),4015,BkCol);
  vm_mouse_but.set_check(OPTION_VMMOUSE);
  hints.add(vm_mouse_but.handle,
    T("Alternative mouse handling. Better for virtual machines"),page_p);
  y+=LineHeight;

#endif
}


void TOptionBox::SSEUpdateIfVisible() {
  if(Handle==NULL)
    return;
  UpdateCaption();
#ifdef WIN32    
  HWND Win;
#if defined(SSE_HD6301_LL)
  Win=GetDlgItem(Handle,IDC_6301); //HD6301 emu (keyboard page now)
  if(Win!=NULL)
  {
    if(!HD6301_OK)
      SendMessage(Win,BN_DISABLE,0,0);
    else
      SendMessage(Win,BM_SETCHECK,OPTION_C1,0);
  }
#endif
  Win=GetDlgItem(Handle,IDC_VMMOUSE);
  if(Win!=NULL)
    SendMessage(Win,BM_SETCHECK,OPTION_VMMOUSE,0);
  REFRESH_STATUS_BAR; //overkill
  UpdateSTVideoPage();
#endif//WIN32  
}


void TOptionBox::CreateInputPage() {
#ifdef WIN32  
  int y=10;
  int Wid,Offset=10,mask;
  mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX;
  HWND Win;
  CreateWindow("Button",T("Keyboard"),WS_CHILD|BS_GROUPBOX,
    page_l,y,page_w,80+30,Handle,(HMENU)8093,HInstance,NULL);
  y+=20;

  Wid=GetTextSize(Font,T("Language PC side")).Width;
  CreateWindow("Static",T("Language PC side"),WS_CHILD,
    page_l+Offset,y+4,Wid,25,Handle,(HMENU)8400,HInstance,NULL);
  HWND Combo=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
    page_l+15+Wid,y,(page_w-20)-(5+Wid),200,Handle,(HMENU)8401,HInstance,NULL);
  CBAddString(Combo,T("United States"),
    MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US));
  CBAddString(Combo,T("United Kingdom"),
    MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_UK));
  CBAddString(Combo,T("Australia (UK TOS)"),
    MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_AUS));
  CBAddString(Combo,T("German"),
    MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN));
  CBAddString(Combo,T("French"),
    MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH));
  CBAddString(Combo,T("Spanish"),
    MAKELANGID(LANG_SPANISH,SUBLANG_SPANISH));
  CBAddString(Combo,T("Italian"),
    MAKELANGID(LANG_ITALIAN,SUBLANG_ITALIAN));
  CBAddString(Combo,T("Swedish"),
    MAKELANGID(LANG_SWEDISH,SUBLANG_SWEDISH));
  CBAddString(Combo,T("Norwegian"),
    MAKELANGID(LANG_NORWEGIAN,SUBLANG_NEUTRAL));
  CBAddString(Combo,T("Belgian (French TOS)"),
    MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_BELGIAN));
#if defined(SSE_IKBD_MAPPINGFILE)
  CBAddString(Combo,T("Mapping file"),MAKELANGID(LANG_CUSTOM,SUBLANG_NEUTRAL));
#endif
  if(CBSelectItemWithData(Combo,KeyboardLangID)<0)
    SendMessage(Combo,CB_SETCURSEL,0,0);
  y+=LineHeight;
  Wid=GetCheckBoxSize(Font,T("Shift and alternate correction")).Width;
  Win=CreateWindow("Button",T("Shift and alternate correction"),WS_CHILD
    |WS_TABSTOP|BS_AUTOCHECKBOX,page_l+10,y,Wid,25,Handle,(HMENU)8402,
    HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,EnableShiftSwitching,0);
  EnableWindow(Win,ShiftSwitchingAvailable);
  ToolAddWindow(ToolTip,Win,
    T("When checked this allows Steem to emulate all keys correctly, it does \
this by changing the shift and alternate state of the ST when you press them.")
+" "+  T("This could interfere with games and other programs, only use it if \
you are doing lots of typing.")+" "+ T("Please note that instead of pressing \
Alt-Gr or Control to access characters on the right-hand side of a key, you \
have to press Alt or Alt+Shift (this is how it was done on an ST)."));
#if defined(SSE_TOS_KEYBOARD_CLICK)
  if(OPTION_HACKS)
  {
    Offset+=Wid+HorizontalSeparation;
    Wid=GetCheckBoxSize(Font,T("Keyboard click")).Width;
    Win=CreateWindow("Button",T("Keyboard click"),mask,page_l
      +HorizontalSeparation+Offset,y,Wid,25,Handle,(HMENU)7301,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,OPTION_KEYBOARD_CLICK,0);
    ToolAddWindow(ToolTip,Win,
      T("This uses address $484, changing before reset is safer - \
MUST be checked for some programs"));
  }
#endif
#if defined(SSE_IKBD_RTC)
  y+=LineHeight;
  Offset=HorizontalSeparation;
  Wid=GetTextSize(Font,T("Battery")).Width;
  CreateWindow("Static",T("Battery"),WS_CHILD,page_l+Offset,y+4,Wid,23,Handle,
    (HMENU)10015,HInstance,NULL);
  Offset+=Wid+HorizontalSeparation;
  mask=WS_CHILD|BS_AUTORADIOBUTTON;
  Wid=GetCheckBoxSize(Font,T("No")).Width;
  Win=CreateWindow("Button",T("No"),mask|WS_GROUP,page_l+Offset,
    y,Wid,25,Handle,(HMENU)7331,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Like on most real STs"));
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Yes")).Width;
  Win=CreateWindow("Button",T("Yes"),mask,page_l+Offset,
    y,Wid,25,Handle,(HMENU)7332,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,
    T("Implies that the 6301 clock is set at the correct time on power on"));
  if(OPTION_HACKS)
  {
    Offset+=Wid+HorizontalSeparation;
    Wid=GetCheckBoxSize(Font,T("Yes, 2000-ready")).Width;
    Win=CreateWindow("Button",T("Yes, 2000-ready"),mask,page_l+Offset,
      y,Wid,25,Handle,(HMENU)7333,HInstance,NULL);
    ToolAddWindow(ToolTip,Win,
      T("Based on TzOk's HW hack to circle around the Y2K bug"));
  }
  SendMessage(GetDlgItem(Handle,7331+OPTION_BATTERY6301),BM_SETCHECK,TRUE,0);
#endif  
  y+=35;
  Offset=HorizontalSeparation;
  CreateWindow("Button",T("Mouse"),WS_CHILD|BS_GROUPBOX,page_l,y,page_w,115,
    Handle,(HMENU)8093,HInstance,NULL);
  y+=20;
  Wid=get_text_width(T("Capture mouse"));
  CreateWindow("Static",T("Capture mouse"),WS_CHILD,
    page_l+Offset,y+4,Wid,21,Handle,(HMENU)209,HInstance,NULL);
  Offset=Wid+HorizontalSeparation;
  mask=WS_CHILD|BS_AUTORADIOBUTTON;
  Wid=GetCheckBoxSize(Font,T("Off")).Width;
  Win=CreateWindow("Button",T("Off"),mask|WS_GROUP,page_l+Offset,
    y,Wid,25,Handle,(HMENU)IDC_RADIO_CAPTURE_MOUSE,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Mouse is free until you click in Steem's window"));
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("On")).Width;
  Win=CreateWindow("Button",T("On"),mask,page_l+Offset,
    y,Wid,25,Handle,(HMENU)(IDC_RADIO_CAPTURE_MOUSE+1),HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Mouse is captured during emulation"));
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Auto")).Width;
  Win=CreateWindow("Button",T("Auto"),mask,page_l+Offset,
    y,Wid,25,Handle,(HMENU)(IDC_RADIO_CAPTURE_MOUSE+2),HInstance,NULL);
  ToolAddWindow(ToolTip,Win,
    T("The behaviour toggles if you use F11 to grab or free the mouse"));
  int current_option=(OPTION_CAPTURE_MOUSE&2) ? 2 : OPTION_CAPTURE_MOUSE;
  SendMessage(GetDlgItem(Handle,IDC_RADIO_CAPTURE_MOUSE+current_option),
    BM_SETCHECK,TRUE,0);
  y+=LineHeight;
  Offset=HorizontalSeparation;
  mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX;
#if 1 // poc, todo, could even use macro
  Win=CreateButton(T("VM-friendly mouse"),IDC_VMMOUSE,10,y,Wid);
#else
  Wid=GetCheckBoxSize(Font,T("VM-friendly mouse")).Width;
  Win=CreateWindow("Button",T("VM-friendly mouse"),mask,
    page_l+10,y,Wid,25,Handle,(HMENU)1035,HInstance,NULL);
#endif
  SendMessage(Win,BM_SETCHECK,OPTION_VMMOUSE,0);
  ToolAddWindow(ToolTip,Win,
T("Alternative mouse handling - cursor not bound to the window. \
Better for virtual machines"));
  y+=LineHeight;
  Wid=GetTextSize(Font,T("Mouse speed")).Width;
  CreateWindow("Static",T("Mouse speed"),WS_VISIBLE|WS_CHILD,
    page_l+HorizontalSeparation,y+4,Wid,23,Handle,(HMENU)995,HInstance,NULL);
  Win=CreateWindow(TRACKBAR_CLASS,"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|TBS_HORZ,
    page_l+Wid+HorizontalSeparation,y,230,27,Handle,(HMENU)1000,HInstance,NULL);
  SendMessage(Win,TBM_SETRANGE,0,MAKELPARAM(1,19));
  SendMessage(Win,TBM_SETPOS,1,mouse_speed);
  SendMessage(Win,TBM_SETLINESIZE,0,1);
  SendMessage(Win,TBM_SETPAGESIZE,0,1);
  SendMessage(Win,TBM_SETTIC,0,10);
#if defined(SSE_HD6301_LL)
  y+=LineHeight+5;
  Win=CreateButton(T("Low-level 6301 emulation (C1)"),IDC_6301,0,y,Wid);
  SendMessage(Win,BM_SETCHECK,OPTION_C1,0);
  ToolAddWindow(ToolTip,Win,
    T("This enables a low-level emulation of the IKBD keyboard chip (using\
 the Sim6xxx code by Arne Riiber, thx dude!), and more precise ACIA timings (\
important for MIDI emulation too)."));
  y+=LineHeight;
#endif
  CreateResetButton(y,
    T("If you change the low-level emulation setting, you should reset the ST"));
#endif    
}


void TOptionBox::CreateSTVideoPage() {
#ifdef WIN32
  int y=10;
  int Wid,Offset,mask;
  HWND Win;
  int h=(OPTION_HACKS && OPTION_ADVANCED)?140:110;
  CreateWindow("Button",T("Monitor"),WS_CHILD|BS_GROUPBOX,
    page_l,y,page_w,h,Handle,(HMENU)8093,HInstance,NULL);
  y+=20;
  Offset=10;
  mask=WS_CHILD|BS_AUTORADIOBUTTON;
  Wid=GetCheckBoxSize(Font,T("Colour")).Width;
  Win=CreateWindow("Button",T("Colour"),mask|WS_GROUP,page_l+Offset,
    y,Wid,25,Handle,(HMENU)IDC_RADIO_STSCREEN,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Low/Med Resolution, 50/60hz"));
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Monochrome")).Width;
  Win=CreateWindow("Button",T("Monochrome"),mask,page_l+Offset,
    y,Wid,25,Handle,(HMENU)(IDC_RADIO_STSCREEN+1),HInstance,NULL);
  ToolAddWindow(ToolTip,Win,"High Resolution, 71hz");
  int monitor_sel=NewMonitorSel;
  if(monitor_sel<0) 
    monitor_sel=GetCurrentMonitorSel();
#ifndef NO_CRAZY_MONITOR
  ADVANCED_HACK_BEGIN
    Offset+=Wid+HorizontalSeparation;
    Wid=GetCheckBoxSize(Font,T("Extended")).Width;
    Win=CreateWindow("Button",T("Extended"),mask,page_l+Offset,
      y,Wid,25,Handle,(HMENU)(IDC_RADIO_STSCREEN+2),HInstance,NULL);
    ToolAddWindow(ToolTip,Win,"Hack - TOS 1.04 or beyond");
    y+=LineHeight;
    Offset=10;
    Wid=get_text_width(T("Extended monitor"));
    CreateWindow("Static",T("Extended monitor"),WS_CHILD,
      page_l+Offset,y+4,Wid,21,Handle,(HMENU)209,HInstance,NULL);
    Offset+=Wid+HorizontalSeparation;
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
      page_l+Offset,y,page_w-Offset-10,200,Handle,(HMENU)IDC_EXTENDED_MONITOR,
      HInstance,NULL);
    for(int n=0;n<EXTMON_RESOLUTIONS;n++)
      CBAddString(Win,T("Extended Monitor At")+" "+extmon_res[n][0]+"x"
        +extmon_res[n][1]+"x"+extmon_res[n][2]);
  ADVANCED_END
#endif
  y+=LineHeight;
  int rbut=monitor_sel;
  if(rbut>1)
    rbut=2;
  SendMessage(GetDlgItem(Handle,IDC_RADIO_STSCREEN+rbut),BM_SETCHECK,TRUE,0);
  Wid=get_text_width(T("Borders"));
  Offset=10;
  CreateWindow("Static",T("Borders"),WS_CHILD,
    page_l+Offset,y+4,Wid,21,Handle,(HMENU)209,HInstance,NULL);
  Offset+=Wid+HorizontalSeparation;
  mask=WS_CHILD|BS_AUTORADIOBUTTON;
  Wid=GetCheckBoxSize(Font,T("Off")).Width;
  Win=CreateWindow("Button",T("Off"),mask|WS_GROUP,page_l+Offset,
    y,Wid,25,Handle,(HMENU)17360,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Fine for normal apps and games"));
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Normal")).Width;
  Win=CreateWindow("Button",T("Normal"),mask,page_l+Offset,
    y,Wid,25,Handle,(HMENU)17361,HInstance,NULL);
  ToolAddWindow(ToolTip,Win,T("Typical ST monitor"));
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Large")).Width;
  Win=CreateWindow("Button",T("Large"),mask,page_l+Offset,
    y,Wid,25,Handle,(HMENU)17362,HInstance,NULL);
  //ToolAddWindow(ToolTip,Win,T(""));
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Max")).Width;
  Win=CreateWindow("Button",T("Max"),mask,page_l+Offset,
    y,Wid,25,Handle,(HMENU)17363,HInstance,NULL);
  //ToolAddWindow(ToolTip,Win,T("Low/Med Resolution"));
  SendMessage(GetDlgItem(Handle,17360+border),BM_SETCHECK,TRUE,0);
  y+=LineHeight;
  mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX;
  Wid=GetCheckBoxSize(Font,T("ST Aspect Ratio")).Width;
  Offset=10;
  Win=CreateWindow("Button",T("ST Aspect Ratio"),mask,
    page_l+Offset,y,Wid,25,Handle,(HMENU)IDC_ST_AR,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_ST_ASPECT_RATIO,0);
  ToolAddWindow(ToolTip,Win,
    T("Reproduces the familiar vertical distortion on standard colour screens"));
  Offset+=Wid+HorizontalSeparation;
  Wid=GetCheckBoxSize(Font,T("Scanlines")).Width;
  Win=CreateWindow("Button",T("Scanlines"),mask,
    page_l+Offset,y,Wid,25,Handle,(HMENU)IDC_SCANLINES,HInstance,NULL);
  SendMessage(Win,BM_SETCHECK,OPTION_SCANLINES,0);
  ToolAddWindow(ToolTip,Win,T("Reproduces scanlines of colour screens"));
#if defined(SSE_VID_VSYNC_WINDOW) // DirectDraw-only
  // We reproduce the option here, it's on Display too
//  ADVANCED_BEGIN // is it advanced?
    Offset+=Wid+HorizontalSeparation;
    Wid=GetCheckBoxSize(Font,T("VSync")).Width;
    Win=CreateWindow("Button",T("VSync"),mask,
      page_l+Offset,y,Wid,25,Handle,(HMENU)1033,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,OPTION_WIN_VSYNC,0);
//  ADVANCED_END
#endif
  y+=LineHeight+5;
  ADVANCED_BEGIN
    CreateWindow("Button",T("Overscan emulation"),WS_CHILD|BS_GROUPBOX,
      page_l,y,page_w,80,Handle,(HMENU)8093,HInstance,NULL);
    y+=20;
#if defined(SSE_HARDWARE_OVERSCAN)
    Wid=GetTextSize(Font,T("Hardware")).Width;
    mask=WS_CHILD|BS_AUTORADIOBUTTON;
    if(IS_STE)
      mask|=WS_DISABLED;
    Offset=10;
    CreateWindow("Static",T("Hardware"),WS_CHILD|(mask&WS_DISABLED),
      page_l+Offset,y+4,Wid,23,Handle,(HMENU)10015,HInstance,NULL);
    Offset+=Wid+HorizontalSeparation;
    Wid=GetCheckBoxSize(Font,T("None")).Width;
    Win=CreateWindow("Button",T("None"),mask|WS_GROUP,page_l+Offset,
      y,Wid,25,Handle,(HMENU)IDC_RADIO_HWOVERSCAN,HInstance,NULL);
    EasyStr hint=T("Those devices trick the DE signal to get permanent overscan.\
 Need drivers on the ST side.");
    ToolAddWindow(ToolTip,Win,hint);
    Offset+=Wid+HorizontalSeparation;
    Wid=GetCheckBoxSize(Font,T("LaceScan")).Width;
    Win=CreateWindow("Button",T("LaceScan"),mask,page_l+Offset,
      y,Wid,25,Handle,(HMENU)(IDC_RADIO_HWOVERSCAN+1),HInstance,NULL);
    ToolAddWindow(ToolTip,Win,hint);
    Offset+=Wid+HorizontalSeparation;
    Wid=GetCheckBoxSize(Font,T("AutoSwitch")).Width;
    Win=CreateWindow("Button",T("AutoSwitch"),mask,page_l+Offset,
      y,Wid,25,Handle,(HMENU)(IDC_RADIO_HWOVERSCAN+2),HInstance,NULL);
    ToolAddWindow(ToolTip,Win,hint);
    SendMessage(GetDlgItem(Handle,IDC_RADIO_HWOVERSCAN+OPTION_HWOVERSCAN),
      BM_SETCHECK,TRUE,0);
    y+=LineHeight;
#endif
    Wid=GetTextSize(Font,T("Software")).Width;
    Offset=10;
    CreateWindow("Static",T("Software"),WS_CHILD,page_l+Offset,y+4,Wid,23,
      Handle,(HMENU)10015,HInstance,NULL);
    Offset+=Wid+HorizontalSeparation;
    mask=WS_CHILD|BS_AUTORADIOBUTTON;
    Wid=GetCheckBoxSize(Font,T("None")).Width;
    Win=CreateWindow("Button",T("None"),mask|WS_GROUP,page_l+Offset,
      y,Wid,25,Handle,(HMENU)7326,HInstance,NULL);
    ToolAddWindow(ToolTip,Win,
      T("This is the correct choice for most applications and games"));
    Offset+=Wid+HorizontalSeparation;
    Wid=GetCheckBoxSize(Font,T("High-level (C2)")).Width;
    Win=CreateWindow("Button",T("High-level (C2)"),mask,page_l+Offset,
      y,Wid,25,Handle,(HMENU)7327,HInstance,NULL);
    ToolAddWindow(ToolTip,Win,T("This should run almost all known demos"));
    Offset+=Wid+HorizontalSeparation;
    Wid=GetCheckBoxSize(Font,T("Low-level (C3)")).Width;
    Win=CreateWindow("Button",T("Low-level (C3)"),mask,page_l+Offset,
      y,Wid,25,Handle,(HMENU)7328,HInstance,NULL);
    // Windows shows the tip only if the control is enabled
    ToolAddWindow(ToolTip,Win,
      T("It uses more CPU for, hopefully, maximum accuracy"));
    SendMessage(GetDlgItem(Handle,7326+OPTION_VLE),BM_SETCHECK,TRUE,0);
    y+=LineHeight+5;
  ADVANCED_END
  ADVANCED_BEGIN
    CreateWindow("Button",T("Wakeup"),WS_CHILD|BS_GROUPBOX,
      page_l,y,page_w,50+15,Handle,(HMENU)8093,HInstance,NULL);
    y+=20;
    Offset=10;
    long disable=(IS_STE)?WS_DISABLED:0;
    Wid=get_text_width(T("GLU"));
    CreateWindow("Static",T("GLU"),WS_CHILD,
      page_l+Offset,y+4,Wid,21,Handle,(HMENU)209,HInstance,NULL);
    Offset+=Wid+HorizontalSeparation;
    HWND hEdit0=CreateWindow("Edit",NULL,WS_CHILD|WS_TABSTOP|WS_BORDER|disable,
      page_l+Offset,y,40,21,Handle,(HMENU)IDC_GLU_WAKEUP0,HInstance,NULL);
    Win=CreateWindow(UPDOWN_CLASS,NULL,
      WS_CHILD|WS_TABSTOP|UDS_ARROWKEYS|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|disable,
      0,0,0,0,Handle,(HMENU)IDC_GLU_WAKEUP,HInstance,NULL);
    SendMessage(Win,UDM_SETBUDDY,(WPARAM)hEdit0,0);
    SendMessageW(Win,UDM_SETRANGE,0,MAKELPARAM(4,1));
    SendMessageW(Win,UDM_SETPOS32,0,Mmu.WS[OPTION_WS]); // triggers option!
    Offset+=40+HorizontalSeparation;
    Wid=get_text_width(T("Shifter"));
    CreateWindow("Static",T("Shifter"),WS_CHILD,
      page_l+Offset,y+4,Wid,21,Handle,(HMENU)209,HInstance,NULL);
    Offset+=Wid+HorizontalSeparation;
    HWND hEdit=CreateWindow("Edit",NULL,WS_CHILD|WS_TABSTOP|WS_BORDER,
      page_l+Offset,y,35,21,Handle,(HMENU)IDC_SHIFTER_WU0,HInstance,NULL);
    Win=CreateWindow(UPDOWN_CLASS,NULL,
      WS_CHILD|WS_TABSTOP|UDS_ARROWKEYS|UDS_SETBUDDYINT|UDS_ALIGNRIGHT,
      0,0,0,0,Handle,(HMENU)IDC_SHIFTER_WU,HInstance,NULL);
    SendMessage(Win,UDM_SETBUDDY,(WPARAM)hEdit,0);
    SendMessageW(Win,UDM_SETRANGE,0,MAKELPARAM(SHIFTER_MAX_WU_SHIFT,-SHIFTER_MAX_WU_SHIFT));
    SendMessageW(Win,UDM_SETPOS32,0,OPTION_SHIFTER_WU);
    Offset+=50; 
    y-=10;
    Win=CreateButton(T("Random on boot"),IDC_RANDOM_WU,Offset,y,Wid);
    SendMessage(Win,BM_SETCHECK,OPTION_RANDOM_WU,0);
    ToolAddWindow(ToolTip,Win,T("This affects both the GLUE and the Shifter"));
    y+=25;
    mask=WS_CHILD|WS_TABSTOP|BS_CHECKBOX;
    Wid=GetCheckBoxSize(Font,T("Unstable Shifter")).Width;
    Win=CreateWindow("Button",T("Unstable Shifter"),mask,
      page_l+Offset,y,Wid,25,Handle,(HMENU)IDC_UNSTABLE_SHIFTER,HInstance,NULL);
    SendMessage(Win,BM_SETCHECK,OPTION_UNSTABLE_SHIFTER,0);
    ToolAddWindow(ToolTip,Win,
  T("Used for some aspects of high or low level software overscan emulation"));
  ADVANCED_END
  if(OPTION_HACKS)
  {
    y+=LineHeight+5;
    CreateWindow("Button",T("Hacks"),WS_CHILD|BS_GROUPBOX,
      page_l,y,page_w,45,Handle,(HMENU)8093,HInstance,NULL);
    y+=12;
    Offset=10;
    Wid=get_text_width(T("Frequency"));
    CreateWindow("Static",T("Frequency"),WS_CHILD,
      page_l+Offset,y+4,Wid,21,Handle,(HMENU)209,HInstance,NULL);
    Offset+=Wid+HorizontalSeparation;
    Win=CreateWindow("Combobox","",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST,
      page_l+Offset,y,50,200,Handle,(HMENU)IDC_VID_FREQUENCY,HInstance,NULL);
    CBAddString(Win,T("50Hz"),MAKELONG(Glue.Freq[Glue.FREQ_50],0));
    CBAddString(Win,T("60Hz"),MAKELONG(Glue.Freq[Glue.FREQ_60],0));
    CBAddString(Win,T("71Hz"),MAKELONG(Glue.Freq[Glue.FREQ_71],0));
#if defined(SSE_OPTION_FASTBLITTER)
    Offset+=60;
    Win=CreateButton("Fast Blitter",IDC_FASTBLITTER,Offset,y,Wid);
    SendMessage(Win,BM_SETCHECK,OPTION_FASTBLITTER,0);
    ToolAddWindow(ToolTip,Win,T("The way it should have been!"));
#endif
  }
  else
    y+=5;
  y+=LineHeight+5;
  CreateResetButton(y,"");
  UpdateSTVideoPage();
#endif  
}


void TOptionBox::UpdateSTVideoPage() {
#ifdef WIN32  
  HWND Win=GetDlgItem(Handle,IDC_RADIO_STSCREEN);
  if(Win==NULL)
    return;
  int monitor_sel=NewMonitorSel; // 0 = colour
  if(monitor_sel<0) 
    monitor_sel=GetCurrentMonitorSel();
  SendMessage(GetDlgItem(Handle,IDC_EXTENDED_MONITOR),CB_SETCURSEL,
    (monitor_sel>1) ? (monitor_sel-2) : ((WPARAM)-1),0);
  for(int i=0;i<4;i++)
    EnableControl(IDC_RADIO_BORDER+i,(FullScreen==0) && (i<2||!monitor_sel)); 
  EnableControl(IDC_ST_AR,(monitor_sel==0));
  EnableControl(IDC_SCANLINES,(monitor_sel==0));
  EnableControl(IDC_RADIO_SWOVERSCAN+2,(SSEConfig.Stvl!=0));
  Win=GetDlgItem(Handle,IDC_VID_FREQUENCY);
  CBSelectItemWithData(Win,Glue.previous_video_freq);
  Win=GetDlgItem(Handle,IDC_SHIFTER_WU);
  if(Win)
  {
    SendMessageW(Win,UDM_SETPOS32,0,OPTION_SHIFTER_WU);
    Win=GetDlgItem(Handle,IDC_UNSTABLE_SHIFTER);
    SendMessage(Win,BM_SETCHECK,OPTION_UNSTABLE_SHIFTER,0);
    Win=GetDlgItem(Handle,IDC_GLU_WAKEUP0);
    EnableWindow(Win,IS_STF);
    Win=GetDlgItem(Handle,IDC_GLU_WAKEUP);
    EnableWindow(Win,IS_STF);
    SendMessageW(Win,UDM_SETPOS32,0,Mmu.WS[OPTION_WS]); // triggers option!
  }
#if defined(SSE_OPTION_FASTBLITTER)
  Win=GetDlgItem(Handle,IDC_FASTBLITTER);
  if(Win)
    EnableWindow(Win,SSEConfig.Blitter);
#endif

#endif//WIN32  
}
