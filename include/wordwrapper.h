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
FILE: wordwrapper.h
DESCRIPTION: used by "screen_saver"
---------------------------------------------------------------------------*/

#pragma once
#ifndef WORDWRAPPER_H
#define WORDWRAPPER_H

#include <easystr.h>
#include <dynamicarray.h>


typedef int TEXTWIDTHFUNCTION(char*, int);
typedef TEXTWIDTHFUNCTION* LPTEXTWIDTHFUNCTION;

class TWordWrapper {
    private:
        LPTEXTWIDTHFUNCTION textWidthFunction;
        bool wrapped;
        int numLines;
        DynamicArray<int> linebreak;
    public:
        TWordWrapper(LPTEXTWIDTHFUNCTION twf);
        void setTextWidthFunction(LPTEXTWIDTHFUNCTION twf);
        int wrap(char* text, int width);
        int wrap(char* text, int width, int maxLines);
        EasyStr getHardWrappedString(char* text, int width);
        int getLineFromCharacterIndex(int ci);
        DynamicArray<int> getLineBreaks();
};

#endif//#ifndef WORDWRAPPER_H

