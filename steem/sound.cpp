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

DOMAIN: Rendering
FILE: sound.cpp
DESCRIPTION: The guts of Steem's sound output code, uses a DirectSound buffer
for output. 
Sound_VBL is the main function writing one frame of sound to the output buffer. 
Outputs from the PSG (YM2149) and the GST Shifter (STE sound) are mixed.
The I/O code isn't included here, see io?.cpp.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <computer.h>
#include <translate.h>
#include <gui.h>
#include <mymisc.h>
#include <notifyinit.h>
#if defined(SSE_VID_RECORD_AVI)
#include <AVI/AviFile.h> // AVI (DD-only)
#endif


FILE *Wav_file=NULL;
int UseSound=0; // not bool for Linux build
int sound_freq=44100,sound_comline_freq=0,sound_chosen_freq=44100;
int MaxVolume=10000;
DWORD sound_record_start_time; //by timer variable = timeGetTime()
int psg_write_n_screens_ahead=3;// UNIX_ONLY(+7);
#if defined(ENABLE_LOGFILE) || defined(SHOW_WAVEFORM)
DWORD min_write_time;
DWORD play_cursor,write_cursor;
#endif
int sound_buffer_length=DEFAULT_SOUND_BUFFER_LENGTH;
DWORD SoundBufStartTime;
#if (SCREENS_PER_SOUND_VBL == 1)
#define MOD_PSG_BUF_LENGTH %PSG_BUF_LENGTH
int cpu_time_of_last_sound_vbl=0;
#endif
DWORD psg_last_play_cursor;
DWORD psg_last_write_time;
DWORD psg_time_of_start_of_buffer;
DWORD psg_time_of_last_vbl_for_writing,psg_time_of_next_vbl_for_writing;
int psg_n_samples_this_vbl;
#if defined(SSE_YM2149_LL)
const WORD ym_low_pass_max=YM_LOW_PASS_MAX;
#endif
bool sound_first_vbl;
BYTE sound_num_channels=2;
BYTE sound_num_bits=16;
BYTE sound_bytes_per_sample=4; //(sound_num_bits/8)*sound_num_channels
bool sound_low_quality=0;

#if defined(SSE_SOUND_ENFORCE_RECOM_OPT)
const bool sound_write_primary=false;
const BYTE sound_time_method=1; // write cursor
#else
bool sound_write_primary=false;
BYTE sound_time_method=1; // write cursor
#endif


bool sound_click_at_start=false;
bool sound_record=false;


BYTE psg_hl_filter=SOUND_PSG_HLF_EMULATED;
bool ste_sound_on_this_screen=0;
bool TrySound=true;
#ifdef SHOW_WAVEFORM
int temp_waveform_display_counter;
BYTE temp_waveform_display[DEFAULT_SOUND_BUFFER_LENGTH];
DWORD temp_waveform_play_counter;
#endif


#ifdef WIN32

#if 0 // not needed by BCC, VS2008, MINGW...
SET_GUID(CLSID_DirectSound,0x47d4d946,0x62e8,0x11cf,0x93,0xbc,0x44,0x45,0x53,
  0x54,0x0,0x0);
SET_GUID(IID_IDirectSound,0x279AFA83,0x4981,0x11CE,0xA5,0x21,0x00,0x20,0xAF,
  0x0B,0xE5,0x60);
#endif

HRESULT DSGetPrimaryBuffer(); // forwards
HRESULT DSCreateSoundBuf();
void CALLBACK DSStopBufferTimerProc(HWND,UINT,UINT_PTR,DWORD);
BOOL CALLBACK DSEnumProc(LPGUID Guid,LPCSTR Desc,LPCSTR,LPVOID);
HRESULT DSReleaseAllBuffers(HRESULT Ret=DS_OK);

IDirectSound *DSObj=NULL;
IDirectSoundBuffer *PrimaryBuf=NULL,*SoundBuf=NULL;
DWORD DS_SetFormat_freq;
DSCAPS SoundCaps;
WAVEFORMATEX PrimaryFormat;
UINT DSStopBufferTimerID=0;
bool DS_GetFormat_Wrong=false;
bool DSOpen=false;

#endif//WIN32


#ifdef UNIX

EasyStr sound_device_name; // =("/dev/dsp");
int console_device=-1;
int sound_buf_pointer=0; // sample count
BYTE x_sound_buf[X_SOUND_BUF_LEN_BYTES+16];

void XSoundInitBuffer(int,int);

int x_sound_lib=
#if !defined(NO_RTAUDIO)
  XS_RT;
#elif !defined(NO_PORTAUDIO)
  XS_PA;
#endif

#endif//UNIX


#define LOGSECTION LOGSECTION_SOUND

void sound_record_open_file() {
  if(Wav_file)
    return;
  Wav_file=fopen(WAVOutputFile.Text,"wb");
  if(Wav_file==NULL)
  {
    Alert(T("Could not open sound file for writing"),T("Sound Recording Error"),
      MB_ICONEXCLAMATION);
    sound_record=false;
    return;
  }
  if(OPTION_SOUND_RECORD_FORMAT==TOption::SoundFormatWav)
  {
    fprintf(Wav_file,"RIFF    WAVEfmt ");
    fputc(16,Wav_file);  
    fputc(0,Wav_file);  
    fputc(0,Wav_file);  
    fputc(0,Wav_file); //size of header=16
    fputc(1,Wav_file);  
    fputc(0,Wav_file); //always
    for(int i=0;i<14;i++) 
      fputc(0,Wav_file); // Skip header (written when close)
    fprintf(Wav_file,"data    ");
  }
  // Need to put size of file - 44 at position 40 in file (as int in binary little endian)
  // Need to put size of file - 8 at position 4 in file (as int in binary little endian)
  // Need to put header of file at position 0x16 in file
}


void SaveInt(int i,FILE *f) { // was in mymisc, used only here
  int a=i;
  SWAP_LITTLE_ENDIAN_DWORD(a); // we think of everything!
  fwrite(&a,sizeof(i),1,f);
}


void sound_record_close_file() {
  if(!Wav_file)
    return;
  fflush(Wav_file);
  size_t length=ftell(Wav_file);
/*  Convert temp file to YM3 format.
    temp file is a dump of PSG registers every VBL, in YM3, the same data
    is grouped per register.
    To do that we copy the temp file in memory, then create it again in
    the correct order.
*/
  if(!length) // just in case
    ;
  else if(OPTION_SOUND_RECORD_FORMAT==TOption::SoundFormatYm)
  {
    //ASSERT(length%14==0);
    size_t nframes=length/14;
    fclose(Wav_file); //must close/open for reading! (is open as "wb")
    Wav_file=fopen(WAVOutputFile.Text,"rb");
    BYTE *copy=new BYTE[length];
    fread(copy,sizeof(BYTE),length,Wav_file);
    fclose(Wav_file); // close and reopen as if new file
    Wav_file=fopen(WAVOutputFile.Text,"wb");
    fprintf(Wav_file,"YM3!");  //header
    // change order
    for(int reg=0;reg<14;reg++)
      for(size_t i=0;i<nframes;i++)
        fwrite(&copy[i*14+reg],sizeof(BYTE),1,Wav_file);
    delete[] copy; // and wavfile will be closed further
    written_to_env_this_vbl=true; // reset this for next recording
  }
  else
  {
    fseek(Wav_file,4,SEEK_SET);
    SaveInt((int)(length-8),Wav_file);
    fseek(Wav_file,40,SEEK_SET);
    SaveInt((int)(length-44),Wav_file);
    // Write out header
    fseek(Wav_file,0x16,SEEK_SET);
    fputc(sound_num_channels,Wav_file); 
    fputc(0,Wav_file);
    SaveInt(sound_freq,Wav_file);
    SaveInt(sound_freq*sound_bytes_per_sample,Wav_file); //bytes per second
    fputc(sound_bytes_per_sample,Wav_file);  
    fputc(0,Wav_file);
    fputc(sound_num_bits,Wav_file);  
    fputc(0,Wav_file);
  }
  fclose(Wav_file);
  Wav_file=NULL;
  sound_record=false;
  OptionBox.UpdateRecordBut();
}


