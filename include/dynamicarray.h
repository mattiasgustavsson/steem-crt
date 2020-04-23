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
FILE: dynamicarray.h
DESCRIPTION: A template class that does the same as std::vector but 
maybe less well. 
---------------------------------------------------------------------------*/

#pragma once
#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#include "mymisc.h"

template <class Type> class DynamicArray {
  Type *Data;
  int Size;
public:
  DynamicArray() { Size=0;SizeInc=16;Data=NULL;NumItems=0; }
  DynamicArray(int n) { Size=n;SizeInc=16;Data=new Type[n];NumItems=0; }
  ~DynamicArray() { if(Data) delete[] Data; }
#if defined(BCC_BUILD)//SS
  Type& DynamicArray::operator[](int i) { return Data[i]; }
#endif
  operator Type*() { return Data; }
  int NumItems;
  void Add(Type x) {
    if(NumItems>=Size) 
      Resize(Size+SizeInc);
    Data[NumItems++]=x;
  }
  void Delete(int i) {
    if(i<NumItems && i>=0)
    {
      if(i+1<NumItems) 
        memmove(Data+i,Data+i+1,sizeof(Type)*(Size-i));
      NumItems--;
    }
  }
  void DeleteAll(bool resize=true) {
    NumItems=0;
    if(resize) Resize(0);
  }
  void Resize(int);
  int GetSize() { return Size; }
  int SizeInc;
};


template <class Type> void DynamicArray<Type>::Resize(int NewSize) {
  Type *NewData=NULL;
  if(NewSize) 
    NewData=new Type[NewSize];
  if(Size>0&&NewSize>0) 
    memcpy(NewData,Data,sizeof(Type)*MIN(Size,NewSize));
  if(Data) 
    delete[] Data;
  Data=NewData;
  Size=NewSize;
  NumItems=MIN(Size,NumItems);
}

#endif//#ifndef DYNAMICARRAY_H

