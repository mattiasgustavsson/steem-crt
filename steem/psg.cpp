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
FILE: psg.cpp
DESCRIPTION: Steem's original high-level Programmable Sound Generator
(Yamaha 2149) emulation, and a low-level emulation vaguely inspired by MESS.
The YM2149 also commands floppy disk drive unit and side.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <conditions.h>
#include <steemh.h>
#include <cpu.h>
#include <device_map.h>
#include <gui.h>
#include <stports.h>
#include "psg.h"
#include "options.h"
#include <debug.h>
#include <computer.h>
#include <sound.h>
#if USE_PASTI
#include <pasti/pasti.h>
#endif
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*                                PSG SOUND                                  */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

int *psg_channels_buf=NULL;
DWORD psg_channels_buf_len=0; // variable
int psg_buf_pointer[3];
DWORD psg_tone_start_time[3];
char psg_noise[PSG_NOISE_ARRAY];

DWORD psg_envelope_start_time=0xfffff000;

int psg_voltage,psg_dv;
BYTE psg_reg[16],psg_reg_data;
BYTE psg_reg_select;

bool written_to_env_this_vbl=true; // for record YM

// Steem Legacy
/*  tone frequency
     fT = fMaster
          -------
           16TP


ie. period = 16TP / 2 000 000
= TP/125000

 = TP / (8*15625)


 the envelope repetition frequency fE is obtained as follows from the
envelope setting period value EP (decimal):

       fE = fMaster        (fMaster if the frequency of the master clock)
            -------
             256EP

The period of the actual frequency fEA used for the envelope generated is
1/32 of the envelope repetition period (1/fE).

ie. period of whole envelope = 256EP/2 000 000 = 2*EP / 15625 (think this is one go through, eg. /)
Period of envelope change frequency is 1/32 of this, so EP/ 16*15625
Scale by 64 and multiply by frequency of buffer to get final period, 4*EP/15625

*/

/*      New scalings - 6/7/01

__TONE__  Period = TP/(8*15625) Hz = (TP*sound_freq)/(8*15625) samples
          Period in samples goes up to 1640.6.  Want this to correspond to 2000M, so
          scale up by 2^20 = 1M.  So period to initialise counter is
          (TP*sound_freq*2^17)/15625, which ranges up to 1.7 thousand million.  The
          countdown each time is 2^20, but we double this to 2^21 so that we can
          spot each half of the period.  To initialise the countdown, bit 0 of
          (time*2^21)/tonemodulo gives high or low.  The counter is initialised
          to tonemodulo-(time*2^21 mod tonemodulo).

__NOISE__ fudged similarly to tone.

__ENV__   Step period = 2EP/(15625*32) Hz.  Scale by 2^17 to scale 13124.5 to 1.7 thousand
          million.  Step period is (EP*sound_freq*2^13)/15625.

*/


#define VFP VOLTAGE_FIXED_POINT
#define VZL VOLTAGE_ZERO_LEVEL
#define VA VFP*(PSG_CHANNEL_AMPLITUDE)
const int psg_flat_volume_level[16]={0*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,
                                      17*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,48*VA/1000+VZL*VFP,
                                      69*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,
                                      287*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP};
