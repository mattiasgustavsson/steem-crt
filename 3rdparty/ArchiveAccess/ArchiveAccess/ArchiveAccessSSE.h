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
FILE: ArchiveAccessSSE.h
DESCRIPTION: Declarations for the interface with the archiveaccess plugin.
---------------------------------------------------------------------------*/

#pragma once
#ifndef ARCHIVEACCESSSSE_H
#define ARCHIVEACCESSSSE_H

#if defined(SSE_ARCHIVEACCESS_SUPPORT)

#include "bgstr.h"
#ifdef MINGW_BUILD
#define NULL 0
#endif
#include "archiveaccessbase.h"

//#include "../ArchiveAccess/ArchiveAccessDynamic.h"
// check that file for some C++ horror :(

#ifndef ArchiveAccess__H // no choice
#pragma pack(push, 8) // bcc problem #2
#define FileInArchiveInfoStringSize 1024
struct FileInArchiveInfo {
	int ArchiveHandle; // handle for Archive/class pointer
    //int FileIndex;
	UINT64 CompressedFileSize;
	UINT64 UncompressedFileSize;
	UINT32 attributes;
	bool IsDir, IsEncrypted;
	_FILETIME LastWriteTime, CreationTime, LastAccessTime;
	unsigned short path[FileInArchiveInfoStringSize];
};
#pragma pack(pop)
#endif
typedef void* aaHandle;
extern FileInArchiveInfo FileInfo;
extern BYTE current_archived_file;
extern aaHandle ArchiveHandle;

bool LoadArchiveAccessDll (const TCHAR* LibName);
void UnloadArchiveAccessDll();

bool ArchiveAccess_Open(TCHAR* name) ; // returns true = success
void ArchiveAccess_Close();
bool ArchiveAccess_Extract(char *dest_dir);
bool ArchiveAccess_Select(int n);

#endif//#if defined(SSE_ARCHIVEACCESS_SUPPORT)

#endif//#ifndef ARCHIVEACCESSSSE_H

