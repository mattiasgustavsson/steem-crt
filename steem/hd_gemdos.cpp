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

DOMAIN: hard drive
FILE: hd_gemdos.cpp
CONDITION: DISABLE_STEMDOS musn't be defined
DESCRIPTION: Steem's virtual hard drive emulation. This is achieved through
intercepting ST OS calls and translating them to PC OS calls. In Steem SSE,
we call this emulation GEMDOS HD emulation, but internally it is STEMDOS,
while GEMDOS is the non-intercepted system.

The starting point is stemdos_intercept_trap_1(), where it is decided if a
call will be intercepted (STEMDOS) or not (GEMDOS).

cpu_op: intercept_os()
        |-> intercept_gemdos()
            |-> stemdos_intercept_trap_1()

When a call is intercepted, it can result in other calls. STEMDOS triggers
its own interrupts and checks at RTE, and intercept_os() is called again
just in case new PC was on an intercepted address.
Interception is not just on TRAP because, probably, programs (such as TOS)
can call the OS routines another way.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <computer.h>
#include <acc.h>
#include <gui.h>
#include <shortcutbox.h>
#include <mymisc.h>
#include <osd.h>
#include <harddiskman.h>
#include <translate.h>


bool mount_flag[26]; // A-Z
EasyStr mount_path[26];
bool stemdos_comline_read_is_rb=false; //Command line option
bool STfile_read_error=0;

#ifndef DISABLE_STEMDOS

void stemdos_Dfree(int dr);
void stemdos_read(int h,MEM_ADDRESS my_sp);
void stemdos_write(int h,MEM_ADDRESS my_sp);
DWORD stemdos_search_wildcard_PC_path();
void stemdos_trap_1();

BYTE stemdos_boot_drive=2; // C
bool stemdos_intercept_datetime=(0);
EasyStr mount_gemdos_path[26];
int stemdos_rte_action;
Tstemdos_file_struct stemdos_file[46];
Tstemdos_file_struct stemdos_new_file;
BYTE stemdos_std_handle_forced_to[6]={0,0,0,0,0,0}; // values between 6 and 45
Tstemdos_fsnext_struct_type stemdos_fsnext_struct[MAX_STEMDOS_FSNEXT_STRUCTS];
WORD stemdos_command;
WORD stemdos_attr;
EasyStr stemdos_filename;
EasyStr stemdos_rename_to_filename;
EasyStr PC_filename;
FILE *stemdos_Pexec_file=NULL;
MEM_ADDRESS stemdos_Pexec_com,stemdos_Pexec_env;
WORD stemdos_Pexec_mode;
int stemdos_Pexec_list_ptr;
MEM_ADDRESS stemdos_Pexec_list[MAX_STEMDOS_PEXEC_LIST];
bool stemdos_ignore_next_pexec4=0;
MEM_ADDRESS stemdos_dfree_buffer;
WORD stemdos_Fattrib_flag; //maybe byte
MEM_ADDRESS stemdos_dta;
short stemdos_save_sr;
BYTE stemdos_current_drive; // A: ...
MEM_ADDRESS original_return_address;

#ifdef SSE_DEBUG
short nMallocs=0; // to track leaks (Megamax C)
#endif


// Find "Allow wildcards?"

// The ST used create time but using standard functions we can't get or
// set create time on DOS (not sure about setting on UNIX). We would have to
// change file access to Windows commands (shudder).

// ST slash is like DOS/Windows: \   //
#define NO_ST_SLASH(cs) {size_t i=strlen(cs);if(i)if(cs[i-1]=='\\')cs[i-1]=0;}

#define LOGSECTION LOGSECTION_STEMDOS


void stemdos_init() {
  for(int n=0;n<26;n++)
  {
    mount_flag[n]=false;
    mount_path[n]="";
    mount_gemdos_path[n]="";
  }
  stemdos_new_file.open=false;
  stemdos_new_file.f=NULL;
  stemdos_new_file.attrib=0;
  stemdos_new_file.owner_program=0;
  stemdos_new_file.filename="";
  stemdos_new_file.date=0;
  stemdos_new_file.time=0;
  for(int n=0;n<46;n++)
  {
    stemdos_file[n].open=false;
    stemdos_file[n].f=NULL;
    stemdos_file[n].attrib=0;
    stemdos_file[n].owner_program=0;
    stemdos_file[n].filename="";
    stemdos_file[n].date=0;
    stemdos_file[n].time=0;
  }
  stemdos_Pexec_file=NULL;
}


void stemdos_reset() {
  stemdos_current_drive=stemdos_get_boot_drive();
  stemdos_close_all_files();
  for(int n=0;n<MAX_STEMDOS_FSNEXT_STRUCTS;n++)
  {
    stemdos_fsnext_struct[n].dta=0;
    stemdos_fsnext_struct[n].path="";
  }
  stemdos_Pexec_list_ptr=0;
  ZeroMemory(stemdos_Pexec_list,sizeof(stemdos_Pexec_list));
  stemdos_ignore_next_pexec4=0;
  for(int n=0;n<26;n++) 
    mount_gemdos_path[n]="";
  stemdos_intercept_datetime=true;
  LPEEK(SV_drvbits)=3;
  stemdos_update_drvbits();
#ifdef SSE_DEBUG
  nMallocs=0;
#endif
}


void stemdos_set_drive_reset() {
  stemdos_current_drive=stemdos_get_boot_drive();
}


