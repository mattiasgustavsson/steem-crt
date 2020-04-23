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
FILE: loadsave_emu.cpp
DESCRIPTION: Functions to load and save emulation variables. This is mainly
for Steem's memory snapshots system.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <computer.h>
#include <draw.h>
#include <palette.h>
#include <gui.h>
#include <loadsave.h>
#include <interface_stvl.h>
#include <stjoy.h>
#include <diskman.h>
#include <translate.h>


void ReadWriteVar(void *lpVar,DWORD szVar,NOT_ONEGAME( FILE *f ) 
              ONEGAME_ONLY( BYTE* &pMem ),int LoadOrSave,int Type,int Version) {
  // v402: throw 2 (corrupt snapshot) on R/W error
  bool SaveSize;
  if(Type==0)  // Variable
    SaveSize=(Version==17);
  else if(Type==1)  // Array
    SaveSize=(Version>=3);
  else   // Struct
    SaveSize=(Version>=5);
#ifndef ONEGAME
  if(SaveSize==0) 
  {
    if(LoadOrSave==LS_SAVE)
    {
      if(fwrite(lpVar,1,szVar,f)!=szVar)
        throw 2;
    }
    else
    {
      if(fread(lpVar,1,szVar,f)!=szVar)
        throw 2;
    }
  }
  else if(Type==3)  // byte loaded/saved as int
  {
    //ASSERT(szVar==4);
    int temp=*(BYTE*)lpVar;
    if(LoadOrSave==LS_SAVE)
    {
      if(fwrite(&temp,1,szVar,f)!=szVar)
        throw 2;
    }
    else
    {
      if(fread(&temp,1,szVar,f)!=szVar)
        throw 2;
    }
    *(BYTE*)lpVar=(BYTE)temp;
  }
  else if(Type==4) // word loaded/saved as int
  {
    //ASSERT(szVar==4);
    int temp=*(WORD*)lpVar;
    if(LoadOrSave==LS_SAVE)
    {
      if(fwrite(&temp,1,szVar,f)!=szVar)
        throw 2;
    }
    else
    {
      if(fread(&temp,1,szVar,f)!=szVar)
        throw 2;
    }
    *(WORD*)lpVar=(WORD)temp;
  }
  else if(LoadOrSave==LS_SAVE)
  {
    if(fwrite(&szVar,1,sizeof(szVar),f)!=sizeof(szVar))
      throw 2;
    if(fwrite(lpVar,1,szVar,f)!=szVar)
      throw 2;
    //      log_write(Str("Block, l=")+szVar);
  }
  else
  {
    DWORD l=0;
    if(fread(&l,1,sizeof(l),f)!=sizeof(l))
      throw 2;
    if(szVar<l) // bigger on file
    {
      if(fread(lpVar,1,szVar,f)!=szVar)
        throw 2;
      fseek(f,l-szVar,SEEK_CUR); // skip rest
    }
    else
    {
      if(fread(lpVar,1,l,f)!=l)
        throw 2;
    }
  }
#else
  if(LoadOrSave==LS_LOAD) {
    BYTE *pVar=(BYTE*)lpVar;
    if(SaveSize==0) {
      for(DWORD n=0;n<szVar;n++) *(pVar++)=*(pMem++);
    }
    else {
      DWORD l=*LPDWORD(pMem);pMem+=4;
      for(DWORD n=0;n<l;n++) {
        BYTE b=*(pMem++);
        if(n<szVar) *(pVar++)=b;
      }
    }
  }
#endif
}


int ReadWriteEasyStr(EasyStr &s,NOT_ONEGAME( FILE *f ) 
                             ONEGAME_ONLY( BYTE* &pMem ),int LoadOrSave,int) {
#ifndef ONEGAME
  size_t l;
  if(LoadOrSave==LS_SAVE)
  {
    l=s.Length();
    fwrite(&l,1,sizeof(l),f);
    fwrite(s.Text,1,l,f);
  }
  else
  {
    l=(size_t)-1;
    if(fread(&l,1,sizeof(l),f)!=sizeof(l))
      throw 2;
    if(l>260) 
      throw 2; // Corrupt snapshot
    s.SetLength(l);
    if(l)
    {
      if(fread(s.Text,1,l,f)!=l)
        throw 2;
    }
  }
#else
  if (LoadOrSave==LS_LOAD){
    int l=*(int*)(pMem);pMem+=4;
    //if (l<0 || l>260) return 2; // Corrupt snapshot
    if (l<0 || l>260) throw 2; // Corrupt snapshot
    s.SetLength(l);
    char *pT=s.Text;
    for (int n=0;n<l;n++) *(pT++)=(char)*(pMem++);
  }
#endif
  return 0;
}

#define ReadWrite(var) ReadWriteVar(&(var),sizeof(var),f,LoadOrSave,0,Version)
#define ReadWriteByteAsInt(var) ReadWriteVar(&(var),sizeof(int),f,LoadOrSave,3,Version)
#define ReadWriteWordAsInt(var) ReadWriteVar(&(var),sizeof(int),f,LoadOrSave,4,Version)
#define ReadWriteArray(var) ReadWriteVar(var,sizeof(var),f,LoadOrSave,1,Version)
#define ReadWriteStruct(var) ReadWriteVar(&(var),sizeof(var),f,LoadOrSave,2,Version)
#define ReadWriteStr(s) {int i=ReadWriteEasyStr(s,f,LoadOrSave,Version);if (i) return i; }

#ifdef ONEGAME
int LoadSaveAllStuff(BYTE* &f,bool LoadOrSave,int Version,bool,int *pVerRet)
#else
int LoadSaveAllStuff(FILE *f,bool LoadOrSave,int Version,
                     bool ChangeDisksAndCart,int *pVerRet)
