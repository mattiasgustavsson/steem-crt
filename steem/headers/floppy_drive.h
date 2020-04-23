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

DOMAIN: disk image
FILE: floppy_drive.h
DESCRIPTION: Declarations for floppy drive emulation.
struct TSF314
---------------------------------------------------------------------------*/

#pragma once
#ifndef SSEDRIVE_H
#define SSEDRIVE_H

#include <easystr.h>
#include "floppy_disk.h"


#pragma pack(push, 8)

struct TSF314 {
  // ENUM
#if defined(SSE_DRIVE_SOUND)
  enum EDriveSound {START,MOTOR,STEP,SEEK,NSOUNDS} ;
#endif
  // FUNCTIONS
  TSF314();
  ~TSF314();
  WORD BytePosition(); //this has to do with IP and rotation speed
  WORD BytesToHbls(int bytes);
#if defined(SSE_DISK_GHOST)
  bool CheckGhostDisk(bool write);
#endif
  DWORD HblsAtIndex();
  WORD HblsNextIndex();
  WORD HblsPerRotation();
  WORD HblsToBytes(WORD hbls);
  void Init(),Restore(BYTE myid);
  void UpdateAdat();
  int CyclesPerByte();
  void IndexPulse(bool image_triggered=false);
  void Motor(bool state);
  void Read();
  void Step(int direction);
  void Write();
  int SetDisk(EasyStr,EasyStr="",TBpbInfo* = NULL,TBpbInfo* = NULL);
  bool DiskInDrive() { return m_DiskInDrive; }
  bool NotEmpty() { return DiskInDrive(); }
  bool Empty()       { return DiskInDrive()==0; }
  void RemoveDisk(bool LoseChanges=0);
  bool ReinsertDisk();
  EasyStr GetDisk();
#if defined(SSE_DRIVE_SOUND)
  void Sound_LoadSamples(IDirectSound *DSObj,DSBUFFERDESC *dsbd,WAVEFORMATEX *wfx);
  void Sound_ReleaseBuffers();
  void Sound_StopBuffers();
  void Sound_CheckCommand(BYTE cr);
  void Sound_CheckIrq();
  void Sound_CheckMotor();
  void Sound_ChangeVolume();
  void Sound_Step();
#endif//sound
  // DATA
  TImageMfm *MfmManager; //polymorphic
#if defined(SSE_DRIVE_SOUND)
#ifdef WIN32 // moved out of the struct
//  IDirectSoundBuffer *Sound_Buffer[NSOUNDS]; // fixed array
#endif
  int Sound_Volume;
#endif
  int cycles_per_byte;
  COUNTER_VAR time_of_next_ip;
  COUNTER_VAR time_of_last_ip;
  TImageType ImageType;
  WORD SectorChecksum; // meta
  BYTE Id; // object has to know if its A: (0) or B: (1)
  BYTE track;
#if defined(SSE_DRIVE_SOUND)
  BYTE old_track;
#endif
  bool motor;
  bool single_sided; // so it becomes a SF354!
  bool freeboot; // read other side as if single-sided
  bool ghost;
  bool reading;
  bool writing;
  bool adat;
  bool m_DiskInDrive;
};

#pragma pack(pop)

#if defined(SSE_DRIVE_SOUND)
extern EasyStr DriveSoundDir[2];
#endif

#endif//#ifndef SSEDRIVE_H
