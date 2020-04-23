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

DOMAIN: File
FILE: loadsave.cpp
DESCRIPTION: Lots of functions to deal with loading and saving various data
to and from files. This includes handling memory snapshots and steem.ini.
---------------------------------------------------------------------------*/


#include "pch.h"
#pragma hdrstop

#include <easycompress.h>
#include <computer.h>
#include <diskman.h>
#include <harddiskman.h>
#include <mymisc.h>
#include <gui.h>
#include <translate.h>
#include <loadsave.h>
#include <draw.h>
#include <dataloadsave.h>
#include <osd.h>
#include <debugger.h>
#include <debugger_trace.h>

void LoadSnapShotChangeDisks(Str NewDisk[2],Str NewDiskInZip[2],
                                          Str NewDiskName[2]) {
 //TRACE("LoadSnapShotChangeDisks\n");
  int save_mediach[2]={floppy_mediach[0],floppy_mediach[1]};
  for(BYTE disk=0;disk<2;disk++)
  {
    if(NewDisk[disk].IsEmpty())
      DiskMan.EjectDisk(disk);
    else 
    {
      bool InsertedDisk=(FloppyDrive[disk].SetDisk(NewDisk[disk],
        NewDiskInZip[disk])==0);
      if(InsertedDisk==0) 
      {
        NewDisk[disk]=EasyStr(GetFileNameFromPath(NewDisk[disk]));
        if(FloppyDrive[disk].SetDisk(DiskMan.HomeFol+SLASH+NewDisk[disk],
          NewDiskInZip[disk])) 
        {
          if(FloppyDrive[disk].SetDisk(RunDir+SLASH+NewDisk[disk],
            NewDiskInZip[disk])) 
          {
            int Ret=Alert(T("When this snapshot was taken there was a disk \
called")+" "+NewDisk[disk]+" "+T("in ST drive")+" "+char('A'+disk)+". "+
              T("Steem cannot find this disk. Having different disks in the \
drives after loading the snapshot could cause errors.")+
              "\n\n"+T("Do you want to find this disk or its equivalent?"),
              T("Cannot Find Disk"),MB_YESNOCANCEL|MB_ICONQUESTION);
            if(Ret==IDYES) 
            {
              EasyStr Fol=DiskMan.HomeFol,NewerDisk;
              for(;;) 
              {
#ifdef WIN32
                NewerDisk=FileSelect(StemWin,T("Locate")+" "+NewDisk[disk],Fol,
                                      FSTypes(2,NULL),1,true,"st");
#endif
#ifdef UNIX
                fileselect.set_corner_icon(&Ico16,ICO16_DISK);
                NewerDisk=fileselect.choose(XD,Fol,GetFileNameFromPath(NewDisk[disk]),
                                        T("Locate")+" "+NewDisk[disk],FSM_LOAD|FSM_LOADMUSTEXIST,
                                        diskfile_parse_routine,".st");
#endif
                if(NewerDisk.IsEmpty()) 
                {
                  if(Alert(T("Do you want to continue trying to load this \
snapshot?"),T("Carry On Regardless?"), MB_YESNO|MB_ICONQUESTION)==IDNO)
                    throw 1;
                  break;
                }
                else 
                {
                  if(FloppyDrive[disk].SetDisk(NewerDisk)) 
                  {
                    Ret=Alert(T("The disk image you selected is not valid. Do \
you want to try again? Click on cancel to give up trying to load this snapshot."),
T("Invalid Disk Image"),MB_YESNOCANCEL|MB_ICONEXCLAMATION);
                    if(Ret==IDCANCEL)
                      throw 1;
                    else if(Ret==IDYES) 
                    {
                      Fol=NewerDisk;
                      RemoveFileNameFromPath(Fol,REMOVE_SLASH);
                    }
                    else
                      break;
                  }
                  else 
                  {
                    InsertedDisk=true;
                    break;
                  }
                }
              }
            }
            else if(Ret==IDCANCEL)
              throw 1;
          }
          else
            InsertedDisk=true;
        }
        else
          InsertedDisk=true;
      }
      if(InsertedDisk) 
      {
        DiskMan.InsertHistoryAdd(disk,NewDiskName[disk],
          FloppyDrive[disk].GetDisk(),NewDiskInZip[disk]);
        FloppyDisk[disk].DiskName=NewDiskName[disk];
        if(DiskMan.IsVisible())
          DiskMan.InsertDisk(disk,FloppyDisk[disk].DiskName,
            FloppyDrive[disk].GetDisk(),true,0,NewDiskInZip[disk]);
        floppy_mediach[disk]=save_mediach[disk];
        FloppyDrive[disk].Restore(disk);
      }
    }
  }
}


