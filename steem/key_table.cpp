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
FILE: key_table.cpp
DESCRIPTION: This translates PC keys to ST keys.
This file contains special characters (accented letters etc.) but only in
comments. Format is ANSI (more precisely Windows-1252).
TODO for Spanish, Italian, Norvegian, Swedish keyboards we need good pictures
of the ST keyboard.
v402: Steem can also use an ini file for keyboard mapping.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop
#include <key_table.h>
#include <emulator.h>
#include <stdarg.h>
#include <debug.h>
#include <tos.h>
#include <options.h>

/*
SCAN CODES
01  Esc     1B  ]             35  /             4F  { NOT USED }
02  1       1C  RET           36  (RIGHT) SHIFT 50  DOWN ARROW
03  2       1D  CTRL          37  { NOT USED }  51  { NOT USED }
04  3       1E  A             38  ALT           52  INSERT
05  4       1F  S             39  SPACE BAR     53  DEL
06  5       20  D             3A  CAPS LOCK     54  { NOT USED }
07  6       21  F             3B  F1            5F  { NOT USED }
08  7       22  G             3C  F2            60  ISO KEY
09  8       23  H             3D  F3            61  UNDO
0A  9       24  J             3E  F4            62  HELP
0B  0       25  K             3F  F5            63  KEYPAD (
0C  -       26  L             40  F6            64  KEYPAD /
0D  ==      27  ;             41  F7            65  KEYPAD *
0E  BS      28  '             42  F8            66  KEYPAD *
0F  TAB     29  `             43  F9            67  KEYPAD 7
10  Q       2A  (LEFT) SHIFT  44  F10           68  KEYPAD 8
11  W       2B  \	      45  { NOT USED }  69  KEYPAD 9
12  E       2C  Z             46  { NOT USED }  6A  KEYPAD 4
13  R       2D  X             47  HOME          6B  KEYPAD 5
14  T       2E  C             48  UP ARROW      6C  KEYPAD 6
15  Y       2F  V             49  { NOT USED }  6D  KEYPAD 1
16  U       30  B             4A  KEYPAD -      6E  KEYPAD 2
17  I       31  N             4B  LEFT ARROW    6F  KEYPAD 3
18  O       32  M             4C  { NOT USED }  70  KEYPAD 0
19  P       33  ,             4D  RIGHT ARROW   71  KEYPAD .
1A  [       34  .             4E  KEYPAD +      72  KEYPAD ENTER
*/

BYTE key_table[256]={
//         $.0  $.1  $.2  $.3  $.4  $.5  $.6  $.7  $.8  $.9  $.a  $.b  $.c  $.d  $.e  $.f
/* $0. */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0e,0x0f,0x00,0x00,0x00,0x1c,0x00,0x00,
/* $1. */ 0x2a,0x1d,0x38,0x00,0x3a,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,
/* $2. */ 0x39,0x62,0x61,0x00,0x47,0x4b,0x48,0x4d,0x50,0x00,0x00,0x00,0x00,0x52,0x53,0x62,
//    PageUp=Help^    ^Page Down=Undo
/* $3. */ 0x0b,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,
/* $4. */ 0x00,0x1e,0x30,0x2e,0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x32,0x31,0x18,
/* $5. */ 0x19,0x10,0x13,0x1f,0x14,0x16,0x2f,0x11,0x2d,0x15,0x2c,0x00,0x00,0x00,0x00,0x00,
/* $6. */ 0x70,0x6d,0x6e,0x6f,0x6a,0x6b,0x6c,0x67,0x68,0x69,0x66,0x4e,0x00,0x4a,0x71,0x65,
/* $7. */ 0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x41,0x42,0x43,0x44,0x63,0x64,0x00,0x00,0x00,0x00,
//                                                           ^F11 ^F12 (numpad brackets)
/* $8. */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* $9. */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* $a. */ 0x2a,0x36,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//        ^ Left/Right distinguishing constants (doesn't work for most things that take VK codes)
/* $b. */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* $c. */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* $d. */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* $e. */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* $f. */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
                    };

BYTE PCCharToSTChar[128]={0,  0,  0,159,  0,  0,187,  0,  0,  0,  0,  0,181,  0,  0,  0,
                          0,  0,154,  0,  0,  0,  0,255,  0,191,  0,  0,182,  0,  0,  0,
                          0,173,  0,156,  0,157,  0,221,185,189,  0,174,170,  0,190,  0,
                          248,241,253,254,  0,230,188,  0,  0,199,  0,175,172,171,  0,168,
                          182,  0,  0,183,142,143,147,128,  0,144,  0,  0,  0,  0,  0,  0,
                          0,165,  0,  0,  0,184,153,194,178,  0,  0,  0,154,  0,  0,158,
                          133,160,131,176,132,134,145,135,138,130,136,137,141,161,140,139,
                          0,164,149,162,147,177,148,246,179,151,163,150,154,  0,  0,152};

BYTE STCharToPCChar[128]={199,  0,233,226,228,224,229,231,234,235,232,239,238,236,196,197,
                          201,230,  0,244,246,242,251,249,255,214,252,  0,163,165,223,131,
                          225,237,243,250,241,209,  0,  0,191,  0,172,189,188,161,171,187,
                          227,245,216,248,  0,140,156,195,213,168,  0,134,182,169,174,153,
                          0,  0,215,  0,  0,  0,  0,185,  0,  0,  0,  0,  0,  0,  0,  0,
                          0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,167,  0,  0,
                          0,  0,  0,  0,  0,  0,181,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                          0,177,  0,  0,  0,  0,247,  0,176,  0,  0,  0,  0,178,179,151};

