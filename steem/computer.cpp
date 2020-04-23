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

DOMAIN: Emu
FILE: computer.cpp
DESCRIPTION: Instantiation of various objects making up the virtual ST.
Steem also uses many global functions and variables.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop
#include <conditions.h>
#include <computer.h>


TGlue Glue;
TMC68000 Cpu;
TMmu Mmu;
TShifter Shifter;
TMC68901 Mfp;
TDma Dma;
TWD1772 Fdc;
TYM2149 Psg;
TBlitter Blitter;
TMC6850 acia[2];
THD6301 Ikbd;
TSF314 FloppyDrive[3]; // 3rd drive is temporary to get properties
TFloppyDisk FloppyDisk[3];
#if defined(SSE_ACSI)
TAcsiHdc AcsiHdc[TAcsiHdc::MAX_ACSI_DEVICES]; // each object is <64 bytes
#endif
TLMC1992 Microwire;
TTos Tos;
#if defined(SSE_MEGASTE)
TMegaSte MegaSte;
#endif


// called at init, reset, run

void ComputerRestore() {
  Mfp.Restore();
  Glue.Restore();
  Glue.previous_video_freq=Glue.video_freq;
  Mmu.Restore();
  Shifter.Restore();
  Psg.Restore(); //necessary
  acia[0].Id=0;
  acia[1].Id=1;
  // 0 and 1 are true drives/images, 2 is temp drive/image
  for(BYTE id=0;id<3;id++)
  {
    FloppyDrive[id].Restore(id);
#if defined(SSE_DISK_STW)
    ImageSTW[id].Id=id;
#endif
#if defined(SSE_DISK_HFE)
    ImageHFE[id].Id=id;
#endif
#if defined(SSE_DISK_SCP)
    ImageSCP[id].Id=id;
#endif
    FloppyDisk[id].Id=id;
  }
#if defined(SSE_STATS)
  Stats.Restore(); // at each run (Start emulation)
#endif
}