void stemdos_intercept_trap_1() {
  bool Invalid=0;
  MEM_ADDRESS my_sp=get_sp_before_trap(&Invalid); //ss by looking at the stack
  if(Invalid) 
  {
    TRACE_LOG("STEMDOS Invalid SP %x\n",my_sp);
    return;
  }
  stemdos_command=m68k_dpeek(my_sp);
#if 0 // disabled: risky and more code...
  if(HardDiskMan.DisableHardDrives 
    && stemdos_command<0x2a && stemdos_command>0x2d //time
    && stemdos_command!=0x4e) //extended_monitor
  {
    return;
  }
#endif
  original_return_address=m68k_lpeek(areg[7]+2); // (areg[7]) is pushed sr
  switch(stemdos_command) {
  case 0: // Pterm0
  case 0x4c: // Pterm
  {
    DBG_LOG(EasyStr("STEMDOS: Pterm at address $")+HEXSl(pc,6));
    bool is_stemdos=stemdos_Pterm();
    if(is_stemdos&&tos_version<0x104) // no need for TOS 1.04 and above
    { // free memory if it was ours
      // (not memory allocated by the child process)
      on_rte=ON_RTE_STEMDOS;
      UPDATE_SR;
      stemdos_save_sr=SR;
      PSWI=7;
      DBG_LOG(EasyStr("STEMDOS: Calling Mfree($")+HEXSl(stemdos_Pexec_list[stemdos_Pexec_list_ptr],6)+")");
      on_rte_interrupt_depth=interrupt_depth+1;
      stemdos_trap_1_Mfree(stemdos_Pexec_list[stemdos_Pexec_list_ptr]);
      stemdos_rte_action=STEMDOS_RTE_MFREE2;
      TRACE_LOG("PTERM %d $%x\n",stemdos_Pexec_list_ptr,stemdos_Pexec_list[stemdos_Pexec_list_ptr]);
    }
    return; // do mfree, then term
  }
  // stdin
  case 0x01: // Cconin con:
  case 0x06: // Crawio N/A
  case 0x07: // Crawcin N/A
  case 0x08: // Cnecin con:
  case 0x0a: // Cconrs con:
  case 0x0b: // Cconis con:
  {
    BYTE h=stemdos_std_handle_forced_to[0]; //see Fforce ($46)
    if(h)
    {
      if(stemdos_file[h].open)
      {
        int c;
        switch(stemdos_command) {
        case 0x06:
          c=m68k_dpeek(my_sp+2);
          if(LOBYTE(c)!=0xff)
            break;
        case 0x01:
        case 0x07:
        case 0x08:
          c=fgetc(stemdos_file[h].f);
          if(c==EOF)
            c=0;
          Cpu.r[0]=c;
          stemdos_final_rte();
          break;
        case 0x0b:
          Cpu.r[0]=0xffffffff;
          stemdos_final_rte();
          break;
        case 0x0a:
          Cpu.r[0]=0;
          stemdos_final_rte();
          break;
        }
      }
      else
        stemdos_std_handle_forced_to[0]=0;
    }
    return;
  }
  // stdout con:
  case 0x02: // Cconout
  case 0x09: // Cconws
  case 0x10: // Cconos
  {
    BYTE h=stemdos_std_handle_forced_to[1];
    if(h)
    {
      if(stemdos_file[h].open)
      {
        if(stemdos_command==0x02)
        {
          fputc(m68k_dpeek(my_sp+2),stemdos_file[h].f);
          Cpu.r[0]=0;
        }
        else if(stemdos_command==0x09)
        {
          Str line=read_string_from_memory(m68k_lpeek(my_sp+2),32000);
          Cpu.r[0]=(int)fwrite(line.Text,1,line.Length(),stemdos_file[h].f);
        }
        else
          Cpu.r[0]=0xffffffff;
        stemdos_final_rte();
      }
      else
        stemdos_std_handle_forced_to[1]=0;
    }
    return;
  }
  // aux:
  case 0x03: // Cauxin
  case 0x04: // Cauxout
  case 0x12: // Cauxis
  case 0x13: // Cauxos
  {
    BYTE h=stemdos_std_handle_forced_to[2];
    if(h)
    {
      if(stemdos_file[h].open)
      {
        if(stemdos_command==0x04)
        {
          fputc(m68k_dpeek(my_sp+2),stemdos_file[h].f);
          Cpu.r[0]=0;
        }
        else if(stemdos_command==0x03)
          Cpu.r[0]=LOBYTE(fgetc(stemdos_file[h].f));
        else if(stemdos_command==0x12)
        {
          if(feof(stemdos_file[h].f))
            Cpu.r[0]=0;
          else
            Cpu.r[0]=0xffffffff;
        }
        else
          Cpu.r[0]=0xffffffff;
        stemdos_final_rte();
      }
      else
        stemdos_std_handle_forced_to[2]=0;
    }
    return;
  }
  // prn:
  case 0x05: // Prnout
  case 0x11: // Prnos
  {
    BYTE h=stemdos_std_handle_forced_to[3];
    if(h)
    {
      if(stemdos_file[h].open)
      {
        if(stemdos_command==0x05)
        {
          fputc(m68k_dpeek(my_sp+2),stemdos_file[h].f);
          Cpu.r[0]=0;
        }
        else
          Cpu.r[0]=0xffffffff;
        stemdos_final_rte();
      }
      else
        stemdos_std_handle_forced_to[3]=0;
    }
    return;
  }
  case 0x0E: // Dsetdrv
    DBG_LOG(Str("STEMDOS: Set current drive to ")+char('A'+m68k_dpeek(my_sp+2))+":");
#if defined(SSE_TOS_PRG_AUTORUN)
    if(!(OPTION_PRG_SUPPORT && FloppyDrive[DRIVE].ImageType.Manager==MNGR_PRG))
#endif
      stemdos_current_drive=BYTE(m68k_dpeek(my_sp+2));
    return;   //let Gemdos set its drive
  case 0x19:  // Dgetdrv
    Cpu.r[0]=stemdos_current_drive;
    stemdos_final_rte();
    return;
  case 0x1a: // Fsetdta
    DBG_LOG(EasyStr("STEMDOS: DTA set to $")+HEXSl(m68k_lpeek(my_sp+2),6));
    stemdos_dta=m68k_lpeek(my_sp+2);
    return; //let GEMDOS set the DTA
  case 0x20:  // Super
    return;
  case 0x2a: // Get Date
  case 0x2b: // Set Date
  case 0x2c: // Get Time
  case 0x2d: // Set Time
    if(stemdos_intercept_datetime&&OPTION_RTC_HACK)
    {
      time_t tmr=time(NULL); //SS used by GFA3 each time the editor appears
      struct tm *lpTime=localtime(&tmr);
      DWORD DOSTime=TMToDOSDateTime(lpTime);
      switch(stemdos_command) {
      case 0x2a: Cpu.r[0]=HIWORD(DOSTime); break;
      case 0x2c: Cpu.r[0]=LOWORD(DOSTime); break;
      case 0x2b: if(m68k_dpeek(my_sp+2)!=HIWORD(DOSTime)) 
        stemdos_intercept_datetime=0; return; // don't RTE
      case 0x2d: if(m68k_dpeek(my_sp+2)!=LOWORD(DOSTime)) 
        stemdos_intercept_datetime=0; return; // don't RTE
      }
      stemdos_final_rte();
    }
    return;
  case 0x31: // Ptermres
    DBG_LOG(EasyStr("STEMDOS: PTermRes at address $")+HEXSl(pc,6));
    stemdos_Pterm();
    // Gemdos TERM handles all memory fiddling
    return; // return to Gemdos
  case 0x36:  //Dfree
  {
    stemdos_dfree_buffer=m68k_lpeek(my_sp+2);
    WORD d=m68k_dpeek(my_sp+6);
    if(d==0)
      //need to get current drive
      d=stemdos_current_drive+1; //done!
    if(stemdos_check_mount((BYTE)(d-1)))
    {
      stemdos_Dfree(d-1);
      stemdos_final_rte(); //prevent GEMDOS
    }
    return;
  }
  case 0x39: // Dcreate
  case 0x3a: // Ddelete
  case 0x41: // Fdelete
  {
    DBG_LOG("STEMDOS: Intercepted Dcreate/Ddelete/Fdelete");
    stemdos_filename=read_string_from_memory(m68k_lpeek(my_sp+2),100);
    DBG_LOG(EasyStr("STEMDOS: Got filename as ")+stemdos_filename);
    int x=stemdos_get_file_path();
    if(x==STEMDOS_FILE_IS_STEMDOS)
    {
      DBG_LOG(EasyStr("STEMDOS: Intercepting the call"));
      switch(stemdos_command) {
      case 0x39: stemdos_mkdir(); break;
      case 0x3a: stemdos_rmdir(); break;
      case 0x41: stemdos_Fdelete(); break;
      }
      stemdos_final_rte(); //stop GEMDOS
      return;
    }
    DBG_LOG(EasyStr("STEMDOS: Leaving the call to GEMDOS"));
    return;  //GEMDOS Dcreate
  }
  case 0x3b: // Dsetpath
  {
    stemdos_filename=read_string_from_memory(m68k_lpeek(my_sp+2),100);
    int x=stemdos_get_file_path();
    if(x==STEMDOS_FILE_IS_STEMDOS)
    {
      if(stemdos_filename[2]==0||IsSameStr(stemdos_filename.Text+2,"\\"))
      {
        mount_gemdos_path[stemdos_current_drive]="";
        Cpu.r[0]=0; //succeeded
      }
      else
      { //nonempty path
        EasyStr NewFol=stemdos_filename.Text+2;
        NO_ST_SLASH(NewFol);
        DWORD Attrib=FILE_ATTRIBUTE_DIRECTORY;
        // Don't check existance if not changing it
        if(NotSameStr_I(NewFol,mount_gemdos_path[stemdos_current_drive]))
        {
          stemdos_get_PC_path();
          Attrib=GetFileAttributes(PC_filename);
        }
        if((Attrib & FILE_ATTRIBUTE_DIRECTORY)==0||Attrib==0xffffffff)
          Cpu.r[0]=-34;  //Path not found
        else
        {
          mount_gemdos_path[stemdos_current_drive]=NewFol;
          Cpu.r[0]=0; //succeeded
        }
      }
      DBG_LOG(EasyStr("STEMDOS: Set path for drive ")+char('A'+stemdos_current_drive)+": to "+mount_gemdos_path[stemdos_current_drive]);
      stemdos_final_rte();
    }
    return;
  }
  case 0x3d: // Fopen
  case 0x3c: // Fcreate
  {
#ifdef ENABLE_LOGFILE
    DBG_LOG((stemdos_command==0x3d)?"STEMDOS: Intercepted Fopen":"STEMDOS: Intercepted Fcreate");
#endif
    UPDATE_SR;
    stemdos_save_sr=SR;
    PSWI=7;
    stemdos_rte_action=0;
    stemdos_filename=read_string_from_memory(m68k_lpeek(my_sp+2),100);
    DBG_LOG(EasyStr("STEMDOS: Got filename as ")+stemdos_filename);
    int x=stemdos_get_file_path();
    if(x==STEMDOS_FILE_IS_STEMDOS)
    {
      if(stemdos_command==0x3c)
      { //fcreate
        if(m68k_dpeek(my_sp+6)&8)
        { //create volume label
          stemdos_finished();
          Cpu.r[0]=0; //succeeded, honest!
          stemdos_final_rte();  //don't do GEMDOS call, freaking volume label!
          return;
        }
      }
      DBG_LOG(EasyStr("STEMDOS: Intercepting the call"));
      stemdos_open_file(m68k_dpeek(my_sp+6));
      if(Cpu.r[0]<0)
        stemdos_final_rte(); //don't do GEMDOS call, couldn't open file
      return; //call GEMDOS to get file handle - interrupt already set up
    }
    DBG_LOG(EasyStr("STEMDOS: Leaving the call to GEMDOS"));
    stemdos_finished();
    return;
  }
  case 0x3e: // Fclose
  {
    UPDATE_SR;
    stemdos_save_sr=SR;
    PSWI=7;
    WORD h=m68k_dpeek(my_sp+2);
    if(h<6)
    {
      h=stemdos_std_handle_forced_to[h];
      stemdos_std_handle_forced_to[h]=0;
    }
    if(h<6||h>45)
      stemdos_finished();  //GEMDOS can handle this one!
    else if(stemdos_file[h].open)
    { //one of ours
      stemdos_close_file(&(stemdos_file[h]));
      DBG_LOG(EasyStr("STEMDOS: Closed file #")+h);
      on_rte=ON_RTE_STEMDOS;
      on_rte_interrupt_depth=interrupt_depth+1;
      stemdos_rte_action=STEMDOS_RTE_FCLOSE;
      stemdos_trap_1_Fclose(h); //deallocate handle
      //Fclose, close phoney file handle
    }
    else
      stemdos_finished();  //GEMDOS file
    return;
  }
  case 0x3f: // Fread
  {
    WORD h=m68k_dpeek(my_sp+2);
    if(h<6) 
      h=stemdos_std_handle_forced_to[h];
    if(h>=6&&h<=45)
    {
      if(stemdos_file[h].open)
      { //one of ours
        stemdos_read(h,my_sp);
        stemdos_final_rte(); //prevent GEMDOS
      }
    }
    return;
  }
  case 0x40: // Fwrite
  {
    WORD h=m68k_dpeek(my_sp+2);
    if(h<6) 
      h=stemdos_std_handle_forced_to[h];
    if(h>=6&&h<=45)
    {
      if(stemdos_file[h].open)
      { //one of ours
        stemdos_write(h,my_sp);
        stemdos_final_rte();  //prevent GEMDOS
      }
    }
    return;
  }
  case 0x42: // Fseek
  {
    WORD h=m68k_dpeek(my_sp+6);
    if(h<6) 
      h=stemdos_std_handle_forced_to[h];
    if(h>=6&&h<=45)
    {
      if(stemdos_file[h].open)
      { //one of ours
        stemdos_seek(h,my_sp);
        stemdos_final_rte();
      }
    }
    return;
  }
  case 0x43: // Fattrib
  {
    DBG_LOG("STEMDOS: intercepted Fattrib");
    stemdos_filename=read_string_from_memory(m68k_lpeek(my_sp+2),100);
    DBG_LOG(EasyStr("STEMDOS: Got filename as ")+stemdos_filename);
    stemdos_attr=m68k_dpeek(my_sp+8);
    stemdos_Fattrib_flag=m68k_dpeek(my_sp+6);
    int x=stemdos_get_file_path();
    if(x==STEMDOS_FILE_IS_STEMDOS)
    {
      DBG_LOG(EasyStr("STEMDOS: Intercepting the call"));
      stemdos_Fattrib();
      stemdos_final_rte(); //stop GEMDOS
      return;
    }
    DBG_LOG(EasyStr("STEMDOS: Leaving the call to GEMDOS"));
    return;  //GEMDOS Fattrib
  }
  case 0x46: // Fforce
  { 
    WORD hStd=m68k_dpeek(my_sp+2),h=m68k_dpeek(my_sp+4);
    if(hStd<6)
    {
      if(h>=6&&h<46)
      {
        if(stemdos_file[h].open)
        { //one of ours
          stemdos_std_handle_forced_to[hStd]=(BYTE)h;
          Cpu.r[0]=0;
          stemdos_final_rte();
        }
        else
          stemdos_std_handle_forced_to[hStd]=0;
      }
    }
    return;
  }
  case 0x47: // Dgetpath
  {
    WORD drive=m68k_dpeek(my_sp+6); //what's it set the drive to?
    if(drive==0)
      drive=stemdos_current_drive;
    else
      drive--;
    if(stemdos_check_mount((BYTE)drive))
    {
      write_string_to_memory(m68k_lpeek(my_sp+2),mount_gemdos_path[drive]);
      DBG_LOG(Str("STEMDOS: DgetPath for drive ")+char('A'+drive)+" returned "+mount_gemdos_path[drive]);
      Cpu.r[0]=0;
      stemdos_final_rte();
    }
    return;
  }
  case 0x48: // Malloc
#ifdef SSE_DEBUG
    TRACE_LOG("PC %X Malloc #%d %d\n",old_pc,++nMallocs,m68k_lpeek(my_sp+2));
#endif
    DBG_LOG(EasyStr("STEMDOS: Malloc(")+(long)m68k_lpeek(my_sp+2)+") called at address $"+HEXSl(old_pc,6));
    return;
  case 0x49: // Mfree
#ifdef SSE_DEBUG
    TRACE_LOG("PC %X Mfree #%d %x\n",old_pc,nMallocs--,m68k_lpeek(my_sp+2));
#endif
    DBG_LOG(EasyStr("STEMDOS: Mfree($")+HEXSl(m68k_lpeek(my_sp+2),6)+") called at address $"+HEXSl(old_pc,6));
    return;
  case 0x4A: // Mshrink
    TRACE_LOG("Mshrink %x to %d\n",m68k_lpeek(my_sp+4),m68k_lpeek(my_sp+8));
    DBG_LOG(EasyStr("STEMDOS: SetBlock(0, $")+HEXSl(m68k_lpeek(my_sp+4),6)+", "+m68k_lpeek(my_sp+8)+") called at address $"+HEXSl(old_pc,6));
    return;
  case 0x4B: // Pexec (mode,fil,com,env)
  {
#if defined(SSE_TOS_KEYBOARD_CLICK)
    Tos.CheckKeyboardClick();
#endif
      //modes - 0=Load n' go
      //        3=Load n' dont go (return basepage address in D0)
      //        4=Run from memory (fil=ignored,com=Address,env=ignored)
      //        5=Make basepage (fil=ignored)
    DBG_LOG("STEMDOS: Intercepted Pexec");
    WORD exec_mode=m68k_dpeek(my_sp+2);
#if defined(SSE_STATS)
    if(exec_mode==0||exec_mode==4)
      Stats.nPrg++;
#endif
    if(exec_mode==0||exec_mode==3)
    {
      UPDATE_SR;
      stemdos_save_sr=SR;
      PSWI=7;
#ifdef DEBUG_BUILD
      // this will work only when going super->user
      if(stop_on_next_program_run)
      {
        stop_on_user_change=1;
        stop_on_next_program_run=2;
      }
#endif
      stemdos_filename=read_string_from_memory(m68k_lpeek(my_sp+4),100);
      DBG_LOG(EasyStr("STEMDOS: Got filename as ")+stemdos_filename);
      int x=stemdos_get_file_path();
      if(x==STEMDOS_FILE_IS_STEMDOS)
      {
        DBG_LOG(EasyStr("STEMDOS: Extended filename to ")+stemdos_filename);
        DBG_LOG(EasyStr("STEMDOS: Intercepting the call"));
        DBG_LOG(EasyStr("A7 = ")+HEXSl(my_sp,6));
        stemdos_Pexec_com=m68k_lpeek(my_sp+8);
        stemdos_Pexec_env=m68k_lpeek(my_sp+12);
        stemdos_Pexec_mode=exec_mode; //SS save that for after basepage...
        stemdos_get_PC_path();
        stemdos_search_wildcard_PC_path();
        DBG_LOG(EasyStr("STEMDOS: Got the PC filename as ")+PC_filename);
        stemdos_Pexec_file=fopen(PC_filename,"rb");
        if(stemdos_Pexec_file)
        {
          TRACE_LOG("Exec STEMDOS %s\n",stemdos_filename.Text);
          DBG_LOG("STEMDOS: Opened the file on the PC's disk");
          on_rte=ON_RTE_STEMDOS;
          on_rte_interrupt_depth=interrupt_depth+1;
          stemdos_rte_action=STEMDOS_RTE_PEXEC;
          DBG_LOG("STEMDOS: Asking Gemdos to set up a basepage");
          stemdos_trap_1_Pexec_basepage(); //unique place of calling
        }
        else
        { //no such file
          DBG_LOG("STEMDOS: Couldn't open file");
          Cpu.r[0]=-33;
          stemdos_finished();
          stemdos_final_rte();
        }
      }
      else if(x==STEMDOS_FILE_IS_GEMDOS)
      {
        DBG_LOG(EasyStr("STEMDOS: Leaving the call to GEMDOS"));
        TRACE_LOG("%s left to GEMDOS\n",stemdos_filename.Text); 
        stemdos_add_to_Pexec_list(0); //log latest program as Gemdos
        stemdos_finished();
      }
    }
    else if(exec_mode==4||exec_mode==6)
    {
      if(stemdos_ignore_next_pexec4)
        // This is a hard drive program, we change the exec_mode 0 call to 
        // exec_mode 4 (or 6)
        stemdos_ignore_next_pexec4=0;
      else
        stemdos_add_to_Pexec_list(0); //log latest program as Gemdos
    }
    return;
  }
  case 0x4e: // Fsfirst
  {
    DBG_LOG("STEMDOS: Intercepted FSfirst");
    stemdos_filename=read_string_from_memory(m68k_lpeek(my_sp+2),100);
    if(extended_monitor==1)
    {
      if(stemdos_filename=="\\AUTO\\*.PRG")
      {
        call_a000();
        extended_monitor++;
        return; // When $A000 RTEs we will return to the gemdos vector address,
                // and then fsfirst will do its magic
      }
    }
    DBG_LOG(EasyStr("STEMDOS: Got filename as ")+stemdos_filename);
    int x=stemdos_get_file_path();
    if(x==STEMDOS_FILE_IS_STEMDOS)
    {
      DBG_LOG(EasyStr("STEMDOS: Intercepting the call"));
      UPDATE_SR;
      stemdos_save_sr=SR;
      PSWI=7;
      on_rte=ON_RTE_STEMDOS;
      on_rte_interrupt_depth=interrupt_depth+1;
      stemdos_rte_action=STEMDOS_RTE_GET_DTA_FOR_FSFIRST;
      stemdos_trap_1_Fgetdta();
      return;  //GEMDOS will now execute
    }
    DBG_LOG(EasyStr("STEMDOS: Leaving the call to GEMDOS"));
    return;  //GEMDOS allowed to continue
  }
  case 0x4f: // Fsnext
  {
    DBG_LOG("STEMDOS: Intercepted FSnext");
    if(m68k_peek(stemdos_dta)==0xb&&m68k_peek(stemdos_dta+1)==0xad
      &&m68k_peek(stemdos_dta+2)==0xde&&m68k_peek(stemdos_dta+3)==0xed)
    { //magic number for STEMDOS search
      stemdos_fsnext();
      stemdos_final_rte();  //don't go to GEMDOS
    }
    return;
  }
  case 0x56: // Frename
  {
    DBG_LOG("STEMDOS: Intercepted Frename");
    stemdos_filename=read_string_from_memory(m68k_lpeek(my_sp+4),100);
    stemdos_rename_to_filename=read_string_from_memory(m68k_lpeek(my_sp+8),100);
    DBG_LOG(EasyStr("STEMDOS: Got filename as ")+stemdos_filename);
    int x=stemdos_get_file_path();
    if(x==STEMDOS_FILE_IS_STEMDOS)
    {
      DBG_LOG(EasyStr("STEMDOS: Intercepting the call"));
      stemdos_rename();
      stemdos_final_rte(); //stop GEMDOS
      return;
    }
    DBG_LOG(EasyStr("STEMDOS: Leaving the call to GEMDOS"));
    return;  //GEMDOS Fattrib
  }
  case 0x57: // Fdatime
  {
    WORD h=m68k_dpeek(my_sp+6);
    if(h<6) 
      h=stemdos_std_handle_forced_to[h];
    if(h>=6&&h<=45)
    {
      if(stemdos_file[h].open)
      { //one of ours
        stemdos_Fdatime(h,my_sp);
        stemdos_final_rte(); //prevent GEMDOS
      }
    }
    return;
  }
  default: 
    break;
  }//sw
}


