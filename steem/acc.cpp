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

DOMAIN: Various
FILE: acc.cpp
DESCRIPTION: Completely random accessory functions.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop
#include <acc.h>
#include <computer.h>
#include <palette.h>
#include <mymisc.h>
#include <debugger.h>
#include <gui.h>
#include <iolist.h>

#ifdef ENABLE_LOGFILE
/*  Logging is a separate function from the Debugger in theory, just
    like the TRACE function. That's why those definitions are here.
    Haven't tried to define it in the release build.
*/

bool logging_suspended=true;
bool logsection_enabled[100];
void log_io_write(MEM_ADDRESS,BYTE);
int log_cpu_count=0;

struct_logsection logsections[NUM_LOGSECTIONS+8]={{"Always",LOGSECTION_ALWAYS},
  {"Video",LOGSECTION_VIDEO},{"MFP",LOGSECTION_MFP_TIMERS},
  {"Interrupts",LOGSECTION_INTERRUPTS},{"IO",LOGSECTION_IO},
  {"CPU",LOGSECTION_CPU},{"Trace",LOGSECTION_TRACE},{"Crash",LOGSECTION_CRASH},
  {"-",-1},
  {"FDC",LOGSECTION_FDC},{"DMA",LOGSECTION_DMA},
  {"Image info",LOGSECTION_IMAGE_INFO},
  {"Pasti",LOGSECTION_PASTI},
  {"-",-1},
  {"MMU",LOGSECTION_MMU},{"Blitter",LOGSECTION_BLITTER},
  {"ACIA",LOGSECTION_ACIA},{"IKBD (6301)",LOGSECTION_IKBD},
  {"MIDI",LOGSECTION_MIDI},{"-",-1},
  {"Gemdos",LOGSECTION_STEMDOS},{"Trap",LOGSECTION_TRAP},
  {"Cartridge",LOGSECTION_CARTRIDGE},
  {"Speed Limiting",LOGSECTION_SPEEDLIMIT},
  {"Init",LOGSECTION_INIT},{"Shutdown",LOGSECTION_SHUTDOWN},
  {"INI File",LOGSECTION_INIFILE},{"GUI",LOGSECTION_GUI},
  {"Options",LOGSECTION_OPTIONS},
  {"Tasks",LOGSECTION_AGENDA},
  {"Video rendering",LOGSECTION_VIDEO_RENDERING},{"Sound",LOGSECTION_SOUND},
  {"*",-1}};

const char *name_of_mfp_interrupt[22]={"Centronics","RS232 DCD","RS232 CTS","Blitter",
  "Timer D","Timer C","ACIAs","FDC","Timer B","RS232 TX Error","RS232 RX Buf Empty",
  "RS232 RX Error","RS232 RX Buf Full","Timer A","RS232 Ring Detector","Mono Monitor",
  "Spurious","HBL","VBL","Line-A","Line-F","Trap"};

//////////////////////////////// names of OS calls //////////////////////////////////
const char* gemdos_calls[0x58]={"Pterm0","Conin","Conout(c=&)","Cauxin","Cauxout (c=&)",
  "Cprnout(c=&)","Raw con io Crawio(c=&)","Crawcin","Cnecic","Print line(text=%)",
  "ReadLine(buf=%)","Constat","","","SetDrv(drv=&)","","Conout stat","Printer status",
  "inp?(serial)","out?(serial)","","","","","","GetDrv","SetDTA(buf=%)","","","","","",
  "super(%)","","","","","","","","","","GetDate","SetDate(date=&)","Gettime",
  "Settime(time=&)","","GetDTA","Get version number","PtermRes(keepcnt=%,retcode=&)",
  "","","","","Dfree(buf=%,drive=&)","","","Mkdir(path=%=$)","Rmdir(path=%=$)",
  "Chdir(path=%)","Fcreate(fname=%=$,attr=&)","Fopen(fname=%=$,mode=&)","Fclose(handle=&)",
  "Fread(handle=&,count=%,buf=%)","Fwrite(handle=&,count=%,buf=%)","Fdelete(fname=%=$)",
  "Fseek(offest=%,handle=&,seekmode=&)","Fattrib(fname=%=$,flag=&,attrib=&)",
  "","Fdup(handle=&)","Fforce(stdh=&,nonstdh=&)","DgetPath(buf=%,drive=&)",
  "Malloc(%)","Mfree(addr=%)","Mshrink(dummy=&,block=%,newsize=%)","Pexec(mode=&,%=$,%,%)",
  "Pterm(retcode=&)","","Fsfirst(fnam=%=$,attr=&)","Fsnext","","","","","","",
  "Frename(dummy=&,oldname=%=$,newname=%=$)","Fdatime(timeptr=%,handle=&,flag=&)"};

