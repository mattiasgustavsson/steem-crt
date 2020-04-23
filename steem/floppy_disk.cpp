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
FILE: floppy_disk.cpp
DESCRIPTION: Definitions for the Floppy Disk objects.
Functions to get ID fields and sector or track data of some image types.
Functions that can give positions relative to the index or the gaps for
some disk image types.
---------------------------------------------------------------------------*/


#include "pch.h"
#pragma hdrstop

#include <conditions.h>
#include <debug.h>
#include <floppy_disk.h>
#include <floppy_drive.h>
#include <computer.h>
#include <gui.h>
#include <acc.h>

#undef CURRENT_TRACK
#define CURRENT_TRACK (FloppyDrive[Id].track)

char *extension_list[NUM_EXT]={ "","ST","MSA","DIM","STT","STX","IPF",
"CTR","STG","STW","PRG","TOS","SCP","HFE"};

char buffer[5]=".XXX";


char *dot_ext(int i) { // is it ridiculous? do we reduce or add overhead?
  strcpy(buffer+1,extension_list[i]);
  return buffer;
}


TFloppyDisk::TFloppyDisk() {
#ifdef SSE_DEBUG
  current_byte=0xFFFF;
#endif
  Id=2; // temporary disk in temporary drive for properties
  Init();
  f=NULL;Format_f=NULL;PastiDisk=0;
  PastiBuf=NULL;
}


void TFloppyDisk::Init() {
  TrackBytes=DISK_BYTES_PER_TRACK;
}


WORD TFloppyDisk::BytePositionOfFirstId() { 
  return ( PostIndexGap() + ( (nSectors()<11)?12+3+1:3+3+1) );
}


WORD TFloppyDisk::BytesToID(BYTE &num) {
/*  Compute distance in bytes between current byte and desired ID
    identified by 'num' (sector)
    return 0 if it doesn't exist
    if num=0, assume next ID, num will contain sector index, 1-based
*/
  short bytes_to_id=0;
  
  const WORD my_current_byte=FloppyDrive[Id].BytePosition();

  if(FloppyDrive[Id].Empty())
    ;
  else
  {
    //here we assume normal ST disk image, sectors are 1...10
    WORD record_length=RecordLength();
    BYTE n_sectors=nSectors();
    WORD byte_first_id=BytePositionOfFirstId();
    WORD byte_target_id;
    // If we're looking for whatever next num, we compute it first
    if(!num)
    {
      num=(BYTE)((my_current_byte-byte_first_id)/record_length+1); // current, 1-based
      //if(((my_current_byte)%record_length)>byte_first_id) // only if past ID! (390)
      if(((my_current_byte)%record_length)>=byte_first_id) //v4: ProCopy Analyze
        num++; //next
      if(num>=n_sectors+1) // >=: Wipe-Out ; it's a special case, 6 sectors/track
        num=1; 
    }
    byte_target_id=byte_first_id+(num-1)*record_length;
    bytes_to_id=byte_target_id-my_current_byte;
    if(bytes_to_id<0) // passed it
      bytes_to_id+=TrackBytes; // next rev
  }
  ASSERT(bytes_to_id>=0);
  return (WORD)bytes_to_id;
}


WORD TFloppyDisk::HblsPerSector() {
  return nSectors()?(FloppyDrive[Id].HblsPerRotation()
    -FloppyDrive[Id].BytesToHbls(TrackGap()))/nSectors() : 0;
}


void TFloppyDisk::NextID(BYTE &RecordIdx,WORD &nHbls) {
  RecordIdx=0;
  nHbls=0;
  if(FloppyDrive[Id].Empty())
    return;
  WORD BytesToRun=BytesToID(RecordIdx); // 0 means first that comes
  if(RecordIdx) // RecordIdx is changed by BytesToID()
    RecordIdx--; // 0-basis
  nHbls=FloppyDrive[Id].BytesToHbls(BytesToRun);
}


BYTE TFloppyDisk::nSectors() { 
  BYTE nSects;
  if(STT_File)
  {
    TWD1772IDField IDList[30]; // much work each time, but STT rare
    nSects=GetIDFields(CURRENT_SIDE,CURRENT_TRACK,IDList);
  }
  else
    nSects=(BYTE)SectorsPerTrack;
  return nSects;
}


BYTE TFloppyDisk::PostIndexGap() {
  switch(nSectors()) {
  case 9:
#if defined(SSE_DISK_HD)
  case 18:
#endif
    return 60;
  case 10:
#if defined(SSE_DISK_HD)
  case 20:
#endif
    return 22; //?
  default:
    return 10;
  }
}