BYTE stemdos_get_boot_drive() {
  if(stemdos_boot_drive<2) 
    return 0;
  // If there is a disk in drive A then boot from it except if
  // the control key is being held down
  bool NoControl=true;
  if(CutDisableKey[VK_CONTROL]==0) 
    NoControl=(GetKeyState(VK_CONTROL)<0)==0;
#if defined(SSE_TOS_PRG_AUTORUN)
  if(FloppyDrive[0].ImageType.Manager!=MNGR_PRG
    && FloppyDrive[0].DiskInDrive()&&stemdos_boot_drive==AUTORUN_HD)
    return 0;
  else if(FloppyDrive[0].ImageType.Manager==MNGR_PRG && OPTION_PRG_SUPPORT)
    return AUTORUN_HD;
#endif
  if(FloppyDrive[0].DiskInDrive()&&NoControl) 
    return 0;
  return (stemdos_check_mount(stemdos_boot_drive)?stemdos_boot_drive:0);
}


bool stemdos_any_files_open() {
  for(int n=6;n<46;n++)
    if(stemdos_file[n].open) 
      return true;
  return 0;
}


void stemdos_final_rte() {
  //clear stack from original GEMDOS call
  m68kPerformRte();
#if defined(SSE_STATS)
  Stats.nGemdosi++; // other places?
#endif
  interrupt_depth--;
  intercept_os();
}


