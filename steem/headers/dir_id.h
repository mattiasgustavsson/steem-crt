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

DOMAIN: GUI
FILE: dir_id.h
DESCRIPTION: Declarations for Steem's DirID system.
---------------------------------------------------------------------------*/

#pragma once
#ifndef DIRID_DECLA_H
#define DIRID_DECLA_H
#define DIRID_SHORTCUT 0
#define DIRID_JOY_KEY  1
#define DIRID_JOY_1    2
#define DIRID_JOY_2    3

extern char *KeyboardButtonName[256];
bool IsDirIDPressed(int,int,bool,bool=0);
void init_DirID_to_text();
EasyStr DirID_to_text(int,bool);

#ifdef WIN32
void RegisterButtonPicker(),UnregisterButtonPicker();
LRESULT CALLBACK ButtonPickerWndProc(HWND,UINT,WPARAM,LPARAM);
#define BLANK_DIRID(i) (i==0 || HIBYTE(i)==0xff)
#define NOT_BLANK_DIRID(i) (i!=0 && HIBYTE(i)!=0xff)
#else
#define BLANK_DIRID(i) (HIBYTE(i)==0xff)
#define NOT_BLANK_DIRID(i) (HIBYTE(i)!=0xff)
#endif

int ConvertDirID(int,int);


#endif//DIRID_DECLA_H