const char* bios_calls[12]={"GetMBP(pointer=%)","Bconstat(dev=&)","Bconin(dev=&)",
  "Bconout(dev=&,c=&)","Rwabs(rwflag=&,buffer=%,number=&,recno=&,dev=&), read/write disk sector",
  "Setexec(number=&,vector=%), set exception vector","Tickcal","Getbpb(dev=&)","Bcostat(dev=&)",
  "Mediach(dev=&)","Drvmap","Kbshift(mode=&)"};

const char* xbios_calls[40]={"InitMouse(type=&,parameter=%,vector=%)","Ssbrk(number=%)",
  "Physbase","Logbase","Getrez","setscreen(log=%,phys=%,res=&)","Setpalette(ptr=%)",
  "Setcolor(colornum=&,color=&)","Floprd(buffer=%,filler=%,dev=&,sector=&,track=&,side=&,count=&)",
  "Flopwr(buffer=%,filler=%,dev=&,sector=&,track=&,side=&,count=&)",
  "Flopfmt(buffer=%,filler=%,dev=&,spt=&,track=&,side=&,interleave=&,magic=&,virgin=&)","",
  "Midiws(count=&,ptr=%)","Mfpint(number=&,vector=%)","Iorec(dev=&)",
  "Rsconf(baud=&,ctrl=&,ucr=&,rsr=&,tsr=&,scr=&)","Keytbl(unshift=%,shift=%,caps=%)",
  "random","protobt(buffer=%,serialno=%,disktype=&,execflag=&)",
  "Flopver(buffer=%,filler=%,dev=&,sector=&,track=&,side=&,count=&)","Scrdmp",
  "Cursconf(function=&,rate=&)","Settime(time=%)","Gettime","Bioskeys","Ikbdws(number=&,ptr=%)",
  "Jdisint(number=&)","Jenabint(number=&)","Giaccess(data=&,register=&)","Offgibit(bitnumber=&)",
  "Ongibit(bitnumber=&)","Xbtimer(timer=&,control=&,data=&,vector=%)","Dosound(pointer=%)",
  "Setprt(config=&)","Kbdvbase","Kbrate(delay=&,repeat=&)","Prtblk(parameter=%)",
  "Vsync","Supexec(%)","Puntaes"};

FILE *logfile=NULL;
EasyStr LogFileName;


void log_write(EasyStr a) {
  // Differences with our TRACE:
  // one EasyStr argument instead of a printf-like list
  // automatic end-of-line
  if(logfile)
  {
    //fprintf(logfile,"%s\r\n",a.Text);
    //fflush(logfile); // makes things slower, moved to FlushTrace()
    if(Debug.log_in_trace && !Debug.trace_in_log) // avoid stack overflow...
    {
      BYTE save=Debug.tracing_suspended;
      Debug.tracing_suspended=false; // wow, edgy!
      TRACE("%s\n",a.Text);
      Debug.tracing_suspended=save;
    }
    else
      fprintf(logfile,"%s\r\n",a.Text);
  }
}


