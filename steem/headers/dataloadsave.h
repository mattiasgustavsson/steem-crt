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
FILE: dataloadsave.h
DESCRIPTION: Declarations for option load/save.
struct TProfileSectionData
---------------------------------------------------------------------------*/

#pragma once
#ifndef DATALOADSAVE_DECLA_H
#define DATALOADSAVE_DECLA_H

#include <easystr.h>
#include <configstorefile.h>
#include <parameters.h>


#define PSEC_SNAP 0
#define PSEC_PASTE 1
#define PSEC_CUT 2
#define PSEC_PATCH 3
#define PSEC_MACHINETOS 4
#define PSEC_MACRO 5
#define PSEC_PORTS 6
#define PSEC_GENERAL 7
#define PSEC_SOUND 8
#define PSEC_DISPFULL 9
#define PSEC_STARTUP 10
#define PSEC_AUTOUP 11
#define PSEC_JOY 12
#define PSEC_HARDDRIVES 13
#define PSEC_DISKEMU 14
#define PSEC_POSSIZE 15
#define PSEC_DISKGUI 16
#define PSEC_PCJOY 17
#define PSEC_OSD 18
#define PSEC_STVIDEO 19
#define PSEC_NSECT 20

#pragma pack(push, 8)

struct TProfileSectionData {
  char *Name;
  int ID;
};

#pragma pack(pop)

extern TProfileSectionData ProfileSection[21];

Str ProfileSectionGetStrFromID(int ID);
void LoadAllDialogData(bool,Str,bool* = NULL,TConfigStoreFile* = NULL);
void SaveAllDialogData(bool,Str,TConfigStoreFile* = NULL);

#endif//DATALOADSAVE_DECLA_H
