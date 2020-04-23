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
FILE: disk_stw.cpp
CONDITION: SSE_DISK_STW must be defined
DESCRIPTION: STW is yet another Atari ST disk image format, devised for Steem
SSE. The W in STW stands for 'write', ST is a reference to the well known 
ST format and of course to the Atari ST itself.
The purpose of this format is to allow emulation of all WD1772 (floppy
disk controller) commands in Steem SSE, and keep the results of command
Write Track (Format).
We really go for it here as this simplistic interface knows nothing but
the side, track and words on the track. MFM encoding/decoding, timing and
all the rest is for the distinct floppy drive and fdc emulators.
v4
We handle ST/MSA/DIM <-> STW conversion here, those routines know about
disk structure.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#if defined(SSE_DISK_STW)

#include <acc.h>
#include <disk_stw.h>
#include <debug.h>
#include <computer.h>
#include <osd.h>

#define HEADER_SIZE (4+2+1+1+2) //STW Version nSides nTracks nTrackBytes
#define TRACK_HEADER_SIZE (3+2) //"TRK" side track
#define IMAGE_SIZE (HEADER_SIZE+nSides*nTracks*\
                    (TRACK_HEADER_SIZE+nTrackBytes*sizeof(WORD)))


#define LOGSECTION LOGSECTION_IMAGE_INFO


TImageSTW::TImageSTW() {
  Init();
}


TImageSTW::~TImageSTW() {
  Close();
}


void TImageSTW::Close() {
  if(fCurrentImage)
  {
#if defined(SSE_DEBUG)
    TRACE_LOG("STW %d %s image\n",Id,FloppyDisk[Id].WrittenTo?"save and close":"close");
#endif
    fseek(fCurrentImage,0,SEEK_SET); // rewind
    if(ImageData && FloppyDisk[Id].WrittenTo)
      fwrite(ImageData,1,IMAGE_SIZE,fCurrentImage);
    fclose(fCurrentImage);
    free(ImageData);
  }
  Init(); // zeroes variables
}


bool TImageSTW::Create(char *path
#if defined(SSE_DISK_HD)
                       ,BYTE density/*=1*/
#endif
                       ) {
  // utility called by Disk manager and for auto MFM conversion
  //ASSERT(Id==0);
  bool ok=false;
  Close();
  fCurrentImage=fopen(path,"wb+"); // create new image
  if(fCurrentImage)
  {
    // write header
    fwrite(DISK_EXT_STW,1,4,fCurrentImage); // "STW"
    SWAP_BIG_ENDIAN_WORD(Version);
    fwrite(&Version,sizeof(WORD),1,fCurrentImage);
    SWAP_BIG_ENDIAN_WORD(Version);
    fwrite(&nSides,sizeof(BYTE),1,fCurrentImage);
    fwrite(&nTracks,sizeof(BYTE),1,fCurrentImage);
#if defined(SSE_DISK_HD)
    nTrackBytes=DISK_BYTES_PER_TRACK*density;
#endif
    SWAP_BIG_ENDIAN_WORD(nTrackBytes);
    fwrite(&nTrackBytes,sizeof(WORD),1,fCurrentImage);
    SWAP_BIG_ENDIAN_WORD(nTrackBytes);
    // init all tracks with random bytes (unformatted disk) 
    for(BYTE track=0;track<nTracks;track++)
    {
      for(BYTE side=0;side<nSides;side++)
      {
        // this can be seen as "metaformat"
        fwrite("TRK",1,3,fCurrentImage);
        fwrite(&side,1,1,fCurrentImage);
        fwrite(&track,1,1,fCurrentImage);

        WORD data; // MFM encoding = clock byte and data byte mixed
        for(int byte=0;byte<nTrackBytes;byte++)
        {
          data=(WORD)rand();
          fwrite(&data,sizeof(data),1,fCurrentImage); 
        }
      }
    }
    ok=true;
    Close(); 
  }
  TRACE_LOG("STW create %s %s\n",path,ok?"OK":"failed");  
  return ok;
}


WORD TImageSTW::GetMfmData(WORD position) {
  WORD mfm_data=0xFFFF;
  // must compute new starting point?
  if(position!=mfm_data) //dubious optimisation, it's 0xFFFF
    ComputePosition(position);
  if(TrackData && Position<nTrackBytes)
  {
    mfm_data=TrackData[Position];
    SWAP_BIG_ENDIAN_WORD(mfm_data);
    IncPosition();
  }
#ifdef SSE_DEBUG
  else 
  {
    TRACE_LOG("GetMfmData(%c:%d) error\n",'A'+DRIVE,Position);
    TRACE_OSD("STW ERR");
  }
#endif
  return mfm_data;
}