void LoadSnapShotChangeCart(Str NewCart) {
  if(NewCart.Empty()) 
  {
    // Remove cart? Yes
    if(cart_save)
      cart=cart_save;
    cart_save=NULL;
    if(cart)
      delete[] cart;
    cart=NULL;
    CartFile="";
    return;
  }
  if(load_cart(NewCart)==0) 
  {
    CartFile=NewCart;
    return;
  }
  Str NewCartName=GetFileNameFromPath(NewCart);
  char *dot=strrchr(NewCartName,'.');
  if(dot) 
    *dot=0;
  Str Fol=NewCart;
  RemoveFileNameFromPath(Fol,REMOVE_SLASH);
  if(GetFileAttributes(Fol)==0xffffffff) 
  {
    Fol=OptionBox.LastCartFile;
    RemoveFileNameFromPath(Fol,REMOVE_SLASH);
  }
  int Ret=Alert(T("When this snapshot was taken there was a cartridge inserted \
called")+" "+NewCartName+". "+T("Steem cannot find this cartridge, the snapshot\
 may not work properly without it.")+"\n\n"+T("Do you want to find this \
cartridge?"),T("Cannot Find Cartridge"),MB_YESNOCANCEL | MB_ICONQUESTION);
  if(Ret==IDCANCEL)
    throw 1;
  else if(Ret==IDYES)
  {
    Str NewerCart;
    for(;;)
    {
#ifdef WIN32
      NewerCart=FileSelect(StemWin,T("Locate")+" "+NewCartName,Fol,
        FSTypes(0,T("ST Cartridge Images").Text,"*.stc",NULL),1,true,"stc");
#endif
#ifdef UNIX
      fileselect.set_corner_icon(&Ico16,ICO16_CHIP);
      NewerCart=fileselect.choose(XD,Fol,NewCartName,T("Locate")+" "+NewCartName,FSM_LOAD | FSM_LOADMUSTEXIST,
                              cartfile_parse_routine,".stc");
#endif
      if(NewerCart.IsEmpty()) 
      {
        if(Alert(T("Do you want to continue trying to load this snapshot?"),
          T("Carry On Regardless?"),MB_YESNO | MB_ICONQUESTION)==IDNO)
          throw 1;
        break;
      }
      else
      {
        if(load_cart(NewerCart)) 
        {
          Ret=Alert(T("The cartridge you selected is not valid. Do you want to \
try again? Click on cancel to give up trying to load this snapshot."),
            T("Invalid Cartridge Image"),MB_YESNOCANCEL|MB_ICONEXCLAMATION);
          if(Ret==IDCANCEL)
            throw 1;
          else if(Ret==IDYES) 
          {
            Fol=NewerCart;
            RemoveFileNameFromPath(Fol,REMOVE_SLASH);
          }
          else
            break;
        }
        else 
        {
          CartFile=NewerCart;
          break;
        }
      }
    }
  }
}


void LoadSnapShotChangeTOS(Str NewROM,int NewROMVer,int NewROMCountry) {
  bool Fail=0;
  if(load_TOS(NewROM)) 
  {
/*  Steem couldn't load this precise file.
    Before prompting user, have a go at matching a TOS with the same
    version number.
*/
    if(OPTION_HACKS)
    {
      DirSearch ds;
      if(ds.Find(RunDir+SLASH+"*.*")) 
      {
        EasyStr Path;
        do {
          Path=Tos.GetNextTos(ds);
          if(has_extension_list(Path,"IMG","ROM",NULL)) {
            WORD Ver,Date;
            BYTE Country,Recognised;
            Tos.GetTosProperties(Path,Ver,Country,Date,Recognised);
            if(Ver==NewROMVer && Country==NewROMCountry)
            {
              ROMFile=Path;
              TRACE_INIT("preselect TOS %s\n",ROMFile.Text);
              if(load_TOS(ROMFile))
                throw 1; // 0 = OK
            }
          }
        } while(ds.Next());
        ds.Close();
      }
    }
    EasyStr NewROMVersionInfo;
    if(NewROMVer<=0x700) 
      NewROMVersionInfo=Str(" (")+T("version number")+" "+HEXSl(NewROMVer,4)+")";
    int Ret=Alert(T("When this snapshot was taken the TOS image being used was ")
      +NewROM+NewROMVersionInfo+". "+T("This file cannot now be used, it is \
either missing or corrupt. Do you want to find an equivalent TOS image, without \
doing so you cannot load this snapshot."),T("Cannot Use TOS Image"),MB_YESNO
      |MB_ICONEXCLAMATION);
    if(Ret==IDNO) 
      throw 1;
    EasyStr ROMName=GetFileNameFromPath(NewROM);
    for(;;)
    {
      EasyStr Title=T("Locate")+" "+ROMName+NewROMVersionInfo;
#ifdef WIN32
      NewROM=FileSelect(StemWin,Title,RunDir,FSTypes(3,NULL),1,true,"img");
#endif
#ifdef UNIX
      fileselect.set_corner_icon(&Ico16,ICO16_CHIP);
      NewROM=fileselect.choose(XD,RunDir,ROMName,Title,FSM_LOAD|FSM_LOADMUSTEXIST,
        romfile_parse_routine,".img");
#endif
      if(NewROM.IsEmpty()) 
      {
        Fail=true;
        break;
      }
      if(load_TOS(NewROM)) 
      {
        Ret=Alert(T("This TOS image is corrupt! Do you want to try again?"),
          T("Cannot Use TOS Image"),MB_YESNO|MB_ICONEXCLAMATION);
        if(Ret==IDNO) 
        {
          Fail=true;
          break;
        }
      }
      else
      {
        // Check version number
        if(NewROMVer>0x700) 
          break;         // No version number saved
        if(NewROMVer==tos_version) 
          break;
        Ret=Alert(T("This TOS image's version number doesn't match. Do you \
want to choose a different one?"),T("TOS Image Version Different"),
          MB_YESNOCANCEL|MB_ICONQUESTION);
        if(Ret==IDCANCEL||Ret==IDNO) 
        {
          if(Ret==IDCANCEL) 
            Fail=true;
          break;
        }
      }
    }
  }
  if(Fail)
    throw 1;
  else
    ROMFile=NewROM;
}