#ifndef STEEMKEYTEST //?
// These are the characters that are produced by pressing Alt+[shift]+key.
// They were really hard to extract from TOS!
// BYTE(STCode),BYTE(Modifiers, bit 0=shift 1=alt),BYTE(STAscii code),BYTE(0)

DWORD AltKeys_French[8]={MAKELONG(MAKEWORD(0x1a,2),'['),  
                         MAKELONG(MAKEWORD(0x1b,2),']'),
                         MAKELONG(MAKEWORD(0x1a,3),'{'),
                         MAKELONG(MAKEWORD(0x1b,3),'}'),
                         MAKELONG(MAKEWORD(0x2b,2),'@'),
                         MAKELONG(MAKEWORD(0x2b,3),'~'),
                         MAKELONG(MAKEWORD(0x28,2),'\\'), 0};
DWORD AltKeys_German[7]={MAKELONG(MAKEWORD(0x1a,2),'@'),
                         MAKELONG(MAKEWORD(0x1a,3),'\\'),
                         MAKELONG(MAKEWORD(0x27,2),'['),
                         MAKELONG(MAKEWORD(0x28,2),']'),
                         MAKELONG(MAKEWORD(0x27,3),'{'),
                         MAKELONG(MAKEWORD(0x28,3),'}'),
                         0};
DWORD AltKeys_Spanish[8]={MAKELONG(MAKEWORD(0x1a,2),'['),
                          MAKELONG(MAKEWORD(0x1b,2),']'),
                          MAKELONG(MAKEWORD(0x1a,3),'{'),
                          MAKELONG(MAKEWORD(0x1b,3),'}'),
                          MAKELONG(MAKEWORD(0x2b,2),'#'),
                          MAKELONG(MAKEWORD(0x2b,3),'@'),
                          MAKELONG(MAKEWORD(0x28,2),129/*ü*/),  0};
DWORD AltKeys_Italian[8]={MAKELONG(MAKEWORD(0x1a,2),'['),
                          MAKELONG(MAKEWORD(0x1b,2),']'),
                          MAKELONG(MAKEWORD(0x1a,3),'{'),
                          MAKELONG(MAKEWORD(0x1b,3),'}'),
                          MAKELONG(MAKEWORD(0x2b,2),248/*°*/),
                          MAKELONG(MAKEWORD(0x2b,3),'~'),
                          MAKELONG(MAKEWORD(0x60,2),'`'),  0};
DWORD AltKeys_Swedish[9]={MAKELONG(MAKEWORD(0x1a,2),'['),
                          MAKELONG(MAKEWORD(0x1b,2),']'),
                          MAKELONG(MAKEWORD(0x1a,3),'{'),
                          MAKELONG(MAKEWORD(0x1b,3),'}'),
                          MAKELONG(MAKEWORD(0x28,2),'`'),
                          MAKELONG(MAKEWORD(0x28,3),'~'),
                          MAKELONG(MAKEWORD(0x2b,2),'^'),
                          MAKELONG(MAKEWORD(0x2b,2),'@'),
                          0};
DWORD AltKeys_Swiss[10]={MAKELONG(MAKEWORD(0x1a,2),'@'),
                         MAKELONG(MAKEWORD(0x1a,3),'\\'),
                         MAKELONG(MAKEWORD(0x1b,2),'#'),
                         MAKELONG(MAKEWORD(0x27,2),'['),
                         MAKELONG(MAKEWORD(0x28,2),']'),
                         MAKELONG(MAKEWORD(0x27,3),'{'),
                         MAKELONG(MAKEWORD(0x28,3),'}'),
                         MAKELONG(MAKEWORD(0x2b,2),'~'),
                         MAKELONG(MAKEWORD(0x2b,3),'|'),
                         0};




void GetAvailablePressChars(DynamicArray<DWORD> *lpChars) {
  MEM_ADDRESS UnshiftTableAddr,ShiftTableAddr;
  GetTOSKeyTableAddresses(&UnshiftTableAddr,&ShiftTableAddr);
  MEM_ADDRESS TableAddr=UnshiftTableAddr;
  int Shift=0;
  for(int t=0;t<2;t++)
  {
    // Go through every entry of both tables
    for(int STCode=0;STCode<128;STCode++)
    {
      // Ignore keypad codes, they just confuse things and are the same on every ST
      if((STCode<0x63||STCode>0x72)&&STCode!=0x4a&&STCode!=0x4e)
      {
        BYTE STAscii=ROM_PEEK(TableAddr+STCode);
        if(STAscii>32&&STAscii!=127)
        { // Viewable character and not delete
          DWORD Code=MAKELONG(MAKEWORD(STCode,Shift),STAscii);
          lpChars->Add(Code);
        }
      }
    }
    TableAddr=ShiftTableAddr;
    Shift=1;
  }
  // Handle characters typed while holding Alt, these aren't
  // in any key table
  DWORD *Alts=NULL;
  switch(ROM_PEEK(0x1d)) { // Country code
  case 5:  Alts=AltKeys_French; break;
  case 9:  Alts=AltKeys_Spanish; break;
  case 3:  Alts=AltKeys_German; break;
  case 11: Alts=AltKeys_Italian; break;
  case 13: Alts=AltKeys_Swedish; break;
  case 17: Alts=AltKeys_Swiss; break;
  }
  if(Alts)
  {
    while(*Alts) 
      lpChars->Add(*(Alts++));
  }
}