bool TImageSTW::GetSectorData(BYTE side,BYTE track,BYTE sector,BYTE *pdata) {
  //  used for conversion back to ST and for disk properties
  TWD1772MFM wd1772mfm;
  // for each sector we start from the index, that's because 11-sector
  // tracks are interleaved and we don't care for performance
  Position=0;
  bool ok=LoadTrack(side,track); // also inefficient, load every time (it's just a pointer for STW)
  bool found_header=false,found_data=false;
  int n4489=0,nbytes=0;
  for(int i=0;ok && i<nTrackBytes;i++)
  {
    wd1772mfm.encoded=GetMfmData(0xFFFF); // it ++
    if(wd1772mfm.encoded==0x4489)
      n4489++;
    wd1772mfm.Decode();
    if(found_data)
    {
      pdata[nbytes]=wd1772mfm.data;
      nbytes++;
      if(nbytes==512)
      {
        //TRACE_LOG("written sector %d %d %d\n",side,track,sector2);
        ok=true;
        break; // sector done
      }
    }
    else switch(n4489) {
    case 0:case 1: case 2: 
      if(wd1772mfm.encoded!=0x4489)
        n4489=0; // 3 or nothing
      break;
    case 3: // 3A1 = Address Mark
      if(wd1772mfm.encoded==0x4489)
        ; // 3rd a1
      else if((wd1772mfm.data&0xFE)==0xFE)
        n4489++; // followed with track side sector
      else 
      {
        if((wd1772mfm.data&0xFE)==0xFA && found_header) // $FB
          found_data=true; // followed with 512 words of MFM data to convert
        n4489=0;
      }
      break;
    case 4: // 4-5-6 find header, we don't care about CRC
      if(wd1772mfm.data==track)
        n4489++;
      else
        n4489=0;
      break;
    case 5:
      if(wd1772mfm.data==side)
        n4489++;
      else
        n4489=0;
      break;
    case 6:
      if(wd1772mfm.data==sector)
      {
        found_header=true;
        //TRACE_LOG("found header %d %d %d\n",side,track,sector2);
      }
      n4489=0;
      break;
    }//sw
  }
  return ok;
}


void TImageSTW::Init() {
  Version=0x0100; // 1.0
  fCurrentImage=NULL;
  ImageData=NULL;
  TrackData=NULL;
  nSides=2;
  nTracks=84;
  nTrackBytes=DISK_BYTES_PER_TRACK;
}


bool  TImageSTW::LoadTrack(BYTE side,BYTE track,bool) {
  //ASSERT(Id==0||Id==1);
  bool ok=false;
  if(side<nSides && track<nTracks && ImageData)  
  {
    int position=HEADER_SIZE
      +track*nSides*(TRACK_HEADER_SIZE+nTrackBytes*sizeof(WORD))
      +side*(TRACK_HEADER_SIZE+nTrackBytes*sizeof(WORD));
    //runtime format check
    if( !strncmp("TRK",(char*)ImageData+position,3) 
      && *(ImageData+position+3)==side && *(ImageData+position+4)==track)
    {
#ifdef SSE_DEBUG
      if(TrackData!=(WORD*)(ImageData+position+TRACK_HEADER_SIZE)) //only once
        TRACE_LOG("STW LoadTrack %c: side %d track %d\n",'A'+DRIVE,side,track);  
#endif
      FloppyDisk[Id].current_side=side;
      FloppyDisk[Id].current_track=track;
      TrackData=(WORD*)(ImageData+position+TRACK_HEADER_SIZE);
      ok=true;
    }
  }
#ifdef SSE_DEBUG
  else
    TRACE_LOG("STW can't load side %d track %d from %p\n",side,track,ImageData);
#endif
  return ok;
}