void LoadSaveChangeNumFloppies(int NumFloppyDrives) {
  DiskMan.SetNumFloppies((BYTE)NumFloppyDrives);
}


void AddSnapShotToHistory(char *FilNam) {
  for(int n=0;n<10;n++) 
  {
    if(IsSameStr_I(FilNam,StateHist[n])) 
      StateHist[n]="";
  }
  for(int n=0;n<10;n++) 
  {
    bool NoMore=true;
    for(int i=n;i<10;i++) 
    {
      if(StateHist[i].NotEmpty()) 
      {
        NoMore=0;
        break;
      }
    }
    if(NoMore)
      break;
    if(StateHist[n].Empty()) 
    {
      for(int i=n;i<9;i++)
        StateHist[i]=StateHist[i+1];
      n--;
    }
  }
  for(int n=9;n>0;n--)
    StateHist[n]=StateHist[n-1];
  StateHist[0]=FilNam;
}


bool LoadSnapShot(char *FilNam,bool AddToHistory,bool ShowErrorMess,
                  bool ChangeDisks) {
  // return true if successful
  //TRACE2("Loading %s\n",FilNam);
  //TRACE("LoadSnapShot(%s %d %d %d\n",FilNam,AddToHistory,ShowErrorMess,ChangeDisks);
#ifndef ONEGAME
  int Failed=2,Version=0;
  bool FileError=0;
  if(Exists(FilNam)==0)
  {
    FileError=true;
    TRACE_INIT("File %s doesn't exist\n",FilNam);
  }
  if(FileError==0) 
  {
    bool LoadingResetBackup=IsSameStr_I(FilNam,WriteDir+SLASH
      +"auto_reset_backup.sts");
    bool LoadingLoadSnapBackup=IsSameStr_I(FilNam,WriteDir+SLASH
      +"auto_loadsnapshot_backup.sts");
    if(ChangeDisks && LoadingResetBackup==0&&LoadingLoadSnapBackup==0) 
    { // Don't backup on auto load
      DeleteFile(WriteDir+SLASH+"auto_reset_backup.sts");
      SaveSnapShot(WriteDir+SLASH+"auto_loadsnapshot_backup.sts",-1,0);
    }
    reset_st(RESET_COLD|RESET_STOP|RESET_NOCHANGESETTINGS|RESET_NOBACKUP);
    FILE *f=fopen(FilNam,"rb");
    if(f) 
    {
      Failed=LoadSaveAllStuff(f,LS_LOAD,-1,ChangeDisks,&Version);
      TRACE_INIT("Load snapshot \"%s\" v%d ERR:%d\n",FilNam,Version,Failed);
      if(Failed==0) 
      {
        Failed=int((EasyUncompressToMem(STMem+MEM_EXTRA_BYTES,mem_len,f)!=0) ? 2:0);
        if(extended_monitor)
          Tos.HackMemoryForExtendedMonitor();
      }
      fclose(f);
    }
    else
    {
      TRACE_INIT("File open error on %s\n",FilNam);
      FileError=true;
    }
  }
  if(FileError) 
  {
    Alert(T("Cannot open the snapshot file:")+"\n\n"+FilNam,
      T("Load Memory Snapshot Failed"),MB_ICONEXCLAMATION);
    return 0;
  }
#else
  reset_st(RESET_COLD | RESET_STOP | RESET_NOCHANGESETTINGS | RESET_NOBACKUP);
  BYTE *p=(BYTE*)FilNam;
  int Failed=LoadSaveAllStuff(p,LS_LOAD,-1,ChangeDisks,&Version);
  if (Failed==0) Failed=EasyUncompressToMemFromMem(STMem+MEM_EXTRA_BYTES,mem_len,p);
  if (Failed) Failed=1; 
#endif
  if(Failed==0)
  {
    if(AddToHistory) 
      AddSnapShotToHistory(FilNam);
    LoadSnapShotUpdateVars(Version);
    OptionBox.NewMemConf0=-1;
    OptionBox.NewMonitorSel=-1;
    OptionBox.NewROMFile="";
    OptionBox.MachineUpdateIfVisible();
    CheckResetIcon();
    CheckResetDisplay();
    DEBUG_ONLY(update_register_display(true); )
    TRACE2("Load snapshot %s v%d\n",FilNam,Version);
    Debug.TraceGeneralInfos(TDebug::RESET);
    FLUSH_TRACE;
  }
  else
  {
    if(Failed>1&&ShowErrorMess)
      //Alert(T("Cannot load the snapshot, it is corrupt."),
      Alert(T("Snapshot incompatible with this version of Steem"), // snapshot may be fine
        T("Load Memory Snapshot Failed"),MB_ICONEXCLAMATION);
    else // crash likely on reset
      reset_st(RESET_COLD|RESET_STOP|RESET_CHANGESETTINGS|RESET_NOBACKUP);
  }
  REFRESH_STATUS_BAR;
  OptionBox.SSEUpdateIfVisible();
  Tos.CheckSTTypeAndTos();
  return (!Failed);
}


