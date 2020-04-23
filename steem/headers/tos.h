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

DOMAIN: OS
FILE: tos.h
DESCRIPTION: Declarations for TOS (The Operating System) utilities
struct TTos
---------------------------------------------------------------------------*/

#pragma once
#ifndef TOS_DECLA_H
#define TOS_DECLA_H

#include <dirsearch.h>
#include <easystr.h>

extern BYTE *STRom;
extern BYTE *Rom_End,*Rom_End_minus_1,*Rom_End_minus_2,*Rom_End_minus_4;
extern BYTE *cart,*cart_save;
extern BYTE *Cart_End_minus_1,*Cart_End_minus_2,*Cart_End_minus_4;
extern unsigned long tos_len;
extern MEM_ADDRESS rom_addr,rom_addr_end;
extern MEM_ADDRESS os_gemdos_vector,os_bios_vector,os_xbios_vector;
extern WORD tos_version;
extern bool tos_high;


#pragma pack(push, 8)

struct TTos {
  void CheckKeyboardClick();
  void CheckSTTypeAndTos();
  EasyStr GetNextTos(DirSearch &ds); // to enumerate TOS files
  void GetTosProperties(EasyStr Path,WORD &Ver,BYTE &Country,WORD &Date,
    BYTE &Recognised);
  void HackMemoryForExtendedMonitor();
};

#pragma pack(pop)

MEM_ADDRESS get_sp_before_trap(bool* pInvalid=NULL);
void intercept_os();
void intercept_gemdos(),intercept_bios(),intercept_xbios();
bool load_TOS(char *);
MEM_ADDRESS get_TOS_address(char *);
void GetTOSKeyTableAddresses(MEM_ADDRESS *lpUnshiftTable,
                             MEM_ADDRESS *lpShiftTable);

#endif//#ifndef TOS_DECLA_H