#endif
{
  //TRACE("LoadSaveAllStuff(%d %d %d)\n",LoadOrSave,Version,ChangeDisksAndCart);
  try { // some functions called may throw integers 1 or 2
    ONEGAME_ONLY(BYTE *pStartByte=f; )
    int dummy_int=0;
//    WORD dummy_word=0;
    BYTE dummy_byte=0;
//    bool dummy_bool=false;
    if(Version==-1)
      Version=SNAPSHOT_VERSION;
    //TRACE_INIT("%s memory snaphot V%d\n",(LoadOrSave==LS_LOAD?"Load":"Save"),Version);
    ReadWrite(Version);
    if(pVerRet)
      *pVerRet=Version;
    ReadWrite(pc);
    ReadWrite(pc_high_byte);
    ReadWriteArray(Cpu.r);
    UPDATE_SR;
    ReadWrite(SR);
    UPDATE_FLAGS;
    ReadWrite(other_sp);
    ReadWrite(vbase);
    ReadWriteArray(STpal);
    ReadWrite(interrupt_depth);
    ReadWrite(on_rte);
    ReadWrite(on_rte_interrupt_depth);
    ReadWrite(shifter_draw_pointer);
    ReadWriteByteAsInt(Glue.video_freq);
    if(Glue.video_freq>65)
      Glue.video_freq=MONO_HZ;
    ReadWriteWordAsInt(shifter_x);
    ReadWriteWordAsInt(shifter_y);
    if(Version>=61)
    {
      // recognize wrong bitness snapshot if it was recorded (since v402)
#ifdef SSE_X64
      int bitness=0x64;
#else
      int bitness=0x32;
#endif
      ReadWrite(bitness);
#ifdef SSE_X64
      switch(bitness&0xff) {
      case 0x32:
        throw 3;
      case 0x64:
        break; // OK
      default:
        Alert(T("Snapshot not recognised as Steem64 compatible"),T("Warning"),0);
      }//sw
#else
      if((bitness&0xff)==0x64)
        throw 3;
#endif
    }
    else
      ReadWrite(dummy_int); //was int shifter_scanline_width_in_bytes;
    ReadWriteByteAsInt(Mmu.linewid);
    ReadWriteByteAsInt(shifter_hscroll);
    ReadWrite(screen_res);
    ReadWrite(Mmu.MemConfig);
    ReadWriteArray(Mfp.reg);
    int dummy[4]; //was mfp_timer_precounter[4];
    ReadWriteArray(dummy);
    // Make sure saving can't affect current emulation
    int save_mfp_timer_counter[4];
    int save_mfp_timer_period[4];
    if(LoadOrSave==LS_SAVE)
    {
      memcpy(save_mfp_timer_counter,mfp_timer_counter,sizeof(mfp_timer_counter));
      memcpy(save_mfp_timer_period,mfp_timer_period,sizeof(mfp_timer_period));
      for(int n=0;n<4;n++)
        mfp_calc_timer_counter(n,ACT);
    }
    ReadWriteArray(mfp_timer_counter);
    if(LoadOrSave==LS_SAVE)
    {
      memcpy(mfp_timer_counter,save_mfp_timer_counter,sizeof(mfp_timer_counter));
      memcpy(mfp_timer_period,save_mfp_timer_period,sizeof(mfp_timer_period));
    }
    ReadWrite(mfp_gpip_no_interrupt);
    SSEConfig.ColourMonitor=((mfp_gpip_no_interrupt&MFP_GPIP_COLOUR)!=0);
    ReadWriteByteAsInt(psg_reg_select);        //4
    ReadWriteArray(psg_reg); //16
    ReadWrite(Mmu.sound_control);
    ReadWrite(ste_sound_start);
    ReadWrite(ste_sound_end);
    ReadWrite(shifter_sound_mode);
    // handle v394 snapshots, verbose but helps player
    if(LoadOrSave==LS_LOAD && Version>=42 && Version<60) //v340->394
    {
      struct IKBD_STRUCT{
        BYTE ram[128]; 
        DWORD cursor_key_joy_time[6];
        DWORD cursor_key_joy_ticks[4];
        int command_read_count,command_parameter_counter;
        int mouse_mode;
        int joy_mode;
        int abs_mouse_max_x,abs_mouse_max_y;
        int cursor_key_mouse_pulse_count_x,cursor_key_mouse_pulse_count_y;
        int relative_mouse_threshold_x,relative_mouse_threshold_y;
        int abs_mouse_scale_x,abs_mouse_scale_y;
        int abs_mouse_x,abs_mouse_y;
        int duration;
        int abs_mousek_flags;
        int psyg_hack_stage;
        int clock_vbl_count;
        int reset_121A_hack;
        int reset_0814_hack;
        int reset_1214_hack;
        int joy_packet_pos;
        int mouse_packet_pos;
        WORD load_memory_address;
        BYTE command_param[8];
        BYTE command;
        BYTE mouse_button_press_what_message;
        BYTE clock[6];
        bool mouse_upside_down;
        bool send_nothing;
        bool port_0_joy;
        bool resetting;
      };
      IKBD_STRUCT ikbd;
      ReadWriteStruct(ikbd);
      // simple but tedious - no need to copy RAM
      for(int i=0;i<6;i++) // struct -> no memcpy
        Ikbd.cursor_key_joy_time[i]=ikbd.cursor_key_joy_time[i];
      for(int i=0;i<4;i++)
        Ikbd.cursor_key_joy_ticks[4]=ikbd.cursor_key_joy_ticks[i];
      Ikbd.command_read_count=ikbd.command_read_count;
      Ikbd.command_parameter_counter=ikbd.command_parameter_counter;
      Ikbd.mouse_mode=ikbd.mouse_mode;
      Ikbd.joy_mode=ikbd.joy_mode;
      Ikbd.abs_mouse_max_x=ikbd.abs_mouse_max_x;
      Ikbd.abs_mouse_max_y=ikbd.abs_mouse_max_y;
      Ikbd.cursor_key_mouse_pulse_count_x=ikbd.cursor_key_mouse_pulse_count_x;
      Ikbd.cursor_key_mouse_pulse_count_y=ikbd.cursor_key_mouse_pulse_count_y;
      Ikbd.relative_mouse_threshold_x=ikbd.relative_mouse_threshold_x;
      Ikbd.relative_mouse_threshold_y=ikbd.relative_mouse_threshold_y;
      Ikbd.abs_mouse_scale_x=ikbd.abs_mouse_scale_x;
      Ikbd.abs_mouse_scale_y=ikbd.abs_mouse_scale_y;
      Ikbd.abs_mouse_x=ikbd.abs_mouse_x;
      Ikbd.abs_mouse_y=ikbd.abs_mouse_y;
      Ikbd.duration=ikbd.duration;
      Ikbd.abs_mousek_flags=ikbd.abs_mousek_flags;
      Ikbd.psyg_hack_stage=ikbd.psyg_hack_stage;
      Ikbd.clock_vbl_count=ikbd.clock_vbl_count;
      Ikbd.reset_121A_hack=ikbd.reset_121A_hack;
      Ikbd.reset_0814_hack=ikbd.reset_0814_hack;
      Ikbd.reset_1214_hack=ikbd.reset_1214_hack;
      Ikbd.joy_packet_pos=ikbd.joy_packet_pos;
      Ikbd.mouse_packet_pos=ikbd.mouse_packet_pos;
      Ikbd.load_memory_address=ikbd.load_memory_address;
      for(int i=0;i<8;i++)
        Ikbd.command_param[i]=ikbd.command_param[i];
      Ikbd.command=ikbd.command;
      Ikbd.mouse_button_press_what_message=ikbd.mouse_button_press_what_message;
      for(int i=0;i<6;i++)
        Ikbd.clock[i]=ikbd.clock[i];
      Ikbd.mouse_upside_down=ikbd.mouse_upside_down;
      Ikbd.send_nothing=ikbd.send_nothing;
      Ikbd.port_0_joy=ikbd.port_0_joy;
      Ikbd.resetting=ikbd.resetting;
    }
    else
      ReadWriteStruct(Ikbd); // merged
    ReadWriteArray(keyboard_buffer);
    ReadWriteWordAsInt(keyboard_buffer_length);
    if(Version<8) {
      keyboard_buffer[0]=0;
      keyboard_buffer_length=0;
    }
    ReadWrite(Dma.mcr);
    ReadWrite(Dma.sr);
    ReadWrite(dma_address);
    ReadWriteWordAsInt(Dma.Counter);
    ReadWrite(Fdc.cr);
    ReadWrite(Fdc.tr);
    ReadWrite(Fdc.sr);
    ReadWrite(Fdc.str);
    ReadWrite(Fdc.dr);
    ReadWrite(Fdc.Lines.direction); //was char fdc_last_step_inwards_flag
    BYTE floppy_head_track[2]={FloppyDrive[0].track,FloppyDrive[1].track};
    ReadWriteArray(floppy_head_track);
    FloppyDrive[0].track=floppy_head_track[0];
    FloppyDrive[1].track=floppy_head_track[1];
    ReadWriteArray(floppy_mediach);
#ifdef DISABLE_STEMDOS
    int stemdos_Pexec_list_ptr=0;
    MEM_ADDRESS stemdos_Pexec_list[76];
    ZeroMemory(stemdos_Pexec_list,sizeof(stemdos_Pexec_list));
    int stemdos_current_drive=0;
#endif
    ReadWrite(stemdos_Pexec_list_ptr);
    ReadWriteArray(stemdos_Pexec_list);
    ReadWriteByteAsInt(stemdos_current_drive);
    EasyStr NewROM=ROMFile;
    ReadWriteEasyStr(NewROM,f,LoadOrSave,Version);
    WORD NewROMVer=tos_version;
    if(Version>=7)
      ReadWrite(NewROMVer);
    else
      NewROMVer=0x701;
    //if(LoadOrSave==LS_LOAD) TRACE("Snapshot Version %d NewROMVer %x\n",Version,NewROMVer);
    if(LoadOrSave==LS_LOAD && NewROMVer<0x106&&Version<41)
      ST_MODEL=STF; //for older snapshots pre SSE
    ReadWrite(SSEConfig.bank_length[0]);
    ReadWrite(SSEConfig.bank_length[1]);
    if(LoadOrSave==LS_LOAD)
    {
      BYTE MemConf[2]={MEMCONF_512,MEMCONF_512};
      GetCurrentMemConf(MemConf);
      SSEConfig.make_Mem(MemConf[0],MemConf[1]);
    }
    EasyStr NewDiskName[2],NewDisk[2];
    if(Version>=1)
    {
      for(int disk=0;disk<2;disk++)
      {
        NewDiskName[disk]=FloppyDisk[disk].DiskName;
        ReadWriteStr(NewDiskName[disk]);
        NewDisk[disk]=FloppyDrive[disk].GetDisk();
        ReadWriteStr(NewDisk[disk]);
      }
    }
    if(Version>=2)
    {
#ifdef DISABLE_STEMDOS
      Str mount_gemdos_path[26];
#endif
      for(int n=0;n<26;n++)
        ReadWriteStr(mount_gemdos_path[n]);
#ifndef DISABLE_STEMDOS
      if(LoadOrSave==LS_LOAD)
        stemdos_check_paths();
#endif
    }
    if(Version>=4)
      ReadWriteStruct(Blitter);
    if(Version>=5)
      ReadWriteArray(ST_Key_Down);
    if(Version>=8)
      ReadWriteStruct(acia[ACIA_IKBD]);
    if(Version<44&&LoadOrSave==LS_LOAD) //v3.5.1
    {
      acia[ACIA_IKBD].cr=0x96; // usually
      acia[ACIA_IKBD].sr=2; // usually
    }
    if(Version>=9)
    {
#ifdef DISABLE_STEMDOS
      MEM_ADDRESS stemdos_dta;
#endif
      ReadWrite(stemdos_dta); //4
    }
    if(Version>=10) 
    {
      ReadWrite(ste_sound_fetch_address);    //4
      ReadWrite(next_ste_sound_end);   //4
      ReadWrite(next_ste_sound_start); //4
    }
    else if(LoadOrSave==LS_LOAD) 
    {
      next_ste_sound_end=ste_sound_end;
      next_ste_sound_start=ste_sound_start;
      ste_sound_fetch_address=ste_sound_start;
    }
    ste_sound_freq=ste_sound_mode_to_freq[shifter_sound_mode&3];
    ste_sound_output_countdown=0;
    DWORD StartOfData=0;
    NOT_ONEGAME(DWORD StartOfDataPos=ftell(f); )
    if(Version>=11) 
      ReadWrite(StartOfData);
    if(Version>=12) 
    {
      ReadWrite(os_gemdos_vector);
      ReadWrite(os_bios_vector);
      ReadWrite(os_xbios_vector);
    }
    if(Version>=13) 
      ReadWrite(paddles_ReadMask);
    EasyStr NewCart=CartFile;
    if(Version>=14) 
      ReadWriteStr(NewCart);
    if(Version>=15) 
    {
      ReadWrite(rs232_recv_byte);
      ReadWrite(rs232_recv_overrun);
      ReadWrite(rs232_bits_per_word);
      ReadWrite(rs232_hbls_per_word);
    }
    EasyStr NewDiskInZip[2];
    if(Version>=20) 
    {
      for(int disk=0;disk<2;disk++) {
        NewDiskInZip[disk]=FloppyDisk[disk].DiskInZip;
        ReadWriteStr(NewDiskInZip[disk]);
      }
    }
#ifndef ONEGAME
    bool ChangeTOS=true,ChangeCart=ChangeDisksAndCart,ChangeDisks=ChangeDisksAndCart;
#endif
    DWORD ExtraFlags=0;
    if(Version>=21) 
      ReadWrite(ExtraFlags);
#ifndef ONEGAME
    if(ExtraFlags & BIT_0) 
      ChangeDisks=0;
    // Flag here for saving disks in this file? (huge!)
    if(ExtraFlags & BIT_1) 
      ChangeTOS=0;
    // Flag here for only asking user to locate version and country code TOS?
    // Flag here for saving TOS in this file?
    if(ExtraFlags & BIT_2) 
      ChangeCart=0;
    // Flag here for saving the cart in this file?
#endif
    if(Version>=22) 
    {
#ifdef DISABLE_STEMDOS
#define MAX_STEMDOS_FSNEXT_STRUCTS 100
      struct _STEMDOS_FSNEXT_STRUCT {
        MEM_ADDRESS dta;
        EasyStr path;
        EasyStr NextFile;
        int attr;
        DWORD start_hbl;
      }stemdos_fsnext_struct[MAX_STEMDOS_FSNEXT_STRUCTS];
#endif
      int max_fsnexts=MAX_STEMDOS_FSNEXT_STRUCTS;
      ReadWrite(max_fsnexts);
      for(int n=0;n<max_fsnexts;n++) 
      {
        ReadWrite(stemdos_fsnext_struct[n].dta);
        // If this is invalid then it will just return "no more files"
        ReadWriteStr(stemdos_fsnext_struct[n].path);
        ReadWriteStr(stemdos_fsnext_struct[n].NextFile);
        ReadWrite(stemdos_fsnext_struct[n].attr);
        ReadWrite(stemdos_fsnext_struct[n].start_hbl);
      }
    }
    if(Version>=23) 
      ReadWrite(Glue.hscroll);
#ifdef NO_CRAZY_MONITOR
    int em_width=480,em_height=480,em_planes=4,extended_monitor=0,aes_calls_since_reset=0;
    long save_r[16];
    MEM_ADDRESS line_a_base=0,vdi_intout=0;
#endif
    bool old_em=(extended_monitor!=0);
    if(Version>=24) 
    {
      ReadWriteWordAsInt(em_width);
      ReadWriteWordAsInt(em_height);
      ReadWriteByteAsInt(em_planes);
      ReadWriteByteAsInt(extended_monitor);
      ReadWrite(aes_calls_since_reset);
      ReadWriteArray(save_r);
      ReadWrite(line_a_base);
      ReadWrite(vdi_intout);
      if(LoadOrSave==LS_LOAD)
        vdi_intout=line_a_base=0; //?
    }
#ifndef NO_CRAZY_MONITOR
    else if(LoadOrSave==LS_LOAD)
      extended_monitor=0;
#endif
    if(Version>=25) 
    {
      if(LoadOrSave==LS_SAVE) 
      {
        memcpy(save_mfp_timer_counter,mfp_timer_counter,sizeof(mfp_timer_counter));
        memcpy(save_mfp_timer_period,mfp_timer_period,sizeof(mfp_timer_period));
      }
      for(int n=0;n<4;n++)
      {
        BYTE prescale_ticks=0;
        if(LoadOrSave==LS_SAVE) 
          prescale_ticks=(BYTE)mfp_calc_timer_counter(n,ACT);
        ReadWrite(prescale_ticks);
      }
      if(LoadOrSave==LS_SAVE) 
      {
        memcpy(mfp_timer_counter,save_mfp_timer_counter,sizeof(mfp_timer_counter));
        memcpy(mfp_timer_period,save_mfp_timer_period,sizeof(mfp_timer_period));
      }
    }
    if(Version>=26)
      ReadWriteArray(mfp_timer_period);
    else if(LoadOrSave==LS_LOAD) 
    {
      for(int t=0;t<4;t++)
        MFP_CALC_TIMER_PERIOD(t);
    }
    if(Version>=27)
      ReadWriteArray(mfp_timer_period_change);
    else if(LoadOrSave==LS_LOAD) 
    {
      for(int t=0;t<4;t++) 
        mfp_timer_period_change[t]=0;
    }
    if(Version>=28) 
    {
      int rel_time=0;
      ReadWrite(rel_time);
    }
    if(Version>=29) 
    {
      ReadWrite(emudetect_called);
      if(LoadOrSave==LS_LOAD) 
        emudetect_init();
    }
    if(Version>=30) 
    {
      ReadWrite(Microwire.Mask);
      ReadWrite(Microwire.Data);
      ReadWriteByteAsInt(Microwire.volume);
      ReadWriteByteAsInt(Microwire.volume_l);
      ReadWriteByteAsInt(Microwire.volume_r);
      ReadWriteByteAsInt(Microwire.top_val_l);
      ReadWriteByteAsInt(Microwire.top_val_r);
      ReadWriteByteAsInt(Microwire.mixer);
    }
    int NumFloppyDrives=num_connected_floppies;
    if(Version>=31)
    {
      if(Version==60) // bug in v400-401
      {
        BYTE as_saved;
        ReadWrite(as_saved);
        NumFloppyDrives=as_saved;
      }
      else
        ReadWrite(NumFloppyDrives);
    }
    else
      NumFloppyDrives=2;
    ASSERT(NumFloppyDrives==1||NumFloppyDrives==2);
    bool spin_up=(fdc_spinning_up>0);
    if(Version>=32) 
      ReadWrite(spin_up);
    if(LoadOrSave==LS_LOAD) fdc_spinning_up=0x99;
    if(Version>=33) 
      ReadWriteByteAsInt(fdc_spinning_up);
    else if(LoadOrSave==LS_LOAD) 
      fdc_spinning_up=spin_up;
    if(Version>=34) 
      ReadWrite(emudetect_write_logs_to_printer);
    if(Version>=35) 
    {
      ReadWrite(psg_reg_data);
      ReadWriteByteAsInt(Fdc.StatusType); // floppy_type1_command_active
      ReadWriteByteAsInt(dummy_byte); //fdc_read_address_buffer_len
      BYTE fdc_read_address_buffer_fake[20];
      ReadWriteArray(fdc_read_address_buffer_fake);
      ReadWriteWordAsInt(Dma.ByteCount);
    }
    if(Version>=36) 
    {
      struct TAgenda temp_agenda[MAX_AGENDA_LENGTH];
      int temp_agenda_length=agenda_length;
      for(int i=0;i<agenda_length;i++) 
        temp_agenda[i]=agenda[i];
      if(LoadOrSave==LS_SAVE) 
      {
        // Convert vectors to indexes and hbl_counts to relative
        for(int i=0;i<temp_agenda_length;i++) 
        {
#if defined(SSE_X64)
          __int64 l=0;
          while((DWORD_PTR)(agenda_list[l])!=1) {
            if(temp_agenda[i].perform==agenda_list[l]) {
              temp_agenda[i].perform=(LPAGENDAPROC)l;
              break;
            }
            l++;
          }
          if(DWORD_PTR(agenda_list[l])==1) temp_agenda[i].perform=(LPAGENDAPROC)-1;
#else
          int l=0;
          while(DWORD(agenda_list[l])!=1) {
            if(temp_agenda[i].perform==agenda_list[l]) {
              temp_agenda[i].perform=(LPAGENDAPROC)l;
              break;
            }
            l++;
          }
          if(DWORD(agenda_list[l])==1) temp_agenda[i].perform=(LPAGENDAPROC)-1;
#endif
          temp_agenda[i].time-=hbl_count;
        }
      }
      ReadWrite(temp_agenda_length);
      for(int i=0;i<temp_agenda_length;i++)
        ReadWriteStruct(temp_agenda[i]);
      if(LoadOrSave==LS_LOAD) 
      {
        int list_len=0;
#if defined(SSE_X64)
        while(DWORD_PTR(agenda_list[++list_len])!=1);
        for(int i=0;i<temp_agenda_length;i++) {
          __int64 idx=__int64(temp_agenda[i].perform);
          if(idx>=list_len||idx<0)
            temp_agenda[i].perform=NULL;
          else
            temp_agenda[i].perform=agenda_list[idx];
        }
#else
        while(DWORD(agenda_list[++list_len])!=1);
        for(int i=0;i<temp_agenda_length;i++) {
          int idx=int(temp_agenda[i].perform);
          if(idx>=list_len||idx<0)
            temp_agenda[i].perform=NULL;
          else
            temp_agenda[i].perform=agenda_list[idx];
        }
#endif
        agenda_length=temp_agenda_length;
        for(int i=0;i<agenda_length;i++) 
          agenda[i]=temp_agenda[i];
        agenda_next_time=0xffffffff;
        if(agenda_length) 
          agenda_next_time=agenda[agenda_length-1].time;
      }
    }
    if(Version>=37) 
    {
#if defined(DISABLE_STEMDOS)
      int stemdos_intercept_datetime=0;
#endif
      ReadWrite(stemdos_intercept_datetime);
    }
    if(Version>=38) 
    {
#if defined(SSE_NO_FALCONMODE)
      ReadWrite(dummy_byte);
#else
      ReadWrite(emudetect_falcon_mode);
#endif
      ReadWrite(emudetect_falcon_mode_size);
      DWORD l=256;
      if(emudetect_called==0) 
        l=0;
      ReadWrite(l);
      for(DWORD n=0;n<l;n++)
        ReadWrite(emudetect_falcon_stpal[n]);
    }
    if(Version>=39) 
    {
      ReadWriteArray(Shifter.sound_fifo);
      ReadWriteByteAsInt(Shifter.sound_fifo_idx);
    }
    BYTE *pasti_block=NULL;
    DWORD pasti_block_len=0;
    bool pasti_old_active;
#if USE_PASTI
    pasti_old_active=pasti_active;
#else
    pasti_old_active=false;
#endif
    if(Version>=40) 
    {
#if USE_PASTI==0
      bool pasti_active=0;
#endif
      ReadWrite(pasti_active);
      ASSERT(pasti_active==0||pasti_active==1);
#if USE_PASTI
      if(hPasti==NULL) 
        pasti_active=false;
#endif
      if(LoadOrSave==LS_SAVE) 
      {
        //ask Pasti for variable block, save length as a long, followed by block
#if USE_PASTI
        if(hPasti&&(pasti_active||FloppyDrive[0].ImageType.Manager==MNGR_PASTI
          ||FloppyDrive[1].ImageType.Manager==MNGR_PASTI))
        {
          DWORD l=0;
          pastiSTATEINFO psi;
          psi.bufSize=0;
          psi.buffer=NULL;
          psi.cycles=ABSOLUTE_CPU_TIME;
          pasti->SaveState(&psi);
          l=psi.bufSize;
          BYTE*buf=new BYTE[l];
          psi.buffer=(void*)buf;
          if(pasti->SaveState(&psi))
            ReadWriteVar(buf,l,f,LS_SAVE,1,Version);
          else 
          {
            l=0;
            ReadWrite(l);
          }
          delete[]buf;
        }
        else
#endif
        {
          DWORD l=0;
          ReadWrite(l);
        }
      }
      else 
      { //load
       //read in length, read in block, pass it to pasti.
        ReadWrite(pasti_block_len);
        if(pasti_block_len) 
        { //something to load in
          if(pasti_block_len>0&&pasti_block_len<1024*1024) // avoid bad crash
          {
            pasti_block=new BYTE[pasti_block_len];
            fread(pasti_block,1,pasti_block_len,f);
#if USE_PASTI
            if(hPasti==NULL)
#endif
            {
              delete[] pasti_block;
              pasti_block=NULL;
            }
          }
        }
      }
    }
    else 
    {
#if USE_PASTI
      pasti_active=0;
#endif
    }
    if(Version>=41) // Steem 3.3
    {
      ReadWrite(ST_MODEL);
      if(LoadOrSave==LS_LOAD)
      {
//        TRACE("LOAD ST_MODEL %d\n",ST_MODEL);
        SSEConfig.SwitchSTModel(ST_MODEL);
      }
      ReadWrite(dummy_int); //dummy for former Program ID
    }
    else if(OPTION_HACKS)
      ST_MODEL=STE; // old Steem snapshots
#if SSE_VERSION>=340
    if(Version>=42) // Steem 3.4
    {
      ReadWrite(SampleRate); // global of 3rd party
      SampleRate=sound_freq;
      ReadWriteByteAsInt(Microwire.bass);
      if(Microwire.bass>=0xC)
        Microwire.bass=6;
      ReadWriteByteAsInt(Microwire.treble);
      if(Microwire.treble>=0xC)
        Microwire.treble=6;
#if defined(SSE_HD6301_LL)
/*  If it must work with ReadWrite, we must use a variable that
    can be used with sizeof, so we take on the stack.
*/
      BYTE buffer_for_hd6301[200];
      if(LoadOrSave==LS_SAVE) // 1=save
      {
        if(HD6301_OK)
          hd6301_load_save(LoadOrSave,buffer_for_hd6301);
        ReadWrite(buffer_for_hd6301); // ReadWriteArray would have been better
      }
      else // 0=load
      {
        ReadWrite(buffer_for_hd6301);
        //TRACE("%d\n",sizeof(buffer_for_hd6301));
        if(HD6301_OK)
          hd6301_load_save(LoadOrSave,buffer_for_hd6301);
      }
#endif
    }
    else
    {
#if defined(SSE_HD6301_LL)
      OPTION_C1=0;
#endif
    }
#endif
    //3.5.0: nothing special
    if(Version>=44) // Steem 3.5.1
    {
      ReadWriteStruct(Shifter); // for res & sync
      if(LoadOrSave==LS_LOAD)
      {
        if(Version<60) //4.0
          OPTION_SHIFTER_WU=SHIFTER_DEFAULT_WAKEUP;
      }
#if defined(SSE_HD6301_LL)
      WORD HD6301EMU_ON_tmp=OPTION_C1;
      ReadWrite(HD6301EMU_ON_tmp);
      OPTION_C1=HD6301EMU_ON_tmp!=0;
      if(!HD6301_OK)
        OPTION_C1=0;
#endif
      ReadWriteStruct(acia[ACIA_MIDI]);
      ReadWriteStruct(Dma); // variables already written
    }
    else
    {
      acia[ACIA_MIDI].cr=0x95; // usually
      acia[ACIA_MIDI].sr=2; // usual
    }
    if(LoadOrSave==LS_LOAD)
    {
      Glue.m_ShiftMode=Shifter.m_ShiftMode=(BYTE)screen_res;
      Glue.m_SyncMode=(BYTE)((Glue.video_freq==50)?2:0);
    }
    if(Version>=44)
    {
      int magic=123456;
      ReadWrite(magic); // Stupid!
      ASSERT(magic==123456);
    }
    if(Version>=45) //3.5.2
    {
      struct oldTSF314 {
        BYTE Id;
        BYTE ImageType;
        BYTE MotorOn;
      } oldSF314[2];
      for(BYTE drive=0;drive<2;drive++)
      {
        oldSF314[drive].Id=drive;
        ReadWriteStruct(oldSF314[drive]);
        if(LoadOrSave==LS_LOAD)
          FloppyDrive[drive].motor=(oldSF314[drive].MotorOn!=0);
      }
      if(LoadOrSave==LS_LOAD)
      {
        FloppyDrive[0].UpdateAdat(); // fixes wrong SCP timings on load snapshot
        FloppyDrive[1].UpdateAdat(); // must do after both are restored because of status bar refresh
      }
    }
    if(Version>=46) // 3.5.4
    {
      ReadWrite(OPTION_WS);
    }
    if(Version>=49) // 3.7.0
    {
#if defined(SSE_DISK_CAPS)
      // This just restore registers, not internal state. TODO
      if(LoadOrSave==LS_LOAD && CAPSIMG_OK 
        && FloppyDrive[0].ImageType.Manager==MNGR_CAPS)
      {
        Caps.WritePsgA(psg_reg[PSGR_PORT_A]);
        Caps.fdc.r_command=Fdc.cr;
        Caps.fdc.r_track=Fdc.tr;
        Caps.fdc.r_sector=Fdc.sr;
        Caps.fdc.r_data=Fdc.dr;
      }
#endif
      WORD *tmp=Psg.p_fixed_vol_3voices;
#if defined(SSE_YM2149_LL)
      Filter *tmp2=Psg.AntiAlias;
#endif
      ReadWriteStruct(Psg);
      Psg.p_fixed_vol_3voices=tmp;
      Psg.SelectedDrive=floppy_current_drive();
      Psg.SelectedSide=floppy_current_side();
#if defined(SSE_YM2149_LL)
      if(Version<56 && LoadOrSave==LS_LOAD)
        Psg.Reset(); //restore sane values
      Psg.AntiAlias=tmp2;
#endif
      ReadWriteStruct(Mfp);
      Mfp.Restore();
    }//3.7.0
    if(Version>=50) // 3.7.1
    {
      ReadWriteStruct(Fdc); // it includes cr, str... again
    }
    if(Version>=52) //380
    {
      // handle v394 snapshots
      if(LoadOrSave==LS_LOAD && Version<60) //v380->394
      {
        struct THD6301_394 {
          enum  {
            CUSTOM_PROGRAM_NONE,
            CUSTOM_PROGRAM_LOADING,
            CUSTOM_PROGRAM_LOADED,
            CUSTOM_PROGRAM_RUNNING
          }custom_program_tag;
          COUNTER_VAR ChipCycles,MouseNextTickX,MouseNextTickY;
          int MouseCyclesPerTickX, MouseCyclesPerTickY;
          short MouseVblDeltaX;
          short MouseVblDeltaY;
          BYTE Initialised;
          BYTE Crashed;
          BYTE click_x,click_y;
          BYTE rdr,rdrs,tdr,tdrs; 
        };
        THD6301_394 ikbd;
        ReadWriteStruct(ikbd);
        Ikbd.custom_program_tag=(THD6301::EProgramTag)ikbd.custom_program_tag;
        Ikbd.ChipCycles=ikbd.ChipCycles;
        Ikbd.MouseNextTickX=ikbd.MouseNextTickX;
        Ikbd.MouseNextTickY=ikbd.MouseNextTickY;
        Ikbd.MouseCyclesPerTickX=ikbd.MouseCyclesPerTickX;
        Ikbd.MouseCyclesPerTickY=ikbd.MouseCyclesPerTickY;
        Ikbd.MouseVblDeltaX=ikbd.MouseVblDeltaX;
        Ikbd.MouseVblDeltaY=ikbd.MouseVblDeltaY;
        Ikbd.Initialised=ikbd.Initialised;
        Ikbd.Crashed=ikbd.Crashed;
        Ikbd.click_x=ikbd.click_x;
        Ikbd.click_y=ikbd.click_y;
        Ikbd.rdr=ikbd.rdr;
        Ikbd.rdrs=ikbd.rdrs;
        Ikbd.tdr=ikbd.tdr;
        Ikbd.tdrs=ikbd.tdrs; 
      }
      else
      {
        struct Tdummy {
          BYTE dummy[44]; // former HD6301
        } mydummy;
        ZeroMemory(&mydummy,sizeof(Tdummy));
        ReadWriteStruct(mydummy); // registers... 
      }
    }
    if(Version>=53) //382
    { 
      // didn't work OK, we lose eclock sync on load
      ReadWrite(dummy_int);
      ReadWrite(dummy_int);
    }
    //390
//    int NewROMCountry=Tos.DefaultCountry;
    int NewROMCountry=0;
    if(Version>=54) //390
    {
      if(LoadOrSave==LS_SAVE)
        NewROMCountry=ROM_PEEK(0x1D);
      ReadWrite(NewROMCountry);
    }
    Blitter.BlitCycles=0;
    if(Version>=56) //392
    {
#if defined(SSE_MMU_MONSTER_ALT_RAM)
      ReadWrite(Mmu.MonSTerHimem);
#else
      ReadWrite(dummy_int);
#endif
    }
    if(Version>=57) //393
    {
      ReadWrite(SSEConfig.OverscanOn);
    }
    if(Version>=59) //395-400
    {
      ReadWriteStruct(MegaRtc);
      ReadWrite(Glue.gamecart);
      WORD dummy=0;
      ReadWrite(dummy); // former SSEOptions.SingleSideDriveMap, SSEOptions.FreebootDriveMap
#ifdef WIN32
      if(LoadOrSave==LS_LOAD && DiskMan.IsVisible())
      {
        InvalidateRect(GetDlgItem(DiskMan.Handle,98),NULL,FALSE);
        InvalidateRect(GetDlgItem(DiskMan.Handle,99),NULL,FALSE);
      }
#endif      
#if defined(SSE_VID_STVL1)
      // skip the pointers, don't RW scanline rendering memory
      // we also lose dbg_ vars and v402 additions
      const int stvl_skip=6*sizeof(void(*))+4*sizeof(void*);
      ReadWriteVar(&Stvl.PC_pal,sizeof(Stvl)-stvl_skip-sizeof(DWORD)*1024
        -sizeof(int)*5-sizeof(WORD)-sizeof(bool)*2,f,LoadOrSave,2,Version);
      if(LoadOrSave==LS_LOAD)
        StvlUpdate();
#endif
      if(LoadOrSave==LS_LOAD)
      {
        Glue.Update();
        update_ipl(0);
      }
/*  Now we save the current CPU time too, and assorted variables.
    To improve Resuming snapshot.
*/
      ReadWrite(cpu_timer);
      ReadWrite(time_of_event_acia);
      ReadWrite(time_of_last_hbl_interrupt);
      ReadWrite(time_of_last_vbl_interrupt);
      ReadWrite(time_of_next_timer_b);
      ReadWrite(cpu_timer_at_start_of_hbl);
      ReadWrite(tvn_latch_time);
#if USE_PASTI==0
      COUNTER_VAR pasti_update_time=0;
#endif
      ReadWrite(pasti_update_time);
      ReadWrite(act);
      ReadWrite(cpu_time_of_first_mfp_tick);
      ReadWriteArray(shifter_freq_change_time);
      ReadWriteArray(shifter_mode_change_time);
      ReadWriteArray(ipl_timing);
      ReadWriteArray(mfp_time_of_start_of_last_interrupt);
      ReadWriteArray(shifter_freq_change);
      ReadWriteArray(shifter_mode_change);
      ReadWriteArray(mfp_timer_timeout);
      ReadWrite(cpu_cycles); // int
      ReadWrite(freq_change_this_scanline);
      ReadWrite(shifter_freq_change_idx);
      ReadWrite(shifter_mode_change_idx);
      ReadWrite(video_freq_idx);
      ReadWrite(ipl_timing_index);
      ReadWriteStruct(Cpu); 
      ReadWriteStruct(Glue);
//      Glue.previous_video_freq=Glue.Freq[video_freq_idx];
      Glue.m_Status.stop_emu=0;
      ReadWriteStruct(Mmu);
      for(BYTE drive=0;drive<2;drive++)
      {
        if(LoadOrSave==LS_LOAD)
        {
          TSF314 tmp; // v402, stop loading FloppyDrive
          ReadWriteStruct(tmp);
        }
        else
          ReadWriteStruct(FloppyDrive[drive]);
      }
    }
    else
    {
      CALC_VIDEO_FREQ_IDX; // so we get correct Hz in status bar
      Cpu.eclock_sync_cycle=0;
    }//v4.0
    Glue.previous_video_freq=Glue.Freq[video_freq_idx];
    if(Version>=61) //402
    {
      ReadWrite(cpu_time_of_last_vbl); // forgotten, causing delay in ll YM emu
    }
    else
    {
      cpu_time_of_last_vbl=time_of_last_vbl_interrupt-64; // not far
    }

    
    if(Version>=62) //410
    {
#if defined(SSE_MEGASTE)
      MEM_ADDRESS *pAdlist=MegaSte.MemCache.pAdlist; // save pointers
      bool *pIsCached=MegaSte.MemCache.pIsCached;
      ReadWriteStruct(MegaSte);
      MegaSte.MemCache.pAdlist=pAdlist;
      MegaSte.MemCache.pIsCached=pIsCached;
#endif
    }
    // End of data, seek to compressed memory
    if(Version>=11) 
    {
#ifndef ONEGAME
      if(LoadOrSave==LS_SAVE) 
      {
        StartOfData=ftell(f);
        fseek(f,StartOfDataPos,SEEK_SET);
        ReadWrite(StartOfData);
      }
      // Seek to start of compressed data (this was loaded earlier if LS_LOAD)
      fseek(f,StartOfData,SEEK_SET);
#else
      f=pStartByte+StartOfData;
#endif
    }
    if(LoadOrSave==LS_SAVE) 
      return 0;
    init_screen();
    //if(bool(extended_monitor)!=old_em||extended_monitor)
    if(old_em||extended_monitor)
    {
      if(FullScreen)
        change_fullscreen_display_mode(true);
      else
        Disp.ScreenChange(); // For extended monitor
    }
#ifndef ONEGAME
    if(ChangeTOS) 
      LoadSnapShotChangeTOS(NewROM,NewROMVer,NewROMCountry);
    if(ChangeDisks) 
      LoadSnapShotChangeDisks(NewDisk,NewDiskInZip,NewDiskName);
    if(ChangeCart) 
      LoadSnapShotChangeCart(NewCart);
    LoadSaveChangeNumFloppies(NumFloppyDrives);
#endif
#if USE_PASTI
    if(hPasti && pasti_block) 
    {
      pastiSTATEINFO psi;
      psi.bufSize=pasti_block_len;
      psi.buffer=pasti_block;
      psi.cycles=ABSOLUTE_CPU_TIME;
      pasti->LoadState(&psi);
    }
    if(pasti_active!=pasti_old_active) 
      LoadSavePastiActiveChange();
#endif
#ifdef WIN32
    if(FullScreen)
      InvalidateRect(StemWin,NULL,false); // erase pic we just drew...
#endif      
  }
  catch(int error) {
    TRACE_INIT("snapshot error %d %d\n",error,LoadOrSave);
    return error;
  }
  catch(...) {
    TRACE_INIT("System exception in LoadSaveAllStuff\n");
    return -1;
  }
  return 0; // OK
}
#undef ReadWrite
#undef ReadWritePtr
#undef ReadWriteStruct
#undef ReadWriteByteAsInt
#undef ReadWriteWordAsInt
#undef ReadWriteArray
#undef ReadWriteStr