void stemdos_rte() {
  if((stemdos_rte_action & STEMDOS_RTE_SUBACTION)==STEMDOS_RTE_DUP)
  { //get file handle
    areg[7]+=4; //correct stack
    if(Cpu.r[0]>=6&&Cpu.r[0]<=45)
    { //valid file handle
      DBG_LOG(EasyStr("STEMDOS: Dup returned file handle #")+Cpu.r[0]);
      if(stemdos_file[Cpu.r[0]].open)
      {
        // Dup has somehow returned file handle that we already have open!
        TRACE_LOG("fdup handle %d already open\n",Cpu.r[0]);
        DBG_LOG("STEMDOS: dup returned a file that already exists! Trying again.");
        stemdos_rte_action=STEMDOS_RTE_FOPEN+STEMDOS_RTE_DUP; //get file handle
        stemdos_trap_1_Fdup();
      }
      else
      {
        stemdos_file[Cpu.r[0]]=stemdos_new_file;
        stemdos_new_file.open=false;
        TRACE_LOG("fdup handle %d for %s\n",Cpu.r[0],stemdos_new_file.filename.Text);
        DBG_LOG(EasyStr("STEMDOS: File set up with handle: ")+Cpu.r[0]);
        stemdos_finished(); //handle in d0
        stemdos_final_rte();  //kill the original GEMDOS call.
      }
    }
    else
    {
      DBG_LOG(EasyStr("STEMDOS: Dup failed and returned ")+Cpu.r[0]);
      TRACE_LOG("fdup error %d\n",Cpu.r[0]);
      stemdos_close_file(&stemdos_new_file);
      if(Cpu.r[0]>=0) 
        Cpu.r[0]=-65;  //internal GEMDOS error if we got crazy error
      stemdos_finished(); //error code in d0
      stemdos_final_rte();  //kill the original GEMDOS call.
    }
  }
  else if(stemdos_rte_action==STEMDOS_RTE_GET_DTA_FOR_FSFIRST)
  {
    areg[7]+=2; //correct stack
    stemdos_dta=(Cpu.r[0]&0xffffff); //get DTA
    bool Invalid=0;
    MEM_ADDRESS my_sp=get_sp_before_trap(&Invalid);
    // Pass stack pointer with original call info on it
    if(Invalid==0) 
      stemdos_fsfirst(my_sp);
    stemdos_finished();
    stemdos_final_rte();  //kill the original GEMDOS call.
  }
  else if(stemdos_rte_action==STEMDOS_RTE_FCLOSE)
  {
    DBG_LOG("STEMDOS: Gemdos has deallocated handle for fclose");
    areg[7]+=4; //correct stack
    stemdos_finished();
    stemdos_final_rte();  //kill the original GEMDOS call.
  }
  else if(stemdos_rte_action==STEMDOS_RTE_PEXEC)
  {
    // SS We come here on the RTE from PEXEC mode 5 basepage
    // called by Stemdos itself when a program calls PEXEC mode
    // 0 or 3 for a HD file
    DBG_LOG("STEMDOS: Created basepage for new program");
    areg[7]+=16; //correct stack, 3 longs and 2 words
    stemdos_Pexec(); //SS only place of calling
  }
  else if(stemdos_rte_action==STEMDOS_RTE_MFREE)
  { // on ctrl-C
    DBG_LOG(EasyStr("STEMDOS: Returned from readline, now calling Mfree($")+HEXSl(stemdos_Pexec_list[stemdos_Pexec_list_ptr],6)+")");
    stemdos_trap_1_Mfree(stemdos_Pexec_list[stemdos_Pexec_list_ptr]);
    stemdos_rte_action=STEMDOS_RTE_MFREE2;
  }
  else if(stemdos_rte_action==STEMDOS_RTE_MFREE2)
  {
    DBG_LOG("STEMDOS: Correcting stack after mfree, now GEMDOS will process term");
    areg[7]+=6; //correct stack
    SR=stemdos_save_sr; //retain status
    UPDATE_FLAGS;
    stemdos_finished();
    // don't need final_rte because GEMDOS has already returned
  }
}


void stemdos_update_drvbits() {
  for(BYTE n=2;n<32;n++)
  {
    if(stemdos_check_mount(n)) 
      LPEEK(SV_drvbits)|=(1<<n);
  }
}


void stemdos_get_PC_path() {
  StrUpperNoSpecial(stemdos_filename);
  PC_filename=mount_path[toupper(stemdos_filename[0])-'A'];
#if defined(UNIX)
  EasyStr sf=stemdos_filename;
  for(int i=0;sf[i];i++) if(sf[i]=='\\') sf[i]='/';
  PC_filename=find_file_i(PC_filename,sf.Text+3);
#else
  PC_filename+=(stemdos_filename.Text+2);
#endif
}


DWORD stemdos_search_wildcard_PC_path() {
  DirSearch ds;
  ds.st_only=true;
  if(ds.Find(PC_filename)==0) 
    return 0xffffffff;
  RemoveFileNameFromPath(PC_filename,KEEP_SLASH);
  PC_filename+=ds.Name;
  DWORD Attrib=ds.Attrib;
  ds.Close();
  return Attrib;
}


void stemdos_open_file(int param) {
  FILE *f=NULL;
  stemdos_get_PC_path();
  DBG_LOG(EasyStr("STEMDOS: PC filename is ")+PC_filename);
  stemdos_new_file.attrib=0;
  Cpu.r[0]=0;
  if(PC_filename.RightChar()==SLASHCHAR)
    // Don't allow empty names
    Cpu.r[0]=-34;
  else if(stemdos_command==0x3d)
  { //open
    DWORD Attrib=stemdos_search_wildcard_PC_path();
    if(Attrib!=0xffffffff)
    {
      if(Attrib & FILE_ATTRIBUTE_DIRECTORY)
      {
        Cpu.r[0]=-34;
        DBG_LOG("STEMDOS: Attempting to open a directory, failing");
      }
      else if((Attrib & FILE_ATTRIBUTE_READONLY)&&param!=0)
      {
        Cpu.r[0]=-36;
        DBG_LOG("STEMDOS: Attempting to open read-only file for write, failing");
      }
      else
      {
        stemdos_new_file.attrib=Attrib &
          (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_READONLY);
        Attrib&=~(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM
          |FILE_ATTRIBUTE_READONLY);
        SetFileAttributes(PC_filename,Attrib);
        DBG_LOG(EasyStr("STEMDOS: The file exists, PC filename is ")+PC_filename);
        char *pc_mode="r+b"; // Don't wipe the file but allow writing
        // There is a problem here, the ST can open files for read, and then write to them.
        // DOS Windows seems fine with it too, but XP and UNIX are sensible.
        // Always opening for read and write causes problems with permissions and
        // clashes with other programs. Command line option only solution.
        if(stemdos_comline_read_is_rb&&param==0) 
          pc_mode="rb";
        f=fopen(PC_filename,pc_mode);
        if(f==NULL)
        {
          Cpu.r[0]=-34;
          DBG_LOG("     Couldn't open file for Fopen");
        }
        else
        {
          Cpu.r[0]=0;
          fseek(f,0,SEEK_SET); // Always start at offset 0, whatever mode opened
          DBG_LOG("     File opened for fopen");
        }
      }
    }
    else
    {
      Cpu.r[0]=-33; //File not found
      DBG_LOG("STEMDOS: File not found for Fopen");
    }
  }
  if(Cpu.r[0]>=0&&stemdos_command==0x3c)
  { // Fcreate
#ifdef WIN32
    // We need to set the creation date of the file, but on Windows deleting the
    // file and re-creating it immediately doesn't work!
    HANDLE h=CreateFile(PC_filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
      FILE_FLAG_WRITE_THROUGH,NULL);
    if(h!=INVALID_HANDLE_VALUE)
    {
      FILETIME ft;
      GetSystemTimeAsFileTime(&ft);
      SetFileTime(h,&ft,&ft,&ft);
      CloseHandle(h);
      stemdos_new_file.attrib=0;
      if(param&2) 
        stemdos_new_file.attrib|=FILE_ATTRIBUTE_HIDDEN;
      if(param&4) 
        stemdos_new_file.attrib|=FILE_ATTRIBUTE_SYSTEM;
      if(param&1) 
        stemdos_new_file.attrib|=FILE_ATTRIBUTE_READONLY;
      // QUESTION: Is this closing/reopening necessary any more?
      SetFileAttributes(PC_filename,0);
      DBG_LOG("STEMDOS: Set new attributes for Fcreate file");
      f=fopen(PC_filename,"w+b");
      TRACE_LOG("Create file %s\n",PC_filename.Text);
      DBG_LOG("STEMDOS: Opened Fcreate file for write");
      if(f!=NULL) fseek(f,0,SEEK_SET); // Always start at offset 0
    }
#else//UNIX: (some code duplication for IDE)
    DeleteFile(PC_filename);
    f=fopen(PC_filename,"wb");
    if(f)
    {
      fclose(f);
      stemdos_new_file.attrib=0;
      if(param&2) stemdos_new_file.attrib|=FILE_ATTRIBUTE_HIDDEN;
      if(param&4) stemdos_new_file.attrib|=FILE_ATTRIBUTE_SYSTEM;
      if(param&1) stemdos_new_file.attrib|=FILE_ATTRIBUTE_READONLY;
      // QUESTION: Is this closing/reopening necessary any more?
      SetFileAttributes(PC_filename,0);
      DBG_LOG("STEMDOS: Set new attributes for Fcreate file");
      f=fopen(PC_filename,"w+b");
      DBG_LOG("STEMDOS: Opened Fcreate file for write");
      if(f!=NULL) 
        fseek(f,0,SEEK_SET); // Always start at offset 0
    }
#endif
    if(f==NULL) 
      Cpu.r[0]=-34;
  }
  if(Cpu.r[0]>=0)
  { //succeeded
    stemdos_new_file.open=true;
    stemdos_new_file.f=f;
    stemdos_new_file.owner_program=stemdos_Pexec_list_ptr;
    stemdos_new_file.filename=PC_filename;
    stemdos_new_file.date=0;
    stemdos_new_file.time=0;
    stemdos_rte_action=STEMDOS_RTE_FOPEN+STEMDOS_RTE_DUP; //get file handle
    on_rte=ON_RTE_STEMDOS;
    on_rte_interrupt_depth=interrupt_depth+1;
    TRACE_LOG("Open %s\n",PC_filename.Text);
    stemdos_trap_1_Fdup();
    DBG_LOG("STEMDOS: Asking GEMDOS for file handle");
  }
  else  // Failed to open file
    stemdos_finished(); // Error code is in D0
}


void stemdos_close_all_files() {
  if(stemdos_new_file.open) 
    stemdos_close_file(&stemdos_new_file);
  if(stemdos_Pexec_file)
  {
    fclose(stemdos_Pexec_file);
    stemdos_Pexec_file=NULL;
  }
  for(int n=6;n<=45;n++)
  {
    if(stemdos_file[n].open) 
      stemdos_close_file(&(stemdos_file[n]));
  }
}