BYTE TFloppyDisk::PreDataGap() {
  BYTE gap=0;
  switch(nSectors()) {
  case 9:
  case 10: // with ID (7) and DAM (1)
#if defined(SSE_DISK_HD)
  case 18: case 20:
#endif
    gap=12+3+7+22+12+3+1; 
    break;
  case 11:
    gap= 3+3+7+22+12+3+1;
    break;
  }
  return gap;
}


BYTE TFloppyDisk::PostDataGap() {
  return (nSectors()<11)? 40 : 1;
}


WORD TFloppyDisk::PreIndexGap() {
  WORD gap=0;
  switch(nSectors()) {
  case 9:
#if defined(SSE_DISK_HD)
  case 18:
#endif
    gap=664+6; // 6256 vs 6250
    break;
  case 10:
#if defined(SSE_DISK_HD)
  case 20:
#endif
    gap=50+6+(60-22);
    break;
#if defined(SSE_DISK_HD)
  default:
#else
  case 11:
#endif
    gap=20;
    break;
  }
  return gap;
}


WORD TFloppyDisk::RecordLength() {
#if defined(SSE_DISK_HD)
  switch(nSectors()) {
    case 11: case 22:
      return 566;
    default:
      return 614;
  }
#else
  return (nSectors()<11)? 614 : 566;
#endif
}


BYTE TFloppyDisk::SectorGap() {
  return 2+PostDataGap()+PreDataGap();
}


WORD TFloppyDisk::TrackGap() {
  return PostIndexGap()+PreIndexGap();
}


BYTE TFloppyDisk::GetIDFields(int Side,int Track,TWD1772IDField *IDList) {
  if(FloppyDrive[Id].Empty()) 
    return 0;
  if(STT_File)
  {
    DWORD TrackStart=STT_TrackStart[Side][Track],Magic=0;
    WORD DataFlags;
    if(TrackStart==0) return 0;
    fseek(f,TrackStart,SEEK_SET);
    if(fread(&Magic,4,1,f)==0)
    {
      if(FloppyDrive[Id].ReinsertDisk()==0) return 0;
      if((TrackStart=STT_TrackStart[Side][Track])==0) return 0;
      fseek(f,TrackStart,SEEK_SET);
      fread(&Magic,4,1,f);
    }
    if(Magic!=MAKECHARCONST('T','R','C','K')) return 0;
    fread(&DataFlags,2,1,f);
    if(DataFlags & BIT_0)
    {       //Sectors
      DWORD Dummy;
      WORD Offset,Flags,NumSectors;
      fread(&Offset,2,1,f);
      fread(&Flags,2,1,f);
      fread(&NumSectors,2,1,f);
      for(int n=0;n<NumSectors;n++)
      {
        fread(&IDList[n].track,1,1,f);
        fread(&IDList[n].side,1,1,f);
        fread(&IDList[n].num,1,1,f);
        fread(&IDList[n].len,1,1,f);
        fread(&IDList[n].CRC[0],1,1,f);
        fread(&IDList[n].CRC[1],1,1,f);
        fread(&Dummy,4,1,f); // SectorOffset, SectorLen
      }
      return (BYTE)NumSectors;
    }
    else if(DataFlags & BIT_1)
    { //Raw track
    }
    return 0;
  }
  else
  {
    bool Format=0;
    if(Track<=FLOPPY_MAX_TRACK_NUM) 
      Format=TrackIsFormatted[Side][Track];
    if(Side>=int(Format?2:Sides))
      return 0;
    else if(Track>=int(Format?FLOPPY_MAX_TRACK_NUM+1:TracksPerSide))
      return 0;
    for(int n=0;n<int(Format?FormatMostSectors:SectorsPerTrack);n++)
    {
      IDList[n].track=BYTE(Track);
      IDList[n].side=BYTE(Side);
      // fake interleave '6' for 11 sectors: 1 7 2 8 3 9 4 10 5 11 6
      if(ADAT&&SectorsPerTrack==11) // not >= (superdisks)
        IDList[n].num=(BYTE)(1+(n*DISK_11SEC_INTERLEAVE)%SectorsPerTrack);
      else
        IDList[n].num=BYTE(1+n);
      switch(BytesPerSector) {
      case 128:  IDList[n].len=0; break;
      case 256:  IDList[n].len=1; break;
      case 1024: IDList[n].len=3; break;
      default:   IDList[n].len=2; break;
      }
      WORD CRC=0xffff;
      fdc_add_to_crc(CRC,0xa1);
      fdc_add_to_crc(CRC,0xa1);
      fdc_add_to_crc(CRC,0xa1);
      fdc_add_to_crc(CRC,0xfe);
      fdc_add_to_crc(CRC,IDList[n].track);
      fdc_add_to_crc(CRC,IDList[n].side);
      fdc_add_to_crc(CRC,IDList[n].num);
      fdc_add_to_crc(CRC,IDList[n].len);
      IDList[n].CRC[0]=HIBYTE(CRC);
      IDList[n].CRC[1]=LOBYTE(CRC);
    }
    return (BYTE)((Format)?FormatMostSectors:SectorsPerTrack);
  }
}