const int psg_envelope_level[8][64]={
    {1000*VA/1000+VZL*VFP,805*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,514*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,342*VA/1000+VZL*VFP,287*VA/1000+VZL*VFP,234*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,163*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,115*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,81*VA/1000+VZL*VFP,69*VA/1000+VZL*VFP,58*VA/1000+VZL*VFP,53*VA/1000+VZL*VFP,41*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,29*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,17*VA/1000+VZL*VFP,14*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,6*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,
    1000*VA/1000+VZL*VFP,805*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,514*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,342*VA/1000+VZL*VFP,287*VA/1000+VZL*VFP,234*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,163*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,115*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,81*VA/1000+VZL*VFP,69*VA/1000+VZL*VFP,58*VA/1000+VZL*VFP,53*VA/1000+VZL*VFP,41*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,29*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,17*VA/1000+VZL*VFP,14*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,6*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP},
    {1000*VA/1000+VZL*VFP,805*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,514*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,342*VA/1000+VZL*VFP,287*VA/1000+VZL*VFP,234*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,163*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,115*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,81*VA/1000+VZL*VFP,69*VA/1000+VZL*VFP,58*VA/1000+VZL*VFP,53*VA/1000+VZL*VFP,41*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,29*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,17*VA/1000+VZL*VFP,14*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,6*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,
    VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP},
    {1000*VA/1000+VZL*VFP,805*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,514*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,342*VA/1000+VZL*VFP,287*VA/1000+VZL*VFP,234*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,163*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,115*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,81*VA/1000+VZL*VFP,69*VA/1000+VZL*VFP,58*VA/1000+VZL*VFP,53*VA/1000+VZL*VFP,41*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,29*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,17*VA/1000+VZL*VFP,14*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,6*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,
    0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,6*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,14*VA/1000+VZL*VFP,17*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,29*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,41*VA/1000+VZL*VFP,53*VA/1000+VZL*VFP,58*VA/1000+VZL*VFP,69*VA/1000+VZL*VFP,81*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,115*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,163*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,234*VA/1000+VZL*VFP,287*VA/1000+VZL*VFP,342*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,514*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,805*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP},
    {1000*VA/1000+VZL*VFP,805*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,514*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,342*VA/1000+VZL*VFP,287*VA/1000+VZL*VFP,234*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,163*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,115*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,81*VA/1000+VZL*VFP,69*VA/1000+VZL*VFP,58*VA/1000+VZL*VFP,53*VA/1000+VZL*VFP,41*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,29*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,17*VA/1000+VZL*VFP,14*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,6*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,
    VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP},
    {0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,6*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,14*VA/1000+VZL*VFP,17*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,29*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,41*VA/1000+VZL*VFP,53*VA/1000+VZL*VFP,58*VA/1000+VZL*VFP,69*VA/1000+VZL*VFP,81*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,115*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,163*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,234*VA/1000+VZL*VFP,287*VA/1000+VZL*VFP,342*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,514*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,805*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP,
    0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,6*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,14*VA/1000+VZL*VFP,17*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,29*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,41*VA/1000+VZL*VFP,53*VA/1000+VZL*VFP,58*VA/1000+VZL*VFP,69*VA/1000+VZL*VFP,81*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,115*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,163*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,234*VA/1000+VZL*VFP,287*VA/1000+VZL*VFP,342*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,514*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,805*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP},
    {0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,6*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,14*VA/1000+VZL*VFP,17*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,29*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,41*VA/1000+VZL*VFP,53*VA/1000+VZL*VFP,58*VA/1000+VZL*VFP,69*VA/1000+VZL*VFP,81*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,115*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,163*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,234*VA/1000+VZL*VFP,287*VA/1000+VZL*VFP,342*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,514*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,805*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP,
    VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP,VA+VZL*VFP},
    {0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,6*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,14*VA/1000+VZL*VFP,17*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,29*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,41*VA/1000+VZL*VFP,53*VA/1000+VZL*VFP,58*VA/1000+VZL*VFP,69*VA/1000+VZL*VFP,81*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,115*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,163*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,234*VA/1000+VZL*VFP,287*VA/1000+VZL*VFP,342*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,514*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,805*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP,
    1000*VA/1000+VZL*VFP,805*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,514*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,342*VA/1000+VZL*VFP,287*VA/1000+VZL*VFP,234*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,163*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,115*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,81*VA/1000+VZL*VFP,69*VA/1000+VZL*VFP,58*VA/1000+VZL*VFP,53*VA/1000+VZL*VFP,41*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,29*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,17*VA/1000+VZL*VFP,14*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,6*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP},
    {0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,4*VA/1000+VZL*VFP,6*VA/1000+VZL*VFP,8*VA/1000+VZL*VFP,10*VA/1000+VZL*VFP,12*VA/1000+VZL*VFP,14*VA/1000+VZL*VFP,17*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,24*VA/1000+VZL*VFP,29*VA/1000+VZL*VFP,35*VA/1000+VZL*VFP,41*VA/1000+VZL*VFP,53*VA/1000+VZL*VFP,58*VA/1000+VZL*VFP,69*VA/1000+VZL*VFP,81*VA/1000+VZL*VFP,95*VA/1000+VZL*VFP,115*VA/1000+VZL*VFP,139*VA/1000+VZL*VFP,163*VA/1000+VZL*VFP,191*VA/1000+VZL*VFP,234*VA/1000+VZL*VFP,287*VA/1000+VZL*VFP,342*VA/1000+VZL*VFP,407*VA/1000+VZL*VFP,514*VA/1000+VZL*VFP,648*VA/1000+VZL*VFP,805*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP,
    VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP,VZL*VFP}};
#undef VFP
#undef VZL
#undef VA


#define ONE_MILLION 1048576
#define TWO_MILLION 2097152
#define TWO_TO_SEVENTEEN 131072

#define PSG_PULSE_NOISE(ntn) (psg_noise[(ntn) MOD_PSG_NOISE_ARRAY] )
#define PSG_PULSE_TONE  ((t*128 / psg_tonemodulo) & 1)
#define PSG_PULSE_TONE_t64  ((t*64 / psg_tonemodulo_2) & 1)



#define LOGSECTION LOGSECTION_SOUND

#define PSG_PREPARE_ENVELOPE          {                      \
      int envperiod=MAX( (((int)psg_reg[PSGR_ENVELOPE_PERIOD_HIGH]) <<8) + psg_reg[PSGR_ENVELOPE_PERIOD_LOW],1);  \
      af=envperiod;                              \
      af*=sound_freq;                  \
      af*=((double)(1<<13))/15625;                               \
      psg_envmodulo=(int)af; \
      bf=(((DWORD)t)-psg_envelope_start_time); \
      bf*=(double)(1<<17); \
      psg_envstage=(int)floor(bf/af); \
      bf=fmod(bf,af); /*remainder*/ \
      psg_envcountdown=psg_envmodulo-(int)bf; \
      envdeath=-1;                                                                  \
      if ((psg_reg[PSGR_ENVELOPE_SHAPE] & PSG_ENV_SHAPE_CONT)==0 ||                  \
           (psg_reg[PSGR_ENVELOPE_SHAPE] & PSG_ENV_SHAPE_HOLD)){                      \
        if(psg_reg[PSGR_ENVELOPE_SHAPE]==11 || psg_reg[PSGR_ENVELOPE_SHAPE]==13){      \
          envdeath=psg_flat_volume_level[15];                                           \
        }else{                                                                           \
          envdeath=psg_flat_volume_level[0];                                              \
        }                                                                                   \
      }                                                                                      \
      envshape=psg_reg[PSGR_ENVELOPE_SHAPE] & 7;                    \
      if (psg_envstage>=32 && envdeath!=-1){                           \
        envvol=envdeath;                                             \
      }else{                                                       \
        envvol=psg_envelope_level[envshape][psg_envstage & 63];            \
        if(OPTION_YM_12DB&&IS_STE&&OPTION_MICROWIRE)\
          envvol>>=1;\
      }																															\
}