bool TImageSTW::Open(char *path) {
  bool ok=false;
  Close(); // make sure previous image is correctly closed
  fCurrentImage=fopen(path,"rb+"); // try to open existing file
  if(!fCurrentImage) // maybe it's read-only
    fCurrentImage=fopen(path,"rb");
  if(fCurrentImage) // image exists
  {
#if defined(SSE_DISK_HD) // read header first to check size (dd or hd)
    BYTE header[10];
    fread(header,1,10,fCurrentImage);
    fseek(fCurrentImage,0,SEEK_SET);
    if(!strncmp(DISK_EXT_STW,(char*)header,3)) // it's STW
    {
      Version=*(WORD*)(header+4);
      SWAP_BIG_ENDIAN_WORD(Version);
      if(Version>=0x100 && Version <0x200)
        ok=true;
      nSides=*(BYTE*)(header+6);
      nTracks=*(BYTE*)(header+7);
      nTrackBytes=*(WORD*)(header+8);
      SWAP_BIG_ENDIAN_WORD(nTrackBytes);
      ImageData=(BYTE*)malloc(IMAGE_SIZE); //actual size
    }
#else
    ImageData=(BYTE*)malloc(IMAGE_SIZE); //max size
#endif
    if(ImageData)
    {
      fread(ImageData,1,IMAGE_SIZE,fCurrentImage); 
      if(!strncmp(DISK_EXT_STW,(char*)ImageData,3)) // it's STW
      {
#if defined(SSE_DISK_HD)
#ifdef SSE_DEBUG
        if(0);
#endif
#else
        Version=* (WORD*)(ImageData+4);
        SWAP_BIG_ENDIAN_WORD(Version);
        if(Version>=0x100 && Version <0x200)
          ok=true;
        nSides=*(BYTE*)(ImageData+6);
        nTracks=*(BYTE*)(ImageData+7);
        nTrackBytes=*(WORD*)(ImageData+8);
        SWAP_BIG_ENDIAN_WORD(nTrackBytes);
        if(nSides>2 || nTracks>88 || nTrackBytes>6800)
          ok=false;
#endif
#ifdef SSE_DEBUG
        // check meta-format
        else for(BYTE track=0;track<nTracks;track++)
        {
          for(BYTE side=0;side<nSides;side++)
          {
            int position=HEADER_SIZE
              +track*nSides*(TRACK_HEADER_SIZE+nTrackBytes*sizeof(WORD))
              +side*(TRACK_HEADER_SIZE+nTrackBytes*sizeof(WORD));
            if(strncmp("TRK",(char*)ImageData+position,3)
              ||  *(ImageData+position+3)!=side 
              ||  *(ImageData+position+3)!=side )
              ok=false;
          }//nxt side
        }//nxt track
        //ASSERT(ok);
        TRACE_LOG("Open STW %s, V%X S%d T%d B%d OK%d\n",path,Version,nSides,nTracks,nTrackBytes,ok); 
#endif
      }
    }
  }//if(fCurrentImage)
  if(!ok)
    Close();
  else 
  {
    FloppyDisk[Id].TrackBytes=nTrackBytes;
    FloppyDrive[Id].MfmManager=this;
  }
  return ok;
}


void TImageSTW::SetMfmData(WORD position,WORD mfm_data) {
  // must compute new starting point?
  if(position!=0xFFFF)
    ComputePosition(position);
  if(TrackData && Position<nTrackBytes)
  {
    TrackData[Position]=mfm_data;
    SWAP_BIG_ENDIAN_WORD(TrackData[Position]);
    if(!FloppyDisk[Id].ReadOnly)
      FloppyDisk[Id].WrittenTo=true;
    IncPosition();
  }
}


#if defined(SSE_DISK_AUTOSTW)
/*  Conversion functions STW <-> ST, MSA, DIM.
    Images can be converted on the fly on insertion and ejection.
    We use data and functions of FloppyDisk and FloppyDrive to access the
    ST/MSA/DIM image. Assume: such image is inserted in drive id.
    That way, we don't need to care for the differences between ST, MSA and
    DIM.
    The ST->STW side already existed as a Disk Manager right click action
    in previous versions.
*/

void wd1772_write_stw(BYTE id,BYTE data,TWD1772MFM* wd1772mfm,
                      TWD1772Crc* wd1772crc,int& p,int write_mode) {
  if(!write_mode)
    wd1772crc->Add(data);
  wd1772mfm->data=data;
  wd1772mfm->Encode((write_mode==1)?(TWD1772MFM::FORMAT_CLOCK)
    :(TWD1772MFM::NORMAL_CLOCK));
  if(write_mode==1)
    wd1772crc->Reset();
  ImageSTW[id].SetMfmData(0xFFFF,wd1772mfm->encoded);
  p++;
}


#define WD1772_WRITE(d) wd1772_write_stw(id,d,&wd1772mfm,&wd1772crc,p,0);
#define WD1772_WRITE_A1 wd1772_write_stw(id,0xA1,&wd1772mfm,&wd1772crc,p,1);
#define WD1772_WRITE_CRC(d) wd1772_write_stw(id,d,&wd1772mfm,&wd1772crc,p,2);


