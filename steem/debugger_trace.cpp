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
FILE: debugger_trace.cpp
CONDITION: DEBUG_BUILD must be defined
DESCRIPTION: Functions to handle tracing instructions in the debug version
of Steem and displaying information on what happened.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#ifdef DEBUG_BUILD
#include <mr_static.h>
#include <mymisc.h>
#include <debugger.h>
#include <gui.h>
#include <computer.h>
#include <cpu.h>
#include <debugger_trace.h>
#include <draw.h>


#define EXTRA_HEIGHT 20 // more space for instruction, it was cut off by scroll bar

Ttrace_display_entry t_d_e[MAX_TRACE_DISPLAY_ENTRIES];
unsigned short trace_sr_before,trace_sr_after;
MEM_ADDRESS trace_pc;
int trace_entries=0;
mem_browser m_b_trace;
HWND trace_window_handle;
HWND trace_repeat_trace_button;
HWND trace_hLABEL[MAX_TRACE_DISPLAY_ENTRIES];
HWND trace_sr_before_display,trace_sr_after_display;
ScrollControlWin trace_scroller;
bool trace_show_window=true;

const char*bombs_name[12]={"SSP after reset","PC after reset","bus error",
  "address error","illegal instruction","division by zero","CHK instruction",
  "TRAPV instruction","Privilege violation","Trace","Line-A","Line-F"};

const char*exception_action_name[4]={"read from","write to","fetch from",
"instruction execution"};


void trace() {
  SendMessage(trace_window_handle,WM_SETTEXT,0,(LPARAM)"Trace");

#if defined(SSE_DEBUGGER_ALERTS_IN_STATUS_BAR)
  BoilerStatusBarMsg("Trace");
#elif defined(SSE_DEBUGGER_STATUS_BAR)
  SendMessage(hStatusBar,SB_SETTEXT,0,(LPARAM)"Trace");
  //REFRESH_STATUS_BAR;
#endif  
  trace_init();
//  ASSERT(trace_pc!=0x722);
  d2_trace=true;
  disa_d2((pc&0x00FFFFFF));
  int time_text_entry=trace_entries,cpu_cycles_this_instruction;
  trace_add_text(EasyStr("Instruction time (not rounded): "));
  d2_trace=false;
  runstate=RUNSTATE_STOPPED;
  runstate_why_stop="";
  debug_in_trace=true;
  COUNTER_VAR old_cpu_time=ABSOLUTE_CPU_TIME;
  //execute
//  try{
  TRY_M68K_EXCEPTION
    pc_history_y[pc_history_idx]=scan_y;
    pc_history_c[pc_history_idx]=(short)LINECYCLES;
    pc_history[pc_history_idx++]=(pc&0x00FFFFFF);
    if(pc_history_idx>=HISTORY_SIZE)
      pc_history_idx=0;
    stem_runmode=STEM_MODE_CPU;
    draw_begin();
    debug_update_drawing_position();
    m68kProcess();
    cpu_cycles_this_instruction=(int)(ABSOLUTE_CPU_TIME-old_cpu_time);
    debug_check_for_events();
    draw_end();
    stem_runmode=STEM_MODE_INSPECT;
    trace_get_after(); //v4
    update_display_after_trace();
//  }catch (m68k_exception &e){
  CATCH_M68K_EXCEPTION
    m68k_exception &e=ExceptionObject;
    stem_runmode=STEM_MODE_INSPECT;
    if(e.bombs>7)
    {
      e.crash();
      cpu_cycles_this_instruction=(int)(ABSOLUTE_CPU_TIME-old_cpu_time);
      trace_exception_display(&e);
    }
    else
    {
#if defined(SSE_DEBUGGER_ALERTS_IN_STATUS_BAR) // v402
      char Mess[40];
      sprintf(Mess,"Exception %d",e.bombs);
      BoilerStatusBarMsg(Mess);
      if(!Debug.PromptOnBreakpoint)
      {
        e.crash();
        trace_exception_display(&e);
      }
      else // this dialig box has always been confusing anyway
#endif
      switch(Alert
  ("Exception - do you want to crash(=ABORT)\nor re-execute?(=RETRY)\nor skip?(=IGNORE)",
        (EasyStr("Exception ")+itoa(e.bombs,d2_t_buf,10)).c_str(),
        MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION))
      {
      case IDABORT:
        e.crash();
        trace_exception_display(&e);
        break;
      case IDRETRY:
        SET_PC(old_pc);
        break;
      case IDIGNORE:
        SET_PC((MEM_ADDRESS)dpc);
        break;
      }
      cpu_cycles_this_instruction=(int)(ABSOLUTE_CPU_TIME-old_cpu_time);
    }
  END_M68K_EXCEPTION
  debug_in_trace=0;
  strcpy(t_d_e[time_text_entry].name,EasyStr("Instruction time (not rounded): ")
    +(cpu_cycles_this_instruction)+" cycles");
  update_register_display(true);
  trace_display();
  if(runstate_why_stop.NotEmpty())
  {
#if defined(SSE_DEBUGGER_ALERTS_IN_STATUS_BAR) // v402
    BoilerStatusBarMsg(runstate_why_stop.Text);
    if(Debug.PromptOnBreakpoint)
#endif
    Alert(runstate_why_stop,"Interrupt",0);
    runstate_why_stop="";
  }
}