void SaveSnapShot(char *FilNam,int Version,bool AddToHistory) {
#ifndef ONEGAME
  FILE *f=fopen(FilNam,"wb");
  if(f!=NULL)
  {
#ifdef SSE_DEBUG
    int Failed=LoadSaveAllStuff(f,LS_SAVE,Version,0,&Version);
    TRACE("Save snapshot \"%s\" v%d ERR:%d\n",FilNam,Version,Failed);
#else
    LoadSaveAllStuff(f,LS_SAVE,Version,0,&Version);
#endif
    EasyCompressFromMem(STMem+MEM_EXTRA_BYTES,mem_len,f);
    fclose(f);
    if(AddToHistory) 
      AddSnapShotToHistory(FilNam);
  }
#endif
}


#ifdef ENABLE_LOGFILE

void load_logsections() {
  FILE *f=fopen(WriteDir+SLASH "logsection.dat","rb");
  if(f!=NULL) 
  {
    for(int n=0;n<100;n++) 
      logsection_enabled[n]=true;
    char tb[50];
    for(;;) 
    {
      if(fgets(tb,49,f)==0) 
        break;
      if(tb[0]==0) 
        break;
      int n=atoi(tb);
      if(n>0&&n<100) 
        logsection_enabled[n]=false;
    }
    fclose(f);
  }
  if(logsection_enabled[LOGSECTION_CPU]) 
    log_cpu_count=CPU_INSTRUCTIONS_TO_LOG;
}

#endif




#ifdef DEBUG_BUILD

#define LITTLE_REFACT

struct TMemBrowLoad {
  MEM_ADDRESS ad;
  ETypeDispType type;
  int x,y,w,h;
  int n_cols,col_w[20];
  char name[256];
};

#endif


