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

DOMAIN: Various
FILE: easystringlist.h
DESCRIPTION: A flexible and easy to use string list class.
class EasyStringList
struct TESL_Data
---------------------------------------------------------------------------*/

#pragma once
#ifndef EASYSTRINGLIST_H
#define EASYSTRINGLIST_H


#include <conditions.h>

#pragma pack(push, 8)

enum ESLSortEnum {eslNoSort=-1,eslSortByName=-2,eslSortByNameI=-3,eslSortByData0=0, 
  eslSortByData1=1,eslSortByData2=2,eslSortByData3=3,eslSortByData4=4, 
  eslSortByData5=5,eslSortByData6=6,eslSortByData7=7,eslSortByData8=8,
  ESL_MINSIZE=16
  }; 

struct TESL_Data {
  char *String;
  LONG_PTR *Data;
  long NumData;
};

class EasyStringList {
private:
  void ReallocData(long Idx,long nDat);
  TESL_Data *Info;
  int Size;
public:
  int NumStrings;
  ESLSortEnum Sort,Sort2;
  EasyStringList(ESLSortEnum sort1=eslSortByNameI,
    ESLSortEnum sort2=eslSortByNameI);
  ~EasyStringList();
  TESL_Data& operator[](int idx);
  TESL_Data& Get(int idx);
  int Compare(int n,ESLSortEnum s,char *Str,LONG_PTR *Dat,long nDat);
  int Add(long nDat,char *AddStr,...);
  int Add(char *AddStr);
  int Add(char *AddStr,LONG_PTR AddData);
  int Add(char *AddStr,LONG_PTR AddData,LONG_PTR AddData2);
  int Add(char *AddStr,LONG_PTR AddData,LONG_PTR AddData2,LONG_PTR AddData3);
  int InsertAt(int i,long nDat,char *AddStr,...);
  int Insert(int n,long nDat,char *AddStr,LONG_PTR *p);
  void SetString(int Idx,char *NewStr);
  int FindString(char *Str),FindString_I(char *Str);
  //void ResizeData(long Idx,long nDat);
  //void ResizeData(long Idx,long nDat,LONG_PTR Data0,...);
  void Delete(int);
  void DeleteAll();
  void ResizeBuf(int NewSize);
};

#pragma pack(pop)

#endif//#ifndef EASYSTRINGLIST_H

