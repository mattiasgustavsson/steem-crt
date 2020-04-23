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
FILE: configstorefile.h
DESCRIPTION: A class to read and write ini properties file in standard Windows
format. This was necessary due to write-delay problems with
WritePrivateProfileString and other related functions.
SS: sometimes Steem authors were radical! 
struct TCsfFind, TCsfSects, TCsfKeys
class TConfigStoreFile
---------------------------------------------------------------------------*/

#pragma once
#ifndef CONFIGSTOREFILE_H
#define CONFIGSTOREFILE_H

#include <dynamicarray.h>
#include <easystr.h>
#include <easystringlist.h>

#pragma pack(push, 8)

struct TCsfFind {
  int iSect,iKey;
};

struct TCsfSects {
  char *szName,*szNameUpr;
};

struct TCsfKeys {
  char *szName,*szNameUpr,*szValue;
  int iSect;
};

class TConfigStoreFile {
private:
  EasyStr Path;
  EasyStr FileBuf,FileUprBuf;
  DynamicArray<TCsfSects> Sects;
  DynamicArray<TCsfKeys> Keys;
  DynamicArray<char*> szNewMem;
public:
  TConfigStoreFile(char* = NULL);
  ~TConfigStoreFile();
  bool Open(char* = NULL);
  bool Close(),SaveTo(char*);
  bool FindKey(EasyStr,char *,TCsfFind*);
  bool GetBool(char *,char *,BYTE);
  BYTE GetByte(char *,char *,BYTE);
  int GetInt(char *,char *,int);
  EasyStr GetStr(char *,char *,char *);
  WORD GetWord(char *,char *,WORD);
  void SetInt(char *s,char *k,int v);
  void SetStr(char *,char *,char *);
  void GetSectionNameList(EasyStringList*);
  void DeleteSection(EasyStr);
  bool GetWholeSect(EasyStringList*,EasyStr,bool=0);
  bool Changed;
};

#pragma pack(pop)

#define WriteCSFInt(s,k,v,f) WriteCSFStr(s,k,EasyStr(v),f)
extern void WriteCSFStr(char *,char *,char *,char *);
extern EasyStr GetCSFStr(char *,char *,char *,char *);
extern int GetCSFInt(char *,char *,int,char *);

#endif//#ifndef CONFIGSTOREFILE_H