HRESULT InitSound() {
  SoundRelease();
  HRESULT Ret;

#ifdef WIN32
  SetNotifyInitText("DirectSound");
  // Hey, this allows Steem to run even if there is no DSound.dll
  DBG_LOG("SOUND: Attempting to load dsound.dll");
  HINSTANCE hDSDll=LoadLibrary("dsound");
  if(hDSDll)
  {
    typedef HRESULT WINAPI DSENUMPROC(LPDSENUMCALLBACK,LPVOID);
    typedef DSENUMPROC* LPDSENUMPROC;
    LPDSENUMPROC DSEnum=(LPDSENUMPROC)GetProcAddress(hDSDll,"DirectSoundEnumerateA");
    DSDriverModuleList.DeleteAll();
    DSDriverModuleList.Sort=eslNoSort;
    DBG_LOG("SOUND: Attempting to enumerate devices");
    if(DSEnum!=NULL) 
      DSEnum(DSEnumProc,NULL);
    DBG_LOG("SOUND: Freeing library");
    FreeLibrary(hDSDll);
  }
  DBG_LOG("SOUND: Initialising, creating DirectSound object");
  Ret=CoCreateInstance(CLSID_DirectSound,NULL,CLSCTX_ALL,IID_IDirectSound,(void**)&DSObj);
  if(Ret!=S_OK||DSObj==NULL)
  {
    DSObj=NULL;
    EasyStr Err="Unknown error";
    switch(Ret) {
    case REGDB_E_CLASSNOTREG:
      Err="The specified class is not registered in the registration database.";
      break;
    case E_OUTOFMEMORY:
      Err="Out of memory.";
      break;
    case E_INVALIDARG:
      Err="One or more arguments are invalid.";
      break;
    case E_UNEXPECTED:
      Err="An unexpected error occurred.";
      break;
    case CLASS_E_NOAGGREGATION:
      Err="This class cannot be created as part of an aggregate.";
      break;
    }
    Err=EasyStr("SOUND: CoCreateInstance error\n\n")+Err;
    log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    log_write(Err);
    log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
#ifndef ONEGAME
    MessageBox(NULL,Err,T("Steem Engine DirectSound Error"),
      MB_ICONEXCLAMATION|MB_SETFOREGROUND|MB_TASKMODAL|MB_TOPMOST);
#endif
    return ~DS_OK;
  }
  LPGUID Driver=NULL;
  EasyStr DSDriverModName=GetCSFStr("Options","DSDriverName","",globalINIFile);
  if(DSDriverModName.NotEmpty())
  {
    for(int i=0;i<DSDriverModuleList.NumStrings;i++)
    {
      if(IsSameStr_I(DSDriverModuleList[i].String,DSDriverModName))
      {
        Driver=LPGUID(DSDriverModuleList[i].Data[0]);
        TRACE_INIT("Sound driver %s\n",DSDriverModName.Text);
        break;
      }
    }
  }
  DBG_LOG("SOUND: Initialising DirectSound object");
  Ret=DSObj->Initialize(Driver);
  if(Ret!=DS_OK) 
    return SoundError("DSObj Initialise Failed",Ret);
  DBG_LOG("SOUND: Calling SetCooperativeLevel");
  DSObj->SetCooperativeLevel(StemWin,DSSCL_PRIORITY);
  DBG_LOG("SOUND: Calling GetCaps");
  SoundCaps.dwSize=sizeof(SoundCaps);
  Ret=DSObj->GetCaps(&SoundCaps);
  if(Ret!=DS_OK) 
    return SoundError("GetCaps Failed",Ret);
#ifdef ENABLE_LOGFILE
  DBG_LOG("------ Sound capabilities: ------");
  DBG_LOG(EasyStr("dwSize=")+SoundCaps.dwSize);
  DBG_LOG(EasyStr("dwFlags=")+itoa(SoundCaps.dwFlags,d2_t_buf,2));
  if(SoundCaps.dwFlags & DSCAPS_PRIMARYMONO) DBG_LOG("    DSCAPS_PRIMARYMONO  ");
  if(SoundCaps.dwFlags & DSCAPS_PRIMARYSTEREO) DBG_LOG("    DSCAPS_PRIMARYSTEREO");
  if(SoundCaps.dwFlags & DSCAPS_PRIMARY8BIT) DBG_LOG("    DSCAPS_PRIMARY8BIT      ");
  if(SoundCaps.dwFlags & DSCAPS_PRIMARY16BIT) DBG_LOG("    DSCAPS_PRIMARY16BIT     ");
  if(SoundCaps.dwFlags & DSCAPS_CONTINUOUSRATE) DBG_LOG("    DSCAPS_CONTINUOUSRATE   ");
  if(SoundCaps.dwFlags & DSCAPS_EMULDRIVER) DBG_LOG("    DSCAPS_EMULDRIVER       ");
  if(SoundCaps.dwFlags & DSCAPS_CERTIFIED) DBG_LOG("    DSCAPS_CERTIFIED        ");
  if(SoundCaps.dwFlags & DSCAPS_SECONDARYMONO) DBG_LOG("    DSCAPS_SECONDARYMONO    ");
  if(SoundCaps.dwFlags & DSCAPS_SECONDARYSTEREO) DBG_LOG("    DSCAPS_SECONDARYSTEREO  ");
  if(SoundCaps.dwFlags & DSCAPS_SECONDARY8BIT) DBG_LOG("    DSCAPS_SECONDARY8BIT    ");
  if(SoundCaps.dwFlags & DSCAPS_SECONDARY16BIT) DBG_LOG("    DSCAPS_SECONDARY16BIT   ");
  DBG_LOG(EasyStr("dwMinSecondarySampleRate=")+SoundCaps.dwMinSecondarySampleRate);
  DBG_LOG(EasyStr("dwMaxSecondarySampleRate=")+SoundCaps.dwMaxSecondarySampleRate);
  DBG_LOG(EasyStr("dwPrimaryBuffers=")+SoundCaps.dwPrimaryBuffers);
  DBG_LOG(EasyStr("dwMaxHwMixingAllBuffers=")+SoundCaps.dwMaxHwMixingAllBuffers);
  DBG_LOG(EasyStr("dwMaxHwMixingStaticBuffers=")+SoundCaps.dwMaxHwMixingStaticBuffers);
  DBG_LOG(EasyStr("dwMaxHwMixingStreamingBuffers=")+SoundCaps.dwMaxHwMixingStreamingBuffers);
  DBG_LOG(EasyStr("dwFreeHwMixingAllBuffers=")+SoundCaps.dwFreeHwMixingAllBuffers);
  DBG_LOG(EasyStr("dwFreeHwMixingStaticBuffers=")+SoundCaps.dwFreeHwMixingStaticBuffers);
  DBG_LOG(EasyStr("dwFreeHwMixingStreamingBuffers=")+SoundCaps.dwFreeHwMixingStreamingBuffers);
  DBG_LOG(EasyStr("dwMaxHw3DAllBuffers=")+SoundCaps.dwMaxHw3DAllBuffers);
  DBG_LOG(EasyStr("dwMaxHw3DStaticBuffers=")+SoundCaps.dwMaxHw3DStaticBuffers);
  DBG_LOG(EasyStr("dwMaxHw3DStreamingBuffers=")+SoundCaps.dwMaxHw3DStreamingBuffers);
  DBG_LOG(EasyStr("dwFreeHw3DAllBuffers=")+SoundCaps.dwFreeHw3DAllBuffers);
  DBG_LOG(EasyStr("dwFreeHw3DStaticBuffers=")+SoundCaps.dwFreeHw3DStaticBuffers);
  DBG_LOG(EasyStr("dwFreeHw3DStreamingBuffers=")+SoundCaps.dwFreeHw3DStreamingBuffers);
  DBG_LOG(EasyStr("dwTotalHwMemBytes=")+SoundCaps.dwTotalHwMemBytes);
  DBG_LOG(EasyStr("dwFreeHwMemBytes=")+SoundCaps.dwFreeHwMemBytes);
  DBG_LOG(EasyStr("dwMaxContigFreeHwMemBytes=")+SoundCaps.dwMaxContigFreeHwMemBytes);
  DBG_LOG(EasyStr("dwUnlockTransferRateHwBuffers=")+SoundCaps.dwUnlockTransferRateHwBuffers);
  DBG_LOG(EasyStr("dwPlayCpuOverheadSwBuffers=")+SoundCaps.dwPlayCpuOverheadSwBuffers);
  DBG_LOG(EasyStr("dwReserved1=")+SoundCaps.dwReserved1);
  DBG_LOG(EasyStr("dwReserved2=")+SoundCaps.dwReserved2);
  DBG_LOG("---------------------------------");
#endif
  if(SoundCaps.dwMaxSecondarySampleRate
    <SOUND_DESIRED_LQ_FREQ-(SOUND_DESIRED_LQ_FREQ/5))
  {
    // Apparently cannot even achieve lowest possible frequency
    SoundCaps.dwMaxSecondarySampleRate=100000; //Ignore!
  }
  UseSound=1;
#endif//WIN32

#ifdef UNIX
  Ret=DSERR_GENERIC;
#ifndef NO_PORTAUDIO
  if(x_sound_lib==XS_PA)
    Ret=PA_Init();
#endif
#ifndef NO_RTAUDIO
  if(x_sound_lib==XS_RT)
    Ret=Rt_Init();
#endif
  if(Ret==DSERR_GENERIC)
    UseSound=0;
#endif

  return Ret;
}


void SoundRelease() {

#ifdef UNIX
  PA_ONLY( PA_Release(); )
  RT_ONLY( Rt_Release(); )
#endif

  UseSound=0;

#ifdef WIN32
  if(DSObj!=NULL)
  {
    DSReleaseAllBuffers();
    DSObj->Release();
    DSObj=NULL;
  }
#endif

#if defined(SSE_YM2149_LL)
  if(Psg.AntiAlias)
  {
    delete Psg.AntiAlias;
    Psg.AntiAlias=NULL;
  }
#endif
}


DWORD SoundGetTime() {
#ifdef WIN32
  if(!DSOpen) 
    return 0;
#if defined(ENABLE_LOGFILE)==0 && defined(SHOW_WAVEFORM)==0
  DWORD play_cursor,write_cursor; // global in debugger
#endif
  DWORD s_time;
  if(sound_time_method<2 
#ifndef SSE_LEAN_AND_MEAN
    && SoundBuf && sound_bytes_per_sample
#endif
    )
  {
    play_cursor=0,write_cursor=0;
/*
Current Play and Write Positions
DirectSound maintains two pointers into the buffer: the current play position
(or play cursor) and the current write position (or write cursor). These
positions are byte offsets into the buffer, not absolute memory addresses.

The IDirectSoundBuffer::Play method always starts playing at the buffer's
current play position. When a buffer is created, the play position is set to
zero. As a sound is played, the play position moves and always points to the
next byte of data to be output. When the buffer is stopped, the play position
remains where it is.

The current write position is the point after which it is safe to write data
into the buffer. The block between the current play position and the current
write position is already committed to be played, and cannot be changed safely.

Visualize the buffer as a clock face, with data written to it in a clockwise
direction. The play position and the write position are like two hands sweeping
around the face at the same speed, the write position always keeping a little
ahead of the play position. If the play position points to the 1 and the write
position points to the 2, it is only safe to write data after the 2. Data
between the 1 and the 2 may already have been queued for playback by
DirectSound and should not be touched.

Note  The write position moves with the play position, not with data written to
the buffer. If you're streaming data, you are responsible for maintaining your
own pointer into the buffer to indicate where the next block of data should be
written.

Also note that the dwWriteCursor parameter to the IDirectSoundBuffer::Lock
method is not the current write position; it is the offset within the buffer
where you actually intend to begin writing data. (If you do want to begin
writing at the current write position, you specify DSBLOCK_FROMWRITECURSOR in
the dwFlags parameter. In this case the dwWriteCursor parameter is ignored.)

An application can retrieve the current play and write positions by calling the
IDirectSoundBuffer::GetCurrentPosition method. The
IDirectSoundBuffer::SetCurrentPosition method lets you set the current play
position, but the current write position cannot be changed.

-> 'write cursor' is the right method, enforced now (sound_time_method is const)

*/
    SoundBuf->GetCurrentPosition(&play_cursor,&write_cursor);
    DWORD cursor= (sound_time_method==0) ? play_cursor : write_cursor;
    cursor/=sound_bytes_per_sample;
    if(cursor<psg_last_play_cursor)
      psg_time_of_start_of_buffer+=sound_buffer_length;
    s_time=psg_time_of_start_of_buffer+cursor;
    psg_last_play_cursor=cursor;
  }
  else
  { //method in ms
    DWORD mSecs=timeGetTime()-SoundBufStartTime;
    s_time=(mSecs*sound_freq)/1000;
  }
#ifdef ENABLE_LOGFILE
  min_write_time=((write_cursor-play_cursor) MOD_PSG_BUF_LENGTH)+s_time;
#endif
  return s_time;
#endif//WIN32

#ifdef UNIX
  PA_ONLY( if (UseSound==XS_PA) return PA_GetTime(); )
  RT_ONLY( if (UseSound==XS_RT) return Rt_GetTime(); )
  return 0;
#endif
}


