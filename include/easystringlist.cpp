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
FILE: easystringlist.cpp
DESCRIPTION: A flexible and easy to use string list class.
---------------------------------------------------------------------------*/

#ifdef UNIX
#include <../pch.h>
#endif

#include <string.h>
#include <easystringlist.h>
#include <stdarg.h>

#pragma warning (disable: 4996) // deprecated


void EasyStringList::ReallocData(long Idx,long nDat) {
  delete[] Info[Idx].Data;
  Info[Idx].NumData=nDat;
  Info[Idx].Data=new LONG_PTR[nDat];
}


EasyStringList::EasyStringList(ESLSortEnum sort1,ESLSortEnum sort2) {
  Size=ESL_MINSIZE;
  Info=new TESL_Data[Size];
  memset(Info,0,sizeof(TESL_Data)*Size);
  Sort=sort1;
  Sort2=sort2;
  NumStrings=0;
}


TESL_Data& EasyStringList::operator[](int idx) {
  return Info[max(min(idx,NumStrings-1),0)];
}


TESL_Data& EasyStringList::Get(int idx) {
  return Info[max(min(idx,NumStrings-1),0)];
}


void EasyStringList::DeleteAll() { 
  ResizeBuf(0); 
}


void EasyStringList::ResizeBuf(int NewSize) {
  Size=MAX(NewSize,ESL_MINSIZE);
  if(NumStrings)
  {
    for(int n=NewSize;n<NumStrings;n++)
    { // If shrinking
      delete[] Info[n].String;
      delete[] Info[n].Data;
    }
    NumStrings=MIN(NumStrings,NewSize);
    TESL_Data *NewInfo=new TESL_Data[Size];
    if(NumStrings) 
      memcpy(NewInfo,Info,NumStrings*sizeof(TESL_Data));
    delete[] Info;
    Info=NewInfo;
  }
  else
  {
    delete[] Info;
    Info=new TESL_Data[Size];
  }
  if(NumStrings==0) 
    memset(Info,0,sizeof(TESL_Data)); // Zero the first entry
}


int EasyStringList::Compare(int n,ESLSortEnum s,char *Str,LONG_PTR *Dat,
                            long nDat) {
  if(s==eslSortByNameI)
    return strcmpi(Str,Info[n].String);
  else if(s==eslSortByName)
    return strcmp(Str,Info[n].String);
  else if((int)s>=0)
  {
    if((int)s>=nDat)
      return 1;
    else if((int)s>=Info[n].NumData)
      return -1;
    else if(Dat[(int)s]<Info[n].Data[(int)s])
      return -1;
    else if(Dat[(int)s]==Info[n].Data[(int)s])
      return 0;
  }
  return 1;
}


int EasyStringList::InsertAt(int i,long nDat,char *AddStr,...) {
  //  used in Unix build
  return Insert(i,nDat,AddStr,((LONG_PTR*)(&AddStr))+1);
}


int EasyStringList::Add(char *AddStr) {
  return Add(0,AddStr);
}


int EasyStringList::Add(char *AddStr,LONG_PTR AddData) {
  return Add(1,AddStr,AddData);
}


int EasyStringList::Add(char *AddStr,LONG_PTR AddData,LONG_PTR AddData2) {
  return Add(2,AddStr,AddData,AddData2);
}


int EasyStringList::Add(char *AddStr,LONG_PTR AddData,LONG_PTR AddData2,
                        LONG_PTR AddData3) {
  return Add(3,AddStr,AddData,AddData2,AddData3);
}


int EasyStringList::Add(long nDat,char *AddStr,...) {
// maybe it shouldn't be LONG_PTR after all
// but it probably doesn't hurt... TODO
  va_list vl;
  va_start(vl,AddStr);
  LONG_PTR tmpDat[12]; // min 0, max 4 (Linux)
  for(int i=0;i<nDat;i++)
    tmpDat[i]=va_arg(vl,LONG_PTR);
  LONG_PTR *p=tmpDat;
  bool Less;
  for(int n=0;n<=NumStrings;n++)
  {
    if(n==NumStrings)
      Less=true;
    else
    {
      int Val=Compare(n,Sort,AddStr,p,nDat);
      if(Val==0) 
        Val=Compare(n,Sort2,AddStr,p,nDat);
      Less=Val<=0;
    }
    if(Less)
    {
      int r=Insert(n,nDat,AddStr,p);
      va_end(vl);
      return r;
    }
  }
  va_end(vl);
  return 0;
}


int EasyStringList::Insert(int n,long nDat,char *AddStr,LONG_PTR *p) {
  if(NumStrings>=Size)
    ResizeBuf(NumStrings+16);
  if(n>NumStrings) 
    n=NumStrings;
  if(n<0) 
    n=0;
  for(int i=NumStrings;i>n;i--) 
    Info[i]=Info[i-1];
  Info[n].String=new char[strlen(AddStr)+1];
  strcpy(Info[n].String,AddStr);
  Info[n].NumData=nDat;
  Info[n].Data=new LONG_PTR[nDat];
  for(int i=0;i<nDat;i++) 
    Info[n].Data[i]=*(p++);
  NumStrings++;
  return n;
}


int EasyStringList::FindString(char *Str) {
  for(int n=0;n<NumStrings;n++) 
    if(strcmp(Str,Info[n].String)==0) 
      return n;
  return -1;
}


int EasyStringList::FindString_I(char *Str) {
  for(int n=0;n<NumStrings;n++) 
    if(strcmpi(Str,Info[n].String)==0) 
      return n;
  return -1;
}


#if 0

void EasyStringList::ResizeData(long Idx,long nDat) {
  LONG_PTR *OldData=Info[Idx].Data;
  long OldNumData=Info[Idx].NumData;
  Info[Idx].NumData=nDat;
  Info[Idx].Data=new LONG_PTR[nDat];
  for(int i=0;i<min(nDat,OldNumData);i++)
    Info[Idx].Data[i]=OldData[i];
  for(int i=OldNumData;i<nDat;i++)        
    Info[Idx].Data[i]=0;
  delete[] OldData;
}


void EasyStringList::ResizeData(long Idx,long nDat,LONG_PTR Data0,...) {
  ReallocData(Idx,nDat);
  LONG_PTR *p=&Data0;
  for(int i=0;i<nDat;i++) 
    Info[Idx].Data[i]=*(p++);
}

#endif


void EasyStringList::SetString(int Idx,char *NewStr) {
  delete[] Info[Idx].String;
  Info[Idx].String=new char[strlen(NewStr)+1];
  strcpy(Info[Idx].String,NewStr);
}


void EasyStringList::Delete(int Idx) {
  NumStrings--;
  delete[] Info[Idx].String;
  delete[] Info[Idx].Data;
  for(int i=Idx;i<NumStrings;i++) 
    Info[i]=Info[i+1];
}


EasyStringList::~EasyStringList() {
  for(int i=0;i<NumStrings;i++)
  {
    delete[] Info[i].String;
    delete[] Info[i].Data;
  }
  delete[] Info;
}
