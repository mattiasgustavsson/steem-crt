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

DOMAIN: OS
FILE: tos.cpp
DESCRIPTION: TOS (The Operating System) utilities, loading TOS images 
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <computer.h>
#include <gui.h>
#include <translate.h>

BYTE *STRom=NULL;  
BYTE *Rom_End,*Rom_End_minus_1,*Rom_End_minus_2,*Rom_End_minus_4;
BYTE *cart=NULL,*cart_save=NULL;
BYTE *Cart_End_minus_1,*Cart_End_minus_2,*Cart_End_minus_4;
unsigned long tos_len;
MEM_ADDRESS rom_addr,rom_addr_end;
// init at first trap:
MEM_ADDRESS os_gemdos_vector=0,os_bios_vector=0,os_xbios_vector=0;
WORD tos_version;
BYTE snapshot_loaded=0;
bool tos_high;


EasyStr TTos::GetNextTos(DirSearch &ds) { // to enumerate TOS files
  // refactoring to avoid duplication, code was originally in options.cpp
  EasyStr Path;
  if((ds.Attrib & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN))==0)
  {
    Path=RunDir+SLASH+ds.Name;
#ifdef WIN32
    if(has_extension(Path,"LNK"))
    {
      WIN32_FIND_DATA wfd;
      EasyStr DestPath=GetLinkDest(Path,&wfd);
      if(has_extension_list(DestPath,"IMG","ROM",NULL))
        if(Exists(DestPath)) 
          Path=DestPath;
    }
#endif
#ifdef UNIX
    char LinkPath[MAX_PATH+1];
    memset(LinkPath,0,MAX_PATH+1);
    if(readlink(Path,LinkPath,MAX_PATH)>0)
    {
      if(has_extension_list(LinkPath,"IMG","ROM",NULL))
      {
        if(Exists(LinkPath))
          Path=LinkPath;
        else
          Path="";
      }
    }
#endif
  }
  return Path;
}


void TTos::GetTosProperties(EasyStr Path,WORD &Ver,BYTE &Country,WORD &Date,
                                BYTE &Recognised) {
  FILE *f=fopen(Path,"rb");
  if(f)
  {
    fseek(f,2,SEEK_SET);
    BYTE b_high,b_low;
    fread(&b_high,1,1,f);fread(&b_low,1,1,f);
    Ver=MAKEWORD(b_low,b_high);
    fseek(f,0x1d,SEEK_SET);
    fread(&Country,1,1,f);
    fseek(f,0x1e,SEEK_SET);
    fread(&b_high,1,1,f);fread(&b_low,1,1,f);
    Date=MAKEWORD(b_low,b_high);
    DWORD Len=GetFileLength(f),checksum=0;
    fseek(f,0,SEEK_SET);
    for(DWORD m=0;m<Len;m++)
      checksum+=(BYTE)fgetc(f);
    switch(checksum) {
    case 0xFECDEE: // 1.0 UK
    case 0x104157E: // 1.02 UK
    case 0x10576E0: // 1.04 UK
    case 0x1ED55B5: // 1.06 UK
    case 0x1ECC678: // 1.62 UK
      // ... could add some
      Recognised=1; // will display in green
      break;
    case 0x1ECC698: // 1.62 UK
      Recognised=0xff; // bad in red
      break;
    default:
      if(Len%1024)
        Recognised=0xff; // bad
      else
      Recognised=0;
    }
    //TRACE_INIT("TOS v%X country %X date %X path %s\n",Ver,Country,Date,Path.Text);
    fclose(f);
  }
}


#if defined(SSE_TOS_KEYBOARD_CLICK)

void TTos::CheckKeyboardClick() {
  if(LPEEK(0x44E)==vbase) // not perfect, at least it's a check //v402 placed here
  {
    if(OPTION_KEYBOARD_CLICK)
      PEEK(0x484)|=0x01;
    else
      PEEK(0x484)&=0xFE;
  }
}

#endif


