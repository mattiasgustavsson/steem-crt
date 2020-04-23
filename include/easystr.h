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
FILE: easystr.h
DESCRIPTION: A (supposedly) easy to use string class.
---------------------------------------------------------------------------*/

#pragma once
#ifndef EASYSTR_H
#define EASYSTR_H	

#include <conditions.h>
#ifndef EASYSTR_BASE
#define EASYSTR_BASE 0
#endif

#define EASYSTR_OPERATORS_DEC(TYPE) \
  EasyStr& operator =(TYPE p); \
  EasyStr operator +(TYPE p); \
  EasyStr& operator +=(TYPE p); \
  bool operator ==(TYPE p); \
  bool operator !=(TYPE p);

#define EASYSTR_OPERATORS(TYPE,lpchar) \
  EasyStr& EasyStr::operator =(TYPE p){return EqualsString(lpchar);} \
  EasyStr EasyStr::operator +(TYPE p){return PlusString(lpchar);} \
  EasyStr& EasyStr::operator +=(TYPE p){return PlusEqualsString(lpchar);} \
  bool EasyStr::operator ==(TYPE p){return strcmp(Text,lpchar)==0;} \
  bool EasyStr::operator !=(TYPE p){return strcmp(Text,lpchar)!=0;}

#pragma pack(push, 8)

struct EasyStr_Dummy_Struct{
  int i;
};


class EasyStr {
private:
  EasyStr& EqualsString(const char *);
  EasyStr PlusString(const char *);
  EasyStr& PlusEqualsString(const char *);
  bool SameAsString(const char *);
  bool NotSameAsString(const char *);
  char *CharToString(char c){
    numbuf[0]=c;numbuf[1]=0;return numbuf;
  }
  void DeleteBuf() { 
    if (Text!=Empty_Text) 
      delete[] Text; 
  }
  void ResizeBuf(INT_PTR size);
  INT_PTR bufsize;
  static char numbuf[32];
  static char Empty_Text[4];
public:
  EasyStr();
  EasyStr(char);
  EasyStr(char *);
  EasyStr(const char *);
  EasyStr(char *,char *);
  EasyStr(const EasyStr&);
  EasyStr(int);EasyStr(unsigned int);
  EasyStr(long);EasyStr(unsigned long);
  EasyStr(bool);
#ifdef _WIN64
  EasyStr(INT_PTR);
  EasyStr(DWORD_PTR);
#endif
  ~EasyStr();
  char *c_str();
  bool IsEmpty();
  bool Empty();
  bool IsNotEmpty();
  bool NotEmpty();
  INT_PTR Length();
  void SetLength(INT_PTR size);
  void SetBufSize(INT_PTR size);
  INT_PTR GetBufSize();
  EasyStr& Insert(EasyStr ToAdd,INT_PTR Pos); //What to add, where to add it
  EasyStr& Delete(INT_PTR start,INT_PTR count);   //First char to delete, number of chars to delete
  char *Right();
  char *Rights(INT_PTR numchars);
  char RightChar();
  EasyStr Lefts(INT_PTR numchars);
  EasyStr Mids(INT_PTR firstchar,INT_PTR numchars);
  EasyStr UpperCase();
  EasyStr LowerCase();
  EasyStr& LPad(INT_PTR NewLen,char c);      //New length of string, char to pad with
  EasyStr& RPad(INT_PTR NewLen,char c);      //Ditto
  INT_PTR InStr(char *);
  int CompareNoCase(char *otext);
  char *Text;
  operator char*();
#if !defined(VC_BUILD) && !defined(MINGW_BUILD) && !defined(UNIX)
  operator struct EasyStr_Dummy_Struct*();
  char& operator[](UINT_PTR idx);
#endif
  EASYSTR_OPERATORS_DEC(char*)
  EASYSTR_OPERATORS_DEC(const char*)
  EASYSTR_OPERATORS_DEC(EasyStr&)
  EASYSTR_OPERATORS_DEC(EasyStr*)
  EASYSTR_OPERATORS_DEC(signed int)
  EASYSTR_OPERATORS_DEC(signed short)
  EASYSTR_OPERATORS_DEC(signed long)
  EASYSTR_OPERATORS_DEC(unsigned int)
  EASYSTR_OPERATORS_DEC(unsigned short)
  EASYSTR_OPERATORS_DEC(unsigned long)
  EASYSTR_OPERATORS_DEC(unsigned char)
  EASYSTR_OPERATORS_DEC(bool)
  EASYSTR_OPERATORS_DEC(char)
#ifdef _WIN64
  EASYSTR_OPERATORS_DEC(INT_PTR)
  EASYSTR_OPERATORS_DEC(DWORD_PTR)
#endif
};

typedef EasyStr Str;

#pragma pack(pop)

#endif//#ifndef EASYSTR_H

