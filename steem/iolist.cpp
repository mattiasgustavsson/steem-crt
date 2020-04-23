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
FILE: iolist.cpp
CONDITION: DEBUG_BUILD must be defined
DESCRIPTION: A global list that stores information about various special
ST addresses and how they should be displayed in the debugger.
Also pseudo IO addresses for further Debugger control.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#ifdef DEBUG_BUILD

#include <iolist.h>
#include <mymisc.h>
#include <gui.h>
#include <computer.h>

int iolist_length=0;
Tiolist_entry iolist[650]; // 633 used yet


void iolist_add_entry(MEM_ADDRESS ad,char*name,int bytes,char*bitmask,
                          BYTE*ptr) {
  Tiolist_entry*p=iolist+iolist_length;
  p->ad=ad;
  if(name==NULL)
    p->name="";
  else 
    p->name=name;
  p->bytes=bytes;
  p->ptr=ptr;
  if(bitmask==NULL)
  { //bitmask not significant
    p->bitmask=""; //"F|E|D|C|B|A|9|8|7|6|5|4|3|2|1|0";
  }
  else if(*bitmask=='#')
  {  //special masks
    p->bitmask=bitmask;
  }
  else
  {
    p->bitmask="";
    int n=15;if(bytes==1)n=7;
    for(char*c=bitmask;*c;c++)
    {
      if(*c=='|')
        n--;
    }
    for(;n>0;n--)
      p->bitmask+=".|"; //pad
    p->bitmask+=bitmask;
  }
  iolist_length++;
};