#define PSG_PREPARE_NOISE  {                              \
      int noiseperiod=(1+(psg_reg[PSGR_NOISE_PERIOD]&0x1f));      \
      af=((int)noiseperiod*sound_freq);                              \
      af*=((double)(1<<17))/15625; \
      psg_noisemodulo=(int)af; \
      bf=t; \
      bf*=(double)(1<<20); \
      psg_noisecounter=(int)floor(bf/af); \
      psg_noisecounter &= (PSG_NOISE_ARRAY-1); \
      bf=fmod(bf,af); \
      psg_noisecountdown=psg_noisemodulo-(int)bf; \
      psg_noisetoggle=(psg_noise[psg_noisecounter]!=0);\
}

#define PSG_PREPARE_TONE       {                          \
      af=((int)toneperiod*sound_freq);                              \
      af*=((double)(1<<17))/15625;                               \
      psg_tonemodulo_2=(int)af; \
      bf=(((DWORD)t)-psg_tone_start_time[abc]); \
      bf*=(double)(1<<21); \
      bf=fmod(bf,af*2); \
      af=bf-af;               \
      if(af>=0){                  \
        psg_tonetoggle=false;       \
        bf=af;                      \
      }                           \
      psg_tonecountdown=psg_tonemodulo_2-(int)bf; \
}

#define PSG_ENVELOPE_ADVANCE           {                        \
          psg_envcountdown-=TWO_TO_SEVENTEEN;  \
          while (psg_envcountdown<0){           \
            psg_envcountdown+=psg_envmodulo;             \
            psg_envstage++;                   \
            if (psg_envstage>=32 && envdeath!=-1){                           \
              envvol=envdeath;                                             \
            }else{                                                       \
              envvol=psg_envelope_level[envshape][psg_envstage & 63];            \
            }																															\
          }\
}


#define PSG_TONE_ADVANCE       {                            \
          psg_tonecountdown-=TWO_MILLION;  \
          while (psg_tonecountdown<0){           \
            psg_tonecountdown+=psg_tonemodulo_2;             \
            psg_tonetoggle=!psg_tonetoggle;                   \
          }\
}


#define PSG_NOISE_ADVANCE  {                         \
          psg_noisecountdown-=ONE_MILLION;   \
          while (psg_noisecountdown<0){   \
            psg_noisecountdown+=psg_noisemodulo;      \
            psg_noisecounter++;                        \
            if(psg_noisecounter>=PSG_NOISE_ARRAY){      \
              psg_noisecounter=0;                        \
            }                                             \
            psg_noisetoggle=(psg_noise[psg_noisecounter]!=0);   \
          }\
}


