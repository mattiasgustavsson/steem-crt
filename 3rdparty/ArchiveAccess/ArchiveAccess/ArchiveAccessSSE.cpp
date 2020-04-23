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
FILE: ArchiveAccessSSE.cpp
CONDITION: SSE_ARCHIVEACCESS_SUPPORT must be defined
DESCRIPTION: Code interfacing Steem SSE with the archiveaccess plugin.
---------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////
// ArchiveAccessSSE.cpp for Steem SSE
// based on:
//
// ArchiveAccessTest.cpp 
// Copyright 2004 X-Ways Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
//
///////////////////////////////////////////////////////////////////////////////

#include <SSE.h>

#if defined(SSE_ARCHIVEACCESS_SUPPORT)

//#include "ArchiveAccessSSE.h" //no!
#include <debug.h>
#include <windows.h>
#include "../ArchiveAccess/bgstr.h"
#include "../ArchiveAccess/ArchiveAccessBase.h"
#define LOGSECTION LOGSECTION_IMAGE_INFO
#if defined(BCC_BUILD)
#define max(a,b) ( (a)>(b)?(a):(b) ) //when 1st compiling as is
#endif

#include "../ArchiveAccess/ArchiveAccessDynamic.h"


#include <archive.h>
#include <acc.h>

///////////////////////////////////////////////////////////////////////////////
#if defined(SSE_X64)
/*64-bit versions of Windows use 32-bit handles for interoperability. 
When sharing a handle between 32-bit and 64-bit applications, only the lower 
32 bits are significant, so it is safe to truncate the handle (when passing
it from 64-bit to 32-bit) or sign-extend the handle (when passing it from 
32-bit to 64-bit). Handles that can be shared include handles to user objects
such as windows (HWND), handles to GDI objects such as pens and brushes 
(HBRUSH and HPEN), and handles to named objects such as mutexes, semaphores,
and file handles.
https://docs.microsoft.com/en-us/windows/desktop/WinProg64/interprocess-communication
*/
#pragma warning( disable : 4302 4311 4312)
#endif
///////////////////////////////////////////////////////////////////////////////
// Dynamically loaded library

HINSTANCE hinstLib;

bool LoadArchiveAccessDll (const TCHAR* LibName)
{
    hinstLib = SteemLoadLibrary(LibName); 
    if (hinstLib != NULL) {
        aaDetermineArchiveType = (paaDetermineArchiveType) GetProcAddress(hinstLib, "aaDetermineArchiveType");
//		aaInit = (paaInit) GetProcAddress(hinstLib, "aaInit");
        aaOpenArchiveFile = (paaOpenArchiveFile) GetProcAddress(hinstLib, "aaOpenArchiveFile");
        aaOpenArchive = (paaOpenArchive) GetProcAddress(hinstLib, "aaOpenArchive"); 
        aaGetFileCount = (paaGetFileCount) GetProcAddress(hinstLib, "aaGetFileCount");
        aaGetFileInfo = (paaGetFileInfo) GetProcAddress(hinstLib, "aaGetFileInfo"); 
        aaExtract = (paaExtract) GetProcAddress(hinstLib, "aaExtract");
        aaCloseArchive = (paaCloseArchive) GetProcAddress(hinstLib, "aaCloseArchive");

		bool t1 = (aaDetermineArchiveType != NULL);
		bool t2 = (aaOpenArchiveFile != NULL);
		bool t3 = (aaOpenArchive != NULL);
		bool t4 = (aaGetFileCount != NULL);
		bool t5 = (aaGetFileInfo != NULL);
		bool t6 = (aaExtract != NULL);
		bool t7 = (aaCloseArchive != NULL);
		TRACE_INIT("%s loaded, t1 %d t2 %d t3 %d t4 %d t5 %d t6 %d t7 %d\n",LibName,t1,t2,t3,t4,t5,t6,t7);
		return (t1 && t2 && t3 && t4 && t5 && t6 && t7);
    } else 
    {
        TRACE_INIT("%s not available\n",LibName);
        return false;
    }
}

void UnloadArchiveAccessDll ()
{
    FreeLibrary(hinstLib); 
}

#pragma warning(disable: 4293)//'<<' : shift count negative or too big, undefined behavior//382

///////////////////////////////////////////////////////////////////////////////
// Callback function for reading data