void trace_again() {
  //reset all values
  SET_PC(trace_pc);
  SR=trace_sr_before;
  for(int n=0;n<trace_entries;n++)
  {
    if(t_d_e[n].regflag)
    {
      switch(t_d_e[n].bytes) {
      case 1:*(BYTE*)(t_d_e[n].ptr)=LOBYTE(t_d_e[n].val[0]);break;
      case 2:*(WORD*)(t_d_e[n].ptr)=LOWORD(t_d_e[n].val[0]);break;
      case 3:case 4:*(LONG*)(t_d_e[n].ptr)=t_d_e[n].val[0];break;
      }
    }
    else
    {
      switch(t_d_e[n].bytes)  {
      case 1:d2_poke(t_d_e[n].ad,LOBYTE(t_d_e[n].val[0]));break;
      case 2:d2_dpoke(t_d_e[n].ad,LOWORD(t_d_e[n].val[0]));break;
      case 3:case 4:d2_lpoke(t_d_e[n].ad,t_d_e[n].val[0]);break;
      }
    }
  }
  trace();
}


void trace_display_clear() {
  SendMessage(trace_scroller.GetControlPage(),WM_SETREDRAW,0,0);
  for(int n=0;n<MAX_TRACE_DISPLAY_ENTRIES;n++)
  {
    if(trace_hLABEL[n])
    {
      if(IsWindow(trace_hLABEL[n]))
      {
        DestroyWindow(trace_hLABEL[n]);
        trace_hLABEL[n]=NULL;
      }
    }
  }
  mr_static_delete_children_of(trace_scroller.GetControlPage());
  SendMessage(trace_scroller.GetControlPage(),WM_SETREDRAW,1,0);
  InvalidateRect(trace_scroller,NULL,0);
}


void trace_init() {
  trace_display_clear();
  trace_entries=0;
  trace_sr_before=SR;
  trace_pc=(pc&0x00FFFFFF);
}


void trace_add_movem_block(char*name,int aregn,short when,int bytes,
                            MEM_ADDRESS ad,int count) {
  char tb[30];
  if(d2_trace && count)
  {
    for(int n=0;n<count;n++)
    {
      if(aregn!=-1)
      {
        strcpy(tb,STRS(ad+n*bytes-areg[aregn]));strcat(tb,"(");
        strcat(tb,reg_name(aregn+8));strcat(tb,")");
      }
      else 
        tb[0]=0;
      trace_add_entry(name,tb,when,false,bytes,
        (MEM_ADDRESS*)(INT_PTR)(ad+n*bytes));
    }
  }
}


void trace_exception_display(m68k_exception*exc) {
  d2_trace=true;
  trace_add_text((char*)bombs_name[exc->bombs]);
  if(exc->action==EA_INST)
  {
    trace_add_text((char*)(char*)exception_action_name[exc->action]);
  }
  else
  {
    trace_add_entry("during ",(char*)exception_action_name[exc->action],0,
      false,4,(MEM_ADDRESS*)(INT_PTR)exc->uaddress.d32);
  }
  trace_add_entry("exception vector ","",TDE_BEFORE,false,4,
    (MEM_ADDRESS*)(INT_PTR)(exc->bombs*4));
  d2_trace=false;
}


void trace_add_text(char*tt) {
  if(d2_trace)
  {
    int n=trace_entries;
    strcpy(t_d_e[n].name,tt);
    t_d_e[n].when=TDE_TEXT_ONLY;
    if(trace_entries<MAX_TRACE_DISPLAY_ENTRIES)
      trace_entries++;
  }
}