EasyStr TFloppyDisk::GetImageFile() {
  return ImageFile;
}


bool TFloppyDisk::OpenFormatFile() {
  if(FloppyDrive[Id].Empty()||f==NULL||ReadOnly||Format_f
    ||STT_File||FloppyDrive[Id].ImageType.Manager!=MNGR_STEEM)
    return 0;
  // The format file is just a max size ST file, any formatted tracks
  // go in here and then are merged with unformatted tracks when
  // the disk is removed from the drive
  FormatTempFile.SetLength(MAX_PATH);
  GetTempFileName(WriteDir,"FMT",0,FormatTempFile);
  // Create it
  Format_f=fopen(FormatTempFile,"wb");
  if(Format_f==NULL) 
    return 0;
  fclose(Format_f);
  SetFileAttributes(FormatTempFile,FILE_ATTRIBUTE_HIDDEN);
  Format_f=fopen(FormatTempFile,"r+b");
  if(Format_f==NULL) 
    return 0;
  char zeros[FLOPPY_MAX_BYTESPERSECTOR];
  ZeroMemory(zeros,sizeof(zeros));
  for(int Side=0;Side<2;Side++)
  {
    for(int Track=0;Track<=FLOPPY_MAX_TRACK_NUM;Track++)
    {
      for(int Sector=1;Sector<=FLOPPY_MAX_SECTOR_NUM;Sector++)
        fwrite(zeros,FLOPPY_MAX_BYTESPERSECTOR,1,Format_f);
    }
  }
  fflush(Format_f);
  return true;
}


bool TFloppyDisk::ReopenFormatFile() {
  if(FloppyDrive[Id].Empty()||f==NULL||ReadOnly||Format_f==NULL
    ||STT_File||FloppyDrive[Id].ImageType.Manager!=MNGR_STEEM)
    return 0;
  fclose(Format_f);
  Format_f=fopen(FormatTempFile,"r+b");
  if(Format_f) 
    return true;
  return 0;
}


// Seek in the disk image to the start of the required sector

#define LOGSECTION LOGSECTION_IMAGE_INFO

bool TFloppyDisk::SeekSector(int Side,int Track,int Sector,bool Format,
                             bool Freeboot/*=true*/) {
  if(Format_f==NULL) 
    Format=0;
  if(FloppyDrive[Id].Empty())
    return true;
  else if(Side<0||Track<0||Side>1)
  {
    DBG_LOG(EasyStr("FDC: Seek Failed - Side ")+Side+" track "+Track+" sector "+Sector+" - negative values!");
    return true;
  }
  else if(Side>=int(Format?2:Sides))
  {
    DBG_LOG(EasyStr("FDC: Seek Failed - Can't seek to side ")+Side);
    return true;
  }
  else if(Track>=int(Format?FLOPPY_MAX_TRACK_NUM+1:TracksPerSide))
  {
    DBG_LOG(EasyStr("FDC: Seek Failed - Can't seek to track ")+Track+" on side "+Side);
    return true;
  }
#if defined(SSE_DRIVE_FREEBOOT)
  if(Freeboot) // during emulation, not when manipulating disk images
  {
    if(FloppyDrive[DRIVE].single_sided&&Side==1)
      return true; // -> RNF
    if(FloppyDrive[DRIVE].freeboot)
      Side=1;
  }
#endif
  if(STT_File)
  {
    DWORD TrackStart=STT_TrackStart[Side][Track],Magic=0;
    WORD DataFlags;
    if(TrackStart==0) return true; // Track doesn't exist
    fseek(f,TrackStart,SEEK_SET);
    if(fread(&Magic,4,1,f)==0)
    {
      if(FloppyDrive[Id].ReinsertDisk()==0) 
        return true;
      if((TrackStart=STT_TrackStart[Side][Track])==0) return true;
      fseek(f,TrackStart,SEEK_SET);
      fread(&Magic,4,1,f);
    }
    if(Magic!=MAKECHARCONST('T','R','C','K')) return true;
    fread(&DataFlags,2,1,f);
    bool Failed=true;
    if(DataFlags & BIT_0)
    {       //Sectors
      WORD Offset,Flags,NumSectors;
      fread(&Offset,2,1,f);
      fread(&Flags,2,1,f);
      fread(&NumSectors,2,1,f);
      BYTE TrackNum,SideNum,SectorNum,LenIdx,CRC1,CRC2;
      WORD SectorOffset,SectorLen;
      for(int n=0;n<NumSectors;n++)
      {
        fread(&TrackNum,1,1,f);
        fread(&SideNum,1,1,f);
        fread(&SectorNum,1,1,f);
        fread(&LenIdx,1,1,f);
        fread(&CRC1,1,1,f);
        fread(&CRC2,1,1,f);
        fread(&SectorOffset,2,1,f);
        fread(&SectorLen,2,1,f);
        // I'm not sure but it is very possible changing sides during a disk operation
        // would cause it to immediately start reading the other side
        //SS: we don't do that for SCP etc. it would return garbage
        if(TrackNum==Track && SideNum==floppy_current_side()
          &&SectorNum==Sector && SectorLen!=0)
        {
          fseek(f,TrackStart+SectorOffset,SEEK_SET);
          BytesPerSector=SectorLen;
          Failed=0;
          break;
        }
      }
    }
    else if(DataFlags & BIT_1)
    { // Raw track data (with bad syncs)
    }
    LOG_ONLY(if(Failed) DBG_LOG(EasyStr("FDC: Seek Failed - Can't find sector ")+Sector+" in track "+Track+" on side "+Side); )
    return Failed;
  }
  else
  {
    if(Sector==0||Sector>int(Format?FLOPPY_MAX_SECTOR_NUM:SectorsPerTrack))
    {
      DBG_LOG(EasyStr("FDC: Seek Failed - Can't seek to sector ")+Sector+" of track "+Track+" on side "+Side);
      return true;
    }

    if(Format==0)
    {
      int HeaderLen=int(DIM_File?32:0);
      fseek(f,HeaderLen+(GetLogicalSector(Side,Track,Sector)*BytesPerSector),
        SEEK_SET);
    }
    else
      fseek(Format_f,GetLogicalSector(Side,Track,Sector,true)
        *FLOPPY_MAX_BYTESPERSECTOR,SEEK_SET);
    return false;  //no error!
  }
}


