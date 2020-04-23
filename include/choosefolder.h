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
FILE: choosefolder.h
DESCRIPTION: Declaration for letting the user pick a folder.
---------------------------------------------------------------------------*/

#pragma once
#ifndef CHOOSEFOLDER_H
#define CHOOSEFOLDER_H

#ifdef WIN32

#include "easystr.h"

EasyStr ChooseFolder(HWND,char *,char *);

#endif

#endif//#ifndef CHOOSEFOLDER_H