void psg_write_buffer(int abc,DWORD to_t) {
  if(!SSEConfig.YmSoundOn)
    return;
  //buffer starts at time time_of_last_vbl
  //we've written up to psg_buf_pointer[abc]
  //so start at pointer and write to to_t,
  int psg_tonemodulo_2,psg_noisemodulo;
  int psg_tonecountdown,psg_noisecountdown;
  int psg_noisecounter;
  double af,bf;
  bool psg_tonetoggle=true,psg_noisetoggle;
  int *p=psg_channels_buf+psg_buf_pointer[abc];
  DWORD t=(psg_time_of_last_vbl_for_writing+psg_buf_pointer[abc]);
  to_t=MAX(to_t,t);
  to_t=MIN(to_t,psg_time_of_last_vbl_for_writing+PSG_CHANNEL_BUF_LENGTH);
  int count=MAX(MIN((DWORD)(to_t-t),PSG_CHANNEL_BUF_LENGTH
    -psg_buf_pointer[abc]),0ul);
  if(!count)
    return;
  int toneperiod=(((int)psg_reg[abc*2+1]&0xf)<<8)+psg_reg[abc*2];
  if((psg_reg[abc+8]&BIT_4)==0)
  { // Not Enveloped
    int vol=psg_flat_volume_level[psg_reg[abc+8]&15];
    if(OPTION_YM_12DB&&IS_STE&&OPTION_MICROWIRE)
      vol>>=1;
    if((psg_reg[PSGR_MIXER]&(1<<abc))==0&&(toneperiod>9))
    { //tone enabled
      PSG_PREPARE_TONE
      if((psg_reg[PSGR_MIXER]&(8<<abc))==0)
      { //noise enabled
        PSG_PREPARE_NOISE
        for(;count>0;count--)
        {
          if(psg_tonetoggle
#if defined(SSE_DEBUGGER_MUTE_SOUNDCHANNELS)
            &&(!((4>>abc)&(d2_dpeek(FAKE_IO_START+20)>>12))) // chan
#endif
            ||psg_noisetoggle
#if defined(SSE_DEBUGGER_MUTE_SOUNDCHANNELS)
            &&(!((1<<11)&d2_dpeek(FAKE_IO_START+20))) // noise
#endif            
            )
            p++;
          else
            *(p++)+=vol;
          //ASSERT(p-psg_channels_buf<=PSG_CHANNEL_BUF_LENGTH);
          PSG_TONE_ADVANCE
          PSG_NOISE_ADVANCE
        }
      }
      else
      { //tone only
        for(;count>0;count--)
        {
          if(psg_tonetoggle
#if defined(SSE_DEBUGGER_MUTE_SOUNDCHANNELS)
            &&(!((4>>abc)&(d2_dpeek(FAKE_IO_START+20)>>12))) // chan
#endif
            )
            p++;
          else
            *(p++)+=vol;
          //ASSERT(p-psg_channels_buf<=PSG_CHANNEL_BUF_LENGTH);
          PSG_TONE_ADVANCE
        }
      }
    }
    else if((psg_reg[PSGR_MIXER]&(8<<abc))==0)
    { //noise enabled
      PSG_PREPARE_NOISE
      for(;count>0;count--)
      {
        if(psg_noisetoggle
#if defined(SSE_DEBUGGER_MUTE_SOUNDCHANNELS)
          &&(!((1<<11)&d2_dpeek(FAKE_IO_START+20))) // noise
#endif          
          )
          p++;
        else
          *(p++)+=vol;
        //ASSERT(p-psg_channels_buf<=PSG_CHANNEL_BUF_LENGTH);
        PSG_NOISE_ADVANCE
      }
    }
    else
    { //nothing enabled
      for(;count>0;count--)
      {
        *(p++)+=vol;
        //ASSERT(p-psg_channels_buf<=PSG_CHANNEL_BUF_LENGTH);
      }
    }
    psg_buf_pointer[abc]=to_t-psg_time_of_last_vbl_for_writing;
    return;
  }
  else
  {// Enveloped
    int envdeath,psg_envstage,envshape;
    int psg_envmodulo,envvol,psg_envcountdown;
    PSG_PREPARE_ENVELOPE;
    if((psg_reg[PSGR_MIXER]&(1<<abc))==0&&(toneperiod>9))
    { //tone enabled
      PSG_PREPARE_TONE
      if((psg_reg[PSGR_MIXER]&(8<<abc))==0)
      { //noise enabled
        PSG_PREPARE_NOISE
        for(;count>0;count--)
        {
          if(psg_tonetoggle
#if defined(SSE_DEBUGGER_MUTE_SOUNDCHANNELS)
            &&(!((4>>abc)&(d2_dpeek(FAKE_IO_START+20)>>12))) // chan
            &&(!((1<<10)&d2_dpeek(FAKE_IO_START+20))) // env
#endif
            ||psg_noisetoggle
#if defined(SSE_DEBUGGER_MUTE_SOUNDCHANNELS)
            &&(!((1<<11)&d2_dpeek(FAKE_IO_START+20))) // noise
#endif            
            )
            p++;
          else
            *(p++)+=envvol;
          PSG_TONE_ADVANCE
          PSG_NOISE_ADVANCE
          PSG_ENVELOPE_ADVANCE
        }
      }
      else
      { //tone only
        for(;count>0;count--)
        {
          if(psg_tonetoggle
#if defined(SSE_DEBUGGER_MUTE_SOUNDCHANNELS)
            &&(!((4>>abc)&(d2_dpeek(FAKE_IO_START+20)>>12))) // chan
            &&(!((1<<10)&d2_dpeek(FAKE_IO_START+20))) // env
#endif            
            )
            p++;
          else
            *(p++)+=envvol;
          PSG_TONE_ADVANCE
          PSG_ENVELOPE_ADVANCE
        }
      }
    }
    else if((psg_reg[PSGR_MIXER]&(8<<abc))==0)
    { //noise enabled
      PSG_PREPARE_NOISE
      for(;count>0;count--)
      {
        if(psg_noisetoggle
#if defined(SSE_DEBUGGER_MUTE_SOUNDCHANNELS)
          &&(!((1<<11)&d2_dpeek(FAKE_IO_START+20))) // noise
#endif          
          )
          p++;
        else
          *(p++)+=envvol;
        PSG_NOISE_ADVANCE
        PSG_ENVELOPE_ADVANCE
      }
    }
    else
    { //nothing enabled
      for(;count>0;count--)
      {
        *(p++)+=envvol;
        PSG_ENVELOPE_ADVANCE
      }
    }
    psg_buf_pointer[abc]=to_t-psg_time_of_last_vbl_for_writing;
  }
}


DWORD psg_adjust_envelope_start_time(DWORD t,DWORD new_envperiod) {
  double b,c;
  int envperiod=MAX((((int)psg_reg[PSGR_ENVELOPE_PERIOD_HIGH])<<8)
    +psg_reg[PSGR_ENVELOPE_PERIOD_LOW],1);
  b=(t-psg_envelope_start_time);
  c=b*(double)new_envperiod;
  c/=(double)envperiod;
  c=t-c;         //new env start time
  return DWORD(c);
}


#if defined(SSE_YM2149_LL)

void check_mamelike_ym_envelope(int reg,BYTE new_val) {
  // now an apart function because we want to update low-level YM's envelope ('buzzer') 
  // parameters also in fast forward or muted modes - code was lifted from MAME
  if(reg==PSGR_ENVELOPE_SHAPE)
  {
    Psg.m_attack=(new_val&0x04)?Psg.m_env_step_mask:0x00;
    if((new_val&0x08)==0)
    {
      /* if Continue = 0, map the shape to the equivalent one which has Continue = 1 */
      Psg.m_hold=1;
      Psg.m_alternate=Psg.m_attack;
    }
    else
    {
      Psg.m_hold=new_val&0x01;
      Psg.m_alternate=new_val&0x02;
    }
    Psg.m_env_step=Psg.m_env_step_mask;
    Psg.m_holding=0;
    //ASSERT(Psg.m_env_step>=0);
    Psg.m_env_volume=(Psg.m_env_step ^ Psg.m_attack); //no need?
    Psg.m_count_env=0; // (from Hatari)
    written_to_env_this_vbl=true;
  }
}