void log_io_write(MEM_ADDRESS addr,BYTE io_src_b) {
  if(logsection_enabled[LOGSECTION_IO]&&!logging_suspended)
  {
    EasyStr a=Str("IO: ")+HEXSl(old_pc,6)+" - wrote byte "+io_src_b
      +" to IO address "+HEXSl(addr,6);
#ifdef DEBUG_BUILD
    Tiolist_entry *iol=search_iolist(addr);
    if(iol)
      a+=EasyStr(" (")+(iol->name)+")";
#endif
    log_write(a);
  }
}


Str scanline_cycle_log() {
  return Str("scanline ")+scan_y+" cycles "
    +(ABSOLUTE_CPU_TIME-cpu_timer_at_start_of_hbl);
}


void stop_cpu_log() {
  logsection_enabled[LOGSECTION_CPU]=0;
  DEBUG_ONLY(CheckMenuItem(logsection_menu,300+LOGSECTION_CPU,
    MF_BYCOMMAND|MF_UNCHECKED); )
  TRACE("CPU LOG OFF\n");
  log_write("CPU LOG OFF");
}


void log_os_call(int trap) {
  if(logging_suspended&&!SSEConfig.TraceFile)
    return;
  EasyStr l="",a="";
  long lpar=0;
  bool Invalid=0;
  MEM_ADDRESS my_sp=get_sp_before_trap(&Invalid);
  if(Invalid)
    return;
  MEM_ADDRESS my_spp=my_sp+2;
  unsigned int call=m68k_dpeek(my_sp);
  l=HEXSl(old_pc,6)+": ";
  if(trap==1)
  {
    if(call<0x58ul)
      a=(char*)gemdos_calls[call];
    l+="GEMDOS $";
    l+=HEXSl(call,3); //3 for later OS bigger numbers, but only know to $57
  }
  else if(trap==13)
  {
    if(call<12ul)
      a=(char*)bios_calls[call];
    l+="BIOS $";
    l+=HEXSl(call,2); //1 digit would be enough
    //l+=(int)call;
  }
  else if(trap==14)
  {
    if(call<40ul)
      a=(char*)xbios_calls[call];
    l+="XBIOS $";
    l+=HEXSl(call,2); //2 digits are enough
    //l+=(int)call;
  }
  else if(trap==2&&Cpu.r[0]==0x73)
  {
    a="VDI with opcode ";
    MEM_ADDRESS adddd=m68k_lpeek(Cpu.r[1]); //Cpu.r[1] has vdi parameter block.  a points to the control array
    a+=m68k_dpeek(adddd+0); //opcode
    a+=", subopcode ";
    a+=m68k_dpeek(adddd+10); //subopcode
  }
  if(a.IsEmpty())
  {
    //l+=" (unrecognised)";
    l+=" (unknown)";
  }
  else
  {
    l+="  ";
    for(INT_PTR i=0;i<a.Length();i++)
    {
      if(a[i]=='%')
      {
        lpar=m68k_lpeek(my_spp);
        my_spp+=4;
        l+=HEXSl(lpar,8);
      }
      else if(a[i]=='&')
      {
        l+=HEXSl(m68k_dpeek(my_spp),4);
        my_spp+=2;
      }
      else if(a[i]=='$')
      {
        char c;
        int ii;
        for(ii=0;ii<30;ii++)
        {
          c=(char)m68k_peek(lpar+ii);
          if(!c)
            break;
          l+=c;
        }
        if(ii>=30)
          l+="...";
      }
      else
        l+=a[i];
    }
  }
  if(!logging_suspended)
    log_write(l);
  TRACE("Trap #%d %s\n",trap,l.Text); // section trap is enabled
}


void log_write_stack() {
#ifdef DEBUG_BUILD
  log_write(EasyStr("A7 = ")+HEXSl(areg[7],6));
  log_write(EasyStr("stack = ")+HEXSl(d2_dpeek(areg[7]),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+2),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+4),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+6),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+8),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+10),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+12),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+14),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+16),4));
  log_write(EasyStr("        ")+HEXSl(d2_dpeek(areg[7]+18),4));
