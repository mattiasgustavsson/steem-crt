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

DOMAIN: IO
FILE: portio.h
DESCRIPTION: Cross-platform direct port input and output class.
---------------------------------------------------------------------------*/

#pragma once
#ifndef TPORTIO_H
#define TPORTIO_H

#ifdef UNIX

#define TPORTIO_NUM_TYPES 5
// Don't change these numbers!
#define TPORTIO_TYPE_SERIAL 0
#define TPORTIO_TYPE_PARALLEL 1
#define TPORTIO_TYPE_MIDI 2
#define TPORTIO_TYPE_UNKNOWN 3
#define TPORTIO_TYPE_PIPE 4
#define RTS_CONTROL_DISABLE    0x00
#define RTS_CONTROL_ENABLE     0x01
#define RTS_CONTROL_HANDSHAKE  0x02
#define RTS_CONTROL_TOGGLE     0x03
#define DTR_CONTROL_DISABLE    0x00
#define DTR_CONTROL_ENABLE     0x01
#define ONESTOPBIT 0
#define ONE5STOPBITS 1
#define TWOSTOPBITS 2
#define EVENPARITY 2
#define ODDPARITY 1
#define MS_CTS_ON           ((DWORD)0x0010)
#define MS_DSR_ON           ((DWORD)0x0020)
#define MS_RING_ON          ((DWORD)0x0040)
#define MS_RLSD_ON          ((DWORD)0x0080)

#endif

#include "circularbuffer.h"

typedef void PORTIOINFIRSTBYTEPROC();
typedef void PORTIOOUTFINISHEDPROC();
typedef PORTIOINFIRSTBYTEPROC* LPPORTIOINFIRSTBYTEPROC;
typedef PORTIOOUTFINISHEDPROC* LPPORTIOOUTFINISHEDPROC;

#ifdef WIN32
typedef BOOL WINAPI CANCELIOPROC(HANDLE);
typedef CANCELIOPROC* LPCANCELIOPROC;
#endif

#pragma pack(push, 8)

class TPortIO {
private:
#ifdef WIN32
  static DWORD CALLBACK InThreadEntryPoint(void*);
  static DWORD CALLBACK OutThreadEntryPoint(void*);
  HANDLE hCom,hInThread,hOutThread;
#else
  static void* InThreadFunc(void*);
  static void* OutThreadFunc(void*);
	int iCom;
	pthread_t iInThread,iOutThread;
	pthread_cond_t OutWaitCond;
	pthread_mutex_t OutWaitMutex;
	int Type;
#endif
  volatile bool InThreadClosed,OutThreadClosed;
  CircularBuffer InpBuf,OutBuf;
  volatile bool Outputting,Closing;
public:
#ifdef WIN32
  TPortIO(char* = NULL,bool=true,bool=true);
  int Open(char*,bool=true,bool=true);
#else
  TPortIO(char* = NULL,bool=true,bool=true,int=3);
  int Open(char*,bool=true,bool=true,int=3);
#endif
  ~TPortIO();
  void SetupCOM(int,bool,int,int,bool,BYTE,BYTE,BYTE);
  bool OutputByte(BYTE),OutputString(char*);
  DWORD GetModemFlags();
  BYTE ReadByte();
  void NextByte();
  bool AreBytesToRead();
  bool AreBytesToOutput();
  void Close();
  void SetPause(bool,bool);
  bool StartBreak(),EndBreak();
  bool SetDTR(bool),SetRTS(bool);
  bool IsOpen();
#ifdef WIN32
  HANDLE Handle();
  LPCANCELIOPROC pCancelIOProc;
  HINSTANCE hKern32;
  bool WinNT;
  static bool AlwaysUseNTMethod;
  HANDLE hOutEvent,hInEvent;
  OVERLAPPED OutOverlapStruct,InOverlapStruct;
  LPOVERLAPPED lpOutOverlapStruct,lpInOverlapStruct;
#endif
  LPPORTIOINFIRSTBYTEPROC lpInFirstByteProc;
  LPPORTIOOUTFINISHEDPROC lpOutFinishedProc;
  volatile bool OutPause,InPause;
  volatile int OutCount,InCount;
};

#pragma pack(pop)

#endif//#ifndef TPORTIO_H

