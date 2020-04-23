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

DOMAIN: Disk image
FILE: disk_hfe.cpp
CONDITION: SSE_DISK_HFE must be defined
DESCRIPTION: The HFE interface is based on the STW interface, so that
integration in Steem (disk manager, FDC commands...) is straightforward.
HFE (Rev.1.1) is a lot like STW, except the bits of each MFM word are reversed
and data of each side is intertwined, which is the way HxC hardware works.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#if defined(SSE_DISK_HFE)

#include <computer.h>
#include <mymisc.h>
#include <gui.h>
#include <disk_hfe.h>
#include <debug.h>
#include <options.h>

#define IMAGE_SIZE image_size //member variable
#define NUM_SIDES 2
#define NUM_TRACKS file_header->number_of_track
#define BLOCK_SIZE 512
#define MFM_SIDE_BLOCK_SIZE 128
#define TRACKBYTES FloppyDisk[Id].TrackBytes

#define LOGSECTION LOGSECTION_IMAGE_INFO


TImageHFE::TImageHFE() {
  Init();
}


TImageHFE::~TImageHFE() {
  Close();
}


void TImageHFE::Close() {
  if(fCurrentImage)
  {
    TRACE_LOG("HFE %s image\n",FloppyDisk[Id].WrittenTo?"save and close":"close");
    fseek(fCurrentImage,0,SEEK_SET); // rewind
    if(ImageData && FloppyDisk[Id].WrittenTo)
      fwrite(ImageData,1,IMAGE_SIZE,fCurrentImage); //save
    fclose(fCurrentImage);
    free(ImageData);
  }
  Init();  
}


bool TImageHFE::Create(char *path) {
  // utility called by Disk manager
  bool ok=false;
  Close();
  fCurrentImage=fopen(path,"wb+"); // create new image
  if(fCurrentImage)
  {
    // copy overhead of a HFE file
#if 1 && defined(SSE_FILES_IN_RC)
    // first look for the file, if it isn't there, use internal resource
    EasyStr filename=RunDir+SLASH+SSE_PLUGIN_DIR1+SLASH+DISK_HFE_BOOT_FILENAME;
    FILE *fp=fopen(filename.Text,"rb"); // open boot
    if(!fp)
    {
      EasyStr filename=RunDir+SLASH+SSE_PLUGIN_DIR2+SLASH+DISK_HFE_BOOT_FILENAME;
      fp=fopen(filename.Text,"rb"); // open boot
    }
    if(!fp)
    {
      filename=RunDir+SLASH+DISK_HFE_BOOT_FILENAME;
      fp=fopen(filename.Text,"rb"); // open boot
    }
    if(fp)
    {
      ASSERT(GetFileLength(fp)==1024);
      for(int i=0;i<1024;i++)
        fputc(fgetc(fp),fCurrentImage);
      for(int i=0;i<84*512*49;i++)
        fputc(rand()&0xff,fCurrentImage); //random data (unformatted)
      ok=true;
      fclose(fp);
    }
    else 
    {
      HRSRC rc=FindResource(NULL,MAKEINTRESOURCE(IDR_HFEBOOT),RT_RCDATA);
      ASSERT(rc);
      if(rc)
      {
        HGLOBAL hglob=LoadResource(NULL,rc);
        if(hglob)
        {
          size_t size=SizeofResource(NULL,rc);
          BYTE *pdata=(BYTE*)LockResource(hglob);
          if(pdata && size==1024) //1Kb
          {
            fwrite((void*)pdata,size,1,fCurrentImage);
            ok=true;
          }
        }
      }//if(rc)
    }
    for(int i=0;i<84*512*49;i++)
      fputc(rand()&0xff,fCurrentImage); //random data (unformatted)
    Close();
#else//#if defined(SSE_FILES_IN_RC)
    EasyStr filename=RunDir+SLASH+"plugins"+SLASH+DISK_HFE_BOOT_FILENAME;
    FILE *fp=fopen(filename.Text,"rb"); // open boot
    if(!fp)
    {
      filename=RunDir+SLASH+DISK_HFE_BOOT_FILENAME;
      fp=fopen(filename.Text,"rb"); // open boot
    }
    if(fp)
    {
      ASSERT(GetFileLength(fp)==1024);
      for(int i=0;i<1024;i++) // copy boot: we use a file to reduce overhead
        fputc(fgetc(fp),fCurrentImage);  //in Steem itself
      for(int i=0;i<84*512*49;i++)
        fputc(rand()&0xff,fCurrentImage); //random data (unformatted)
      ok=true;
      fclose(fp);
      Close(); 
    }
#endif//#if defined(SSE_FILES_IN_RC)
  }
  TRACE_LOG("HFE create %s %s\n",path,ok?"OK":"failed");  
  return ok;
}


int TImageHFE::ComputeIndex() {
/*
"  A track data is a table containing the bit stream of a track of the floppy disk. A track can
contain a MFM / FM / GCR or a custom encoding.
The track is divided in block of 512bytes and each block contains a part of the Side 0
track and a part of the Side 1 track:
Figure 1 : A track data
The bits transmitting order to the FDC is :
Bit 0-> Bit 1-> Bit 2-> Bit 3-> Bit 4-> Bit 5-> Bit 6-> Bit 7->(next byte)"

->  HFE format is confusing with interleaved sides:
    256 (MFM) bytes for side 0 then 256 bytes for side 1...
    Each MFM word (clock + data) is reversed : Bit 0-> ... -> Bit 15
*/

    BYTE &current_side=FloppyDisk[Id].current_side;
    int block=(Position/MFM_SIDE_BLOCK_SIZE)*2+current_side;
    int index=(Position%MFM_SIDE_BLOCK_SIZE)+block*MFM_SIDE_BLOCK_SIZE;
    //ASSERT( index>=0 && index< TRACKBYTES*2+128*current_side );
    return index;
}