void TTos::HackMemoryForExtendedMonitor() {
  TRACE_INIT("EM mem_len %X vbase %X phystop %X _memtop %X\n",mem_len,vbase,LPEEK(0x42E),LPEEK(0x436));
  MEM_ADDRESS bytes_needed=MAX((int)((em_width*em_height*em_planes)/8),0x8000);
  //ASSERT(bytes_needed>0x8000);
  MEM_ADDRESS xbios2a=mem_len-(bytes_needed+256);
#if defined(SSE_TOS_GEMDOS_EM_382)
  MEM_ADDRESS xbios2b=(xbios2a+255)&-256;
  if(xbios2b+bytes_needed<mem_len) // should add minimum
    vbase=xbios2b;
#else
  vbase=(xbios2a+255)&-256;
#endif
  //ASSERT(vbase+bytes_needed<=mem_len);
#if defined(SSE_TOS_GEMDOS_EM_382)
  LPEEK(0x436)=vbase; //_memtop
#else
  m68k_lpoke(0x436,vbase); //_memtop
#endif
  LPEEK(SV_v_bas_ad)=vbase;
  LPEEK(SVscreenpt)=vbase;
  if(em_planes==1)
    Mfp.reg[MFPR_GPIP]|=0x80;
  TRACE_INIT("EM bytes_needed %d vbase %X phystop %X _memtop %X\n",bytes_needed,vbase,LPEEK(0x42E),LPEEK(0x436));
}


void TTos::CheckSTTypeAndTos() {
  if(!OPTION_WARNINGS)
    return;
  // each warning only once per session, one warning at a time
  // we use bits 1-... of OPTION_WARNINGS as mask
  EasyStr warning=T("Warning");
  int a=MB_OK|MB_ICONWARNING;
  if(!(OPTION_WARNINGS&(1<<1)) && (tos_version<0x106&&IS_STE
    ||tos_version>=0x106&&IS_STF && tos_version!=0x206))
  {
    OPTION_WARNINGS|=(1<<1); 
    Alert(T("TOS and ST type normally not compatible"),warning,a);
  }
  // we check more than TOS, now player has a good reason to disable the feature
  else if(!(OPTION_WARNINGS&(1<<3)) && DONGLE_ID)
  {
    Alert(T("A dongle is inserted"),warning,a);
    OPTION_WARNINGS|=(1<<3);
  }
  else if(!(OPTION_WARNINGS&(1<<4)) && !OPTION_C1)
  {
    Alert(T("Keyboard option Low-level emulation (C1) not active"),warning,a);
    OPTION_WARNINGS|=(1<<4);
  }
  else if(!(OPTION_WARNINGS&(1<<7)) && Disp.Method==DISPMETHOD_GDI)
  {
    Alert(T("No DirectX Graphics"),warning,a);
    OPTION_WARNINGS|=(1<<7);
  }
}


#define LOGSECTION LOGSECTION_TRAP

#define INVALID_SP (mem_len-128)

// This always returns a valid address
MEM_ADDRESS get_sp_before_trap(bool *pInvalid) {
  MEM_ADDRESS my_sp=(sp&0xffffff)+6;
  // my_sp now points to first byte after the exception stack frame
  if(my_sp<mem_len)
  {
    // First byte on stack is high byte of sr
    if((PEEK(areg[7]&0xffffff) & BIT_5)==0)
      // Supervisor bit not set in stacked sr
      my_sp=(other_sp&0xffffff);
  }
  if(my_sp>=mem_len)
  {
    if(pInvalid)
      *pInvalid=true;
    return INVALID_SP;
  }
  return my_sp;
}


