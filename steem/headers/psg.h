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
FILE: psg.h
DESCRIPTION: Declarations for the Yamaha YM2149 PSG (Programmable Sound
Chip).
struct TYM2149
---------------------------------------------------------------------------*/

#pragma once
#ifndef SSEYM2149_H
#define SSEYM2149_H

#include <sound.h>
#include "dsp/FIR-filter-class/filt.h" // 3rd party

#define NUM_CHANNELS 3

/*  In v3.5.1, TYM2149 was only used for drive management (drive, side).
    In v3.7.0, sound functions were introduced (sampled soundchip, more realistic).
    In v3.9.2, it harbours an alternative PSG emu based on MAME.
*/

#pragma pack(push, 8)

struct TYM2149 {
  enum EYM2149 {NO_VALID_DRIVE=0xFF};
  // FUNCTIONS
  TYM2149();
  ~TYM2149();
  BYTE CurrentDrive(); // may return NO_VALID_DRIVE
  void CheckFreeboot();
  void FreeFixedVolTable();
  bool LoadFixedVolTable(bool check_dmamix=false);
  void Reset();
  void Restore();
#if defined(SSE_YM2149_LL)
  void psg_write_buffer(DWORD to_t, bool vbl=false);
#endif
  // DATA
#if defined(SSE_YM2149_LL)
  double ym2149_cycles_per_sample;
#endif
  WORD *p_fixed_vol_3voices;
#if defined(SSE_YM2149_LL)
  Filter* AntiAlias;
  COUNTER_VAR m_cycles; // added
  COUNTER_VAR time_at_vbl_start,time_of_last_sample;
  int frame_samples;
  int m_count[NUM_CHANNELS];
  int m_count_noise;
  int m_count_env;
  int m_rng;
  DWORD m_env_volume;
  BYTE m_output[NUM_CHANNELS];
  BYTE m_env_step_mask;
  char m_env_step;
  BYTE m_hold,m_alternate,m_attack,m_holding;
  BYTE m_prescale_noise;
  BYTE m_vol_enabled[NUM_CHANNELS];
#endif
  BYTE SelectedDrive; //0/1 (use CurrentDrive() to check validity)
  BYTE SelectedSide;  //0/1
};

#pragma pack(pop)



extern BYTE psg_reg[16],psg_reg_data;
extern int psg_voltage,psg_dv;
#define PSGR_PORT_A 14
#define PSGR_PORT_B 15

void psg_write_buffer(int,DWORD);

#define PSG_NOISE_ARRAY 8192
#define MOD_PSG_NOISE_ARRAY & 8191


extern DWORD psg_tone_start_time[3];
extern char psg_noise[PSG_NOISE_ARRAY];

#define PSGR_NOISE_PERIOD 6
#define PSGR_MIXER 7
#define PSGR_AMPLITUDE_A 8
#define PSGR_AMPLITUDE_B 9
#define PSGR_AMPLITUDE_C 10
#define PSGR_ENVELOPE_PERIOD 11
#define PSGR_ENVELOPE_PERIOD_LOW 11
#define PSGR_ENVELOPE_PERIOD_HIGH 12
#define PSGR_ENVELOPE_SHAPE 13

/*
       |     |13 Envelope Shape                         BIT 3 2 1 0|
       |     |  Continue -----------------------------------' | | ||
       |     |  Attack ---------------------------------------' | ||
       |     |  Alternate --------------------------------------' ||
       |     |  Hold ---------------------------------------------'|
       |     |   00xx - \____________________________________      |
       |     |   01xx - /|___________________________________      |
       |     |   1000 - \|\|\|\|\|\|\|\|\|\|\|\|\|\|\|\|\|\|\      |
       |     |   1001 - \____________________________________      |
       |     |   1010 - \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\      |
       |     |   1011 - \|-----------------------------------      |
       |     |   1100 - /|/|/|/|/|/|/|/|/|/|/|/|/|/|/|/|/|/|/      |
       |     |   1101 - /------------------------------------      |
       |     |   1110 - /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/      |
       |     |   1111 - /|___________________________________      |

*/

#define PSG_ENV_SHAPE_HOLD BIT_0
#define PSG_ENV_SHAPE_ALT BIT_1
#define PSG_ENV_SHAPE_ATTACK BIT_2
#define PSG_ENV_SHAPE_CONT BIT_3