#endif
}

#endif//#ifdef ENABLE_LOGFILE


#if defined(SSE_VC_INTRINSICS)
int count_bits_set_in_word1(unsigned short w)
#else
int count_bits_set_in_word(unsigned short w) // now a pointer to function
#endif
{
  int t=0;
  for(int n=15;n>=0;n--)
  {
    if(w&1)
      t++;
    w>>=1;
  }
  return t;
}


#ifdef UNIX

EasyStr GetEXEDir() {
  EasyStr Path=_argv[0];
  RemoveFileNameFromPath(Path,REMOVE_SLASH);
  return Path;
}


EasyStr GetCurrentDir() {
  EasyStr Path=GetEXEDir();
  Path.SetLength(MAX_PATH);
  getcwd(Path,MAX_PATH);
  return Path;
}


EasyStr GetEXEFileName() {
  return _argv[0];
}

#endif


#if !defined(SSE_NO_UPDATE)

EasyStr time_or_times(int n) {
  if(n==1)return T("time");
  return T("times");
}

#endif


long colour_convert(int red,int green,int blue) {
#if !defined(SSE_VID_32BIT_ONLY)
  long col;
#endif
  switch(BytesPerPixel) {
#if !defined(SSE_VID_32BIT_ONLY)
  case 1:
#ifdef WIN32
    if((red|green|blue)==0)return 0;
    if((red&green&blue)==255)return 0xffff;
    col=GetNearestPaletteIndex(winpal,RGB(red,green,blue))+1;
    return (col<<8)|col;
#endif
#ifdef UNIX
    return 0;
#endif
  case 2:
    //.RrrrrGggggBbbbb
    //        Ccccc...
    if(rgb555)                                  //%-000001111100000 1555
      col=((red&0xf8)<<7)|((green&0xf8)<<2)|((blue)>>3);
    else                                        //%00000011111100000 565
      col=((red&0xf8)<<8)|((green&0xfc)<<3)|((blue)>>3);
    return (col<<16)|col;
#endif
  case 3:case 4:
    return ((red<<16)|(green<<8)|blue)<<rgb32_bluestart_bit;
  }
  return 0;
}


EasyStr read_string_from_memory(MEM_ADDRESS ad,int max_len) {
  // used by hd_gemdos and iow
  if(ad==0)
    return "";
  EasyStr a;a.SetLength(max_len);
  int n;
  char i;
  for(n=0;n<max_len;n++)
  {
#if defined(SSE_MMU_MONSTER_ALT_RAM)
    if(ad<mem_len)
#else
    if(ad<himem)
#endif
      i=PEEK(ad);
    else if(ad>=rom_addr && ad<rom_addr+tos_len)
      i=ROM_PEEK(ad-rom_addr);
    else
      break;
    ad++;
    if(i==0)
      break;
    a[n]=i;
  }
  a[n]=0;
  return a;
}


MEM_ADDRESS write_string_to_memory(MEM_ADDRESS ad,char*c) {
  // used by hd_gemdos and "onegame"
  if(ad<MEM_FIRST_WRITEABLE)
    return 0;
  do
  {
    m68k_poke(ad++,*c);
  } while(*(c++));
  return ad;
}


EasyStr HEXSl(long n,int ln) {
  char bf[17];
  strcpy(bf,"00000000");
  itoa(n,bf+8,16);
#ifdef DEBUG_BUILD
  if(debug_uppercase_disa)
#endif
    strupr(bf);
  return bf+8-ln+strlen(bf+8);
}


#ifdef SSE_X64

EasyStr HEXSll(long long n,int ln) {
  char bf[17+8];
  strcpy(bf,"0000000000000000");
  _i64toa(n,bf+8,16);
#ifdef DEBUG_BUILD
  if(debug_uppercase_disa)
#endif
    strupr(bf);
  return bf+8-ln+strlen(bf+8);
}