HRESULT SoundStartBuffer(int flatlevel1,int flatlevel2) {
  HRESULT Ret=DSERR_GENERIC;

#ifdef WIN32
  if(UseSound==0) 
    return Ret;
  void *DatAdr,*DatAdr2;
  DWORD LockLength,LockLength2;
  if(DSStopBufferTimerID) 
    KillTimer(NULL,DSStopBufferTimerID);
  DSStopBufferTimerID=0;
  DSReleaseAllBuffers();
  Ret=DSGetPrimaryBuffer();
  if(Ret!=DS_OK)
    return Ret;
  Ret=DSCreateSoundBuf();
  if(Ret!=DS_OK) 
    return Ret;
  //ASSERT(SoundBuf);
  sound_low_quality=(sound_freq<35000);
  if(sound_write_primary)
  {
    Ret=DSObj->SetCooperativeLevel(StemWin,DSSCL_WRITEPRIMARY);
    if(Ret!=DS_OK)
    {
      LOG_ONLY(SoundLogError("SOUND: SetCooperativeLevel for DSSCL_WRITEPRIMARY failed\r\n\r\n",Ret); )
        return DSReleaseAllBuffers(Ret);
    }
  }
  Ret=SoundBuf->Lock(0,0,&DatAdr,&LockLength,&DatAdr2,&LockLength2,
    DSBLOCK_ENTIREBUFFER);
  if(Ret==DSERR_BUFFERLOST)
  {
    Ret=SoundBuf->Restore();
    if(Ret==DS_OK) 
      Ret=SoundBuf->Lock(0,0,&DatAdr,&LockLength,&DatAdr2,&LockLength2,
        DSBLOCK_ENTIREBUFFER);
  }
  if(Ret!=DS_OK)
  {
    LOG_ONLY(SoundLogError("SOUND: Lock for Sound Buffer Failed\r\n\r\n",Ret); )
    return DSReleaseAllBuffers(Ret);
  }
  // Want to set the value to the highest byte
  BYTE *p=LPBYTE(DatAdr);
  BYTE *p_end=LPBYTE(DatAdr)+LockLength;
  int current_level=(sound_num_bits==8) ? 128 : 0;
  if(sound_click_at_start) 
    current_level=flatlevel1;
  double v[2]={(double)current_level,(double)current_level};
  double inc[2]={double(flatlevel1-v[0])/600,double(flatlevel2-v[1])/600};
  while(p && p<p_end)
  {
    if(sound_num_bits==8)
    {
      *(p++)=BYTE(v[0]);
      if(sound_num_channels==2) 
        *(p++)=BYTE(v[1]);
    }
    else
    {
      *LPWORD(p)=WORD(char(v[0])<<8);p+=2;
      if(sound_num_channels==2)
      {
        *LPWORD(p)=WORD(char(v[1])<<8);
        p+=2;
      }
    }
    if(int(v[0])!=flatlevel1) 
      v[0]+=inc[0];
    if(int(v[1])!=flatlevel2) 
      v[1]+=inc[1];
  }
  SoundBuf->Unlock(DatAdr,LockLength,DatAdr2,LockLength2);
  SoundBuf->SetVolume(MaxVolume);
  SoundBuf->Play(0,0,DSBPLAY_LOOPING);
  SoundBufStartTime=timeGetTime();
  return DS_OK;
#endif//WIN32

#ifdef UNIX
  switch(UseSound) {
#ifndef NO_PORTAUDIO
  case XS_PA:
    Ret=PA_StartBuffer(flatlevel1,flatlevel2);
    break;
#endif
#ifndef NO_RTAUDIO
  case XS_RT:
    Ret=Rt_StartBuffer(flatlevel1,flatlevel2);
    break;
#endif
  }//sw
  if(Ret==DS_OK)
  {
    SoundBufStartTime=timeGetTime();
    sound_low_quality=(sound_freq<35000);
    SoundCreateDynamicBuffer();    
  }
  return Ret;
#endif
}


bool SoundActive() {
#ifdef WIN32
  return (UseSound && DSOpen);
#endif

#ifdef UNIX
  PA_ONLY( if (UseSound==XS_PA) return PA_IsPlaying(); )
  RT_ONLY( if (UseSound==XS_RT) return Rt_IsPlaying(); )
  return 0;
#endif
}


HRESULT Sound_Stop() {
  sound_record_close_file();
  sound_record=false;
#if !defined(SSE_NO_INTERNAL_SPEAKER)
  if(sound_internal_speaker) SoundStopInternalSpeaker();
#endif

#ifdef WIN32
  DSReleaseAllBuffers();
#endif

#ifdef UNIX
  PA_ONLY( if (UseSound==XS_PA) return PA_Stop(0); )
  RT_ONLY( if (UseSound==XS_RT) return Rt_Stop(0); )
#endif

#if defined(SSE_YM2149_LL)
  if(Psg.AntiAlias)
  {
    delete Psg.AntiAlias;
    Psg.AntiAlias=NULL;
  }
#endif
  return DS_OK;
}


void SoundChangeVolume() {
#ifdef WIN32
  if(SoundBuf)
  {
    SoundBuf->Stop();
    SoundBuf->SetVolume(MaxVolume);
    SoundBuf->Play(0,0,DSBPLAY_LOOPING);
  }
#endif

#ifdef UNIX // those functions do nothing
  PA_ONLY( if (UseSound==XS_PA) PA_ChangeVolume(); )
  RT_ONLY( if (UseSound==XS_RT) Rt_ChangeVolume(); )
#endif
}


HRESULT SoundLockBuffer(DWORD Start,DWORD Len,LPVOID *lpDatAdr1,
            DWORD *lpLockLength1,LPVOID *lpDatAdr2,DWORD *lpLockLength2) {

#ifdef WIN32
  if(DSOpen==0) 
    return DSERR_GENERIC;
  HRESULT Ret=SoundBuf->Lock(Start,Len,lpDatAdr1,lpLockLength1,lpDatAdr2,
    lpLockLength2,0);
  if(Ret==DSERR_BUFFERLOST)
  {
    log_write("SOUND: Restoring sound buffer");
    Ret=SoundBuf->Restore();
    if(Ret==DS_OK)
    {
      Ret=SoundBuf->Play(0,0,DSBPLAY_LOOPING);
      if(Ret==DS_OK)
        Ret=SoundBuf->Lock(Start,Len,lpDatAdr1,lpLockLength1,lpDatAdr2,
          lpLockLength2,0);
    }
  }
  return Ret;
#endif

#ifdef UNIX
  if (SoundActive()==0) return DSERR_GENERIC;

  DWORD buflen_bytes=X_SOUND_BUF_LEN_BYTES;
  *lpDatAdr1=x_sound_buf+Start;
  *lpLockLength1=Len;
  if (Start+Len>buflen_bytes){
    *lpDatAdr2=x_sound_buf;
    *lpLockLength2=(Start+Len)-buflen_bytes;
    *lpLockLength1-=*lpLockLength2;
  }else{
    *lpDatAdr2=NULL;
    *lpLockLength2=0;
  }
//  log_write(Str("PortAudio: Writing - ")+Start+" to "+(Start+*lpLockLength1));
//  if (*lpLockLength2) log_write(Str("PortAudio: Writing - ")+0+" to "+*lpLockLength2);
  return DS_OK;
#endif
}


void SoundUnlock(LPVOID DatAdr1,DWORD LockLength1,LPVOID DatAdr2,
                    DWORD LockLength2) {
#ifdef WIN32
  if(SoundBuf) 
    SoundBuf->Unlock(DatAdr1,LockLength1,DatAdr2,LockLength2);
#endif
}


HRESULT SoundError(char *ErrorText,HRESULT DErr) {
  SoundRelease();

#ifdef WIN32
  Str Err=SoundLogError(Str(ErrorText)+"\n\n",DErr);
  Err+=Str("\n\n")+T("Steem will not be able to output any sound until you restart the program. Would you like to permanently stop Steem trying to use DirectSound at startup?");
#if !defined(SSE_SOUND_NO_NOSOUND_OPTION)
#ifndef ONEGAME
  int Ret=MessageBox(NULL,Err,T("Steem Engine DirectSound Error"),
    MB_YESNO|MB_ICONEXCLAMATION|MB_SETFOREGROUND|MB_TASKMODAL|MB_TOPMOST);
  if(Ret==IDYES) WriteCSFStr("Options","NoDirectSound","1",globalINIFile);
  OptionBox.UpdateForDSError();
#endif
#endif
#endif//WIN32

  return DErr;
}


void SoundCreateDynamicBuffer() {
/*  Create dynamic buffer for PSG.
    It will be deleted/recreated at each sound option change that provokes a
    call to this function.
    It will be deleted when Steem closes.
    We do so because of higher sample rates, not to reserve too much memory
    if SR isn't high.
*/
  DWORD samples_per_vbl=((sound_freq/50)+1)*SCREENS_PER_SOUND_VBL;
  if(psg_channels_buf_len+16!=samples_per_vbl)
  {
    if(psg_channels_buf!=NULL)
      delete[] psg_channels_buf;
    psg_channels_buf=new int[samples_per_vbl+16+PSG_WRITE_EXTRA];
    //ASSERT(psg_channels_buf);
    ZeroMemory(psg_channels_buf,(samples_per_vbl+16+PSG_WRITE_EXTRA)
      *sizeof(int));
    psg_channels_buf_len=samples_per_vbl;
    TRACE_INIT("buffer for psg %dHz = %p, %d x32bit =%d bytes\n",psg_channels_buf,
      sound_freq,psg_channels_buf_len,psg_channels_buf_len*sizeof(int));
  }
  samples_per_vbl=((sound_freq/50)+1)*SCREENS_PER_SOUND_VBL*2;
  if(ste_sound_channel_buf_len+16!=samples_per_vbl)
  {
    if(ste_sound_channel_buf!=NULL)
      delete[] ste_sound_channel_buf;
    ste_sound_channel_buf=new WORD[samples_per_vbl+16];
    //ASSERT(ste_sound_channel_buf);
    ZeroMemory(ste_sound_channel_buf,(samples_per_vbl+16)*sizeof(WORD));
    ste_sound_channel_buf_len=samples_per_vbl;
    TRACE_INIT("buffer for dma %dHz = %p, %d x16bit =%d bytes\n",ste_sound_channel_buf,
      sound_freq,ste_sound_channel_buf_len,ste_sound_channel_buf_len*sizeof(WORD));
  }
#if defined(SSE_YM2149_LL)
  if(sound_freq)
    Psg.ym2149_cycles_per_sample=((float)CpuNormalHz/4)/(float)sound_freq;
  if(!Psg.AntiAlias && !DSP_DISABLED && 
    SSEOptions.low_pass_frequency<ym_low_pass_max)
  {
    if((Psg.AntiAlias=new Filter(LPF,51,250.0,
      (double)SSEOptions.low_pass_frequency/1000))!=NULL)
    {
      if(int error=Psg.AntiAlias->get_error_flag())
      {
        TRACE_INIT("AntiAlias error %d\n",error);
        delete Psg.AntiAlias;
        Psg.AntiAlias=NULL;
      }
    }
  }
#endif
  
}