#if defined(SSE_SOUND_16BIT_CENTRED) // see note in TYM2149::LoadFixedVolTable() 
#define PSG_CHANNEL_AMPLITUDE 40
#else
#define PSG_CHANNEL_AMPLITUDE 60
#endif

//#define PSG_VOLSCALE(vl) (volscale[vl]/4+VOLTAGE_ZERO_LEVEL)
/*
#define PSG_ENV_DOWN 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
#define PSG_ENV_UP   00,01,02,03,04,05,6,7,8,9,10,11,12,13,14,15
#define PSG_ENV_0    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
#define PSG_ENV_LOUD 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15

const BYTE psg_envelopes[8][32]={
    {PSG_ENV_DOWN,PSG_ENV_DOWN},
    {PSG_ENV_DOWN,PSG_ENV_0},
    {PSG_ENV_DOWN,PSG_ENV_UP},
    {PSG_ENV_DOWN,PSG_ENV_LOUD},
    {PSG_ENV_UP,PSG_ENV_UP},
    {PSG_ENV_UP,PSG_ENV_LOUD},
    {PSG_ENV_UP,PSG_ENV_DOWN},
    {PSG_ENV_UP,PSG_ENV_0}};

*/
//const int psg_flat_volume_level[16]={0*VA/1000+VZL*VFP,0*VA/1000+VZL*VFP,2*VA/1000+VZL*VFP,5*VA/1000+VZL*VFP,20*VA/1000+VZL*VFP,50*VA/1000+VZL*VFP,65*VA/1000+VZL*VFP,80*VA/1000+VZL*VFP,100*VA/1000+VZL*VFP,125*VA/1000+VZL*VFP,178*VA/1000+VZL*VFP,250*VA/1000+VZL*VFP,354*VA/1000+VZL*VFP,510*VA/1000+VZL*VFP,707*VA/1000+VZL*VFP,1000*VA/1000+VZL*VFP};
extern const int psg_flat_volume_level[16];

extern const int psg_envelope_level[8][64];

DWORD psg_quantize_time(int,DWORD);
void psg_set_reg(int,BYTE,BYTE&);

extern DWORD psg_envelope_start_time;
extern bool written_to_env_this_vbl;

#define VOLTAGE_ZERO_LEVEL 0
#define VOLTAGE_FIXED_POINT 256
//must now be fixed at 256!
#define VOLTAGE_FP(x) ((x) << 8)

// on release build, inline functions were tested and perform even better 
// than the original macros

#define CALC_V_CHIP             1
#define CALC_V_CHIP_25KHZ       2
#define CALC_V_EMU              3


//CALC_V_CHIP
inline void CalcVChip(int &v,int &dv,int *source_p) {  
  if(psg_hl_filter==SOUND_PSG_HLF_MONITOR) 
  {
    v=(*source_p+dv)/2;
    dv=v;
  }
  else if(v!=*source_p || dv)
  {
    v+=dv;                                            
    dv-=(v-(*source_p)) >> 3;                         
    dv*=13;                                           
    dv>>=4; 
  }
}


//CALC_V_CHIP_25KHZ = low quality
inline void CalcVChip25Khz(int &v,int &dv,int *source_p) { 
  if(psg_hl_filter==SOUND_PSG_HLF_MONITOR)
  {
    v=(*source_p+dv)/2;
    dv=v;
  }
  else if(v!=*source_p||dv)
  {
    v+=dv;                                            
    dv-=((v-(*source_p)) *3) >>3;                         
    dv*=3;                                           
    dv>>=2;                                           
  }
}


//CALC_V_EMU  = direct
inline void CalcVEmu(int &v,int *source_p) { 
  v=*source_p;
}


inline void AlterV(int Alter_V,int &v,int &dv,int *source_p) {
  // Dispatch to the correct function  
  if(Alter_V==CALC_V_CHIP)                    
    CalcVChip(v,dv,source_p);                 
  else if(Alter_V==CALC_V_CHIP_25KHZ)         
    CalcVChip25Khz(v,dv,source_p);            
  else if(Alter_V==CALC_V_EMU)                
    CalcVEmu(v,source_p);
}


#undef NUM_CHANNELS

#endif//SSEYM2149_H