void stemdos_close_file(Tstemdos_file_struct* sfs) {
  // close PC file
  if(sfs->open)
  {
    fflush(sfs->f);
    fclose(sfs->f);sfs->f=NULL;
    if(sfs->time||sfs->date)
    {
#ifdef WIN32
      DWORD attrib=GetFileAttributes(sfs->filename);
      HANDLE f=CreateFile(sfs->filename,GENERIC_READ|GENERIC_WRITE,0,NULL,
        OPEN_EXISTING,attrib,NULL);
      if(f!=INVALID_HANDLE_VALUE)
      {
        FILETIME ft,gmt_ft;
        DosDateTimeToFileTime(sfs->date,sfs->time,&ft);
        // Convert to GMT (all ST times are local time, all PC times are GMT)
        LocalFileTimeToFileTime(&ft,&gmt_ft);
        SetFileTime(f,&gmt_ft,NULL,NULL);
        CloseHandle(f);
      }
#endif
#ifdef UNIX
      ///// How?
#endif
    }
    if(sfs->attrib)
    {
      DWORD win_attr=GetFileAttributes(sfs->filename);
      win_attr&=~(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_READONLY);
      win_attr|=sfs->attrib & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_READONLY);
      SetFileAttributes(sfs->filename,win_attr);
      sfs->attrib=0;
    }
    sfs->open=false;
  }
}


void stemdos_read(int h,MEM_ADDRESS my_sp) {
  long count=m68k_lpeek(my_sp+4);
#if defined(SSE_STATS)
  Stats.nHdsector+=count/512;
#endif
  MEM_ADDRESS buf=m68k_lpeek(my_sp+8);
  DBG_LOG(EasyStr("STEMDOS: fread(Handle=")+h+", Count="+count+")");
//  TRACE_LOG("read %d from %d to %X\n",count,h,buf);
  int c=0,i;
  while(c<count)
  {
    i=fgetc(stemdos_file[h].f);
    if(i==EOF) 
      break;
    c++;
    m68k_poke(buf,LOBYTE(i));
    buf++;
  }
  Cpu.r[0]=c; //number of characters read
  HDDisplayTimer=timer+HD_TIMER;
  DBG_LOG(EasyStr("STEMDOS: FRead returned ")+Cpu.r[0]);
}


void stemdos_write(int h,MEM_ADDRESS my_sp) {
  long count=m68k_lpeek(my_sp+4);
#if defined(SSE_STATS)
  Stats.nHdsector+=count/512;
#endif
  MEM_ADDRESS buf=m68k_lpeek(my_sp+8);
//  TRACE_LOG("write %d from %X to %d\n",count,buf,h);
  int c=0,i;
  while(c<count)
  {
    i=m68k_peek(buf);
    buf++;
    c++;
    if(fputc(i,stemdos_file[h].f)==EOF)
    { //error
      DBG_LOG("STEMDOS: fwrite - error writing to file");
      Cpu.r[0]=-36;return;
    }
  }
  Cpu.r[0]=c; //number of characters written
  HDDisplayTimer=timer+HD_TIMER;
  DBG_LOG(EasyStr("STEMDOS: fwrite wrote ")+c+" bytes successfully");
}


void stemdos_seek(int h,MEM_ADDRESS my_sp) {
  long offset=m68k_lpeek(my_sp+2);
  int seekmode=m68k_dpeek(my_sp+8);
  DBG_LOG(EasyStr("STEMDOS: FSeek(Offset=")+offset+", Handle="+h+", SeekMode="+seekmode+")");
  long new_pos=-1,old_pos=ftell(stemdos_file[h].f);
  long file_len=GetFileLength(stemdos_file[h].f);
  if(seekmode==0)
    new_pos=offset;
  else if(seekmode==1)
    new_pos=old_pos+offset;
  else if(seekmode==2)
    new_pos=file_len+offset; // Offset must be negative!
  int error=1;
  if(new_pos>=0&&new_pos<=file_len)
  {
    error=fseek(stemdos_file[h].f,new_pos,SEEK_SET);
    // error might be 0 even if function fails - DOS doesn't verify!
    if(error) 
      fseek(stemdos_file[h].f,old_pos,SEEK_SET);
  }
  if(error)
    Cpu.r[0]=-64;
  else
    Cpu.r[0]=ftell(stemdos_file[h].f);
  DBG_LOG(EasyStr("STEMDOS: FSeek returned ")+Cpu.r[0]);
}


void stemdos_Fdatime(int h,MEM_ADDRESS my_sp) {
  MEM_ADDRESS timeptr=m68k_lpeek(my_sp+2);
  int flag=m68k_dpeek(my_sp+8);
  if(flag==0)
  { //read
    WORD date,time;
    if(stemdos_file[h].time||stemdos_file[h].date)
    {
      date=stemdos_file[h].date;
      time=stemdos_file[h].time;
    }
    else
    {
#ifdef WIN32
      FILETIME local_ft;
      DirSearch ds(stemdos_file[h].filename);
#if 1 //from Petari
      FileTimeToLocalFileTime(&ds.LastWriteTime,&local_ft); // Convert from GMT
#else
      FileTimeToLocalFileTime(&ds.CreationTime,&local_ft); // Convert from GMT
#endif
      FileTimeToDosDateTime(&local_ft,&date,&time);
#endif
#ifdef UNIX
      struct stat s;
      fstat(fileno(stemdos_file[h].f),&s);
      DWORD ddt=TMToDOSDateTime(localtime(&(s.st_ctime)));
      date=HIWORD(ddt);
      time=LOWORD(ddt);
#endif
    }
    m68k_poke(timeptr+0,HIBYTE(time));
    m68k_poke(timeptr+1,LOBYTE(time));
    m68k_poke(timeptr+2,HIBYTE(date));
    m68k_poke(timeptr+3,LOBYTE(date));
  }
  else
  {
    stemdos_file[h].time=MAKEWORD(m68k_peek(timeptr+1),m68k_peek(timeptr+0));
    stemdos_file[h].date=MAKEWORD(m68k_peek(timeptr+3),m68k_peek(timeptr+2));
  }
  Cpu.r[0]=0; //success!
}


void stemdos_fsfirst(MEM_ADDRESS my_sp) {
  int fsn=-1;
  stemdos_get_PC_path();
  DBG_LOG(EasyStr("STEMDOS: Stemdos -- fsfirst, the PC path to search is ")+PC_filename);
  // Search for search with this DTA
  for(int n=0;n<MAX_STEMDOS_FSNEXT_STRUCTS;n++)
  {
    if(stemdos_fsnext_struct[n].dta==stemdos_dta)
    {
      fsn=n;
      break;
    }
  }
  if(fsn==-1)
  {
    // New search
    for(int n=0;n<MAX_STEMDOS_FSNEXT_STRUCTS;n++)
    {
      if(stemdos_fsnext_struct[n].dta==0)
      {
        fsn=n;
        break;
      }
    }
  }
  if(fsn==-1)
  {
    // There are 100 active searches!!
    DBG_LOG("STEMDOS: Fsfirst has run out of structures, destroying oldest search");
    DWORD oldest=0xffffffff;
    int oldest_n=0;
    for(int n=0;n<MAX_STEMDOS_FSNEXT_STRUCTS;n++)
    {
      if(stemdos_fsnext_struct[n].start_hbl<oldest)
      {
        oldest=stemdos_fsnext_struct[n].start_hbl;
        oldest_n=n;
      }
    }
    fsn=oldest_n;
  }
  stemdos_fsnext_struct[fsn].dta=stemdos_dta;
  stemdos_fsnext_struct[fsn].NextFile="";         // Get first file 
  stemdos_fsnext_struct[fsn].path=PC_filename;
  stemdos_fsnext_struct[fsn].attr=m68k_dpeek(my_sp+6); //attributes
  stemdos_fsnext_struct[fsn].start_hbl=hbl_count;
  m68k_poke(stemdos_dta,0xb);  //magic number for STEMDOS search
  m68k_poke(stemdos_dta+1,0xad);  //magic number for STEMDOS search
  m68k_poke(stemdos_dta+2,0xde);  //magic number for STEMDOS search
  m68k_poke(stemdos_dta+3,0xed);  //magic number for STEMDOS search
  m68k_poke(stemdos_dta+4,BYTE(fsn));  //store number of search
  stemdos_fsnext();
}


int PCAttrToSTAttr(DWORD PCAttr) {
  int STAttr=0;
  if(PCAttr & FILE_ATTRIBUTE_HIDDEN)    
    STAttr|=0x2;
  if(PCAttr & FILE_ATTRIBUTE_SYSTEM)    
    STAttr|=0x4;
  if(PCAttr & FILE_ATTRIBUTE_DIRECTORY) 
    STAttr|=0x10;
  return STAttr;
}