void trace_add_entry(char*name1,char*name2,short when,bool regflag,
                        int bytes,MEM_ADDRESS *ad) {
  if(d2_trace)
  {
    int n=trace_entries;
    if(strlen(name1)+strlen(name2)>100)
      strcpy(t_d_e[n].name,"name too long");
    else 
    {
      strcpy(t_d_e[n].name,name1);
      strcat(t_d_e[n].name,name2);
    }
    t_d_e[n].when=when;
    t_d_e[n].bytes=bytes;
    t_d_e[n].regflag=regflag;
    if(t_d_e[n].regflag)
    {
      t_d_e[n].ptr=(unsigned long*)ad;
      if((t_d_e[n].when)&TDE_BEFORE)
        t_d_e[n].val[0]=*(t_d_e[n].ptr);
    }
    else
    {
      t_d_e[n].ad=((MEM_ADDRESS)(INT_PTR)ad)&0xffffff;
      if((t_d_e[n].when)&TDE_BEFORE)
      {
        switch(t_d_e[n].bytes) {
        case 1:
          t_d_e[n].val[0]=d2_peek(t_d_e[n].ad);
          break;
        case 2:
          t_d_e[n].val[0]=d2_dpeek(t_d_e[n].ad);
          break;
        case 4:
          t_d_e[n].val[0]=d2_lpeek(t_d_e[n].ad);
          break;
        }
      }
    }
    if(trace_entries<MAX_TRACE_DISPLAY_ENTRIES)
      trace_entries++;
  }
/*  We give the 'before' values as seen here. It's a little
    confusing, not always relevant but it can help.
*/
  else if((TRACE_ENABLED(LOGSECTION_CPU))
    &&(TRACE_MASK4 & TRACE_CONTROL_CPU_VALUES)
    &&(runstate==RUNSTATE_RUNNING))
  {
    unsigned long value=0;
    if(regflag)
      value=*(unsigned long *)ad; //32bit only...
    else
    {
      MEM_ADDRESS my_ad=((MEM_ADDRESS)(INT_PTR)ad)&0xffffff;
      switch(bytes) {
      case 1:value=d2_peek(my_ad);
        break;
      case 2:value=d2_dpeek(my_ad);
        break;
      case 4:value=d2_lpeek(my_ad);
        break;
      }
    }
    if(value)
    {
      char value_str[20];
      sprintf(value_str," ($%x)",value);
      d2_pc_rel_ex+=value_str;
    }
  }
}


void trace_get_after() { // wasn't used before v4, why?
  for(int n=0;n<trace_entries;n++)
  {
    if((t_d_e[n].when)&TDE_AFTER)
    {
      if(t_d_e[n].regflag)
        t_d_e[n].val[1]=*(t_d_e[n].ptr);
      else
      {
        switch(t_d_e[n].bytes) {
        case 1:
          t_d_e[n].val[1]=d2_peek(t_d_e[n].ad);
          break;
        case 2:
          t_d_e[n].val[1]=d2_dpeek(t_d_e[n].ad);
          break;
        case 4:
          t_d_e[n].val[1]=d2_lpeek(t_d_e[n].ad);
          break;
        }
      }
    }
  }
}