#endif


void psg_set_reg(int reg,BYTE old_val,BYTE &new_val) {
  // suggestions for global variables:  n_samples_per_vbl=sound_freq/video_freq,   shifter_y+(SCANLINES_ABOVE_SCREEN+SCANLINES_BELOW_SCREEN)
  if(reg==1||reg==3||reg==5||reg==13)
    new_val&=15;
  else if(reg==6||(reg>=8&&reg<=10))
    new_val&=31;
  if(reg>=PSGR_PORT_A) // 14, 15
    return;
  if(old_val==new_val && reg!=PSGR_ENVELOPE_SHAPE) 
    return;
#if defined(SSE_STATS)
  if(reg!=7) // not mixer
    Stats.nPsgSound++;
#endif
  if(SoundActive()==0) // fast forward, mute...
  {
    DBG_LOG(Str("SOUND: ")+HEXSl(old_pc,6)+" - PSG reg "+reg+" changed to "+new_val+" at "+scanline_cycle_log());
#if defined(SSE_YM2149_LL)
    check_mamelike_ym_envelope(reg,new_val);
#endif
    return;
  }
  if(!video_freq_at_start_of_vbl)
    return;
  int cpu_cycles_per_vbl=n_cpu_cycles_per_second/video_freq_at_start_of_vbl;
#if SCREENS_PER_SOUND_VBL != 1
  cpu_cycles_per_vbl*=SCREENS_PER_SOUND_VBL;
  DWORDLONG a64=(ABSOLUTE_CPU_TIME-cpu_time_of_last_sound_vbl);
#else
  DWORDLONG a64=(ABSOLUTE_CPU_TIME-cpu_time_of_last_vbl);
#endif
  a64*=psg_n_samples_this_vbl; //SS eg *882  (44100/50)
  a64/=cpu_cycles_per_vbl; //SS eg 160420
  DWORD t=psg_time_of_last_vbl_for_writing+(DWORD)a64; //SS t's unit is #samples (total)
  DBG_LOG(EasyStr("SOUND: PSG reg ")+reg+" changed to "+new_val+" at "+scanline_cycle_log()+"; samples "+t+"; vbl was at "+psg_time_of_last_vbl_for_writing);
#if defined(SSE_YM2149_LL)
  if(OPTION_MAME_YM)
  {
    Psg.psg_write_buffer(t);
    check_mamelike_ym_envelope(reg,new_val);
  }
  else 
#endif
  switch(reg) {
  case 0:case 1:
  case 2:case 3:
  case 4:case 5:
  {
    int abc=reg/2;
    // Freq is double bufferred, it cannot change until the PSG reaches the end of the current square wave.
    // psg_tone_start_time[abc] is set to the last end of wave, so if it is in future don't do anything.
    // Overflow will be a problem, however at 50Khz that will take a day of non-stop output.
    if(t>psg_tone_start_time[abc])
    {
      // before change
      int toneperiod1=(((int)psg_reg[abc*2+1]&0xf)<<8)+psg_reg[abc*2];
      if(toneperiod1>1)
      {
        double a=toneperiod1*sound_freq;
        a/=(15625*8);
        double b=(t-psg_tone_start_time[abc]);
        double a2=fmod(b,a);
        b-=a2;
        DWORD t2=psg_tone_start_time[abc]+DWORD(b); // adjusted start
        // after change
        int toneperiod2=(reg&1)
          ?(((int)new_val&0xf)<<8)+psg_reg[abc*2]
          :(((int)psg_reg[abc*2+1]&0xf)<<8)+new_val;
        double a_bis=toneperiod2*sound_freq;
        a_bis/=(15625*8);
        // check progress of current wave
        if(!a2||a2>=a_bis)
          t2=t-(DWORD)a_bis; // start at once
        t=t2;
      }
      psg_write_buffer(abc,t);
      psg_tone_start_time[abc]=t;
    }
    break;
  }
  case PSGR_NOISE_PERIOD: //6:  //changed noise
    psg_write_buffer(0,t);
    psg_write_buffer(1,t);
    psg_write_buffer(2,t);
    break;
  case PSGR_MIXER: //7:  //mixer
    psg_write_buffer(0,t);
    psg_write_buffer(1,t);
    psg_write_buffer(2,t);
    break;
  case PSGR_AMPLITUDE_A:
  case PSGR_AMPLITUDE_B:
  case PSGR_AMPLITUDE_C:
  //case 8:case 9:case 10:  //channel A,B,C volume
    // YM doesn't quantize, it changes the level straight away.
    // changing the volume fast is a method used to render samples
    psg_write_buffer(reg-8,t);
    break;
  case PSGR_ENVELOPE_PERIOD_LOW: //11: //changing envelope period low
  {
    psg_write_buffer(0,t);
    psg_write_buffer(1,t);
    psg_write_buffer(2,t);
    int new_envperiod=MAX((((int)psg_reg[PSGR_ENVELOPE_PERIOD_HIGH])<<8)
      +new_val,1);
    psg_envelope_start_time=psg_adjust_envelope_start_time(t,new_envperiod);
    break;
  }
  case PSGR_ENVELOPE_PERIOD_HIGH: //12: //changing envelope period high
  {
    psg_write_buffer(0,t);
    psg_write_buffer(1,t);
    psg_write_buffer(2,t);
    int new_envperiod=MAX((((int)new_val)<<8)
      +psg_reg[PSGR_ENVELOPE_PERIOD_LOW],1);
    psg_envelope_start_time=psg_adjust_envelope_start_time(t,new_envperiod);
    break;
  }
  case PSGR_ENVELOPE_SHAPE: //13: //envelope shape
    psg_write_buffer(0,t);
    psg_write_buffer(1,t);
    psg_write_buffer(2,t);
    psg_envelope_start_time=t;
    written_to_env_this_vbl=true;
    break;
  }//sw
}