void stemdos_fsnext() {
  int fsn=m68k_peek(stemdos_dta+4); // Search number
  if(fsn>=0&&fsn<MAX_STEMDOS_FSNEXT_STRUCTS)
  { // In range
    if(stemdos_fsnext_struct[fsn].dta!=stemdos_dta) 
      fsn=-1; // Not the current search
  }
  else
  {
    if(fsn==255)
    { // Flag to indicate finished search
      Cpu.r[0]=-49;  // No more files error (or is it -47? cf gemdos.txt)
      return;
    }
    fsn=-1;
  }
  if(fsn<0)
  {
    int n;
    for(n=0;n<MAX_STEMDOS_FSNEXT_STRUCTS;n++)
    {
      if(stemdos_fsnext_struct[n].dta==stemdos_dta)
      { //look for DTA match
        fsn=n;
        m68k_poke(stemdos_dta+4,BYTE(fsn));
      }
    }
    if(n>=MAX_STEMDOS_FSNEXT_STRUCTS)
    { //no match found
      Cpu.r[0]=-49;  //no more files (or is it -47? cf gemdos.txt)
      return;
    }
  }
  Tstemdos_fsnext_struct_type *find_struct=&(stemdos_fsnext_struct[fsn]);
  bool First=(find_struct->NextFile=="");
  Cpu.r[0]=DWORD(First?-33:-49);  //file not found:no more files
  bool LastFile=0;
  if(find_struct->attr==0x8)
  { //search for volume label
    m68k_poke(stemdos_dta+21,1+8); //file attributes, volume label
    m68k_poke(stemdos_dta+22,0); //file clock time, 00:00
    m68k_poke(stemdos_dta+23,0); //file clock time, 00:00
    m68k_poke(stemdos_dta+24,0); //file date, beginning of time
    m68k_poke(stemdos_dta+25,0); //file date, beginning of time
    m68k_poke(stemdos_dta+26,0); //file size, 0
    m68k_poke(stemdos_dta+27,0); //file size, 0
    m68k_poke(stemdos_dta+28,0); //file size, 0
    m68k_poke(stemdos_dta+29,0); //file size, 0
    for(int n=0;n<14;n++) 
      m68k_poke(stemdos_dta+30+n,EasyStr("STEMDISK.MNT")[n]);
    DBG_LOG("STEMDOS: Found volume label");
    LastFile=true; // Only 1 volume label (thank goodness)
    Cpu.r[0]=0;
  }
  else
  {
    DirSearch ds;
    ds.st_only=true;
    if(ds.Find(find_struct->path))
    {
      for(;;) {
        char *fname=StrUpperNoSpecial(ds.ShortName);
        // NextFile contains the name of the next matching file
        if(IsSameStr_I(find_struct->NextFile,fname)||First)
        {
          // Check if found file's attributes match what you asked for
          int that_files_attr=PCAttrToSTAttr(ds.Attrib);
          if((find_struct->attr & that_files_attr)==that_files_attr)
          {
            if(ds.Attrib & FILE_ATTRIBUTE_READONLY) 
              that_files_attr|=0x1;
            m68k_poke(stemdos_dta+21,(BYTE)that_files_attr); //file attributes
            WORD CreateDate,CreateTime;
#ifdef WIN32
            FILETIME lft;
#if 1 //from Petari
            FileTimeToLocalFileTime(&ds.LastWriteTime,&lft); // File time is always GMT
#else
            FileTimeToLocalFileTime(&ds.CreationTime,&lft); // File time is always GMT
#endif
            FileTimeToDosDateTime(&lft,&CreateDate,&CreateTime);
#endif
#ifdef UNIX
#if 1 //from Petari
            CreateDate=WORD(ds.LastWriteTime>>16);
            CreateTime=WORD(ds.LastWriteTime);
#else
            CreateDate=WORD(ds.CreationTime>>16);
            CreateTime=WORD(ds.CreationTime);
#endif
#endif
            m68k_poke(stemdos_dta+22,HIBYTE(CreateTime)); //file clock time
            m68k_poke(stemdos_dta+23,LOBYTE(CreateTime)); //file clock time
            m68k_poke(stemdos_dta+24,HIBYTE(CreateDate)); //file date
            m68k_poke(stemdos_dta+25,LOBYTE(CreateDate)); //file date
            m68k_poke(stemdos_dta+26,BYTE((ds.SizeLow&0xff000000)>>24)); //file size, high byte
            m68k_poke(stemdos_dta+27,BYTE((ds.SizeLow&0xff0000)>>16)); //file size, mid-high byte
            m68k_poke(stemdos_dta+28,BYTE((ds.SizeLow&0xff00)>>8)); //file size, mid-low byte
            m68k_poke(stemdos_dta+29,BYTE(ds.SizeLow&0xff)); //file size, low byte
            for(int n=0;n<14;n++) 
              m68k_poke(stemdos_dta+30+n,fname[n]);
            DBG_LOG(EasyStr("STEMDOS: Stemdos found file ")+fname);
            Cpu.r[0]=0; //success
            break;
          }
        }
        if(ds.Next()==0)
        {
          LastFile=true;
          break;
        }
      }
      if(LastFile==0)
      {
        // Find next matching file (for next call to fsnext)
        for(;;) {
          if(ds.Next()==0)
          {
            LastFile=true;
            break;
          }
          else
          {
            int STAttr=PCAttrToSTAttr(ds.Attrib);
            if((find_struct->attr & STAttr)==STAttr)
            {
              find_struct->NextFile=StrUpperNoSpecial(ds.ShortName);
              break;
            }
          }
        }
      }
    }
    else
      // ds.Find failed
      LastFile=true; // No files
  }
  if(Cpu.r[0]<0||LastFile)
  { // Error or finished
    find_struct->dta=0;
    find_struct->path="";
    m68k_poke(stemdos_dta+4,255); // return no more files next time you fsnext
  }
  DBG_LOG(EasyStr("STEMDOS: fsnext returned ")+Cpu.r[0]);
}


void stemdos_Dfree(int dr) {
#ifdef WIN32
  EasyStr root_path=mount_path[dr].Lefts(2);
  root_path+="\\";
  DWORD dw[4];
  GetDiskFreeSpace(root_path.Text,&(dw[3]),&(dw[2]),&(dw[0]),&(dw[1]));
  // max clusers under 0x3e88888 (about 64Mb)
  // Clusters free * sectors per cluster * bytes per sector = num bytes free
  if(DWORDLONG(dw[0])*dw[3]*dw[2]>=0x3e88888)
    dw[0]=MAX(0x3e88888/(dw[3]*dw[2]),DWORD(1));
  for(int n=0;n<4;n++) 
    m68k_lpoke(stemdos_dfree_buffer+n*4,dw[n]);
#endif
#ifdef UNIX
  DWORD free_units=65536,total_units=100000;
  DWORD bytes_per_sector=512,sectors_per_unit=2;    // 64Mb
#ifdef LINUX
  struct statfs sfs;
  if(statfs(mount_path[dr],&sfs)==0)
  {
// f_bsize;    /* optimal transfer block size */
// f_blocks;   /* total data blocks in file system */
// f_bfree;    /* free blocks in fs */
// f_bavail;   /* free blocks avail to non-superuser */
    sectors_per_unit=sfs.f_bsize/bytes_per_sector;
    total_units=sfs.f_blocks;
    free_units=sfs.f_bavail;
  }
  if(((long double)free_units) * bytes_per_sector*sectors_per_unit>=0x3e88888)
  { // 64Mb
    free_units=MAX(0x3e88888/(bytes_per_sector*sectors_per_unit),1);
  }
#endif
  m68k_lpoke(stemdos_dfree_buffer+0,free_units);
  m68k_lpoke(stemdos_dfree_buffer+4,total_units);
  m68k_lpoke(stemdos_dfree_buffer+8,bytes_per_sector);
  m68k_lpoke(stemdos_dfree_buffer+12,sectors_per_unit);
#endif
  Cpu.r[0]=0;
}


void stemdos_mkdir() {
  stemdos_get_PC_path();
  DBG_LOG(EasyStr("STEMDOS: Got the PC filename as ")+PC_filename);
  if(CreateDirectory(PC_filename,NULL)==0)
  {
    Cpu.r[0]=-34;
    WIN_ONLY(if(GetLastError()!=ERROR_PATH_NOT_FOUND) Cpu.r[0]=-36; )
  }
  else
    Cpu.r[0]=0; //succeed!
  HDDisplayTimer=timer+HD_TIMER;
}


void stemdos_rmdir() {
  stemdos_get_PC_path();
  DBG_LOG(EasyStr("STEMDOS: Got the PC filename as ")+PC_filename);
  if(RemoveDirectory(PC_filename)==0)
  {
    Cpu.r[0]=-34;
    WIN_ONLY(if(GetLastError()!=ERROR_PATH_NOT_FOUND) Cpu.r[0]=-36; )
  }
  else
    Cpu.r[0]=0; //succeed!
  HDDisplayTimer=timer+HD_TIMER;
}


void stemdos_Fdelete() {
  TRACE_LOG("Del %s\n",stemdos_filename.Text);
  stemdos_get_PC_path();
  stemdos_search_wildcard_PC_path();
  DBG_LOG(EasyStr("STEMDOS: Got the PC filename as ")+PC_filename);
  if(DeleteFile(PC_filename)==0)
  {
    Cpu.r[0]=-33;
#ifdef WIN32
    DWORD er=GetLastError();
    if(er==ERROR_PATH_NOT_FOUND)
      Cpu.r[0]=-34;
    else if(er!=ERROR_FILE_NOT_FOUND)
      Cpu.r[0]=-36;
#endif
  }
  else
    Cpu.r[0]=0; //succeed!
  HDDisplayTimer=timer+HD_TIMER;
}


void stemdos_Fattrib() {
  HDDisplayTimer=timer+HD_TIMER;
  stemdos_get_PC_path();
  stemdos_search_wildcard_PC_path();
  DBG_LOG(EasyStr("STEMDOS: Got the PC filename as ")+PC_filename);
  if(stemdos_Fattrib_flag)
  { //set attributes
    DBG_LOG("STEMDOS: Fattrib set attributes");
    if(stemdos_attr&0x8)
      //trying to change a file into a volume label, the fools!
      Cpu.r[0]=-36;
    else
    {
      DWORD win_attr=GetFileAttributes(PC_filename);
      if((!(win_attr & FILE_ATTRIBUTE_DIRECTORY))^(!(stemdos_attr&0x10)))
      { //bad!
        DBG_LOG("     trying to change attributes to/from folder - aborted");
        Cpu.r[0]=-36; //not a chance!
      }
      else
      {
        win_attr&=
          ~(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_READONLY);
        if(stemdos_attr&2)    
          win_attr|=FILE_ATTRIBUTE_HIDDEN;
        if(stemdos_attr&4)    
          win_attr|=FILE_ATTRIBUTE_SYSTEM;
        if(stemdos_attr&1)    
          win_attr|=FILE_ATTRIBUTE_READONLY;
        if(SetFileAttributes(PC_filename,win_attr)) // TODO: only on ST, normal for PC
        { //succeed
          DBG_LOG("     set new attributes");
          Cpu.r[0]=stemdos_attr;
        }
        else
        {
          DBG_LOG("     SetFileAttributes didn't work");
          Cpu.r[0]=-36; //access denied
        }
      }
    }
  }
  else
  { //read attributes
    DWORD win_attr=GetFileAttributes(PC_filename);
    if(win_attr==0xFFFFFFFF)
    {
      Cpu.r[0]=-33; //file not found
      DBG_LOG("STEMDOS: Fattrib get attributes failed");
    }
    else
    {
      Cpu.r[0]=0;
      if(win_attr & FILE_ATTRIBUTE_READONLY)
        Cpu.r[0]|=0x1;
      if(win_attr & FILE_ATTRIBUTE_SYSTEM)   
        Cpu.r[0]|=0x4;
      if(win_attr & FILE_ATTRIBUTE_HIDDEN)    
        Cpu.r[0]|=0x2;
      if(win_attr & FILE_ATTRIBUTE_DIRECTORY) 
        Cpu.r[0]|=0x10;
      DBG_LOG("STEMDOS: Fattrib get attributes succeeded");
    }
  }
}