void trace_display() {
  m_b_trace.ad=trace_pc;
  m_b_trace.update();
  trace_display_clear();
  HWND Parent=trace_scroller.GetControlPage();
  int y=2,x,max_x=430;
  for(int n=0;n<trace_entries;n++)
  {
    x=2;
    int name_width=get_text_width(t_d_e[n].name);
    trace_hLABEL[n]=CreateWindowEx(0,"Static",t_d_e[n].name,WS_VISIBLE|WS_CHILD,
      x,y+3,name_width,22,Parent,(HMENU)101,Inst,NULL);
    SendMessage(trace_hLABEL[n],WM_SETFONT,(UINT_PTR)fnt,0);
    if(t_d_e[n].when!=TDE_TEXT_ONLY)
    {
      x+=name_width+20;
      if(t_d_e[n].regflag)
      {
        if(t_d_e[n].when & TDE_BEFORE)
        {
          new mr_static("before",t_d_e[n].name,x,y,Parent,
            (HMENU)1,(MEM_ADDRESS*)&(t_d_e[n].val[0]),t_d_e[n].bytes,MST_REGISTER,true,NULL);
          x+=100;
        }
        if(t_d_e[n].when&TDE_AFTER)
        {
          new mr_static("after",t_d_e[n].name,x,y,Parent,
            (HMENU)1,(MEM_ADDRESS*)(t_d_e[n].ptr),t_d_e[n].bytes,MST_REGISTER,true,NULL);
          x+=100;
        }
      }
      else
      {
          //memory address
        EasyStr ad_desc="";
        Tiolist_entry*il[4]={NULL,NULL,NULL,NULL};
        int iwid[4]={0,0,0,0};
        int ic=0,ox;
        bool iols=false;
        for(int b=0;b<t_d_e[n].bytes;b++)
        {
          Tiolist_entry*i=search_iolist(t_d_e[n].ad+b);
          if(i)
          {
            if(ad_desc[0])ad_desc+=", ";
            ad_desc+=i->name;
            if(i->bitmask[0])
            {
              iwid[ic]=iolist_box_width(i);
              il[ic++]=i;
              iols=true;
              if(i->bytes==2)b++;
            }
          }
        }
        if(ad_desc[0])
        {  //special addresses
          ad_desc+=" - ";
        }
        ad_desc+="address";
        new mr_static(ad_desc,"address",x,y,Parent,
          (HMENU)0,(MEM_ADDRESS*)&(t_d_e[n].ad),3,MST_ADDRESS,false,NULL);
        x+=get_text_width(ad_desc)+60;
        if(t_d_e[n].when & TDE_BEFORE)
        {
          ox=x;
          new mr_static("before",t_d_e[n].name,x,y,Parent,(HMENU)1,
            (MEM_ADDRESS*)(INT_PTR)t_d_e[n].ad,t_d_e[n].bytes,
            MST_HISTORIC_MEMORY,true,(mem_browser*)&(t_d_e[n].val[0]));
          if(iols)
          {
            x+=100;
            for(int m=0;m<ic;m++)
            {
              new mr_static("",il[m]->name,x,y,Parent,(HMENU)1,
                (MEM_ADDRESS*)(il[m]),il[m]->bytes,MST_HISTORIC_IOLIST,true,
                (mem_browser*)&(t_d_e[n].val[0]));
              x+=iwid[m]+10;
            }
            max_x=MAX(x,max_x);
            x=ox;
            y+=24;
          }
          else
            x+=100;
        }
        if(t_d_e[n].when & TDE_AFTER)
        {
          new mr_static("after",t_d_e[n].name,x,y,Parent,(HMENU)1,
            (MEM_ADDRESS*)(INT_PTR)t_d_e[n].ad,t_d_e[n].bytes,MST_MEMORY,true,
            (mem_browser*)&(t_d_e[n].val[1]));
          x+=100;
          if(iols)
          {
            for(int m=0;m<ic;m++)
            {
              new mr_static("",il[m]->name,x,y,Parent,(HMENU)1,
                (MEM_ADDRESS*)(il[m]),il[m]->bytes,MST_IOLIST,true,
                (mem_browser*)&(t_d_e[n].val[1]));
              x+=iwid[m]+10;
            }
          }
        }
      }
    }
    max_x=MAX(x,max_x);
    y+=24;
  }
  y+=5;
  trace_scroller.SetHeight(1);trace_scroller.SetWidth(1);
  if(trace_show_window)
  {
    SetWindowPos(trace_window_handle,HWND_TOP,0,0,max_x+30,y+153+EXTRA_HEIGHT+
      GuiSM.cy_caption()+GetSystemMetrics(SM_CYHSCROLL),
      SWP_NOMOVE|SWP_FRAMECHANGED|SWP_SHOWWINDOW|SWP_NOACTIVATE);
  }
  else
    ShowWindow(trace_window_handle,SW_HIDE);
  trace_scroller.AutoSize();
}


