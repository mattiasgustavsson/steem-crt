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

FILE: main.cpp
DESCRIPTION: This file contains the various main routines, general 
startup/shutdown code for all the versions of Steem, and command-line options.
GUI objects instantiated.
---------------------------------------------------------------------------*/

#include "pch.h" // SS this includes SSE.h (switches) and  windows.h (if in WIN32) etc.
#pragma hdrstop // SS this says 'precompile up to this point'

/*
------------------------------------------------------------------
       S T E E M   E N G I N E

       The STE Emulating Engine

       Last updated [today]
                                                  |||
                                                  |||
                                                 / | \
                                               _/  |  \_
------------------------------------------------------------------
*/


#include <conditions.h>
#include <options.h>
#include <debug.h>
#include <computer.h>
#include <gui.h>
#if defined(SSE_VID_STVL1)
#include <interface_stvl.h>
#endif
#include <translate.h>
#include <diskman.h>
#include <notifyinit.h>
#include <draw.h>
#include <loadsave.h>
#include <harddiskman.h>
#include <display.h>
#include <archive.h>
#include <ArchiveAccess/ArchiveAccess/ArchiveAccessSSE.h>
#include <dir_id.h>
#include <shortcutbox.h>
#include <stjoy.h>
#include <infobox.h>
#include <patchesbox.h>
#include <macros.h>
#include <key_table.h>
#if defined(SSE_MAIN_LOOP3)
#include <eh.h>
#include <psapi.h>
#endif
#include <screen_saver.h>
#if defined(SSE_VID_RECORD_AVI)
#include <AVI/AviFile.h> // AVI (DD-only)
#endif

#ifdef DEBUG_BUILD
#include <debugger.h>
#include <mr_static.h>
#include <debugger_trace.h>
#endif

 //singleton objects
TSteemDisplay Disp;
TPatchesBox PatchesBox;
TDiskManager DiskMan;
TGeneralInfo InfoBox;
TOptionBox OptionBox;
TJoystickConfig JoyConfig;
TOption SSEOptions;
TConfig SSEConfig;

#ifdef WIN32

HINSTANCE Inst;
HINSTANCE &HInstance=Inst;

#endif

#if defined(MINGW_BUILD) || defined(UNIX)

char *ultoa(unsigned long l,char *s,int radix) {
  if (radix==10) sprintf(s,"%u",(unsigned int)l);
  if (radix==16) sprintf(s,"%x",(unsigned int)l);
  return s;
}

char strupr_convert_buf[256]={0},strlwr_convert_buf[256]={0};

char *strupr(char *s) {
  if (strupr_convert_buf[0]==0){
    strupr_convert_buf[0]=1;
    for (int i=1;i<256;i++){
      strupr_convert_buf[i]=(char)i;
      if (islower(i)) strupr_convert_buf[i]=toupper((char)i);
    }
  }
  char *p=s;
  while (*p){
    *p=strupr_convert_buf[(unsigned char)(*p)];
    p++;
  }
  return s;
}

char *strlwr(char *s) {
  if (strlwr_convert_buf[0]==0){
    strlwr_convert_buf[0]=1;
    for (int i=1;i<256;i++){
      strlwr_convert_buf[i]=(char)i;
      if (isupper(i)) strlwr_convert_buf[i]=tolower((char)i);
    }
  }
  char *p=s;
  while (*p){
    *p=strlwr_convert_buf[(unsigned char)(*p)];
    p++;
  }
  return s;
}

#endif

#ifdef UNIX

Display *XD;
XContext cWinThis,cWinProc;

char **_argv;
int _argc;

KeyCode VK_LBUTTON,VK_RBUTTON,VK_MBUTTON;
KeyCode VK_F1,VK_F11,VK_F12,VK_END;
KeyCode VK_LEFT,VK_RIGHT,VK_UP,VK_DOWN,VK_TAB;
KeyCode VK_SHIFT,VK_LSHIFT,VK_RSHIFT;
KeyCode VK_MENU,VK_LMENU,VK_RMENU;
KeyCode VK_CONTROL,VK_LCONTROL,VK_RCONTROL;
KeyCode VK_NUMLOCK,VK_SCROLL;

void UnixOutput(char *Str) {
  printf("%s\r\n",Str);
}


char *itoa(int i,char *s,int radix) {
  if (radix==10) sprintf(s,"%i",(int)i);
  if (radix==16) sprintf(s,"%x",(int)i);
  return s;
}

#endif

#include <resnum.h>

bool OpenComLineFilesInCurrent(bool AlwaysSendToCurrent);

const char *stem_version_date_text=__DATE__ " - " __TIME__;

char stem_version_text[SSE_VERSION_TXT_LEN];


#ifndef ONEGAME

#if defined(SSE_BUILD)

char stem_window_title[WINDOW_TITLE_MAX_CHARS+1];

#else

const char *stem_window_title="Steem Engine";

#endif

#else

const char *stem_window_title=ONEGAME;
#define _USE_MEMORY_TO_MEMORY_DECOMPRESSION
#include <unrarlib/unrarlib.h>
//#include <urarlib/urarlib.c>
#include "onegame.cpp"

#endif


bool Initialise();
void PerformCleanShutdown();

EasyStr CrashFile;

#define LOGSECTION LOGSECTION_INIT

int MainRetVal=-50;


#ifdef WIN32

#if defined(SSE_MAIN_LOOP3)
/*  The idea is to report a SEH exception in a normal try/catch block.
    https://msdn.microsoft.com/en-us/library/5z4bw5h5(VS.80).aspx
*/
void __cdecl trans_func( unsigned int u, EXCEPTION_POINTERS* pExp )
{
    throw SE_Exception(u,pExp); //caught in WinMain()
}
 
#pragma comment (lib, "Psapi.lib")