#ifdef WIN32

BOOL CALLBACK DSEnumProc(LPGUID Guid,LPCSTR Desc,LPCSTR /* Mod */,LPVOID) {
  DBG_LOG(Str("SOUND: Found device ")+Desc);
  //TRACE("add %s %d\n",Desc,Guid);
  DSDriverModuleList.Add((char*)Desc,(LONG_PTR)Guid);
  return TRUE;
}


HRESULT DSReleaseAllBuffers(HRESULT Ret) {
  DSOpen=0;
  if(SoundBuf && sound_write_primary==0)
  {
    SoundBuf->Stop();
    SoundBuf->Release();
  }
#if defined(SSE_DRIVE_SOUND)
  FloppyDrive[0].Sound_ReleaseBuffers();
  FloppyDrive[1].Sound_ReleaseBuffers();
#endif
  if(PrimaryBuf)
  {
    if(sound_write_primary) 
      PrimaryBuf->Stop();
    PrimaryBuf->Release();
  }
  SoundBuf=NULL;
  PrimaryBuf=NULL;
  //DSOpen=0;
  if(sound_write_primary && DSObj) 
    DSObj->SetCooperativeLevel(StemWin,DSSCL_PRIORITY);
  return Ret;
}


HRESULT DSGetPrimaryBuffer() { // called by SoundStartBuffer()
  HRESULT Ret;
  DSBUFFERDESC dsbd;
  DSReleaseAllBuffers();
  DSOpen=true; // This will be cleared if any error occurs
  ZeroMemory(&dsbd,sizeof(DSBUFFERDESC));
  dsbd.dwSize=sizeof(DSBUFFERDESC);
  dsbd.dwFlags=DSBCAPS_PRIMARYBUFFER;
  if(sound_write_primary) 
    dsbd.dwFlags|=DSBCAPS_GETCURRENTPOSITION2;
  Ret=DSObj->CreateSoundBuffer(&dsbd,&PrimaryBuf,NULL);
  if(Ret!=DS_OK)
  {
    LOG_ONLY(SoundLogError("SOUND: CreateSoundBuffer for Primary Failed\r\n\r\n",Ret); )
    return DSReleaseAllBuffers(Ret);
  }
  PrimaryFormat.wFormatTag=WAVE_FORMAT_PCM;
  PrimaryFormat.nChannels=sound_num_channels;
  PrimaryFormat.nSamplesPerSec=12000;
  PrimaryFormat.wBitsPerSample=sound_num_bits;
  PrimaryFormat.nBlockAlign=WORD(sound_bytes_per_sample);
  PrimaryFormat.nAvgBytesPerSec=PrimaryFormat.nSamplesPerSec
    *PrimaryFormat.nBlockAlign;
  PrimaryFormat.cbSize=0;
  PrimaryBuf->SetFormat(&PrimaryFormat);
  DS_GetFormat_Wrong=0;
  int desired_freq=sound_chosen_freq;
  for(;;)
  {
    while(desired_freq>=20000)
    {
      if(desired_freq==sound_comline_freq)
      { // force if comline freq
        DS_SetFormat_freq=sound_comline_freq;
        PrimaryFormat.nSamplesPerSec=DS_SetFormat_freq;
        PrimaryFormat.nAvgBytesPerSec=PrimaryFormat.nSamplesPerSec
          *PrimaryFormat.nBlockAlign;
        Ret=PrimaryBuf->SetFormat(&PrimaryFormat);
        DBG_LOG(EasyStr("SOUND: SetFormat to ")+DS_SetFormat_freq+"Hz, it "+LPSTR(Ret==DS_OK?"succeeded.":"failed."));
        if(Ret==DS_OK)
        {
          sound_freq=sound_comline_freq;
          DS_GetFormat_Wrong=true;
          break;
        }
      }
      DS_SetFormat_freq=MIN((DWORD)(desired_freq),
        SoundCaps.dwMaxSecondarySampleRate);
      PrimaryFormat.nSamplesPerSec=DS_SetFormat_freq;
#if defined(SSE_DEBUG)
      if(DS_SetFormat_freq<(DWORD)sound_chosen_freq)
        TRACE_INIT("max SR %d\n",DS_SetFormat_freq);
#endif
      PrimaryFormat.nAvgBytesPerSec=PrimaryFormat.nSamplesPerSec
        *PrimaryFormat.nBlockAlign;
      Ret=PrimaryBuf->SetFormat(&PrimaryFormat);
      DBG_LOG(EasyStr("SOUND: SetFormat to ")+DS_SetFormat_freq+"Hz, it "+LPSTR(Ret==DS_OK?"succeeded.":"failed."));
      if(Ret==DS_OK) 
        break;
      DS_SetFormat_freq=MIN((DWORD)((desired_freq/1000)*1000),
        SoundCaps.dwMaxSecondarySampleRate);
      for(;;)
      {
        PrimaryFormat.nSamplesPerSec=DS_SetFormat_freq;
        PrimaryFormat.nAvgBytesPerSec=PrimaryFormat.nSamplesPerSec
          *PrimaryFormat.nBlockAlign;
        if((Ret=PrimaryBuf->SetFormat(&PrimaryFormat))==DS_OK) 
          break;
        DBG_LOG(EasyStr("SOUND: Couldn't SetFormat to ")+DS_SetFormat_freq+"Hz");
        DS_SetFormat_freq-=500;
        // Fail if less than 4/5th of the desired frequency
        if(DS_SetFormat_freq<(DWORD)(desired_freq-(desired_freq/5+500))) 
          break;
      }
      if(Ret==DS_OK) 
        break;
      desired_freq*=4;
      desired_freq/=5;
    }
    //GetFormat
    Ret=PrimaryBuf->GetFormat(&PrimaryFormat,sizeof(PrimaryFormat),NULL);
    if(Ret!=DS_OK)
    {
      sound_freq=DS_SetFormat_freq;
      DS_GetFormat_Wrong=true;
      DBG_LOG(EasyStr("SOUND: GetFormat for primary sound buffer failed, assuming ")+sound_freq+"Hz");
    }
    else
    {
      sound_freq=PrimaryFormat.nSamplesPerSec;
      DBG_LOG(EasyStr("SOUND: GetFormat for primary sound buffer returned ")+sound_freq+"Hz");
    }
    if(DS_GetFormat_Wrong) 
      break;
    if(DWORD(sound_freq)>=(DS_SetFormat_freq-2500)&&(DWORD)(sound_freq)
      <=(DS_SetFormat_freq+2500))
      break;
    else if(desired_freq<20000)
    {
      DBG_LOG("   Sound card is a dirty liar! Ignoring what it says and restarting.");
      DS_GetFormat_Wrong=true;
      desired_freq=sound_chosen_freq;
    }
    else
    {
      DBG_LOG("   SetFormat failed or sound card is a dirty liar! Trying again.");
      desired_freq*=4;
      desired_freq/=5;
    }
  }
  if(sound_write_primary)
  {
    DSBCAPS caps={sizeof(DSBCAPS)};
    Ret=PrimaryBuf->GetCaps(&caps);
    if(Ret!=DS_OK)
    {
      LOG_ONLY(SoundLogError("SOUND: GetCaps for Primary Failed\r\n\r\n",Ret); )
        return DSReleaseAllBuffers(Ret);
    }
    // sound_buffer_length is in number of samples
    sound_buffer_length=caps.dwBufferBytes/sound_bytes_per_sample;
    if(DS_GetFormat_Wrong) 
      sound_freq=DS_SetFormat_freq;
  }
  SoundCreateDynamicBuffer();
  //ASSERT(SoundBuf);
  //if(!SoundBuf) DSOpen=false;
  return DS_OK;
}