void intercept_os() {
  ioaccess&=~IOACCESS_INTERCEPT_OS;
  if(pc==os_gemdos_vector)
  {
    intercept_gemdos();
    ioaccess|=IOACCESS_INTERCEPT_OS;
  }
  else if(pc==os_bios_vector)
  {
    intercept_bios();
    ioaccess|=IOACCESS_INTERCEPT_OS;
  }
  else if(pc==os_xbios_vector)
  {
    intercept_xbios();
    ioaccess|=IOACCESS_INTERCEPT_OS;
  }
#ifdef DEBUG_BUILD__ //was for specific use, could be developed
  else if(IRD==0x4E42)
  {
    if(Cpu.r[0]==0x73) // vdi
    {
/*
_contrl: ds.w 12
_intin: ds.w 128
_ptsin: ds.w 256
_intout: ds.w 128
_ptsout: ds.w 256
_VDIpb: dc.l _contrl, _intin, _ptsin
*/
      WORD vdi_op=d2_dpeek(d2_lpeek(Cpu.r[1]));
      TRACE_LOG("Trap #%d D1 %X function %d\n",(IRD&15),Cpu.r[1],vdi_op);
      switch(vdi_op) {
      case 124:
/*
vq_mouse()
contrl[0] = 124;
contrl[1] = contrl[3] = 0;
contrl[6] = handle;
vdi();
*mb = intout[0];
*mx = ptsout[0];
*my = ptsout[1];
*/
        TRACE_LOG(" button %X x %X y %X\n",d2_lpeek(Cpu.r[1]+12),d2_lpeek(Cpu.r[1]+16),d2_lpeek(Cpu.r[1]+16)+2);
        break;
      }
    }
    else
    {
    }
  }
#endif
#ifdef SSE_DEBUG
  else if(IRD>=0x4E40&&IRD<=0x4E4F)
  {
    TRACE_LOG("Trap #%d %x\n",(IRD&15),pc);
  }
#endif
  if(extended_monitor) 
  {
    if((IRD&0xffff)==0x4e42) 
    { //instruction is TRAP #2 (VDI or AES) (Megar)
#ifdef ENABLE_LOGFILE
      if(logsection_enabled[LOGSECTION_TRAP]) 
        log_os_call(2);
#endif
      if(Cpu.r[0]==0x73) 
      { //vdi
        MEM_ADDRESS adddd=m68k_lpeek(Cpu.r[1]); //r[1] has vdi parameter block.  
        if(m68k_dpeek(adddd+0)==1) //adddd points to the control array
        { //v_opnwk OPCODE
#if defined(SSE_STATS)
          Stats.nVdii++;
#endif
          on_rte=ON_RTE_EMHACK;
          on_rte_interrupt_depth=interrupt_depth;
          vdi_intout=m68k_lpeek(Cpu.r[1]+12);
        }
      }
    }
    ioaccess|=IOACCESS_INTERCEPT_OS;
  }
}


void intercept_gemdos() {
#ifdef ENABLE_LOGFILE
  if(logsection_enabled[LOGSECTION_TRAP])
  {
    log_os_call(1);
  }
#endif
#ifndef DISABLE_STEMDOS
  stemdos_intercept_trap_1();
#endif
}


void intercept_bios() {
  bool Invalid=0;
  MEM_ADDRESS my_sp=get_sp_before_trap(&Invalid);
  if(Invalid)
    return;
  WORD func=m68k_dpeek(my_sp);
#ifdef ENABLE_LOGFILE
  if(logsection_enabled[LOGSECTION_TRAP] && func!=1) 
    log_os_call(13);
#endif
  if(func==10)
  { // Drvbits - Drvmap()
    if(disable_input_vbl_count>30)
      disable_input_vbl_count=0; // If TOS is making BIOS calls then the IKBD int is ready
#ifndef DISABLE_STEMDOS
    stemdos_update_drvbits();  //in case it's BIOS(10)
#endif
  }
#ifndef DISABLE_STEMDOS
  else if(func==7)
  {
    WORD d=m68k_dpeek(my_sp+2);
    if(d>=2)
    {
      if(stemdos_check_mount((BYTE)d))
      {
        m68k_dpoke(my_sp+2,0); // Make it get the BPB of A: instead, this might contain nonsense!
#if defined(SSE_STATS)
        Stats.nBios++;
#endif
      }
    }
  }
  else if(func==9)
  { // Mediach
    WORD d=m68k_dpeek(my_sp+2);
    if(d>=2)
    {
      if(stemdos_check_mount((BYTE)d))
      {
        Cpu.r[0]=0; // Hasn't changed - everything ignores this anyway
        m68kPerformRte();  //don't need to check interrupts because sr won't actually have changed
#if defined(SSE_STATS)
        Stats.nBios++;
#endif
      }
    }
  }
#endif
}


