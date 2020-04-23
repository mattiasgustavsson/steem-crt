/*---------------------------------------------------------------------------
PROJECT: Steem SSE
Atari ST emulator
Copyright (C) 2020 by Anthony Hayward, Russel Hayward and STeven

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
FILE: interface_pa.cpp
DESCRIPTION: This file contains client code for PortAudio.
Apparently it is supposed to work with Windows or Unix but it is only
used in the Unix builds. The file isn't part of the Windows builds.
---------------------------------------------------------------------------*/

#ifndef NO_PORTAUDIO

#include "pch.h"
#include <computer.h>
#include <translate.h>
#include <gui.h>
#include <sound.h>


PaStream *pa_out=NULL;
PaTime pa_start_time;
bool pa_init=0;
int pa_output_buffer_size=128;


HRESULT PA_Init() {
  WIN_ONLY( if (Pa_LoadDLL()==paNoError) ) // that function doesn't exist
  {
    PaError err=Pa_Initialize();
    if(err==paNoError)
    {
      pa_init=true;
#ifdef UNIX
      if(sound_device_name.IsEmpty())
        sound_device_name=Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->name;
#endif
      UseSound=XS_PA;
      return DS_OK;
    }
    TRACE2("PA_Init failed (%s)\n",Pa_GetErrorText(err));
  }
  return DSERR_GENERIC;
}


DWORD PA_GetTime() {
  if(pa_out==NULL) 
    return 0;
  if(sound_time_method<2)
  {
   // return DWORD(Pa_GetStreamTime(pa_out)-pa_start_time);
   return sound_buf_pointer;
  }
  else
  {
    DWORD mSecs=(timeGetTime()-SoundBufStartTime);
    return (mSecs*sound_freq)/1000;
  }
}


void PA_Release() {
  if(pa_init)
  {
    PA_FreeBuffer(true);
    Pa_Terminate();
    WIN_ONLY( Pa_FreeDLL(); ) // that function doesn't exist
    pa_init=0;
  }
}


HRESULT PA_StartBuffer(int flatlevel1,int flatlevel2) {
  if(!pa_init || !sound_bytes_per_sample)
    return DSERR_GENERIC;
  if(pa_out)
    PA_FreeBuffer(true);
  sound_buffer_length=X_SOUND_BUF_LEN_BYTES/sound_bytes_per_sample;
  PaSampleFormat pa_format=(sound_num_bits==16) ? paInt16 : paUInt8;
  PaDeviceIndex out_dev=Pa_GetDefaultOutputDevice();
#ifdef UNIX
  int c=Pa_GetDeviceCount();
  for(PaDeviceIndex i=0;i<c;i++)
  {
    const PaDeviceInfo *pdev=Pa_GetDeviceInfo(i);
    if(pdev->maxOutputChannels>0)
    {
      if(IsSameStr_I(pdev->name,sound_device_name))
      {
        out_dev=i;
        break;
      }
    }
  }
#endif
  const PaDeviceInfo *pdev=Pa_GetDeviceInfo(out_dev);
  sound_freq=sound_chosen_freq;
  PaStreamParameters outStreamParams;
  ZeroMemory(&outStreamParams,sizeof(PaStreamParameters));
  outStreamParams.device=out_dev;
  outStreamParams.channelCount=sound_num_channels;
  outStreamParams.sampleFormat=pa_format;
  PaError err=Pa_OpenStream(&pa_out,
                            NULL,
                            &outStreamParams,
                            sound_freq,
                            pa_output_buffer_size,
                            paDitherOff | paClipOff,
                            PA_Callback,
                            NULL);
  if(pa_out==NULL)
  {
    TRACE2("Pa_OpenStream Error: %s\n",Pa_GetErrorText(err));
    return DSERR_GENERIC;
  }
  XSoundInitBuffer(flatlevel1,flatlevel2);
  err=Pa_StartStream(pa_out);
  if(err)
  {
    PA_FreeBuffer(true);
    TRACE2("Pa_StartStream Error: %s\n",Pa_GetErrorText(err));
    return DSERR_GENERIC;
  }
  pa_start_time=Pa_GetStreamTime(pa_out);
  sound_buf_pointer=0;// sample count
  WIN_ONLY( DSOpen=true; )
  return DS_OK;
}


int PA_Callback(const void*,void *pOutBuf,unsigned long bufferSize, const 
                PaStreamCallbackTimeInfo* OutTime,PaStreamCallbackFlags,void*) {
  if(pOutBuf==NULL || !bufferSize || !sound_buffer_length) 
    return 0;
  char *buffer=(char*)pOutBuf;
  int pointer_byte=sound_buf_pointer;
  pointer_byte%=sound_buffer_length; // Get sample count within buffer
  pointer_byte*=sound_bytes_per_sample; // Convert to bytes
  for(DWORD i=0;i<bufferSize;i++)
  {
    for(int a=0;a<sound_bytes_per_sample;a++)
    {
      *buffer=x_sound_buf[pointer_byte];
      if(sound_num_bits==8) // CPU optimises this
        *buffer^=128;
      pointer_byte++;
      if(pointer_byte>=X_SOUND_BUF_LEN_BYTES)
        pointer_byte-=X_SOUND_BUF_LEN_BYTES;
      buffer++;
    }
    sound_buf_pointer++;
  }
  return paContinue;
}


bool PA_IsPlaying() { 
  return pa_out!=NULL; 
}


void PA_ChangeVolume() {
}


void PA_FreeBuffer(bool Immediate) {
  if(pa_out==NULL)
    return;
  if(Immediate)
    Pa_AbortStream(pa_out);
  else
    Pa_StopStream(pa_out);
  Pa_CloseStream(pa_out);
  pa_out=NULL;
  WIN_ONLY( DSOpen=0; )
}


HRESULT PA_Stop(bool Immediate) {
  PA_FreeBuffer(Immediate);
  return DS_OK;
}

#endif//#ifndef NO_PORTAUDIO
