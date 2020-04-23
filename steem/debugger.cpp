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

DOMAIN: Debug
FILE: debugger.cpp
CONDITION: DEBUG_BUILD must be defined
DESCRIPTION: This file contains a lot of utility functions for Steem's debug
build (now called the Debugger, before it was the Boiler but we're in serious
business here) and the basis of the debug GUI.
Don't confuse this debug build with the Visual Studio debug build (_DEBUG
is defined).
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#ifdef DEBUG_BUILD

#include <steemh.h>
#include <computer.h>
#include <debug.h>
#include <debugger.h>
#include <gui.h>
#include <draw.h>
#include <palette.h>
#include <osd.h>
#include <mymisc.h>
#include <diskman.h>
#include <input_prompt.h>
#include <debug_emu.h>
#include <debug_framereport.h>
#include <mr_static.h>
#include <debugger_trace.h>
#include <dwin_edit.h>
#include <key_table.h>

#if defined(SSE_DEBUGGER_TOGGLE)
BOOL DebuggerVisible=TRUE; // start as before with both windows (main + debugger)
#endif

DWORD_PTR dpc;
MEM_ADDRESS old_dpc;
HWND DWin=NULL,HiddenParent=NULL;
HMENU menu,breakpoint_menu,monitor_menu,breakpoint_irq_menu;
HMENU insp_menu=NULL;
HMENU mem_browser_menu,history_menu,logsection_menu;
HMENU menu1;
HMENU boiler_op_menu,shift_screen_menu;
HMENU logsection_menu2,sse_menu;
HWND sr_display,DWin_edit;
mr_static *lpms_other_sp;
HWND DWin_trace_button,DWin_trace_over_button,DWin_run_button;

Str LogViewProg="notepad.exe";

void boiler_show_stack_display(int);
ScrollControlWin DWin_timings_scroller;
HWND DWin_right_display_combo;

WNDPROC Old_sr_display_WndProc;

mem_browser m_b_mem_disa,m_b_stack;

HWND simultrace=NULL;

bool debug_monospace_disa=0,debug_uppercase_disa=0;

bool d2_trace=false;


#if defined(SSE_DEBUGGER_ALERTS_IN_STATUS_BAR)

void BoilerStatusBarMsg(char *Mess) { //v4.0
  // Main window status bar
   if(!Debug.PromptOnBreakpoint)
   {
     strncpy(StatusInfo.text,Mess,255);
     StatusInfo.MessageIndex=TStatusInfo::BOILER_MESSAGE;
     HWND status_bar_win=GetDlgItem(StemWin,IDC_STATUS_BAR); // get handle
     InvalidateRect(status_bar_win,NULL,false);
   }
#if defined(SSE_DEBUGGER_STATUS_BAR)
  // Debugger status bar
  SendMessage(hStatusBar,SB_SETTEXT,0,(LPARAM)Mess);
#endif
}

#endif


#if defined(SSE_DEBUGGER_STATUS_BAR)

HWND hStatusBar=NULL;

// Description: 
//   Creates a status bar and divides it into the specified number of parts.
// Parameters:
//   hwndParent - parent window for the status bar.
//   idStatus - child window identifier of the status bar.
//   hinst - handle to the application instance.
//   cParts - number of parts into which to divide the status bar.
// Returns:
//   The handle to the status bar.
//
HWND DoCreateStatusBar(HWND hwndParent,HMENU idStatus,HINSTANCE
  hinst,int cParts) {
  HWND hwndStatus;
  RECT rcClient;
  HLOCAL hloc;
  PINT paParts;
  int i,nWidth;

  // Ensure that the common control DLL is loaded.
//    InitCommonControls();

    // Create the status bar.
  hwndStatus=CreateWindowEx(
    0,                       // no extended styles
    STATUSCLASSNAME,         // name of status bar class
    (LPCSTR)NULL,           // no text when first created
    SBARS_SIZEGRIP|         // includes a sizing grip
    WS_CHILD|WS_VISIBLE,   // creates a visible child window
    0,0,0,0,              // ignores size and position
    hwndParent,              // handle to parent window
    (HMENU)idStatus,       // child window identifier
    hinst,                   // handle to application instance
    NULL);                   // no window creation data

// Get the coordinates of the parent window's client area.
  GetClientRect(hwndParent,&rcClient);

  // Allocate an array for holding the right edge coordinates.
  hloc=LocalAlloc(LHND,sizeof(int) * cParts);
  paParts=(PINT)LocalLock(hloc);

  // Calculate the right edge coordinate for each part, and
  // copy the coordinates to the array.
  nWidth=rcClient.right/cParts;
  int rightEdge=nWidth;
  for(i=0; i<cParts; i++)
  {
    paParts[i]=rightEdge;
    rightEdge+=nWidth;
  }

  // Tell the status bar to create the window parts.
  SendMessage(hwndStatus,SB_SETPARTS,(WPARAM)cParts,(LPARAM)paParts);

// Free the array, and return.
  LocalUnlock(hloc);
  LocalFree(hloc);

  return hwndStatus;
}

#endif


/////////////////////////////// insp menu ////////////////////////////////////
int insp_menu_subject_type;
void* insp_menu_subject;
long insp_menu_long[3];
char insp_menu_long_name[3][100];
int insp_menu_long_bytes[3];
void insp_menu_setup();
int insp_menu_col,insp_menu_row;

/////////////////////////////// breakpoints ////////////////////////////////////

DynamicArray<TDebugAddress> debug_ads;

/////////////////////////////// logfile  ////////////////////////////////////

//////////////////////////////// routines //////////////////////////////////

THistoryList HistList;


void debug_trace_crash(m68k_exception &e) {
  SendMessage(trace_window_handle,WM_SETTEXT,0,(LPARAM)"Exception");
  trace_init();
  trace_pc=e.ucrash_address.d32;
  trace_sr_before=e.crash_sr;
  update_register_display(true);
  trace_exception_display(&e);
  trace_display();
}


void debug_run_start() {
  ShowWindow(trace_window_handle,SW_HIDE);
  SetWindowText(DWin_run_button,"Stop");
#if defined(SSE_DEBUGGER_STATUS_BAR)
  SendMessage(hStatusBar,SB_SETTEXT,0,(LPARAM)"Running");
#endif  
}


void debug_run_end() {
  if(runstate_why_stop.NotEmpty())
  {
    TRACE("BREAKPOINT " PRICV " F%d y%d c%d %s\n",ACT,TIMING_INFO,runstate_why_stop.Text);
#if defined(SSE_DEBUGGER_ALERTS_IN_STATUS_BAR)
    BoilerStatusBarMsg(runstate_why_stop.Text);
    if(Debug.PromptOnBreakpoint)
#endif
    Alert(runstate_why_stop,"Break",0);
    runstate_why_stop="";
  }
#if defined(SSE_DEBUGGER_ALERTS_IN_STATUS_BAR)
#if defined(SSE_DEBUGGER_STATUS_BAR)
  else if(StatusInfo.MessageIndex!=TStatusInfo::BOILER_MESSAGE)
    SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)NULL);
#endif  
#endif
  trace_over_breakpoint=0xffffffff;
  SetWindowText(DWin_run_button,"Run");
  update_register_display(true);
  osd_hide();
}


void debug_reset() {
  update_register_display(true);
}


TDebugAddress* debug_find_address(MEM_ADDRESS ad) {
  for(int i=0;i<debug_ads.NumItems;i++)
    if(debug_ads[i].ad==ad) 
      return &debug_ads[i];
  return NULL;
}


TDebugAddress* debug_find_or_add_address(MEM_ADDRESS ad) {
  TDebugAddress *pda=debug_find_address(ad);
  if(pda==NULL)
  {
    TDebugAddress da={ad,1,0,{0,0},{0}};
    debug_ads.Add(da);
  }
  return debug_find_address(ad);
}


void debug_remove_address(MEM_ADDRESS ad) {
  bool Changed=0;
  for(int i=0;i<debug_ads.NumItems;i++)
  {
    if(debug_ads[i].ad==ad)
    {
      debug_ads.Delete(i--);
      Changed=true;
    }
  }
  if(Changed)
  {
    debug_update_bkmon();
    breakpoint_menu_setup();
    mem_browser_update_all();
  }
}


void debug_set_bk(MEM_ADDRESS ad,bool set) {
  TDebugAddress *pda=debug_find_or_add_address(ad);
  int new_val=int(set?BIT_0:0);
  if((pda->bwr & BIT_0)==new_val) 
    return;
  pda->bwr&=~BIT_0;
  pda->bwr|=new_val;
  if(pda->bwr==0&&pda->name[0]==0)
    debug_remove_address(ad);
  else
  {
    breakpoint_menu_setup();
    mem_browser_update_all();
  }
  debug_update_bkmon();
}


void debug_set_mon(MEM_ADDRESS ad,bool read,WORD mask) {
  if(ad>=0xe00000&&ad<MEM_IO_BASE) 
    return; // Monitors only on RAM and IO  
  TDebugAddress *pda=debug_find_or_add_address(ad);
  int bit=int(read?2:1);
  if(mask==0)
  {
    pda->bwr&=~(1<<bit);
    pda->mask[bit-1]=0;
  }
  else
  {
    pda->bwr|=1<<bit;
    pda->mask[bit-1]=mask;
  }
  if(pda->bwr==0&&pda->name[0]==0)
    debug_remove_address(ad);
  else
  {
    breakpoint_menu_setup();
    mem_browser_update_all();
  }
  debug_update_bkmon();
}


void debug_set_name(MEM_ADDRESS ad,EasyStr name) {
  TDebugAddress *pda=debug_find_or_add_address(ad);
  strcpy(pda->name,name.Lefts(63));
  if(pda->bwr==0&&pda->name[0]==0)
    debug_remove_address(ad);
  else
  {
    breakpoint_menu_setup();
    mem_browser_update_all();
  }
  debug_update_bkmon();
}


void debug_update_bkmon() {
  int *num[]={&debug_num_bk,&debug_num_mon_reads,&debug_num_mon_writes,
    &debug_num_mon_reads_io,&debug_num_mon_writes_io};
  MEM_ADDRESS *ad[]={debug_bk_ad,debug_mon_read_ad,debug_mon_write_ad,
    debug_mon_read_ad_io,debug_mon_write_ad_io};
  WORD *mask[]={NULL,debug_mon_read_mask,debug_mon_write_mask,
    debug_mon_read_mask_io,debug_mon_write_mask_io};
  for(int i=0;i<5;i++) 
    *(num[i])=0;
  for(int i=0;i<debug_ads.NumItems;i++)
  {
    int ad_mode=debug_ads[i].mode;
    if(ad_mode==1) 
      ad_mode=int((debug_ads[i].bwr&1)?breakpoint_mode:monitor_mode);
    if(ad_mode)
    {
      if((debug_ads[i].bwr & BIT_0)&&*(num[0])<MAX_BREAKPOINTS)
      {
        ad[0][*(num[0])]=debug_ads[i].ad;
        (*(num[0]))++;
      }
      int wrbase=1;
      if(debug_ads[i].ad>=MEM_IO_BASE)
        wrbase=3;
      else if(debug_ads[i].ad>=0xe00000)
        wrbase=0;
      if(wrbase)
      {
        // reads
        if((debug_ads[i].bwr & BIT_2)&&*(num[wrbase])<MAX_BREAKPOINTS)
        {
          ad[wrbase][*(num[wrbase])]=debug_ads[i].ad;
          mask[wrbase][*(num[wrbase])]=debug_ads[i].mask[1];
          (*(num[wrbase]))++;
        }
        // writes
        if((debug_ads[i].bwr & BIT_1)&&*(num[wrbase+1])<MAX_BREAKPOINTS)
        {
          ad[wrbase+1][*(num[wrbase+1])]=debug_ads[i].ad;
          mask[wrbase+1][*(num[wrbase+1])]=debug_ads[i].mask[0];
          (*(num[wrbase+1]))++;
        }
      }
    }
  }
}


int debug_get_ad_mode(MEM_ADDRESS ad) {
  TDebugAddress *pda=debug_find_address(ad);
  if(pda==NULL) 
    return 0;
  int ad_mode=pda->mode;
  if(ad_mode==1) 
    ad_mode=int((pda->bwr&1)?breakpoint_mode:monitor_mode);
  return ad_mode;
}


WORD debug_get_ad_mask(MEM_ADDRESS ad,bool read) {
  TDebugAddress *pda=debug_find_address(ad);
  if(pda==NULL) 
    return 0;
  return pda->mask[int(read?1:0)];
}


void debug_check_break_on_irq(int irq) { //only if breakpoints enabled
  if(breakpoint_mode==2&&break_on_irq[irq])
  {
    if(runstate==RUNSTATE_RUNNING)
    {
      runstate=RUNSTATE_STOPPING;
      runstate_why_stop=HEXSl(old_pc,6)+": "+name_of_mfp_interrupt[irq]
        +" Interrupt";
    }
  }
  if(debug_in_trace && irq!=BREAK_IRQ_LINEA_IDX && irq!=BREAK_IRQ_LINEF_IDX 
    && irq!=BREAK_IRQ_TRAP_IDX)
  {
    runstate_why_stop=HEXSl(old_pc,6)+": "+name_of_mfp_interrupt[irq]
      +" Interrupt";
  }
}