void SE_Exception::handle_exception() {
  PVOID& pc_crash_address=m_pExp->ExceptionRecord->ExceptionAddress;
  // get module name where exception happened
  char module_name[80];
  module_name[0]='\0';
  HANDLE hProcess=GetCurrentProcess();
  HMODULE hMods[1024];
  DWORD cbNeeded;
  if( EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
  {
    for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
    {
      MODULEINFO modinfo;
      if(GetModuleInformation(hProcess,hMods[i],&modinfo,sizeof(modinfo)))
      {
        if(pc_crash_address>=modinfo.lpBaseOfDll && pc_crash_address
          < (BYTE*)modinfo.lpBaseOfDll+modinfo.SizeOfImage)
        {
          // Get the full path to the module's file.
          TCHAR szModName[MAX_PATH];
          if(GetModuleFileName(hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
          {
            char *name=GetFileNameFromPath(szModName);
            if(name)
              strncpy(module_name,name,80-1);
          }
        }
      }
    }
  }
  //In some cases this SEH info is enough to track the bug (using a debugger)
  char exc_string[256];
  sprintf(exc_string,"System exception $%X at $%p in %s",
    m_pExp->ExceptionRecord->ExceptionCode,pc_crash_address,module_name);
  StatusInfo.MessageIndex=TStatusInfo::INTEL_CRASH;
  REFRESH_STATUS_BAR;
  Alert(exc_string,"STEEM CRASHED AGAIN!",MB_ICONEXCLAMATION); // alert box before trace
  SetClipboardText(exc_string);
  TRACE2("%s\n",exc_string);
}

#endif


int WINAPI WinMain(HINSTANCE ourInstance,HINSTANCE,char *,int) {
  Inst=ourInstance;
  RunDir=GetEXEDir();
  NO_SLASH(RunDir);
  DocDir=RunDir+SLASH+"doc"+SLASH;
  SetCurrentDirectory(RunDir.Text); // can help using relative paths in ini
#ifndef _DEBUG
  try 
#endif
  {
#if defined(SSE_MAIN_LOOP3) 
    _set_se_translator(trans_func);
#endif
/*
  Windows Server 2003 and Windows XP:  In low memory situations, 
  InitializeCriticalSection can raise a STATUS_NO_MEMORY exception.
  Starting with Windows Vista, this exception was eliminated and
  InitializeCriticalSection always succeeds, even in low memory situations.
  SS I think this critical section is useless. It isn't active if an emulation
  thread is running.
*/
    InitializeCriticalSection(&agenda_cs);
    if(Initialise()==0)
    {
      CleanUpSteem();
      if(CrashFile.NotEmpty())
        DeleteFile(CrashFile);
      return MainRetVal;
    }
    DBG_LOG("STARTUP: Starting Message Loop");
    MSG MainMess;
    while(GetMessage(&MainMess,NULL,0,0))
    {
      if(HandleMessage(&MainMess))
      {
        TranslateMessage(&MainMess);
        //TScreenSaver::checkMessage(&MainMess);
        DispatchMessage(&MainMess);
      }
    }
    if(StemWin)
      ShowWindow(StemWin,SW_HIDE);
#ifdef DEBUG_BUILD
#if defined(SSE_DEBUGGER_TOGGLE)
    BOOL a=DebuggerVisible;
    if(DWin)
      ShowWindow(DWin,SW_HIDE);
    DebuggerVisible=a; // for ini file
#else
    if(DWin)
      ShowWindow(DWin,SW_HIDE);
#endif
    if(trace_window_handle)
      ShowWindow(trace_window_handle,SW_HIDE);
#endif
    if(SnapShotGetLastBackupPath().NotEmpty())
    {
      DBG_LOG("SHUTDOWN: Deleting last memory snapshot backup");
      DeleteFile(SnapShotGetLastBackupPath());
    }
    PerformCleanShutdown();
    return EXIT_SUCCESS;
  }
#if defined(SSE_MAIN_LOOP3)
  catch(SE_Exception e) {
    e.handle_exception();
  }
#endif
#ifndef _DEBUG
  catch(...){
    Alert("Unknown exception","STEEM CRASHED AGAIN!",MB_ICONEXCLAMATION); // C++, CRT ?
    TRACE2("Unknown exception\n");
  }
  SetErrorMode(0);
  log_write("Fatal Error - attempting to shut down cleanly");
  PerformCleanShutdown();
  return EXIT_FAILURE;
#endif
}

#endif

#ifdef UNIX

int main(int argc,char *argv[]) {
  _argv=argv;
  _argc=argc;
  for(int n=0;n<_argc-1;n++) 
  {
    EasyStr butt;
    int Type=GetComLineArgType(_argv[1+n],butt);
    if(Type==ARG_HELP) 
    {
      PrintHelpToStdout();
      return 0;
    }
  }
  if(_argv[0][0]=='/')
  { //Full path
    RunDir=_argv[0];
    RemoveFileNameFromPath(RunDir,REMOVE_SLASH);
  }
  else
  {
    RunDir.SetLength(MAX_PATH+1);
    getcwd(RunDir,MAX_PATH);
    NO_SLASH(RunDir);
  }
  TRACE2("\n-- Steem Engine v%s --\n\n",stem_version_text);
  TRACE2("Steem will save all its settings to %s\n",RunDir.Text);
  //printf(EasyStr("\n-- Steem Engine v")+stem_version_text+" --\n\n");
  //printf(EasyStr("Steem will save all its settings to ")+RunDir.Text+"\n");
#if defined(SSE_HD6301_LL) 
  Ikbd.Init();
#endif  
  XD=XOpenDisplay(NULL);
  if(XD==NULL)
  {
    printf("\nFailed to open X display\n");
    return EXIT_FAILURE;
  }
  XSetErrorHandler(HandleXError);
  hxc::modal_notifyproc=steem_hxc_modal_notify;
  InitColoursAndIcons();
  NO_SLASH(RunDir);
///  SetCurrentDirectory(RunDir.Text); // can help using relative paths in ini
/// doesn't exist in linux
  try {
    if(Initialise()==0) 
    {
      CleanUpSteem();
      if(CrashFile.NotEmpty()) 
        DeleteFile(CrashFile);
      return MainRetVal;
    }
    XEvent Ev;
    for(;;) {
      if (hxc::wait_for_event(XD,&Ev)){
        if (ProcessEvent(&Ev)==PEEKED_RUN){
          Window FocusWin;
          int RevertFlag;
          XGetInputFocus(XD,&FocusWin,&RevertFlag);
          if (FocusWin==StemWin && fast_forward!=RUNSTATE_STOPPED+1 && slow_motion!=RUNSTATE_STOPPED+1){
            if(OPTION_CAPTURE_MOUSE&1)
              SetStemMouseMode(STEM_MOUSEMODE_WINDOW);
          }
          RunBut.set_check(true);
          run();
          RunBut.set_check(0);
        }
      }
      if (Quitting && XPending(XD)==0) break;
    }
    XUnmapWindow(XD,StemWin);
    XFlush(XD);
    if(SnapShotGetLastBackupPath().NotEmpty()) 
    {
      DBG_LOG("SHUTDOWN: Deleting last memory snapshot backup");
      DeleteFile(SnapShotGetLastBackupPath());
    }
    PerformCleanShutdown();
  	return EXIT_SUCCESS;
  }catch(...){}
  log_write("Fatal Error - attempting to shut down cleanly");
  PerformCleanShutdown();
  return EXIT_FAILURE;
}

#endif

void FindWriteDir() {
  char TestOutFileName[MAX_PATH+1];
  bool RunDirCanWrite=0;
  if(GetTempFileName(RunDir,"TST",0,TestOutFileName)) // RunDir init in WinMain()
  {
    FILE *f=fopen(TestOutFileName,"wb");
    if(f)
    {
      RunDirCanWrite=true;
      fclose(f);
    }
    DeleteFile(TestOutFileName);
  }
  if(RunDirCanWrite)
    WriteDir=RunDir;
  else 
  {
#ifdef WIN32
    ITEMIDLIST *idl;
    IMalloc *Mal;
    SHGetMalloc(&Mal);
    if(SHGetSpecialFolderLocation(NULL,CSIDL_APPDATA,&idl)==NOERROR)
    {
      SHGetPathFromIDList(idl,TestOutFileName);
      Mal->Free(idl);
    }
    else
      GetTempPath(MAX_PATH,TestOutFileName);
    NO_SLASH(TestOutFileName);
#ifndef ONEGAME
    WriteDir=Str(TestOutFileName)+SLASH+"Steem";
#else
    WriteDir=Str(TestOutFileName)+SLASH+ONEGAME;
    CreateDirectory(WriteDir,NULL);
    WriteDir+=Str(SLASH)+ONEGAME_NAME;
#endif
    CreateDirectory(WriteDir,NULL);
#else
    // Must find a location that is r/w
    WriteDir=RunDir;
#endif
  }
}


int SteemIntro();

bool Initialise() { // called once by WinMain()

  ComputerRestore(); // for drives

  // update WriteDir, should be RunDir
  FindWriteDir(); 
  // build stem_version_text eg "3.7.0" - quite complicated for what it does
  int d1=SSE_VERSION/100;
  int d2=(SSE_VERSION-d1*100)/10;
  int d3=SSE_VERSION-d1*100-d2*10;
  sprintf(stem_version_text,"%d.%d.%d",d1,d2,d3);
#ifdef SSE_BETA
  strcpy((char*)stem_window_title,"Steem 4 beta");
#else
  strcpy((char*)stem_window_title,"Steem 4");
#endif
  runstate=RUNSTATE_STOPPED;
  DEBUG_ONLY( stem_runmode=STEM_MODE_INSPECT; )
  TranslateFileName=RunDir+SLASH "Translate.txt";
#ifndef ONEGAME
  bool CustomINI=false;
  // treat command line arguments
  globalINIFile=WriteDir+SLASH "steem.ini";
  bool NoNewInst=0,AlwaysNewInst=0,QuitNow=0;
  for(int n=0;n<_argc-1;n++) 
  {
    EasyStr Path;
    int Type=GetComLineArgType(_argv[1+n],Path);
    if(Type==ARG_SETINIFILE)
    {
      globalINIFile=Path;
      CustomINI=true;
    }
    else if(Type==ARG_SETTRANSFILE)
    {
      if(Exists(Path))
        TranslateFileName=Path;
    }
    else if(Type==ARG_NONEWINSTANCE||Type==ARG_TAKESHOT)
      NoNewInst=true;
    else if(Type==ARG_ALWAYSNEWINSTANCE)
      AlwaysNewInst=true;
    else if(Type==ARG_TOSIMAGEFILE)
    {
      ROMFile=Path;
      BootTOSImage=true;
    }
    else if(Type==ARG_QUITQUICKLY)
      QuitNow=true;
    else if(Type==ARG_SETFONT)
    {
      /////////////////UNIX_ONLY( hxc::font_sl.Insert(0,0,Path,NULL); )
    }
    else if(Type==ARG_NONOTIFYINIT)
      SSEConfig.ShowNotify=0;
    else if(Type==ARG_NOTRACE)
      SSEConfig.TraceFile=false;
#if 0 //defined(SSE_UNIX_TRACE)
    else if(Type==ARG_TRACEFILE)
    {
      if(Path.Length()>0) // room for improvement...
        OPTION_TRACE_FILE=(Path.Mids(0,1)=="Y"||Path.Mids(0,1)=="y")?1:0;
    }
    else if(Type==ARG_LOGSECTION) 
    {
      for(int i=0;i<NUM_LOGSECTIONS;i++)
      {
        if(Path.Length()>i)
          Debug.logsection_enabled[i]=Path.Mids(i,1)=="1"?1:0;
      }
    }
#endif    
  }
#else
  INIFile=RunDir+SLASH ONEGAME_NAME ".ini";
#endif
  TConfigStoreFile CSF(globalINIFile);
  SSEConfig.TraceFile=(CSF.GetInt("Options","TraceFile",0)!=0);
#ifdef UNIX
  SSEConfig.TraceFile=true; // temp
#endif  
  if(SSEConfig.TraceFile)
    Debug.TraceInit();
  SSEConfig.ShowNotify=(CSF.GetInt("Main","NoNotify",!SSEConfig.ShowNotify)==0);
#ifdef WIN32
  SSEConfig.WindowsVersion = GetVersion();
#if defined(SSE_DEBUG) //from Microsoft
  DWORD dwVersion = SSEConfig.WindowsVersion; 
  DWORD dwMajorVersion = 0;
  DWORD dwMinorVersion = 0; 
  DWORD dwBuild = 0;
  // Get the Windows version.
  dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
  dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
  // Get the build number.
  if (dwVersion < 0x80000000)              
    dwBuild = (DWORD)(HIWORD(dwVersion));
  TRACE_INIT("Windows version %d.%d (%d) - $%X\n",dwMajorVersion,dwMinorVersion,dwBuild,SSEConfig.WindowsVersion);
#endif
#endif
#ifndef ONEGAME
  if((CSF.GetInt("Options","OpenFilesInNew",true)==0||NoNewInst)&&AlwaysNewInst==0) 
  {
    if(OpenComLineFilesInCurrent(NoNewInst)) 
    {
      MainRetVal=EXIT_SUCCESS;
      return 0;
    }
  }
  if(BootTOSImage) 
    CSF.SetStr("Machine","ROM_File",ROMFile);
  NoINI=(!CustomINI && (CSF.GetBool("Update",GetFileNameFromPath(GetEXEFileName().Text),0)==0)
    && (CSF.GetBool("Main","NoIntro",0)==0));
  CSF.SetInt("Main","DebugBuild",0 DEBUG_ONLY( +1 ) );
  CSF.SetStr("Update","CurrentVersion",Str((char*)stem_version_text));
  if(QuitNow) 
  {
    MainRetVal=EXIT_SUCCESS;
    return 0;
  }
#endif
  srand(timeGetTime());
#ifdef ENABLE_LOGFILE
  int n=2;
  LogFileName=WriteDir+SLASH "steem.log";
  while((logfile=fopen(LogFileName,"wb"))==NULL) 
  {
    int a=MessageBox(WINDOWTYPE(0),EasyStr("Can't open ")+LogFileName+"\nSelect\
 Ignore to try a different name","Error",MB_ABORTRETRYIGNORE|
      MB_ICONEXCLAMATION|MB_TASKMODAL|MB_TOPMOST|MB_SETFOREGROUND);
    if(a==IDABORT) 
    {
      QuitSteem();
      break;
    }
    else if(a==IDIGNORE)
      LogFileName=WriteDir+SLASH "steem ("+(n++)+").log";
  }
  DBG_LOG("STARTUP: Logfile Open");
  load_logsections();
#endif
  InitTranslations();
#if defined(SSE_GUI_RICHEDIT)
  LoadLibrary("RICHED20.DLL");
#endif
#if USE_PASTI
  hPasti=SteemLoadLibrary(PASTI_DLL);
  if(hPasti)
  {
    bool Failed=true;
    LPPASTIINITPROC pastiInit=(LPPASTIINITPROC)GetProcAddress(hPasti,"pastiInit");
    if(pastiInit) 
    {
      struct pastiCALLBACKS pcb;
      ZeroMemory(&pcb,sizeof(pcb));
      pcb.LogMsg=pasti_log_proc;
      pcb.WarnMsg=pasti_warn_proc;
      pcb.MotorOn=pasti_motor_proc;
      struct pastiINITINFO pii;
      pii.dwSize=sizeof(pii);
      pii.applFlags=0;
      pii.applVersion=2;
      pii.cBacks=&pcb;
      Failed=(pastiInit(&pii)==FALSE);
      pasti=pii.funcs;
    }
    if(Failed)
    {
      FreeLibrary(hPasti);
      hPasti=NULL;
      Alert(T("Pasti initialisation failed"),T("Pasti Error"),MB_ICONEXCLAMATION);
    }
    else
    {
      char p_exts[PASTI_FILE_EXTS_BUFFERSIZE];
      ZeroMemory(p_exts,PASTI_FILE_EXTS_BUFFERSIZE);

      pasti->GetFileExtensions(p_exts,PASTI_FILE_EXTS_BUFFERSIZE,TRUE); // returns 0

      // Convert to NULL terminated list
      for(int i=0;i<PASTI_FILE_EXTS_BUFFERSIZE;i++)
      {
        if(p_exts[i]==0) 
          break;
        if(p_exts[i]==';') 
          p_exts[i]=0;
      }
      // Strip *.
      char *p_src=p_exts,*p_dest=pasti_file_exts;
      ZeroMemory(pasti_file_exts,PASTI_FILE_EXTS_BUFFERSIZE);
      while(*p_src)
      {
        if(*p_src=='*') 
          p_src++;
        if(*p_src=='.') 
          p_src++;
        TRACE_INIT("%s ",p_src);
        strcpy(p_dest,p_src);
        p_dest+=strlen(p_dest)+1;
        p_src+=strlen(p_src)+1;
      }
      TRACE_INIT("can be handled by %s\n",PASTI_DLL);
      SSEConfig.PastiDll=true;
    }
  }
#endif //USE_PASTI
  DiskMan.InitGetContents();
#ifndef ONEGAME
  bool TwoSteems=CheckForSteemRunning();
#if defined(UNIX)
  TwoSteems=true; //SS certainly a hack
#endif
  bool CrashedLastTime=CleanupTempFiles();
  if(TwoSteems==0) 
  {
    if(CrashedLastTime) 
    {
#if defined(SSE_DEBUG) // Debugger too
      // Crashes are common while testing
      StepByStepInit=0;
#elif defined(SSE_BUILD) // don't want emails
      StepByStepInit=Alert(T("It seems that Steem did not close properly. If it\
 crashed we are terribly sorry, it shouldn't happen. If you can get Steem to \
crash 2 or more times when doing the same thing then please post a bug report \
here: ")+"\n\n" STEEM_WEB_BUG_REPORTS "\n\n"+ T("Please write as much detail as \
you can and we'll look into it as soon as possible. ")+"\n\n"+T("If you are \
having trouble starting Steem, you might want to step carefully through the \
initialisation process.  Would you like to do a step-by-step confirmation?"),
        T("Step-By-Step Initialisation"),MB_ICONQUESTION|MB_YESNO)==IDYES;
#else
      StepByStepInit=Alert(T("It seems that Steem did not close properly. If it crashed we are terribly sorry, it shouldn't happen. If you can get Steem to crash 2 or more times when doing the same thing then please tell us, it would be a massive help.")+
        "\n\nE-mail: " STEEM_EMAIL "\n\n"+
        T("Please send as much detail as you can and we'll look into it as soon as possible. ")+
        "\n\n"+T("If you are having trouble starting Steem, you might want to step carefully through the initialisation process.  Would you like to do a step-by-step confirmation?"),
        T("Step-By-Step Initialisation"),MB_ICONQUESTION|MB_YESNO)==IDYES;
#endif
      }
    CrashFile.SetLength(MAX_PATH);
    GetTempFileName(WriteDir,"CRA",0,CrashFile);
    FILE *f=fopen(CrashFile,"wb");
    if(f)
    {
      fclose(f);
      SetFileAttributes(CrashFile,FILE_ATTRIBUTE_HIDDEN);
    }
  }
  DeleteFile(WriteDir+SLASH "steemcrash.ini");
  if(StepByStepInit && NoINI==0) 
  {
    if(Alert(T("It is possible that one of the settings you changed has made \
Steem crash, do you want to use the default settings? (Note: this won't lose \
your settings, anything you change this time will be saved to steemcrash.ini)"),
      T("Use Default Settings?"),MB_ICONQUESTION|MB_YESNO)==IDYES) 
    {
      ROMFile=CSF.GetStr("Machine","ROM_File",RunDir+SLASH "tos.img");
      CSF.Close();
      globalINIFile=WriteDir+SLASH "steemcrash.ini";
      CSF.Open(globalINIFile);
      CSF.SetStr("Machine","ROM_File",ROMFile);
    }
  }
#endif
  LoadAllIcons(&CSF,true);
#if !defined(_DEBUG)
  if(SSEConfig.ShowNotify)
    CreateNotifyInitWin(T("Steem is Initialising").Text);
#endif
#ifdef WIN32
  SetNotifyInitText(T("COM and Common Controls"));
  CoInitialize(NULL);
  InitCommonControls();
#endif
  SetNotifyInitText(T("ST Memory"));
  try{
    BYTE ConfigBank1=(BYTE)CSF.GetInt("Machine","Mem_Bank_1",MEMCONF_512);
    BYTE ConfigBank2=(BYTE)CSF.GetInt("Machine","Mem_Bank_2",MEMCONF_512);
    if(ConfigBank1!=MEMCONF_128 && ConfigBank1!=MEMCONF_512 &&
#if defined(SSE_MMU_MONSTER_ALT_RAM)
      ConfigBank1!=MEMCONF_6MB &&
#endif
      ConfigBank1!=MEMCONF_2MB && ConfigBank1!=MEMCONF_7MB)
    {
      // Invalid memory somehow
      ConfigBank1=MEMCONF_512;
      ConfigBank2=MEMCONF_512;
    }
    SSEConfig.make_Mem(ConfigBank1,ConfigBank2);
  }catch (...){
    MessageBox(WINDOWTYPE(0),T("Could not allocate enough memory!"),
      T("Out Of Memory"),MB_ICONEXCLAMATION|MB_TASKMODAL|MB_TOPMOST
      |MB_SETFOREGROUND);
    MainRetVal=EXIT_FAILURE;
    return 0;
  }
#ifdef DEBUG_BUILD
  for(int m=0;m<MAX_MEMORY_BROWSERS;m++) 
    m_b[m]=NULL;
  for(int m=0;m<MAX_MR_STATICS;m++) 
    m_s[m]=NULL;
#endif
#ifdef WIN32
  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
  GetVersionEx(&osvi);
  WinNT=(osvi.dwPlatformId==VER_PLATFORM_WIN32_NT);
  SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS
    |SEM_NOALIGNMENTFAULTEXCEPT);
#endif
#ifdef DISABLE_STEMDOS
  for(int n=0;n<26;n++)
  {
    mount_flag[n]=false;
    mount_path[n]="";
  }
#else
  stemdos_init();
#endif
  DBG_LOG("STARTUP: draw_routines_init Called");
  if(draw_routines_init()==0)
  {
    MainRetVal=EXIT_FAILURE;
    return 0;
  }
#ifndef ONEGAME
  int IntroResult=2;
#ifdef TEST_STEEM_INTRO
  SteemIntro();
#endif
#if !(defined(SSE_NO_INTRO))
  if(NoINI) 
  {
    WIN_ONLY(ShowWindow(NotifyWin,SW_HIDE); )
    IntroResult=SteemIntro();
    CSF.SetInt("Update",GetFileNameFromPath(GetEXEFileName().Text),1);
    if(IntroResult==1) 
    {
      MainRetVal=EXIT_FAILURE;
      return 0;
    }
#ifdef WIN32
    if(NotifyWin) 
    {
      SetWindowPos(NotifyWin,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE
        |SWP_SHOWWINDOW);
      UpdateWindow(NotifyWin);
    }
#endif
  }//noini
#endif
  SetNotifyInitText(T("ST Operating System"));
  if(IntroResult==2)
  {
    ROMFile=CSF.GetStr("Machine","ROM_File",RunDir+SLASH "tos.img");
#if defined(SSE_GUI_CONFIG_FILE)
    // add current TOS path if necessary
    if(strchr(ROMFile.Text,SLASHCHAR)==NULL) // no slash = no path
    {
      EasyStr tmp=CSF.GetStr("Machine","ROM_Add_Dir",RunDir)+SLASH+ROMFile;
      ROMFile=tmp;
    }
#endif
    if(StepByStepInit) 
    {
      if(Alert(T("A different TOS version may help to stop Steem crashing, would\
 you like to choose one?"),
        T("Change TOS?"),MB_ICONQUESTION|MB_YESNO)==IDYES)
        ROMFile="";
    }
    while(load_TOS(ROMFile)) 
    {
#ifdef WIN32
      if(NotifyWin) 
        ShowWindow(NotifyWin,SW_HIDE);
#endif
      if(ROMFile.NotEmpty()) 
      {
        if(Exists(ROMFile)==0) 
        {
          MessageBox((WINDOWTYPE)0,EasyStr(T("Can't find file"))+" "+ROMFile,T("Error Loading OS"),
            MB_ICONEXCLAMATION|MB_TASKMODAL|MB_TOPMOST|MB_SETFOREGROUND);
        }
        else 
        {
          MessageBox((WINDOWTYPE)0,ROMFile+" "+T("is not a valid TOS"),T("Error Loading OS"),
            MB_ICONEXCLAMATION|MB_TASKMODAL|MB_TOPMOST|MB_SETFOREGROUND);
        }
      }
#ifdef WIN32
      ROMFile=FileSelect(NULL,T("Select TOS Image"),RunDir,FSTypes(3,NULL),1,
        true,"img");
#endif
#ifdef UNIX
      fileselect.set_corner_icon(&Ico16,ICO16_CHIP);
      ROMFile=fileselect.choose(XD,RunDir,NULL,
        T("Select TOS Image"),FSM_LOAD|FSM_LOADMUSTEXIST,romfile_parse_routine,".img");
#endif
      if(ROMFile.IsEmpty()) 
      {
        MainRetVal=EXIT_FAILURE;
        return 0;
      }
    }
#ifdef WIN32
    if(NotifyWin) 
    {
      SetWindowPos(NotifyWin,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
      UpdateWindow(NotifyWin);
    }
#endif
  }//if(IntroResult==2) 
  CartFile=CSF.GetStr("Machine","Cart_File",CartFile);
  if(CartFile.NotEmpty())
  {
    if(load_cart(CartFile)) 
      CartFile="";
  }
#else
  if(load_TOS("")) 
  {
    MainRetVal=EXIT_FAILURE;
    return 0;
  }
#endif
#if defined(SSE_ACSI) && defined(SSE_UNIX)
  // ACSI crawler back for Linux, should have dialog anyway
/*  We use the existing Steem "crawler" to load whatever hard disk IMG 
    files are in Steem/ACSI, up to MAX_ACSI_DEVICES.
*/
  ASSERT(!acsi_dev && !SSEConfig.AcsiImg);
  DirSearch ds; 
  EasyStr Fol=RunDir+SLASH+ACSI_HD_DIR+SLASH;
  if (ds.Find(Fol+"*.img")){
    do{
      strcpy(ansi_string,Fol.Text);
      strcat(ansi_string,ds.Name);
      bool ok=AcsiHdc[acsi_dev].Init(acsi_dev,ansi_string); 
      if(ok)
      {
        SSEConfig.AcsiImg=true;
        acsi_dev++;
      }
    }while (ds.Next() && acsi_dev<TAcsiHdc::MAX_ACSI_DEVICES);
    ds.Close();
  }
#endif//ACSI
  SetNotifyInitText(T("Jump Tables"));
  DBG_LOG("STARTUP: cpu_routines_init Called");
  cpu_routines_init();
#if defined(SSE_ARCHIVEACCESS_SUPPORT)
  SetNotifyInitText(ARCHIVEACCESS_DLL);
  WIN_ONLY( ARCHIVEACCESS_OK=LoadArchiveAccessDll(ARCHIVEACCESS_DLL); )
  if(ARCHIVEACCESS_OK)
    enable_zip=true;
  WIN_ONLY( TRACE_LOG("%s ok:%d\n",ARCHIVEACCESS_DLL,ARCHIVEACCESS_OK); )
#endif
#ifdef WIN32
#if !defined(SSE_NO_UNZIPD32)
  SetNotifyInitText(UNZIP_DLL);
  LoadUnzipDLL();
#endif
#endif
#if defined(SSE_UNRAR_SUPPORT_WIN)
  SetNotifyInitText(UNRAR_DLL);
  LoadUnrarDLL();
#endif
#if defined(SSE_DISK_CAPS)
  SetNotifyInitText(SSE_DISK_CAPS_PLUGIN_FILE);
  Caps.Init();
#endif
#if defined(SSE_HD6301_LL) 
  SetNotifyInitText(HD6301_ROM_FILENAME);
  Ikbd.Init();
#endif
#ifdef DEBUG_BUILD
  DBG_LOG("STARTUP: d2_routines_init Called");
  d2_routines_init();
  for(int i=0;i<HISTORY_SIZE;i++) 
    pc_history[i]=0xffffff71;
  pc_history_idx=0;
#endif
  SetNotifyInitText(T("GUI"));
  DBG_LOG("STARTUP: MakeGUI Called");
  if(MakeGUI()==0)
  {
    MainRetVal=EXIT_FAILURE;
    return 0;
  }
#if !defined(SSE_NO_UPDATE)
  if(Exists(RunDir+SLASH "new_steemupdate.exe")) 
  {
    DeleteFile(RunDir+SLASH "steemupdate.exe");
    if(Exists(RunDir+SLASH "steemupdate.exe")==0)
      MoveFile(RunDir+SLASH "new_steemupdate.exe",RunDir+SLASH "steemupdate.exe");
  }
#endif
#ifndef ONEGAME
  ParseCommandLine(_argc-1,_argv+1);
#endif
#ifdef WIN32
  Disp.DrawToVidMem=(CSF.GetInt("Options","DrawToVidMem",Disp.DrawToVidMem)!=0);
  Disp.BlitHideMouse=(CSF.GetInt("Options","BlitHideMouse",Disp.BlitHideMouse)!=0);
  if(CSF.GetInt("Options","NoDirectDraw",0)) 
    TryDX=0;
  if(TryDX && StepByStepInit) 
  {
#ifdef SSE_VID_D3D
    if(Alert(T("DirectX can cause problems on some set-ups, would you like \
  Steem to stop using Direct3D for this session? (Note: Not using Direct3D slows\
 down Steem).")+" "+T("To permanently stop using Direct3D turn on Options->\
Startup->Never Use Direct3D."),T("No Direct3D?"),MB_ICONQUESTION|MB_YESNO)==IDYES)
#else
    if(Alert(T("DirectX can cause problems on some set-ups, would you like \
Steem to stop using DirectDraw for this session? (Note: Not using DirectDraw \
slows down Steem).")+" "+T("To permanently stop using DirectDraw turn on \
Options->Startup->Never Use DirectDraw."),T("No DirectDraw?"),MB_ICONQUESTION
      |MB_YESNO)==IDYES)
#endif
      TryDX=0;
  }
  if(TryDX)
#if defined(SSE_VID_D3D) 
    Disp.SetMethods(DISPMETHOD_D3D,DISPMETHOD_GDI,0);
#else
    Disp.SetMethods(DISPMETHOD_DD,DISPMETHOD_GDI,0);
#endif
  else
    Disp.SetMethods(DISPMETHOD_GDI,0);
#endif
#ifdef UNIX
  if(CSF.GetInt("Options","NoSHM",0)) 
    TrySHM=0;
  if(TrySHM && StepByStepInit) 
  {
    if(Alert(T("It is possible that using the MIT Shared Memory Extension to speed up drawing might cause problems on some systems.  Do you want to disable SHM?"),
      T("No SHM?"),MB_ICONQUESTION|MB_YESNO)==IDYES)
      TrySHM=0;
  }
  if(TrySHM)
    Disp.SetMethods(DISPMETHOD_XSHM,DISPMETHOD_X,0);
  else
    Disp.SetMethods(DISPMETHOD_X,0);
#endif
#if defined(SSE_VID_STVL1)
  SetNotifyInitText(VIDEO_LOGIC_DLL);
  StvlInit();
#endif
  DBG_LOG("STARTUP: Initialising display");
  Disp.Init();
  Debug.TraceGeneralInfos(TDebug::INIT);
#if defined(SSE_VID_D3D)
  if(Disp.pD3D) // previous build crashed here when GDI was used
  {
    D3DFORMAT DisplayFormat=D3DFMT_X8R8G8B8; //32bit; D3DFMT_R5G6B5=16bit
    UINT nD3Dmodes=Disp.pD3D->GetAdapterModeCount(Disp.m_Adapter,DisplayFormat);
    //ASSERT(nD3Dmodes);
    D3DDISPLAYMODE Mode;
    Disp.pD3D->EnumAdapterModes(Disp.m_Adapter,DisplayFormat,nD3Dmodes-1,&Mode);
    for(int i=0;i<EXTMON_RESOLUTIONS;i++) 
    {
      if(!extmon_res[i][0])
        extmon_res[i][0]=Mode.Width;
      if(!extmon_res[i][1])
        extmon_res[i][1]=Mode.Height;
    }
  }
#endif
#ifdef ENABLE_LOGFILE
  EasyStr Mess="STARTUP: Display Init finished. ";
  switch(Disp.Method) {
  case DISPMETHOD_NONE:Mess+="It failed, nothing will be drawn.";break;
#ifdef SSE_VID_D3D
  case DISPMETHOD_DD:  Mess+="Direct3D will be used to draw.";break;
#else
  case DISPMETHOD_D3D:  Mess+="DirectDraw will be used to draw.";break;
#endif
  case DISPMETHOD_GDI: Mess+="The GDI will be used to draw.";break;
  case DISPMETHOD_X:   Mess+="Standard Xlib will be used to draw.";break;
  case DISPMETHOD_XSHM:Mess+="Xlib with shared memory extension will be used to draw.";break;
  case DISPMETHOD_BE:  Mess+="Standard Be will be used to draw.";break;
  }
  TRACE_INIT("%s\n",Mess.Text);
  DBG_LOG(Mess);
#endif
#if !defined(SSE_SOUND_NO_NOSOUND_OPTION)
#ifdef WIN32
  if(CSF.GetInt("Options","NoDirectSound",0)) 
    TrySound=0;
#else
  x_sound_lib=CSF.GetInt("Sound","Library",x_sound_lib);
  if(CSF.GetInt("Sound","NoPortAudio",0)&&CSF.GetInt("Sound","IgnoreNoPortAudio",0)==0) 
  {
    x_sound_lib=0;
    CSF.SetInt("Sound","IgnoreNoPortAudio",1);
  }
  TrySound=x_sound_lib!=0;
#endif
#endif
  if(TrySound && StepByStepInit) 
  {
    if(Alert(T("Would you like to disable sound for this session?")+" "+
      UNIX_ONLY(T("To permanently disable sound turn on Options->Startup->Never Use PortAudio."))
      WIN_ONLY(T("To permanently disable sound turn on Options->Startup->Never Use DirectSound.")),
      T("No Sound?"),MB_ICONQUESTION|MB_YESNO)==IDYES)
      TrySound=0;
  }
  if(TrySound) 
  {
    DBG_LOG("STARTUP: InitSound Called");
#if defined(SSE_SOUND_OPTION_DISABLE_DSP)
    DSP_DISABLED=CSF.GetInt("Sound","NoDsp",DSP_DISABLED);
#endif
    InitSound();
    DBG_LOG(EasyStr("STARTUP: InitSound finished. ")+LPSTR(UseSound ? "DirectSound will be used.":"DirectSound will not be used."));
  }
  if(CSF.GetInt("Options","RunOnStart",0)!=0)
    BootInMode|=BOOT_MODE_RUN;
  init_DirID_to_text();
  if(StepByStepInit) 
  {
    bool PortOpen=0;
    for(int p=0;p<3;p++) 
    {
      EasyStr PNam=EasyStr("Port_")+p+"_";
      if(CSF.GetInt("MIDI",PNam+"Type",PORTTYPE_NONE)) 
      {
        PortOpen=true;
        break;
      }
    }
    if(PortOpen) 
    {
      TRACE_INIT("PortOpen\n");
      if(Alert(T("Accessing parallel/serial/MIDI ports can cause Steem to freeze up or crash on some systems.")+" "+
        T("Do you want to stop Steem accessing these ports?"),
        T("Disable Ports?"),MB_ICONQUESTION|MB_YESNO)==IDYES) 
      {
        for(int p=0;p<3;p++) 
        {
          EasyStr PNam=EasyStr("Port_")+p+"_";
          CSF.SetInt("MIDI",PNam+"Type",PORTTYPE_NONE);
        }
      }
    }
  }
  CSF.SaveTo(globalINIFile); // Update the INI just in case a dialog does GetCSFInt
  DBG_LOG("STARTUP: LoadState Called");
#ifndef ONEGAME
  SetNotifyInitText(T("Loading state")); // can take quite some time if big disk
#endif
  LoadState(&CSF);
  DBG_LOG("STARTUP: LoadState finished");
  DBG_LOG("STARTUP: power_on Called");
#ifndef ONEGAME
  SetNotifyInitText(T("Power on"));
#endif
  power_on();
#ifdef WIN32
#if !defined(SSE_NO_UPDATE) && !defined(ONEGAME)
  if(CSF.GetInt("Update","AutoUpdateEnabled",true)) 
  {
    if(Exists(RunDir+"\\SteemUpdate.exe")) 
    {
      EasyStr Online=LPSTR(CSF.GetInt("Update","AlwaysOnline",0)?" online":"");
      EasyStr NoPatch=LPSTR(CSF.GetInt("Update","PatchDownload",true)==0?" nopatchcheck":"");
      EasyStr AskPatch=LPSTR(CSF.GetInt("Update","AskPatchInstall",0)?" askpatchinstall":"");
      WinExec(EasyStr("\"")+RunDir+"\\SteemUpdate.exe\" silent"+Online+NoPatch+AskPatch,SW_SHOW);
    }
  }
#endif
#endif
#ifdef DEBUG_BUILD
  DBG_LOG("STARTUP: update_register_display called");
  update_register_display(true);
#endif
  DBG_LOG("STARTUP: draw_init_resdependent called");
  draw_init_resdependent(); //set up palette conversion & stuff
  DBG_LOG("STARTUP: draw called");
  draw(true);
  DBG_LOG("STARTUP: draw finished");
#ifdef WIN32
  SendMessage(ToolTip,TTM_ACTIVATE,ShowTips,0);
  SetTimer(StemWin,SHORTCUTS_TIMER_ID,50,NULL);
#endif
#ifdef UNIX
  if(ShowTips) 
    hints.start();
  hxc::set_timer(StemWin,SHORTCUTS_TIMER_ID,50,timerproc,NULL);
#endif
#ifndef ONEGAME
#if defined(SSE_VID_D3D_MISC)
  bool snapshot_was_loaded=false;
#endif
  //TRACE("Disk A %s, statefile %s\n",BootDisk[0].Text,BootStateFile.Text);
  if(BootDisk[0].NotEmpty())
  {
    if(BootStateFile.NotEmpty()) 
    {
      //  Request: specify both memory snapshot and disks.
      int cnt=0;
      for(int drive=0;drive<2;drive++)
      {
        if(BootDisk[drive]!=".")
        {
          cnt++;
          EasyStr Name=GetFileNameFromPath(BootDisk[drive]);
          *strrchr(Name,'.')=0;
          DiskMan.InsertDisk(drive,Name,BootDisk[drive],0,0);
        }
      }
      if(cnt)
      {
        if(LoadSnapShot(BootStateFile,false,false,false))
          BootInMode|=BOOT_MODE_RUN;
      }
      else
        if(LoadSnapShot(BootStateFile)) 
          BootInMode|=BOOT_MODE_RUN;
#if defined(SSE_VID_D3D_MISC)
      snapshot_was_loaded=true;
#endif
      LastSnapShot=BootStateFile;
      TRACE_INIT("BootStateFile %s\n",BootStateFile.Text);
    }
    else
    {
#if USE_PASTI
      if(pasti_active)
      {
        // Check you aren't booting with pasti when passing a non-pasti compatible disk
        for(int drive=0;drive<2;drive++)
        {
          if(BootDisk[drive].NotEmpty()&&NotSameStr_I(BootDisk[drive],".")) 
          {
            if(ExtensionIsPastiDisk(strrchr(BootDisk[drive],'.'))==0) 
              BootPasti=BOOT_PASTI_OFF;
          }
        }
      }
      if(BootPasti!=BOOT_PASTI_DEFAULT) 
      {
        bool old_pasti=pasti_active;
        pasti_active=BootPasti==BOOT_PASTI_ON;
        //TRACE_LOG("pasti_active %d\n",pasti_active);
        if(DiskMan.IsVisible()&&old_pasti!=pasti_active) 
          DiskMan.RefreshDiskView();
      }
#endif
      for(int drive=0;drive<2;drive++) 
      {
        if(BootDisk[drive].NotEmpty()&&NotSameStr_I(BootDisk[drive],".")) 
        {
          EasyStr Name=GetFileNameFromPath(BootDisk[drive]);
          *strrchr(Name,'.')=0;
          //TRACE("insert %s in %c\n",BootDisk[drive].Text,drive+'A');
          if(DiskMan.InsertDisk(drive,Name,BootDisk[drive],0,0)) 
          {
            if(drive==0) 
              BootInMode|=BOOT_MODE_RUN;
          }
        }
      }
    }
  }
  else if(AutoLoadSnapShot && BootTOSImage==0)
  {
    if(Exists(WriteDir+SLASH+AutoSnapShotName+".sts")) 
    {
      bool Load=true;
      if(StepByStepInit) 
      {
        if(Alert(T("Would you like to restore the state of the ST?"),
          T("Restore State?"),MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2)==IDNO)
          Load=0;
      }
      if(Load) 
      {
        LoadSnapShot(WriteDir+SLASH+AutoSnapShotName+".sts",0,true,0); // Don't add to history, don't change disks
#if defined(SSE_VID_D3D_MISC)
        snapshot_was_loaded=true;
#endif
      }
    }
  }
#ifndef ONEGAME
  SetNotifyInitText(T("Get Ready..."));
#endif
  if(OptionBox.NeedReset())
    reset_st(RESET_COLD | RESET_STOP | RESET_CHANGESETTINGS | RESET_NOBACKUP);
  CheckResetDisplay();
  if(Disp.CanGoToFullScreen()) 
  {
    bool Full=(BootInMode & BOOT_MODE_FLAGS_MASK)==BOOT_MODE_FULLSCREEN;
    if((BootInMode & BOOT_MODE_FLAGS_MASK)==BOOT_MODE_DEFAULT) 
      Full=(CSF.GetInt("Options","StartFullscreen",0)!=0);
    if(Full)
    {
      TRACE_INIT("StartFullscreen\n");
#ifdef WIN32
      PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,2);
#endif
#ifdef UNIX
#endif
    }
  }
#else
  if(OGInit()==0) 
    QuitSteem();
  PostMessage(StemWin,WM_SYSCOMMAND,SC_MAXIMIZE,2);
#endif
#if defined(SSE_VID_D3D_MISC)
  if(!snapshot_was_loaded) // otherwise res_change() will erase starting pic
#endif
  res_change();
#ifndef ONEGAME
  DestroyNotifyInitWin();
#endif
#ifdef WIN32
  int ShowState=SW_SHOW;
  if(CSF.GetInt("Main","Maximized",0)) 
    ShowState=SW_MAXIMIZE;
#endif
  CSF.Close();
#ifdef WIN32
#ifdef DEBUG_BUILD
#if defined(SSE_DEBUGGER_TOGGLE)
  //TRACE("show Boiler %d\n",DebuggerVisible);
  if(DebuggerVisible)
#endif
    ShowWindow(DWin,SW_SHOW);
#endif
  ShowWindow(StemWin,ShowState);
  if(bAOT)
#if defined(SSE_VID_2SCREENS)
    SetWindowPos(StemWin,HWND_TOPMOST,Disp.rcMonitor.left,Disp.rcMonitor.top,
      0,0,SWP_NOMOVE | SWP_NOSIZE);
#else
    SetWindowPos(StemWin,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
#endif
#endif
#ifdef UNIX
  XMapWindow(XD,StemWin);
  XFlush(XD);
#endif
  SSEConfig.IsInit=TRUE;
  if(BootInMode & BOOT_MODE_RUN)
  {
    //if(GetForegroundWindow()==StemWin) 
    CLICK_PLAY_BUTTON();
  }
  return true;
}


void GetCurrentMemConf(BYTE MemConf[2]) {
  MemConf[0]=MEMCONF_512;
  MemConf[1]=MEMCONF_512;
  for(int i=0;i<2;i++) {
    for(BYTE n=0;n<N_MEMCONF;n++) {
      if(SSEConfig.bank_length[i]==mmu_bank_length_from_config[n]) 
      {
        MemConf[i]=n;
        break;
      }
    }
  }
}

#undef LOGSECTION


void QuitSteem() {
  Debug.TraceGeneralInfos(TDebug::EXIT);
  Quitting=true;
#ifdef WIN32
  if(runstate!=RUNSTATE_STOPPED)
  {
#if defined(SSE_EMU_THREAD)
    //ASSERT(bAppActive);
    if(OPTION_EMUTHREAD&&!bAppActive)
      bAppActive=TRUE;
#endif
    Glue.m_Status.stop_emu=1;
    PostMessage(StemWin,WM_CLOSE,0,0);
  }
  else if(FullScreen)
  {
    PostMessage(StemWin,WM_COMMAND,MAKEWPARAM(106,BN_CLICKED),
      (LPARAM)GetDlgItem(StemWin,106));
    PostMessage(StemWin,WM_CLOSE,0,0);
  }
  else 
  {
    draw_end();
    PostQuitMessage(0);
  }
#endif
#ifdef UNIX
  if(runstate==RUNSTATE_RUNNING)
    runstate=RUNSTATE_STOPPING;
  else if(FullScreen)
    Disp.ChangeToWindowedMode();
  XAutoRepeatOn(XD);
#endif
}


#define LOGSECTION LOGSECTION_SHUTDOWN

void CloseAllDialogs() {
  DBG_LOG("SHUTDOWN: Hiding ShortcutBox");
  ShortcutBox.Hide();
  DBG_LOG("SHUTDOWN: Hiding HardDiskMan");
  HardDiskMan.Hide();
  DBG_LOG("SHUTDOWN: Hiding DiskMan");
  DiskMan.Hide();
  DBG_LOG("SHUTDOWN: Hiding JoyConfig");
  JoyConfig.Hide();
  DBG_LOG("SHUTDOWN: Hiding InfoBox");
  InfoBox.Hide();
  DBG_LOG("SHUTDOWN: Hiding OptionBox");
  OptionBox.Hide();
  DBG_LOG("SHUTDOWN: Hiding PatchesBox");
  PatchesBox.Hide();
}


void PerformCleanShutdown() {
#ifndef ONEGAME
  //TRACE2("is init%d\n",SSEConfig.IsInit);
  // don't try to save if we just crashed, it could corrupt files
  if(SSEConfig.IsInit && StatusInfo.MessageIndex!=TStatusInfo::INTEL_CRASH)
  {
    DBG_LOG("SHUTDOWN: Opening settings file");
    TConfigStoreFile CSF(globalINIFile);
    DBG_LOG("SHUTDOWN: Saving visible dialog info");
    HardDiskMan.Hide();
    for(int n=0;n<nStemDialogs;n++) 
      DialogList[n]->SaveVisible(&CSF);
    DBG_LOG("SHUTDOWN: CloseAllDialogs()");
    CloseAllDialogs();
    DBG_LOG("SHUTDOWN: SaveState()");
    SaveState(&CSF);
    DBG_LOG("SHUTDOWN: Closing settings file");
    CSF.Close();
  }
#endif
  DBG_LOG("SHUTDOWN: CleanUpSteem()");
  CleanUpSteem();
  if(CrashFile.NotEmpty()) 
    DeleteFile(CrashFile);
  CrashFile="";
#if defined(SSE_VID_RECORD_AVI)
  if(pAviFile)
    delete pAviFile;
  pAviFile=NULL;
#endif
}


void CleanUpSteem() {
#ifdef WIN32
  KillTimer(StemWin,SHORTCUTS_TIMER_ID);
#endif
#ifdef UNIX
  hxc::kill_timer(StemWin,HXC_TIMER_ALL_IDS);
#endif
  macro_end(MACRO_ENDRECORD|MACRO_ENDPLAY);
  DBG_LOG("SHUTDOWN: Calling  CloseAllDialogs()");
  CloseAllDialogs();
  DBG_LOG("SHUTDOWN: Closing MIDIPort");
  MIDIPort.Close();
  DBG_LOG("SHUTDOWN: Closing Parallel Port");
  ParallelPort.Close();
  DBG_LOG("SHUTDOWN: Closing Serial Port");
  SerialPort.Close();
#ifndef DISABLE_STEMDOS
  DBG_LOG("SHUTDOWN: Closing all Stemdos files");
  stemdos_close_all_files();
#endif
  DBG_LOG("SHUTDOWN: Releasing Disp (DX shutdown)");
  Disp.Release();
  if(osd_plasma_pal) 
  {
    delete[] osd_plasma_pal; osd_plasma_pal=NULL;
    delete[] osd_plasma;     osd_plasma=NULL;
  }
  DBG_LOG("SHUTDOWN: Releasing Sound");
  SoundRelease();
  DBG_LOG("SHUTDOWN: Releasing Joysticks");
  FreeJoysticks();
  DBG_LOG("SHUTDOWN: Calling CleanupGUI()");
  CleanupGUI();
  DestroyKeyTable();

#ifdef WIN32
#if !defined(SSE_NO_UNZIPD32)
  if(hUnzip) 
    FreeLibrary(hUnzip); 
#if !defined(SSE_LEAN_AND_MEAN)
  enable_zip=false; hUnzip=NULL;
#endif
#endif
#if defined(SSE_ARCHIVEACCESS_SUPPORT)
  if(ARCHIVEACCESS_OK)
    UnloadArchiveAccessDll();
#if !defined(SSE_LEAN_AND_MEAN)
  ARCHIVEACCESS_OK=0;
#endif
#endif
#if defined(SSE_UNRAR_SUPPORT_WIN)
  if(hUnrar)
    FreeLibrary(hUnrar); 
#if !defined(SSE_LEAN_AND_MEAN)
  hUnrar=NULL;
#endif
#endif
#endif//WIN32

#ifdef UNIX
  if(XD) 
  {
    XCloseDisplay(XD);
    XD=NULL;
  }
#endif

  DBG_LOG("SHUTDOWN: Freeing cart memory");
  if(cart_save)
    cart=cart_save;
#if !defined(SSE_LEAN_AND_MEAN)
  cart_save=NULL;
#endif
  if(cart) 
  { 
    delete[] cart;
#if !defined(SSE_LEAN_AND_MEAN)
    cart=NULL; 
#endif
  }
  DBG_LOG("SHUTDOWN: Freeing RAM memory");
  if(STMem) 
  { 
    delete[] STMem;
#if !defined(SSE_LEAN_AND_MEAN)    
    STMem=NULL; 
#endif
  }
  DBG_LOG("SHUTDOWN: Freeing ROM memory");
  if(STRom) 
  { 
    delete[] STRom;
#if !defined(SSE_LEAN_AND_MEAN)    
    STRom=NULL;
#endif
  }
  DBG_LOG("SHUTDOWN: DeleteCriticalSection()");
  WIN_ONLY(DeleteCriticalSection(&agenda_cs); )
  DBG_LOG("SHUTDOWN: Deleting TranslateBuf");
  if(TranslateBuf) 
    delete[] TranslateBuf;
  if(TranslateUpperBuf) 
    delete[] TranslateUpperBuf;
#if !defined(SSE_LEAN_AND_MEAN)
  TranslateBuf=NULL;TranslateUpperBuf=NULL;
#endif
  DBG_LOG("SHUTDOWN: Deleting sound buffers");
  if(psg_channels_buf!=NULL)
    delete[] psg_channels_buf;
#if !defined(SSE_LEAN_AND_MEAN)
  psg_channels_buf=NULL;
#endif
  if(ste_sound_channel_buf!=NULL)
    delete[] ste_sound_channel_buf;
#if !defined(SSE_LEAN_AND_MEAN)
  ste_sound_channel_buf=NULL;
#endif
  DBG_LOG("SHUTDOWN: Closing logfile - bye!!!");
#ifdef ENABLE_LOGFILE
  if(logfile) 
    fclose(logfile);
#if !defined(SSE_LEAN_AND_MEAN)
  logfile=NULL;
#endif
#endif
  ONEGAME_ONLY(OGCleanUp(); )
  WIN_ONLY(if(SteemRunningMutex) CloseHandle(SteemRunningMutex); )
#if USE_PASTI
  if(hPasti) 
    FreeLibrary(hPasti);
#if !defined(SSE_LEAN_AND_MEAN)
  hPasti=NULL;
#endif
#endif
#if !defined(SSE_VID_NO_FREEIMAGE)
  if(Disp.hFreeImage)
    FreeLibrary(Disp.hFreeImage);
#if !defined(SSE_LEAN_AND_MEAN)
  Disp.hFreeImage=NULL;
#endif
#endif
#if defined(SSE_VID_STVL1)
  if(hStvl) 
    FreeLibrary(hStvl);
#if !defined(SSE_LEAN_AND_MEAN)
  hStvl=NULL;
#endif
#endif
}

#undef LOGSECTION
#define LOGSECTION LOGSECTION_INIT

bool ComLineArgCompare(char*Arg,char*s,bool truncate=false) {
  if(*Arg=='/'||*Arg=='-') 
    Arg++;
  if(*Arg=='-') 
    Arg++;
  if(truncate) 
    return(IsSameStr_I(EasyStr(Arg).Lefts(strlen(s)),s));
  return IsSameStr_I(Arg,s);
}


int GetComLineArgType(char *Arg,EasyStr &Path) {
  if(ComLineArgCompare(Arg,"NODD")||ComLineArgCompare(Arg,"GDI"))
    return ARG_GDI;
  else if(ComLineArgCompare(Arg,"NODS")||ComLineArgCompare(Arg,"NOSOUND"))
    return ARG_NODS;
  else if(ComLineArgCompare(Arg,"WINDOW"))
    return ARG_WINDOW;
  else if(ComLineArgCompare(Arg,"FULLSCREEN"))
    return ARG_FULLSCREEN;
  else if(ComLineArgCompare(Arg,"NONEW"))
    return ARG_NONEWINSTANCE;
  else if(ComLineArgCompare(Arg,"OPENNEW"))
    return ARG_ALWAYSNEWINSTANCE;
  else if(ComLineArgCompare(Arg,"NOLPT"))
    return ARG_NOLPT;
  else if(ComLineArgCompare(Arg,"NOCOM"))
    return ARG_NOCOM;
  else if(ComLineArgCompare(Arg,"NOSHM"))
    return ARG_NOSHM;
  else if(ComLineArgCompare(Arg,"SCLICK"))
    return ARG_SOUNDCLICK;
  else if(ComLineArgCompare(Arg,"HELP")||ComLineArgCompare(Arg,"H"))
    return ARG_HELP;
  else if(ComLineArgCompare(Arg,"QUITQUICKLY"))
    return ARG_QUITQUICKLY;
  else if(ComLineArgCompare(Arg,"DOUBLECHECKSHORTCUTS"))
    return ARG_DOUBLECHECKSHORTCUTS;
  else if(ComLineArgCompare(Arg,"DONTLIMITSPEED"))
    return ARG_DONTLIMITSPEED;
  else if(ComLineArgCompare(Arg,"ACCURATEFDC"))
    return ARG_ACCURATEFDC;
  else if(ComLineArgCompare(Arg,"NOPCJOYSTICKS"))
    return ARG_NOPCJOYSTICKS;
  else if(ComLineArgCompare(Arg,"OLDPORTIO"))
    return ARG_OLDPORTIO;
  else if(ComLineArgCompare(Arg,"ALLOWREADOPEN"))
    return ARG_ALLOWREADOPEN;
  else if(ComLineArgCompare(Arg,"NOINTS"))
    return ARG_NOINTS;
  else if(ComLineArgCompare(Arg,"STFMBORDER"))
    return ARG_STFMBORDER;
  else if(ComLineArgCompare(Arg,"SCREENSHOTUSEFULLNAME"))
    return ARG_SCREENSHOTUSEFULLNAME;
  else if(ComLineArgCompare(Arg,"SCREENSHOTALWAYSADDNUM"))
    return ARG_SCREENSHOTALWAYSADDNUM;
  else if(ComLineArgCompare(Arg,"ALLOWLPTINPUT"))
    return ARG_ALLOWLPTINPUT;
  else if(ComLineArgCompare(Arg,"NONOTIFYINIT"))
    return ARG_NONOTIFYINIT;
  else if(ComLineArgCompare(Arg,"PSGCAPTURE"))
    return ARG_PSGCAPTURE;
  else if(ComLineArgCompare(Arg,"CROSSMOUSE"))
    return ARG_CROSSMOUSE;
  else if(ComLineArgCompare(Arg,"RUN"))
    return ARG_RUN;
  else if(ComLineArgCompare(Arg,"GDIFSBORDER"))
    return ARG_GDIFSBORDER;
  else if(ComLineArgCompare(Arg,"PASTI"))
    return ARG_PASTI;
  else if(ComLineArgCompare(Arg,"NOPASTI"))
    return ARG_NOPASTI;
  else if(ComLineArgCompare(Arg,"NOAUTOSNAPSHOT"))
    return ARG_NOAUTOSNAPSHOT;
  else if(ComLineArgCompare(Arg,"SOF=",true)) 
  {
    Path=strchr(Arg,'=')+1;
    return ARG_SETSOF;
  }
  else if(ComLineArgCompare(Arg,"FONT=",true)) 
  {
    Path=strchr(Arg,'=')+1;
    return ARG_SETFONT;
  }
  else if(ComLineArgCompare(Arg,"SCREENSHOT=",true)) 
  {
    Path=strchr(Arg,'=')+1;
    return ARG_TAKESHOT;
  }
  else if(ComLineArgCompare(Arg,"SCREENSHOT",true)) 
  {
    Path="";
    return ARG_TAKESHOT;
  }
  else if(ComLineArgCompare(Arg,"PABUFSIZE=",true)) 
  {
    Path=strchr(Arg,'=')+1;
    return ARG_SETPABUFSIZE;
  }
  else if(ComLineArgCompare(Arg,"RTBUFSIZE",true)) 
  {
    Path=strchr(Arg,'=')+1;
    return ARG_RTBUFSIZE;
  }
  else if(ComLineArgCompare(Arg,"RTBUFNUM",true)) 
  {
    Path=strchr(Arg,'=')+1;
    return ARG_RTBUFNUM;
  }
  else if(ComLineArgCompare(Arg,"NOTRACE",true))
    return ARG_NOTRACE;
#if defined(SSE_UNIX_TRACE)
  else if(ComLineArgCompare(Arg,"TRACEFILE=",true)) 
  { //Y,N
    Path=strchr(Arg,'=')+1;
    return ARG_TRACEFILE;
  }
  else if(ComLineArgCompare(Arg,"LOGSECTION=",true)) 
  {
    Path=strchr(Arg,'=')+1;
    return ARG_LOGSECTION;
  }
#endif
  else 
  {
    int Type=ARG_UNKNOWN;
    char *pArg=Arg;
    if(ComLineArgCompare(Arg,"INI=",true)) 
    {
      pArg=strchr(Arg,'=')+1;
      Type=ARG_SETINIFILE;
    }
    else if(ComLineArgCompare(Arg,"TRANS=",true)) 
    {
      pArg=strchr(Arg,'=')+1;
      Type=ARG_SETTRANSFILE;
    }
    else if(ComLineArgCompare(Arg,"CUTS=",true)) 
    {
      pArg=strchr(Arg,'=')+1;
      Type=ARG_SETCUTSFILE;
    }
    Path.SetLength(MAX_PATH);
    GetLongPathName(pArg,Path,MAX_PATH);
    if(Type!=ARG_UNKNOWN) 
      return Type;
    char *dot=strrchr(GetFileNameFromPath(Path),'.');
    if(dot)
    {
      if(ExtensionIsDisk(dot))
        return ARG_DISKIMAGEFILE;
      else if(ExtensionIsPastiDisk(dot))
        return ARG_PASTIDISKIMAGEFILE;
      else if(IsSameStr_I(dot,".STS"))
        return ARG_SNAPSHOTFILE;
      else if(IsSameStr_I(dot,".STC"))
        return ARG_CARTFILE;
      else if(IsSameStr_I(dot,".PRG")||IsSameStr_I(dot,".APP")||IsSameStr_I(dot,".TOS"))
        return ARG_STPROGRAMFILE;
      else if(IsSameStr_I(dot,".GTP")||IsSameStr_I(dot,".TTP"))
        return ARG_STPROGRAMTPFILE;
      else if(IsSameStr_I(dot,".LNK"))
        return ARG_LINKFILE;
      else if(IsSameStr_I(dot,".IMG")||IsSameStr_I(dot,".ROM"))
        return ARG_TOSIMAGEFILE;
    }
    return ARG_UNKNOWN;
  }
}


void ParseCommandLine(int NumArgs,char *Arg[],int Level) {
  DBG_LOG("STARTUP: Command line arguments:");
  for(int n=0;n<NumArgs;n++) 
  {
    DBG_LOG(Str("     ")+Arg[n]);
    EasyStr Path;
    int Type=GetComLineArgType(Arg[n],Path);
    //TRACE("ARG %d %s %d\n",n,Path.Text,Type);
    switch(Type) {
    case ARG_GDI:    WIN_ONLY(TryDX=0; ) break;
    case ARG_NODS:   TrySound=0; break;
    case ARG_NOSHM:  UNIX_ONLY(TrySHM=0; ) break;
    case ARG_NOLPT:  AllowLPT=0; break;
    case ARG_NOCOM:  AllowCOM=0; break;
    case ARG_WINDOW: BootInMode&=~BOOT_MODE_FLAGS_MASK;BootInMode=BOOT_MODE_WINDOW; break;
    case ARG_FULLSCREEN: BootInMode&=~BOOT_MODE_FLAGS_MASK;BootInMode=BOOT_MODE_FULLSCREEN; break;
    case ARG_SETSOF: sound_comline_freq=atoi(Path);sound_chosen_freq=sound_comline_freq;break;
    case ARG_SOUNDCLICK: sound_click_at_start=true; break;
    case ARG_DOUBLECHECKSHORTCUTS: WIN_ONLY(DiskMan.DoExtraShortcutCheck=true; ) break;
    case ARG_DONTLIMITSPEED: disable_speed_limiting=true; break;
    case ARG_ACCURATEFDC: floppy_instant_sector_access=0; break;
    case ARG_NOPCJOYSTICKS: DisablePCJoysticks=true; break;
    case ARG_OLDPORTIO: WIN_ONLY(TPortIO::AlwaysUseNTMethod=0; ) break;
    case ARG_TAKESHOT:
      Disp.ScreenShotNextFile=Path;
      if(runstate==RUNSTATE_RUNNING)
        DoSaveScreenShot|=1;
      else
        Disp.SaveScreenShot();
      break;
    case ARG_SETPABUFSIZE:  UNIX_ONLY(pa_output_buffer_size=atoi(Path); ) break;
    case ARG_ALLOWREADOPEN: stemdos_comline_read_is_rb=true; break;
    case ARG_NOINTS:        no_ints=true; break; // SS removed _
    case ARG_STFMBORDER:
      ST_MODEL=STF; // to help DemobaseST just in case
      break;
    case ARG_SCREENSHOTUSEFULLNAME: Disp.ScreenShotUseFullName=true; break;
    case ARG_SCREENSHOTALWAYSADDNUM: Disp.ScreenShotAlwaysAddNum=true; break;
    case ARG_ALLOWLPTINPUT: comline_allow_LPT_input=true; break;
    case ARG_CROSSMOUSE: no_set_cursor_pos=true; break;
#if defined(UNIX) && !defined(NO_RTAUDIO)
    case ARG_RTBUFSIZE: rt_buffer_size=atoi(Path); break;
    case ARG_RTBUFNUM: rt_buffer_num=atoi(Path); break;
#endif
    case ARG_RUN: BootInMode|=BOOT_MODE_RUN; break;
#ifdef WIN32
    case ARG_GDIFSBORDER:   Disp.DrawLetterboxWithGDI=true; break;
#endif
    case ARG_PASTI: BootPasti=BOOT_PASTI_ON; break;
    case ARG_NOPASTI: BootPasti=BOOT_PASTI_OFF; break;
    case ARG_NOAUTOSNAPSHOT:
      BootDisk[0]=".";
      BootDisk[1]=".";
      break;
    case ARG_DISKIMAGEFILE:
        //TRACE("ARG_DISKIMAGEFILE %s A free %d B free %d\n",Path.Text,BootDisk[0].Empty(),BootDisk[1].Empty());
      if(BootDisk[1].Empty()||BootDisk[1]==".")
        BootDisk[int((BootDisk[0].Empty()||BootDisk[0]==".")?0:1)]=Path;
        //TRACE("Boot disks %s %s\n",BootDisk[0].Text,BootDisk[1].Text);
      break;
    case ARG_PASTIDISKIMAGEFILE:
      BootPasti=BOOT_PASTI_ON;
      if(BootDisk[1].Empty()) BootDisk[int(BootDisk[0].Empty()?0:1)]=Path;
      break;
    case ARG_SNAPSHOTFILE:
      BootDisk[0]=".";
      BootDisk[1]=".";
      BootStateFile=Path;
      TRACE_INIT("BootStateFile %s given as argument\n",BootStateFile.Text);
      break;
    case ARG_CARTFILE:
      if(load_cart(Path)==0)
      {
        CartFile=Path;
        OptionBox.MachineUpdateIfVisible();
      }
      break;
    case ARG_STPROGRAMFILE:
      // Mount folder as Z: (disable all normal hard drives)
      // Copy autorun program into auto folder
      break;
    case ARG_LINKFILE:
#ifdef WIN32
      if(Level<10)
      {
        WIN32_FIND_DATA wfd;
        Path=GetLinkDest(Path,&wfd);
        if(Path.NotEmpty())
          ParseCommandLine(1,&(Path.Text),Level+1);
      }
#endif
      break;
    case ARG_TOSIMAGEFILE:
      if(BootTOSImage==0)
      {
        if(load_TOS(Path)==0)
        {
          ROMFile=Path;
          BootTOSImage=true;
        }
      }
      break;
    }
  }
}


bool OpenComLineFilesInCurrent(bool AlwaysSendToCurrent) {
  EasyStringList esl;
  esl.Sort=eslNoSort;
  for(int n=0;n<_argc-1;n++) 
  {
    EasyStr Path;
    int Type=GetComLineArgType(_argv[1+n],Path);
#ifdef WIN32
    if(Type==ARG_LINKFILE) {
      WIN32_FIND_DATA wfd;
      int Level=0;
      EasyStr DestPath=Path;
      do {
        DestPath=GetLinkDest(DestPath,&wfd);
        if(DestPath.NotEmpty()) 
          break;
        Type=GetComLineArgType(DestPath,Path);
      } while(Type==ARG_LINKFILE&&(++Level)<10);
    }
#endif
    switch(Type) {
    case ARG_DISKIMAGEFILE:case ARG_PASTIDISKIMAGEFILE:case ARG_SNAPSHOTFILE:
    case ARG_CARTFILE:case ARG_TOSIMAGEFILE:
      esl.Add(Path,0);
    case ARG_TAKESHOT:
      esl.Add(_argv[1+n],0);
      break;
    case ARG_RUN:
      esl.Add(_argv[1+n],1);
      break;
    }
  }
  if(esl.NumStrings) 
  {
    bool RunOnly=true;
    for(int i=0;i<esl.NumStrings;i++) 
    {
      if(esl[i].Data[0]==0) 
      {
        RunOnly=0;
        break;
      }
    }
    // If you only pass the RUN command and haven't specified to open in current
    // then we shouldn't do anything, RUN is handled later.
    if(RunOnly && AlwaysSendToCurrent==0) 
      return 0;
    // Send strings to running Steem
#ifdef WIN32
    HWND CurSteemWin=FindWindow("Steem Window",NULL);
    if(CurSteemWin) 
    {
      bool Success=0;
      COPYDATASTRUCT cds;
      cds.dwData=MAKECHARCONST('S','C','O','M');
      for(int n=0;n<esl.NumStrings;n++) 
      {
        cds.cbData=(DWORD)strlen(esl[n].String)+1;
        cds.lpData=esl[n].String;
        if(SendMessage(CurSteemWin,WM_COPYDATA,0,LPARAM(&cds))
          ==MAKECHARCONST('Y','A','Y','S')) 
          Success=true;
      }
      if(Success) 
        return true;
      }
#endif
#ifdef UNIX
#endif
    }
  return 0;
  }