LRESULT CALLBACK trace_window_WndProc(HWND Win,UINT Mess,
                      WPARAM wPar,LPARAM lPar) {
  switch(Mess)   {
  case WM_SIZE:
    MoveWindow(m_b_trace.handle,10,5,LOWORD(lPar)-20,50+EXTRA_HEIGHT,true);
    MoveWindow(trace_repeat_trace_button,10,HIWORD(lPar)-35,LOWORD(lPar)-20,
      30,true);
    SetWindowPos(trace_scroller,0,0,0,LOWORD(lPar)-20,HIWORD(lPar)
      -145-EXTRA_HEIGHT,SWP_NOZORDER|SWP_NOMOVE);
    break;
  case WM_DRAWITEM:
  {
    DRAWITEMSTRUCT *pDIS=(DRAWITEMSTRUCT*)lPar;
    mem_browser *mb=(mem_browser*)GetWindowLongPtr(pDIS->hwndItem,GWLP_USERDATA);
    if(mb) 
      mb->draw(pDIS);
    break;
  }
  case WM_CLOSE:
    ShowWindow(Win,SW_HIDE);
    return 0;
  case WM_DESTROY:
    trace_display_clear(); // Free mr_static memory
    trace_entries=0;
    break;
  case WM_CONTEXTMENU:
  case WM_COMMAND:
    if(HIWORD(wPar)==BN_CLICKED)
    {
      switch(LOWORD(wPar)) {
      case 1003:  //trace;
        trace_again();
        break;
      }
    }
    break;
  case WM_ACTIVATE:
    break;
  }
  return DefWindowProc(Win,Mess,wPar,lPar);
}


void trace_window_init() {
  trace_window_handle=CreateWindowEx(/*WS_EX_TOOLWINDOW*/0,"Steem Trace Window",
    "Trace",WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_SIZEBOX|WS_MAXIMIZEBOX,
    110,310,370,400+EXTRA_HEIGHT,NULL,NULL,Inst,0);
  m_b_trace.owner=trace_window_handle;
  m_b_trace.handle=CreateWindowEx(512,WC_LISTVIEW,"",
    LVS_REPORT|LVS_SHAREIMAGELISTS|LVS_NOSORTHEADER|LVS_OWNERDRAWFIXED
    |WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS,10,1,400,55,m_b_trace.owner,
    (HMENU)1,Inst,NULL);
  SetWindowLongPtr(m_b_trace.handle,GWLP_WNDPROC,(LONG_PTR)mem_browser_WndProc);
  SetWindowLongPtr(m_b_trace.handle,GWLP_USERDATA,(LONG_PTR)&m_b_trace);
//  m_b_trace.active=false;
  m_b_trace.disp_type=DT_INSTRUCTION;
  m_b_trace.mode=MB_MODE_FIXED;
  m_b_trace.editflag=true;
  m_b_trace.editbox=NULL;
  m_b_trace.lb_height=1;
  m_b_trace.init();
  CreateWindowEx(0,"Static","sr before",WS_VISIBLE|WS_CHILDWINDOW
    |SS_LEFTNOWORDWRAP,10,63+EXTRA_HEIGHT,50,17,trace_window_handle,
    (HMENU)0,Inst,NULL);
  trace_sr_before_display=CreateWindowEx(512,"Static","trace sr display",
    WS_BORDER|WS_VISIBLE|WS_CHILDWINDOW|SS_NOTIFY,60,60+EXTRA_HEIGHT,200,
    20,trace_window_handle,(HMENU)0,Inst,NULL);
  SetWindowLongPtr(trace_sr_before_display,GWLP_USERDATA,
    (LONG_PTR)&trace_sr_before);
  SetWindowLongPtr(trace_sr_before_display,GWLP_WNDPROC,
    (LONG_PTR)sr_display_WndProc);
  CreateWindowEx(0,"Static","sr after",WS_VISIBLE|WS_CHILDWINDOW
    |SS_LEFTNOWORDWRAP,10,83+EXTRA_HEIGHT,50,17,trace_window_handle,
    (HMENU)0,Inst,NULL);
  trace_sr_after_display=CreateWindowEx(512,"Static","trace sr display",
    WS_BORDER|WS_VISIBLE|WS_CHILDWINDOW|SS_NOTIFY,60,80+EXTRA_HEIGHT,200,20,
    trace_window_handle,(HMENU)0,Inst,NULL);
  SetWindowLongPtr(trace_sr_after_display,GWLP_USERDATA,(LONG_PTR)&SR);
  SetWindowLongPtr(trace_sr_after_display,GWLP_WNDPROC,
    (LONG_PTR)sr_display_WndProc);
  trace_scroller.CreateEx(512,WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL,
    10,105+EXTRA_HEIGHT,260,130,trace_window_handle,(HMENU)100,Inst);
  trace_repeat_trace_button=CreateWindowEx(512,"Button","Repeat Trace",
    WS_BORDER|WS_VISIBLE|WS_CHILDWINDOW|BS_PUSHBUTTON,150,76+EXTRA_HEIGHT,
    130,35,trace_window_handle,(HMENU)1003,Inst,NULL);
  SetWindowAndChildrensFont(trace_window_handle,fnt);
}

#endif//#ifdef DEBUG_BUILD