#endif//#ifndef STEEMKEYTEST


WORD *shift_key_table[4]={NULL,NULL,NULL,NULL};

bool EnableShiftSwitching=0,ShiftSwitchingAvailable=0;

#ifdef WIN32

HKL keyboard_layout; // could do without, it's just to be up-to-date


void SetSTKeys(char *Letters,int Val1,...) {
  va_list vl;
  int arg=Val1;
  int l=0;
  WORD Code;
  va_start(vl,Val1);
  do
  {
    Code=VkKeyScanEx(Letters[l],keyboard_layout);
    //ASSERT(Code!=0xFF); // FF = fail, may happen if the current keyboard doesn't have the letter
    if(HIBYTE(Code)==0) // VK code exists + No PC modifier required
      key_table[LOBYTE(Code)]=LOBYTE(arg);
    arg=va_arg(vl,int);
  } while(Letters[++l]);
  va_end(vl);
}

#endif//WIN32

#ifdef UNIX

KeyCode Key_Pause,Key_Quit;

void SetSTKeys(char *Letters,int Val1,...) {
  va_list vl;
  int arg=Val1;
  va_start(vl,Val1);
  int l=0;
  KeyCode Code;
  do
  {
    // Somehow KeySym codes are exactly the same as Windows standard ASCII codes!
    KeySym ks=(KeySym)((unsigned char)(Letters[l])); // Don't sign extend this!
    Code=XKeysymToKeycode(XD,ks);
    // Now assign this X scan code to the ST scan code, we should only do this if you
    // do not need shift/alt to access the character. However, in a vain attempt to
    // improve mapping, we assign it anyway if the code isn't already assigned.
    if(XKeycodeToKeysym(XD,Code,0)==ks||key_table[LOBYTE(Code)]==0)
    {
      key_table[LOBYTE(Code)]=LOBYTE(arg);
      arg=va_arg(vl,int);
    }
  } while(Letters[++l]);
}


void SetSTKey(KeySym Sym,BYTE STCode,bool CanOverwrite) {
  KeyCode Code=XKeysymToKeycode(XD,Sym);
  if(key_table[BYTE(Code)]==0||CanOverwrite)
    key_table[BYTE(Code)]=STCode;
}

#endif//UNIX


void SetSTKeys(int Letter...) {
  // new function that doesn't require a string as first argument, takes codes
  // stop when Letter=-1
  va_list vl;
  int arg=Letter;
  va_start(vl,Letter);
  for(int l=0;Letter!=-1 && l<128;l++) // 128 max, if -1 forgotten by caller
  {
    ASSERT(l<128);
    arg=va_arg(vl,int);
#ifdef WIN32
/*
If the function succeeds, the low-order byte of the return value contains the
virtual-key code and the high-order byte contains the shift state, which can
be a combination of the following flag bits.

Table 1
Return value 	Description

1	Either SHIFT key is pressed.

2	Either CTRL key is pressed.

4	Either ALT key is pressed.

8	The Hankaku key is pressed

16	Reserved (defined by the keyboard layout driver).

32	Reserved (defined by the keyboard layout driver).
 

If the function finds no key that translates to the passed character code, both
the low-order and high-order bytes contain -1.
*/
    WORD Code=VkKeyScanEx((CHAR)Letter,keyboard_layout);
    //ASSERT(Code!=0xFFFF);
    if(HIBYTE(Code)==0) // VK code exists + No PC modifier required
      key_table[LOBYTE(Code)]=LOBYTE(arg);
#endif
#ifdef UNIX
    // Somehow KeySym codes are exactly the same as Windows standard ASCII codes!
    KeySym ks=(KeySym)((unsigned char)(Letter)); // Don't sign extend this!
    KeyCode Code=XKeysymToKeycode(XD,ks);
    // Now assign this X scan code to the ST scan code, we should only do this if you
    // do not need shift/alt to access the character. However, in a vain attempt to
    // improve mapping, we assign it anyway if the code isn't already assigned.
    if(XKeycodeToKeysym(XD,Code,0)==ks||key_table[LOBYTE(Code)]==0)
    {
      key_table[LOBYTE(Code)]=LOBYTE(arg);
    }
#endif
    Letter=va_arg(vl,int);
  }
  va_end(vl);
}


#define PC_SHIFT 1
#define NO_PC_SHIFT 0
#define PC_ALT 2
#define NO_PC_ALT 0

#define ST_SHIFT 1
#define NO_ST_SHIFT 0
#define ST_ALT 2
#define NO_ST_ALT 0


void AddToShiftSwitchTable(int PCModifiers,int PCAscii,BYTE STModifiers,
                           BYTE STCode) {
  BYTE Code;
  ShiftSwitchingAvailable=true; // not true for US or UK keyboard
#ifdef WIN32
  Code=LOBYTE(VkKeyScanEx((CHAR)PCAscii,keyboard_layout));
#endif
#ifdef UNIX
  Code=(BYTE)XKeysymToKeycode(XD,(KeySym)PCAscii);
#endif
  //ASSERT(Code!=0xFF);
  if(Code!=0xff && shift_key_table[PCModifiers])
    shift_key_table[PCModifiers][Code]=MAKEWORD(STCode,STModifiers);
}