void breakpoint_menu_setup() {
  DWORD_PTR save_dpc=dpc;
  RemoveAllMenuItems(breakpoint_menu);
  RemoveAllMenuItems(breakpoint_irq_menu);
  RemoveAllMenuItems(monitor_menu);
  AppendMenu(breakpoint_irq_menu,MF_STRING|int(break_on_irq[BREAK_IRQ_HBL_IDX]
    ?MF_CHECKED:MF_UNCHECKED),
    9000+BREAK_IRQ_HBL_IDX,"HBL");
  AppendMenu(breakpoint_irq_menu,MF_STRING|int(break_on_irq[BREAK_IRQ_VBL_IDX]
    ?MF_CHECKED:MF_UNCHECKED),
    9000+BREAK_IRQ_VBL_IDX,"VBL");
  for(int n=0;n<16;n++)
  {
    AppendMenu(breakpoint_irq_menu,MF_STRING|int(break_on_irq[n]
      ? MF_CHECKED : MF_UNCHECKED),9000+n,name_of_mfp_interrupt[n]);
  }
  AppendMenu(breakpoint_irq_menu,MF_STRING|int(break_on_irq[BREAK_IRQ_SPURIOUS]
    ?MF_CHECKED:MF_UNCHECKED),
    9000+BREAK_IRQ_SPURIOUS,"Spurious"); //v4.0
  AppendMenu(breakpoint_irq_menu,MF_STRING|int(break_on_irq[BREAK_IRQ_LINEA_IDX]
    ?MF_CHECKED:MF_UNCHECKED),
    9000+BREAK_IRQ_LINEA_IDX,"Line-A");
  AppendMenu(breakpoint_irq_menu,MF_STRING|int(break_on_irq[BREAK_IRQ_LINEF_IDX]
    ?MF_CHECKED:MF_UNCHECKED),
    9000+BREAK_IRQ_LINEF_IDX,"Line-F");
  AppendMenu(breakpoint_irq_menu,MF_STRING|int(break_on_irq[BREAK_IRQ_TRAP_IDX]
    ?MF_CHECKED:MF_UNCHECKED),
    9000+BREAK_IRQ_TRAP_IDX,"Trap");
  AppendMenu(breakpoint_irq_menu,MF_SEPARATOR,0,NULL);
  AppendMenu(breakpoint_irq_menu,MF_STRING,9030,"Check All");
  AppendMenu(breakpoint_irq_menu,MF_STRING,9031,"Uncheck All");
  AppendMenu(breakpoint_menu,MF_STRING|int((breakpoint_mode==2)?MF_CHECKED:0),
    1107,"Stop On Breakpoints");
  AppendMenu(breakpoint_menu,MF_STRING|int((breakpoint_mode==3)?MF_CHECKED:0),
    1108,"Log On Breakpoints");
  AppendMenu(breakpoint_menu,MF_STRING,1100,"Clear All Breakpoints");
  AppendMenu(breakpoint_menu,MF_STRING,1101,"Set Breakpoint At PC");
  AppendMenu(breakpoint_menu,MF_SEPARATOR,0,NULL);
  AppendMenu(breakpoint_menu,MF_POPUP,(UINT_PTR)breakpoint_irq_menu,
    "Break On Interrupt");
#if USE_PASTI
  if(hPasti)
  {
    AppendMenu(breakpoint_menu,MF_SEPARATOR,0,NULL);
    AppendMenu(breakpoint_menu,MF_STRING,1109,"Pasti Breakpoints");
  }
#endif
  AppendMenu(monitor_menu,MF_STRING|int((monitor_mode==MONITOR_MODE_STOP)
    ?MF_CHECKED:0),1103,"Stop On Activation");
  AppendMenu(monitor_menu,MF_STRING|int((monitor_mode==MONITOR_MODE_LOG)
    ?MF_CHECKED:0),1104,"Log On Activation");
  AppendMenu(monitor_menu,MF_STRING,1106,"Clear All Monitored Addresses");
  AppendMenu(monitor_menu,MF_STRING,1105,"Set Monitor On Screen");
  Str t;
  AppendMenu(breakpoint_menu,MF_SEPARATOR,0,NULL);
  AppendMenu(monitor_menu,MF_SEPARATOR,0,NULL);
  for(int i=0;i<debug_ads.NumItems;i++)
  {
    if(debug_ads[i].bwr & BIT_0)
    {
      Str mode_text="Off";
      int ad_mode=debug_ads[i].mode;
      switch(ad_mode) {
      case 1:  stem_runmode=breakpoint_mode; break;
      case 2: mode_text="Stop"; break;
      case 3: mode_text="Log"; break;
      }
      t=HEXSl(debug_ads[i].ad,6)+" - "+mode_text+" - "+disa_d2(debug_ads[i].ad);
      if(debug_ads[i].name[0]) 
        t+=Str("  (")+debug_ads[i].name+")";
      AppendMenu(breakpoint_menu,MF_STRING,1110+i,t);
    }
    char *wr_text[]={"WRITE","READ"};
    for(int wr=0;wr<2;wr++)
    {
      if(debug_ads[i].bwr & (BIT_1<<wr))
      {
        Str mode_text="Off";
        int ad_mode=debug_ads[i].mode;
        switch(ad_mode) {
        case 1:  stem_runmode=monitor_mode; break;
        case 2: mode_text="Stop"; break;
        case 3: mode_text="Log"; break;
        }
        MEM_ADDRESS ad=debug_ads[i].ad;
        if(debug_ads[i].mask[wr]==0x00ff) 
          ad++;
        char *suff=".b";
        if(debug_ads[i].mask[wr]==0xffff) 
          suff=".w";
        t=HEXSl(ad,6)+suff+" - "+wr_text[wr]+" - "+mode_text;
        if(debug_ads[i].name[0]) 
          t+=Str("  (")+debug_ads[i].name+")";
        Tiolist_entry *io=search_iolist(ad);
        if(io) 
          t+=Str(" - ")+io->name;
        if(debug_ads[i].mask[wr]==0xffff)
        {
          Tiolist_entry *io2=search_iolist(ad+1);
          if(io2)
          {
            if(io)
              t+=" | ";
            else
              t+=" - ";
            t+=io2->name;
          }
        }
        AppendMenu(monitor_menu,MF_STRING,1110+i,t);
      }
    }
  }
  dpc=save_dpc;
}


void insp_menu_setup() {
  char ttt[150];
  for(int n=0;n<3;n++)
  {
    if(insp_menu_long_bytes[n])
    {
      if(insp_menu_long_bytes[n]>2)
      {
        strcpy(ttt,"New instruction browser at ");strcat(ttt,insp_menu_long_name[n]);
        AppendMenu(insp_menu,MF_ENABLED|MF_STRING,3010+n,ttt);
        strcpy(ttt,"New memory browser at ");strcat(ttt,insp_menu_long_name[n]);
        AppendMenu(insp_menu,MF_ENABLED|MF_STRING,3013+n,ttt);
      }
      HMENU pop=CreatePopupMenu();
      for(int m=0;m<NUM_REGISTERS_IN_REGISTER_BROWSER;m++)
      {
        AppendMenu(pop,MF_ENABLED|MF_STRING,4000+n*32+m,
          reg_browser_entry_name[m]);
      }
      strcpy(ttt,"Set register to ");strcat(ttt,insp_menu_long_name[n]);
      AppendMenu(insp_menu,MF_ENABLED|MF_STRING|MF_POPUP,(UINT_PTR)pop,ttt);
      AppendMenu(insp_menu,MF_SEPARATOR,0,NULL);
    }
  }
}


void update_register_display(bool reset_pc_display) {
  if(reset_pc_display)
  {
    m_b_mem_disa.ad=pc;
    m_b_stack.ad=Cpu.r[15];
  }
  UPDATE_SR;
#if defined(SSE_DEBUGGER_STACK_CHOICE)
  m_b_stack.ad=Debug.StackDisplayUseOtherSp?other_sp:Cpu.r[15];
  m_b_stack.ad&=0xFFFFFF;
#endif
  debug_update_cycle_counts();
  InvalidateRect(sr_display,NULL,0);
  InvalidateRect(trace_sr_before_display,NULL,0);
  InvalidateRect(trace_sr_after_display,NULL,0);
  mr_static_update_all();
  mem_browser_update_all();
#if defined(SSE_DEBUGGER_TIMERS_ACTIVE)
  InvalidateRect(HWND(DWin_timings_scroller),NULL,0);
#endif
}


LRESULT CALLBACK sr_display_WndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar) {
//  static wait_button_up;
  unsigned short *lpsr;
  int wid;
  switch(Mess) {
  case WM_PAINT:
  {
    lpsr=(unsigned short*)GetWindowLongPtr(Win,GWLP_USERDATA);
    if(lpsr==NULL) lpsr=&SR;
    PAINTSTRUCT ps;
    BeginPaint(Win,&ps);
    RECT box,rc;GetClientRect(Win,&box);
    HBRUSH bg_br=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    HBRUSH hi_br=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
    COLORREF hi_text_col=GetSysColor(COLOR_HIGHLIGHTTEXT);
    COLORREF std_text_col=GetSysColor(COLOR_WINDOWTEXT);
    wid=box.right;
    int x;
    HPEN pen=CreatePen(PS_SOLID,1,GetSysColor(COLOR_WINDOWTEXT));
    HFONT old_fnt=(HFONT)SelectObject(ps.hdc,fnt);
    char *lab="T.S..210...XNZVC";
    WORD mask=0x8000;
    SetBkMode(ps.hdc,TRANSPARENT);
    for(int n=0;n<16;n++)
    {
      x=(n*wid)/16;
      rc.left=x;rc.top=0;rc.right=((n+1)*wid)/16;rc.bottom=box.bottom;
      FillRect(ps.hdc,&rc,HBRUSH(((*lpsr)&mask)?hi_br:bg_br));
      SetTextColor(ps.hdc,COLORREF(((*lpsr)&mask)?hi_text_col:std_text_col));
      if(n)
      {
        MoveToEx(ps.hdc,x,0,NULL);
        LineTo(ps.hdc,x,box.bottom);
      }
      CentreTextOut(ps.hdc,x,0,(wid/16),box.bottom,lab+n,1);
      mask>>=1;
    }
    DeleteObject(hi_br);
    DeleteObject(bg_br);
    DeleteObject(pen);
    SelectObject(ps.hdc,old_fnt);
    EndPaint(Win,&ps);
    return 0;
  }
  case WM_LBUTTONDOWN:
  {
    RECT box;GetClientRect(Win,&box);
    int x=(short)LOWORD(lPar);
    if(x>=0)
    {
      int n=(16*x)/box.right;
      n=MAX(0,MIN(15,n));
      lpsr=(WORD*)GetWindowLongPtr(Win,GWLP_USERDATA);
      if(lpsr==NULL)lpsr=&SR;
      *lpsr^=(unsigned short)(0x8000>>n);
      InvalidateRect(Win,NULL,0);
      InvalidateRect(sr_display,NULL,0);
      InvalidateRect(trace_sr_after_display,NULL,0);
      mr_static_update_all(); // inefficiency unimportant here
    }
  }
  break;
  }
  return CallWindowProc(Old_sr_display_WndProc,Win,Mess,wPar,lPar);
}


#if defined(SSE_DEBUGGER_TOGGLE)

void DebuggerToggle(BOOL visible) {
  //ASSERT(visible==TRUE||visible==FALSE);
  //TRACE("FS%d DebuggerToggle(%d)\n",FullScreen,visible);
  DebuggerVisible=visible;
  SendMessage(GetDlgItem(StemWin,IDC_DEBUGGER),BM_SETCHECK,DebuggerVisible,0);
  int cmd=(visible==FALSE) ? SW_HIDE : SW_SHOW;
  ShowWindow(DWin,cmd);
  for(int n=0;n<MAX_MEMORY_BROWSERS;n++)
    if(m_b[n]) 
      ShowWindow(m_b[n]->owner,cmd);
  if(!visible)
  {
    if(HistList.IsVisible())
      HistList.Hide();
    if(trace_window_handle)
      ShowWindow(trace_window_handle,cmd);
  }
  else if(!FullScreen)
    //If the menu bar changes after the system has created the window, 
    // this function must be called to draw the changed menu bar.
    DrawMenuBar(DWin);
}

#endif