#endif


void acc_parse_search_string(Str OriginalText,DynamicArray<BYTE> &ByteList,
                             bool &WordOnly) {
  // used by debugger and patches
  bool ReturnLengths=WordOnly;
  ByteList.DeleteAll();
  WordOnly=0;
  char *Buf=new char[OriginalText.Length()+1];
  strcpy(Buf,OriginalText);
  for(INT_PTR i=0;i<OriginalText.Length();i++)
  {
    if(Buf[i]==' '||Buf[i]=='\t')
      Buf[i]=0;
  }
  char *pBuf=Buf,*pBufEnd=Buf+OriginalText.Length();
  while(pBuf<pBufEnd)
  {
    Str Text=pBuf;
    if(Text[0]=='\"'||(Text[0]>'F' && Text[0]<='Z')
      ||(Text[0]>'f' && Text[0]<='z'))
    {
      if(Text[0]=='\"')
        Text.Delete(0,1);
      if(Text.RightChar()=='\"')
        *(Text.Right())=0;
      for(INT_PTR i=0;i<Text.Length();i++)
      {
        ByteList.Add(Text[i]);
        if(ReturnLengths)
          ByteList.Add(1);
      }
    }
    else if(Text[0])
    {
      strupr(Text);
      if(Text.RightChar()=='W')
      {
        *(Text.Right())=0; // Just in case this messes with the atoi etc..
        WordOnly=true;
      }
      DWORD Num=0;
      INT_PTR NumLen=0;
      if(Text.Lefts(2)=="0X"||Text[0]=='$'||Text[0]>='A' && Text[0]<='F')
      {
        char *t=Text.Text;
        if(t[1]=='X')
          t+=2;
        else if(t[0]=='$')
          t++;
        int HexLen=0;
        while(*t)
        {
          if(((*t)>='A'&&(*t)<='F')==0&&((*t)>='0'&&(*t)<='9')==0)
            break;
          HexLen++;
          t++;
        }
        if(HexLen>0)
        {
          NumLen=MIN((HexLen+1)/2,4);
          Num=HexToVal(Text);
        }
      }
      else if(Text[0]=='%')
      {  // Binary
        INT_PTR BinLen=0;
        for(;BinLen<Text.Length();BinLen++)
          if(Text[BinLen+1]!='0' && Text[BinLen+1]!='1')
            break;
        if(BinLen>0&&BinLen<=32)
        {
          NumLen=(BinLen+7)/8;
          int Bit=0;
          for(INT_PTR n=BinLen;n>0;n--)
          {
            if(Text[n]=='1')
              Num|=1<<Bit;
            Bit++;
          }
        }
      }
      else
      {                    // Decimal
        NumLen=0;
        if(Text.Rights(2)==(char*)".W")
        {
          NumLen=2;
          *(Text.Right()-1)=0;
        }
        else if(Text.Rights(2)==(char*)".L")
        {
          NumLen=4;
          *(Text.Right()-1)=0;
        }
        Num=(DWORD)atoi(Text);
        if((Num||Text[0]=='0'||Text.Lefts(2)=="-0")&&NumLen==0)
        {
          if(Num<=0xff)
            NumLen=1;
          else if(Num<=0xffff)
            NumLen=2;
          else if(Num<=0xffffff)
            NumLen=3;
          else
            NumLen=4;
        }
      }
      if(NumLen)
      {
#ifndef BIG_ENDIAN_PROCESSOR
        BYTE *lpHiNum=LPBYTE(&Num)+NumLen-1;
        int mem_dir=-1;
#else
        BYTE *lpHiNum=LPBYTE(&Num);
        int mem_dir=1;
#endif
        for(int i=0;i<NumLen;i++)
        {
          ByteList.Add(*(lpHiNum+i*mem_dir));
          if(ReturnLengths)
            ByteList.Add(BYTE(NumLen));
        }
      }
    }
    pBuf+=strlen(pBuf)+1;
  }//wend
  delete[] Buf;
}