void DestroyKeyTable() {
  for(int i=0;i<4;i++)
  {
    if(shift_key_table[i])
    {
      free(shift_key_table[i]);
      shift_key_table[i]=NULL;
    }
  }
}


#if defined(SSE_IKBD_MAPPINGFILE)
/*  Steem can use an ini file to set up the keyboard mapping.
    Format of the file is explained in the ini files and the manual.
    This is the parser, it uses existing functions to assign keys.
*/

EasyStr KeyboardMappingPath;

#define SHIFT 1 // ST or PC
#define ALT 2 // ST or PC

bool ReadMappingFile(char *path) {
  bool ok=false;
  TRACE_INIT("Parsing %s, EnableShiftSwitching %d ",path,EnableShiftSwitching);
  TRACE_INIT("T%X-%d\n",tos_version,SSEConfig.TosLanguage);
  FILE *fp=fopen(path,"rb"); // it's a text file but we treat it as binary
  if(fp!=NULL)
  {
    size_t fl=GetFileLength(fp);
    BYTE *mem=(BYTE*)malloc(fl);
    fread(mem,1,fl,fp);
    BYTE *minimum=mem+1;
    BYTE *maximum=mem+fl;
    BYTE *c=minimum;
    int Field=0,BytesInField=0;
    int Modifiers[2]={0,0};
    int nEntries1=0,nEntries2=0;
    int &PcModifiers=Modifiers[0];
    int &StModifiers=Modifiers[1];
    bool Comment=false;
    char Char='\0';
    BYTE AnsiCode=0,Scancode=0;
    do {
      BYTE b0=*(c-1);
      BYTE b1=*c;
      if(b0=='#' && b1!=';')
        Comment=true; // ignore rest of line
      else if(/*b0=='\r' &&*/ b1=='\n') // newline, n or rn
      {
        if(Field==5)
        {
          if(PcModifiers||StModifiers)
          {
            TRACE("AddToShiftSwitchTable PC %c %X (%X) ST %X (%X)\n",Char,AnsiCode,PcModifiers,Scancode,StModifiers);
            nEntries2++;
            AddToShiftSwitchTable(PcModifiers,AnsiCode,(BYTE)StModifiers,Scancode);
          }
          else
          {
            TRACE("SetSTKeys %c %X %X\n",AnsiCode,AnsiCode,Scancode);
            nEntries1++;
            SetSTKeys(AnsiCode,Scancode,-1);
          }
        }
        Comment=false;
        Field=PcModifiers=StModifiers=BytesInField=0;
        AnsiCode=Scancode=0;
        Char='\0';
      }
      else if(!Comment)
      {
        switch(Field) {
        case 0:
          if(b1==';' && BytesInField) // Char can be ;
          {
            switch(BytesInField) {
            case 1: // ANSI encoding
              Char=b0;
              break;
            case 2: // UTF-8 2 bytes, assume 8bit, convert to ANSI
              Char=(b0&0x3F) | (((*(c-2))&3)<<6);
              break;
            }//sw
          }
          break;
        case 1:
        case 3:
          if(b1==';')
          {
            char s[3]={(char)*(c-2),(char)*(c-1),'\0'};
            BYTE code=(s[0]==' '&&s[1]==' ') ? '\0' : (BYTE)strtol(s,NULL,16);
            if(Field==3)
              Scancode=code; // ST, always
            else
              AnsiCode=(code=='\0') ? Char : code; // PC, default to Char if no code
          }
          break;
        case 2: // PC
        case 4: // ST
          if(b0=='a'||b0=='A')
            Modifiers[Field>>2]|=ALT;
          else if(b0=='s'||b0=='S')
            Modifiers[Field>>2]|=SHIFT;
          break;
        }//sw
        BytesInField++;
        if(b0==';' && BytesInField>1) // Char can be ;
        {
          Field++;
          BytesInField=0;
        }
      }//!Comment
      c++;
    } while(c>minimum && c<maximum);
    free(mem);
    fclose(fp);
    if(nEntries1||nEntries2)
      ok=true;
  }
  return ok;
}

#endif