void LoadState(TConfigStoreFile *pCSF) {
  // this is called only once by Initialise() in main.cpp
  LoadAllDialogData(true,globalINIFile,NULL,pCSF);
  LOG_TO(LOGSECTION_INIT,"STARTUP: Finished LoadAllDialogData");

#ifdef DEBUG_BUILD

#if !defined(LITTLE_REFACT)
  DynamicArray<TMemBrowLoad> browsers;
#endif

  int dru_combo_idx=0;
  Str dru_edit;
  debug_ads.DeleteAll();
  for(int n=0;;n++) 
  {
    TDebugAddress da;
    da.ad=pCSF->GetInt("Debug Addresses",Str("Address")+n,0xffffffff);
    if(da.ad==0xffffffff) 
      break;
    da.mode=pCSF->GetInt("Debug Addresses",Str("Mode")+n,0);
    da.bwr=pCSF->GetInt("Debug Addresses",Str("BWR")+n,0);
    da.mask[0]=(WORD)pCSF->GetInt("Debug Addresses",Str("MaskW")+n,0xffff);
    da.mask[1]=(WORD)pCSF->GetInt("Debug Addresses",Str("MaskR")+n,0xffff);
    strcpy(da.name,pCSF->GetStr("Debug Addresses",Str("Name")+n,""));
    debug_ads.Add(da);
  }
  TWinPositionData wpd;
  GetWindowPositionData(DWin,&wpd);
  MoveWindow(DWin,pCSF->GetInt("Debug Options","Boiler Left",wpd.Left),pCSF
    ->GetInt("Debug Options","Boiler Top",wpd.Top),pCSF->GetInt("Debug Options",
    "Boiler Width",wpd.Width),pCSF->GetInt("Debug Options","Boiler Height",
    wpd.Height),0);
  GetWindowPositionData(trace_window_handle,&wpd);
  MoveWindow(trace_window_handle,pCSF->GetInt("Debug Options","Trace Left",
    wpd.Left),pCSF->GetInt("Debug Options","Trace Top",wpd.Top),wpd.Width,
    wpd.Height,0);

#if !defined(LITTLE_REFACT)
  for(int n=0;n<MAX_MEMORY_BROWSERS;n++) 
  {
    Str Key=Str("Browser")+n+" ";
    TMemBrowLoad b;
    b.x=pCSF->GetInt("Debug Browsers",Key+"Left",-300);
    if(b.x==-300) 
      break;
    b.y=pCSF->GetInt("Debug Browsers",Key+"Top",0);
    b.w=pCSF->GetInt("Debug Browsers",Key+"Width",100);
    b.h=pCSF->GetInt("Debug Browsers",Key+"Height",100);
    b.ad=pCSF->GetInt("Debug Browsers",Key+"Address",0);
    strcpy(b.name,pCSF->GetStr("Debug Browsers",Key+"Name","Memory"));
    b.type=(ETypeDispType)pCSF->GetInt("Debug Browsers",Key+"Type",0);
    b.n_cols=0;
    for(int m=0;m<20;m++) 
    {
      b.col_w[m]=pCSF->GetInt("Debug Browsers",Key+"Column"+m,-1);
      if(b.col_w[m]<0) 
        break;
      b.n_cols++;
    }
    browsers.Add(b);
  }
#endif

  breakpoint_mode=pCSF->GetInt("Debug Options","Breakpoint Mode",breakpoint_mode);
  monitor_mode=pCSF->GetInt("Debug Options","Monitor Mode",monitor_mode);
  mem_browser::ex_style=pCSF->GetInt("Debug Options","Browsers on Taskbar",mem_browser::ex_style);
  logging_suspended=pCSF->GetBool("Debug Options","Suspend Logging",logging_suspended);
  Debug.tracing_suspended=pCSF->GetByte("Debug Options","Suspend Tracing",Debug.tracing_suspended);
  Debug.log_in_trace=pCSF->GetByte("Debug Options","log_in_TRACE",Debug.log_in_trace);
  debug_wipe_log_on_reset=pCSF->GetBool("Debug Options","Wipe Log On Reset",debug_wipe_log_on_reset);
  LogViewProg=pCSF->GetStr("Debug Options","Log Viewer",LogViewProg);
  crash_notification=pCSF->GetInt("Debug Options","Crash Notify",crash_notification);
  boiler_show_stack_display(pCSF->GetInt("Debug Options","Stack Display",0));
  debug_gun_pos_col=pCSF->GetInt("Debug Options","Gun Display Colour",debug_gun_pos_col);
  trace_show_window=pCSF->GetBool("Debug Options","Trace Show",trace_show_window);
  dru_combo_idx=pCSF->GetInt("Debug Options","Run Until",dru_combo_idx);
  dru_edit=pCSF->GetStr("Debug Options","Run Until Text",dru_edit);
  debug_monospace_disa=pCSF->GetBool("Debug Options","Monospace Disa",debug_monospace_disa);
  debug_uppercase_disa=pCSF->GetBool("Debug Options","Uppercase Disa",debug_uppercase_disa);
  LOG_TO(LOGSECTION_INIT,"STARTUP: Updating debug GUI");
  debug_update_bkmon();
  CheckMenuRadioItem(boiler_op_menu,1501,
#if defined(SSE_DEBUGGER_EXCEPTION_NOT_TOS)
    1504,
#else
    1503,
#endif
    1501+crash_notification,MF_BYCOMMAND);
  CheckMenuItem(boiler_op_menu,1514,MF_BYCOMMAND
    |(trace_show_window ? MF_CHECKED : MF_UNCHECKED));
  CheckMenuItem(boiler_op_menu,1515,MF_BYCOMMAND
    |(debug_monospace_disa ? MF_CHECKED : MF_UNCHECKED));
  CheckMenuItem(boiler_op_menu,1516,MF_BYCOMMAND
    |(debug_uppercase_disa ? MF_CHECKED : MF_UNCHECKED));
  CheckMenuItem(logsection_menu,1013,MF_BYCOMMAND
    |(debug_wipe_log_on_reset ? MF_CHECKED : MF_UNCHECKED));
  CheckMenuItem(breakpoint_menu,1103,MF_BYCOMMAND 
    |((monitor_mode==MONITOR_MODE_STOP) ? MF_CHECKED : MF_UNCHECKED));
  CheckMenuItem(breakpoint_menu,1104,MF_BYCOMMAND 
    |((monitor_mode==MONITOR_MODE_LOG) ? MF_CHECKED : MF_UNCHECKED));
  CheckMenuItem(breakpoint_menu,1107,MF_BYCOMMAND
    |((breakpoint_mode==2) ? MF_CHECKED : MF_UNCHECKED));
  CheckMenuItem(breakpoint_menu,1108,MF_BYCOMMAND
    |((breakpoint_mode==3) ? MF_CHECKED : MF_UNCHECKED));
  CheckMenuItem(logsection_menu2,1012,MF_BYCOMMAND
    |(logging_suspended ? MF_UNCHECKED : MF_CHECKED));
  EnableMenuItem(logsection_menu2,1532,MF_BYCOMMAND
    |(logging_suspended ? MF_DISABLED : MF_ENABLED));
  CheckMenuItem(logsection_menu2,1532,MF_BYCOMMAND
    |(Debug.log_in_trace ? MF_CHECKED : MF_UNCHECKED));
  CheckMenuItem(logsection_menu2,1533,MF_BYCOMMAND
    |(Debug.tracing_suspended ? MF_UNCHECKED : MF_CHECKED));
  CheckMenuItem(logsection_menu2,1534,MF_BYCOMMAND
    |(Debug.trace_in_log ? MF_CHECKED : MF_UNCHECKED));
  EnableMenuItem(logsection_menu2,1534,MF_BYCOMMAND
    |(Debug.tracing_suspended ? MF_DISABLED : MF_ENABLED));
  CheckMenuItem(mem_browser_menu,907,MF_BYCOMMAND 
    |(mem_browser::ex_style ? 0 : MF_CHECKED));
  SendDlgItemMessage(DWin,1020,CB_SETCURSEL,dru_combo_idx,0);
  SetWindowText(GetDlgItem(DWin,1021),dru_edit);

#if !defined(LITTLE_REFACT)
  for(int b=0;b<browsers.NumItems;b++) 
  {
    mem_browser *mb=new mem_browser(browsers[b].ad,browsers[b].type);
    if(browsers[b].n_cols==mb->columns) 
    {
      SendMessage(mb->handle,WM_SETREDRAW,0,0);
      for(int n=0;n<browsers[b].n_cols;n++) 
      {
        if(browsers[b].col_w[n]>=0&&browsers[b].col_w[n]<2000) 
          SendMessage(mb->handle,LVM_SETCOLUMNWIDTH,n,MAKELPARAM(browsers[b].col_w[n],0));
      }
      SendMessage(mb->handle,WM_SETREDRAW,1,0);
    }
    MoveWindow(mb->owner,browsers[b].x,browsers[b].y,browsers[b].w,browsers[b].h,true);
    SetWindowText(mb->owner,browsers[b].name);
#if defined(SSE_DEBUGGER_TOGGLE)
    if(!DebuggerVisible)
      ShowWindow(mb->owner,SW_HIDE);
#endif
  }
#endif//#if !defined(LITTLE_REFACT)

#if defined(LITTLE_REFACT) // avoid DynamicArray

  for(int n=0;n<MAX_MEMORY_BROWSERS;n++) 
  {
    Str Key=Str("Browser")+n+" ";
    TMemBrowLoad b;
    b.x=pCSF->GetInt("Debug Browsers",Key+"Left",-300);
    if(b.x==-300) // is default => no data loaded
      break;
    b.y=pCSF->GetInt("Debug Browsers",Key+"Top",0);
    b.w=pCSF->GetInt("Debug Browsers",Key+"Width",100);
    b.h=pCSF->GetInt("Debug Browsers",Key+"Height",100);
    b.ad=pCSF->GetInt("Debug Browsers",Key+"Address",0);
    strcpy(b.name,pCSF->GetStr("Debug Browsers",Key+"Name","Memory"));
    b.type=(ETypeDispType)pCSF->GetInt("Debug Browsers",Key+"Type",0);
    b.n_cols=0;
    for(int m=0;m<20;m++) 
    {
      b.col_w[m]=pCSF->GetInt("Debug Browsers",Key+"Column"+m,-1);
      if(b.col_w[m]<0) 
        break;
      b.n_cols++;
    }
    mem_browser *mb=new mem_browser(b.ad,b.type);
    if(b.n_cols==mb->columns) 
    {
      SendMessage(mb->handle,WM_SETREDRAW,0,0);
      for(int n=0;n<b.n_cols;n++) 
      {
        if(b.col_w[n]>=0&&b.col_w[n]<2000) 
          SendMessage(mb->handle,LVM_SETCOLUMNWIDTH,n,MAKELPARAM(b.col_w[n],0));
      }
      SendMessage(mb->handle,WM_SETREDRAW,1,0);
    }
    MoveWindow(mb->owner,b.x,b.y,b.w,b.h,true);
    SetWindowText(mb->owner,b.name);
#if defined(SSE_DEBUGGER_TOGGLE)
    if(!DebuggerVisible)
      ShowWindow(mb->owner,SW_HIDE);
#endif

  }

#endif//#if defined(LITTLE_REFACT)

#endif

  LOG_TO(LOGSECTION_INIT,Str("STARTUP: Loading ")+RunDir+SLASH "steem.new");
#ifdef SSE_FILES_IN_RC
  HRSRC rc=FindResource(NULL,MAKEINTRESOURCE(IDR_STEEMNEW),RT_RCDATA);
  ASSERT(rc);
  if(rc)
  {
    HGLOBAL hglob=LoadResource(NULL,rc);
    if(hglob)
    {
      size_t size=SizeofResource(NULL,rc);
      char *ptxt=(char*)LockResource(hglob);
      if(ptxt)
      {
        // identify tag eg [SCROLLERS], load strings, leave after 2 blank lines
        int blanks=0;
        osd_scroller_array.Sort=eslNoSort;
        char tb[2000];
        bool ScrollerSection=0;
        int newline=0;
        int j=0;
        for(size_t i=0;i<size && blanks<2 && j<2000;i++)
        {
          tb[j]=ptxt[i];
          if((tb[j]=='\n'||tb[j]=='\r' )&& newline<2)
          {
            tb[j]=0;
            newline++; // can be n, nr, rn, r
            j++;
          }
          else if(newline)
          {
            newline=0;
            if(tb[0])
            {
              if(ScrollerSection)
              {
                strupr(tb);
//                TRACE("add scroller: %s\n",tb);
                osd_scroller_array.Add(tb);
              }
              else
              {
                if(IsSameStr_I(tb,"[SCROLLERS]")) 
                  ScrollerSection=true;
                if(IsSameStr_I(tb,"[XSCROLLERS]")) 
                  ScrollerSection=true;
                WIN_ONLY( if (IsSameStr_I(tb,"[WINSCROLLERS]")) ScrollerSection=true; )
                UNIX_ONLY( if (IsSameStr_I(tb,"[UNIXSCROLLERS]")) ScrollerSection=true; )
              }
            }
            else
            {
              ScrollerSection=false;
              blanks++;
            }
            j=0;
            i--;
          }
          else
            j++;
        }//nxt i
        //FreeResource(ghlob);
      }
    }
  }
#else
  FILE *f=fopen(RunDir+SLASH "steem.new","rt");
  if (f){
    int blanks=0;
    osd_scroller_array.Sort=eslNoSort;
    for(;;){
      char tb[200];
      if (fgets(tb,198,f)==NULL) break;
      strupr(tb);
      if (tb[strlen(tb)-1]=='\n') tb[strlen(tb)-1]=0;
      if (tb[strlen(tb)-1]=='\r') tb[strlen(tb)-1]=0;
      if (tb[0]){
        bool ScrollerSection=0;
        if (IsSameStr_I(tb,"[SCROLLERS]")) ScrollerSection=true;
        if (IsSameStr_I(tb,"[XSCROLLERS]")) ScrollerSection=true;
        WIN_ONLY( if (IsSameStr_I(tb,"[WINSCROLLERS]")) ScrollerSection=true; )
        UNIX_ONLY( if (IsSameStr_I(tb,"[UNIXSCROLLERS]")) ScrollerSection=true; )
        if (ScrollerSection){
          while (tb[0]){
            if (fgets(tb,198,f)==NULL) break;
            if (tb[strlen(tb)-1]=='\n') tb[strlen(tb)-1]=0;
            if (tb[strlen(tb)-1]=='\r') tb[strlen(tb)-1]=0;
            if (tb[0]==0) break;
            osd_scroller_array.Add(tb);
          }
        }
      }else{
        if ((++blanks)>=2) break;
      }
    }
    LOG_TO(LOGSECTION_INIT,Str("STARTUP: Finished loading ")+RunDir+SLASH "steem.new");
    fclose(f);
  }
#endif
}


