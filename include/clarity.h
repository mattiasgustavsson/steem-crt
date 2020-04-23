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
FILE: clarity.h
DESCRIPTION: Miscellaneous declarations.
---------------------------------------------------------------------------*/
#pragma once
#ifndef CLARITY_H
#define CLARITY_H

#define IsSameStr(string1,string2)   (!strcmp(string1,string2))
#define StringsMatch(string1,string2)   (!strcmp(string1,string2))
#define NotSameStr(string1,string2)  (strcmp(string1,string2)!=0)
#define IsSameStr_I(string1,string2)   (!strcmpi(string1,string2))
#define StringsMatch_I(string1,string2)   (!strcmpi(string1,string2))
#define NotSameStr_I(string1,string2)  (strcmpi(string1,string2)!=0)
#define Exists(path)           (access(path,0)==0)

#ifdef WIN32
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A // BCC
#endif
#endif

#endif//#ifndef CLARITY_H