void intercept_xbios() {
  bool Invalid=0;
  MEM_ADDRESS my_sp=get_sp_before_trap(&Invalid);
  if(Invalid)
    return;
#ifdef ENABLE_LOGFILE
  if(logsection_enabled[LOGSECTION_TRAP])
    log_os_call(14);
#endif
  if(OPTION_EMU_DETECT && m68k_dpeek(my_sp)==37
    &&Cpu.r[7]==0x456d753f&&Cpu.r[6]==Cpu.r[7])
  { // Vsync with Emu? in D6, D7 ->emudetect on
    Cpu.r[6]=0x53544565;
    Cpu.r[7]=0x6d456e67;
    areg[0]=0xffc100;
    emudetect_called=true;
    emudetect_init();
#if defined(SSE_STATS)
    Stats.mskSpecial|=Stats.EMU_DETECT;
#endif
    m68kPerformRte();  //don't need to check interrupts because sr won't actually have changed
  }
#if !(defined(DISABLE_STEMDOS))
  else if(m68k_dpeek(my_sp)==23&&stemdos_intercept_datetime && OPTION_RTC_HACK)
  { // Get clock time
    time_t t=time(NULL);
    struct tm *lpTime=localtime(&t);
    Cpu.r[0]=TMToDOSDateTime(lpTime);
    m68kPerformRte();  //don't need to check interrupts because sr won't actually have changed
#if defined(SSE_STATS)
    Stats.nXbios++;
#endif
  }
#endif
//#define SSE_TOS_RANDOM //
#if defined(SSE_TOS_RANDOM)
  //Random() returns a 24 bit random number
  else if(m68k_dpeek(sp)==17&&OPTION_HACKS)
  {
#ifdef SSE_BETA
    TRACE_OSD("RND");
    r[0]=rand()&0xffffff;
    m68kPerformRte();  //don't need to check interrupts because sr won't actually have changed
#if defined(SSE_STATS)
    Stats.nXbios++;
#endif
#endif
  }
#endif
}


#ifndef ONEGAME

bool load_cart(char *filename) {
/*  Loading a ROM cartridge.
    Steem original format STC has 4 null bytes at the start, then the 128 KB
    of the cartridge.
    Now we accept files where there are no extra null bytes
    We also accept 64KB cartridges (like test kits).
    We recognise the MV16 and RP16 sound cartridges (our custom, fake dumps).
*/
  bool failed=false; // return true on failure (!)
  SSEConfig.mv16=SSEConfig.mr16=false;
  FILE *f=fopen(filename,"rb");
  if(f==NULL)
    failed=true;
  else
  {
    long FileLen=GetFileLength(f); //can be 64KB, 128KB, 128KB+4bytes
    DWORD FirstBytes;
    int offset=0;
    switch(FileLen) {
    case 64*1024:
      offset=FileLen+4;
      break;
    case 128*1024:
      offset=4;
      break;
    case 128*1024+4: // Steem original
      fread(&FirstBytes,4,1,f);
      if(FirstBytes) // must be 0, don't ask why
        failed=true;
      break;
    default:
      failed=true;
    }
    if(!failed)
    {
      fread(&FirstBytes,4,1,f);
#if defined(SSE_SOUND_CARTRIDGE)
      if(FirstBytes==0x3631564D) // "MV16"
        SSEConfig.mv16=true;
      else if(FirstBytes==0x3631524D) // "MR16"
        SSEConfig.mv16=SSEConfig.mr16=true;
#endif
      if(cart_save)
        cart=cart_save;
      cart_save=NULL;
      if(cart)
        delete[] cart;
      cart=new BYTE[128*1024]; //TODO: 64K? but more code vs rarely used memory
      memset(cart,0xFF,128*1024);
      long Len=FileLen-4;
      fseek(f,-4,SEEK_CUR); //hehe
      int checksum=0;
      for(int bn=Len-1;bn>=0;bn--)
      {
        fread(cart+bn+offset,1,1,f); // backwards
        checksum+=*(cart+bn+offset);
      }
      Cart_End_minus_1=cart+(128*1024-1);
      Cart_End_minus_2=Cart_End_minus_1-1;
      Cart_End_minus_4=Cart_End_minus_1-3;
      fclose(f);
    }
  }
  return failed;
}


MEM_ADDRESS get_TOS_address(char *File) {
  if(File[0]==0) 
    return 0;
  FILE *f=fopen(File,"rb");
  if(f==NULL) 
    return 0;
  BYTE HiHi=0,LoHi=0,HiLo=0,LoLo=0;
  fread(&HiLo,1,1,f);
  fread(&LoLo,1,1,f);
  if(HiLo==0x60&&LoLo==0x06) 
  { // Pre-tos machines, need boot disk, no header
    fclose(f);
    return 0xfc0000;
  }
  else
  {
    fseek(f,8,SEEK_SET);
    fread(&HiHi,1,1,f);
    fread(&LoHi,1,1,f);
    fread(&HiLo,1,1,f);
    fread(&LoLo,1,1,f);
    fclose(f);
    MEM_ADDRESS new_rom_addr
      =MAKELONG(MAKEWORD(LoLo,HiLo),MAKEWORD(LoHi,HiHi))&0xffffff;
    if(new_rom_addr==0xfc0000) 
      return 0xfc0000;
    if(new_rom_addr==0xe00000) 
      return 0xe00000;
  }
  return 0;
}