LRESULT CALLBACK DWndProc(HWND Win,UINT Mess,WPARAM wPar,LPARAM lPar) {
  if(Win==HiddenParent) 
    return DefWindowProc(Win,Mess,wPar,lPar);
  WORD wpar_lo=LOWORD(wPar);
  WORD wpar_hi=HIWORD(wPar);
  WORD lpar_lo=LOWORD(lPar);
  switch(Mess) {
#if defined(SSE_DEBUGGER_STATUS_BAR)
  case WM_CREATE:
  {
    hStatusBar=DoCreateStatusBar(Win,0,Inst,1);
    ASSERT(hStatusBar);
    break;
  }
#endif
#if defined(SSE_DEBUGGER_TIMERS_ACTIVE)
//http://msdn.microsoft.com/en-us/library/windows/desktop/bb787524(v=vs.85).aspx
  case WM_CTLCOLORSTATIC:
  {
    // reverse video timers in use
    MFP_CALC_INTERRUPTS_ENABLED; //call macro first
    MFP_CALC_TIMERS_ENABLED
    for(int n=0;n<4;n++)
    {
      bool enabled=
      //  ((sr & SR_IPL) < SR_IPL_6) && // SR control  ?
        mfp_interrupt_enabled[mfp_timer_irq[n]] //control register
        &&((Mfp.get_timer_control_register(n)&7)//&&n!=1 
          ||n==1&&(Mfp.get_timer_control_register(n)&8)); //event count B
      if(enabled && Debug.boiler_timer_hwnd[n]==(HWND)lPar)
      { // from the example
        HDC hdcStatic=(HDC)wPar;
        SetTextColor(hdcStatic,RGB(255,255,255));
        if(n==1&&(Mfp.get_timer_control_register(n)&8))
          SetBkColor(hdcStatic,RGB(128,128,128)); //event count B
        else
          SetBkColor(hdcStatic,RGB(0,0,0)); //regular timer
        return (INT_PTR)CreateSolidBrush(RGB(0,0,0));
      }
    }
    break;
  }
#endif
  case WM_SIZE:
    MoveWindow(DWin_trace_button,340,1,(lpar_lo-350)/4-5,27,true);
    MoveWindow(DWin_trace_over_button,340+(lpar_lo-350)/4,1,
      (lpar_lo-350)/4-5,27,true);
    MoveWindow(DWin_run_button,345+(lpar_lo-350)/2,1,
      (lpar_lo-350)/2-5,27,true);
    MoveWindow(GetDlgItem(Win,1020),340,30,lpar_lo-10-340-100-50,27*3/*200*/,true);//run to...
    MoveWindow(GetDlgItem(Win,1021),lpar_lo-10-100-50,30,95,27,true);
    MoveWindow(GetDlgItem(Win,1022),lpar_lo-10-50,30,50,27,true);
#if defined(SSE_DEBUGGER_STATUS_BAR)
    MoveWindow(m_b_mem_disa.handle,10,118,lpar_lo/2-13,
      HIWORD(lPar)-130-12,true);
    MoveWindow(DWin_timings_scroller,lpar_lo/2+3,148,lpar_lo/2-13,
      HIWORD(lPar)-160-12,true);
    MoveWindow(m_b_stack.handle,lpar_lo/2+3,148,lpar_lo/2-13,
      HIWORD(lPar)-160-12,true);
#else
    MoveWindow(m_b_mem_disa.handle,10,118,lpar_lo/2-13,
      HIWORD(lPar)-130,true);
    MoveWindow(DWin_timings_scroller,lpar_lo/2+3,148,lpar_lo/2-13,
      HIWORD(lPar)-160,true);
    MoveWindow(m_b_stack.handle,lpar_lo/2+3,148,lpar_lo/2-13,
      HIWORD(lPar)-160,true);
#endif
    SetWindowPos(DWin_right_display_combo,NULL,lpar_lo/2+3,118,
      lpar_lo/2-13,60,SWP_NOZORDER|SWP_NOREDRAW);
    m_b_mem_disa.update();
    m_b_stack.update();
#if defined(SSE_DEBUGGER_STATUS_BAR)
    SendMessage(hStatusBar,WM_SIZE,0,0);
#endif
    break;
  case WM_CLOSE:
#if defined(SSE_DEBUGGER_TOGGLE)
    DebuggerToggle(FALSE);
#else
    QuitSteem();
#endif
    return 0;
#if defined(SSE_DEBUGGER_TOGGLE)
  case WM_SHOWWINDOW:
    DebuggerToggle(wPar!=0);
    return 0;
#endif
  case WM_DESTROY:
//    m_b_mem_disa.active=false;
//    m_b_stack.active=false;
    DWin=NULL;
    break;
  case WM_CONTEXTMENU:
    if((HWND)wPar==DWin)
    {
      POINT pt={0,0};ClientToScreen(DWin,&pt);
      if(HIWORD(lPar)-pt.y<0) 
        break;
    }
    insp_menu_subject_type=78; //78=vague click
    insp_menu_subject=(void*)NULL;
    DeleteAllMenuItems(insp_menu);
    AppendMenu(insp_menu,MF_ENABLED|MF_STRING,3001,"New instruction browser at pc");
    AppendMenu(insp_menu,MF_ENABLED|MF_STRING,3002,"New memory browser at pc");
    AppendMenu(insp_menu,MF_ENABLED|MF_STRING,3003,"Register browser");
    TrackPopupMenu(insp_menu,TPM_LEFTALIGN|TPM_LEFTBUTTON,
      lpar_lo,HIWORD(lPar),0,DWin,NULL);
    break;
  case WM_COMMAND:
  {
    if(simultrace!=0&&simultrace!=SIMULTRACE_CHOOSE)
      SendMessage(simultrace,Mess,wPar,lPar);
    int id=wpar_lo;
    if(wpar_hi==STN_CLICKED||wpar_hi==BN_CLICKED)
    {
      if(id>=4000&&id<=4150)
      {
        id-=4000;
        int rn=id&31;
        int n=id/32;
        if((insp_menu_long_bytes[n])==2)
          *(WORD*)(reg_browser_entry_pointer[rn])=LOWORD(insp_menu_long[n]);
        else
          *(reg_browser_entry_pointer[rn])=insp_menu_long[n];
        update_register_display(true);
      }
      else if(id>=1110&&id<1200)
      {
        id-=1110;
        ETypeDispType type=DT_MEMORY;
        if(debug_ads[id].bwr & BIT_0) 
          type=DT_INSTRUCTION;
        new mem_browser(debug_ads[id].ad,type);
      }
      else if(id>=301&&id<400)
      {
        id-=300;
        logsection_enabled[id]=!logsection_enabled[id];
        CheckMenuItem((id<LOGSECTION_INIT)?logsection_menu:logsection_menu2,wpar_lo,
          MF_BYCOMMAND|int(logsection_enabled[id]?MF_CHECKED:MF_UNCHECKED));
        if(id==LOGSECTION_CPU)
        {
          if(logsection_enabled[id]) 
            log_cpu_count=CPU_INSTRUCTIONS_TO_LOG;
        }
      }
      else if(id>=950&&id<1000)
        SetForegroundWindow(m_b[id-950]->owner);
      else if(id>=17000&&id<40000)
        new mem_browser(pc_history[id-17000],DT_INSTRUCTION);
      else if(id>=40000&&id<50000)
      {
        id-=40000;
        if(id/100<debug_plugins.NumItems)
          debug_plugins[id/100].Activate(id%100); // mysterious...
      }
      else if(id>=9000&&id<9040)
      {
        id-=9000;
        bool set=0;
        switch(id)  {
        case 30: /*Check All*/
          set=true;
        case 31: /*Uncheck All*/
          for(int n=0;n<NUM_BREAK_IRQS;n++)
          {
            break_on_irq[n]=set;
            CheckMenuItem(breakpoint_irq_menu,9000+n,int(set?MF_CHECKED:MF_UNCHECKED));
          }
          break;
        default:
          break_on_irq[id]=!break_on_irq[id];
          CheckMenuItem(breakpoint_irq_menu,9000+id,int(break_on_irq[id]?MF_CHECKED:MF_UNCHECKED));
        }
      }
      else if(id>=3050&&id<3500)
      {
        mem_browser *mb=(mem_browser*)insp_menu_subject;
        int offset=((id-3050)/20)*2;
        MEM_ADDRESS ad=mb->get_address_from_row(insp_menu_row)+offset;
        int action=(id-3050)%20;
        int mask=-1;
        bool read=0;
        TDebugAddress *pda=debug_find_or_add_address(ad);
        switch(action) {
        case 0:
        {
          bool bk=0;
          if(pda) 
            bk=pda->bwr & BIT_0;
          debug_set_bk(ad,!bk);
          break;
        }
        case 1: // name address
        {
          EnableAllWindows(0,mb->owner);
          Str NewName=pda->name;
          if(InputPrompt_Choose(mb->owner,"Enter Address Name",NewName))
            debug_set_name(pda->ad,NewName);
          EnableAllWindows(true,mb->owner);
          break;
        }
        case 2: mask=0; break;
        case 3: mask=0xffff; break;
        case 4: mask=0xff00; break;
        case 5: mask=0x00ff; break;
        case 6: mask=0;     read=true; break;
        case 7: mask=0xffff;read=true; break;
        case 8: mask=0xff00;read=true; break;
        case 9: mask=0x00ff;read=true; break;
        case 16:case 17:case 18:case 19:
          pda->mode=action-16;
          breakpoint_menu_setup();
          mem_browser_update_all();
          debug_update_bkmon();
          break;
        }
        if(mask!=-1)
          debug_set_mon(ad,read,WORD(mask));
        if(pda->bwr==0&&pda->name[0]==0)
          debug_remove_address(ad);
      }
      else
      {
        switch(id) {
        case 3001:
          new mem_browser(pc,DT_INSTRUCTION);
          break;
        case 3002:
          new mem_browser(pc,DT_MEMORY);
          break;
        case 3003:
          new mem_browser(0,DT_REGISTERS);
          break;
        case 3010:case 3011:case 3012:
          new mem_browser((MEM_ADDRESS)(insp_menu_long[(wPar-3010)]),DT_INSTRUCTION);
          break;
        case 3013:case 3014:case 3015:
          new mem_browser((MEM_ADDRESS)(insp_menu_long[(wPar-3013)]),DT_MEMORY);
          break;
        case 3016: 
        {
          mr_static*ms=(mr_static*)insp_menu_subject;
          if(ms->editflag)
            set_DWin_edit(0,ms,0,0);
          break;
        }
        case 3025:
        {
          mem_browser*mb=(mem_browser*)insp_menu_subject;
          if(mb->editflag)
            set_DWin_edit(1,(void*)mb,insp_menu_row,insp_menu_col);
          break;
        }
        case 3026:
        {
          mem_browser *mb=(mem_browser*)insp_menu_subject;
          EnableAllWindows(0,mb->owner);
          Str NewName=GetWindowTextStr(mb->owner);
          if(InputPrompt_Choose(mb->owner,"Enter Browser Name",NewName))
            SetWindowText(mb->owner,NewName);
          EnableAllWindows(true,mb->owner);
          break;
        }
        case 3027:
        {
          mem_browser*mb=(mem_browser*)insp_menu_subject;
          debug_load_file_to_address(mb->owner,insp_menu_long[0]);
          break;
        }
        case 3028:
        {
          trace_over_breakpoint=insp_menu_long[0];
          LOG_TO(LOGSECTION_GUI,Str("GUI: Running until hit $")+HEXSl(trace_over_breakpoint,6));
          CLICK_PLAY_BUTTON();
          break;
        }
        case 104:
        {
          //disassemble file
          EasyStr sfn=FileSelect(DWin,"Select a program to disassemble",DiskMan.HomeFol,
            "ST Program Files\0*.PRG;*.APP;*.TOS;*.TTP;*.GTP\0All Files\0*.*\0\0",1,
            true,"PRG");
          if(sfn.NotEmpty())
          {
            EasyStr dfn=FileSelect(DWin,"Save disassembly as",WriteDir,
              "Source (.s)\0*.s\0text\0*.TXT\0All Files\0*.*\0\0",1,false,"s");
            if(dfn.NotEmpty())
            {
              FILE *sf=fopen(sfn.Text,"rb");
              if(sf)
              {
                int prg_len=filelength(fileno(sf))-28;
                fseek(sf,28,SEEK_SET);
    //            fread(Mem+0x400,prg_len,1,sf);
                for(int m=0;m<prg_len;m++)
                  PEEK(0x400+m)=(BYTE)fgetc(sf);
                fclose(sf);
                char *tp=dfn.Right();
                bool add_extn=true;
                for(INT_PTR m=0;m<dfn.Length();m++)
                {
                  if(*tp=='.')
                  {
                    add_extn=false;
                    break;
                  }
                  else if(*tp=='\\'||*tp=='/'||*tp==':')
                    break;
                  tp--;
                }
                if(add_extn) dfn+=".txt";
                FILE *f=fopen(dfn.Text,"wb");
                if(f)
                {
                  if(strcmpi(dfn.Rights(2),".S"))
                    disa_to_file(f,0x400,prg_len,false);
                  else
                    disa_to_file(f,0x400,prg_len,true);
                  fclose(f);
                }
                else
                {
                  MessageBox(NULL,EasyStr("Can't open file ")+dfn,"Oi!",
                    MB_ICONEXCLAMATION|MB_SETFOREGROUND|MB_TASKMODAL|MB_TOPMOST);
                }
              }
              else
              {
                MessageBox(NULL,EasyStr("Can't load file ")+sfn,"Oi!",
                  MB_ICONEXCLAMATION|MB_SETFOREGROUND|MB_TASKMODAL|MB_TOPMOST);
              }
            }

          }
          break;
        }
        case 1001:
        {
          //draw a file
          EasyStr sfn=FileSelect(DWin,"Select a picture",WriteDir,
            "Raw Image Files\0*.IMG\0All Files\0*.*\0\0",1,true,"IMG");
          if(sfn.NotEmpty())
          {
            FILE *sf=fopen(sfn.Text,"rb");
            if(sf)
            {
              int pic_len=filelength(fileno(sf));
              if(pic_len==32000)
              {
                if(vbase+32000>mem_len)
                  vbase=0x4000;
                for(int m=0;m<32000;m++)
                  PEEK(vbase+m)=(BYTE)fgetc(sf);
              }
              fclose(sf);
              draw(false);
            }
            else
            {
              MessageBox(NULL,EasyStr("Can't load file ")+sfn,"Oi!",
                MB_ICONEXCLAMATION|MB_SETFOREGROUND|MB_TASKMODAL|MB_TOPMOST);
            }
          }
          break;
        }
        case 1003:  //Step over
        {
          bool step_over=0;
          Str Instr=disa_d2(pc);
          LOG_TO(LOGSECTION_GUI,Str("GUI: Received step over click, instruction=\"")+Instr+"\"");
          if(IsSameStr_I(Instr.Lefts(2),"db")) //dbCC
            step_over=true;
          else if(MatchesAnyString_I(Instr.Lefts(3),"bsr","jsr",NULL))
            step_over=true;
          else if(MatchesAnyString_I(Instr.Lefts(4),"trap","stop","line",NULL))
            step_over=true;
          if(step_over)
          {
            trace_over_breakpoint=oi(pc,1);
            LOG_TO(LOGSECTION_GUI,Str("GUI: Running until hit $")+
              HEXSl(trace_over_breakpoint,6));
            CLICK_PLAY_BUTTON();
            break;
          }
          // Trace any other instruction
          LOG_TO(LOGSECTION_GUI,Str("GUI: Tracing"));
        }
        //no break
        case 1002:  //Trace into
          trace();
          break;
        case 1783: // Debugger reset
          SendMessage(Win,WM_COMMAND,905,0); // close all browsers
          SendMessage(Win,WM_COMMAND,1100,0); // clear all breakpoints
          if(breakpoint_mode!=2) SendMessage(Win,WM_COMMAND,1107,0);
          SendMessage(Win,WM_COMMAND,1106,0); // clear all monitors
          if(monitor_mode!=MONITOR_MODE_STOP)
            SendMessage(Win,WM_COMMAND,1103,0);
          SendMessage(Win,WM_COMMAND,9031,0); // turn off all irq breaks
          SendMessage(Win,WM_COMMAND,1009,0); // turn off all logsections
          SendMessage(Win,WM_COMMAND,1502,0); // notify on crash with bombs
          SendMessage(Win,WM_COMMAND,1600,0); // screen shift to 0;
          //if(logging_suspended) 
          SendMessage(Win,WM_COMMAND,1012,0); // reset = no steem.log
          SendMessage(Win,WM_COMMAND,1532,0);
          logfile_wipe();
          if(stop_on_blitter_flag) 
            SendMessage(Win,WM_COMMAND,1510,0);
          if(stop_on_user_change) 
            SendMessage(Win,WM_COMMAND,1512,0);
          if(stop_on_next_program_run) 
            SendMessage(Win,WM_COMMAND,1513,0);
          if(debug_cycle_colours) 
            SendMessage(Win,WM_COMMAND,1789,0);
          break;
        case 1004:  //reset
          reset_st(RESET_COLD|RESET_STOP|RESET_CHANGESETTINGS|RESET_BACKUP);
          break;
        case 1005:   //view logfile
          if(logfile) 
            ShellExecute(NULL,NULL,LogViewProg,LogFileName,"",SW_SHOW);
          break;
        case 1014:
        {
          EnableAllWindows(0,DWin);
          EasyStr CurLogPath=LogViewProg;
          RemoveFileNameFromPath(CurLogPath,REMOVE_SLASH);
          EasyStr FilNam=FileSelect(DWin,"Choose Program",CurLogPath,
            "Executables\0*.exe\0\0",1,true,"exe",GetFileNameFromPath(LogViewProg));
          if(FilNam.NotEmpty())
            LogViewProg=FilNam;
          SetForegroundWindow(DWin);
          EnableAllWindows(true,DWin);
          break;
        }
        case 1006:   //add mark in logfile
          if(logfile)
          {
            fprintf(logfile,"\r\n\r\n***************************************************************************\r\n");
            fprintf(logfile,"***************************************************************************\r\n");
            fprintf(logfile,"***************************************************************************\r\n\r\n\r\n");
            fflush(logfile);
          }
          break;
        case 1007: //wipe logfile
          logfile_wipe();
          break;
        case 1013:
          debug_wipe_log_on_reset=!debug_wipe_log_on_reset;
          CheckMenuItem(logsection_menu,1013,MF_BYCOMMAND|int(debug_wipe_log_on_reset
            ?MF_CHECKED:MF_UNCHECKED));
          break;
        case 1008: //simultrace
          if(simultrace==SIMULTRACE_CHOOSE)
          {
            simultrace=NULL;
            CheckMenuItem(menu1,1008,MF_BYCOMMAND|MF_UNCHECKED);
          }
          else if(simultrace)
          {
#ifdef SSE_BUILD // in other places, we changed directly
            SetWindowText(simultrace,"Debugger");
            SetWindowText(Win,"Debugger");
#else
            SetWindowText(simultrace,"The Boiler Room");
            SetWindowText(Win,"The Boiler Room");
#endif
            simultrace=NULL;
            CheckMenuItem(menu1,1008,MF_BYCOMMAND|MF_UNCHECKED);
          }
          else
          {
            MessageBox(NULL,"Move the mouse over the window that you want to control and press S, don't change the focus!","Simultrace",
              MB_TASKMODAL|MB_TOPMOST|MB_SETFOREGROUND);
            simultrace=SIMULTRACE_CHOOSE;
          }
          break;
        case 1009: //Uncheck all logsections
          for(int n=1;logsections[n].Name[0]!='*';n++)
          {
            int i=logsections[n].Index;
            if(i>=0)
            {
              logsection_enabled[i]=0;
              CheckMenuItem((i<LOGSECTION_INIT) ? logsection_menu :
                logsection_menu2,300+i,MF_BYCOMMAND|MF_UNCHECKED);
            }
          }
          break;
        case 1012: //suspend logging
          logging_suspended=!logging_suspended;
          CheckMenuItem(logsection_menu2,1012,MF_BYCOMMAND
            |(logging_suspended ? MF_UNCHECKED : MF_CHECKED)); // reversed
          EnableMenuItem(logsection_menu2,1532,MF_BYCOMMAND
            |(logging_suspended ? MF_DISABLED : MF_ENABLED));
          break;
        case 1532:
          Debug.log_in_trace=!Debug.log_in_trace;
          CheckMenuItem(logsection_menu2,1532,MF_BYCOMMAND
            |(Debug.log_in_trace ? MF_CHECKED : MF_UNCHECKED));
          break;
        case 1533:
          Debug.tracing_suspended=!Debug.tracing_suspended;
          CheckMenuItem(logsection_menu2,1533,MF_BYCOMMAND
            |(Debug.tracing_suspended ? MF_UNCHECKED : MF_CHECKED)); // reversed
          EnableMenuItem(logsection_menu2,1534,MF_BYCOMMAND
            |(Debug.tracing_suspended ? MF_DISABLED : MF_ENABLED));
          break;
        case 1534:
          Debug.trace_in_log=!Debug.trace_in_log;
          CheckMenuItem(logsection_menu2,1534,MF_BYCOMMAND
            |(Debug.trace_in_log ? MF_CHECKED : MF_UNCHECKED));
          break;
        case 1025: //redraw on stop
          redraw_on_stop=!redraw_on_stop;
          CheckMenuItem(boiler_op_menu,1025,MF_BYCOMMAND
            |(redraw_on_stop ? MF_CHECKED : MF_UNCHECKED));
          break;
        case 1026: //redraw after trace
          redraw_after_trace=!redraw_after_trace;
          CheckMenuItem(boiler_op_menu,1026,MF_BYCOMMAND
            |(redraw_after_trace ? MF_CHECKED : MF_UNCHECKED));
          break;
        case 1027: //Gun position colour
        {
          CHOOSECOLOR cc;
          cc.lStructSize=sizeof(cc);
          cc.hwndOwner=DWin;
          cc.rgbResult=debug_gun_pos_col;
          COLORREF CustCols[16];
          for(int n=0;n<16;n++) CustCols[n]=0;
          cc.lpCustColors=CustCols;
          cc.Flags=CC_FULLOPEN|CC_RGBINIT;
          if(ChooseColor(&cc))
          {
            debug_gun_pos_col=cc.rgbResult;
#if !defined(SSE_DEBUGGER)
            update_display_after_trace();
#endif
          }
          break;
        }
        case 1501:case 1502:case 1503:  //crash notification
#if defined(SSE_DEBUGGER_EXCEPTION_NOT_TOS)
        case 1504:
#endif
          crash_notification=wpar_lo-1501;
          CheckMenuRadioItem(boiler_op_menu,1501,
#if defined(SSE_DEBUGGER_EXCEPTION_NOT_TOS)
            1504,
#else
            1503,
#endif
            1501+crash_notification,MF_BYCOMMAND);
          break;
        case 1510:
          stop_on_blitter_flag=!stop_on_blitter_flag;
          CheckMenuItem(boiler_op_menu,1510,int(stop_on_blitter_flag
            ?(MF_BYCOMMAND|MF_CHECKED):(MF_BYCOMMAND|MF_UNCHECKED)));
          break;
        case 1512:
          stop_on_user_change=!stop_on_user_change;
          CheckMenuItem(boiler_op_menu,1512,MF_BYCOMMAND|int(stop_on_user_change
            !=0?MF_CHECKED:MF_UNCHECKED));
          break;
        case 1513:
          stop_on_next_program_run=int(stop_on_next_program_run?0:1);
          CheckMenuItem(boiler_op_menu,1513,MF_BYCOMMAND|
            int(stop_on_next_program_run?MF_CHECKED:MF_UNCHECKED));
          break;
        case 1531:
          stop_on_next_reset=int(stop_on_next_reset?0:1);
          CheckMenuItem(boiler_op_menu,1531,MF_BYCOMMAND|
            int(stop_on_next_reset?MF_CHECKED:MF_UNCHECKED));
          break;
        case 1514:
          trace_show_window=!trace_show_window;
          CheckMenuItem(boiler_op_menu,1514,MF_BYCOMMAND|
            int(trace_show_window?MF_CHECKED:MF_UNCHECKED));
          break;
        case 1515:
          debug_monospace_disa=!debug_monospace_disa;
          CheckMenuItem(boiler_op_menu,1515,MF_BYCOMMAND|
            int(debug_monospace_disa?MF_CHECKED:MF_UNCHECKED));
          mem_browser_update_all();
          break;
        case 1516:
          debug_uppercase_disa=!debug_uppercase_disa;
          CheckMenuItem(boiler_op_menu,1516,MF_BYCOMMAND|
            int(debug_uppercase_disa?MF_CHECKED:MF_UNCHECKED));
          mem_browser_update_all();
          break;
        case 1518: // Limit TRACE file size
          TRACE_FILE_REWIND=!TRACE_FILE_REWIND;
          CheckMenuItem(sse_menu,1518,
            MF_BYCOMMAND|((int)(TRACE_FILE_REWIND)
              ? MF_CHECKED : MF_UNCHECKED));
          break;
#if defined(SSE_DEBUGGER_MONITOR_VALUE)
        case 1522:
          Debug.MonitorValueSpecified=!Debug.MonitorValueSpecified;
          CheckMenuItem(sse_menu,1522,
            MF_BYCOMMAND|((int)(Debug.MonitorValueSpecified)
              ? MF_CHECKED : MF_UNCHECKED));
          break;
#endif
#if defined(SSE_DEBUGGER_MONITOR_RANGE)
        case 1523:
          Debug.MonitorRange=!Debug.MonitorRange;
          CheckMenuItem(sse_menu,1523,
            MF_BYCOMMAND|((int)(Debug.MonitorRange)
              ? MF_CHECKED : MF_UNCHECKED));
          break;
#endif
#if defined(SSE_DEBUGGER_DUMP_6301_RAM)
        case 1524:
          hd6301_dump_ram();
          if(Debug.trace_file_pointer)
            fflush(Debug.trace_file_pointer);
          break;
#endif
        case 1530: // Dump Registers
          UPDATE_SR; // just in case!
          TRACE("SR=%04X D0=%X D1=%X D2=%X D3=%X D4=%X D5=%X D6=%X D7=%X\n",SR,Cpu.r[0],Cpu.r[1],Cpu.r[2],Cpu.r[3],Cpu.r[4],Cpu.r[5],Cpu.r[6],Cpu.r[7]);
          TRACE("PC=%X A0=%X A1=%X A2=%X A3=%X A4=%X A5=%X A6=%X A7=%X\n",pc,Cpu.r[8],Cpu.r[9],Cpu.r[10],Cpu.r[11],Cpu.r[12],Cpu.r[13],Cpu.r[14],sp);
          if(Debug.trace_file_pointer)
            fflush(Debug.trace_file_pointer);
          break;
#if defined(SSE_DEBUGGER_STACK_CHOICE)
/*  v3.6.1 Handy feature, in the SSE menu you may decide which version
    of the stack will be displayed. As usual USP or SSP depends
    on supervisor bit.
*/
        case 1528:
          Debug.StackDisplayUseOtherSp=!Debug.StackDisplayUseOtherSp;
          CheckMenuItem(sse_menu,1528,
            MF_BYCOMMAND|((int)(Debug.StackDisplayUseOtherSp)
              ? MF_CHECKED : MF_UNCHECKED));
          update_register_display(false);
          break;
#endif
#if defined(SSE_DEBUGGER_FAKE_IO)
        case 1527: // fake IO zone for Debugger control
          new mem_browser(FAKE_IO_START,DT_MEMORY);
          break;
#endif
#if defined(SSE_DEBUGGER_FRAME_REPORT)
        case 1529:
          FrameEvents.Report();
          break;
#endif
#if defined(SSE_DEBUGGER_ALERTS_IN_STATUS_BAR)
        case 1535:
          Debug.PromptOnBreakpoint=!Debug.PromptOnBreakpoint;
          CheckMenuItem(sse_menu,1535,
            MF_BYCOMMAND|((int)(Debug.PromptOnBreakpoint)
              ? MF_CHECKED : MF_UNCHECKED));
          break;
#endif
        case 1780: //turn screen red
        {
          MEM_ADDRESS ad=vbase;
          for(int o=32000/8;o>0;o--)
          {
            m68k_lpoke(ad,0xffff0000);ad+=4;
            m68k_lpoke(ad,0);ad+=4;
          }
          draw(false);
          break;
        }
        case 1781: //Send MIDI messages
          if(MIDIPort.MIDI_In==NULL) 
            return 0;
          if(MIDIPort.MIDI_In->Handle==NULL) 
            return 0;
          for(int n=1;n<60;n++) 
            MIDIPort.MIDI_In->SysExHeader[0].lpData[n]=b00111100;
          MIDIPort.MIDI_In->SysExHeader[0].lpData[0]=b11110000;
          MIDIPort.MIDI_In->SysExHeader[0].lpData[60]=b11110111;
          MIDIPort.MIDI_In->SysExHeader[0].dwBytesRecorded=61;
          MIDIPort.MIDI_In->InProc(MIDIPort.MIDI_In->Handle,MIM_LONGDATA,
            (DWORD_PTR)MIDIPort.MIDI_In,(DWORD_PTR)&MIDIPort.MIDI_In->
            SysExHeader[0],0);
          MIDIPort.MIDI_In->InProc(MIDIPort.MIDI_In->Handle,MIM_DATA,
            (DWORD_PTR)MIDIPort.MIDI_In,MAKEWORD(b10110001,0),0);
          MIDIPort.MIDI_In->InProc(MIDIPort.MIDI_In->Handle,MIM_DATA,
            (DWORD_PTR)MIDIPort.MIDI_In,MAKEWORD(b10101010,3),0);
          MIDIPort.MIDI_In->SysExHeader[0].lpData[0]=b11110000;
          for(int n=1;n<32;n++) MIDIPort.MIDI_In->SysExHeader[0].lpData[n]=char(n);
          MIDIPort.MIDI_In->SysExHeader[0].lpData[32]=b11110111;
          MIDIPort.MIDI_In->SysExHeader[0].dwBytesRecorded=33;
          MIDIPort.MIDI_In->InProc(MIDIPort.MIDI_In->Handle,MIM_LONGDATA,
            (DWORD_PTR)MIDIPort.MIDI_In,(DWORD_PTR)&MIDIPort.MIDI_In->
            SysExHeader[0],0);
          break;
        case 1782: // Send All Keys
          debug_send_alt_keys=0x2;
          debug_send_alt_keys_vbl_countdown=1;
          break;
        case 1789:
          if(debug_cycle_colours)
          {
            debug_cycle_colours=0;
            palette_convert_all();
          }
          else
            debug_cycle_colours=1;
          CheckMenuItem(boiler_op_menu,1789,MF_BYCOMMAND|int(debug_cycle_colours
            ?MF_CHECKED:MF_UNCHECKED));
          draw(false);
          break;
        case 1600:case 1601:case 1602:case 1603:
          debug_screen_shift=(wpar_lo-1600)*2;
          CheckMenuRadioItem(shift_screen_menu,1600,1603,1600
            +(debug_screen_shift/2),MF_BYCOMMAND);
          draw(false);
          break;
        case 1010:  //run
        {
          CLICK_PLAY_BUTTON();
          break;
        }
        case 1011:  //run to rte
        {
          if(runstate==RUNSTATE_STOPPED)
          {
            on_rte=ON_RTE_STOP;
            on_rte_interrupt_depth=interrupt_depth;
            CLICK_PLAY_BUTTON();
          }
          break;
        }
        case 1015:  //run to rts
        {
          if(runstate==RUNSTATE_STOPPED)
          {
            on_rte=ON_RTS_STOP;
            CLICK_PLAY_BUTTON();
          }
          break;
        }
        case 1100:  //clear all breakpoints
          for(int i=0;i<debug_ads.NumItems;i++)
          {
            if(debug_ads[i].bwr & BIT_0)
            {
              debug_ads[i].bwr&=~BIT_0;
              if(debug_ads[i].bwr==0) debug_ads.Delete(i--);
            }
          }
          debug_update_bkmon();
          breakpoint_menu_setup();
          mem_browser_update_all();
          break;
        case 1107:   //toggle breakpoint checking
        case 1108:   //toggle breakpoint checking to logfile
          if(id==1107) 
            breakpoint_mode=int((breakpoint_mode==2)?0:2);
          if(id==1108) 
            breakpoint_mode=int((breakpoint_mode==3)?0:3);
          mem_browser_update_all();
          breakpoint_menu_setup();
          debug_update_bkmon();
          CheckMenuItem(breakpoint_menu,1107,MF_BYCOMMAND|
            int((breakpoint_mode==2)?MF_CHECKED:MF_UNCHECKED));
          CheckMenuItem(breakpoint_menu,1108,MF_BYCOMMAND|
            int((breakpoint_mode==3)?MF_CHECKED:MF_UNCHECKED));
          break;
        case 1101:   //set breakpoint at pc
          debug_set_bk(pc,true);
          break;
        case 1106:  //clear all monitors
          for(int i=0;i<debug_ads.NumItems;i++)
          {
            if(debug_ads[i].bwr & (BIT_1|BIT_2))
            {
              debug_ads[i].bwr&=~(BIT_1|BIT_2);
              if(debug_ads[i].bwr==0) 
                debug_ads.Delete(i--);
            }
          }
          debug_update_bkmon();
          breakpoint_menu_setup();
          mem_browser_update_all();
          break;
        case 1103:   //toggle monitoring
        case 1104:   //toggle monitoring to logfile
        case 1105:   //set monitor on screen
          if(id==1103) monitor_mode=int((monitor_mode==MONITOR_MODE_STOP)
            ?0:MONITOR_MODE_STOP);
          if(id==1104) monitor_mode=int((monitor_mode==MONITOR_MODE_LOG)
            ?0:MONITOR_MODE_LOG);
          if(id==1105)
          {
            monitor_mode=MONITOR_MODE_STOP;
            stem_mousemode=STEM_MOUSEMODE_BREAKPOINT;
          }
          mem_browser_update_all();
          breakpoint_menu_setup();
          debug_update_bkmon();
          CheckMenuItem(monitor_menu,1103,MF_BYCOMMAND|
            int((monitor_mode==MONITOR_MODE_STOP)?MF_CHECKED:MF_UNCHECKED));
          CheckMenuItem(monitor_menu,1104,MF_BYCOMMAND|
            int((monitor_mode==MONITOR_MODE_LOG)?MF_CHECKED:MF_UNCHECKED));
          break;
#if USE_PASTI
        case 1109:
          if(hPasti)
            pasti->DlgBreakpoint(Win);
          break;
#endif
        case 1999:  //quit
          QuitSteem();
          break;
        case 2200:
          HistList.Show();
          break;
        //////////////////// Browsers Menu
        case 900:case 901:
        {
          new mem_browser(pc,ETypeDispType(wpar_lo==900
            ?DT_MEMORY:DT_INSTRUCTION));
#if !defined(SSE_DEBUGGER_MOUSE_WHEEL)
          // we don't want to preselect address, we want the wheel to work at once
          SetFocus(GetDlgItem(mb->owner,3));
          SendMessage(GetDlgItem(mb->owner,3),WM_LBUTTONDOWN,0,0);
#endif
          break;
        }
        case 902:
          new mem_browser(0,DT_REGISTERS);
          break;
        case 903:
          new mem_browser(IOLIST_PSEUDO_AD_PSG,DT_MEMORY);
          break;
        case 904:
          new mem_browser(0xfffa00,DT_MEMORY);
          break;
        case 906:
        {
          mem_browser *mb=new mem_browser;
          mb->init_text=true;
          mb->new_window(MEM_START_OF_USER_AREA,DT_MEMORY);
          SetFocus(GetDlgItem(mb->owner,3));
          SendMessage(GetDlgItem(mb->owner,3),WM_LBUTTONDOWN,0,0);
          break;
        }
        case 905:
          for(int n=0;n<MAX_MEMORY_BROWSERS;n++)
            if(m_b[n]!=NULL)
              PostMessage(m_b[n]->owner,WM_CLOSE,0,0);
          break;
        case 907:
          if(mem_browser::ex_style)
            mem_browser::ex_style=0;
          else
            mem_browser::ex_style=WS_EX_TOOLWINDOW;
          CheckMenuItem(mem_browser_menu,907,MF_BYCOMMAND|
            int(mem_browser::ex_style?0:MF_CHECKED));
          break;
        case 908:
          new mem_browser(IOLIST_PSEUDO_AD_FDC,DT_MEMORY);
          break;
        case 909:
          new mem_browser(IOLIST_PSEUDO_AD_IKBD,DT_MEMORY);
          break;
#if USE_PASTI
        case 910:
          if(hPasti==NULL) break;
          pasti->DlgStatus(DWin);
          break;
#endif
        case 911:
          new mem_browser(0x000008,DT_MEMORY);
          break;
        case 925: // Bus Error
          new mem_browser(LPEEK(0x8),DT_INSTRUCTION);
          break;
        case 926: // Illegal
          new mem_browser(LPEEK(0x10),DT_INSTRUCTION);
          break;
        case 927: // Trace
          new mem_browser(LPEEK(0x24),DT_INSTRUCTION);
          break;
        case 918: // timer A
          new mem_browser(LPEEK(0x134),DT_INSTRUCTION);
          break;
        case 919: // timer B
          new mem_browser(LPEEK(0x120),DT_INSTRUCTION);
          break;
        case 920: // timer C
          new mem_browser(LPEEK(0x114),DT_INSTRUCTION);
          break;
        case 921: // timer D
          new mem_browser(LPEEK(0x110),DT_INSTRUCTION);
          break;
        case 922: // ACIA
          new mem_browser(LPEEK(0x118),DT_INSTRUCTION);
          break;
        case 923: // HBI
          new mem_browser(LPEEK(0x68),DT_INSTRUCTION);
          break;
        case 924: // VBI
          new mem_browser(LPEEK(0x70),DT_INSTRUCTION);
          break;
        case 912:
          new mem_browser(0xFF8240,DT_MEMORY);
          break;
        case 913:
          new mem_browser(0xFF8900,DT_MEMORY);
          break;
#if defined(SSE_DEBUGGER_BROWSER_6301)
        case 914:
          new mem_browser(IOLIST_PSEUDO_AD_6301,DT_MEMORY);
          break;
#endif
        case 915:
          new mem_browser(0xFF8A00,DT_MEMORY);
          break;
        case 916:
          new mem_browser(0xFFFC00,DT_MEMORY);
          break;
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
        case 917:
          new mem_browser(IOLIST_PSEUDO_AD_STACK,DT_MEMORY);
          break;
#endif
        case 1022:// this is when you click on 'Go'
        {
          DWORD dat=(DWORD)CBGetSelectedItemData(GetDlgItem(Win,1020));
          debug_run_until=LOWORD(dat);
          int len=(int)SendDlgItemMessage(Win,1021,WM_GETTEXTLENGTH,0,0)+1;
          EasyStr valstr;
          valstr.SetLength(len);
          SendDlgItemMessage(Win,1021,WM_GETTEXT,len,LPARAM(valstr.Text));
          debug_run_until_val=atoi(valstr);
          if(debug_run_until==DRU_CYCLE)
            debug_run_until_val+=ABSOLUTE_CPU_TIME;
          else if(debug_run_until==DRU_INSTCHANGE)
          {
            debug_run_until=DRU_OFF;
            Str cur_inst=disa_d2(pc);
            char *spc=strchr(cur_inst,' ');
            if(spc) 
              *spc=0;
            if((cur_inst[0]=='b' && cur_inst.Length()==3)||cur_inst.Lefts(2)
              =="db"||cur_inst[0]=='j') 
              break;
            MEM_ADDRESS new_pc=pc;
            for(;;)
            {
              new_pc=oi(new_pc,1);
              if(new_pc==0) 
                break;
              Str new_inst=disa_d2(new_pc);
              spc=strchr(new_inst,' ');
              if(spc) 
                *spc=0;
              if(NotSameStr_I(cur_inst,new_inst))
              {
                trace_over_breakpoint=new_pc;
                break;
              }
            }
            if(new_pc==0) 
              break;
          }
          if(runstate==RUNSTATE_STOPPED)
          {
            CLICK_PLAY_BUTTON();
          }
          break;
        }
        }  //end switch
      }
    }
    if(id==209&&wpar_hi==CBN_SELENDOK) 
      boiler_show_stack_display(-1);
    break;
  }
  case WM_CHAR:
  {
    if(wPar=='S'||wPar=='s')
    {
      if(simultrace==SIMULTRACE_CHOOSE)
      {
        POINT pt;
        GetCursorPos(&pt);
        HWND sw=WindowFromPoint(pt);
        if(sw)
        {
          simultrace=sw;
#ifdef SSE_BUILD
          SetWindowText(Win,"Master Debugger");
          SetWindowText(simultrace,"Slave Debugger");
#else
          SetWindowText(Win,"Master Boiler Room");
          SetWindowText(simultrace,"Slave Boiler Room");
#endif
          CheckMenuItem(menu1,1008,MF_BYCOMMAND|MF_CHECKED);
        }
      }
    }
    break;
  }
  case WM_INITMENUPOPUP:
    if((HMENU)wPar==mem_browser_menu)
    {
      int n=GetMenuItemCount(mem_browser_menu);
      int items=12;
#if USE_PASTI
      if(hPasti) 
        items=14;
#endif
      items++;
      items++;
      items++;
      items+=3;
      items+=4+2+1; // timers + HBI + VBI + ACIA
#if defined(SSE_DEBUGGER_BROWSER_6301)
      items++;
#endif
      items++;
      items++;
      for(int i=0;i<n-items;i++)
        DeleteMenu(mem_browser_menu,items,MF_BYPOSITION);
      bool NoBar=true;
      for(int i=0;i<MAX_MEMORY_BROWSERS;i++)
      {
        if(m_b[i])
        {
          if(NoBar && m_b[i]->disp_type!=DT_REGISTERS)
          {
            AppendMenu(mem_browser_menu,MF_STRING|MF_SEPARATOR,0,NULL);
            NoBar=0;
          }
          Str Pre;
          if(m_b[i]->disp_type==DT_INSTRUCTION||(m_b[i]->disp_type==DT_MEMORY 
            && IS_IOLIST_PSEUDO_ADDRESS(m_b[i]->ad)==0))
            Pre=HEXSl(m_b[i]->ad,6)+" - ";
          AppendMenu(mem_browser_menu,MF_STRING,950+i,Pre+GetWindowTextStr(m_b[i]->owner));
        }
      }
      return 0;
    }
    else if((HMENU)wPar==history_menu)
    {
      RemoveAllMenuItems(history_menu);
      int n=pc_history_idx,c=0;
      EasyStr Disassembly;
      do
      {
        n--;
        if(n<0) 
          n=HISTORY_SIZE-1;
        if(pc_history[n]==0xffffff71) 
          break;
        if(pc_history[n]==0x98764321)
          Disassembly="BLiT";
        else if(pc_history[n]==0x12346789)
          Disassembly="DMA";
        else if((pc_history[n]&0xFF0000FF)==0x99000001)
          Disassembly=Str("irq ")+Str((pc_history[n]>>16)&0xff)+"-"
          +Str((pc_history[n]>>8)&0xFF);
        else
          Disassembly=HEXSl(pc_history[n],6)+" - "+disa_d2(pc_history[n]);
        InsertMenu(history_menu,0,MF_BYPOSITION|MF_STRING,17000+n,Disassembly);
      } while(n!=pc_history_idx&&(c++)<HIST_MENU_SIZE);
      InsertMenu(history_menu,0,MF_BYPOSITION|MF_STRING|MF_SEPARATOR,99,"-");
      InsertMenu(history_menu,0,MF_BYPOSITION|MF_STRING,2200,"History List");
      return 0;
    }
    else if((HMENU)wPar==breakpoint_menu||(HMENU)wPar==monitor_menu)
    {
      breakpoint_menu_setup();
      return 0;
    }
    break;
  case WM_DRAWITEM:
  {
    DRAWITEMSTRUCT *pDIS=(DRAWITEMSTRUCT*)lPar;
    mem_browser *mb=(mem_browser*)GetWindowLongPtr(pDIS->hwndItem,GWLP_USERDATA);
    if(mb) 
      mb->draw(pDIS);
    break;
  }
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}


void disa_to_file(FILE*f,MEM_ADDRESS dstart,int dlen,bool as_source) {
  // if f is NULL, we copy to clipboard  instead
  MEM_ADDRESS dend=dstart+dlen,odpc;
  EasyStr dt,ot;
  int tp;
  char t[20];
  dpc=dstart;
  // text bytes for 1 code byte, need a lot, your typical line is like
  // cmpi.b #$39,$8252                                ; 008078: 0C39 0039 0000 8252 
  const int multiplier=40;
  BYTE *buffer=0,*buffer_ptr=0;
  if(!f) // clipboard, reserve memory
  {
    buffer=new BYTE[dlen*multiplier];
    if(!buffer)
      return;
    buffer_ptr=buffer;
  }
  while(dpc<dend)
  {
    odpc=(MEM_ADDRESS)dpc;
    if(as_source)
    {
      dt=disa_d2((MEM_ADDRESS)dpc);
      ot=EasyStr("\t")+dt;
      while(ot.Length()<50) ot+=" ";
      ot+="; ";
      ot+=HEXSl(odpc,6)+": ";
      while(odpc<dpc)
      {
        ot+=HEXSl(d2_dpeek(odpc),4);
        ot+=" ";
        odpc+=2;
      }
    }
    // we don't want disassembly for data dump to clipboard
    else if(!f)
    {
      ot="000000 : 0000 0000 0000 0000 0000 : ";
      itoa((MEM_ADDRESS)dpc,t,16);
      memcpy((ot.Text+6)-strlen(t),t,strlen(t)); // address
      dpc+=2*2; // wordsx2 (1-5)
      tp=13;
      while(odpc<dpc)
      {
        itoa(d2_dpeek(odpc),t,16);
        memcpy((ot.Text+tp)-strlen(t),t,strlen(t));
        tp+=5;
        odpc+=2;
      }
      while(tp<34)
      {
        memcpy(ot.Text+(tp-4),"    ",4);
        tp+=5;
      }
    }
    else
    {
      ot="000000 : 0000 0000 0000 0000 0000 : ";
      itoa((MEM_ADDRESS)dpc,t,16);
      memcpy((ot.Text+6)-strlen(t),t,strlen(t));
      dt=disa_d2((MEM_ADDRESS)dpc);
      tp=13;
      while(odpc<dpc)
      {
        itoa(d2_dpeek(odpc),t,16);
        memcpy((ot.Text+tp)-strlen(t),t,strlen(t));
        tp+=5;
        odpc+=2;
      }
      while(tp<34)
      {
        memcpy(ot.Text+(tp-4),"    ",4);
        tp+=5;
      }
      ot+=dt;
    }
    if(!f)
    {
      int c=sprintf((char*)buffer_ptr,"%s\r\n",ot.Text);
      //ASSERT(c!=-1);
      buffer_ptr+=c;
      //ASSERT(buffer_ptr-buffer<=dlen*multiplier);
    }
    else
      fprintf(f,"%s\r\n",ot.Text);
  }
  if(!f)
  {
    SetClipboardText((LPCTSTR)buffer);
    delete[] buffer;
  }
}


#define LOGSECTION LOGSECTION_INIT

void boiler_show_stack_display(int sel) {
  if(sel==-1)
    sel=(int)SendDlgItemMessage(DWin,209,CB_GETCURSEL,0,0);
  else
    SendDlgItemMessage(DWin,209,CB_SETCURSEL,sel,0);
  ShowWindow(DWin_timings_scroller,int((sel==1)?SW_SHOW:SW_HIDE));
  ShowWindow(m_b_stack.handle,int((sel==0)?SW_SHOW:SW_HIDE));
}


void DWin_init() {
  char ttt[200];
  int x=0,y=0;
  DBG_LOG("STARTUP: Setting up Reg Browser");
  {//set up reg browser
    int n=1;
    strcpy(reg_browser_entry_name[0],"pc");
    reg_browser_entry_pointer[0]=&pc;
    for(int m=0;m<16;m++)
    {
      ttt[0]="da"[int((m&8)?1:0)];
      ttt[1]=(char)('0'+(m&7));
      ttt[2]=0;
      strcpy(reg_browser_entry_name[n],ttt);
      reg_browser_entry_pointer[n]=(unsigned long*)&(Cpu.r[m]);
      n++;
    }
    strcpy(reg_browser_entry_name[n],"oth/sp");
    reg_browser_entry_pointer[n]=(unsigned long*)&(Cpu.r[15]);
    n++;
    reg_browser_entry_name[n][0]=0;
  }
  DBG_LOG("STARTUP: Reg Browser Done");
  shift_screen_menu=CreatePopupMenu();
  AppendMenu(shift_screen_menu,MF_STRING,1600,"0 bytes");
  AppendMenu(shift_screen_menu,MF_STRING,1601,"2 bytes");
  AppendMenu(shift_screen_menu,MF_STRING,1602,"4 bytes");
  AppendMenu(shift_screen_menu,MF_STRING,1603,"6 bytes");
  CheckMenuRadioItem(shift_screen_menu,1600,1603,1600
    +(debug_screen_shift/2),MF_BYCOMMAND);
  DBG_LOG("STARTUP: Creating Menu");
  menu=CreateMenu();
  menu1=CreatePopupMenu();
  AppendMenu(menu1,MF_STRING,1783,"Debugger &Reset");
  AppendMenu(menu1,MF_STRING,104,"&Disassemble a File");
  AppendMenu(menu1,MF_STRING,1001,"Load a &Picture");
  AppendMenu(menu1,MF_STRING,1002,"&Trace");
  AppendMenu(menu1,MF_STRING,1010,"&Run");
  AppendMenu(menu1,MF_STRING,1011,"Run to RT&E");
  AppendMenu(menu1,MF_STRING,1015,"Run to RTS");
  AppendMenu(menu1,MF_STRING|MF_SEPARATOR,0,"-");
  AppendMenu(menu1,MF_STRING,1004,"&Cold Reset");
  AppendMenu(menu1,MF_STRING|MF_SEPARATOR,0,"-");
  AppendMenu(menu1,MF_STRING,1008,"&Simul-trace");
  AppendMenu(menu1,MF_STRING,1780,"Turn Screen Red");
  AppendMenu(menu1,MF_STRING,1781,"Send MIDI message");
  AppendMenu(menu1,MF_STRING,1782,"Send Key Codes With Alt");
  AppendMenu(menu1,MF_STRING|MF_SEPARATOR,0,"-");
  AppendMenu(menu1,MF_STRING,1999,"&Quit");
  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT_PTR)menu1,"&Debug");
  DBG_LOG("STARTUP: Menu Done");
  DBG_LOG("STARTUP: Creating Breakpoint Menu");
  breakpoint_menu=CreatePopupMenu();
  breakpoint_irq_menu=CreatePopupMenu();
  monitor_menu=CreatePopupMenu();
  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT_PTR)breakpoint_menu,"B&reakpoints");
  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT_PTR)monitor_menu,"&Monitors");
  breakpoint_menu_setup();
  DBG_LOG("STARTUP: Breakpoint Menu Done");
  mem_browser_menu=CreatePopupMenu();
  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT_PTR)mem_browser_menu,"&Browsers");
  AppendMenu(mem_browser_menu,MF_STRING,900,"New &Memory Browser");
  AppendMenu(mem_browser_menu,MF_STRING,901,"New &Instruction Browser");
  AppendMenu(mem_browser_menu,MF_STRING,902,"New &Register Browser");
  AppendMenu(mem_browser_menu,MF_STRING,911,"New &Vectors Browser");
  AppendMenu(mem_browser_menu,MF_STRING,925,"New Bus Error Browser");
  AppendMenu(mem_browser_menu,MF_STRING,926,"New Illegal Browser");
  AppendMenu(mem_browser_menu,MF_STRING,927,"New Trace Browser");
  AppendMenu(mem_browser_menu,MF_STRING,923,"New HBI Browser");
  AppendMenu(mem_browser_menu,MF_STRING,924,"New VBI Browser");
  AppendMenu(mem_browser_menu,MF_STRING,918,"New Timer A Browser");
  AppendMenu(mem_browser_menu,MF_STRING,919,"New Timer B Browser");
  AppendMenu(mem_browser_menu,MF_STRING,920,"New Timer C Browser");
  AppendMenu(mem_browser_menu,MF_STRING,921,"New Timer D Browser");
  AppendMenu(mem_browser_menu,MF_STRING,922,"New ACIA Handler Browser");
  AppendMenu(mem_browser_menu,MF_STRING,912,"New &Shifter Browser");
  AppendMenu(mem_browser_menu,MF_STRING,913,"New &DMA Sound Browser");
  AppendMenu(mem_browser_menu,MF_STRING,915,"New Blitter Browser");
  AppendMenu(mem_browser_menu,MF_STRING,903,"New &PSG Browser");
  AppendMenu(mem_browser_menu,MF_STRING,904,"New M&FP Browser");
  AppendMenu(mem_browser_menu,MF_STRING,906,"New &Text Browser");
  AppendMenu(mem_browser_menu,MF_STRING,908,"New &FDC Browser");
  AppendMenu(mem_browser_menu,MF_STRING,916,"New ACIA Browser");