void iolist_init() {
  iolist_length=0;
  iolist_add_entry(0x0,"Initial SSP",4);
  iolist_add_entry(0x4,"Initial PC",4);
  iolist_add_entry(0x8,"Bus Error Vector",4);
  iolist_add_entry(0xc,"Address Error Vector",4);
  iolist_add_entry(0x10,"Illegal Instruction Vector",4);
  iolist_add_entry(0x14,"Divide By Zero Vector",4);
  iolist_add_entry(0x18,"CHK vector",4);
  iolist_add_entry(0x1c,"TRAPV vector",4);
  iolist_add_entry(0x20,"Privelege Violation Vector",4);
  iolist_add_entry(0x24,"Trace Exception Vector",4);
  iolist_add_entry(0x28,"Line-A Emulator",4);
  iolist_add_entry(0x2c,"Line-F Emulator",4);
  iolist_add_entry(0x60,"68000 Spurious Interrupt",4);
  iolist_add_entry(0x64,"68000 Level 1 Interrupt (not connected)",4);
  iolist_add_entry(0x68,"68000 Level 2 Interrupt (HBL)",4);
  iolist_add_entry(0x6c,"68000 Level 3 Interrupt (not connected)",4);
  iolist_add_entry(0x70,"68000 Level 4 Interrupt (VBL)",4);
  iolist_add_entry(0x74,"68000 Level 5 Interrupt (not connected)",4);
  //iolist_add_entry(0x78,"68000 Level 6 Interrupt (MFP)",4);
  iolist_add_entry(0x78,"68000 Level 6 Interrupt (MFP, no autovector)",4);
  iolist_add_entry(0x7c,"68000 Level 7 Interrupt (not connected)",4);
  iolist_add_entry(0x80,"Trap #0",4);
  iolist_add_entry(0x84,"Trap #1 (GEMDOS)",4);
  iolist_add_entry(0x88,"Trap #2 (AES/VDI)",4);
  iolist_add_entry(0x8C,"Trap #3",4);
  iolist_add_entry(0x90,"Trap #4",4);
  iolist_add_entry(0x94,"Trap #5",4);
  iolist_add_entry(0x98,"Trap #6",4);
  iolist_add_entry(0x9C,"Trap #7",4);
  iolist_add_entry(0xA0,"Trap #8",4);
  iolist_add_entry(0xA4,"Trap #9",4);
  iolist_add_entry(0xA8,"Trap #10",4);
  iolist_add_entry(0xAC,"Trap #11",4);
  iolist_add_entry(0xB0,"Trap #12",4);
  iolist_add_entry(0xB4,"Trap #13 (BIOS)",4);
  iolist_add_entry(0xB8,"Trap #14 (XBIOS)",4);
  iolist_add_entry(0xBc,"Trap #15",4);

  iolist_add_entry(0x100,"MFP 0 - Centronics Busy",4);
  iolist_add_entry(0x104,"MFP 1 - RS232 DCD",4);
  iolist_add_entry(0x108,"MFP 2 - RS232 CTS",4);
  iolist_add_entry(0x10c,"MFP 3 - Blitter Done",4);
  iolist_add_entry(0x110,"MFP 4 - Timer D",4);
  iolist_add_entry(0x114,"MFP 5 - Timer C",4);
  iolist_add_entry(0x118,"MFP 6 - ACIA",4);
  iolist_add_entry(0x11c,"MFP 7 - FDC",4);
  iolist_add_entry(0x120,"MFP 8 - Timer B",4);
  iolist_add_entry(0x124,"MFP 9 - Send Error",4);
  iolist_add_entry(0x128,"MFP 10 - Send Buffer Empty",4);
  iolist_add_entry(0x12c,"MFP 11 - Receive Error",4);
  iolist_add_entry(0x130,"MFP 12 - Receive Buffer Full",4);
  iolist_add_entry(0x134,"MFP 13 - Timer A",4);
  iolist_add_entry(0x138,"MFP 14 - RS232 Ring Detect",4);
  iolist_add_entry(0x13c,"MFP 15 - Monochrome Detect",4);

  iolist_add_entry(0x380,"proc_lives",4);
  iolist_add_entry(0x384,"proc_dregs",4);
  iolist_add_entry(0x3A4,"proc_aregs",4);
  iolist_add_entry(0x384,"proc_enum",4);
  iolist_add_entry(0x3C8,"proc_usp",4);
  iolist_add_entry(0x384,"proc_stk",2);
  iolist_add_entry(0x400,"etv_timer",4);
  iolist_add_entry(0x404,"etv_critic",4);
  iolist_add_entry(0x408,"etv_term",4);
  iolist_add_entry(0x40C,"etv_xtra",4*5);
  iolist_add_entry(0x420,"memvalid",4);
  iolist_add_entry(0x424,"memctrl",2);
  iolist_add_entry(0x426,"resvalid",4);
  iolist_add_entry(0x42A,"resvector",4);
  iolist_add_entry(0x42E,"phystop",4);
  iolist_add_entry(0x432,"_membot",4);
  iolist_add_entry(0x436,"_memtop",4);
  iolist_add_entry(0x43A,"memval2",4);
  iolist_add_entry(0x43E,"flock",2);
  iolist_add_entry(0x440,"seekrate",2);
  iolist_add_entry(0x442,"_timer_ms",2);
  iolist_add_entry(0x444,"_fverify",2);
  iolist_add_entry(0x446,"_bootdev",2);
  iolist_add_entry(0x448,"palmode",2);
  iolist_add_entry(0x44A,"defshiftmod",2);
  iolist_add_entry(0x44C,"sshiftmod",2);
  iolist_add_entry(0x44E,"_v_bas_ad",4);
  iolist_add_entry(0x452,"vblsem",2);
  iolist_add_entry(0x454,"nvbls",2);
  iolist_add_entry(0x456,"_vblqueue",4);
  iolist_add_entry(0x45A,"colorptr",4);
  iolist_add_entry(0x45E,"screenpt",4);
  iolist_add_entry(0x462,"_vbclock",4);
  iolist_add_entry(0x466,"_frclock",4);
  iolist_add_entry(0x46A,"hdv_init",4);
  iolist_add_entry(0x46E,"swv_vec",4);
  iolist_add_entry(0x472,"hdv_bpb",4);
  iolist_add_entry(0x476,"hdv_rw",4);
  iolist_add_entry(0x47A,"hdv_boot",4);
  iolist_add_entry(0x47E,"hdv_mediach",4);
  iolist_add_entry(0x482,"_comload",2);
  iolist_add_entry(0x484,"conterm",1);
  iolist_add_entry(0x486,"trp14ret",4);
  iolist_add_entry(0x48A,"criticret",4);
  iolist_add_entry(0x48E,"themd",4);
  iolist_add_entry(0x49E,"themdmd",4);
  iolist_add_entry(0x4A2,"savptr",4);
  iolist_add_entry(0x4A6,"_nflops",2);
  iolist_add_entry(0x4A8,"con_state",4);
  iolist_add_entry(0x4AC,"save_row",2);
  iolist_add_entry(0x4AE,"sav_context",4);
  iolist_add_entry(0x4B2,"_bufl",4);
  iolist_add_entry(0x4B6,"_bufl",4);
  iolist_add_entry(0x4BA,"_hz_200",4);
  iolist_add_entry(0x4BC,"the_env",4);
  iolist_add_entry(0x4C2,"_drvbits",4);
  iolist_add_entry(0x4C6,"_dskbufp",4);
  iolist_add_entry(0x4CA,"_autopath",4);
  iolist_add_entry(0x4CE,"_vbl_lis",4);
  iolist_add_entry(0x4EE,"_dumpflg",4);
  iolist_add_entry(0x4F0,"_prtabt",2);
  iolist_add_entry(0x4F2,"_sysbase",4);
  iolist_add_entry(0x4F6,"_shell_p",4);
  iolist_add_entry(0x4FA,"end_os",4);
  iolist_add_entry(0x4FE,"exec_os",4);
  iolist_add_entry(0x502,"scr_dump",4);
  iolist_add_entry(0x506,"prv_lsto",4);
  iolist_add_entry(0x50A,"prv_lst",4);
  iolist_add_entry(0x50E,"prv_auxo",4);
  iolist_add_entry(0x512,"prv_aux",4);
  iolist_add_entry(0x516,"pun_ptr",4);
  iolist_add_entry(0x51A,"memval3",4);
  iolist_add_entry(0x51E,"xconstat",4);
  iolist_add_entry(0x53E,"xconin",4);
  iolist_add_entry(0x55E,"xcostat",4);
  iolist_add_entry(0x57E,"xconout",4);
  iolist_add_entry(0x59E,"_longframe",2);
  iolist_add_entry(0x5A0,"_p_cookies",4);
  iolist_add_entry(0x5A4,"ramtop",4);
  iolist_add_entry(0x5A8,"ramvalid",4);
  iolist_add_entry(0x5AC,"bell_hook",4);
  iolist_add_entry(0x5B0,"kcl_hook",4);

  iolist_add_entry(0xff8001,"CONFIG",1,"Bank0h|Bank0l|Bank1h|Bank1l");
  iolist_add_entry(0xff8201,"VBASEHI",1);
  iolist_add_entry(0xff8203,"VBASEMI",1);
  iolist_add_entry(0xff8205,"VCOUNTHI",1);
  iolist_add_entry(0xff8207,"VCOUNTMI",1);
  iolist_add_entry(0xff8209,"VCOUNTLO",1);
  iolist_add_entry(0xff820a,"SYNC",1,"50Hz|.");//,&Glue.m_SyncMode);
  iolist_add_entry(0xff820d,"VBASELO",1);
  iolist_add_entry(0xff820f,"LINEWID",1);
  for(int n=0;n<16;n++)
    iolist_add_entry(0xff8240+n*2,EasyStr("pal")+n,2,"#COLOUR");
  iolist_add_entry(0xff8260,"SHIFT",1);
  iolist_add_entry(0xff8264,"HSCROLL (No Extra Fetch)",1);
  iolist_add_entry(0xff8265,"HSCROLL",1);
  iolist_add_entry(0xff8604,"FDC access/sector count",2);
  iolist_add_entry(0xff8606,"DMA mode/status",2);
  iolist_add_entry(0xff8609,"DMA high",1);
  iolist_add_entry(0xff860b,"DMA mid",1);
  iolist_add_entry(0xff860d,"DMA low",1);
  iolist_add_entry(0xff860e,"Freq/Density Control",2);
  iolist_add_entry(0xff8800,"PSG read data/reg sel",1);
  iolist_add_entry(0xff8802,"PSG write data",1);

  iolist_add_entry(0xff8901,"DMA Sound Control",1,"Loop|Play");
  iolist_add_entry(0xff8903,"DMA Sound Start High",1);
  iolist_add_entry(0xff8905,"DMA Sound Start Mid",1);
  iolist_add_entry(0xff8907,"DMA Sound Start Low",1);
  iolist_add_entry(0xff8909,"DMA Sound Current High",1);
  iolist_add_entry(0xff890b,"DMA Sound Current Mid",1);
  iolist_add_entry(0xff890d,"DMA Sound Current Low",1);
  iolist_add_entry(0xff890f,"DMA Sound End High",1);
  iolist_add_entry(0xff8911,"DMA Sound End Mid",1);
  iolist_add_entry(0xff8913,"DMA Sound End Low",1);
  iolist_add_entry(0xff8921,"DMA Sound Mode",1,"Mono|.|.|.|.|.|Freq|Freq");
  iolist_add_entry(0xff8922,"Microwire Data",1);
  iolist_add_entry(0xff8924,"Microwire Mask",1);

  for(int line=0;line<16;line++)
    iolist_add_entry(0xFF8A00+(line*2),EasyStr("Halftone RAM [")+line+"]",2);
  iolist_add_entry(0xFF8A20,"SrcXInc",2);
  iolist_add_entry(0xFF8A22,"SrcYInc",2);
  iolist_add_entry(0xFF8A24,"SrcAdr",4);
  iolist_add_entry(0xFF8A28,"EndMask[0]",2);
  iolist_add_entry(0xFF8A2A,"EndMask[1]",2);
  iolist_add_entry(0xFF8A2C,"EndMask[2]",2);
  iolist_add_entry(0xFF8A2E,"DestXInc",2);
  iolist_add_entry(0xFF8A30,"DestYInc",2);
  iolist_add_entry(0xFF8A32,"DestAdr",4);
  iolist_add_entry(0xFF8A36,"XCount",2);
  iolist_add_entry(0xFF8A38,"YCount",2);
  iolist_add_entry(0xFF8A3A,"Halftone Op",1);
  iolist_add_entry(0xFF8A3B,"Op",1);
  iolist_add_entry(0xFF8A3C,"Flags1",1,"Busy|Hog|Smudge|.|LineNum3|LineNum2|LineNum1|LineNum0");
  iolist_add_entry(0xFF8A3D,"Flags2",1,"FXSR|NFSR|.|.|Skew3|Skew2|Skew1|Skew0");

  iolist_add_entry(0xfffa01,"MFP GPIP (1)",1,"mono|rs232 ring|FDC/HDC|ACIA|Blitter|rs232 CTS|rs232 data carrier|centronics busy");
  iolist_add_entry(0xfffa03,"MFP AER (2)",1,"mono|rs232 ring|FDC/HDC|ACIA|Blitter|rs232 CTS|rs232 data carrier|centronics busy");
  iolist_add_entry(0xfffa05,"MFP DDR (3)",1,"mono|rs232 ring|FDC/HDC|ACIA|Blitter|rs232 CTS|rs232 data carrier|centronics busy");
  iolist_add_entry(0xfffa07,"MFP IERA (4)",1,"mono|rs232 ring|TIMER A|RX buf full|RX error|TX buf empty|TX error|TIMER B");
  iolist_add_entry(0xfffa09,"MFP IERB (5)",1,"FDC|ACIA|TIMER C|TIMER D|Blitter|CTS|rs232con|LPT");
  iolist_add_entry(0xfffa0b,"MFP IPRA (6)",1,"mono|rs232 ring|TIMER A|RX buf full|RX error|TX buf empty|TX error|TIMER B");
  iolist_add_entry(0xfffa0d,"MFP IPRB (7)",1,"FDC|ACIA|TIMER C|TIMER D|Blitter|CTS|rs232con|LPT");
  iolist_add_entry(0xfffa0f,"MFP ISRA (8)",1,"mono|rs232 ring|TIMER A|RX buf full|RX error|TX buf empty|TX error|TIMER B");
  iolist_add_entry(0xfffa11,"MFP ISRB (9)",1,"FDC|ACIA|TIMER C|TIMER D|Blitter|CTS|rs232con|LPT");
  iolist_add_entry(0xfffa13,"MFP IMRA (10)",1,"mono|rs232 ring|TIMER A|RX buf full|RX error|TX buf empty|TX error|TIMER B");
  iolist_add_entry(0xfffa15,"MFP IMRB (11)",1,"FDC|ACIA|TIMER C|TIMER D|Blitter|CTS|rs232con|LPT");
  iolist_add_entry(0xfffa17,"MFP VR (12)",1,"V7|V6|V5|V4|S|.|.|.");
  iolist_add_entry(0xfffa19,"MFP TACR (13)",1,"low|extern|subdiv2|subdiv1|subdiv0");
  iolist_add_entry(0xfffa1b,"MFP TBCR (14)",1,"low|extern|subdiv2|subdiv1|subdiv0");
  iolist_add_entry(0xfffa1d,"MFP TCDCR (15)",1,".|Csubdiv2|Csubdiv1|Csubdiv0|.|Dsubdiv2|Dsubdiv1|Dsubdiv0");
  iolist_add_entry(0xfffa1f,"MFP TADR (16)",1);
  iolist_add_entry(0xfffa21,"MFP TBDR (17)",1);
  iolist_add_entry(0xfffa23,"MFP TCDR (18)",1);
  iolist_add_entry(0xfffa25,"MFP TDDR (19)",1);
  iolist_add_entry(0xfffa27,"MFP SCR (20)",1);
  iolist_add_entry(0xfffa29,"MFP UCR (21)",1,"freq/16|wordlen1|wordlen0|startstopbits&sync1|ss&s0|parity on|parity even|.");
  iolist_add_entry(0xfffa2b,"MFP RSR (22)",1,"full|overrun|parity err|frame err|break|match|sync strip|enable");
  iolist_add_entry(0xfffa2d,"MFP TSR (23)",1,"empty|underrun|auto turnaround|end|break|H|L|enable");
  iolist_add_entry(0xfffa2f,"MFP UDR (24)",1);

#if defined(SSE_DEBUGGER_FAKE_IO)
/*  We may name those variables how we wish.
    Each is 1 word but we use only higher byte at most for readability.
    It seems the GUI is more confused when there are . before real fields.
    With this system, many controls are easy to add.
*/

  iolist_add_entry(FAKE_IO_START,"Frame report1",2,
    "sync|shift|pal|vc|lines|hscroll|vb|.|.|.|.|.|.|.|.|.");
  iolist_add_entry(FAKE_IO_START+2,"Frame report2",2, // problem +2 etc hardcoded
    "irq|blt|tricks|bytes|tb|.|.|.|.|.|.|.|.|.|.|.");

  iolist_add_entry(FAKE_IO_START+4,"OSD1",2,
    "irq|ikbd|fdc|.|.|.|.|.|.|.|.|.|.|.|.|.");
  iolist_add_entry(FAKE_IO_START+6,"OSD2 CPU",2,
    "trc|bmb|.|.|.|.|.|.|.|.|.|.|.|.|.|.");
  iolist_add_entry(FAKE_IO_START+8,"OSD2 Glue",2,
    "tricks|RS|.|.|.|.|.|.|.|.|.|.|.|.|.|.");
  iolist_add_entry(FAKE_IO_START+10,"Free",2,
    ".|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.");

  iolist_add_entry(FAKE_IO_START+12,"TRACE Glue",2,
    "vert|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.");
  iolist_add_entry(FAKE_IO_START+14,"TRACE irq",2,
    "a|b|c|d|e|syn|rte|evt|.|.|.|.|.|.|.|.");
  iolist_add_entry(FAKE_IO_START+16,"TRACE disk",2,
    "str|data|psg|reg|wd|mfm|boot|.|.|.|.|.|.|.|.|.");

  iolist_add_entry(FAKE_IO_START+18,"History",2,
    // long list | display frame timing
    "lng|tmg|.|.|.|.|.|.|.|.|.|.|.|.|.|.");

  iolist_add_entry(FAKE_IO_START+20,"Sound mute",2,
    "dma|psg1|psg2|psg3|noise|env|vol|.|.|.|.|.|.|.|.|.");
//   15   14   13   12    11   10  9
  iolist_add_entry(FAKE_IO_START+22,"meta breakpoint",2,
    "prg|top|bot|6301|.|.|.|.|.|.|.|.|.|.|.|.");

  iolist_add_entry(FAKE_IO_START+24,"TRACE IO",2,
    "w|r|.|.|.|.|.|.|.|.|.|.|.|.|.|.");

  iolist_add_entry(FAKE_IO_START+26,"TRACE CPU",2,
    "reg|cyc|lim|val|.|.|.|.|.|.|.|.|.|.|.|.");

  iolist_add_entry(FAKE_IO_START+28,"Free",2,
    ".|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.");

#if defined(SSE_VID_STVL3)
  // SYNC DE BLANK LOAD
  iolist_add_entry(FAKE_IO_START+30,"STVL1",2,
    "sc|de|bk|ld|.|.|.|.|.|.|.|.|.|.|.|.");
  // PIXEL_COUNTER RELOAD MODE LIMIT
  iolist_add_entry(FAKE_IO_START+32,"STVL2",2,
    "px|rl|md|lim|.|.|.|.|.|.|.|.|.|.|.|.");
#endif

#endif//fake io

  iolist_add_entry(0xfffc00,"Keyboard ACIA Control",1,"IRQ|Parity Err|Rx Overrun|Framing Err|CTS|DCD|Tx Data Empty|Rx Data Full");
  iolist_add_entry(0xfffc02,"Keyboard ACIA Data",1);
  iolist_add_entry(0xfffc04,"MIDI ACIA Control",1,"IRQ|Parity Err|Rx Overrun|Framing Err|CTS|DCD|Tx Data Empty|Rx Data Full");
  iolist_add_entry(0xfffc06,"MIDI ACIA Data",1);

//----------------- pseudo addresses ------------------------------
  iolist_debug_add_pseudo_addresses();
  TRACE_INIT("iolist_length %d\n",iolist_length);//633
}