bool load_TOS(char *File) {
  if(File[0]==0) 
    return true;
  MEM_ADDRESS new_rom_addr=get_TOS_address(File);
  FILE *f=fopen(File,"rb");
  if(f==NULL) // for example loading alien snapshot
    return true;
  DWORD Len=(GetFileLength(f)/1024)*1024;
  //ASSERT(STRom || !tos_len);
  if(Len!=tos_len || !STRom)
  {
    if(STRom)
      delete[] STRom;
    STRom=new BYTE[Len];
    tos_len=Len;
  }
#ifdef DEBUG_BUILD
  if(new_rom_addr==0xfc0000)
    tos_high=true;
#endif
  rom_addr=new_rom_addr;
  rom_addr_end=rom_addr+tos_len;
  Rom_End=STRom+tos_len;
  Rom_End_minus_1=Rom_End-1;
  Rom_End_minus_2=Rom_End-2;
  Rom_End_minus_4=Rom_End-4;
  memset(STRom,0xff,Len);
  DWORD checksum=0;
  for(DWORD m=0;m<Len;m++) 
  {
    ROM_PEEK(m)=(BYTE)fgetc(f);
    checksum+=ROM_PEEK(m);
  }
  fclose(f);
  tos_version=ROM_DPEEK(2);
  SSEConfig.TosLanguage=ROM_PEEK(0x1D);
  SSEConfig.SwitchSTModel(ST_MODEL); // to adapt CPU clock
  TRACE2("Load TOS %s v%x c%d checksum %X\n",File,tos_version,SSEConfig.TosLanguage,checksum);
/*  If we don't need ACSI hard drive emulation, neutralise TOS check for
    a faster boot (from Hatari)*/
  if(OPTION_HACKS && (tos_version==0x106||tos_version==0x162)
    && !pasti_active  && !ACSI_EMU_ON)
  {
    TRACE_INIT("STE tos boot patch %X %X %X\n",0x576,ROM_LPEEK(0x576),0x4E714E71);
    ROM_LPEEK(0x576)=0x4E714E71; // bsr +$e4 -> nop, "dma boot"
  }
#if defined(SSE_MEGASTE)
  // nuke hd spinup delay of T205, on T206, there's a time bar so it's not useful
  MEM_ADDRESS ta=0x0007A0;
  if(OPTION_HACKS && tos_version==0x205 && ROM_DPEEK(ta)==0x631E)
  {
    TRACE_INIT("MSTE tos boot patch %X %X %X\n",ta,ROM_DPEEK(ta),0x631E);
    ROM_DPEEK(ta)=0x601E; 
  }
#endif
  return 0;
}

#else

bool load_TOS(char *) {
  tos_len=192*1024;
  tos_high=true;
  rom_addr=0xFC0000;
  Rom_End=STRom+tos_len;
  Rom_End_minus_1=Rom_End-1;
  Rom_End_minus_2=Rom_End-2;
  Rom_End_minus_4=Rom_End-4;
  tos_version=0x0102;
  return 0;
}

#endif


void GetTOSKeyTableAddresses(MEM_ADDRESS *lpUnshiftTable,
                             MEM_ADDRESS *lpShiftTable) {
  MEM_ADDRESS addr=0;
  while(addr<tos_len)
  {
    if(ROM_PEEK(addr++)=='u')
    {
      if(ROM_PEEK(addr)=='i')
      {
        addr++;
        if(ROM_PEEK(addr)=='o')
        {
          addr++;
          if(ROM_PEEK(addr)=='p')
          {
            *lpUnshiftTable=addr-25;
            break;
          }
        }
      }
    }
  }
  addr=(*lpUnshiftTable)+127;
  while(addr<tos_len)
  {
    if(ROM_PEEK(addr++)==27)
    {
      *lpShiftTable=addr-2;
      break;
    }
  }
}