WORD TImageHFE::GetMfmData(WORD position) {
  WORD mfm_data=0xFFFF;
  if(ImageData && TrackData)
  {
    // must compute new starting point?
    if(position!=mfm_data) //dubious optimisation, it's 0xFFFF
      ComputePosition(position);
    int index=ComputeIndex();
    WORD mirror_mfm=TrackData[index];
    mfm_data=Fdc.Mfm.encoded=MirrorMFM(mirror_mfm); // reverse bits
    IncPosition();
  }
  return mfm_data;
}


void TImageHFE::Init() {
  fCurrentImage=NULL;
  ImageData=NULL;
  TrackData=NULL;
  IMAGE_SIZE=0;
}


bool TImageHFE::LoadTrack(BYTE side,BYTE track,bool) {
  //ASSERT(Id==0||Id==1);
  bool ok=false;
  if(side<NUM_SIDES && track<NUM_TRACKS && ImageData)  
  {
    int position= track_header[track].offset*BLOCK_SIZE;
    TRACKBYTES=track_header[track].track_len/4; // same for both sides
    //ASSERT(TRACKBYTES>=0 && TRACKBYTES<6500);
    //ASSERT(Id==DRIVE);
    FloppyDisk[Id].TrackBytes=TRACKBYTES; // for HFE it's track-dependent
#ifdef SSE_DEBUG
    if(TrackData!=(WORD*)(ImageData+position)) //only once
      TRACE_LOG("HFE LoadTrack side %d track %d offset %d position %d len %d bytes %d\n", side,track,track_header[track].offset,position,track_header[track].track_len,TRACKBYTES);
#endif
    TrackData=(WORD*)(ImageData+position);
    FloppyDisk[Id].current_side=side;
    FloppyDisk[Id].current_track=track;
    ok=true;
  }
#ifdef SSE_DEBUG
  else
    TRACE_LOG("HFE can't load side %d track %d from %p\n",side,track,ImageData);
#endif
  return ok;
}


WORD TImageHFE::MirrorMFM(WORD mfm_word) {
/*  Because it's easier so for HxC hardware, bits are in the
    reverse order each word.  eg  $4489 reads $9122
    Needed some time to figure this out :)
*/
    WORD mirror_mfm=0;
    for(int i=0;i<16;i++)
    {
      int bit=mfm_word&1;
      mirror_mfm|=bit; // bit is 0 otherwise (init)
      mfm_word>>=1;
      if(i<15)
        mirror_mfm<<=1;
    }
    return mirror_mfm;
}


bool TImageHFE::Open(char *path) {
  bool ok=false;
  Close(); // make sure previous image is correctly closed
  fCurrentImage=fopen(path,"rb+"); // try to open existing file
  if(!fCurrentImage) // maybe it's read-only
    fCurrentImage=fopen(path,"rb");
  //ASSERT(fCurrentImage);
  if(fCurrentImage) // image exists
  {
    IMAGE_SIZE=GetFileLength(fCurrentImage);
    //ASSERT(IMAGE_SIZE>0);
    ImageData=(BYTE*)malloc(IMAGE_SIZE);
    //ASSERT(ImageData);
    if(ImageData)
    {
      fread(ImageData,1,IMAGE_SIZE,fCurrentImage); // read all in memory (2MB OK)

      file_header=(picfileformatheader*)ImageData;
      if(!strncmp("HXCPICFE",(char*)file_header->HEADERSIGNATURE,8))  // it's HFE
      {
#ifdef SSE_DEBUG //lots of info
        TRACE_LOG("Open HFE size %d v%d sides %d tracks %d encoding %X mode %X bitRate %d\n",
  IMAGE_SIZE,file_header->formatrevision,file_header->number_of_side,
  file_header->number_of_track,file_header->track_encoding,
  file_header->floppyinterfacemode,file_header->bitRate);
TRACE_LOG("RPM %d  WP %d WA %X offset %d step %X TR0/1 %X%X TR1/1 %X%X\n",
  file_header->floppyRPM,file_header->write_protected,file_header->write_allowed,
  file_header->track_list_offset,file_header->single_step,file_header->track0s0_altencoding,
  file_header->track0s0_encoding,file_header->track0s1_altencoding,file_header->track0s1_encoding);
#endif

        track_header=(pictrack*)(ImageData+file_header->track_list_offset*BLOCK_SIZE);
        ok=true;
      }
    }
  }//if(fCurrentImage)
  if(!ok)
    Close();
  else 
    FloppyDrive[Id].MfmManager=this;
  return ok;
}


void TImageHFE::SetMfmData(WORD position,WORD mfm_data) {
  // if disk is read-only, we still write but changes will be lost
  if(ImageData && TrackData)
  {
    // must compute new starting point?
    if(position!=0xFFFF)
      ComputePosition(position);
    if(!FloppyDisk[Id].ReadOnly)
      FloppyDisk[Id].WrittenTo=true;
    //ASSERT(mfm_data==Fdc.Mfm.encoded);
    int index=ComputeIndex();
    WORD mirror_mfm=MirrorMFM(mfm_data); // reverse bits
    TrackData[index]=mirror_mfm; 
    Fdc.Mfm.encoded=0;
    IncPosition();
  }
}


#endif//#if defined(SSE_DISK_HFE)