void SaveState(TConfigStoreFile *pCSF) {
  if(AutoLoadSnapShot) // save snapshot before dialogs -> disk removal...
    SaveSnapShot(WriteDir+SLASH+AutoSnapShotName+".sts",-1,0);

  SaveAllDialogData(true,globalINIFile,pCSF);
#ifdef DEBUG_BUILD
  pCSF->DeleteSection("Debug Addresses");
  for(int n=0;n<debug_ads.NumItems;n++) 
  {
    pCSF->SetInt("Debug Addresses",Str("Address")+n,debug_ads[n].ad&0xffffff);
    pCSF->SetInt("Debug Addresses",Str("Mode")+n,debug_ads[n].mode);
    pCSF->SetInt("Debug Addresses",Str("BWR")+n,debug_ads[n].bwr);
    pCSF->SetInt("Debug Addresses",Str("MaskW")+n,debug_ads[n].mask[0]);
    pCSF->SetInt("Debug Addresses",Str("MaskR")+n,debug_ads[n].mask[1]);
    pCSF->SetStr("Debug Addresses",Str("Name")+n,debug_ads[n].name);
  }
  TWinPositionData wpd;
  GetWindowPositionData(DWin,&wpd);
  pCSF->SetInt("Debug Options","Boiler Left",wpd.Left);
  pCSF->SetInt("Debug Options","Boiler Top",wpd.Top);
  pCSF->SetInt("Debug Options","Boiler Width",wpd.Width);
  pCSF->SetInt("Debug Options","Boiler Height",wpd.Height);
  GetWindowPositionData(trace_window_handle,&wpd);
  pCSF->SetInt("Debug Options","Trace Left",wpd.Left);
  pCSF->SetInt("Debug Options","Trace Top",wpd.Top);
  pCSF->DeleteSection("Debug Browsers");
  int i=0;
  for(int n=0;n<MAX_MEMORY_BROWSERS;n++) 
  {
    if(m_b[n]!=NULL) 
    {
      Str Key=Str("Browser")+i+" ";
      GetWindowPositionData(m_b[n]->owner,&wpd);
      pCSF->SetInt("Debug Browsers",Key+"Left",wpd.Left);
      pCSF->SetInt("Debug Browsers",Key+"Top",wpd.Top);
      pCSF->SetInt("Debug Browsers",Key+"Width",wpd.Width);
      pCSF->SetInt("Debug Browsers",Key+"Height",wpd.Height);
      pCSF->SetInt("Debug Browsers",Key+"Address",int(m_b[n]->ad));
      pCSF->SetInt("Debug Browsers",Key+"Type",m_b[n]->disp_type);
      for(int m=0;m<m_b[n]->columns;m++)
        pCSF->SetInt("Debug Browsers",Key+"Column"+m,
          (int)SendMessage(m_b[n]->handle,LVM_GETCOLUMNWIDTH,m,0));
      pCSF->SetStr("Debug Browsers",Key+"Name",GetWindowTextStr(m_b[n]->owner));
      i++;
    }
  }
  pCSF->SetInt("Debug Options","Breakpoint Mode",breakpoint_mode);
  pCSF->SetInt("Debug Options","Monitor Mode",monitor_mode);
  pCSF->SetInt("Debug Options","Browsers on Taskbar",mem_browser::ex_style);
  pCSF->SetInt("Debug Options","Suspend Logging",logging_suspended);
  pCSF->SetInt("Debug Options","Suspend Tracing",Debug.tracing_suspended);
  pCSF->SetInt("Debug Options","log_in_TRACE",Debug.log_in_trace);
  pCSF->SetInt("Debug Options","Wipe Log On Reset",debug_wipe_log_on_reset);
  pCSF->SetStr("Debug Options","Log Viewer",LogViewProg);
  pCSF->SetInt("Debug Options","Crash Notify",crash_notification);
  pCSF->SetInt("Debug Options","Stack Display",(int)SendDlgItemMessage(DWin,209,CB_GETCURSEL,0,0));
  pCSF->SetInt("Debug Options","Gun Display Colour",debug_gun_pos_col);
  pCSF->SetInt("Debug Options","Trace Show",trace_show_window);
  pCSF->SetInt("Debug Options","Run Until",(int)SendDlgItemMessage(DWin,1020,CB_GETCURSEL,0,0));
  pCSF->SetStr("Debug Options","Run Until Text",GetWindowTextStr(GetDlgItem(DWin,1021)));
  pCSF->SetInt("Debug Options","Monospace Disa",debug_monospace_disa);
  pCSF->SetInt("Debug Options","Uppercase Disa",debug_uppercase_disa);
  FILE *bf=fopen(WriteDir+SLASH "logsection.dat","wb"); //SS another odd part!
  if(bf) 
  {
    for(int n=0;n<100;n++) 
    {
      if(logsection_enabled[n]==false) 
        fprintf(bf,"%i\r\n",n);
    }
    fprintf(bf,"\r\n");
    fclose(bf);
  }
#endif
}


#if USE_PASTI

void LoadSavePastiActiveChange() {
  DiskMan.RefreshDiskView();
}

#endif