long TFloppyDisk::GetLogicalSector(int Side,int Track,int Sector,
                                    bool FormatFile) {
  if(FloppyDrive[Id].Empty()) 
    return 0;
  if(FormatFile==0||Format_f==NULL)
    return ((Track*Sides*SectorsPerTrack)+(Side*SectorsPerTrack)+(Sector-1));
  return (Track*2*FLOPPY_MAX_SECTOR_NUM)+(Side*FLOPPY_MAX_SECTOR_NUM)+(Sector-1);
}



int TFloppyDisk::GetRawTrackData(int Side,int Track) {
  if(STT_File)
  {
    DWORD TrackStart=STT_TrackStart[Side][Track],Magic;
    WORD DataFlags;
    if(TrackStart==0)
      return 0;
    fseek(f,TrackStart,SEEK_SET);
    if(fread(&Magic,4,1,f)==0)
    {
      if(FloppyDrive[Id].ReinsertDisk()==0)
        return 0;
      if((TrackStart=STT_TrackStart[Side][Track])==0)
        return 0;
      fseek(f,TrackStart,SEEK_SET);
      fread(&Magic,4,1,f);
    }
    if(Magic!=MAKECHARCONST('T','R','C','K'))
      return 0;
    fread(&DataFlags,2,1,f);
    if(DataFlags & BIT_0)
    { // Skip this section if it exists
      WORD Offset;
      fread(&Offset,2,1,f);
      fseek(f,TrackStart+Offset,SEEK_SET);
    }
    if(DataFlags & BIT_1)
    { //Raw
      WORD Offset,Flags,TrackDataOffset,TrackDataLen;
      fread(&Offset,2,1,f);
      fread(&Flags,2,1,f);
      fread(&TrackDataOffset,2,1,f);
      fread(&TrackDataLen,2,1,f);
      fseek(f,TrackStart+TrackDataOffset,SEEK_SET);
      return TrackDataLen;
    }
  }
  return 0;
}


//////////////////
// MFM MANAGERS //
//////////////////

void  TImageMfm::ComputePosition(WORD position) {
  // when we start reading/writing, where on the disk?
  //ASSERT(FloppyDisk[Id].TrackBytes); 
  if(FloppyDisk[Id].TrackBytes) // good old div /0 crashes the PC like it did the ST
    position=position%FloppyDisk[Id].TrackBytes; // 0-~6256, safety
  //TRACE_LOG("old position %d new position %d\n",Position,position);
  Position=FloppyDisk[Id].current_byte=position;
}


void TImageMfm::IncPosition() {
  //ASSERT(FloppyDisk[Id].TrackBytes);
  if(FloppyDisk[Id].TrackBytes)
    Position=(Position+1)%FloppyDisk[Id].TrackBytes;
  if(!Position)
    FloppyDrive[Id].IndexPulse(true);
}