#if defined(SSE_DEBUGGER_BROWSER_6301)
  AppendMenu(mem_browser_menu,MF_STRING,909,"New I&KBD 6301 fake emu Browser");
  AppendMenu(mem_browser_menu,MF_STRING,914,"New IKBD 6301 true emu Browser");
#else
  AppendMenu(mem_browser_menu,MF_STRING,909,"New I&KBD Browser");
#endif
#if USE_PASTI
  if(hPasti)
  {
    AppendMenu(mem_browser_menu,MF_STRING|MF_SEPARATOR,0,NULL);
    AppendMenu(mem_browser_menu,MF_STRING,910,"Pa&sti Status");
  }
#endif
  AppendMenu(mem_browser_menu,MF_STRING|MF_SEPARATOR,0,NULL);
  AppendMenu(mem_browser_menu,MF_STRING|int(mem_browser::ex_style?0:MF_CHECKED),
    907,"Put Browsers On Taskbar");
  AppendMenu(mem_browser_menu,MF_STRING|MF_SEPARATOR,0,NULL);
  AppendMenu(mem_browser_menu,MF_STRING,905,"&Close All");
  DBG_LOG("STARTUP: mem_browser menu done");
  history_menu=CreatePopupMenu();
  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT_PTR)history_menu,"&History");
  logsection_menu=CreatePopupMenu();
  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT_PTR)logsection_menu,"&Log");
  AppendMenu(logsection_menu,MF_STRING,1005,"&View Logfile");
  AppendMenu(logsection_menu,MF_STRING,1014,"&Set Log Viewing Program");
  AppendMenu(logsection_menu,MF_SEPARATOR,0,NULL);
  AppendMenu(logsection_menu,MF_STRING,1006,"Add &Mark");
  AppendMenu(logsection_menu,MF_STRING,1007,"Wipe Logfile/Trace");
  AppendMenu(logsection_menu,MF_STRING|
    int(debug_wipe_log_on_reset?MF_CHECKED:MF_UNCHECKED),1013,"Wipe On Reboot");
  AppendMenu(logsection_menu,MF_SEPARATOR,0,NULL);
  logsection_menu2=CreatePopupMenu();
  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT_PTR)logsection_menu2,"Log&2");
  for(int n=1;logsections[n].Name[0]!='*';n++)
  {
    int i=logsections[n].Index;
    if(logsections[n].Name[0]=='-')
      AppendMenu(logsection_menu,MF_SEPARATOR,0,NULL);
    else
      AppendMenu((i<LOGSECTION_INIT)?logsection_menu:logsection_menu2,MF_STRING
        |int(logsection_enabled[i]?MF_CHECKED:0),300+i,logsections[n].Name);
  }