void InitKeyTable() {
  long Language,SubLang;
  DestroyKeyTable();
  ShiftSwitchingAvailable=0;
  if(EnableShiftSwitching)
  {
    for(int i=0;i<4;i++)
    {
      // i: BIT 0=shift, BIT 1=alt
      shift_key_table[i]=(WORD*)malloc(sizeof(WORD)*256);
      ZeroMemory(shift_key_table[i],sizeof(WORD)*256);
    }
  }

#ifdef WIN32
  Language=PRIMARYLANGID(KeyboardLangID);
  SubLang=SUBLANGID(KeyboardLangID);
  /*
   Any keys with VK_ constants plus 'A'-'Z' and '0' to '9' will be the same
   on all keyboards/languages, these are set up in ikbd.h. The rest are put
   into the key_table here.

   NOTE: On Windows SetSTKeys doesn't put it in the table if modifiers are
         required to produce the characters.
  */
  SetSTKeys("-=[];" "\'",0x0c,0x0d,0x1a,0x1b,0x27,0x28);
  keyboard_layout=GetKeyboardLayout(0); // current keyboard
  // reinit
  SetSTKeys("1234567890-=",0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD);
  SetSTKeys("qwertyuiop[]",0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B);
  SetSTKeys("asdfghjkl;'",0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28);
  SetSTKeys("zxcvbnm,./",0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,0x35);
  SetSTKeys("`#,./",0x29,0x2b,0x33,0x34,0x35);
#endif//WIN32

#ifdef UNIX
  Language=LOWORD(KeyboardLangID);
  SubLang=HIWORD(KeyboardLangID);
  ZeroMemory(key_table,sizeof(key_table));
  SetSTKey(XK_Escape,0x1);
  SetSTKey(XK_grave,0x29);
  SetSTKey(XK_BackSpace,0xE);
  SetSTKey(XK_Tab,0xF);
  SetSTKey(XK_Return,0x1C);
  SetSTKey(XK_Delete,0x53);
  key_table[VK_CONTROL]=0x1d;
  SetSTKey(XK_Control_L,0x1D);
  SetSTKey(XK_Control_R,0x1D);
  // Should never get VK_SHIFT
  SetSTKey(XK_Shift_L,0x2A);
  SetSTKey(XK_Shift_R,0x36);
  key_table[VK_MENU]=0x38;
  SetSTKey(XK_Alt_L,0x38);
  SetSTKey(XK_Alt_R,0x38);
  SetSTKey(XK_space,0x39);
  SetSTKey(XK_Caps_Lock,0x3A);
  SetSTKey(XK_F1,0x3B);
  SetSTKey(XK_F2,0x3C);
  SetSTKey(XK_F3,0x3D);
  SetSTKey(XK_F4,0x3E);
  SetSTKey(XK_F5,0x3F);
  SetSTKey(XK_F6,0x40);
  SetSTKey(XK_F7,0x41);
  SetSTKey(XK_F8,0x42);
  SetSTKey(XK_F9,0x43);
  SetSTKey(XK_F10,0x44);
  SetSTKey(XK_Help,0x62);
  SetSTKey(XK_Undo,0x61);
  // If you don't have help and undo keys use Page Up and Page Down
  SetSTKey(XK_Page_Up,0x62);
  SetSTKey(XK_Page_Down,0x61);
  SetSTKey(XK_Insert,0x52);
  SetSTKey(XK_Home,0x47);
  SetSTKey(XK_Up,0x48);
  SetSTKey(XK_Left,0x4B);
  SetSTKey(XK_Down,0x50);
  SetSTKey(XK_Right,0x4D);
  SetSTKey(XK_F11,0x63);  // (
  SetSTKey(XK_F12,0x64);  // )
  SetSTKey(XK_KP_Divide,0x65);
  SetSTKey(XK_KP_Multiply,0x66);
  SetSTKey(XK_KP_7,0x67);
  SetSTKey(XK_KP_8,0x68);
  SetSTKey(XK_KP_9,0x69);
  SetSTKey(XK_KP_Subtract,0x4A);
  SetSTKey(XK_KP_4,0x6A);
  SetSTKey(XK_KP_5,0x6B);
  SetSTKey(XK_KP_6,0x6C);
  SetSTKey(XK_KP_Add,0x4E);
  SetSTKey(XK_KP_1,0x6D);
  SetSTKey(XK_KP_2,0x6E);
  SetSTKey(XK_KP_3,0x6F);
  SetSTKey(XK_KP_0,0x70);
  SetSTKey(XK_KP_Decimal,0x71);
  SetSTKey(XK_KP_Enter,0x72);
#endif//UNIX

  switch(Language) {
  case LANG_ENGLISH:
    if(SubLang==SUBLANG_ENGLISH_AUS)
    {
      // missing: good ST keyboard pic
      SetSTKeys("\\",0x60);  // # not unshifted on Aus keyboard, might overwrite something if we map
      AddToShiftSwitchTable(PC_SHIFT,'2',ST_SHIFT,0x28);     // Shift+"2" = Shift+"'" = "@"
      AddToShiftSwitchTable(PC_SHIFT,'\'',ST_SHIFT,0x3);     // Shift+"'" = Shift+"2" =  "
      AddToShiftSwitchTable(PC_SHIFT,'3',NO_ST_SHIFT,0x2b);  // Shift+"3" =       "#" = "#"
      AddToShiftSwitchTable(PC_SHIFT,'`',ST_SHIFT,0x2b);     // Shift+"`" = Shift+"#" = "~"
    }
    else if(SubLang==SUBLANG_ENGLISH_UK)
      SetSTKeys("\\" "#",0x60,0x2b);
    else // US (only? 2b is # above)
      // missing: good ST keyboard pic
      SetSTKeys("\\",0x2b);
    break;
  // Here we use Windows-1252 numbers instead of characters so that even if
  // various text editors mess the encoding, Steem will not be bugged
  // + some other fixes/bugs
  // Apparently Unicode UTF-8 has the same numbers but encoded on two bytes
  case LANG_FRENCH:
    if(SubLang==SUBLANG_FRENCH_BELGIAN) // PC keyboard is AZERTY Belgian
    { //          &        é        "       '        (
      SetSTKeys('&',0x2,0xe9,0x3,'\"',0x4,'\'',0x5,'(',0x6,-1);
      //           §        è       !        ç        à       )       -  
      SetSTKeys(0xa7,0x7,0xe8,0x8,'!',0x9,0xe7,0xa,0xe0,0xb,')',0xc,'-',0xd,-1);
      SetSTKeys("az^$",0x10,0x11,0x1a,0x1b);
      //          q        m         ù         µ
      SetSTKeys('q',0x1e,'m',0x27,0xf9,0x28,0xb5,0x29,-1);
      //                    <    w    ,    ;    :    =
      SetSTKeys("<w,;:=",0x60,0x2c,0x32,0x33,0x34,0x35);
      AddToShiftSwitchTable(PC_ALT,'&',ST_SHIFT,0x2b);        // |
      AddToShiftSwitchTable(PC_ALT,0xe9,ST_ALT,0x2b);          // é -> @
      AddToShiftSwitchTable(PC_ALT,'\"',0,0x2b);               // " -> #
      AddToShiftSwitchTable(PC_ALT,0xe7,ST_ALT+ST_SHIFT,0x1a); // ç -> {
      AddToShiftSwitchTable(PC_ALT,0xe0,ST_ALT+ST_SHIFT,0x1b); // à -> }
      AddToShiftSwitchTable(PC_ALT,'^',ST_ALT,0x1a);          // [
      AddToShiftSwitchTable(PC_ALT,'$',ST_ALT,0x1b);          // ]
      AddToShiftSwitchTable(PC_ALT,0xb5,0,0x29);               // µ -> `
      AddToShiftSwitchTable(PC_ALT,'<',ST_ALT,0x28);          /* \ */
      AddToShiftSwitchTable(PC_ALT,'=',ST_ALT+ST_SHIFT,0x2b); // ~
    }
    else // PC keyboard is AZERTY French
    { 
      //          &        é       "         '       (
      SetSTKeys('&',0x2,0xe9,0x3,'\"',0x4,'\'',0x5,'(',0x6,-1);
      //           è                ç        à       )      =      
      SetSTKeys(0xe8,0x8,'!',0x9,0xe7,0xa,0xe0,0xb,')',0xc,'=',0x35,-1);
      //SetSTKeys('-',0x7,'_',0x9,-1);
      // French ST keyboard has § on 6 key
      // French PC keyboard has - on 6 key, if player presses - for 6 he's screwed!
      // French ST keyboard has ! on 8 key
      // French PC keyboard has _ on 8 key, if player presses _ for 8 he's screwed!
      SetSTKeys("-az^$",0xd,0x10,0x11,0x1a,0x1b);
      //          q      m       ù
      SetSTKeys('q',30,'m',39,0xf9,40,'*',0x66,-1);
      //                    <    w    ,    ;    :
      SetSTKeys("<w,;:",0x60,0x2c,0x32,0x33,0x34);
//      AddToShiftSwitchTable(0,'-',0,0xd);                // -
      AddToShiftSwitchTable(PC_SHIFT,'-',ST_SHIFT,0x7);  // 6
      AddToShiftSwitchTable(0,'_',ST_SHIFT,0xd);         // _
      AddToShiftSwitchTable(PC_SHIFT,'_',ST_SHIFT,0x9);  // 8
      AddToShiftSwitchTable(0,'!',0,0x9);                // !
      AddToShiftSwitchTable(PC_SHIFT,'!',0,0x7);         // §
      AddToShiftSwitchTable(PC_SHIFT,'$',ST_SHIFT,0x29); // £
      AddToShiftSwitchTable(PC_ALT,0xe9,ST_ALT+ST_SHIFT,0x2b);  // é -> ~
      AddToShiftSwitchTable(PC_ALT,'\"',0,0x2b);                // " -> #
      AddToShiftSwitchTable(PC_ALT,'\'',ST_ALT+ST_SHIFT,0x1a);  // ' -> {
      AddToShiftSwitchTable(PC_ALT,'(',ST_ALT,0x1a);           // [
      AddToShiftSwitchTable(PC_ALT,'-',ST_SHIFT,0x2b);         // |
      AddToShiftSwitchTable(PC_ALT,0xe8,0,0x29);                // è -> `
      AddToShiftSwitchTable(PC_ALT,'_',ST_ALT,0x28);           /* \ */
//      AddToShiftSwitchTable(PC_ALT,0xe7,0,0x1a);                // ç -> ^
      AddToShiftSwitchTable(PC_ALT,0xe0,ST_ALT,0x2b);           // à -> @
      AddToShiftSwitchTable(PC_ALT,')',ST_ALT,0x1b);           // ]
      AddToShiftSwitchTable(PC_ALT,'=',ST_ALT+ST_SHIFT,0x1b);  // }
//      AddToShiftSwitchTable(PC_ALT,'$',ST_SHIFT,0xc);          // ¤ !
    }
    break;
  case LANG_GERMAN:    
    //           ß        ü         ö         ä
    SetSTKeys(0xdf,0xc,0xfc,0x1a,0xf6,0x27,0xe4,0x28,-1);
    SetSTKeys("z+#~y-<",0x15,0x1b,0x29,0x2b,0x2c,0x35,0x60);
    /*                                                          ___
      Key #220 = ASCII '^' (#94) = ST keycode 0x2b             ; / ;
      Key #221 = ASCII '´' (#180) = ST keycode 0xd        #180=;   ;
    */
#ifdef WIN32
    //SetSTKeys('^',0x2b,'´',0xd,-1); // 2b is ~
    //           ´
    //SetSTKeys(0xb4,0xd,-1); // d is '  ?? - see below
#else
    //SetSTKey(XK_dead_circumflex,0x2b,true);  // 2b is ~
    //SetSTKey(XK_dead_acute,0xd,true);
#endif
    /*
      Shift + Key #191 = ASCII '#' (#35) = ST keycode No Shift+ 0xd
      No shift + Key #220 = ASCII '^' (#94) = ST keycode Shift+ 0x29
      Shift + Key #220 = ASCII '^' (#94) = ST keycode No Shift+ 0x2b
    */
    AddToShiftSwitchTable(PC_SHIFT,'#',NO_ST_SHIFT,0xd);  // '
    AddToShiftSwitchTable(NO_PC_SHIFT,'^',ST_SHIFT,0x29); // ^ // left of 1
//    AddToShiftSwitchTable(PC_SHIFT,'^',NO_ST_SHIFT,0x2b); // ~ //?
    // PC alt to no ST alt
    AddToShiftSwitchTable(PC_ALT,'+',0,0x2b);        /* ~ */
    AddToShiftSwitchTable(PC_ALT,'<',ST_SHIFT,0x2b); /* | */
    // PC alt to ST alt (but moved)
    AddToShiftSwitchTable(PC_ALT,'q',ST_ALT,0x1a);           /* @ */
    AddToShiftSwitchTable(PC_ALT,0xdf,ST_ALT+ST_SHIFT,0x1a);  /* ß -> \ */
    AddToShiftSwitchTable(PC_ALT,'8',ST_ALT,0x27);           /* [ */
    AddToShiftSwitchTable(PC_ALT,'9',ST_ALT,0x28);           /* ] */
    AddToShiftSwitchTable(PC_ALT,'7',ST_ALT+ST_SHIFT,0x27);  /* { */
    AddToShiftSwitchTable(PC_ALT,'0',ST_ALT+ST_SHIFT,0x28);  /* } */
    break;
  case LANG_SPANISH:
  case LANG_CATALAN:
  case LANG_BASQUE:
    // missing: good ST keyboard pic
    //           '         ñ         ç         \                 //
    SetSTKeys('\'',0x1a,0xf1,0x27,0xe7,0x29,'\\',0x2b,-1);
    //             °
//    SetSTKeys(0xb0,0x35,-1);
    // '^',0x1b
    SetSTKeys("`;.<[{=",0x1b,0x28,0x34,0x60,0xc,0x28,0x35);
//    SetSTKeys("\'" "`´ñ;ç" "\\" ".°<[{^",26,0x1b,0x1a,39,40,41,43,0x34,0x35,0x60,0x0c,0x28,0x1b);
    AddToShiftSwitchTable(PC_SHIFT,'1',ST_SHIFT,0x34);               // !
    AddToShiftSwitchTable(PC_SHIFT,'2',ST_SHIFT,0x1a);               // "
    AddToShiftSwitchTable(PC_SHIFT,'3',NO_ST_SHIFT,0x71);            // · (central .)
    AddToShiftSwitchTable(PC_SHIFT,'6',ST_SHIFT,0x8);                // &
    AddToShiftSwitchTable(PC_SHIFT,'7',ST_SHIFT,0x7);                // /
    AddToShiftSwitchTable(PC_SHIFT,'8',ST_SHIFT,0xa);                // (     ___
    AddToShiftSwitchTable(PC_SHIFT,'9',ST_SHIFT,0xb);                // )    ; o ;
    AddToShiftSwitchTable(PC_SHIFT,'\''/*39 */,ST_SHIFT,0x33);    // ?    ;   ;
    AddToShiftSwitchTable(NO_PC_SHIFT,0xba,NO_ST_SHIFT,0x35); // º     --- #186
//    AddToShiftSwitchTable(PC_SHIFT,0xba,NO_ST_SHIFT,0x2b); /* \ */
    AddToShiftSwitchTable(NO_PC_SHIFT,0xa1,ST_SHIFT,0x2);  // ¡
    AddToShiftSwitchTable(PC_SHIFT,0xa1,ST_SHIFT,0x3);  // ? (upside down)
    AddToShiftSwitchTable(NO_PC_SHIFT,'+',ST_SHIFT,0xd);  // +     ___
    AddToShiftSwitchTable(PC_SHIFT,'+',NO_ST_SHIFT,0x66); // *    ; . ; //on numpad
    AddToShiftSwitchTable(PC_SHIFT,',',NO_ST_SHIFT,0x28); // ;    ; | ;
    AddToShiftSwitchTable(PC_SHIFT,'.',ST_SHIFT,0x28); // :     --- #161
    AddToShiftSwitchTable(PC_SHIFT,'0',NO_ST_SHIFT,0x0d); // =
    AddToShiftSwitchTable(PC_ALT,0xba,0,0x2b);              /* º  \ */
    AddToShiftSwitchTable(PC_ALT,'1',ST_SHIFT,0x2b);                // |    ___
    AddToShiftSwitchTable(PC_ALT,'2',ST_ALT+ST_SHIFT,0x2b);         // @   ; C ;
    AddToShiftSwitchTable(PC_ALT,'3',ST_ALT,0x2b);                  // #   ; j ;
    AddToShiftSwitchTable(PC_ALT,0x60,ST_ALT,0x1a);                  // alt-`-> [    --- #231
    AddToShiftSwitchTable(PC_ALT,'+',ST_ALT,0x1b);                  // ]
    AddToShiftSwitchTable(PC_ALT,0xb4,ST_ALT+ST_SHIFT,0x1a); // alt-´ -> { 180=horz flip `
    AddToShiftSwitchTable(PC_ALT,0xe7,ST_ALT+ST_SHIFT,0x1b); // alt-ç -> }
    AddToShiftSwitchTable(PC_ALT,'4',ST_SHIFT,0x29); // alt-4 -> ~
    break;
  case LANG_ITALIAN:
    // missing: good ST keyboard pic
    //          ì         è         ò         à         ù         \          //
    SetSTKeys(0xec,0xd,0xe8,0x1a,0xf2,0x27,0xe0,0x28,0xf9,0x29,0x5c,0x2b,-1);
    SetSTKeys(".-<'+",0x34,0x35,0x60,0xc,0x1b);
    break;
  case LANG_SWEDISH:
    // missing: good ST keyboard pic
    //                 é          å         ü         ö         ä
    SetSTKeys('+',0xc,0xe9,0xd,0xe5,0x1a,0xfc,0x1b,0xf6,0x27,0xe4,0x28,-1);
    //         '     \ 
    SetSTKeys('\'',0x29,'\\',0x2b,'-',0x35,'<',0x60,-1);
    AddToShiftSwitchTable(PC_ALT,'+',NO_ST_SHIFT,0x2b); /* \ */
    AddToShiftSwitchTable(PC_ALT,'2',ST_ALT,0x2b);      // @
//    AddToShiftSwitchTable(PC_ALT,'3',0,0);      // £
//    AddToShiftSwitchTable(PC_ALT,'4',0,0);      // $
    AddToShiftSwitchTable(PC_ALT,'7',ST_ALT+ST_SHIFT,0x1a);      // {
    AddToShiftSwitchTable(PC_ALT,'8',ST_ALT,0x1a);      // [
    AddToShiftSwitchTable(PC_ALT,'9',ST_ALT,0x1b);      // ]
    AddToShiftSwitchTable(PC_ALT,'0',ST_ALT+ST_SHIFT,0x1b);      // }
    break;
  case LANG_NORWEGIAN:
    // missing: good ST keyboard pic
    //           ß        '        ü         ö         ä
    SetSTKeys(0xdf,0xc,'\'',0xd,0xfc,0x1a,0xf6,0x27,0xe4,0x28,-1);
    SetSTKeys("z#~y-<+",0x2c,0x29,0x2b,0x15,0x35,0x60,0x1b);
    {
      /*
        But the key #186 has a ^ shifted, not £. There is one key that didn't
        come out: key 219 = \, and ` shifted, and one for which
        I didn't find the right unshifted key: key #220, which is |, shifted §.

        Key #186 = ASCII ¨ (#-88) = ST keycode 0x1b
        Key #187 = ASCII + (#43) = ST keycode 0xc
        Key #191 = ASCII ' (#39) = ST keycode 0x29
        Key #192 = ASCII ø (#-8) = ST keycode 0x28
        Key #221 = ASCII å (#-27) = ST keycode 0x1a
        Key #222 = ASCII æ (#-26) = ST keycode 0x27
      */
//      char char_list[]={(char)168,43,39,(char)248,(char)229,(char)230,0};
//      SetSTKeys(char_list,0x1b,0xc,0x29,0x28,0x1a,0x27);
    }
    break;
#if defined(SSE_IKBD_MAPPINGFILE)
  case LANG_CUSTOM:
    ReadMappingFile(KeyboardMappingPath.Text);
    break;
#endif

  }//sw

#ifndef STEEMKEYTEST
  if(ShiftSwitchingAvailable==0) 
    DestroyKeyTable();
#endif
}

