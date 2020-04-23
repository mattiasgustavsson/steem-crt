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
FILE: dirsearch.h
DESCRIPTION: Cross-platform class to retrieve directory contents.
---------------------------------------------------------------------------*/

#pragma once
#ifndef DIRSEARCH_H
#define DIRSEARCH_H

#include <conditions.h>
#include "easystr.h"

#ifdef VC_BUILD
//assignment operator could not be generated (references)
#pragma warning(disable: 4512)
#endif

#pragma pack(push, 8)

class DirSearch {
private:
#ifdef WIN32
  HANDLE hFind;
#else
  DIR *dp;
  struct dirent *ep;
  struct stat s;
  EasyStr fullpath;
  EasyStr mask;
#endif
public:
  DirSearch();
  DirSearch(char *);
  ~DirSearch();
  bool Find(char *);
  bool Next();
#ifdef WIN32
  void SetFoundFile();
#else
  void SetFoundFile(char *);
#endif
  void Close();
#ifdef WIN32
  WIN32_FIND_DATA FindDat;
  DWORD &Attrib;
  FILETIME &CreationTime;
  FILETIME &LastAccessTime;
  FILETIME &LastWriteTime;
  DWORD &SizeHigh;
  DWORD &SizeLow;
  char *Name;
  char *ShortName;
#else
  DWORD Attrib;
  DWORD CreationTime;
  DWORD LastAccessTime;
  DWORD LastWriteTime;
  DWORD SizeHigh;
  DWORD SizeLow;
  EasyStr Name;
  EasyStr ShortName;
#endif
  bool FoundFile;
  bool st_only;
};

#pragma pack(pop)

#ifdef VC_BUILD
#pragma warning(default: 4512)
#endif

#endif//#ifndef DIRSEARCH_H