void stemdos_rename() {
  HDDisplayTimer=timer+HD_TIMER;
  //try to rename stemdos_filename to stemdos_rename_to_filename
  StrUpperNoSpecial(stemdos_rename_to_filename.Text);
  if(stemdos_rename_to_filename[1]==':')
  {
    if(toupper(stemdos_rename_to_filename[0])!=toupper(stemdos_filename[0]))
    { //trying to rename accross drives
      Cpu.r[0]=-46;  //or rather ENSAME
      return;
    }
  }
  stemdos_get_PC_path();
  if(stemdos_search_wildcard_PC_path()==0xffffffff)
  {
    Cpu.r[0]=-33;
    return;
  }
  EasyStr f1=PC_filename;
  stemdos_filename=stemdos_rename_to_filename;
  if(stemdos_get_file_path()==STEMDOS_FILE_IS_GEMDOS)
  { // Extend to full path
    Cpu.r[0]=-46;
    return;
  }
  stemdos_get_PC_path();
  DBG_LOG(EasyStr("STEMDOS: Trying to rename ")+f1+" to "+PC_filename);
  if(Exists(PC_filename))
  {
    DBG_LOG("     dest already exists");
    Cpu.r[0]=-36;
  }
  else
  {
    if(MoveFile(f1,PC_filename))
    { //succeeded
      DBG_LOG("     succeeded!");
      Cpu.r[0]=0;
    }
    else
    { //failed
      DBG_LOG("     call to MoveFile failed");
      Cpu.r[0]=-36;
    }
  }
}


void stemdos_finished() {
  on_rte=ON_RTE_RTE;
  SR=stemdos_save_sr;  //restore status
  UPDATE_FLAGS;
  Cpu.sr_check_z_n_l_for_r0();
}


void stemdos_Pexec() {
  //called from stemdos_rte, nothing done after this fn called.
  // Stack is as for original GEMDOS call, PC is at os_gemdos_vector
  //ASSERT(stemdos_command==0x4B);
  long text,data,bss,symbol_table,fixup_ad;
  // when we come here, basepage has just been created, address
  // was returned in D0
  MEM_ADDRESS basepage=Cpu.r[0],ad,textbase; //address of basepage
  BYTE b;
  if(Cpu.r[0]<0)
  {
    DBG_LOG(EasyStr("STEMDOS: Exec returned error ")+Cpu.r[0]);
    fclose(stemdos_Pexec_file);
    stemdos_Pexec_file=NULL;
    stemdos_finished();
    stemdos_final_rte();
  }
  else
  {
    if(STfile_read_word(stemdos_Pexec_file)!=0x601a)
    { //not executable
      Cpu.r[0]=-66;  //not executable
      DBG_LOG("STEMDOS: Exec didn't find magic number in file");
      fclose(stemdos_Pexec_file);
      stemdos_Pexec_file=NULL;
      SR=stemdos_save_sr;  //restore status - we don't restore the old status after term
      UPDATE_FLAGS;
      stemdos_trap_1_Mfree(stemdos_Pexec_list[stemdos_Pexec_list_ptr]);
      stemdos_rte_action=STEMDOS_RTE_MFREE2; //correct stack after finish
    }
    else
    {
      //ss we must update fields of basepage
      text=STfile_read_long(stemdos_Pexec_file);
      data=STfile_read_long(stemdos_Pexec_file);
      bss=STfile_read_long(stemdos_Pexec_file);
      symbol_table=STfile_read_long(stemdos_Pexec_file);
      fseek(stemdos_Pexec_file,28,SEEK_SET); //seek to end of header
      DBG_LOG(EasyStr("TEXT=")+text);
      DBG_LOG(EasyStr("data=")+data);
      DBG_LOG(EasyStr("bss=")+bss);
      DBG_LOG(EasyStr("symbol table=")+symbol_table);
      log_stack;
      TRACE_LOG("Starting process basepage $%X\n",basepage);
      if((MEM_ADDRESS)(basepage+0x100UL+text+data+bss)>(MEM_ADDRESS)(m68k_lpeek(basepage+0x4)))
      { //basepage+4 contains hi-tpa
        Cpu.r[0]=-39;  //out of memory
        DBG_LOG("STEMDOS: Program too big! Out of memory.");
        TRACE_LOG("STEMDOS: Program too big! Out of memory.\n");
        TRACE_LOG("(basepage+4):%x  basepage %x + 0x100UL + text %x + data %x + bss %x = %x\n",m68k_lpeek(basepage+0x4),basepage,text,data,bss,basepage+0x100UL+text+data+bss);
        fclose(stemdos_Pexec_file);
        stemdos_Pexec_file=NULL;
        stemdos_Pterm();
        stemdos_trap_1_Mfree(stemdos_Pexec_list[stemdos_Pexec_list_ptr]);
        SR=stemdos_save_sr;  //restore status - we don't restore the old status after term
        UPDATE_FLAGS;
        stemdos_rte_action=STEMDOS_RTE_MFREE2; //correct stack after finish
      }
      else
      {
        DBG_LOG(EasyStr("basepage at ")+HEXSl(basepage,6));
        TRACE_LOG("basepage at %X",basepage);
        textbase=basepage+0x100; //start of text area
        DBG_LOG(EasyStr("text at ")+HEXSl(textbase,6));
        TRACE_LOG(" text at %X",textbase);
        ad=textbase;
        // Clear entire heap, correct? Yes
        MEM_ADDRESS lo_tpa=textbase,hi_tpa=m68k_lpeek(basepage+0x4);
        if(lo_tpa<mem_len && hi_tpa<mem_len)
        {
          int bytes=hi_tpa-lo_tpa;
          if(bytes>0)
          {
            BYTE *pFirst=lpPEEK(lo_tpa),*pLast=lpPEEK(hi_tpa-1);
            if(pLast<pFirst) pFirst=pLast;
            ZeroMemory(pLast,bytes);
          }
        }
        //set up basepage
        m68k_lpoke(basepage+0x8,ad); //start of text
        m68k_lpoke(basepage+0xc,text); //length of text
#if defined(SSE_STATS)
        Stats.nHdsector+=text/512;
#endif
        STfile_read_to_ST_memory(stemdos_Pexec_file,ad,text); //load in text segment
        ad+=text;
        DBG_LOG(EasyStr("data at ")+HEXSl(ad,6));
        TRACE_LOG(" data at %X",ad);
        m68k_lpoke(basepage+0x10,ad); //start of data
        m68k_lpoke(basepage+0x14,data); //length of data
#if defined(SSE_STATS)
        Stats.nHdsector+=data/512;
#endif
        STfile_read_to_ST_memory(stemdos_Pexec_file,ad,data); //load in data segment
        ad+=data;
        DBG_LOG(EasyStr("bss at ")+HEXSl(ad,6));
        TRACE_LOG(" bss at %X\n",ad);
        m68k_lpoke(basepage+0x18,ad); //start of bss
        m68k_lpoke(basepage+0x1c,bss); //length of bss
        // BSS already zeroed (must be in TPA!)
        fseek(stemdos_Pexec_file,symbol_table,SEEK_CUR); //seek to end of symbol table
        fixup_ad=STfile_read_long(stemdos_Pexec_file);
        if(fixup_ad && STfile_read_error==0)
        { // longs to relocate
          ad=m68k_lpeek(fixup_ad+textbase);
          m68k_lpoke(fixup_ad+textbase,ad+textbase);
          for(;;)
          {
            if(!fread(&b,1,1,stemdos_Pexec_file))
              break;
            else if(b==0)
              break;
            else if(b==1)
              fixup_ad+=254;
            else if(b&1)
            {
              fclose(stemdos_Pexec_file);
              stemdos_Pexec_file=NULL;
              stemdos_finished();
              exception(BOMBS_ADDRESS_ERROR,EA_WRITE,fixup_ad+b);
            }
            else
            { //valid!
              fixup_ad+=b;
              ad=m68k_lpeek(fixup_ad+textbase); //relocate this address
              m68k_lpoke(fixup_ad+textbase,ad+textbase);
            }
          }
        }
        fclose(stemdos_Pexec_file); //ss in Windows it doesn't stay open
        stemdos_Pexec_file=NULL;
        if(stemdos_Pexec_mode==0)
        {
          DBG_LOG("STEMDOS: Writing Pexec Go command over Load n Go");
          stemdos_add_to_Pexec_list(basepage);
          // This makes sure Steem doesn't take the Pexec mode 4 call below as a
          // different program being run.
          stemdos_ignore_next_pexec4=true;
          // Pexec call to just go, must write everything as the sp
          // might have been on the now cleared heap (Team STF demo)
          MEM_ADDRESS my_sp=get_sp_before_trap();
          m68k_dpoke(my_sp,0x4b);  // PEXEC
/*  Later Gemdos had a new Pexec mode 6 where you don't need to delete
    memory of the child process, contrary to mode 4.
    With this redmcsb should compile.
    To simplify we test TOS version.
*/
          m68k_dpoke(my_sp+2,((tos_version>=0x104)?6:4));
          m68k_lpoke(my_sp+8,basepage);
          log_stack;
          stemdos_finished();
        }
        else
        {
          DBG_LOG("STEMDOS: Load finished - Pexec was just for load");
          stemdos_finished();
          stemdos_final_rte();
        }
      }
    }
  }
}


bool stemdos_Pterm() {
  // return true if process is STEMDOS
  if(stemdos_Pexec_list_ptr) // there are processes
  {
    // close files: important because some programs forget to do it (Lattice C) 
    for(int n=6;n<=45;n++)
    {
      if(stemdos_file[n].open)
      {
        if(stemdos_file[n].owner_program==stemdos_Pexec_list_ptr)
        {
          TRACE_LOG("Pterm close file %d %s\n",n,stemdos_file[n].filename.Text);
          stemdos_close_file(&(stemdos_file[n]));
        }
      }
    }
    stemdos_Pexec_list_ptr--;
    //TRACE_LOG("PExec ptr %d -> %d\n",stemdos_Pexec_list_ptr+1,stemdos_Pexec_list_ptr);
    DBG_LOG(EasyStr("STEMDOS: Taking ")+HEXSl(stemdos_Pexec_list[stemdos_Pexec_list_ptr],6)+" from Pexec list");
    if(stemdos_Pexec_list[stemdos_Pexec_list_ptr])
    { //one of ours
      DBG_LOG("     one of ours!");
      return true; // Only Mfree if load n go
    }
  }
  return false;
}