#undef LOGSECTION



#define ENVELOPE_MASK 31

TYM2149::TYM2149() {
  p_fixed_vol_3voices=NULL;
#if defined(SSE_YM2149_LL)
  AntiAlias=NULL;
#endif
}


TYM2149::~TYM2149() {
  FreeFixedVolTable();
}


#define LOGSECTION LOGSECTION_SOUND

void TYM2149::FreeFixedVolTable() {
  if(p_fixed_vol_3voices)
  {
    TRACE_LOG("free memory of PSG table %p\n",p_fixed_vol_3voices);
    delete [] p_fixed_vol_3voices;
    p_fixed_vol_3voices=NULL;
  }
}


bool TYM2149::LoadFixedVolTable(bool check_dmamix) {
  bool ok=false;
  FreeFixedVolTable();
  p_fixed_vol_3voices=new WORD[16*16*16];
  //ASSERT(p_fixed_vol_3voices);
  // first look for the file, if it isn't there, use internal resource
  EasyStr filename=RunDir+SLASH+SSE_PLUGIN_DIR1+SLASH+YM2149_FIXED_VOL_FILENAME;
  FILE *fp=fopen(filename.Text,"r+b");
  if(!fp)
  {
    filename=RunDir+SLASH+SSE_PLUGIN_DIR2+SLASH+YM2149_FIXED_VOL_FILENAME;
    fp=fopen(filename.Text,"r+b");
  }
  if(!fp)
  {
    filename=RunDir+SLASH+YM2149_FIXED_VOL_FILENAME;
    fp=fopen(filename.Text,"r+b");
  }
  if(fp)
  {
    int nwords=(int)fread(p_fixed_vol_3voices,sizeof(WORD),16*16*16,fp);
    if(nwords==16*16*16)
      ok=true;
    TRACE_LOG("PSG %s loaded %d\n",filename.Text,ok);
    fclose(fp);
  }
#if defined(SSE_FILES_IN_RC)  
  else
  {
    HRSRC rc=FindResource(NULL,MAKEINTRESOURCE(IDR_YM2149),RT_RCDATA);
    ASSERT(rc);
    if(rc)
    {
      HGLOBAL hglob=LoadResource(NULL,rc);
      if(hglob)
      {
        size_t size=SizeofResource(NULL,rc);
        BYTE *pdata=(BYTE*)LockResource(hglob);
        if(pdata && size==sizeof(WORD)*16*16*16) //8Kb
        {
          memcpy(p_fixed_vol_3voices,pdata,size);
          ok=true;
          TRACE_LOG("PSG table loaded in %p\n",p_fixed_vol_3voices);
        }
      }
    }
  }
#endif  
#if defined(SSE_SOUND_16BIT_CENTRED)
/*  In previous versions, the zero (silence) was a very negative value,
    for sampled YM as well as for 'Steem native'.
    If we want the zero to be set at the zero of a signed 16bit wave, we
    must reduce the range in both tables.
    So we simply lose the last bit to half the range.
    I doubt the ST has a 16bit dynamic range anyway :), we lose nothing.
    All values are positive, the signed sound wave will be totally lopsided.
    
    STE sound mixer:
    According to Atari doc, 0 means YM -12db and 2 means no YM, but it doesn't
    work like this. 
    On my STE, there's still YM sound at full volume when it is 2,
    and 0 mutes the YM.
    Petari devised a HW hack/fix where '2' attenuates YM.
    The option attenuates YM sound ('2' or not), it's a hack.

TODO: shift on the fly to keep precision when interpolating?
*/
  int shift=1;
  if(IS_STE&&OPTION_MICROWIRE)
  {
    if(check_dmamix&&!Microwire.mixer) // only on write to register
      shift=16; // zeroes
    else if(OPTION_YM_12DB)
      shift=2;
  }
  for(int i=0;i<16*16*16;i++)
    p_fixed_vol_3voices[i]>>=shift; 
#endif
  SSEConfig.ym2149_fixed_vol=ok;
  return ok;
}

#undef LOGSECTION


BYTE TYM2149::CurrentDrive() {
  BYTE drive=NO_VALID_DRIVE; // different from floppy_current_drive()
  switch(psg_reg[PSGR_PORT_A]&(BIT_2|BIT_1)) {
    case BIT_2:
      drive=0;
      break;
    case BIT_1:
      drive=1;
      break;
    case 0:
      if(num_connected_floppies==1) // Captain Dynamo
        drive=0;
      break;
  }
  return drive;
}


#if defined(SSE_DRIVE_FREEBOOT)

void TYM2149::CheckFreeboot() {
  // this is used by CURRENT_SIDE hence the MFM manager
  if(FloppyDrive[SelectedDrive].freeboot)
  {
    SelectedSide=1;
#if defined(SSE_DISK_CAPS) // not tested: no image
    if(FloppyDrive[SelectedDrive].ImageType.Manager==MNGR_CAPS)
      Caps.Drive[Caps.fdc.drivenew].newside=1;
#endif
#if USE_PASTI // not tested: no image
    if(hPasti&&(pasti_active
      ||FloppyDrive[SelectedDrive].ImageType.Extension==EXT_STX))
    {
      pastiPEEKINFO ppi;
      pasti->Peek(&ppi);
      BYTE porta=ppi.drvSelect;
      porta&=~BIT_0;
      pasti->WritePorta(porta,ABSOLUTE_CPU_TIME);
    }  
#endif
  }
}