Tiolist_entry* search_iolist(MEM_ADDRESS ad) {
  if((ad&0xff000000)!=IOLIST_PSEUDO_AD)
    ad&=0xffffff;
  for(int n=iolist_length-1;n>=0;n--)
  {
//    if(!(((iolist[n].ad)^ad)&~1)){ //even parts agree
    if((iolist[n].ad & -2)==(ad & -2))
    { //even parts agree
      if(iolist[n].bytes==2)
      {
        return iolist+n;
      }
      else if(iolist[n].ad==ad)
      {
        return iolist+n;
      }
    }
    if(iolist[n].ad<ad) return NULL;
  }
  return NULL;
}
//revise calc_wpl

void iolist_box_click(int xc,Tiolist_entry*il,BYTE*ptr) { //toggle bit clicked
  if(il->bitmask[0]!='#')
  {
    EasyStr bitdesc;
    int x=0,wid;
    int cells=0;

  //  long mask=1;
    for(int n=(int)strlen(il->bitmask)-1;n>=0;n--)
    {
      if((il->bitmask)[n]=='|')
        cells++;
    }
    int b=0;
    int nn=0;
    int c1=0,c2=0;
    while(il->bitmask[c2])
    {
      for(c2=c1;il->bitmask[c2]!='|' && il->bitmask[c2];c2++); //search out next delimeter or end of string
      bitdesc=il->bitmask.Mids(c1,c2-c1);
      wid=get_text_width(bitdesc.Text)+6;
      if(xc>=x && xc<x+wid)
      {
        b=cells-nn;
        break;
      }
      x+=wid;
      c1=c2+1;
      nn++;
    }
    bool crashy=false;
    WORD mask=WORD(1<<b);
    if(ptr)
    { //force value
      if(il->bytes==1)
        //one byte to edit
        (*ptr)^=LOBYTE(mask);
      else
        *((WORD*)ptr)^=mask;
    }
    else if(!(il->ad&0xff000000))
    { //not pseudo-address
      if(il->bytes==1)
        //one byte to edit
        crashy=!d2_poke(il->ad,d2_peek(il->ad)^LOBYTE(mask));
      else
        crashy=!d2_dpoke(il->ad,d2_dpeek(il->ad)^mask);
    }
    if(crashy)
      MessageBox(0,"A bus error occurred while trying|to edit this memory.",
        "ттт",0);
  }
}


