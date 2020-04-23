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
FILE: easycompress.cpp
DESCRIPTION: Basic compression facility.
---------------------------------------------------------------------------*/
#define MIN_PACK_LENGTH 4  //If we have 4 words the same in a row then pack them

//#pragma warning (disable: 4701) //SameLen==0 if break; //390

#ifdef UNIX
#include <../pch.h>
#endif

#include <conditions.h>
#include <stdio.h>

void EasyCompressFromMem(void *Buf,long Len,FILE *f) {
  WORD *Mem=(WORD*)Buf,*SearchAdr,*MemEnd=(WORD*)((LONG_PTR)(Buf)+Len);
  WORD ChangedLen,SameLen;
  WORD Last,This;
  WORD Version=0;
  fwrite(&Version,1,2,f);
  while(Mem<MemEnd)
  {
    Last=0;
    ChangedLen=0;
    SameLen=0;
    SearchAdr=Mem;
    for(;;)
    {
      if(SearchAdr>=MemEnd)
      {
        if(SameLen)
        {
          ChangedLen+=SameLen; //They weren't the same for long enough
          SameLen=0;
        }
        break;
      }
      This=*SearchAdr;
      if(This==Last)
      {
        SameLen++;
        if(SameLen>=MIN_PACK_LENGTH) 
          break;
      }
      else
      {
        if(SameLen)
        {
          ChangedLen+=SameLen;  //They weren't the same for long enough
          SameLen=0;
        }
        ChangedLen++;
        if(ChangedLen>=16000)
          break;
      }
      Last=This;
      SearchAdr++;
    }//nxt
    if(ChangedLen)
    {
      fwrite(&ChangedLen,1,2,f);
      fwrite(Mem,1,ChangedLen*2,f);
      Mem+=ChangedLen;
    }
    if(SameLen)
    {
      SearchAdr=Mem+SameLen;  //SearchAdr is just after the last character searched
      while(*SearchAdr==This && SameLen<16000&&SearchAdr<MemEnd)
      {
        SameLen++;
        SearchAdr++;
      }
      Mem+=SameLen;
      SameLen|=0x8000; //Set high bit
      fwrite(&SameLen,1,2,f);
      fwrite(&This,1,2,f);
    }
  }
  SameLen=0xffff;
  fwrite(&SameLen,1,2,f);
}

//#pragma warning (default: 4701)

#define EASYCOMPRESS_BUFFERTOSMALL 1
#define EASYCOMPRESS_CORRUPTFILE 2

int EasyUncompressToMem(void *Buf,int Len,FILE* &f,bool FIsMem) {
  WORD *Mem=(WORD*)Buf,*MemEnd=(WORD*)((LONG_PTR)(Buf)+Len),Desc,NumWords;
  WORD *p=(WORD*)f;
  WORD Version=0xffff;
  if(FIsMem==0) 
    fread(&Version,1,2,f);
  if(FIsMem) 
    Version=*(p++);
  if(Version!=0) 
    return EASYCOMPRESS_CORRUPTFILE;
  for(;;)
  {
    if(FIsMem)
      Desc=*(p++);
    else if(fread(&Desc,1,2,f)<2)
      return EASYCOMPRESS_CORRUPTFILE;
    if(Desc==0xffff) 
      break;
    if(Desc&0x8000)
    {
      NumWords=Desc & (WORD)0x7fff;
      if(Mem+NumWords>MemEnd)
        return EASYCOMPRESS_BUFFERTOSMALL;
      if(FIsMem==0) fread(&Desc,1,2,f);
      if(FIsMem) Desc=*(p++);
      for(WORD n=0;n<NumWords;n++) *(Mem++)=Desc;
    }
    else
    {
      NumWords=Desc;
      if(Mem+NumWords>MemEnd)
        return EASYCOMPRESS_BUFFERTOSMALL;
      if(FIsMem==0)
      {
        fread(Mem,1,NumWords*2,f);
        Mem+=NumWords;
      }
      else
      {
        for(int n=0;n<NumWords;n++) *(Mem++)=*(p++);
      }
    }
  }
  if(FIsMem)
    f=(FILE*)p;
  return 0;
}


int EasyUncompressToMemFromMem(void *Buf,int Len,BYTE* &pByte) {
  FILE *f=(FILE*)pByte;
  int Ret=EasyUncompressToMem(Buf,Len,f,true);
  pByte=(BYTE*)f;
  return Ret;
}