HRESULT DSCreateSoundBuf() { // called by SoundStartBuffer()
  if(sound_write_primary)
  {
    SoundBuf=PrimaryBuf;
    return DS_OK;
  }
  if(SoundBuf)
  {
    SoundBuf->Stop();SoundBuf->Release();SoundBuf=NULL;
    DSOpen=0;
  }
  sound_buffer_length=DEFAULT_SOUND_BUFFER_LENGTH;
  HRESULT Ret;
  DSBUFFERDESC dsbd;
  WAVEFORMATEX wfx;
  wfx.wFormatTag=WAVE_FORMAT_PCM;
  wfx.nChannels=sound_num_channels;
  wfx.nSamplesPerSec=DWORD(DS_GetFormat_Wrong?DS_SetFormat_freq:sound_freq);
  wfx.wBitsPerSample=sound_num_bits;
  wfx.nBlockAlign=WORD(sound_bytes_per_sample);
  wfx.nAvgBytesPerSec=wfx.nSamplesPerSec*wfx.nBlockAlign;
  wfx.cbSize=0;
  ZeroMemory(&dsbd,sizeof(DSBUFFERDESC));
  dsbd.dwSize=sizeof(DSBUFFERDESC);
  dsbd.dwFlags=DSBCAPS_CTRLVOLUME|DSBCAPS_GLOBALFOCUS
    |DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_STICKYFOCUS;
  dsbd.dwBufferBytes=sound_buffer_length * sound_bytes_per_sample;
  dsbd.lpwfxFormat=&wfx;
  Ret=DSObj->CreateSoundBuffer(&dsbd,&SoundBuf,NULL);
  if(Ret!=DS_OK)
  {
    if(DS_GetFormat_Wrong)
    {
      wfx.nSamplesPerSec=sound_freq;
      wfx.nAvgBytesPerSec=wfx.nSamplesPerSec;
      ZeroMemory(&dsbd,sizeof(DSBUFFERDESC));
      dsbd.dwSize=sizeof(DSBUFFERDESC);
      dsbd.dwFlags=DSBCAPS_CTRLVOLUME|
        DSBCAPS_GLOBALFOCUS|DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_STICKYFOCUS;
      dsbd.dwBufferBytes=sound_buffer_length * sound_bytes_per_sample;
      dsbd.lpwfxFormat=&wfx;
      Ret=DSObj->CreateSoundBuffer(&dsbd,&SoundBuf,NULL);
    }
    if(Ret!=DS_OK)
    {
      LOG_ONLY(SoundLogError("SOUND: CreateSoundBuffer for Secondry Buffer Failed\r\n\r\n",Ret); )
      return DSReleaseAllBuffers(Ret);
    }
  }
  else
  {
    // Successfully created a buffer at DS_SetFormat_freq so make output match it
    if(DS_GetFormat_Wrong) 
      sound_freq=DS_SetFormat_freq;
#if defined(SSE_DRIVE_SOUND)
    if(OPTION_DRIVE_SOUND)
    {
      FloppyDrive[0].Sound_LoadSamples(DSObj,&dsbd,&wfx);
      FloppyDrive[1].Sound_LoadSamples(DSObj,&dsbd,&wfx);
    }
#endif
  }
  DSBCAPS caps={sizeof(DSBCAPS)};
  if(SoundBuf->GetCaps(&caps)==DS_OK) 
    sound_buffer_length=caps.dwBufferBytes/sound_bytes_per_sample;
  DBG_LOG(EasyStr("SOUND: Created secondry sound buffer at ")+wfx.nSamplesPerSec+"Hz");
  return DS_OK;
}


void CALLBACK DSStopBufferTimerProc(HWND,UINT,UINT_PTR,DWORD) {
  if(DSStopBufferTimerID)
  {
    if(DSOpen==0&&SoundBuf) 
      DSReleaseAllBuffers();
    KillTimer(NULL,DSStopBufferTimerID);
    DSStopBufferTimerID=0;
  }
}


Str SoundLogError(Str Text,HRESULT DErr) {
  switch(DErr) {
  case DSERR_ALLOCATED:
    Text+="The request failed because resources, such as a priority level,\nwere already in use by another caller.";
    break;
  case DSERR_ALREADYINITIALIZED:
    Text+="The object is already initialized.";
    break;
  case DSERR_BADFORMAT:
    Text+="The specified wave format is not supported.";
    break;
  case DSERR_BUFFERLOST:
    Text+="The buffer memory has been lost and must be restored.";
    break;
  case DSERR_CONTROLUNAVAIL:
    Text+="The buffer control (volume, pan, and so on) requested\nby the caller is not available.";
    break;
  case DSERR_GENERIC:
    Text+="An undetermined error occurred inside the DirectSound subsystem.";
    break;
  case DSERR_INVALIDCALL:
    Text+="This function is not valid for the current state of this object.";
    break;
  case DSERR_INVALIDPARAM:
    Text+="An invalid parameter was passed to the returning function.";
    break;
  case DSERR_NOAGGREGATION:
    Text+="The object does not support aggregation.";
    break;
  case DSERR_NODRIVER:
    Text+="No sound driver is available for use.";
    break;
  case DSERR_NOINTERFACE:
    Text+="The requested COM interface is not available.";
    break;
  case DSERR_OTHERAPPHASPRIO:
    Text+="Another application has a higher priority level,\npreventing this call from succeeding";
    break;
  case DSERR_OUTOFMEMORY:
    Text+="The DirectSound subsystem could not allocate sufficient\nmemory to complete the caller's request.";
    break;
  case DSERR_PRIOLEVELNEEDED:
    Text+="The caller does not have the priority level required\nfor the function to succeed.";
    break;
  case DSERR_UNSUPPORTED:
    Text+="The function called is not supported at this time.";
    break;
  }
  log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  log_write(Text);
  log_write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  return Text;
}

#endif//WIN32


#ifdef UNIX

void XSoundInitBuffer(int flatlevel1,int flatlevel2) {
  BYTE *p=x_sound_buf;
  for(int i=0;i<sound_buffer_length;i++)
  {
    if(sound_num_bits==8)
    {
      *(p++)=BYTE(flatlevel1);
      if(sound_num_channels==2)
        *(p++)=BYTE(flatlevel2);
    }
    else
    {
      *LPWORD(p)=WORD(char(flatlevel1) << 8);
      p+=2;
      if(sound_num_channels==2)
      {
        *LPWORD(p)=WORD(char(flatlevel2) << 8);
         p+=2;
      }
    }
  }
}


#if !defined(SSE_NO_INTERNAL_SPEAKER)

void internal_speaker_sound_by_period(int UNIX_ONLY( counter ))
{
#ifdef LINUX
	if (console_device!=-1) ioctl(console_device,KIOCSOUND,counter);
#endif
}

#endif

#endif//UNIX


#ifdef SHOW_WAVEFORM
  #define WAVEFORM_SET_VAL(v) (val=(v))
  #define WAVEFORM_ONLY(x) x
#else
  #define WAVEFORM_SET_VAL(v) v
  #define WAVEFORM_ONLY(x)
#endif

#ifdef WRITE_ONLY_SINE_WAVE
#define SINE_ONLY(s) s
//todo, one day...
#define WRITE_SOUND_LOOP(Alter_V)         \
	          while (c>0){                                                  \
                *(p++)=WAVEFORM_SET_VAL(BYTE(sin((double)t*(M_PI/64))*120+128)); \
                t++;                                                       \
                WAVEFORM_ONLY( temp_waveform_display[((int)(source_p-psg_channels_buf)+psg_time_of_last_vbl_for_writing) % MAX_temp_waveform_display_counter]=(BYTE)val; ) \
    	          *(source_p++)=VOLTAGE_FP(VOLTAGE_ZERO_LEVEL);                 \
                c--;    \
	          }
#else
#define SINE_ONLY(s)
#endif

#ifdef UNIX
#define min(a,b) ((a)>(b) ? (b):(a))
#endif


/*  The function is called at VBL. The sounds have already been computed.
    The function adds an optional low-pass filter to PSG sound and adds
    PSG and DMA sound together. (Ground for improvement).
    It also applies Microwire filters.
*/

