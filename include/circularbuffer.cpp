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

DOMAIN: Emu
FILE: circularbuffer.cpp
DESCRIPTION: Class to implement a circular FIFO buffer.
---------------------------------------------------------------------------*/

#ifdef UNIX
#include <../pch.h>
#endif

#include "circularbuffer.h"


CircularBuffer::CircularBuffer(DWORD Size) {
  Buf=NULL;
  Lock=0;
  if(Size>=2) Create(Size);
}


CircularBuffer::~CircularBuffer() { 
  Destroy(); 
}


bool CircularBuffer::AreBytesInBuffer() {
  return bool(Buf?(pCurRead!=pCurWrite-1
    &&!(pCurRead==pEnd-1&&pCurWrite==pStart)):0);
}


BYTE CircularBuffer::ReadByte() { 
  return BYTE(Buf?*pCurRead:BYTE(0)); 
}


#ifndef WIN32
// On Windows Sleep(0) will let another thread take over instantly
int CircularBuffer::Sleep(int n) { 
  return n; 
}
#endif


bool CircularBuffer::IsLocked() { 
  return Lock; 
}


bool CircularBuffer::Create(DWORD Size) {
  if(Buf||Size<2) 
    return 0;
  try
  {
    Buf=new BYTE[Size];
  }
  catch(...)
  {
    return 0;
  }
  BufSize=Size;
  pStart=Buf;
  pEnd=Buf+BufSize;
  Reset();
  return true;
}


bool CircularBuffer::AddByte(BYTE Data) {
  if(Buf==NULL) 
    return 0;
  while(Lock) 
    Sleep(0);
  Lock=true;
  bool Overflow=(pCurRead==pCurWrite);
  *(pCurWrite++)=Data;
  if(pCurWrite>=pEnd) 
    pCurWrite=pStart;
  if(Overflow) 
    pCurRead=pCurWrite;
  Lock=0;
  return Overflow==0;
}


bool CircularBuffer::AddBytes(BYTE *pData,DWORD DataLen) {
  if(Buf==NULL||DataLen>=BufSize) 
    return 0;
  while(Lock) Sleep(0);
  Lock=true;
  bool Overflow=0;
  BYTE *pOldWrite=pCurWrite;
  if(pCurWrite+DataLen<pEnd)
  {
    pCurWrite+=DataLen;
    if(pCurRead>=pOldWrite && pCurRead<pCurWrite)
    {
      pCurRead=pCurWrite;
      Overflow=true;
    }
    Lock=0;
    memcpy(pOldWrite,pData,DataLen);
  }
  else
  {
    bool Overlap=(pCurRead>=pCurWrite);
    LONG_PTR ToEnd=pEnd-pCurWrite;
    pCurWrite=pStart+(DataLen-ToEnd);
    if(pCurRead<pCurWrite||Overlap)
    {
      pCurRead=pCurWrite;
      Overflow=true;
    }
    Lock=0;
    memcpy(pOldWrite,pData,ToEnd);
    memcpy(pStart,pData+ToEnd,DataLen-ToEnd);
  }
  return Overflow==0;
}


void CircularBuffer::NextByte() {
  while(Lock) 
    Sleep(0);
  if(AreBytesInBuffer())
  {
    if((++pCurRead)>=pEnd) 
      pCurRead=pStart;
  }
}


void CircularBuffer::Reset() {
  if(Buf==NULL) 
    return;
  while(Lock) 
    Sleep(0);
  Buf[0]=0;
  pCurRead=pStart;
  pCurWrite=pStart+1;
}


void CircularBuffer::Destroy() {
  if(Buf==NULL) 
    return;
  while(Lock) 
    Sleep(0);
  delete[] Buf;Buf=NULL;
}