#ifdef UNIX

void UNIX_get_fake_VKs() {
  VK_LBUTTON=0xf0;VK_RBUTTON=0xf1;VK_MBUTTON=0xf2;
  VK_F11=XKeysymToKeycode(XD,XK_F11);
  VK_F12=XKeysymToKeycode(XD,XK_F12);
  VK_END=XKeysymToKeycode(XD,XK_End);
  VK_LEFT=XKeysymToKeycode(XD,XK_Left);
  VK_RIGHT=XKeysymToKeycode(XD,XK_Right);
  VK_UP=XKeysymToKeycode(XD,XK_Up);
  VK_DOWN=XKeysymToKeycode(XD,XK_Down);
  VK_TAB=XKeysymToKeycode(XD,XK_Tab);
  VK_SHIFT=0xf3;
  VK_LSHIFT=XKeysymToKeycode(XD,XK_Shift_L);
  VK_RSHIFT=XKeysymToKeycode(XD,XK_Shift_R);
  VK_CONTROL=0xf4;
  VK_RCONTROL=XKeysymToKeycode(XD,XK_Control_R);
  VK_LCONTROL=XKeysymToKeycode(XD,XK_Control_L);
  VK_MENU=0xf5;
  VK_LMENU=XKeysymToKeycode(XD,XK_Alt_L);
  VK_RMENU=XKeysymToKeycode(XD,XK_Alt_R);
  VK_SCROLL=0xf6;
  VK_NUMLOCK=0xf7;
  Key_Pause=XKeysymToKeycode(XD,XK_Pause);
}

#endif