#ifdef SSE_DEBUGGER
  AppendMenu(logsection_menu2,MF_SEPARATOR,0,NULL);
  AppendMenu(logsection_menu2,MF_STRING,1009,"&Uncheck All");
  AppendMenu(logsection_menu2,MF_SEPARATOR,0,NULL);
  AppendMenu(logsection_menu2,MF_STRING,1012,"steem.log active");
  AppendMenu(logsection_menu2,MF_STRING,1533,"TRACE.txt active");
  AppendMenu(logsection_menu2,MF_STRING,1532,"log in TRACE.txt");
  AppendMenu(logsection_menu2,MF_STRING,1534,"TRACE in steem.log");
#else
  AppendMenu(logsection_menu,MF_SEPARATOR,0,NULL);
  AppendMenu(logsection_menu,MF_STRING,1009,"&Uncheck All");
  AppendMenu(logsection_menu2,MF_STRING,1012,"Suspend Logging");
#endif
  boiler_op_menu=CreatePopupMenu();
  AppendMenu(boiler_op_menu,MF_STRING,1501,"Notify on &all m68k exceptions 2-8");
  AppendMenu(boiler_op_menu,MF_STRING,1502,"Notify only on crash with &bombs");
  AppendMenu(boiler_op_menu,MF_STRING,1503,"&Don't notify on exceptions");