void WriteSoundLoop(int Alter_V, int* Out_P,int Size,int& c,int &val,
  int &v,int &dv,int **source_p,WORD**lp_ste_sound_channel,
  WORD**lp_max_ste_sound_channel) {
/*  Try to avoid clicks when a program aggressively changes microwire volume
    (Sea of Colour).
    Now it won't work if a program does a lot of quick changes for effect.
    This is enabled with option Hacks, else we use dsp.
*/
  if(!IS_STE||!OPTION_MICROWIRE||!OPTION_HACKS)
    ;
  else if(Microwire.old_top_val_l<Microwire.top_val_l)
    Microwire.old_top_val_l++;
  else if(Microwire.old_top_val_l>Microwire.top_val_l)
    Microwire.old_top_val_l--;
  if(Microwire.old_top_val_r<Microwire.top_val_r)
    Microwire.old_top_val_r++;
  else if(Microwire.old_top_val_r>Microwire.top_val_r)
    Microwire.old_top_val_r--;
  // check size once
  if(Size==sizeof(BYTE)) //8bit
  {
    while(c>0)
    {       
      AlterV(Alter_V,v,dv,*source_p);
      //LEFT-8bit
      val=v+(**lp_ste_sound_channel);                           
      if(IS_STE&&OPTION_MICROWIRE)
      { 
        Microwire.Process(0,val);
        if(OPTION_HACKS && sound_num_channels==2 // not for monosound: Rebirth
          && (Microwire.top_val_l!=128
          ||Microwire.old_top_val_l!=Microwire.top_val_l))
        {
          val*=Microwire.old_top_val_l;
          val/=128;
        }
      }
      if(val<VOLTAGE_FP(0))
        val=VOLTAGE_FP(0); 
      else if(val>VOLTAGE_FP(255))
        val=VOLTAGE_FP(255); 
      *(BYTE*)*(BYTE**)Out_P=(BYTE)((val&0x00FF00)>>8);
      (*(BYTE**)Out_P)++;
      // stereo: do the same for right channel
      if(sound_num_channels==2) 
      {   
        //RIGHT-8bit
        val=v+(*(*lp_ste_sound_channel+1)); 
        if(IS_STE&&OPTION_MICROWIRE)
        {
          Microwire.Process(1,val);
          if(OPTION_HACKS && (Microwire.top_val_r!=128 
            || Microwire.old_top_val_r!=Microwire.top_val_r))
          {
            val*=Microwire.old_top_val_r;
            val/=128;
          }
        }
        if(val<VOLTAGE_FP(0))
          val=VOLTAGE_FP(0); 
        else if(val>VOLTAGE_FP(255))
          val=VOLTAGE_FP(255); 
        *(BYTE*)*(BYTE**)Out_P=(BYTE)((val&0x00FF00)>>8);
        (* (BYTE**)Out_P )++;
      }//right channel 
      WAVEFORM_ONLY(temp_waveform_display[((int)(*source_p-psg_channels_buf)+psg_time_of_last_vbl_for_writing) % MAX_temp_waveform_display_counter]=WORD_B_1(&val)); 
      *(*source_p)++=0;//VOLTAGE_FP(VOLTAGE_ZERO_LEVEL);
      if(*lp_ste_sound_channel<*lp_max_ste_sound_channel) 
        *lp_ste_sound_channel+=2;
      c--;                                                          
    }//wend
  }
  else //16bit
  { // choose our loop according to config
    if(IS_STE&&OPTION_MICROWIRE) // most complex
    {
      const int DMA_SOUND_MULTIPLIER=64; 
      while(c>0)
      { 
        AlterV(Alter_V,v,dv,*source_p);
        //LEFT-16bit
        val=v; //inefficient?
        char dma_sample_l=(char)**lp_ste_sound_channel; 
        val+=dma_sample_l*DMA_SOUND_MULTIPLIER;
        Microwire.Process(0,val);
        if(OPTION_HACKS&&((Microwire.top_val_l!=128||Microwire.old_top_val_l
          !=Microwire.top_val_l)&&sound_num_channels==2))
        {
            val*=Microwire.old_top_val_l;
            val/=128;
        }
        if(val>32767)
          val=32767;
        *(WORD*)*(WORD**)Out_P=((WORD)val);
        (*(WORD**)Out_P)++;
        // stereo: do the same for right channel
        if(sound_num_channels==2)
        { //RIGHT-16bit
          val=v;
          char dma_sample_r=(char)*(*lp_ste_sound_channel+1);
          val+=dma_sample_r*DMA_SOUND_MULTIPLIER;
          Microwire.Process(1,val);
          if(OPTION_HACKS&&(Microwire.top_val_r<128||Microwire.old_top_val_r
            !=Microwire.top_val_r))
          {
            val*=Microwire.old_top_val_r;
            val/=128;
          }
          if(val>32767)
            val=32767;
          *(WORD*)*(WORD**)Out_P=((WORD)val);
          (*(WORD**)Out_P)++;
        }
        WAVEFORM_ONLY(temp_waveform_display[((int)(*source_p-psg_channels_buf)+psg_time_of_last_vbl_for_writing) % MAX_temp_waveform_display_counter]=WORD_B_1(&val)); 
        *(*source_p)++=0;//VOLTAGE_FP(VOLTAGE_ZERO_LEVEL);
        if(*lp_ste_sound_channel<*lp_max_ste_sound_channel) 
          *lp_ste_sound_channel+=2;
        c--;                                                          
      }//wend
    }
    else if(IS_STE)
    {
      const int DMA_SOUND_MULTIPLIER=32;
      while(c>0)
      { 
        AlterV(Alter_V,v,dv,*source_p);
        //LEFT-16bit
        val=v; //inefficient?
        char dma_sample_l=(char)**lp_ste_sound_channel; 
        val+=dma_sample_l*DMA_SOUND_MULTIPLIER;
        if(val>32767)
          val=32767;
        *(WORD*)*(WORD**)Out_P=((WORD)val);
        (*(WORD**)Out_P)++;
        // stereo: do the same for right channel
        if(sound_num_channels==2)
        { //RIGHT-16bit
          val=v;
          char dma_sample_r=(char)*(*lp_ste_sound_channel+1);
          val+=dma_sample_r*DMA_SOUND_MULTIPLIER;
          if(val>32767)
            val=32767;
          *(WORD*)*(WORD**)Out_P=((WORD)val);
          (*(WORD**)Out_P)++;
        }
        WAVEFORM_ONLY(temp_waveform_display[((int)(*source_p-psg_channels_buf)+psg_time_of_last_vbl_for_writing) % MAX_temp_waveform_display_counter]=WORD_B_1(&val)); 
        *(*source_p)++=0;//VOLTAGE_FP(VOLTAGE_ZERO_LEVEL);
        if(*lp_ste_sound_channel<*lp_max_ste_sound_channel) 
          *lp_ste_sound_channel+=2;
        c--;                                                          
      }//wend
    }
#if defined(SSE_SOUND_CARTRIDGE)
    else if(SSEConfig.mv16||SSEConfig.mr16)
    { // B.A.T's MV16 cartridge, Action Replay 16, no PSG
      //int snd_multiplier=(SSEConfig.mr16)?1:16; // see POV129
      while(c>0)
      { 
        //LEFT-16bit
        WORD dma_sample_l=**lp_ste_sound_channel; 
        *(WORD*)*(WORD**)Out_P=dma_sample_l;
        (*(WORD**)Out_P)++;
        // stereo: do the same for right channel
        if(sound_num_channels==2)
        { //RIGHT-16bit
          WORD dma_sample_r=*(*lp_ste_sound_channel+1);
          *(WORD*)*(WORD**)Out_P=dma_sample_r;
          (*(WORD**)Out_P)++;
        }
        if(*lp_ste_sound_channel<*lp_max_ste_sound_channel) 
          *lp_ste_sound_channel+=2;
        c--;                                                          
      }//wend
    }
    else if(DONGLE_ID==TDongle::PROSOUND) // Pro Sound Designer + psg
    {
      int snd_multiplier=8;
      while(c>0)
      { 
        AlterV(Alter_V,v,dv,*source_p);
        val=v;
        WORD dma_sample_l=**lp_ste_sound_channel; 
        val+=dma_sample_l*snd_multiplier;
        //LEFT-16bit
        if(val>32767)
          val=32767;
        *(WORD*)*(WORD**)Out_P=((WORD)val);
        (*(WORD**)Out_P)++;
        // stereo: do the same for right channel
        if(sound_num_channels==2)
        { //RIGHT-16bit
          val=v;
          WORD dma_sample_r=*(*lp_ste_sound_channel+1);
          val+=dma_sample_r*snd_multiplier;
          *(WORD*)*(WORD**)Out_P=((WORD)val);
          (*(WORD**)Out_P)++;
        }
        WAVEFORM_ONLY(temp_waveform_display[((int)(*source_p-psg_channels_buf)+psg_time_of_last_vbl_for_writing) % MAX_temp_waveform_display_counter]=WORD_B_1(&val)); 
        *(*source_p)++=0;//VOLTAGE_FP(VOLTAGE_ZERO_LEVEL);
        if(*lp_ste_sound_channel<*lp_max_ste_sound_channel) 
          *lp_ste_sound_channel+=2;
        c--;                                                          
      }//wend
    }
#endif
    else // STF PSG-only
    {
      while(c>0)
      { 
        AlterV(Alter_V,v,dv,*source_p);
        //LEFT-16bit
        if(v>32767)
          v=32767;
        *(WORD*)*(WORD**)Out_P=((WORD)v);
        (*(WORD**)Out_P)++;
        // stereo: do the same for right channel
        if(sound_num_channels==2) // guess this test is assumed by the processor?
        { //RIGHT-16bit
          *(WORD*)*(WORD**)Out_P=((WORD)v);
          (*(WORD**)Out_P)++;
        }
        WAVEFORM_ONLY(temp_waveform_display[((int)(*source_p-psg_channels_buf)+psg_time_of_last_vbl_for_writing) % MAX_temp_waveform_display_counter]=WORD_B_1(&val)); 
        *(*source_p)++=0;//VOLTAGE_FP(VOLTAGE_ZERO_LEVEL);
        c--;                                                          
      }//wend
    }

  }    
}

#define WRITE_SOUND_LOOP(Alter_V,Out_P,Size,GetSize) \
  WriteSoundLoop(Alter_V,(int*)&Out_P,sizeof(Size),c,val,v,dv,&source_p,\
  &lp_ste_sound_channel,&lp_max_ste_sound_channel)
#define WRITE_TO_WAV_FILE_B 1 
#define WRITE_TO_WAV_FILE_W 2 

void SoundRecord(int Alter_V, int Write,int& c,int &val,
  int &v,int &dv,int **source_p,WORD**lp_ste_sound_channel,
  WORD**lp_max_ste_sound_channel,FILE* wav_file) {
/*  Try to avoid clicks when a program aggressively changes microwire volume
    (Sea of Colour).
    Now it won't work if a program does a lot of quick changes for effect.
*/
  if(!IS_STE||!OPTION_MICROWIRE||!OPTION_HACKS)
    ;
  else if(Microwire.old_top_val_l<Microwire.top_val_l)
    Microwire.old_top_val_l++;
  else if(Microwire.old_top_val_l>Microwire.top_val_l)
    Microwire.old_top_val_l--;
  if(Microwire.old_top_val_r<Microwire.top_val_r)
    Microwire.old_top_val_r++;
  else if(Microwire.old_top_val_r>Microwire.top_val_r)
    Microwire.old_top_val_r--;
  while(c>0)
  {       
    AlterV(Alter_V,v,dv,*source_p);
    val=v;
    if(RENDER_SIGNED_SAMPLES)
    {
      WORD dma_sample_l=**lp_ste_sound_channel; 
#if defined(SSE_SOUND_CARTRIDGE)
      val+=(SSEConfig.mv16||SSEConfig.mr16||(DONGLE_ID==TDongle::PROSOUND))
        ? (dma_sample_l) : ((char)dma_sample_l*32);
#else
      val+=dma_sample_l*DMA_SOUND_MULTIPLIER;
#endif
    }
    else
      val+= (**lp_ste_sound_channel);  
    if(IS_STE&&OPTION_MICROWIRE)
    {
      Microwire.Process(0,val);
      if(OPTION_HACKS&&((Microwire.top_val_l!=128||Microwire.old_top_val_l
        !=Microwire.top_val_l)&&sound_num_channels==2))
      {
        val*=Microwire.old_top_val_l; // changed in live sound loop
        val/=128;
      }
    }
    if(val>32767)
      val=32767;
    if(Write==WRITE_TO_WAV_FILE_B) 
      fputc(BYTE(WORD_B_1(&(val))),wav_file);
    else 
    {
      fputc(LOBYTE(val),wav_file);
      fputc(HIBYTE(val),wav_file);
    }
    if(sound_num_channels==2)
    { // RIGHT CHANNEL
      val=v; // restore val! 
      if(RENDER_SIGNED_SAMPLES)
      {
        WORD dma_sample_r=*(*lp_ste_sound_channel+1); 
#if defined(SSE_SOUND_CARTRIDGE)
        val+=(SSEConfig.mv16||SSEConfig.mr16||(DONGLE_ID==TDongle::PROSOUND))
          ? dma_sample_r : ((char)dma_sample_r*32);
#else
        val+=dma_sample_r*DMA_SOUND_MULTIPLIER;
#endif
      }
      else
        val+= (*(*lp_ste_sound_channel+1)); 
      if(IS_STE&&OPTION_MICROWIRE)
      {
        Microwire.Process(1,val);
        if(OPTION_HACKS&&(Microwire.top_val_r!=128||Microwire.old_top_val_r
          !=Microwire.top_val_r))
        {
          val*=Microwire.old_top_val_r;
          val/=128;
        }
      }
      if(val>32767)
        val=32767;
      if(Write==WRITE_TO_WAV_FILE_B) 
        fputc(BYTE(WORD_B_1(&(val))),wav_file);
      else 
      {
        fputc(LOBYTE(val),wav_file);
        fputc(HIBYTE(val),wav_file);
      }
    }//right
    (*source_p)++; // don't zero! (or mute when recording)
    SINE_ONLY( t++ );
    if(*lp_ste_sound_channel<*lp_max_ste_sound_channel) 
      *lp_ste_sound_channel+=2;
    c--;   
  }//wend
}