void stemdos_add_to_Pexec_list(MEM_ADDRESS ad) {
  DBG_LOG(EasyStr("STEMDOS: Adding ")+HEXSl(ad,6)+" to Pexec list");
  ASSERT(stemdos_Pexec_list_ptr<MAX_STEMDOS_PEXEC_LIST);
  if(stemdos_Pexec_list_ptr>=MAX_STEMDOS_PEXEC_LIST)
  {
    for(int n=0;n<MAX_STEMDOS_PEXEC_LIST-1;n++)
      stemdos_Pexec_list[n]=stemdos_Pexec_list[n+1];
    stemdos_Pexec_list_ptr--;
  }
  stemdos_Pexec_list[stemdos_Pexec_list_ptr++]=ad;
  //TRACE_LOG("PExec ptr %d -> %d\n",stemdos_Pexec_list_ptr-1,stemdos_Pexec_list_ptr);
}


void stemdos_control_c() {
  //control-c pressed
  if(stemdos_command==0xa)
  { //readline
    DBG_LOG("STEMDOS: Readline active and CTRL+C pressed, quit! Calling Mfree when readline returns.");
    if(stemdos_Pterm())
    {
      on_rte=ON_RTE_STEMDOS;
      on_rte_interrupt_depth=interrupt_depth; //get RTE from current interrupt
      stemdos_rte_action=STEMDOS_RTE_MFREE; //SS only use
    }
  }
}


bool stemdos_check_mount(BYTE a) {
  if(a<26) 
    return mount_flag[a];
  return 0;
}


void stemdos_parse_path() { 
  //remove \..\ etc.
  DBG_LOG(EasyStr("STEMDOS: Parsing path ")+stemdos_filename);
  INT_PTR c=0;
  while(c<stemdos_filename.Length())
  {
    if(stemdos_filename.Mids(c,3)=="\\..")
    { //back
      INT_PTR cc=c-1;
      while(cc>=0)
      {
        if(stemdos_filename[cc]=='\\')
        { //found previous folder
          stemdos_filename.Delete(cc,c+3-cc); //remove folder and \..
          c=cc;
          cc=-99; //stop looking back
        }
        cc--;
      }
      if(cc>-99)  //didn't find a previous folder
        stemdos_filename.Delete(c,3);
    }
    else if(stemdos_filename.Mids(c,2)=="\\.")
      //refresh!
      stemdos_filename.Delete(c,2); //remove \.
    else
      c++;
  }
  if(stemdos_filename[2]!='\\') 
    stemdos_filename.Insert("\\",2);
  char *i=strchr(stemdos_filename,' ');
  if(i) 
    *i=0;  //truncate
  char *slash1,*slash2;
  slash2=stemdos_filename.Right()+1; //point to null-termination
  slash1=slash2;
  while(slash1>stemdos_filename.Text)
  {
    slash1--;
    if(*slash1=='\\')
    {  //slash!
      i=slash1+1;
      int letters=8; //8 letters for filename
      bool period=false;  //only one extension!
      while(i<slash2)
      {
        if(*i=='.')
        {
          if(period)
          { //second .
            while(slash2>i)
            { //excise to slash or null
              memmove(i,i+1,strlen(i));
              slash2--;
            }
            break;   //finished between these slashes
          }
          else
          { //first .
            period=true;
            letters=3; //3 letters for extension
            i++; //look at next letter
          }
        }
        if(letters==0)
        { //already gone past 8 letters
          while(*i!='.' && *i!='\\' && *i)
          { //remove extra characters
            memmove(i,i+1,strlen(i));
            slash2--;
          }
          letters=3;  //max 3 left for filename
          //i now points to next letter, probably .
        }
        else
        {
          letters--;
          i++; //count letter and move to next one
        }
      }
      slash2=slash1;  //look left of this slash now
    }
  }
  RemoveIllegalFromPath(stemdos_filename,true,0,'-',0);
  DBG_LOG(EasyStr("STEMDOS: Path changed to ")+stemdos_filename);
}


int stemdos_get_file_path() {
  StrUpperNoSpecial(stemdos_filename);
  if(stemdos_filename=="CON:"||stemdos_filename=="AUX:"
    ||stemdos_filename=="VID:"||stemdos_filename=="MID:"
    ||stemdos_filename=="PRN:"||stemdos_filename=="LST:"
    ||stemdos_filename=="IKB:"||stemdos_filename=="STD:")
  {
    return STEMDOS_FILE_IS_GEMDOS;
  }
  else if(stemdos_filename.NotEmpty()&&stemdos_filename[1]==':')
  {
    if(stemdos_check_mount(stemdos_filename[0]-'A'))
    {
      stemdos_parse_path();
      return STEMDOS_FILE_IS_STEMDOS;
    }
    else
      return STEMDOS_FILE_IS_GEMDOS;
  }
  else
  { //not full path
    if(stemdos_check_mount(stemdos_current_drive))
    {
      if(stemdos_filename[0]!='\\')
      {
        stemdos_filename.Insert("\\",0); //make sure it begins with a slash
        stemdos_filename.Insert(mount_gemdos_path[stemdos_current_drive],0); //put on current path
      }
      stemdos_filename.Insert(EasyStr(char('A'+stemdos_current_drive))+":",0); //and C:
      stemdos_parse_path();  //remove slash..slash
      return STEMDOS_FILE_IS_STEMDOS;
    }
    else
      return STEMDOS_FILE_IS_GEMDOS;
  }
}


void stemdos_check_paths() {
  if(stemdos_current_drive>1&&mount_flag[stemdos_current_drive]==0)
    stemdos_current_drive=0;
  for(int d=0;d<26;d++)
  {
    if(mount_flag[d])
    {
      if(mount_gemdos_path[d].NotEmpty())
      {
        DWORD Attrib=GetFileAttributes(mount_path[d]+mount_gemdos_path[d]);
        if((Attrib & FILE_ATTRIBUTE_DIRECTORY)==0||Attrib==0xffffffff)
          mount_gemdos_path[d]="";
      }
    }
  }
}


void stemdos_trap_1() {
  Cpu.ProcessingState=TMC68000::EXCEPTION;
  SET_PC(original_return_address);
  m68k_interrupt(os_gemdos_vector);
  Cpu.ProcessingState=TMC68000::NORMAL;
}


void stemdos_trap_1_Fdup() {
  TRACE_LOG("STEMDOS calls $45 Fdup\n");
  m68k_PUSH_W(3);
  m68k_PUSH_W(0x45);
  stemdos_trap_1();
}


void stemdos_trap_1_Fgetdta() {
  TRACE_LOG("STEMDOS calls $2F Fgetdta\n");
  m68k_PUSH_W(0x2f);
  stemdos_trap_1();
}


void stemdos_trap_1_Fclose(WORD h) {
  TRACE_LOG("STEMDOS calls $3E Fclose %d\n",h);
  m68k_PUSH_W(h);
  m68k_PUSH_W(0x3e);
  stemdos_trap_1();
}


void stemdos_trap_1_Pexec_basepage() {
  TRACE_LOG("STEMDOS calls $4B Pexec mode 5 %x %x\n",stemdos_Pexec_com,stemdos_Pexec_env);
  Cpu.ProcessingState=TMC68000::EXCEPTION;
  m68k_PUSH_L(stemdos_Pexec_env);
  m68k_PUSH_L(stemdos_Pexec_com);
  m68k_PUSH_L(0);
  m68k_PUSH_W(5);
  m68k_PUSH_W(0x4b);
  m68k_interrupt(os_gemdos_vector); // want to return from this interrupt into GEMDOS
  Cpu.ProcessingState=TMC68000::NORMAL;
}


void stemdos_trap_1_Mfree(MEM_ADDRESS ad) {
  TRACE_LOG("STEMDOS calls $49 Mfree(%X)\n",ad);
  m68k_PUSH_L(ad);
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  Debug.PseudoStackPush(ad);
#endif
  m68k_PUSH_W(0x49);
  Cpu.ProcessingState=TMC68000::EXCEPTION;
  m68k_interrupt(os_gemdos_vector);
  Cpu.ProcessingState=TMC68000::NORMAL;
}


// Misc 

char* StrUpperNoSpecial(char *Str) {
  int Len=(int)strlen(Str);
  for(int n=0;n<Len;n++)
  {
    if(Str[n]>32) 
      Str[n]=char(islower(Str[n])?toupper(Str[n]):Str[n]);
  }
  return Str;
}


WORD STfile_read_word(FILE*f) {
  WORD wrd;
#ifdef BIG_ENDIAN_PROCESSOR
  fread(&wrd,1,2,f);
#else
  fread((BYTE*)(&wrd)+1,1,1,f); //high byte in file ->high byte in word
  fread((BYTE*)(&wrd),1,1,f); //low byte in file ->low byte in word
#endif
  return wrd;
}


LONG STfile_read_long(FILE*f) {
  LONG lng;
#ifdef BIG_ENDIAN_PROCESSOR
  if(!fread(&lng,1,4,f)) { STfile_read_error=true;return 0; };
#else
  STfile_read_error=0;
  if(!fread((BYTE*)(&lng)+3,1,1,f))
  { //high byte in file ->high byte in word
    STfile_read_error=true;
    return 0;
  }
  if(!fread((BYTE*)(&lng)+2,1,1,f))
  { //mid-high byte in file ->mid-high byte in word
    STfile_read_error=true;
    return 0;
  }
  if(!fread((BYTE*)(&lng)+1,1,1,f))
  { //mid-low byte in file ->mid-low byte in word
    STfile_read_error=true;
    return 0;
  }
  if(!fread((BYTE*)(&lng)+0,1,1,f))
  { //low byte in file ->low byte in word
    STfile_read_error=true;
    return 0;
  }
#endif
  return lng;
}



void STfile_read_to_ST_memory(FILE*f,MEM_ADDRESS ad,int n_bytes) {
  // this is used by the Debugger and by hd_gemdos
  ASSERT(ad+n_bytes<mem_len);
  LOG_TO(LOGSECTION_STEMDOS,EasyStr("STEMDOS: Reading ")+n_bytes+" bytes into "+HEXSl(ad,6));
  BYTE buf;
  for(int n=0;n<n_bytes;n++)
  {
    fread(&buf,1,1,f);
    m68k_poke(ad,buf);
    ad++;
  }
}

#endif//DISABLE_STEMDOS