#if defined(SSE_DEBUGGER_EXCEPTION_NOT_TOS)
  AppendMenu(boiler_op_menu,MF_STRING,1504,"&Don't notify on TOS exceptions");
#endif
  AppendMenu(boiler_op_menu,MF_STRING|MF_SEPARATOR,0,NULL);
  AppendMenu(boiler_op_menu,MF_STRING|MF_POPUP,(UINT_PTR)shift_screen_menu,
    "Shift display");
  AppendMenu(boiler_op_menu,MF_STRING,1025,"Redraw on stop");
  AppendMenu(boiler_op_menu,MF_STRING,1026,"Redraw after trace");
  AppendMenu(boiler_op_menu,MF_STRING,1789,"Psy&chedelic mode");
  AppendMenu(boiler_op_menu,MF_STRING,1027,"Choose gun position display colour");
  AppendMenu(boiler_op_menu,MF_STRING|MF_SEPARATOR,0,NULL);
  AppendMenu(boiler_op_menu,MF_STRING,1510,"Stop on blitter");
  AppendMenu(boiler_op_menu,MF_STRING,1512,"Stop on switch to user mode");
  AppendMenu(boiler_op_menu,MF_STRING,1513,"Stop on next program run");
  AppendMenu(boiler_op_menu,MF_STRING,1531,"Stop on reset opcode");
  AppendMenu(boiler_op_menu,MF_STRING|MF_SEPARATOR,0,NULL);
  AppendMenu(boiler_op_menu,MF_STRING|MF_CHECKED,1514,"Show trace window");
  AppendMenu(boiler_op_menu,MF_STRING,1515,"Monospaced disassembly");
  AppendMenu(boiler_op_menu,MF_STRING,1516,"Uppercase disassembly");
//  AppendMenu(boiler_op_menu,MF_STRING|MF_SEPARATOR,0,"-");
  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT_PTR)boiler_op_menu,"&Options");
#if defined(SSE_DEBUGGER)
  sse_menu=CreatePopupMenu();
  AppendMenu(menu,MF_STRING|MF_POPUP,(UINT_PTR)sse_menu,"&SSE");
  AppendMenu(sse_menu,MF_STRING,1518,"Limit Trace file size");
  AppendMenu(sse_menu,MF_STRING,1530,"Dump Registers");
#if defined(SSE_DEBUGGER_DUMP_6301_RAM)
  AppendMenu(sse_menu,MF_STRING,1524,"Dump 6301 RAM");