bool STtoSTW(BYTE id,char *dst_path) { // ST/MSA/DIM -> STW
  // id = drive which contains the image to convert (with FILE pointer)
  // dst_path = path to STW disk image to  create
  TWD1772MFM wd1772mfm;
  TWD1772Crc wd1772crc;
#if defined(SSE_DISK_HD)
  BYTE density=(FloppyDisk[id].SectorsPerTrack<=11) ? 1 : 2;
  bool ok=ImageSTW[id].Create(dst_path,density);
#else
  bool ok=ImageSTW[id].Create(dst_path);
#endif
  if(ok)
    ok=ImageSTW[id].Open(dst_path);
  for(BYTE track=0;ok && track<FloppyDisk[id].TracksPerSide;track++)
  {
    for(BYTE side=0;ok && side<FloppyDisk[id].Sides;side++)
    {
      if(!ImageSTW[id].LoadTrack(side,track))
      {
        TRACE_LOG("can't load track %d %d\n",side,track);
        ok=false;
      }
      ImageSTW[id].Position=0;
      int p=0;
      for(int i=0;i<FloppyDisk[id].PostIndexGap();i++)
        WD1772_WRITE(0x4E)
      BYTE sector;
      // write all sectors of this track/side
      for(BYTE sector2=1;ok && sector2<=FloppyDisk[id].SectorsPerTrack;sector2++)
      {
        //  We must use interleave 6 for 11 sectors, eg Pang -EMP
        sector=(FloppyDisk[id].SectorsPerTrack==11
          ? ((((sector2-1)*DISK_11SEC_INTERLEAVE)%11)+1) : sector2);
        if(FloppyDisk[id].SeekSector(side,track,sector,false,false))
        {
          TRACE_LOG("can't find sector %d %d %d\n",side,track,sector);
          break; // not in source, write nothing more
        }
        for(int i=0;i<(FloppyDisk[id].SectorsPerTrack==11?3:12);i++)
          WD1772_WRITE(0)
        for(int i=0;i<3;i++)
          WD1772_WRITE_A1
        // write sector ID
        WD1772_WRITE(0xFE)
        WD1772_WRITE(track)
        WD1772_WRITE(side)
        WD1772_WRITE(sector)
        WD1772_WRITE(2) //512 bytes
        WD1772_WRITE_CRC(wd1772crc.crc>>8)
        WD1772_WRITE_CRC(wd1772crc.crc&0xFF)
        for(int i=0;i<22;i++)
          WD1772_WRITE(0x4E)
        for(int i=0;i<12;i++) // 11 sectors too?
          WD1772_WRITE(0)
        for(int i=0;i<3;i++)
          WD1772_WRITE_A1
        // write sector data
        WD1772_WRITE(0xFB)
        BYTE b=0;
        for(int i=0;ok && i<512;i++)
        {
          if(fread(&b,1,1,FloppyDisk[id].f)!=1)
          {
            TRACE_LOG("fail read byte %d %d %d %d\n",side,track,sector,i);
            ok=false;
          }
          WD1772_WRITE(b)
        }
        WD1772_WRITE_CRC(wd1772crc.crc>>8)
        WD1772_WRITE_CRC(wd1772crc.crc&0xFF)
        WD1772_WRITE(0xFF) // so 1 byte gap fewer ?
        for(int i=0;i<(FloppyDisk[id].SectorsPerTrack==11?1-1:40-1);i++)
          WD1772_WRITE(0x4E)
      }//sector
      // pre-index gap: the rest
      while(p<FloppyDisk[id].TrackBytes)
        WD1772_WRITE(0x4E)
      //TRACE_LOG("STW track %d/%d written %d sectors %d bytes\n",side,track,sector,p);
    }//side
  }//track
  return ok;
}


bool STWtoST(BYTE id) { // STW -> ST/MSA/DIM
  // ImageSTW[id] contains a valid STW image that we convert back to ST
  // FloppyDisk[id] has the correct FILE pointer
  bool ok=(ImageSTW[id].ImageData!=NULL && FloppyDisk[id].f!=NULL);
  for(BYTE track=0;ok && track<FloppyDisk[id].TracksPerSide;track++)
  {
    for(BYTE side=0;ok && side<FloppyDisk[id].Sides;side++)
    {
      BYTE pdata[512]; // sector buffer
      for(BYTE sector=1;ok && sector<=FloppyDisk[id].SectorsPerTrack;sector++)
      {
        if(FloppyDisk[id].SeekSector(side,track,sector,false,false))
        {
          TRACE_LOG("sector %d %d %d not found\n",side,track,sector);
          ok=false; // not on dest image
        }
        if(!ImageSTW[id].GetSectorData(side,track,sector,pdata))
        {
          TRACE_LOG("STW %d can't retrieve sector %d %d %d\n",id,side,track,sector);
          ok=false;
        }
        else if(fwrite(&pdata,sizeof(BYTE),512,FloppyDisk[id].f)!=512)
        {
          TRACE_LOG("fail write sector %d %d %d\n",side,track,sector);
          ok=false; // read-only -> close STW image and leave
        }
      }//sector
    }//side
  }//track
  ImageSTW[id].Close();
  return ok;
}

#endif

#endif//#if defined(SSE_DISK_STW)