#define SOUND_RECORD(Alter_V,WRITE) \
SoundRecord(Alter_V,WRITE,c,val,v,dv,&source_p,&lp_ste_sound_channel,\
&lp_max_ste_sound_channel,Wav_file)


void sound_record_to_wav(int c,DWORD SINE_ONLY(t),bool chipmode,int *source_p) {
  if(timer<sound_record_start_time) 
    return;
  int v=psg_voltage,dv=psg_dv; //restore from last time
  WORD *lp_ste_sound_channel=ste_sound_channel_buf;
  WORD *lp_max_ste_sound_channel=ste_sound_channel_buf+ste_sound_channel_buf_idx;
  int val;
  if(source_p==NULL)
    ;
  else if(sound_num_bits==8)
  {
    if(chipmode)
    {
      if(sound_low_quality==0)
        SOUND_RECORD(CALC_V_CHIP,WRITE_TO_WAV_FILE_B);
      else
        SOUND_RECORD(CALC_V_CHIP_25KHZ,WRITE_TO_WAV_FILE_B);
    }
    else
      SOUND_RECORD(CALC_V_EMU,WRITE_TO_WAV_FILE_B);
  }
  else
  {
    if(chipmode)
    {
      if(sound_low_quality==0)
        SOUND_RECORD(CALC_V_CHIP,WRITE_TO_WAV_FILE_W);
      else
        SOUND_RECORD(CALC_V_CHIP_25KHZ,WRITE_TO_WAV_FILE_W);
    }
    else
      SOUND_RECORD(CALC_V_EMU,WRITE_TO_WAV_FILE_W);
  }
}


HRESULT Sound_VBL() {
#if SCREENS_PER_SOUND_VBL != 1 //SS it is 1
  static int screens_countdown=SCREENS_PER_SOUND_VBL;
  screens_countdown--;if(screens_countdown>0) return DD_OK;
  screens_countdown=SCREENS_PER_SOUND_VBL;
  cpu_time_of_last_sound_vbl=ABSOLUTE_CPU_TIME;
#endif
#if !defined(SSE_NO_INTERNAL_SPEAKER)
  if(sound_internal_speaker)
  {
    static double op=0;
    int abc,chan=-1,max_vol=0,vol;
    // Find loudest channel
    for(abc=0;abc<3;abc++)
    {
      if((psg_reg[PSGR_MIXER]&(1<<abc))==0)
      { // Channel enabled in mixer
        vol=(psg_reg[PSGR_AMPLITUDE_A+abc]&15);
        if(vol>max_vol)
        {
          chan=abc;
          max_vol=vol;
        }
      }
    }
    if(chan==-1)
    { //no sound
      internal_speaker_sound_by_period(0);
      op=0;
    }
    else
    {
      double p=((((int)psg_reg[chan*2+1]&0xf)<<8)+psg_reg[chan*2]);
      p*=(1193181.0/125000.0);
      if(op!=p)
      {
        op=p;
        internal_speaker_sound_by_period((int)p);
      }
    }
  }
#endif
  // This just clears up some clicks when Sound_VBL is called very soon after Sound_Start
  if(sound_first_vbl)
  {
    sound_first_vbl=false;
    return DS_OK;
  }
  if(OPTION_SOUNDMUTE || (MuteWhenInactive&&bAppActive==false))
    return DS_OK;
  if(UseSound==0)  
    return DSERR_GENERIC;  // Not initialised
#ifdef WIN32    
  if(SoundBuf==NULL)  
    return DSERR_GENERIC;  // no buffer
#endif    
  if(SoundActive()==0) 
    return DS_OK;        // Not started
  //ASSERT(SoundBuf);
  DBG_LOG("");
  DBG_LOG("SOUND: Start of Sound_VBL");
  void *DatAdr[2]={NULL,NULL};
  DWORD LockLength[2]={0,0};
  DWORD s_time,write_time_1,write_time_2;
  HRESULT Ret;
  int *source_p;
  DWORD n_samples_per_vbl=(sound_freq*SCREENS_PER_SOUND_VBL)/Glue.video_freq;
  DBG_LOG(EasyStr("SOUND: Calculating time; psg_time_of_start_of_buffer=")+psg_time_of_start_of_buffer);
  s_time=SoundGetTime();
  //we have data from time_of_last_vbl+PSG_WRITE_N_SCREENS_AHEAD*n_samples_per_vbl up to
  //wherever we want
  write_time_1=psg_time_of_last_vbl_for_writing; //3 screens ahead of where the cursor was
  //write_time_1=MAX(write_time_1,min_write_time); //minimum time for new write
  write_time_2=MAX(write_time_1+(n_samples_per_vbl+PSG_WRITE_EXTRA),
    s_time+(n_samples_per_vbl+PSG_WRITE_EXTRA));
  if((write_time_2-write_time_1)>PSG_CHANNEL_BUF_LENGTH)
    write_time_2=write_time_1+PSG_CHANNEL_BUF_LENGTH;
  //psg_last_write_time=write_time_2;
  DWORD time_of_next_vbl_to_write=MAX(s_time+n_samples_per_vbl*psg_write_n_screens_ahead,psg_time_of_next_vbl_for_writing);
  if(time_of_next_vbl_to_write>s_time+n_samples_per_vbl
    *(psg_write_n_screens_ahead+2))
    time_of_next_vbl_to_write=s_time+n_samples_per_vbl
    *(psg_write_n_screens_ahead+2); // new bit added by Ant 9/1/2001 to stop the sound lagging behind
  DBG_LOG(EasyStr("   writing from ")+write_time_1+" to "+write_time_2+"; current play cursor at "+s_time+" ("+play_cursor+"); minimum write at "+min_write_time+" ("+write_cursor+")");
//  log_write(EasyStr("writing ")+(write_time_1-s_time)+" samples ahead of play cursor, "+(write_time_1-min_write_time)+" ahead of min write");
#ifdef SHOW_WAVEFORM
  temp_waveform_display_counter=write_time_1 MOD_PSG_BUF_LENGTH;
  temp_waveform_play_counter=play_cursor;
#endif
  DBG_LOG("SOUND: Working out data up to the end of this VBL plus a bit more for all channels");
#if defined(SSE_YM2149_LL)
  if(OPTION_MAME_YM)
  {
    Psg.psg_write_buffer(time_of_next_vbl_to_write,true);
    // TRACE_OSD("%d",Psg.frame_samples); // should be 882 @50hz
    Psg.frame_samples=0;
    Psg.time_at_vbl_start=Psg.m_cycles; //Psg.time_of_last_sample;
    // Fill extra buffer, but without advancing the YM emu
    for(int i=MAX(1,psg_buf_pointer[0]);i<psg_buf_pointer[0]+PSG_WRITE_EXTRA;i++)
      *(psg_channels_buf+i)=*(psg_channels_buf+i-1);
  }
  else
#endif
  {
    //TRACE_OSD("%d",MAX(MAX(psg_buf_pointer[2],psg_buf_pointer[1]),psg_buf_pointer[0]));
    for(int abc=2;abc>=0;abc--)
      psg_write_buffer(abc,time_of_next_vbl_to_write+PSG_WRITE_EXTRA);
  }
  if(ste_sound_on_this_screen)
  {
#if(PSG_WRITE_EXTRA>0) //300
    WORD w[2]={ste_sound_channel_buf[ste_sound_channel_buf_idx-2],
      ste_sound_channel_buf[ste_sound_channel_buf_idx-1]};
    for(int i=0;i<PSG_WRITE_EXTRA;i++)
    {
      if(ste_sound_channel_buf_idx>=STE_SOUND_BUFFER_LENGTH)
        break;
      ste_sound_channel_buf[ste_sound_channel_buf_idx++]=w[0];
      ste_sound_channel_buf[ste_sound_channel_buf_idx++]=w[1];
    }
#endif
#if defined(SSE_STATS) // record for every frequency
    Stats.mskDigitalSound|=( (1 | ((shifter_sound_mode&0x80)>>3))
      <<shifter_sound_mode);
#endif
  }
#if defined(SSE_SOUND_CARTRIDGE)
  else if(SSEConfig.mv16)
  {
    ste_sound_freq=(WORD)sound_freq; // seems to work with 96K though
    ste_sound_channel_buf[0]=ste_sound_channel_buf[1]
      =ste_sound_last_word;
    ste_sound_channel_buf_idx=0;
  }
#endif
  else
  {
    WORD w1,w2;
    Shifter.sound_get_last_sample(&w1,&w2);
    ste_sound_channel_buf[0]=w1;
    ste_sound_channel_buf[1]=w2;
    ste_sound_channel_buf_idx=0;
  }
  // write_time_1 and 2 are sample variables, convert to bytes
  DWORD StartByte=(write_time_1 MOD_PSG_BUF_LENGTH)*sound_bytes_per_sample;
  DWORD NumBytes=((write_time_2-write_time_1)+1)*sound_bytes_per_sample;
  DBG_LOG(EasyStr("SOUND: Trying to lock from ")+StartByte+", length "+NumBytes);
  Ret=SoundLockBuffer(StartByte,NumBytes,&DatAdr[0],&LockLength[0],
    &DatAdr[1],&LockLength[1]);
  if(Ret!=DSERR_BUFFERLOST)
  {
    if(Ret!=DS_OK)
    {
      log_write("SOUND: Lock totally failed, disaster!");
      return SoundError("Lock for PSG Buffer Failed",Ret);
    }
    DBG_LOG(EasyStr("SOUND: Locked lengths ")+LockLength[0]+", "+LockLength[1]);
    int i=MIN((DWORD)MAX((write_time_1-psg_time_of_last_vbl_for_writing),0ul),
      PSG_CHANNEL_BUF_LENGTH-10);
    int v=psg_voltage,dv=psg_dv; //restore from last time
    DBG_LOG(EasyStr("SOUND: Zeroing channels buffer up to ")+i);
    for(int j=0;j<i;j++)
    {
      //ASSERT(j<PSG_CHANNEL_BUF_LENGTH);
      psg_channels_buf[j]=VOLTAGE_FP(VOLTAGE_ZERO_LEVEL); //zero the start of the buffer
    }
    source_p=psg_channels_buf+i;
    int samples_left_in_buffer=MAX(PSG_CHANNEL_BUF_LENGTH-i,0ul);
    int countdown_to_storing_values=
      MAX((int)(time_of_next_vbl_to_write-write_time_1),0);
    //this is set when we are counting down to the start time of the next write
    bool store_values=false;
    bool chipmode=(!OPTION_MAME_YM && psg_hl_filter==SOUND_PSG_HLF_CHIP);
    if(sound_record)
    {
      if(OPTION_SOUND_RECORD_FORMAT==TOption::SoundFormatYm)
      {
/*  Each VBL we dump PSG registers. We must write the envelope register
    only if it was written to (even same value), otherwise we write $FF.
*/
        fwrite(psg_reg,sizeof(BYTE),13,Wav_file);
        BYTE env=written_to_env_this_vbl?psg_reg[13]:0xFF;
        fwrite(&env,sizeof(BYTE),1,Wav_file);
        written_to_env_this_vbl=false;
      }
      else
      {
        //ASSERT(countdown_to_storing_values<=PSG_CHANNEL_BUF_LENGTH);
        sound_record_to_wav(MIN((DWORD)countdown_to_storing_values,
          PSG_CHANNEL_BUF_LENGTH),write_time_1,chipmode,source_p);
      }
    }
#ifdef WRITE_ONLY_SINE_WAVE
    DWORD t=write_time_1;
#endif
    int val;
    DBG_LOG("SOUND: Starting to write to buffers");
    WORD *lp_ste_sound_channel=ste_sound_channel_buf;
    WORD *lp_max_ste_sound_channel=ste_sound_channel_buf
      +ste_sound_channel_buf_idx;
    BYTE *pb;
    WORD *pw;
    for(int n=0;n<2;n++)
    {
      if(DatAdr[n])
      {
        pb=(BYTE*)(DatAdr[n]);
        pw=(WORD*)(DatAdr[n]);
        int c=MIN(int(LockLength[n]/sound_bytes_per_sample),
          samples_left_in_buffer),oc=c;
        if(c>countdown_to_storing_values)
        {
          c=countdown_to_storing_values;
          oc-=countdown_to_storing_values;
          store_values=true;
        }
        for(;;)
        {
          //TRACE("F%d WRITE_SOUND_LOOP\n",FRAME); 2-3/frame
          if(sound_num_bits==8)
          {
            if(chipmode)
            {
              if(sound_low_quality==0)
                WRITE_SOUND_LOOP(CALC_V_CHIP,pb,BYTE,DWORD_B_1);
              else
                WRITE_SOUND_LOOP(CALC_V_CHIP_25KHZ,pb,BYTE,DWORD_B_1);
            }
            else
              WRITE_SOUND_LOOP(CALC_V_EMU,pb,BYTE,DWORD_B_1);
          }
          else
          {
            if(chipmode)
            {
              if(sound_low_quality==0)
                WRITE_SOUND_LOOP(CALC_V_CHIP,pw,WORD,MSB_W ^ DWORD_W_0);
              else
                WRITE_SOUND_LOOP(CALC_V_CHIP_25KHZ,pw,WORD,MSB_W ^ DWORD_W_0);
            }
            else
              WRITE_SOUND_LOOP(CALC_V_EMU,pw,WORD,MSB_W ^ DWORD_W_0);
          }
          if(store_values)
          {
            c=oc;
            psg_voltage=v;
            psg_dv=dv;
            store_values=false;
            countdown_to_storing_values=0x7fffffff; //don't store the values again.
          }
          else
          {
            countdown_to_storing_values-=LockLength[n]/sound_bytes_per_sample;
            break;
          }
        }
        samples_left_in_buffer-=LockLength[n]/sound_bytes_per_sample;
      }
    }
#if defined(SSE_VID_RECORD_AVI) 
    if(video_recording&&SoundBuf&&pAviFile&&pAviFile->Initialised)
      pAviFile->AppendSound(DatAdr[0],LockLength[0]);
#endif
    SoundUnlock(DatAdr[0],LockLength[0],DatAdr[1],LockLength[1]);
    //ASSERT(source_p<=(psg_channels_buf+PSG_CHANNEL_BUF_LENGTH));
    while(source_p<(psg_channels_buf+PSG_CHANNEL_BUF_LENGTH))
      *(source_p++)=VOLTAGE_FP(VOLTAGE_ZERO_LEVEL); //zero the rest of the buffer
  }
  psg_buf_pointer[0]=psg_buf_pointer[1]=psg_buf_pointer[2]=0;
  psg_time_of_last_vbl_for_writing=time_of_next_vbl_to_write;
  psg_time_of_next_vbl_for_writing=MAX(s_time+n_samples_per_vbl
    *(psg_write_n_screens_ahead+1),time_of_next_vbl_to_write+n_samples_per_vbl);
  psg_time_of_next_vbl_for_writing=MIN(psg_time_of_next_vbl_for_writing,
    s_time+(PSG_BUF_LENGTH/2));
  DBG_LOG(EasyStr("SOUND: psg_time_of_next_vbl_for_writing=")+psg_time_of_next_vbl_for_writing);
  psg_n_samples_this_vbl=psg_time_of_next_vbl_for_writing-psg_time_of_last_vbl_for_writing;
  DBG_LOG("SOUND: End of Sound_VBL");
  DBG_LOG("");
  return DS_OK;
}