#endif
#if defined(SSE_DEBUGGER_MONITOR_VALUE)
  //AppendMenu(sse_menu,MF_STRING|MF_SEPARATOR,0,NULL);
  AppendMenu(sse_menu,MF_STRING,1522,"Monitor: specific value");
#endif
#if defined(SSE_DEBUGGER_MONITOR_RANGE)
  AppendMenu(sse_menu,MF_STRING,1523,"Monitor: address range");
#endif
#if defined(SSE_DEBUGGER_STACK_CHOICE)
  AppendMenu(sse_menu,MF_STRING,1528,"Swap display USP/SSP");
#endif
#if defined(SSE_DEBUGGER_FAKE_IO)
  //AppendMenu(sse_menu,MF_STRING|MF_SEPARATOR,0,NULL);
  AppendMenu(sse_menu,MF_STRING,1527,STR_FAKE_IO_CONTROL);
#endif
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  AppendMenu(sse_menu,MF_STRING,917,"Pseudo Stack");
#endif
#if defined(SSE_DEBUGGER_FRAME_REPORT)
  AppendMenu(sse_menu,MF_STRING,1529,"Save frame report");
#endif
#if defined(SSE_DEBUGGER_ALERTS_IN_STATUS_BAR)
  AppendMenu(sse_menu,MF_STRING,1535,"Prompt on breakpoint");//v402
#endif
#endif
  DBG_LOG("STARTUP: calling iolist_init");
  iolist_init();
  DBG_LOG("STARTUP: iolist_init done");
#ifdef SSE_BUILD
  DBG_LOG("STARTUP: Creating Debugger Window");
#else
  DBG_LOG("STARTUP: Creating Boiler Room Window");
#endif
  WNDCLASS wnd;
  wnd.style=CS_DBLCLKS;
  wnd.lpfnWndProc=DWndProc;
  wnd.cbWndExtra=0;
  wnd.cbClsExtra=0;
  wnd.hInstance=Inst;
  wnd.hIcon=hGUIIcon[RC_ICO_BOMB];
  wnd.hCursor=PCArrow;
  wnd.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
  wnd.lpszMenuName=NULL;
  wnd.lpszClassName="Steem Debug Window";
  RegisterClass(&wnd);
  // since this is commented out, it is very well hidden:
//  HiddenParent=CreateWindow("Steem Debug Window","Steem Hidden Window",0,0,0,0,0,NULL,NULL,Inst,NULL);
#if defined(SSE_BUILD)
  DWin=CreateWindowEx(WS_EX_APPWINDOW,"Steem Debug Window","Debugger",
    WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SIZEBOX,
    60,60,640,400,ParentWin,menu,Inst,0);
  DBG_LOG("STARTUP: Debugger Window Done");
#else
  DWin=CreateWindowEx(WS_EX_APPWINDOW,"Steem Debug Window",EasyStr("The Boiler Room: Steem v")+stem_version_text
    ,WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX
    |WS_SIZEBOX
    ,60,60,640,400,ParentWin,menu,Inst,0);
  DBG_LOG("STARTUP: Boiler Room Window Done");
#endif
  {
    SIZE sz;
    HDC dc=GetDC(DWin);
    HFONT old_font=(HFONT)SelectObject(dc,fnt);
    GetTextExtentPoint32(dc,"CCCC ",5,&sz);
    how_big_is_0000=sz.cx;
    SelectObject(dc,old_font);
    ReleaseDC(DWin,dc);
  }
  DBG_LOG("STARTUP: Registering Mem Browser and Trace Window Classes");
  wnd.style=CS_DBLCLKS;
  wnd.lpfnWndProc=mem_browser_window_WndProc;
  wnd.cbWndExtra=0;
  wnd.cbClsExtra=0;
  wnd.hInstance=Inst;
  wnd.hIcon=hGUIIcon[RC_ICO_STCLOSE];
  wnd.hCursor=PCArrow;
  wnd.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
  wnd.lpszMenuName=NULL;
  wnd.lpszClassName="Steem Mem Browser Window";
  RegisterClass(&wnd);
  mem_browser::icons_bmp=LoadBitmap(Inst,"DEBUGICONS");
  mem_browser::icons_dc=CreateScreenCompatibleDC();
  SelectObject(mem_browser::icons_dc,mem_browser::icons_bmp);
  wnd.style=CS_DBLCLKS;
  wnd.lpfnWndProc=trace_window_WndProc;
  wnd.cbWndExtra=0;
  wnd.cbClsExtra=0;
  wnd.hInstance=Inst;
  wnd.hIcon=hGUIIcon[RC_ICO_STCLOSE];
  wnd.hCursor=PCArrow;
  wnd.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
  wnd.lpszMenuName=NULL;
  wnd.lpszClassName="Steem Trace Window";
  RegisterClass(&wnd);
  wnd.style=CS_CLASSDC|CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
  wnd.lpfnWndProc=mr_static_WndProc;
  wnd.cbWndExtra=0;
  wnd.cbClsExtra=0;
  wnd.hInstance=Inst;
  wnd.hIcon=NULL;
  wnd.hCursor=PCArrow;
  wnd.hbrBackground=NULL;
  wnd.lpszMenuName=NULL;
  wnd.lpszClassName="Steem Mr Static Control";
  RegisterClass(&wnd);
  DBG_LOG("STARTUP: Mem Browser and Trace Window Classes Registered");
  DBG_LOG("STARTUP: Creating Child Windows");
  new mr_static(/*label*/"PC",/*name*/"pc",/*x*/10,
    /*y*/1,/*owner*/DWin,/*id*/(HMENU)201,/*pointer*/&pc,
    /*bytes*/ 3,/*regflag*/ MST_REGISTER, /*editflag*/true,
    /*mem_browser to update*/&m_b_mem_disa);
  DWin_trace_button=CreateWindow("Button","Trace Into",WS_VISIBLE|
    WS_CHILD|BS_CHECKBOX|BS_PUSHLIKE,330,1,140,25,DWin,(HMENU)1002,Inst,NULL);
  DWin_trace_over_button=CreateWindow("Button","Step Over",WS_VISIBLE|
    WS_CHILD|BS_CHECKBOX|BS_PUSHLIKE,330,1,140,25,DWin,(HMENU)1003,Inst,NULL);
  DWin_run_button=CreateWindow("Button","Run",WS_VISIBLE|WS_CHILD|BS_CHECKBOX
    |BS_PUSHLIKE|WS_CLIPSIBLINGS,490,1,140,50,DWin,(HMENU)1010,Inst,NULL);
/* - see below
  HWND Win=CreateWindowEx(512,"Combobox","",WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST
    |CBS_HASSTRINGS,330+50,26,100-20-30,200,DWin,(HMENU)1020,Inst,NULL);
  CBAddString(Win,"Run to next VBL",MAKELONG(DRU_VBL,0));
  CBAddString(Win,"Run to scanline n",MAKELONG(DRU_SCANLINE,0));
  CBAddString(Win,"Run for n cycles",MAKELONG(DRU_CYCLE,0));
  CBAddString(Win,"Run until instruction changes",MAKELONG(DRU_INSTCHANGE,0));
*/
  CreateWindowEx(512,"Edit","",WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,
    490,26,140,25,DWin,(HMENU)1021,Inst,NULL);
  CreateWindow("Button","Go",WS_VISIBLE|WS_CHILD|BS_CHECKBOX|BS_PUSHLIKE
    |WS_CLIPSIBLINGS,490,26,140,25,DWin,(HMENU)1022,Inst,NULL);
  // show SR in hex as well as bits
  new mr_static("SR","SDP",10,30,DWin,(HMENU)203,
    (MEM_ADDRESS*)&SR,2,MST_REGISTER,false,NULL); //no need to edit this
  sr_display=CreateWindowEx(512,"Static","sr display",WS_BORDER|WS_VISIBLE
    |WS_CHILDWINDOW|SS_NOTIFY,30+40,30,200,20,DWin,(HMENU)230,Inst,NULL);
  SetWindowLongPtr(sr_display,GWLP_USERDATA,(LONG_PTR)&SR);
  Old_sr_display_WndProc=(WNDPROC)SetWindowLongPtr(sr_display,GWLP_WNDPROC,
    (LONG_PTR)sr_display_WndProc);
  DBG_LOG("STARTUP: Subclassed sr display");
  for(int n=0;n<16;n++)
  {
    strcpy(ttt,reg_name(n));
    strcat(ttt," ");
    x=10+(n&7)*80;
    y=60+(n&8)*3;
    new mr_static(/*label*/ttt,/*name*/ttt,/*x*/x,/*y*/y,
      /*owner*/DWin,/*id*/(HMENU)(276+n),/*pointer*/(MEM_ADDRESS*)&(Cpu.r[n]),
      /*bytes*/ 4,/*regflag*/ MST_REGISTER, /*editflag*/true,
      /*mem_browser to update*/NULL);
  }
  new mr_static("SSP","SSP",x+80,y,
    DWin,(HMENU)203,(MEM_ADDRESS*)&debug_SSP,4,MST_REGISTER,
    true,NULL);
  new mr_static("USP","USP",x+80,y-24,
    DWin,(HMENU)203,(MEM_ADDRESS*)&debug_USP,4,MST_REGISTER,
    true,NULL);
  m_b_mem_disa.handle=CreateWindowEx(512,WC_LISTVIEW,"",
    LVS_REPORT|LVS_SHAREIMAGELISTS|LVS_NOSORTHEADER|LVS_OWNERDRAWFIXED|
    WS_VISIBLE|WS_CHILDWINDOW|WS_CLIPSIBLINGS,
    10,120,320,190,DWin,(HMENU)200,Inst,NULL);
  SetWindowLongPtr(m_b_mem_disa.handle,GWLP_USERDATA,(LONG_PTR)&m_b_mem_disa);
  Old_mem_browser_WndProc=(WNDPROC)SetWindowLongPtr(m_b_mem_disa.handle,GWLP_WNDPROC,
    (LONG_PTR)mem_browser_WndProc);
  DBG_LOG("STARTUP: SubClassed Mem_Browser Listview");
  //  m_b_mem_disa.active=true;
  m_b_mem_disa.owner=DWin;
  m_b_mem_disa.disp_type=DT_INSTRUCTION;
  m_b_mem_disa.ad=(pc&0x00FFFFFF);
  m_b_mem_disa.mode=MB_MODE_PC;
  m_b_mem_disa.editbox=NULL;
  m_b_mem_disa.editflag=true;
  DBG_LOG("STARTUP: Initing Mem_Browser Listview");
  m_b_mem_disa.init();
  DBG_LOG("STARTUP: Mem_Browser Initialised");
  // User controlled area, stack or timings
  /* -see below
  DWin_right_display_combo=CreateWindowEx(512,"Combobox","",WS_CHILD|WS_VISIBLE
    |CBS_HASSTRINGS|CBS_DROPDOWNLIST,345,120,320,60,DWin,(HMENU)209,Inst,NULL);
  SendMessage(DWin_right_display_combo,CB_ADDSTRING,0,LPARAM("Stack Display"));
  SendMessage(DWin_right_display_combo,CB_ADDSTRING,0,LPARAM("Timings Display"));
  */
  // Stack
  m_b_stack.handle=CreateWindowEx(512,WC_LISTVIEW,"",
    LVS_REPORT|LVS_SHAREIMAGELISTS|LVS_NOSORTHEADER|LVS_OWNERDRAWFIXED|
    WS_CHILD|WS_CLIPSIBLINGS,
    345,150,320,190,DWin,(HMENU)210,Inst,NULL);
  SetWindowLongPtr(m_b_stack.handle,GWLP_USERDATA,(LONG_PTR)&m_b_stack);
  SetWindowLongPtr(m_b_stack.handle,GWLP_WNDPROC,(LONG_PTR)mem_browser_WndProc);
  DBG_LOG("STARTUP: SubClassed M_B_Stack Listview");
  m_b_stack.owner=DWin;
  m_b_stack.disp_type=DT_MEMORY;
  m_b_stack.ad=(pc&0x00FFFFFF);
  m_b_stack.mode=MB_MODE_STACK;
  m_b_stack.wpl=1;
  m_b_stack.editbox=NULL;
  m_b_stack.editflag=true;
  DBG_LOG("STARTUP: Initing M_B_Stack Listview");
  m_b_stack.init();
  { // Timings
    DWin_timings_scroller.CreateEx(512,WS_CHILD,0,0,1,1,DWin,(HMENU)220,Inst);
    HWND Par=DWin_timings_scroller.GetControlPage();
    RECT rc;
    y=5;
    mr_static *ms;
    ms=new mr_static("Frame ","",5,y,Par,
      NULL,(MEM_ADDRESS*)&FRAME,3,MST_DECIMAL,true,NULL);
    GetWindowRectRelativeToParent(ms->handle,&rc);
    ms=new mr_static("Tricks ","",rc.right+5,y,Par,
      NULL,(MEM_ADDRESS*)&Debug.ShifterTricks,3,MST_REGISTER,0,NULL);
    GetWindowRectRelativeToParent(ms->handle,&rc);
    ms=new mr_static("#HBI ","",rc.right+5,y,Par,
      NULL,(MEM_ADDRESS*)&Debug.nHbis,2,MST_DECIMAL,0,NULL);
    // correct but hard to interpret
    GetWindowRectRelativeToParent(ms->handle,&rc);
    ms=new mr_static("IRQs ","",rc.right+5,y,Par,
      NULL,(MEM_ADDRESS*)&debug_frame_interrupts,3,MST_REGISTER,0,NULL);
    y+=30;
    ms=new mr_static("Cycle counters   CPU ","",5,y,Par,
      NULL,(MEM_ADDRESS*)&debug_ACT,8,MST_DECIMAL,0,NULL);
    GetWindowRectRelativeToParent(ms->handle,&rc);
    ms=new mr_static("Frame","",rc.right+5,y,Par,
      NULL,(MEM_ADDRESS*)&debug_cycles_since_VBL,3,MST_DECIMAL,0,NULL);
    GetWindowRectRelativeToParent(ms->handle,&rc);
    ms=new mr_static("Line ","",rc.right+5,y,Par,
        NULL,(MEM_ADDRESS*)&debug_cycles_since_HBL,2,MST_DECIMAL,0,NULL);
    y+=30;
    ms=new mr_static("VBASE","screen address",5,y,Par,
      (HMENU)294,(MEM_ADDRESS*)&vbase,3,MST_REGISTER,true,NULL);
    GetWindowRectRelativeToParent(ms->handle,&rc);
    ms=new mr_static("VCOUNT ","",rc.right+5,y,Par,
      NULL,(MEM_ADDRESS*)&debug_VAP,3,MST_REGISTER,0,NULL);
    GetWindowRectRelativeToParent(ms->handle,&rc);
    ms=new mr_static("scanline ","",rc.right+5,y,Par,
      NULL,(MEM_ADDRESS*)&scan_y,2,MST_DECIMAL,0,NULL);
    y+=30;
    ms=new mr_static("Shift ","",5,y,Par,
      NULL,(MEM_ADDRESS*)&Shifter.m_ShiftMode,1,MST_REGISTER,0,NULL);
    GetWindowRectRelativeToParent(ms->handle,&rc);
    ms=new mr_static("Sync ","",rc.right+5,y,Par,
      NULL,(MEM_ADDRESS*)&Glue.m_SyncMode,1,MST_REGISTER,0,NULL);
    // Shifter tricks of the line!
    GetWindowRectRelativeToParent(ms->handle,&rc);
    ms=new mr_static("Tricks ","",rc.right+5,y,Par,
      NULL,(MEM_ADDRESS*)&Glue.CurrentScanline.Tricks,3,MST_REGISTER,0,NULL);
    // and since we still have room:
    GetWindowRectRelativeToParent(ms->handle,&rc);
    new mr_static("bytes ","",rc.right+5,y,Par,
      NULL,(MEM_ADDRESS*)&Glue.CurrentScanline.Bytes,2,MST_DECIMAL,0,NULL);
    y+=30;
    x=5;
    new mr_static("MFP ","",x,y,Par,
      NULL,(MEM_ADDRESS*)&Mfp.NextIrq,1,MST_REGISTER,0,NULL);
    x+=60;
    new mr_static("VBi ","",x,y,Par,
      NULL,(MEM_ADDRESS*)&Glue.vbl_pending,1,MST_REGISTER,0,NULL);
    x+=60;
    new mr_static("HBi ","",x,y,Par,
      NULL,(MEM_ADDRESS*)&Glue.hbl_pending,1,MST_REGISTER,0,NULL);
    x+=60;
    new mr_static("cc ","",x,y,Par,
      NULL,(MEM_ADDRESS*)&cpu_cycles,4,MST_DECIMAL,0,NULL);
    y+=30;
    x=5;
    for(int t=0;t<4;t++)
    {
      ms=new mr_static(Str("Timer ")+char('A'+t)+" ","",x,y,Par,
        NULL,(MEM_ADDRESS*)&debug_time_to_timer_timeout[t],4,MST_DECIMAL,0,NULL);
#if defined(SSE_DEBUGGER_TIMERS_ACTIVE)
      //ASSERT(ms); // record handles to edit properties on update
      Debug.boiler_timer_hwnd[t]=ms->hLABEL;
#endif
      y+=30;
      if(t==1)
      {
        y-=60;
        GetWindowRectRelativeToParent(ms->handle,&rc);
        x=rc.right+5;
      }
    }
#if defined(SSE_DEBUGGER_DECRYPT_TIMERS)
    for(int t=0;t<4;t++)
    {
      ms=new mr_static(Str("T")+char('A'+t)+" prescaler ","",5,y,Par,
        NULL,(MEM_ADDRESS*)&debug_time_to_timer_prescale[t],2,MST_DECIMAL,0,NULL);
      GetWindowRectRelativeToParent(ms->handle,&rc);
      ms=new mr_static(Str("DR "),"",rc.right+5,y,Par,
        NULL,(MEM_ADDRESS*)&debug_time_to_timer_data[t],2,MST_DECIMAL,0,NULL);
      GetWindowRectRelativeToParent(ms->handle,&rc);
      ms=new mr_static(Str("counter "),"",rc.right+5,y,Par,
        NULL,(MEM_ADDRESS*)&debug_time_to_timer_count[t],2,MST_DECIMAL,0,NULL);
      GetWindowRectRelativeToParent(ms->handle,&rc);
      ms=new mr_static(Str("prescale "),"",rc.right+5,y,Par,
        NULL,(MEM_ADDRESS*)&debug_time_to_timer_ticks[t],2,MST_DECIMAL,0,NULL);
      y+=30;
    }
#endif
    DWin_timings_scroller.AutoSize();
  }
  //
  // User controlled area, stack or timings - create now to avoid GUI glitch
  DWin_right_display_combo=CreateWindowEx(512,"Combobox","",WS_CHILD|WS_VISIBLE
    |CBS_HASSTRINGS|CBS_DROPDOWNLIST,345,120,320,60,DWin,(HMENU)209,Inst,NULL);
  SendMessage(DWin_right_display_combo,CB_ADDSTRING,0,LPARAM("Stack Display"));
  SendMessage(DWin_right_display_combo,CB_ADDSTRING,0,LPARAM("Timings Display"));
  // same reason for moving this here
  HWND Win=CreateWindowEx(512,"Combobox","",WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST
    |CBS_HASSTRINGS,330+50,26,100-20-30,200,DWin,(HMENU)1020,Inst,NULL);
  CBAddString(Win,"Run to next VBL",MAKELONG(DRU_VBL,0));
  CBAddString(Win,"Run to scanline n",MAKELONG(DRU_SCANLINE,0));
  CBAddString(Win,"Run for n cycles",MAKELONG(DRU_CYCLE,0));
  CBAddString(Win,"Run until instruction changes",MAKELONG(DRU_INSTCHANGE,0));
  //
  boiler_show_stack_display(0);
  trace_window_init();
  DBG_LOG("STARTUP: Creating DWin_edit");
  DWin_edit=CreateWindowEx(512,"Edit","hi",
    WS_BORDER|WS_CHILDWINDOW|WS_CLIPSIBLINGS|ES_AUTOHSCROLL,
    220,10,60,25,DWin,(HMENU)255,Inst,NULL);
  DWin_edit_subject=NULL;
  DWin_edit_subject_type=-1;
  Old_edit_WndProc=(WNDPROC)SetWindowLongPtr(DWin_edit,GWLP_WNDPROC,
    (LONG_PTR)DWin_edit_WndProc);
  BringWindowToTop(DWin_edit);
  DBG_LOG("STARTUP: DWin_edit Created");
  DBG_LOG("STARTUP: Child Windows Created");
  SetWindowAndChildrensFont(DWin,fnt);
  DBG_LOG("STARTUP: Creating insp_menu");
  insp_menu=CreatePopupMenu();
  DBG_LOG("STARTUP: insp_menu Created");
  debug_plugin_load();
  if(debug_plugins.NumItems)
  {
    HMENU plugin_menu=CreatePopupMenu();
    for(int i=0;i<debug_plugins.NumItems;i++)
    {
      int added=0;
      char *p=(char*)debug_plugins[i].Menu;
      while(p[0])
      {
        AppendMenu(plugin_menu,MF_STRING,40000+i*100+added,p);
        added++;
        p+=strlen(p)+1;
      }
      if(added && i<debug_plugins.NumItems-1) 
        AppendMenu(plugin_menu,MF_SEPARATOR,0,NULL);
    }
    AppendMenu(menu,MF_STRING|MF_POPUP,(UINT_PTR)plugin_menu,"&Plugins");
  }
}


