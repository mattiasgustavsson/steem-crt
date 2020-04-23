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
FILE: interface_rta.cpp
DESCRIPTION: This file contains client code for RtAudio.
Apparently it is supposed to work with Windows or Unix but it is only
used in the Unix builds.  The file isn't part of the Windows builds.
---------------------------------------------------------------------------*/

#ifndef NO_RTAUDIO

#include "pch.h"
#include <computer.h>
#include <translate.h>
#include <gui.h>
#include <sound.h>


bool rt_started=0; // true when playing a sample
int rt_buffer_size=256,rt_buffer_num=4;
RtAudio *rt_audio=NULL;
int rt_unsigned_8bit=0;


HRESULT Rt_Init() {
  try {
    rt_audio=new RtAudio(); // declared in init_sound.h
  } 
  catch (RtError &error) {
    TRACE2("Create RtAudio Fail!\n");
    error.printMessage();
    return DSERR_GENERIC;
  }
  UseSound=XS_RT;
  if(sound_device_name.IsEmpty())
  {
    RtAudio::DeviceInfo info;
    unsigned int devices=rt_audio->getDeviceCount();
    //TRACE2("There are %u devices\n",devices);
    for(unsigned int i=0;i<devices;i++)
    {
      //TRACE2("getting info for device %u\n",i);
      info=rt_audio->getDeviceInfo(i);
      if(info.probed && info.outputChannels>0)
      {
//        TRACE2("RtAudio device %u #output channels: %u Formats:%X\n",i,info.outputChannels,info.nativeFormats);
        if(info.isDefaultOutput)
          sound_device_name=info.name.c_str();
      }
    }//nxt
  }
  return DS_OK;
}


DWORD Rt_GetTime() {
  return sound_buf_pointer;
}


void Rt_Release() {
  Rt_FreeBuffer(true);
  delete rt_audio;
  rt_audio=NULL;
}


HRESULT Rt_StartBuffer(int flatlevel1,int flatlevel2) {
  if(rt_audio==NULL || !sound_bytes_per_sample)
    return DSERR_GENERIC;
  Rt_FreeBuffer(true);
  unsigned int bufferSize=rt_buffer_size;  // 256 sample frames
  unsigned int device=0;        // 0 indicates the default or first available device
  RtAudioFormat format=(sound_num_bits==16) ? RTAUDIO_SINT16 : RTAUDIO_SINT8;
  RtAudio::DeviceInfo info;
  unsigned int devices=rt_audio->getDeviceCount();
  for(unsigned int i=0;i<devices;i++)
  {
    info=rt_audio->getDeviceInfo(i);
    if(IsSameStr_I(info.name.c_str(),sound_device_name))
    {
      device=i;
      break;
    }
  }//nxt
  info=rt_audio->getDeviceInfo(device);
  int closest_freq=0,f;
  for(int n=0;n<(int)(info.sampleRates.size());n++)
  {
    f=info.sampleRates[n]; // it's a vector
    if(abs(sound_chosen_freq-f)<abs(sound_chosen_freq-closest_freq))
      closest_freq=f;
  }
  if(!closest_freq) 
    closest_freq=44100;
  sound_freq=closest_freq;
  // Let's open a stream
  RtAudio::StreamParameters outParams;
  //ASSERT(device==rt_audio->getDefaultOutputDevice());
  outParams.deviceId=device;
  outParams.nChannels=sound_num_channels; // psg.h, =1
  try {
    rt_audio->openStream(&outParams,NULL,format,sound_freq,
      (unsigned int*)&rt_buffer_size, Rt_Callback);
  }
  catch(RtError &error) {
    TRACE2("RtAudio failed to open stream\n");
    try { //?
      rt_audio->closeStream();
    }
    catch(...) {
      TRACE2("Error while closing stream\n");
    }
    error.printMessage();
    return DSERR_GENERIC;
  }
  sound_buffer_length=X_SOUND_BUF_LEN_BYTES/sound_bytes_per_sample;
  XSoundInitBuffer(flatlevel1,flatlevel2);
  sound_buf_pointer=0; // sample count
  try {
    rt_audio->startStream();
  }
  catch (RtError &error) {
    TRACE2("RtAudio failed to start stream\n");
    try{
      rt_audio->stopStream();
    }
    catch(...) {
      try {
        rt_audio->closeStream();
      } catch(...) {}
    }
    error.printMessage();
    return DSERR_GENERIC;
  }
  rt_started=true;
  WIN_ONLY( DSOpen=true; )
  return DS_OK;
}


int Rt_Callback(void *pOutBuf, void*, unsigned int bufferSize, double, 
                RtAudioStreamStatus, void*) {
  char *buffer=(char*)pOutBuf;
  int pointer_byte=sound_buf_pointer;
  pointer_byte%=sound_buffer_length; // Get sample count within buffer
  pointer_byte*=sound_bytes_per_sample; // Convert to bytes
  for(DWORD i=0;i<bufferSize;i++)
  {
    for(int a=0;a<sound_bytes_per_sample;a++)
    {
      *buffer=x_sound_buf[pointer_byte];
      if(sound_num_bits==8 && !rt_unsigned_8bit) // CPU optimises this
        *buffer^=128;
      pointer_byte++;
      if(pointer_byte>=X_SOUND_BUF_LEN_BYTES)
        pointer_byte-=X_SOUND_BUF_LEN_BYTES;
      buffer++;
    }
    sound_buf_pointer++;
  }
  return 0; // value for OK
}


bool Rt_IsPlaying() {
 return rt_started; 
}


void Rt_ChangeVolume() {
}


void Rt_FreeBuffer(bool) {
  if(!rt_started)
    return;
  try {
    rt_audio->stopStream();
  }
  catch(RtError& e) {
    TRACE2("RtAudio failed to stop stream\n");
    e.printMessage();
  }
  if(rt_audio->isStreamOpen())
    rt_audio->closeStream(); 
  rt_started=0;
}


HRESULT Rt_Stop(bool Immediate) {
  Rt_FreeBuffer(Immediate);
  return DSERR_GENERIC;
}

#endif//#ifndef NO_RTAUDIO