MEM_ADDRESS acc_find_bytes(DynamicArray<BYTE> &BytesToFind,bool WordOnly,
                          MEM_ADDRESS ad,int dir) {
  // used by debugger and patches
  BYTE ToFind=BytesToFind[0];
  bool Found=0;
  int n;
  for(;;)
  {
    if(ad>=himem && ad<rom_addr)
      ad=(MEM_ADDRESS)(dir>0?rom_addr:himem-1);
    if(ad>=rom_addr+tos_len||ad>0xffffff)
      break;
    if(((ad&1)&&WordOnly)==0)
    { // if odd and word-only then skip byte
      n=-1;
      if(ad<himem)
      {
        if(PEEK(ad)==ToFind)
        {
          if(ad+BytesToFind.NumItems<=himem)
          {
            for(n=1;n<BytesToFind.NumItems;n++)
              if(PEEK(ad+n)!=BytesToFind[n])
                break;
          }
        }
      }
      else
      {
        if(ROM_PEEK(ad-rom_addr)==ToFind)
        {
          if(ad+BytesToFind.NumItems<=rom_addr+tos_len)
          {
            for(n=1;n<BytesToFind.NumItems;n++)
              if(ROM_PEEK(ad+n-rom_addr)!=BytesToFind[n])
                break;
          }
        }
      }
      if(n>=BytesToFind.NumItems)
      {
        Found=true;
        break;
      }
    }
    ad+=dir;
  }
  if(Found)
    return ad;
  return 0xffffffff;
}


#ifdef WIN32

int get_text_width(char *t) {

  SIZE sz;
  HDC dc=GetDC(StemWin);
  HANDLE oldfnt=SelectObject(dc,fnt);
  GetTextExtentPoint32(dc,t,(int)strlen(t),&sz);
  SelectObject(dc,oldfnt);
  ReleaseDC(StemWin,dc);
  return sz.cx+1; // For grayed string
}


BOOL SetClipboardText(LPCTSTR pszText) {// from the 'net
/*  Used for -copy 68000 code from browser (Debugger)
             -copy disk name (excluding extension) from Disk manager
*/
  BOOL ok=FALSE;
  if(OpenClipboard(NULL))
  {
    EmptyClipboard(); // SS added this
    // the text should be placed in "global" memory
    HGLOBAL hMem=GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE,
      (lstrlen(pszText)+1)*sizeof(pszText[0]));
    LPTSTR ptxt=(LPTSTR)GlobalLock(hMem);
    lstrcpy(ptxt,pszText);
    GlobalUnlock(hMem);
    // set data in clipboard; we are no longer responsible for hMem
    ok=(SetClipboardData(CF_TEXT,hMem)!=0);
    CloseClipboard(); // relinquish it for other windows
  }
  return ok;
}


HMODULE SteemLoadLibrary(LPCSTR lpLibFileName) {
  // first look in /plugins32 or /plusings64, then in /plugins
  // then same folder as plugin name (pasti/pasti) then
  // steem root; lpLibFileName should have no extension
  HMODULE hm=NULL;
  char rel_path[512];
  sprintf(rel_path,"%s\\%s",SSE_PLUGIN_DIR1,lpLibFileName);
  hm=LoadLibrary(rel_path);
  if(hm==NULL)
  {
    sprintf(rel_path,"%s\\%s",SSE_PLUGIN_DIR2,lpLibFileName);
    hm=LoadLibrary(rel_path);
  }
  if(hm==NULL)
  {
    sprintf(rel_path,"%s\\%s",lpLibFileName,lpLibFileName);
    hm=LoadLibrary(rel_path);
  }
  if(hm==NULL)
  {
    sprintf(rel_path,"%s",lpLibFileName);
    hm=LoadLibrary(rel_path);
  }
  return hm;
}

#endif