void logfile_wipe() {
  if(logfile)
  {
    fclose(logfile);
    logfile=fopen(LogFileName,"wb");
  }
  if(Debug.trace_file_pointer)
  {
    fclose(Debug.trace_file_pointer);
    EasyStr trace_file=RunDir+SLASH+SSE_TRACE_FILE_NAME; 
    Debug.trace_file_pointer=freopen(trace_file, "w", stdout );
    Debug.TraceGeneralInfos(TDebug::INIT);
  }
}


void stop_new_program_exec() {
  stop_on_user_change=0;
  stop_on_next_program_run=1;
#if defined(SSE_DEBUGGER_FAKE_IO)
  if(!(DEBUGGER_CONTROL_MASK2&DEBUGGER_CONTROL_NEXT_PRG_RUN))
#endif
  {
    stop_on_next_program_run=0;
    CheckMenuItem(boiler_op_menu,1513,MF_BYCOMMAND|MF_UNCHECKED);
  }
#ifdef SSE_DEBUG
#undef LOGSECTION
#define LOGSECTION LOGSECTION_STEMDOS
  MEM_ADDRESS bp=d2_lpeek(areg[7]+4);
  TRACE_LOG("Basepage %X parent %X DTA %X ENV %X CMD %X TEXT %X (%d) DATA %X (%d) BSS %X (%d)\n",
    bp,d2_lpeek(bp+0x24),d2_lpeek(bp+0x20),d2_lpeek(bp+0x2C),
    d2_lpeek(bp+0x80),d2_lpeek(bp+8),d2_lpeek(bp+0xc),d2_lpeek(bp+0x10),
    d2_lpeek(bp+0x14),d2_lpeek(bp+0x18),d2_lpeek(bp+0x1c));
#undef LOGSECTION
#endif
  SET_WHY_STOP(HEXSl(pc,6)+": New program executed")
}


void debug_vbl() {
  if(debug_send_alt_keys)
  {
    debug_send_alt_keys_vbl_countdown--;
    if(debug_send_alt_keys_vbl_countdown==0)
    {
      for(;;)
      {
        bool SpecialKey=0;
        if(debug_send_alt_keys>=0x3b&&debug_send_alt_keys<=0x44) 
          SpecialKey=true;
        if(debug_send_alt_keys>=0x61&&debug_send_alt_keys<=0x72) 
          SpecialKey=true;
        switch(debug_send_alt_keys) {
        case 0xe:case 0xf:case 0x1d:case 0x2a:case 0x36:case 0x3a:
        case 0x39:case 0x38:case 0x1c:case 0x53:case 0x52:case 0x48:
        case 0x47:case 0x4b:case 0x50:case 0x4d:case 0x4a:case 0x4e:
          SpecialKey=true;
          break;
        }
        if(SpecialKey==0)
        {
          Str HexI=HEXSl(debug_send_alt_keys,2).LowerCase();
          for(int n=0;n<2;n++)
          {
            BYTE VKCode=0;
            if(HexI[n]>='0' && HexI[n]<='9') 
              VKCode=BYTE(VK_NUMPAD0+(HexI[n]-'0'));
            if(HexI[n]>='a' && HexI[n]<='f') 
              VKCode=BYTE('A'+(HexI[n]-'a'));
            keyboard_buffer_write(key_table[VKCode]);
            keyboard_buffer_write(BYTE(key_table[VKCode]|BIT_7));
          }
          keyboard_buffer_write(key_table[VK_SPACE]);
          keyboard_buffer_write(BYTE(key_table[VK_SPACE]|BIT_7));
          keyboard_buffer_write(key_table[VK_MENU]);
          keyboard_buffer_write(debug_send_alt_keys);
          keyboard_buffer_write(BYTE(debug_send_alt_keys|BIT_7));
          keyboard_buffer_write(BYTE(key_table[VK_MENU]|BIT_7));
          keyboard_buffer_write(key_table[VK_RETURN]);
          keyboard_buffer_write(BYTE(key_table[VK_RETURN]|BIT_7));
        }
        if((++debug_send_alt_keys)>0x75)
        {
          debug_send_alt_keys=0;
          break;
        }
        debug_send_alt_keys_vbl_countdown=5;
        if(SpecialKey==0) 
          break;
      }
    }
  }
}


void debug_plugin_load() { // ?
  debug_plugin_free();
  DirSearch ds;
  EasyStr Fol=RunDir+SLASH "plugins" SLASH;
  if(ds.Find(Fol+"*.dll"))
  {
    do
    {
      TDebugPluginfo dbi;
      dbi.hDll=LoadLibrary(Fol+ds.Name);
      if(dbi.hDll)
      {
        dbi.Init=(DEBUGPLUGIN_INITPROC*)GetProcAddress(dbi.hDll,"Init");
        dbi.Activate=(DEBUGPLUGIN_ACTIVATEPROC*)GetProcAddress(dbi.hDll,"Activate");
        dbi.Close=(DEBUGPLUGIN_CLOSEPROC*)GetProcAddress(dbi.hDll,"Close");
        if(dbi.Init!=NULL && dbi.Activate!=NULL && dbi.Close!=NULL)
        {
          ZeroMemory(dbi.Menu,sizeof(dbi.Menu));
          dbi.Init(debug_plugin_routines,dbi.Menu);
          debug_plugins.Add(dbi);
        }
      }
      else
        DisplayLastError();
    } while(ds.Next());
    ds.Close();
  }
}


void debug_plugin_free() {
  for(int i=0;i<debug_plugins.NumItems;i++)
  {
    debug_plugins[i].Close();
    FreeLibrary(debug_plugins[i].hDll);
  }
  debug_plugins.DeleteAll();
}


Str debug_parse_disa_for_display(Str s) {
  if(debug_uppercase_disa)
    strupr(s);
  if(debug_monospace_disa==0)
    return s;
  Str part[2];
  char *spc;
  part[0]=s;
  for(int i=0;i<2-1;i++) // reason for the break at the end, i should never be 1!
  {
    spc=part[i].Text;
    for(;;)
    {
      spc=strchr(spc,' ');
      if(spc==NULL)
        break;
      if(*(spc+1)!='.') // can have bxx .s or bxx.s
      {
        *spc=NULL;
        break;
      }
      spc++;
    }
    if(spc==NULL)
      break;
    //if(spc!=NULL)
    part[i+1]=spc+1; // part[i+1] if i==1 ??
    //break; // warning C4702: unreachable code -> see above
  }
  s=part[0].RPad(MAX(part[0].Length()+1,8),' ');
  if(part[1].NotEmpty())
    s+=part[1];
  return s;
}


void debug_load_file_to_address(HWND par,MEM_ADDRESS ad) {
  EasyStr fn;
  fn=FileSelect(par,Str("Load File To $")+HEXSl(ad,6),RunDir,
    "All Files\0*.*\0\0",1,true);
  if(fn.Empty()) 
    return;
  FILE *f=fopen(fn,"rb");
  if(f==NULL) return;
  STfile_read_to_ST_memory(f,ad,GetFileLength(f));
  fclose(f);
  update_register_display(true);
}

#endif//#ifdef DEBUG_BUILD