int iolist_box_width(Tiolist_entry*il) {
  if(il->bitmask.NotEmpty())
  {
    if(il->bitmask[0]!='#')
    { //not special display
      int x=0,wid;
      EasyStr bitdesc;
      long mask=1;
      for(int n=(int)strlen(il->bitmask)-1;n>=0;n--)
      {
        if((il->bitmask)[n]=='|')
          mask<<=1;
      }
      int c1=0,c2=0;
      while(il->bitmask[c2]&&mask)
      {
        for(c2=c1;il->bitmask[c2]!='|' && il->bitmask[c2];c2++); //search out next delimeter or end of string
        bitdesc=il->bitmask.Mids(c1,c2-c1);
        wid=get_text_width(bitdesc.Text)+4;
        mask>>=1;
        x+=wid;
        c1=c2+1;
      }
      return x;
    }
    else if(il->bitmask=="#COLOUR")
      return 60;
  }
  return 10;
}


int iolist_box_draw(HDC dc,int x1,int y1,int w,int h,
                      Tiolist_entry *il,BYTE *ptr) {
  if(il->bitmask.NotEmpty())
  {
    RECT rc={x1,y1,x1+w,y1+h+1};
    HRGN Rgn=CreateRectRgnIndirect(&rc);
    SelectObject(dc,Rgn);
    if(il->bitmask[0]!='#')
    { //not special display
      HBRUSH bg_br=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
      HBRUSH hi_br=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));

      COLORREF hi_text_col=GetSysColor(COLOR_HIGHLIGHTTEXT);
      COLORREF std_text_col=GetSysColor(COLOR_WINDOWTEXT);

      HANDLE OldFont=SelectObject(dc,fnt);
      SetBkMode(dc,TRANSPARENT);

      EasyStr bitdesc;
      int x=0,wid;

//      int cells=1;
      long mask=1;
      for(int n=(int)strlen(il->bitmask)-1;n>=0;n--)
      {
        if((il->bitmask)[n]=='|')
        {
//          cells++;
          mask<<=1;
        }
      }
      long dat=0;
      if(ptr)
      {  //force value
        if(il->bytes==2)
          dat=(long)*((WORD*)ptr);
        else
          dat=(long)*((BYTE*)ptr);
      }
      else if(IS_IOLIST_PSEUDO_ADDRESS(il->ad))
      {
        if(il->ptr) 
          dat=*((BYTE*)(il->ptr));
      }
      else
      {
        if(il->bytes==2)
          dat=d2_dpeek(il->ad);
        else
          dat=d2_peek(il->ad);
      }
      int c1=0,c2=0;
      while(il->bitmask[c2]&&mask)
      {
        for(c2=c1;il->bitmask[c2]!='|' && il->bitmask[c2];c2++); //search out next delimeter or end of string
        bitdesc=il->bitmask.Mids(c1,c2-c1);
        wid=get_text_width(bitdesc.Text)+4;
        rc.left=x+x1;rc.top=y1;rc.right=x+x1+wid+1;rc.bottom=y1+h+1;
        FillRect(dc,&rc,HBRUSH((dat & mask)?hi_br:bg_br));
        SetTextColor(dc,COLORREF((dat & mask)?hi_text_col:std_text_col));
        TextOut(dc,x1+x+2,y1,bitdesc,c2-c1);
        FrameRect(dc,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
        mask>>=1;
        x+=wid;
        c1=c2+1;
      }
      SelectObject(dc,OldFont);
      SelectClipRgn(dc,NULL);

      DeleteObject(hi_br);
      DeleteObject(bg_br);
      DeleteObject(Rgn);
      return x;
    //  EndPaint(Win,&ps);
    }
    else if(il->bitmask=="#COLOUR")
    {
      rc.right=x1+60;

      WORD dat;

      if(ptr)
      {  //force value
        dat=*((WORD*)ptr);
      }
      else if(!(il->ad&0xff000000))
      { //not pseudo-address
        dat=d2_dpeek(il->ad);
      }
      else
      {
        dat=0;
      }

//      STE colour to 24-bit RGB colour
//      700 -> e0 0000, 800 -> 10 0000
//      070 -> 00 e000, 080 -> 00 1000
//      007 -> 00 00e0, 008 -> 00 0010

/*    DWORD rgb=((dat&0x700)<<13) | ((dat&0x800)<<9) //red
              | ((dat&0x070)<<9)  | ((dat&0x080)<<5) //green
              | ((dat&0x007)<<5)  | ((dat&0x008)<<1); //blue

*/
      dat=WORD(((dat&0x888)>>3)|((dat&0x777)<<1));  //fix up stupid rRRRgGGGbBBB colour pattern

      HBRUSH cl=CreateSolidBrush(RGB((dat&0xf00)>>4,(dat&0x0f0),(dat&0xf)<<4));
      FillRect(dc,&rc,cl);
      FrameRect(dc,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));

      SelectClipRgn(dc,NULL);

      DeleteObject(cl);
      DeleteObject(Rgn);
      return 60;
    }
  }
  return 10;
}

#endif//#ifdef DEBUG_BUILD