#endif


void TYM2149::Reset() {
  for(int n=0;n<16;n++) 
    psg_reg[n]=0;
  psg_reg[PSGR_PORT_A]=0xff; // Set DTR RTS, no drive, side 0, strobe and GPO
#if defined(SSE_YM2149_LL)
  m_rng = 1; //it will take 2exp17=131072 values
  m_output[0] = 0;
  m_output[1] = 0;
  m_output[2] = 0;
  m_count[0] = 0;
  m_count[1] = 0;
  m_count[2] = 0;
  m_count_noise = 0;
  m_count_env = 0;
  m_prescale_noise = 0;
  m_cycles=0; 
  // 32 steps for envelope in YM - we do it at reset for old snapshots
  m_env_step_mask=ENVELOPE_MASK; 
  m_holding=0;
  m_attack=15;
  m_hold=1; // Captain Blood
  time_at_vbl_start=0;
  time_of_last_sample=0;
#endif
}


#if defined(SSE_YM2149_LL)

#define NOISE_ENABLEQ(_chan)  ((psg_reg[PSGR_MIXER] >> (3 + _chan)) & 1)
#define TONE_ENABLEQ(_chan)   ((psg_reg[PSGR_MIXER] >> (_chan)) & 1)

#if defined(SSE_DEBUGGER_MUTE_SOUNDCHANNELS)
#define NOISE_OUTPUT() (((1<<11)&d2_dpeek(FAKE_IO_START+20))?0:(m_rng & 1))
#else
#define NOISE_OUTPUT()          (m_rng & 1)
#endif

#define TONE_PERIOD(_chan)   \
  ( psg_reg[(_chan) << 1] | ((psg_reg[((_chan) << 1) | 1] & 0x0f) << 8) )
#define ENVELOPE_PERIOD()       ((psg_reg[PSGR_ENVELOPE_PERIOD_LOW] \
  | (psg_reg[PSGR_ENVELOPE_PERIOD_HIGH]<<8)))