void LoadSnapShotUpdateVars(int Version) {
  SET_PC(pc);
  if(Version>=59) //395-400
  {
    // some necessary parts of init_timings()
    screen_res_at_start_of_vbl=screen_res;
    video_freq_at_start_of_vbl=Glue.video_freq;
    scanline_time_in_cpu_cycles_at_start_of_vbl=scanline_time_in_cpu_cycles[video_freq_idx];
    MFP_CALC_INTERRUPTS_ENABLED;
    MFP_CALC_TIMERS_ENABLED;
    prepare_next_event();
  }
  else
    init_timings();
  UpdateSTKeys();
  if(Version<36)
  {
    // No agendas saved
    if(Ikbd.resetting) 
      ikbd_reset(0);
    if(Ikbd.mouse_mode==IKBD_MOUSE_MODE_OFF) 
      Ikbd.port_0_joy=1;
    if(keyboard_buffer_length) 
      agenda_add(agenda_keyboard_replace,
        ACIAClockToHBLS(acia[ACIA_IKBD].clock_divide)+1,0);
    if(MIDIPort.AreBytesToCome()) 
      agenda_add(agenda_midi_replace,
        ACIAClockToHBLS(acia[ACIA_MIDI].clock_divide,true)+1,0);
    if(floppy_irq_flag==FLOPPY_IRQ_YES||floppy_irq_flag==FLOPPY_IRQ_ONESEC)
      agenda_add(agenda_fdc_finished,milliseconds_to_hbl(2),0);
    if(fdc_spinning_up) agenda_add(agenda_fdc_spun_up,milliseconds_to_hbl(40),
      fdc_spinning_up==2);
    if(acia[ACIA_MIDI].tx_flag) 
      agenda_add(agenda_acia_tx_delay_MIDI,2,0);
    if(acia[ACIA_IKBD].tx_flag) 
      agenda_add(agenda_acia_tx_delay_IKBD,2,0);
  }
#if USE_PASTI
  if(hPasti) 
  {
    pastiPEEKINFO ppi;
    pasti->Peek(&ppi);
    if(ppi.intrqState)
      Mfp.reg[MFPR_GPIP]&=~(1 << MFP_GPIP_FDC_BIT);
    else
      Mfp.reg[MFPR_GPIP]|=(1 << MFP_GPIP_FDC_BIT);
    pasti_motor_proc(ppi.motorOn);
  }
#endif
  /////ste_sound_on_this_screen=1;
  ste_sound_output_countdown=0;
  ste_sound_samples_countdown=0;
  ste_sound_channel_buf_idx=0;
  prepare_next_event();
  disable_input_vbl_count=0;
  snapshot_loaded=true;
  res_change();
  palette_convert_all();
  draw(false);
  for(int n=0;n<16;n++) 
    PAL_DPEEK(n*2)=STpal[n];
}