HRESULT __stdcall readCallback (int StreamID, INT64 offset, UINT32 count, 
                                void* buf, UINT32* processedSize)
{
   unsigned long ReadBytes;
   HANDLE handle = reinterpret_cast <HANDLE> (StreamID);
   long offsHi = (long) (offset >> 32);
   //long  offsLo = (long) (offset & ((1 << 32) -1));
   long offsLo=(long)offset; // bcc problem #1
   //TRACE("o%d\n",offsLo);
   SetFilePointer (handle, offsLo, &offsHi, FILE_BEGIN);
   int result = ReadFile(handle, buf, count, &ReadBytes, NULL);
   if (processedSize != NULL)
       (*processedSize) = ReadBytes;
   if (result != 0)
     return S_OK;
   else
     return S_FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// Callback function for writing data

HRESULT __stdcall writeCallback (int StreamID, INT64 offset, UINT32 count, 
                                 const void* buf, UINT32* processedSize)
{
  int result = 1;
  if (count != 0) {
    unsigned long procSize;
    HANDLE outFileHandle = reinterpret_cast <HANDLE> (StreamID);
    long offsHi = (long) (offset >> 32);
    //long offsLo = (long) (offset & ((1 << 32) -1));
    long offsLo=(long)offset; // bcc
    SetFilePointer (outFileHandle, offsLo, &offsHi, FILE_BEGIN);
    result = WriteFile (outFileHandle, buf, count, &procSize, NULL);
    if (processedSize != NULL)
      (*processedSize) = procSize;
  }
  if (result != 0)
    return S_OK;
  else
    return S_FALSE;
}


FileInArchiveInfo FileInfo;
BYTE current_archived_file;
aaHandle ArchiveHandle=0;

void ArchiveAccess_Close() {
  if(ArchiveHandle) // or big crash...
  {
    aaCloseArchive (ArchiveHandle);
    CloseHandle(zippy.hArcData); // fixes handle leak + impossible to move file
  }
  ArchiveHandle=0;
}


bool ArchiveAccess_Extract(char *dest_dir) {
  HANDLE outFileHandle = 
    CreateFile (dest_dir, FILE_WRITE_DATA, FILE_SHARE_READ, 
    NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 
    NULL);
  UINT64 ExtractedFileSize;
  int oStreamID = reinterpret_cast <int> (outFileHandle);
  aaExtract (ArchiveHandle, current_archived_file, oStreamID, writeCallback, 
    &ExtractedFileSize);
  CloseHandle (outFileHandle);
  return true; //of course
}


bool ArchiveAccess_Open(TCHAR* ArchiveFileName) { 
  // ArchiveFileName is the full path
  bool ok=false;
  wchar_t password[] = L"xxx";
  int OpenArchiveError;
  HANDLE FileHandle = CreateFile (ArchiveFileName, FILE_READ_DATA, 
                                       FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL, NULL);
  if(FileHandle!=(void*)-1) 
  {
    // Open archive
    int iStreamID = reinterpret_cast <int> (FileHandle);
    unsigned long FileSizeHi;
    UINT32 FileSizeLo = GetFileSize (FileHandle, &FileSizeHi);
//    INT64 FileSize = (FileSizeHi << 32) + FileSizeLo;
    INT64 FileSize = FileSizeHi;
    FileSize <<= 32;
    FileSize += FileSizeLo;
    int flags;
    int ArchiveType = 
      aaDetermineArchiveType (readCallback, iStreamID, FileSize, &flags);
    ArchiveHandle = aaOpenArchive(readCallback, iStreamID, FileSize,
      ArchiveType, &OpenArchiveError, password);
    zippy.hArcData=FileHandle; // memorise this
#ifdef SSE_DEBUG
    TRACE_LOG("ArchiveAccess open %s Type %c%c%c%c flags %X: ERR %d handle:%d\n",
      ArchiveFileName,(ArchiveType>>(3*8))&0xFF,(ArchiveType>>(2*8))&0xFF,
      (ArchiveType>>8)&0xFF,ArchiveType&0xFF,flags,OpenArchiveError,iStreamID);
#endif
    if (OpenArchiveError == 0) { //OK
      current_archived_file=0; // first file
//  from ArchiveAccessClass:
// Unfortunately, not all archive formats store full information on the files 
// contained in them. If any data is missing, the function result will be set
// to S_FALSE to indicate this. In particular,
// - the file name may be missing. In this case, a valid file name may often
//   be built by truncating one extension (.bz2, .gz) from the archive file 
//   name or by appending .unpacked
// - the file size may be reported as 0 (bzip). In this case, the file size
//   can be obtained by unpacking the file to disk and reading the file size
//   from the unpacked file

      //BOOL err=
      aaGetFileInfo (ArchiveHandle,current_archived_file, &FileInfo); // was fine as is!
      //TRACE_LOG("sizeof(FileInfo)=%d\n",sizeof(FileInfo)); // 2104 vc + bcc
#if 0
      if(err==S_FALSE)
      {
        // oops it returns S_FALSE on everything? forget it then, nobody launches a bz2 file
        // at once, at least so the player could first load a disk with the correct
        // image extension, anyway, I've never seen a bz2 ST image
        //TRACE_LOG("aaGetFileInfo failed->dummy\n");
        //if(FileInfo.path==L"")
          wcscpy((wchar_t*)FileInfo.path,L"dummy.st");
        if(FileInfo.UncompressedFileSize==0)
          FileInfo.UncompressedFileSize=720*1024;
      }
#endif
      ok=true;
    }
  }
  return ok;
}


bool ArchiveAccess_Select(int n) {
  if(ArchiveHandle)
  {
    int FileCount = aaGetFileCount (ArchiveHandle);
    if(n<FileCount)
    {
      aaGetFileInfo (ArchiveHandle,n,&FileInfo);
      TRACE_LOG("select %S\n",FileInfo.path);
      return true;
    }
  }
  return false;
}

#endif//SSE_ARCHIVEACCESS_SUPPORT