void TYM2149::psg_write_buffer(DWORD to_t, bool vbl) {
  //ASSERT(OPTION_MAME_YM);

  if(!psg_n_samples_this_vbl||!SSEConfig.YmSoundOn)
    return;

  // compute #samples at our current sample rate
  DWORD t=(psg_time_of_last_vbl_for_writing+psg_buf_pointer[0]);
  to_t=MAX(to_t,t);
  to_t=MIN(to_t,psg_time_of_last_vbl_for_writing+PSG_CHANNEL_BUF_LENGTH);
  int count=MAX(MIN((DWORD)(to_t-t),PSG_CHANNEL_BUF_LENGTH-psg_buf_pointer[0]),0ul);
  if(!count && !AntiAlias)
    return;

  int *p=psg_channels_buf+psg_buf_pointer[0];
  //ASSERT(p-psg_channels_buf<=PSG_CHANNEL_BUF_LENGTH);

  if(!AntiAlias)
    *p=0;

  //ASSERT(sound_freq);
  // YM2149 @2mhz = 1/4 * 8mhz clock  - On STF, same as CPU
  // when is next sample due?
  COUNTER_VAR time_to_send_next_sample=(m_cycles+(int)ym2149_cycles_per_sample);

  COUNTER_VAR ym2149_cycles_at_start_of_loop=m_cycles;
  int samples_sent=0;

  COUNTER_VAR cycles_to_run=0;
  if(AntiAlias)
  {
    COUNTER_VAR cpu_frame_cycles=FRAMECYCLES;
    COUNTER_VAR psg_frame_cycles=cpu_frame_cycles/4;
    COUNTER_VAR psg_already_run=m_cycles-time_at_vbl_start;
    if(psg_already_run<0)
      time_at_vbl_start=m_cycles,psg_already_run=0;
    cycles_to_run=psg_frame_cycles-psg_already_run;
    if(cycles_to_run<=0)
      return;
  }

/*  The following was inspired by MAME project, especially ay8910.cpp.
    thx Couriersud.
    Notice the emulation is both simple and short. But tests in VS2015 profiler
    show that it uses more CPU than Steem's way (PREPARE, ADVANCE...), a good
    deal of that is used by the antialiasing filter.
    We take advantage of more powerful computers...
*/

  /* The 8910 has three outputs, each output is the mix of one of the three */
  /* tone generators and of the (single) noise generator. The two are mixed */
  /* BEFORE going into the DAC. The formula to mix each channel is: */
  /* (ToneOn | ToneDisable) & (NoiseOn | NoiseDisable). */
  /* Note that this means that if both tone and noise are disabled, the output */
  /* is 1, not 0, and can be modulated changing the volume. */

  /* buffering loop */
  
  for(int i=0; 
    // override vbl: Steem may want to run longer TODO
    (AntiAlias&&!vbl)?(i<cycles_to_run):
    count;i+=8)
  {
    m_cycles+=8;  //the driver is clocked with clock / 8  (250Khz)

    //SS We compute noise then envelope, then we compute each tone and
    // mix each channel

    m_count_noise++;

    if (m_count_noise >= (psg_reg[PSGR_NOISE_PERIOD] & 0x1f))
    {
      /* toggle the prescaler output. Noise is no different to
       * channels.
       */
      m_count_noise = 0;
      //ASSERT(!(m_prescale_noise&0xfe));
      m_prescale_noise ^= 1;

      if (m_prescale_noise)
      {
        /* The Random Number Generator of the 8910 is a 17-bit shift */
        /* register. The input to the shift register is bit0 XOR bit3 */
        /* (bit0 is the output). This was verified on AY-3-8910 and YM2149 chips. */
        //ASSERT(m_rng);
        m_rng ^= (((m_rng & 1) ^ ((m_rng >> 3) & 1)) << 17);
        m_rng >>= 1;
      }
    }

    /* update envelope */
    //ASSERT(m_env_step_mask==ENVELOPE_MASK);
    //ASSERT(!(m_holding&0xfe));

    if (m_holding == 0)
    {
      m_count_env++;
      if (m_count_env >= ENVELOPE_PERIOD()) // "m_step"=1 for YM2149
      {
        m_count_env = 0;
        m_env_step--;

        /* check envelope current position */
        if (m_env_step < 0)
        {
          if (m_hold)
          {
            if (m_alternate)
              m_attack ^=m_env_step_mask;
            m_holding = 1;
            m_env_step = 0;
          }
          else
          {
            /* if CountEnv has looped an odd number of times (usually 1), */
            /* invert the output. */
            if (m_alternate && (m_env_step & (m_env_step_mask + 1)))
              m_attack ^= m_env_step_mask;

            m_env_step &= m_env_step_mask;
          }
          //ASSERT(m_env_step>=0);
        }

      }
    }
    m_env_volume = (m_env_step ^ m_attack);
    //ASSERT(m_env_volume>=0 && m_env_volume<32);

    //as in psg's AlterV
    BYTE index[3],interpolate[4];
    *(int*)interpolate=0;
    int vol=0;
    //TRACE_OSD("%d %d %d",TONE_PERIOD(0),TONE_PERIOD(1),TONE_PERIOD(2));
    for(int abc=0;abc<3;abc++)
    {
      // Tone
      bool enveloped=((psg_reg[abc+8] & BIT_4)!=0);

      m_count[abc]++;
#if defined(SSE_DEBUGGER_MUTE_SOUNDCHANNELS)
      if(!((4>>abc)&(d2_dpeek(FAKE_IO_START+20)>>12))) // chan      
#endif
      if(m_count[abc]>=TONE_PERIOD(abc)) 
      {
        //ASSERT(!(m_output[abc]&0xfe));
        m_output[abc] ^= 1;
        m_count[abc]=0;
      }

      // mixing
      m_vol_enabled[abc] = (m_output[abc] | TONE_ENABLEQ(abc)) 
        & (NOISE_OUTPUT() | NOISE_ENABLEQ(abc));

      // from here on, specific to Steem rendering (different options)
      int digit=(enveloped)?BIT_6:0;
      // pick correct volume or 0
      if(!m_vol_enabled[abc])
        ; // 0
      else if(enveloped)
      {
#if defined(SSE_DEBUGGER_MUTE_SOUNDCHANNELS)
        if(!((1<<10)&d2_dpeek(FAKE_IO_START+20))) // env
#endif
        digit|=m_env_volume; // vol 5bit
      }
      else
        digit |=(psg_reg[abc+8] & 15)<<1; // vol 4bit shifted

      index[abc]=(digit >>1)&0xF; // 4bit volume
      interpolate[abc]=((digit&BIT_6) && index[abc]>0 && !(digit &1) ) ? 1 : 0;
    }//nxt abc
    // because of averaging, we "render" the 4/5bit samples now
    vol=p_fixed_vol_3voices[(16*16)*index[2]+16*index[1]+index[0]];
    if(*(int*)(&interpolate[0]))
    {
      int vol2=p_fixed_vol_3voices[(16*16)*(index[2]-interpolate[2])
        +16*(index[1]-interpolate[1])+(index[0]-interpolate[0])];
      vol=(int)sqrt((float)vol * (float)vol2);
    }
    // Thanks to this kick-ass filter, we can avoid horrible aliasing in all
    // sample rates.
    *p=(AntiAlias) ? (int)AntiAlias->do_sample((double)vol) : vol;

    if(m_cycles-time_to_send_next_sample>=0
      && (unsigned)(p-psg_channels_buf)<=PSG_CHANNEL_BUF_LENGTH)
    {
      int copy=*p;
      *(++p)=copy; //same value, not zero
      count--;
      samples_sent++;
      frame_samples++;
      time_of_last_sample=m_cycles;
      if(AntiAlias)
      {
        time_to_send_next_sample=(COUNTER_VAR)((double)time_at_vbl_start
          +((double)frame_samples+1.0)*ym2149_cycles_per_sample);
      }
      else
        time_to_send_next_sample=ym2149_cycles_at_start_of_loop
          +(int)(((double)samples_sent+1)*ym2149_cycles_per_sample);
    }
  }
  psg_buf_pointer[0]=to_t-psg_time_of_last_vbl_for_writing;
  psg_buf_pointer[2]=psg_buf_pointer[1]=psg_buf_pointer[0];
}


void TYM2149::Restore() {
  m_env_step_mask=ENVELOPE_MASK;
  if(sound_freq)
    ym2149_cycles_per_sample=((double)CpuNormalHz/4)/(double)sound_freq;
}

#endif//mame-like
