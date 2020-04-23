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
FILE: circularbuffer.h
DESCRIPTION: Declarations for circular FIFO buffer.
---------------------------------------------------------------------------*/

#pragma once
#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <conditions.h>

#pragma pack(push, 8)

class CircularBuffer {
private:
  LPBYTE Buf,pStart,pEnd,pCurRead,pCurWrite;
  DWORD BufSize;
  volatile bool Lock;
public:
  CircularBuffer(DWORD Size=0);
  ~CircularBuffer();
  bool Create(DWORD),AddByte(BYTE),AddBytes(BYTE*,DWORD);
  void Reset();
  void Destroy();
  bool AreBytesInBuffer();
  BYTE ReadByte();
  void NextByte();
#ifndef WIN32
  // On Windows Sleep(0) will let another thread take over instantly
  int Sleep(int n);
#endif
  bool IsLocked();
};

#pragma pack(pop)

#endif