HRESULT Sound_Start() { //Called from run()

#ifdef UNIX
#if !defined(SSE_NO_INTERNAL_SPEAKER)
  if(sound_internal_speaker)
  {
    console_device=open("/dev/console",O_RDONLY|O_NDELAY,0);
    if(console_device==-1)
    {
      printf("Couldn't open console for internal speaker output\n");
      sound_internal_speaker=false;
      GUIUpdateInternalSpeakerBut();
    }
  }
#endif
#endif//UNIX

  if(OPTION_SOUNDMUTE) 
    return DS_OK;
  if(UseSound==0) 
    return DSERR_GENERIC;  // Not initialised
  if(SoundActive()) 
    return DS_OK;        // Already started
  if(fast_forward||slow_motion||runstate!=RUNSTATE_RUNNING) 
    return DSERR_GENERIC;
  if(!OPTION_MAME_YM)
    sound_first_vbl=true;
  DBG_LOG("SOUND: Starting sound buffers and initialising PSG variables");
  // Work out startup voltage
  int envshape=psg_reg[13]&15;
  int flatlevel=0;
  for(int abc=0;abc<3;abc++)
  {
    if((psg_reg[8+abc]&BIT_4)==0)
      flatlevel+=psg_flat_volume_level[psg_reg[8+abc]&15];
    else if(envshape==b1011||envshape==b1101)
      flatlevel+=psg_flat_volume_level[15];
  }
  psg_voltage=flatlevel;
  psg_dv=0;
  WORD dma_l,dma_r;
  Shifter.sound_get_last_sample(&dma_l,&dma_r);
  int current_l=HIBYTE(flatlevel)+HIBYTE(dma_l),current_r=HIBYTE(flatlevel)
    +HIBYTE(dma_r);
  if(SoundStartBuffer((signed char)current_l,(signed char)current_r)!=DS_OK)
    return DDERR_GENERIC;
  {
    WORD *p=(WORD*)psg_noise,*q=(WORD*)(psg_noise+PSG_NOISE_ARRAY);
    do
      *p++=(rand()&0x0101); //assume RAND_MAX=$7FFF
    while(p<q);
  }
#ifdef ONEGAME
  // Make sure sound is still good(ish) if you are running below 80% speed
  OGExtraSamplesPerVBL=300;
  if(run_speed_ticks_per_second>1000)
  {
// Get the number of extra ms of sound per "second", change that to number
// of samples, divide to get the number of samples per VBL and add extra.
    OGExtraSamplesPerVBL=((((run_speed_ticks_per_second-1000)*sound_freq)/1000)/Glue.video_freq)+300;
  }
#endif
  psg_time_of_start_of_buffer=psg_last_play_cursor=0;
  psg_time_of_last_vbl_for_writing=psg_time_of_next_vbl_for_writing=0;
  for(int abc=2;abc>=0;abc--)
  {
    psg_buf_pointer[abc]=0;
    psg_tone_start_time[abc]=0;
  }
  for(DWORD i=0;i<PSG_CHANNEL_BUF_LENGTH;i++) 
    psg_channels_buf[i]=VOLTAGE_FP(VOLTAGE_ZERO_LEVEL);
  psg_envelope_start_time=0xff000000;
  if(sound_record)
  {
    timer=timeGetTime();
    sound_record_start_time=timer+200; //start recording in 200ms time
    sound_record_open_file();
  }
#if defined(SSE_YM2149_LL)
  if(Psg.AntiAlias)
  {
    Psg.time_at_vbl_start=Psg.m_cycles;
    Psg.AntiAlias->init();
  }
#endif
  return DS_OK;
}


#if !defined(SSE_NO_INTERNAL_SPEAKER)

void SoundStopInternalSpeaker() {
  internal_speaker_sound_by_period(0);
}

bool sound_internal_speaker=false;

#endif


// Microwire (STE)

void TLMC1992::Reset(bool Cold) { // there's no reset line
  if(Cold)
  {
    ZeroMemory(this,sizeof(TLMC1992));
    bass=treble=6; // 6 is neutral value
    mixer=1;
    Mask=0x07ff;
    volume=40;
    volume_l=20;
    volume_r=20;
    top_val_l=128;
    top_val_r=128;
  }
  StartTime=0;
  old_top_val_l=top_val_l;
  old_top_val_r=top_val_r;
}


void TLMC1992::Process(int channel,int &val) {
  if(DSP_DISABLED)
    return;
  double d_dsp_v=val;
  if(bass!=6)
    d_dsp_v=MicrowireBass[channel].FilterAudio(d_dsp_v,MW_LOW_SHELF_FREQ,
      bass-6);
  if(treble!=6)
    d_dsp_v=MicrowireTreble[channel].FilterAudio(d_dsp_v,MW_HIGH_SHELF_FREQ,
     treble-6);
  if(!OPTION_HACKS && (volume<0x28||volume_l<0x14 &&!channel 
    ||volume_r<0x14 &&channel))//3.6.1: 2 channels
    d_dsp_v=MicrowireVolume[channel].FilterAudio(d_dsp_v,volume-0x28
      + ( (channel) ? (volume_r-0x14) : (volume_l-0x14)));
  val=(int)d_dsp_v;
}
