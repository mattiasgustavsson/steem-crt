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

FILE: floppy_drive.cpp
DESCRIPTION: Floppy drive object. 
When a disk is spinning, we get an index pulse (IP) on each rotation, which
is sent to the fdc.
The fdc and the psg command the drive, to start or stop the motor, step, 
read, or write. 
The image type manager and type are stored in this object. When there's no
disk, the manager is MNGR_STEEM or MNGR_WD1772 according to option MFM.
Steem's drive sound is handled here.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <conditions.h>
#include <steemh.h>
#include <run.h>
#include <gui.h>
#include <options.h>
#include <display.h>
#include <debug.h>
#include <floppy_drive.h>
#include <computer.h>
#include <harddiskman.h>
#include <diskman.h>
#include <archive.h>
#include <dataloadsave.h>
#include <mymisc.h>
#include <translate.h>
#include <notifyinit.h>
#ifdef WIN32
#include <pasti/pasti.h>
#endif

#if defined(SSE_DRIVE_SOUND)

#include "various/sound.h" //struct TWavFileFormat

char* drive_sound_wav_files[]={ "drive_startup.wav","drive_spin.wav",
                                "drive_click.wav","drive_seek.wav"};

EasyStr DriveSoundDir[2];

#ifdef WIN32
IDirectSoundBuffer *Sound_Buffer[2][TSF314::NSOUNDS];
#endif

#endif

#define LOGSECTION LOGSECTION_FDC


TSF314::TSF314() {
  Init();
}


TSF314::~TSF314() {
  //RemoveDisk(); //trouble?
}


#if defined(SSE_DISK_GHOST)

bool TSF314::CheckGhostDisk(bool write) {
  //ASSERT(OPTION_GHOST_DISK);
  if(!ghost) // need to open ghost image?
  {
    EasyStr STGPath=FloppyDisk[Id].GetImageFile();
    STGPath+=dot_ext(EXT_STG); 
    if(write || Exists(STGPath))
    {
      if(GhostDisk[Id].Open(STGPath.Text))
        ghost=1; 
    }
  }
  return ghost;
}

#endif


void TSF314::Init() {
#if defined(SSE_DRIVE_SOUND)
  for(int i=0;i<NSOUNDS;i++)
    Sound_Buffer[Id][i]=NULL;
  Sound_Volume=5000; 
#endif
  SectorChecksum=0;
  m_DiskInDrive=0;
  ImageType.Manager=(BYTE)((pasti_active) ? MNGR_PASTI
    : (OPTION_AUTOSTW ? MNGR_WD1772 : MNGR_STEEM));
  MfmManager=NULL;
}


void TSF314::Restore(BYTE myid) {
  //ASSERT(!(ImageType.RealExtension>=NUM_EXT));
  Id=myid;
  if(Empty())
  {
    //TRACE_LOG("drive %c empty\n",'A'+Id);
    ImageType.Extension=ImageType.RealExtension=0;
    ImageType.Manager=(BYTE)((pasti_active) ? MNGR_PASTI
      : (OPTION_AUTOSTW ? MNGR_WD1772 : MNGR_STEEM));
  }
  if(ImageType.Extension>=NUM_EXT)
    ImageType.Extension=0;
}


void TSF314::UpdateAdat() {
/*  ADAT=accurate disk access times
    This is defined so: Steem slow (original ADAT) or STX or Caps (IPF,CTR)
    or SCP.
*/
  adat= (!floppy_instant_sector_access && ImageType.Manager==MNGR_STEEM
    || ImageType.Extension==EXT_STX || ImageType.Manager==MNGR_CAPS
    || ImageType.Manager==MNGR_WD1772
      &&(!floppy_instant_sector_access || ImageType.Extension==EXT_SCP));
  REFRESH_STATUS_BAR;
}


WORD TSF314::BytePosition() { //TODO
  WORD position=0;
#ifdef SSE_DISK_STW
/*  This assumes constant bytes/track (some protected disks have more)
    This should be 0-6255
    This is independent of #sectors
    This is independent of disk type
    This is based on Index Pulse, that is sent by the drive 
*/
  if(ImageType.Manager==MNGR_WD1772 && CyclesPerByte())
  {
    position=(WORD)((ACT-time_of_last_ip)/cycles_per_byte);
    if(position>=FloppyDisk[Id].TrackBytes)
    {
      // argh! IP didn't occur yet
      position=FloppyDisk[Id].TrackBytes
        -(WORD)((time_of_next_ip-ACT)/cycles_per_byte);
      time_of_last_ip=ACT;
      if(position>=FloppyDisk[Id].TrackBytes)
        position=0; //some safety
    }
  }
  else
#endif
    position=HblsToBytes( hbl_count% HblsPerRotation() );
  return position;

}


WORD TSF314::BytesToHbls(int bytes) {
  return FloppyDisk[Id].TrackBytes ? (WORD)(HblsPerRotation()*bytes
    /FloppyDisk[Id].TrackBytes) : 0;
}


DWORD TSF314::HblsAtIndex() { // absolute
  return (hbl_count/HblsPerRotation())*HblsPerRotation();
}


WORD TSF314::HblsNextIndex() { // relative
  return HblsPerRotation()-hbl_count%HblsPerRotation();
}


WORD TSF314::HblsPerRotation() {
  WORD hbls=HBL_PER_SECOND/(DRIVE_RPM/60);
  return (hbls) ? hbls : 1;  // not 0, it's a divisor
}


WORD TSF314::HblsToBytes(WORD hbls) {
  return FloppyDisk[Id].TrackBytes*hbls/HblsPerRotation();
}


/*  The Fdc emu written for HFE, SCP, STW images uses events following
    a spinning drive.
    Drive events are index pulse (IP), reading or writing a byte.
*/

int TSF314::CyclesPerByte() { // normally 256
  int cycles=CpuNormalHz; // per second  
  cycles/=DRIVE_RPM/60; // per rotation (300/60 = 5)
  cycles/=FloppyDisk[Id].TrackBytes; // per byte
  if(!ADAT)
    cycles=DRIVE_FAST_CYCLES_PER_BYTE; //hack!
#if defined(SSE_MEGASTE)
  if(IS_MEGASTE && MegaSte.FdHd)
    cycles/=2;
#endif
  cycles_per_byte=cycles; // save
  //ASSERT(cycles);
  return cycles;
}

/*  Function called by event manager in run.cpp based on preset timing
    or by the image object when we're  going through the last byte.
    Motor must be on, a floppy disk must be inside.
    The drive must be selected for the pulse to go to the Fdc.
    If conditions are not met, we put timing of next check 1 second away,
    because it seems to be Steem's way. TODO: better way with less checking,
    though 1/sec is OK!
*/

void TSF314::IndexPulse(bool image_triggered) {

  //ASSERT(Id==0||Id==1);
  time_of_next_ip=time_of_next_event+n_cpu_cycles_per_second; 

  //  timeout is normally 1.5s
  if(fdc_spinning_up && FloppyDrive[Id].Empty() || (!motor))
    Fdc.TimeOut++;

  if(ImageType.Manager!=MNGR_WD1772||FloppyDrive[Id].Empty()||!motor)
    return; 

  time_of_last_ip=time_of_next_event; // record timing

  // Make sure that we always end track at the same byte when R/W
  if(!reading && !writing 
#if defined(SSE_DISK_SCP)
    || IMAGE_SCP
#endif
    || FloppyDisk[Id].current_byte>=FloppyDisk[Id].TrackBytes-1) // 0 - n-1
    FloppyDisk[Id].current_byte=0;

  // Program next event, at next IP or in 1 sec (more?)
  //ASSERT(motor);
#if defined(SSE_DISK_SCP)
/*  We set up a timing for next IP, but if the drive is reading there are 
    chances the SCP object will trigger IP itself at the end of the track.
    This is a place where we could have bug reports.
*/
  if(IMAGE_SCP // an image is inside
    && ImageSCP[DRIVE].track_header.TDH_TABLESTART[ImageSCP[DRIVE].rev].\
    TDH_DURATION) // a track is loaded... (else it's 0 and it hangs)
  {
    time_of_next_ip=time_of_last_ip 
      + ImageSCP[DRIVE].track_header.TDH_TABLESTART[ImageSCP[DRIVE].rev].\
      TDH_DURATION/5;
    //TRACE_LOG("SCP pos %d/%d next IP in %d cycles\n",ImageSCP[DRIVE].Position,ImageSCP[DRIVE].nBits-1,ImageSCP[DRIVE].track_header.TDH_TABLESTART[ImageSCP[DRIVE].rev].TDH_DURATION/5);
  }
  else
#endif
  {
    if(!ADAT)
      // make it longer or it may come before the Fdc event
      time_of_next_ip=time_of_last_ip+CyclesPerByte()*FloppyDisk[Id].TrackBytes 
        *DRIVE_FAST_IP_MULTIPLIER;
    else
      time_of_next_ip=time_of_last_ip+CyclesPerByte()*FloppyDisk[Id].TrackBytes;
  }

  //ASSERT(time_of_next_ip-time_of_last_ip>0);

  // send pulse to Fdc
  if(Psg.CurrentDrive()==Id)
    Fdc.OnIndexPulse(Id,image_triggered); // transmitting image_triggered
}


void TSF314::Motor(bool state) {
/*  If we're starting the motor, we must program time of next IP.
    We start from last position or from a new random one.
*/

  if(num_connected_floppies==Id)
    state=false; // no drive

#ifdef SSE_DEBUG
  //if(state!=(bool)motor && FloppyDrive[Id].NotEmpty() )   //empty is never updated...
  {
    TRACE_LOG("Drive %c: motor %s\n",'A'+Id,state?"on":"off");
  }
#endif
  
  if(ImageType.Manager!=MNGR_WD1772)
    ; //TODO
  else if(motor && !state) //stopping - record position
    FloppyDisk[Id].current_byte=(BytePosition())%FloppyDisk[Id].TrackBytes; 
  else if(!motor && state) // starting
  {
    WORD bytes_to_next_ip= (FloppyDisk[Id].current_byte<FloppyDisk[Id].TrackBytes) 
      ? FloppyDisk[Id].TrackBytes-FloppyDisk[Id].current_byte : rand()%FloppyDisk[Id].TrackBytes;
    time_of_next_ip=ACT + bytes_to_next_ip * CyclesPerByte();
  }
  motor=state;
}


void TSF314::Read() {
/*  For Read() and Write():
    We "sync" on data in the beginning of a sequence.
    After that, we do each successive byte until the command is
    finished. That way, we won't lose any byte, and we add
    2 events/scanline only when reading.
*/
  //ASSERT(!writing);
  //ASSERT(IMAGE_STW || IMAGE_SCP || IMAGE_HFE); // only for those now
  //ASSERT(Id==DRIVE);
  //it works but side could change again in the interval
  if(FloppyDisk[Id].current_side!=CURRENT_SIDE && ImageType.Manager==MNGR_WD1772)
    MfmManager->LoadTrack(CURRENT_SIDE,CURRENT_TRACK);

#if defined(SSE_DISK_SCP) || defined(SSE_DISK_HFE)
  bool new_position=!reading;
#endif

  if(!reading || FloppyDisk[Id].current_byte>=FloppyDisk[Id].TrackBytes-1)
  {
#if defined(SSE_DISK_SCP)
/*  We should refactor this so that also STW images trigger IP, but my
    first attempt didn't work at all so...
*/
    if(!reading || !(IMAGE_SCP))
      FloppyDisk[Id].current_byte=BytePosition();
    else
      FloppyDisk[Id].current_byte++;
    reading=true; 
#else
    reading=true; 
    FloppyDisk[Id].current_byte=BytePosition();
#endif
//    TRACE_LOG("Start reading at byte %d\n",FloppyDisk[Id].current_byte);
  }
  else // get next byte regardless of timing
    FloppyDisk[Id].current_byte++;

  if(ImageType.Manager==MNGR_WD1772)
  {
    Fdc.Mfm.encoded=MfmManager->GetMfmData(new_position
      ?FloppyDisk[Id].current_byte:0xffff);
  }
#if defined(SSE_DRIVE_FREEBOOT)
  if(single_sided && CURRENT_SIDE==1)
    Fdc.Mfm.encoded=rand()%0xFFFF;
#endif
  // set up next byte event
#if defined(SSE_DISK_SCP)
  if(IMAGE_SCP)
    ; // it is done by SCP
  else
#endif
  if(FloppyDisk[Id].current_byte<=FloppyDisk[Id].TrackBytes)
  {
    Fdc.update_time=time_of_last_ip+cycles_per_byte*(FloppyDisk[Id].current_byte+1);
    if(Fdc.update_time-ACT<0)
      Fdc.update_time=ACT+cycles_per_byte;
  }
}


void TSF314::Step(int direction) {
#if defined(SSE_DRIVE_SOUND)
  if(OPTION_DRIVE_SOUND)
    Sound_Step();
#endif
  if(direction && track<DRIVE_MAX_CYL)
    track++;
  else if(track)
    track--;
  //TRACE("STEP %d\n",track);
  Fdc.Lines.track0=(track==0 && num_connected_floppies!=Id);
  if(Fdc.Lines.track0)
    Fdc.str|=FDC_STR_T0; // doing it here?
  CyclesPerByte();  // compute - should be the same every track but...
  //TRACE_LOG("Drive %d Step d%d new track: %d\n",Id,direction,track[Id]);
}


void TSF314::Write() {
  //ASSERT(IMAGE_STW||IMAGE_SCP||IMAGE_HFE); // only for those now
  //ASSERT(Id==DRIVE);
  //ASSERT(FloppyDisk[Id].current_side==CURRENT_SIDE);
  //ASSERT(FloppyDisk[Id].current_track==CURRENT_TRACK);

#if defined(SSE_DISK_SCP) || defined(SSE_DISK_HFE)
  bool new_position=!writing;
#endif

  if(!writing || FloppyDisk[Id].current_byte>=FloppyDisk[Id].TrackBytes-1)
  {
    if(reading && FloppyDisk[Id].current_byte<FloppyDisk[Id].TrackBytes-1)
      FloppyDisk[Id].current_byte++;
    else
      FloppyDisk[Id].current_byte=BytePosition();
    writing=true; 
    reading=false;
//    TRACE_LOG("Start writing at byte %d\n",FloppyDisk[Id].current_byte);
  }
  else
    FloppyDisk[Id].current_byte++;
#if defined(SSE_DRIVE_FREEBOOT)
  if(single_sided && CURRENT_SIDE==1)
    ; else 
#endif
  if(ImageType.Manager==MNGR_WD1772)
  {
    MfmManager->SetMfmData(new_position?FloppyDisk[Id].current_byte:0xffff,
      Fdc.Mfm.encoded);
  }

  // set up next byte event
  if(FloppyDisk[Id].current_byte<=FloppyDisk[Id].TrackBytes)
    Fdc.update_time=time_of_last_ip+cycles_per_byte*(FloppyDisk[Id].current_byte+1);
  if(Fdc.update_time-ACT<0)
    Fdc.update_time=ACT+cycles_per_byte;
}


//#pragma warning (disable: 4701) //MSA vars
#undef LOGSECTION
#define LOGSECTION LOGSECTION_IMAGE_INFO

int TSF314::SetDisk(EasyStr FilePath,EasyStr CompressedDiskName,
                                TBpbInfo *pDetectBPB,TBpbInfo *pFileBPB) {
  TRACE_LOG("%c: SetDisk %s\n",'A'+Id,FilePath.c_str());
/*  Note that the drive may still be spinning when a floppy is removed
    or inserted -> that state mustn't be changed (Braindamage)
    This function is rather unoptimised but performance doesn't matter here,
    if efforts should be made, it could be toward smaller code footprint.
*/
  if(Exists(FilePath)==0)
    return FIMAGE_FILEDOESNTEXIST;
#ifdef WIN32    
  TNotify myNotify(T("Disk operation"));
#endif
  RemoveDisk(); // remove current disk first thing
  EasyStr OriginalFile=FilePath,NewZipTemp;
  FloppyDisk[Id].ReadOnly=
    ((GetFileAttributes(FilePath)&FILE_ATTRIBUTE_READONLY)!=0);
  Str Ext;
  bool ST=0,MSA=0,STT=0,DIM=0,f_PastiDisk=0;
  bool IPF=0,CTR=0,SCP=0,STW=0,PRG=0,TOS,HFE=0;
  char *dot=strrchr(FilePath,'.');
  if(dot) 
    Ext=dot+1;
  int Type=ExtensionIsDisk(dot);
  // NewDiskInZip will be blank for default disk, RealDiskInZip will be the
  // actual name of the file in the zip that is a disk image
  EasyStr NewDiskInZip,RealDiskInZip;
  if(Type==DISK_COMPRESSED)
  {
    int HOffset=-1;
    bool CorruptZip=true;
    if(zippy.first(FilePath)==0)
    {
      CorruptZip=0;
      do
      {
        EasyStr fn=zippy.filename_in_zip();
        TRACE_LOG("File in zip %s\n",fn.c_str());
        Type=FileIsDisk(fn); // SS this changes Type
        if(Type==DISK_UNCOMPRESSED||Type==DISK_PASTI)
        {
          if(CompressedDiskName.Empty()||IsSameStr_I(CompressedDiskName,
            fn.Text))
          {
            // Blank DiskInZip name means default disk (first in zip)
            MSA=has_extension(fn,DISK_EXT_MSA);
            ST=has_extension(fn,DISK_EXT_ST);
            if(Type==DISK_PASTI)
            {
              TRACE_LOG("Disk in %c (%s) is managed by Pasti.dll\n",'A'+Id,fn.Text);
              f_PastiDisk=true;
            }
            else
            {
              STT=has_extension(fn,DISK_EXT_STT);
              DIM=has_extension(fn,DISK_EXT_DIM);
#if defined(SSE_DISK_CAPS)
              IPF=has_extension(fn,DISK_EXT_IPF);
              CTR=has_extension(fn,DISK_EXT_CTR);
#endif
#if defined(SSE_DISK_SCP)
              SCP=has_extension(fn,DISK_EXT_SCP);
#endif
#if defined(SSE_DISK_STW)
              STW=has_extension(fn,DISK_EXT_STW);
#endif
#if defined(SSE_DISK_HFE)
              HFE=has_extension(fn,DISK_EXT_HFE);
#endif
#if defined(SSE_TOS_PRG_AUTORUN)
              PRG=OPTION_PRG_SUPPORT && has_extension(fn,DISK_EXT_PRG);
              TOS=OPTION_PRG_SUPPORT && has_extension(fn,DISK_EXT_TOS);
#endif
            }
            HOffset=zippy.current_file_offset;
            NewDiskInZip=CompressedDiskName;
            RealDiskInZip=fn.Text;
            break;
          }
        }
      } while(zippy.next()==0);
    }
    zippy.close(); // SS archive always opened twice
    if(HOffset!=-1)
    {
      NewZipTemp.SetLength(MAX_PATH);
      GetTempFileName(WriteDir,"ZIP",0,NewZipTemp);
      if(zippy.extract_file(FilePath,HOffset,NewZipTemp,true /*bool hide*/))
      {
        TRACE_LOG("Failed to extract%s\n",NewZipTemp.c_str());
        DeleteFile(NewZipTemp);
        return FIMAGE_WRONGFORMAT;
      }
      FilePath=NewZipTemp;
      if(FloppyArchiveIsReadWrite)
        FloppyDisk[Id].ReadOnly=0;
      else
        FloppyDisk[Id].ReadOnly=true;
    }
    else
    {
      if(CorruptZip) 
        return FIMAGE_CORRUPTZIP;
      return FIMAGE_NODISKSINZIP;
    }
  }
  else if(Type==DISK_PASTI)
  {
    f_PastiDisk=true;
  }
  else if(Type==DISK_IS_CONFIG)
  {
    TConfigStoreFile CSF; //on the stack
    bool ok=CSF.Open(FilePath);
    if(ok)
    {
      LoadAllDialogData(false,"",NULL,&CSF);
      ROMFile=CSF.GetStr("Machine","ROM_File",ROMFile);
      // add current TOS path if necessary
      if(strchr(ROMFile.Text,SLASHCHAR)==NULL) // no slash = no path
      {
        EasyStr tmp=OptionBox.TOSBrowseDir+SLASH+ROMFile;
        ROMFile=tmp;
      }
      OptionBox.NewROMFile=ROMFile;
      CSF.Close();
      reset_st(RESET_COLD|RESET_STOP|RESET_CHANGESETTINGS|RESET_BACKUP);
      SetForegroundWindow(StemWin);
    }
    CSF.Close();
    return (ok) ? FIMAGE_IS_CONFIG : FIMAGE_WRONGFORMAT;
  }
  else if(Type==0)
  {
    TRACE_LOG("Disk type 0\n");
    return FIMAGE_WRONGFORMAT;
  }
  else
  {
    ST=IsSameStr_I(Ext,DISK_EXT_ST);
    MSA=IsSameStr_I(Ext,DISK_EXT_MSA);
    STT=IsSameStr_I(Ext,DISK_EXT_STT);
    DIM=IsSameStr_I(Ext,DISK_EXT_DIM);
#if defined(SSE_DISK_CAPS)
    IPF=IsSameStr_I(Ext,DISK_EXT_IPF);
    CTR=IsSameStr_I(Ext,DISK_EXT_CTR);
#endif
#if defined(SSE_DISK_SCP)
    SCP=IsSameStr_I(Ext,DISK_EXT_SCP);
#endif
#if defined(SSE_DISK_STW)
    STW=IsSameStr_I(Ext,DISK_EXT_STW);
#endif
#if defined(SSE_TOS_PRG_AUTORUN)
    PRG=OPTION_PRG_SUPPORT && IsSameStr_I(Ext,DISK_EXT_PRG);
    TOS=OPTION_PRG_SUPPORT && IsSameStr_I(Ext,DISK_EXT_TOS);
#endif
#if defined(SSE_DISK_HFE)
    HFE=IsSameStr_I(Ext,DISK_EXT_HFE);
#endif
  }
  int drive=-1;
  if(this==&FloppyDrive[0]) 
    drive=0;
  if(this==&FloppyDrive[1]) 
    drive=1;
  if(drive==-1) 
    f_PastiDisk=0; // Never use pasti for extra drives
  if(this==&FloppyDrive[2]) 
    drive=2;
  ASSERT(drive!=-1);
  if(f_PastiDisk)
  {
#if USE_PASTI
    int Ret=0;
    if(drive>=0&&hPasti)
    {
      FILE *nf=fopen(FilePath,"rb");
      if(nf)
      {
        FloppyDisk[Id].PastiBufLen=GetFileLength(nf);
        FloppyDisk[Id].PastiBuf=new BYTE[FloppyDisk[Id].PastiBufLen+16];
        fread(FloppyDisk[Id].PastiBuf,1,FloppyDisk[Id].PastiBufLen,nf);
        fclose(nf);
        Str RealFile=FilePath;
        if(NewZipTemp.NotEmpty()) 
          RealFile=RealDiskInZip;
        struct pastiDISKIMGINFO pdi;
        pdi.imgType=PASTI_ITPROT;
        ImageType.Manager=MNGR_PASTI;
        if(has_extension(RealFile,extension_list[EXT_ST])) 
        {
          pdi.imgType=PASTI_ITST;
          ImageType.Extension=EXT_ST;
        }
        else if(has_extension(RealFile,extension_list[EXT_MSA])) 
        {
          pdi.imgType=PASTI_ITMSA;
          ImageType.Extension=EXT_MSA;
        }
        else
          ImageType.Extension=EXT_STX;
        if(pdi.imgType==PASTI_ITPROT && NewZipTemp.Empty())
          pdi.mode=PASTI_LDFNAME;
        else
          pdi.mode=PASTI_LDMEM;
        pdi.fileName=RealFile;
        pdi.fileBuf=FloppyDisk[Id].PastiBuf;
        pdi.fileLength=FloppyDisk[Id].PastiBufLen;
        pdi.bufSize=FloppyDisk[Id].PastiBufLen+16;
        pdi.bDirty=0;
        BOOL bMediachDelay=TRUE;
        if(LITTLE_PC==rom_addr)
          bMediachDelay=FALSE;
        if(pasti->ImgLoad(drive,FloppyDisk[Id].ReadOnly,bMediachDelay,
          ABSOLUTE_CPU_TIME,&pdi)==FALSE) 
          Ret=FIMAGE_CANTOPEN;
      }
      else
        Ret=FIMAGE_CANTOPEN;
    }
    else
      Ret=FIMAGE_WRONGFORMAT;
    if(Ret)
    {
      delete[] FloppyDisk[Id].PastiBuf;
      FloppyDisk[Id].PastiBuf=NULL;
      if(NewZipTemp.NotEmpty()) 
        DeleteFile(NewZipTemp);
      return Ret;
    }
#endif
  }
#if defined(SSE_DISK_CAPS)
  else if(CAPSIMG_OK&&(CTR||IPF))
  {
    if(drive==-1) // no extra drive
      return FIMAGE_WRONGFORMAT;
    CapsImageInfo img_info;
    if(Caps.InsertDisk(drive,FilePath,&img_info))
      return FIMAGE_WRONGFORMAT;
    ImageType.Manager=MNGR_CAPS;
    ImageType.Extension=(BYTE)(IPF ? EXT_IPF : EXT_CTR);
#if defined(SSE_DISK_HD) // check if HD disk
    Caps.Drive[drive].side=0;
    Caps.Drive[drive].track=0;
    Caps.CallbackTRK(NULL,0); // parameters not used
    FloppyDisk[drive].TrackBytes=DISK_BYTES_PER_TRACK;
    if(Caps.Drive[drive].tracklen>20000) // in dwords, 12000+ for DD
      FloppyDisk[drive].TrackBytes*=2;
#endif
  }
#endif
#if defined(SSE_DISK_SCP)
  else if(SCP)
  {
    if(drive==-1||!ImageSCP[drive].Open(FilePath)) // no extra drive
      return FIMAGE_WRONGFORMAT;
    FloppyDisk[drive].TrackBytes=ImageSCP[drive].nBytes;
#if defined(SSE_DISK_HD) // check if HD disk
    ImageSCP[drive].LoadTrack(0,0,0);
    if(ImageSCP[drive].nBits>60000) // DD disks have more than 40000 bits, less than 50000
      FloppyDisk[drive].TrackBytes*=2;
#endif
    ImageType.Manager=MNGR_WD1772;
    ImageType.Extension=EXT_SCP;
    reading=writing=0;
  }
#endif
#if defined(SSE_DISK_STW)
  else if(STW)
  {
    if(!ImageSTW[Id].Open(FilePath)) // extra drive OK
      return FIMAGE_WRONGFORMAT;
    ImageType.Manager=MNGR_WD1772;
    ImageType.Extension=EXT_STW;
    reading=writing=0;
    BYTE pdata[512]; // sector buffer
    if(ImageSTW[Id].GetSectorData(0,0,1,pdata)) // boot sector for disk properties
    {
      FloppyDisk[Id].BytesPerSector=*(WORD*)(pdata+0xB);
      WORD total_sectors=*(WORD*)(pdata+0x13);
      FloppyDisk[Id].SectorsPerTrack=*(WORD*)(pdata+0x18);
      FloppyDisk[Id].Sides=*(WORD*)(pdata+0x1A);
      if(FloppyDisk[Id].SectorsPerTrack && FloppyDisk[Id].Sides)
        FloppyDisk[Id].TracksPerSide
          =(total_sectors/FloppyDisk[Id].SectorsPerTrack)/FloppyDisk[Id].Sides;
      FloppyDisk[Id].DiskFileLen=total_sectors*FloppyDisk[Id].BytesPerSector;
      FloppyDisk[Id].ValidBPB=( (FloppyDisk[Id].Sides==1||FloppyDisk[Id].Sides==2)
        && FloppyDisk[Id].SectorsPerTrack>5 && FloppyDisk[Id].SectorsPerTrack<12
        && total_sectors>359); // guess
      //TRACE("found %d %d %d %d\n",total_sectors,FloppyDisk[Id].Sides,FloppyDisk[Id].TracksPerSide,FloppyDisk[Id].SectorsPerTrack);
    }
#if 0
    // using Steem to convert, because HxC software counts too many bytes
    // see just above
    // this code is only compiled on demand, it's no Steem feature
    if(drive==1&&FloppyDrive[0].ImageType.Extension==EXT_HFE)
    {
      TRACE("Converting HFE to STW...\n");
      for(int si=0;si<2;si++)
        for(int tr=0;tr<ImageHFE[0].file_header.number_of_track;tr++)
        {
          ImageHFE[0].LoadTrack(si,tr);
          ImageSTW[1].LoadTrack(si,tr);
          for(int by=0;by<FloppyDisk[1].TrackBytes;by++)
          {
            WORD mfm=ImageHFE[0].GetMfmData(by);
            ImageSTW[1].SetMfmData(by,mfm);
          }
        }
      WrittenTo=true;
      ImageSTW[1].Close();
    }
#endif
  }
#endif//stw
#if defined(SSE_DISK_HFE)
  else if(HFE)
  {
    if(drive==-1||!ImageHFE[drive].Open(FilePath))
      return FIMAGE_WRONGFORMAT;
    FloppyDisk[drive].TrackBytes=DISK_BYTES_PER_TRACK; //default
#if defined(SSE_DISK_HD) // check if HD disk
    if(ImageHFE[drive].file_header->floppyinterfacemode==ATARIST_HD_FLOPPYMODE)
                            // or ->bitRate==500)
      FloppyDisk[drive].TrackBytes*=2;
#endif
    ImageType.Manager=MNGR_WD1772;
    ImageType.Extension=EXT_HFE;
    reading=writing=0;
  }
#endif//hfe
#if defined(SSE_TOS_PRG_AUTORUN)
/*  v3.7
    Support for PRG and TOS files
    Not disk images but single PRG or TOS files may be selected.
    In that case we copy to harddisk Z: and boot from it.
    This approach minimises RAM/code use.
*/
  else if(PRG||TOS)
  {
    SSEConfig.old_DisableHardDrives=HardDiskMan.DisableHardDrives;
    HardDiskMan.DisableHardDrives=false; // or mount path is wrong
    HardDiskMan.update_mount();
    Str PrgPath=mount_path[AUTORUN_HD]; // Z: (2+'Z'-'C')
    TRACE_LOG("PRG/TOS, path of disk %c: is %s\n",AUTORUN_HD+'A',PrgPath.Text);
    if(!strncmp(PrgPath.Rights(3),"PRG",3)) //our dedicated folder
    {
      FloppyDrive[drive].ImageType.Manager=MNGR_PRG; //and drive not empty
      FloppyDrive[drive].ImageType.Extension=(BYTE)(TOS ? EXT_TOS : EXT_PRG);
      Str NewPath,AutoPath;
      AutoPath=PrgPath+SLASH+"AUTO"+SLASH+"AUTORUN.PRG";
      DeleteFile(AutoPath.Text); // anyway
      if(TOS||tos_version<=0x102||tos_version>=0x200)
      {
        // we'll use AUTO, provided in PRG folder
        NewPath=AutoPath;
      }
      else
      {
        // we'll use DESKTOP.INF, provided in PRG folder
        NewPath=PrgPath+SLASH+"AUTORUN.PRG";
        DeleteFile(NewPath.Text);
      }
      TRACE_LOG("copy %s to %s\n",FilePath.Text,NewPath.Text);
      CopyFile(FilePath.Text,NewPath.Text,FALSE); // there's no CopyFile in Unix
    }//if
    else
      HardDiskMan.DisableHardDrives=(SSEConfig.old_DisableHardDrives!=0);
  }
#endif//prg  
  else // regular disk images
  {
    FloppyDisk[drive].TrackBytes=DISK_BYTES_PER_TRACK; // should be 6256
    ImageType.Manager=MNGR_STEEM;
    if(MSA)
      ImageType.Extension=EXT_MSA;
    else if(DIM)
      ImageType.Extension=EXT_DIM;
    else if(STT)
      ImageType.Extension=EXT_STT;
    else
      ImageType.Extension=EXT_ST;
    // Open for read for an MSA (going to convert to ST and write to that)
    // and if the file is read-only, otherwise open for update
    FILE *nf=fopen(FilePath,LPSTR((MSA||FloppyDisk[Id].ReadOnly)?"rb":"r+b"));
    if(nf==NULL)
    {
      if(NewZipTemp.NotEmpty()) 
        DeleteFile(NewZipTemp);
      return FIMAGE_CANTOPEN;
    }
    if(GetFileLength(nf)<512)
    {
      TRACE_LOG("File length of %s = %d\n",FilePath.c_str(),GetFileLength(nf));
      fclose(nf);
      if(NewZipTemp.NotEmpty()) 
        DeleteFile(NewZipTemp);
      return FIMAGE_WRONGFORMAT;
    }
    fseek(nf,0,SEEK_SET);
    EasyStr NewMSATemp="";
    short MSA_SecsPerTrack=0,MSA_EndTrack=0,MSA_Sides=0;
    if(MSA)
    {
      NewMSATemp.SetLength(MAX_PATH);
      GetTempFileName(WriteDir,extension_list[EXT_MSA],0,NewMSATemp);
      FILE *tf=fopen(NewMSATemp,"wb");
      if(tf)
      {
        bool Err=0;
        short ID,StartTrack;
        fseek(nf,0,SEEK_SET);
        // Read header
        fread(&ID,2,1,nf);               SWAPBYTES(ID);
        fread(&MSA_SecsPerTrack,2,1,nf); SWAPBYTES(MSA_SecsPerTrack);
        fread(&MSA_Sides,2,1,nf);        SWAPBYTES(MSA_Sides);
        fread(&StartTrack,2,1,nf);       SWAPBYTES(StartTrack);
        fread(&MSA_EndTrack,2,1,nf);     SWAPBYTES(MSA_EndTrack);
/*
Header:
  Word	ID marker, should be $0E0F
  Word	Sectors per track
  Word	Sides (0 or 1; add 1 to this to get correct number of sides)
  Word	Starting track (0-based)
  Word	Ending track (0-based)
*/
        TRACE_LOG("MSA ID %X sides %d tracks %d (%d-%d) sectors %d\n",
          ID,MSA_Sides+1,MSA_EndTrack-StartTrack+1,StartTrack,MSA_EndTrack,MSA_SecsPerTrack);
        if(MSA_SecsPerTrack<1||MSA_SecsPerTrack>FLOPPY_MAX_SECTOR_NUM||
          MSA_Sides<0||MSA_Sides>1||StartTrack<0
          ||StartTrack>FLOPPY_MAX_TRACK_NUM||StartTrack>=MSA_EndTrack||
          MSA_EndTrack<1||MSA_EndTrack>FLOPPY_MAX_TRACK_NUM)
        {
          Err=true;
        }
        if(Err==0)
        {
          // Read data
          WORD Len,NumRepeats;
          BYTE *TrackData=new BYTE[(MSA_SecsPerTrack*512)+16];
          BYTE *pDat,*pEndDat,dat;
          BYTE *STBuf=new BYTE[(MSA_SecsPerTrack*512)+16];
          BYTE *pSTBuf,*pSTBufEnd=STBuf+(MSA_SecsPerTrack*512)+8;
          for(int n=0;n<=MSA_EndTrack;n++)
          {
            for(int s=0;s<=MSA_Sides;s++)
            {
              if(n>=StartTrack)
              {
                Len=0;
                fread(&Len,1,2,nf); SWAPBYTES(Len);
                if(Len>MSA_SecsPerTrack*512||Len==0)
                {
                  Err=true;
                  break;
                }
                if(WORD(fread(TrackData,1,Len,nf))<Len)
                {
                  Err=true;
                  break;
                }
                if(Len==(MSA_SecsPerTrack*512))
                  fwrite(TrackData,Len,1,tf);
                else
                {
                  // Convert compressed MSA format track in TrackData to ST format in STBuf
                  pSTBuf=STBuf;
                  pDat=TrackData;
                  pEndDat=TrackData+Len;
                  while(pDat<pEndDat && pSTBuf<pSTBufEnd)
                  {
                    dat=*(pDat++);
                    if(dat==0xE5)
                    {
                      dat=*(pDat++);
                      NumRepeats=*LPWORD(pDat);pDat+=2;
                      SWAPBYTES(NumRepeats);
                      for(int s2=0;s2<NumRepeats && pSTBuf<pSTBufEnd;s2++)
                        *(pSTBuf++)=dat;
                    }
                    else
                      *(pSTBuf++)=dat;
                  }
                  if(pSTBuf>=pSTBufEnd)
                  {
                    Err=true;
                    break;
                  }
                  fwrite(STBuf,MSA_SecsPerTrack*512,1,tf);
                }
              }
              else
              {
                ZeroMemory(TrackData,MSA_SecsPerTrack*512);
                if(n==0&&s==0)
                {   // Write BPB
                  *LPWORD(TrackData+11)=512;
                  TrackData[13]=2;           // SectorsPerCluster
                  *LPWORD(TrackData+17)=112; // nDirEntries
                  *LPWORD(TrackData+19)=WORD(MSA_EndTrack * MSA_SecsPerTrack);
                  *LPWORD(TrackData+22)=3;   // SectorsPerFAT
                  *LPWORD(TrackData+24)=MSA_SecsPerTrack;
                  *LPWORD(TrackData+26)=MSA_Sides;
                  *LPWORD(TrackData+28)=0;
                }
                fwrite(TrackData,MSA_SecsPerTrack*512,1,tf);
              }
            }
            if(Err) 
              break;
          }
          delete[] TrackData;
          delete[] STBuf;
        }
        fclose(tf);
        fclose(nf);
        if(Err==0)
        {
          SetFileAttributes(NewMSATemp,FILE_ATTRIBUTE_HIDDEN);
          nf=fopen(NewMSATemp,"r+b");
          Err=(nf==NULL);
        }
        if(Err)
        {
          TRACE_LOG("Error opening %s\n",NewMSATemp.c_str());
          DeleteFile(NewMSATemp);
          if(NewZipTemp.NotEmpty()) 
            DeleteFile(NewZipTemp);
          return FIMAGE_WRONGFORMAT;
        }
      }
      else
      {
          // Couldn't open NewMSATemp
        fclose(nf);
        if(NewZipTemp.NotEmpty()) 
          DeleteFile(NewZipTemp);
        return FIMAGE_CANTOPEN;
      }
    }
    bool f_ValidBPB=true;
    DWORD f_DiskFileLen=GetFileLength(nf);
    if(STT)
    {
      bool Err=0;
      DWORD Magic;
      WORD Version,Flags,AllTrackFlags,NumTracks,NumSides;
      fread(&Magic,4,1,nf);
      fread(&Version,2,1,nf);
      fread(&Flags,2,1,nf);
      fread(&AllTrackFlags,2,1,nf);
      fread(&NumTracks,2,1,nf);
      fread(&NumSides,2,1,nf);
      Err=(Magic!=MAKECHARCONST('S','T','E','M')||Version!=1
        ||(AllTrackFlags & BIT_0)==0);
      if(Err==0)
      {
        ZeroMemory(FloppyDisk[Id].STT_TrackStart,sizeof(FloppyDisk[Id].STT_TrackStart));
        ZeroMemory(FloppyDisk[Id].STT_TrackLen,sizeof(FloppyDisk[Id].STT_TrackLen));
        for(int s=0;s<NumSides;s++)
        {
          for(int t=0;t<NumTracks;t++)
          {
            fread(&FloppyDisk[Id].STT_TrackStart[s][t],4,1,nf);
            fread(&FloppyDisk[Id].STT_TrackLen[s][t],2,1,nf);
          }
        }
      }
      else
      {
        fclose(nf);
        if(NewMSATemp.NotEmpty()) 
          DeleteFile(NewMSATemp);
        if(NewZipTemp.NotEmpty()) 
          DeleteFile(NewZipTemp);
        return FIMAGE_WRONGFORMAT;
      }
      fseek(nf,0,SEEK_SET);
      FloppyDisk[Id].f=nf;
      FloppyDisk[Id].STT_File=true;
      FloppyDisk[Id].TracksPerSide=NumTracks;
      FloppyDisk[Id].Sides=NumSides;
      FloppyDisk[Id].BytesPerSector=512;
      FloppyDisk[Id].SectorsPerTrack=0xff; // Variable
    }
    else
    {
      TBpbInfo bpbi={0,0,0,0};
      int HeaderLen=int(DIM?32:0); //SS 0 for .ST
      f_DiskFileLen-=HeaderLen;
      if(DIM)
      {
        int Err=0;
        fseek(nf,0,SEEK_SET);
        WORD Magic;
        fread(&Magic,1,2,nf);
        if(Magic!=0x4242)
          Err=FIMAGE_DIMNOMAGIC;
        else
        {
          BYTE UsedSectors;
          fseek(nf,3,SEEK_SET);
          fread(&UsedSectors,1,1,nf);
          if(UsedSectors!=0) Err=FIMAGE_DIMTYPENOTSUPPORTED;
        }
        if(Err)
        {
          fclose(nf);
          if(NewMSATemp.NotEmpty()) 
            DeleteFile(NewMSATemp);
          if(NewZipTemp.NotEmpty()) 
            DeleteFile(NewZipTemp);
          return Err;
        }
      }
      // Always append the name of the real disk file in the zip to the name
      // of the steembpb file, even if we are using default disk
      // This is so we don't need 2 .steembpb files for the default disk
      EasyStr BPBFile=OriginalFile+RealDiskInZip+".steembpb";
      bool HasBPBFile=(GetCSFInt("BPB","Sides",0,BPBFile)!=0);
      if(MSA)
        bpbi.BytesPerSector=512; //SS MSA no choice
      else
      {
        fseek(nf,HeaderLen+11,SEEK_SET);
        fread(&bpbi.BytesPerSector,2,1,nf); //SS .ST, .DIM, we have choice?
      }
      fseek(nf,HeaderLen+19,SEEK_SET);
      fread(&bpbi.Sectors,2,1,nf);
      fseek(nf,HeaderLen+24,SEEK_SET);
      fread(&bpbi.SectorsPerTrack,2,1,nf);
      fread(&bpbi.Sides,2,1,nf);
      if(pFileBPB) 
        *pFileBPB=bpbi; // Store BPB exactly as it is in the file (for DiskMan)
      // A BPB is corrupt when one of its fields is totally wrong
      bool BPBCorrupt=false;
      if(bpbi.BytesPerSector!=128&&bpbi.BytesPerSector!=256&&
        bpbi.BytesPerSector!=512&&bpbi.BytesPerSector!=1024) 
        BPBCorrupt=true; //SS 1024 possible
      if(bpbi.SectorsPerTrack<1||bpbi.SectorsPerTrack>FLOPPY_MAX_SECTOR_NUM) 
        BPBCorrupt=true;
      if(bpbi.Sides<1||bpbi.Sides>2) 
        BPBCorrupt=true;
      // Has to be exact length for Steem to accept it
      if(DWORD(bpbi.Sectors*bpbi.BytesPerSector)!=f_DiskFileLen||BPBCorrupt)
      {
        f_ValidBPB=0;
        // If the BPB is only a few sectors out then we don't want to destroy
        // the value in BytesPerSector.
        if(BPBCorrupt) 
          bpbi.BytesPerSector=512; // 99.9% of ST disks used sectors this size
      }
      if(f_ValidBPB==0)
      {
        if(MSA)
        {
          // Probably got a better chance of being right than guessing
          bpbi.SectorsPerTrack=MSA_SecsPerTrack;
          bpbi.Sides=short(MSA_Sides+1);
          bpbi.Sectors=short((MSA_EndTrack+1)*bpbi.SectorsPerTrack*bpbi.Sides);
        }
        else
        {
          // BPB's wrong, time to guess the format
          bpbi.SectorsPerTrack=0;
          bpbi.Sectors=short(f_DiskFileLen/bpbi.BytesPerSector);
          bpbi.Sides=WORD((bpbi.Sectors<1100)?1:2); // Total guess
          // Work out bpbi.SectorsPerTrack from bpbi.Sides and bpbi.Sectors
          bool Found=false;
          for(;;)
          {
            for(int t=75;t<=FLOPPY_MAX_TRACK_NUM;t++)
            {
              for(int s=8;s<=13;s++)
              {
                if(bpbi.Sectors==(t+1)*s*bpbi.Sides)
                {
                  bpbi.SectorsPerTrack=WORD(s);
                  Found=true;
                  break;
                }
              }
              if(Found) 
                break;
            }
            if(Found) 
              break;
            if(bpbi.Sectors<10) 
              break;
            bpbi.Sectors--;
          }
          if(bpbi.SectorsPerTrack==0&&HasBPBFile==0)
          {
            fclose(nf);
            if(NewMSATemp.NotEmpty()) 
              DeleteFile(NewMSATemp);
            if(NewZipTemp.NotEmpty()) 
              DeleteFile(NewZipTemp);
            return FIMAGE_WRONGFORMAT;
          }
        }
      }
      if(pDetectBPB) 
        *pDetectBPB=bpbi; // Steem's best guess (or the BPB if it is valid)
      if(HasBPBFile)
      {
        // User specified disk parameters
        TConfigStoreFile CSF(BPBFile);
        bpbi.Sides=CSF.GetInt("BPB","Sides",2);
        bpbi.SectorsPerTrack=CSF.GetInt("BPB","SectorsPerTrack",9);
        bpbi.BytesPerSector=CSF.GetInt("BPB","BytesPerSector",512);
        bpbi.Sectors=CSF.GetInt("BPB","Sectors",1440);
        CSF.Close();
      }
      fseek(nf,HeaderLen,SEEK_SET);
      FloppyDisk[Id].f=nf;
      FloppyDisk[Id].BytesPerSector=short(bpbi.BytesPerSector);
      FloppyDisk[Id].SectorsPerTrack=short(bpbi.SectorsPerTrack);
      FloppyDisk[Id].Sides=short(bpbi.Sides);
      if(FloppyDisk[Id].SectorsPerTrack && FloppyDisk[Id].Sides)
        FloppyDisk[Id].TracksPerSide
        =short(short(bpbi.Sectors/FloppyDisk[Id].SectorsPerTrack)/FloppyDisk[Id].Sides);
      FloppyDisk[Id].DIM_File=DIM;
    }
    FloppyDisk[Id].MSATempFile=NewMSATemp;
    FloppyDisk[Id].ValidBPB=f_ValidBPB;
    FloppyDisk[Id].DiskFileLen=f_DiskFileLen;
  }
  FloppyDisk[Id].ZipTempFile=NewZipTemp;
  FloppyDisk[Id].DiskInZip=NewDiskInZip;
#if defined(SSE_STATS)
  FloppyDisk[Id].RealDiskInZip=RealDiskInZip;
#endif
  FloppyDisk[Id].ImageFile=OriginalFile;
  FloppyDisk[Id].PastiDisk=f_PastiDisk;
  if(ImageType.Extension)
    m_DiskInDrive=true;
  FloppyDisk[Id].WrittenTo=0;
  //SS note that options haven't been retrieved yet when starting
  //steem and auto.sts is loaded with its disks -> we can't open a
  //ghost image here, we don't know if option is set 
//TODO, now we could...
  // Media change, write protect for 10 VBLs, unprotect for 10 VBLs, wp for 10
  if(this==&FloppyDrive[0]) 
    floppy_mediach[0]=30;
  if(this==&FloppyDrive[1]) 
    floppy_mediach[1]=30;
  // disable input for pasti
  disable_input_vbl_count=MAX(disable_input_vbl_count,30);
#ifdef ENABLE_LOGFILE
  DBG_LOG("");
  DBG_LOG(EasyStr("FDC: Inserted disk ")+OriginalFile);
  DBG_LOG(EasyStr("     Into drive ")+LPSTR(floppy_current_drive()?"B":"A")+" its BPB was "+LPSTR(FloppyDisk[Id].ValidBPB?"valid.":"invalid."));
  DBG_LOG(EasyStr("     BytesPerSector=")+FloppyDisk[Id].BytesPerSector+", SectorsPerTrack="+FloppyDisk[Id].SectorsPerTrack+
    ", Sides="+FloppyDisk[Id].Sides);
  DBG_LOG(Str("     TracksPerSide=")+FloppyDisk[Id].TracksPerSide+", ReadOnly="+FloppyDisk[Id].ReadOnly);
  DBG_LOG("");
#endif
  if(!OPTION_AUTOSTW && Id<2 && ImageType.Manager==MNGR_STEEM // eg Union Demo STW!
    && FloppyDisk[Id].SectorsPerTrack>=12 
    && FloppyDisk[drive].TrackBytes==DISK_BYTES_PER_TRACK)
  {
    FloppyDisk[drive].TrackBytes*=2; // we're still slow but maybe it's accurate! (ADAT)
  }
#if defined(SSE_DISK_AUTOSTW)  
  ImageType.RealExtension=ImageType.Extension;
//  ASSERT(ImageType.RealExtension!=5);
  if(OPTION_AUTOSTW && Id<2 && ImageType.Manager==MNGR_STEEM && DiskInDrive()
#if !defined(SSE_DISK_HD)
    && FloppyDisk[Id].SectorsPerTrack<=11 // but <9 OK: Wipe-Out -RPL
#endif
    ) 
  {
    // create temp STW, Steem will use it instead of the ST/MSA/DIM
    EasyStr tmppath=RunDir;
    char tmpname[14];
    sprintf(tmpname,"\\TEMPSTW%d.STW",Id);
    tmppath+=tmpname;
    bool ok=STtoSTW(Id,tmppath.Text);
    if(ok)
    {
      TRACE_LOG("Converted %s (%d-%d-%d) to STW\n",extension_list[ImageType.Extension],FloppyDisk[Id].Sides,FloppyDisk[Id].TracksPerSide,FloppyDisk[Id].SectorsPerTrack);
      ImageType.Manager=MNGR_WD1772;
      ImageType.Extension=EXT_STW;
      reading=writing=0;
      FloppyDisk[Id].WrittenTo=false; // set during conversion
    }
  }
#if defined(SSE_DISK_HD) // determine DD or HD
  FloppyDisk[drive].Density=(FloppyDisk[Id].TrackBytes>DISK_BYTES_PER_TRACK //=*2
    || FloppyDisk[Id].SectorsPerTrack>11
    || FloppyDisk[Id].BytesPerSector>=1024 && FloppyDisk[Id].SectorsPerTrack>6)
    ? 2 : 1;
#endif
  TRACE2("%c:%s (%d) %d-%d-%d-%d %s %s\n",
    'A'+Id,extension_list[ImageType.RealExtension],ImageType.Manager,
    FloppyDisk[Id].Sides,FloppyDisk[Id].TracksPerSide,FloppyDisk[Id].SectorsPerTrack,
    FloppyDisk[Id].TrackBytes,RealDiskInZip.Text,GetFileNameFromPath(OriginalFile.Text));
#endif
  FloppyDrive[drive].UpdateAdat();
#if defined(SSE_DEBUG)
  TRACE_LOG("Ext %s Manager %d Sides %d Tracks %d Sectors %d adat %d\n",
    extension_list[ImageType.Extension],
    ImageType.Manager,FloppyDisk[Id].Sides,FloppyDisk[Id].TracksPerSide,FloppyDisk[Id].SectorsPerTrack,
    adat);
#endif
#if USE_PASTI
  // catch player's mistake
  if(ImageType.Manager!=MNGR_PASTI && pasti_active)
  {
    Alert(T("Disabling pasti.dll to run current disk image"),T("Warning"),0);
    pasti_active=false;
  }
#endif  
  if(ImageType.Manager!=MNGR_STEEM)
    agenda_delete(agenda_fdc_motor_flag_off); // just in case
  else if(!ADAT && (Fdc.str&FDC_STR_MO)) // assume drive was empty
    agenda_add(agenda_fdc_motor_flag_off,milliseconds_to_hbl(1800),0);
  return 0;
}


void TSF314::RemoveDisk(bool LoseChanges) {
  if(Empty()&&Id<2) // nothing to do
    return;
  if((this==&FloppyDrive[0])&&(DiskMan.AutoInsert2&2))
    DiskMan.EjectDisk(1); // v3.7.2 don't keep former disk B
  DiskMan.AutoInsert2&=~2; //TODO def
  TRACE_LOG("Remove disk %s from drive %c\n",FloppyDisk[Id].DiskName.Text,'A'+Id);
#if defined(SSE_DISK_AUTOSTW)  
  if(OPTION_AUTOSTW && ImageType.Manager==MNGR_WD1772 && Id<2 
    && ImageType.Extension==EXT_STW && (ImageType.RealExtension==EXT_ST
    ||ImageType.RealExtension==EXT_MSA||ImageType.RealExtension==EXT_DIM))
  {
    if(!LoseChanges && FloppyDisk[Id].WrittenTo)
    {
      if(STWtoST(Id)) // if not, write fails, image is closed
      {
        TRACE_LOG("Converted back (%d-%d-%d) from STW\n",FloppyDisk[Id].Sides,FloppyDisk[Id].TracksPerSide,FloppyDisk[Id].SectorsPerTrack);
      }
    }
    else
      ImageSTW[Id].Close(); // or it won't be deleted!
    // delete temp STW
    EasyStr tmppath=RunDir;
    char tmpname[14];
    sprintf(tmpname,"\\TEMPSTW%d.STW",Id);
    tmppath+=tmpname;
    DeleteFile(tmppath);
    ImageType.Manager=MNGR_STEEM; // temp
    ImageType.Extension=ImageType.RealExtension;
  }
#endif
  if(FloppyDisk[Id].f && FloppyDisk[Id].ReadOnly==0&&LoseChanges==0&&FloppyDisk[Id].WrittenTo 
    && FloppyDisk[Id].ZipTempFile.Empty())
  {
    short MSASecsPerTrack,MSAStartTrack=0,MSAEndTrack,MSASides;
    bool MSAResize=0;
    if(FloppyDisk[Id].Format_f)
    {
      if(FloppyDisk[Id].FormatLargestSector>0)
      { // Formatted any track properly?
        // Try to merge the formatted data on Format_f with the old data on f
        int MaxTrack=0;
        for(int Side=0;Side<2;Side++)
        {
          for(int Track=FLOPPY_MAX_TRACK_NUM;Track>0;Track--)
          {
            if(FloppyDisk[Id].TrackIsFormatted[Side][Track])
            {
              if(Track>MaxTrack) MaxTrack=Track;
              break;
            }
          }
        }
        if(MaxTrack>0)
        {
          bool CanShrink=true,WipeOld=true;
          // Only shrink if all tracks were written
          for(int Track=MaxTrack;Track>=1;Track--)
          {
            if(FloppyDisk[Id].TrackIsFormatted[0][Track]==0)
            {
              CanShrink=0;
              WipeOld=0;
              break;
            }
          }
          if(MaxTrack<70)
          { // Might want some old data left on the end
            CanShrink=0;
            WipeOld=0;
          }
          // Should we make it single-sided?
          int NewSides=1;
          for(int Track=MaxTrack;Track>=0;Track--)
          {
            if(FloppyDisk[Id].TrackIsFormatted[1][Track])
              NewSides=2;
            else
              // Don't wipe if haven't formatted over all sectors
              WipeOld=0;
          }
          if(CanShrink==0) 
            NewSides=MAX(NewSides,(int)(FloppyDisk[Id].Sides));
          int NewTracksPerSide=(int)(CanShrink?MaxTrack+1:
            MAX((int)FloppyDisk[Id].TracksPerSide,MaxTrack+1));
          int NewSectorsPerTrack=(int)(CanShrink?FloppyDisk[Id].FormatMostSectors:
            MAX((int)FloppyDisk[Id].SectorsPerTrack,FloppyDisk[Id].FormatMostSectors));
          int NewBytesPerSector=(int)(WipeOld?FloppyDisk[Id].FormatLargestSector:
            MAX((int)FloppyDisk[Id].BytesPerSector,FloppyDisk[Id].FormatLargestSector));
          int NewBytesPerTrack=NewBytesPerSector*NewSectorsPerTrack;
          DBG_LOG("FDC: Formatted disk removed, copying data to disk image");
          DBG_LOG(EasyStr("  New format: Sides=")+NewSides+"  Tracks per side="+NewTracksPerSide+
            "     SectorsPerTrack="+NewSectorsPerTrack);
          int HeaderLen=(int)(FloppyDisk[Id].DIM_File?32:0);
          BYTE *NewDiskBuf=new BYTE[HeaderLen+NewBytesPerSector
            *NewSectorsPerTrack*NewTracksPerSide*NewSides];
          BYTE *lpNewDisk=NewDiskBuf;
          ZeroMemory(NewDiskBuf,HeaderLen+NewBytesPerSector
            *NewSectorsPerTrack*NewTracksPerSide*NewSides);
          if(HeaderLen)
          {
            // Keep the header if there is one
            fseek(FloppyDisk[Id].f,0,SEEK_SET);
            fread(lpNewDisk,HeaderLen,1,FloppyDisk[Id].f);
            lpNewDisk+=HeaderLen;
          }
          for(int t=0;t<NewTracksPerSide;t++)
          {
            for(int Side=0;Side<NewSides;Side++)
            {
              int Countdown=3;
              if(FloppyDisk[Id].TrackIsFormatted[Side][t])
              {
                // Read a track from the format file
                for(int s=1;s<=NewSectorsPerTrack;s++)
                {
                  bool NextSector=true;
                  FloppyDisk[Id].SeekSector(Side,t,s,true,false);
                  if(fread(lpNewDisk,1,NewBytesPerSector,FloppyDisk[Id].Format_f)
                    <size_t(NewBytesPerSector))
                  {
                    if((Countdown--)>0)
                    {
                      if(FloppyDisk[Id].ReopenFormatFile())
                      {
                        s--; // Try to redo a sector 3 times
                        NextSector=0;
                      }
                    }
                  }
                  if(NextSector) 
                    lpNewDisk+=NewBytesPerSector;
                }
              }
              else if(t<FloppyDisk[Id].TracksPerSide && Side<FloppyDisk[Id].Sides)
              {
                // Copy information from the old disk onto the new disk
                for(int s=1;s<=MIN(int(FloppyDisk[Id].SectorsPerTrack),
                  NewSectorsPerTrack);s++)
                {
                  bool NextSector=true;
                  FloppyDisk[Id].SeekSector(Side,t,s,false,false);
                  if(fread(lpNewDisk,1,MIN(int(FloppyDisk[Id].BytesPerSector),
                    NewBytesPerSector),FloppyDisk[Id].f)
                    <size_t(MIN(int(FloppyDisk[Id].BytesPerSector),NewBytesPerSector)))
                  {
                    if((Countdown--)>0)
                    {
                      if(ReinsertDisk())
                      {
                        s--; // Try to redo a sector 3 times
                        NextSector=0;
                      }
                    }
                  }
                  if(NextSector) 
                    lpNewDisk+=NewBytesPerSector;
                }
                // If getting bigger then skip
                for(int s=FloppyDisk[Id].SectorsPerTrack;s<NewSectorsPerTrack;s++) 
                  lpNewDisk+=NewBytesPerSector;
              }
              else
                lpNewDisk+=NewBytesPerTrack;
            }
          }
          // Write it back to the original file (finally)
          int Countdown=3;
          for(;;)
          {
            fclose(FloppyDisk[Id].f);
            FloppyDisk[Id].SectorsPerTrack=short(NewSectorsPerTrack);
            FloppyDisk[Id].Sides=short(NewSides);
            FloppyDisk[Id].TracksPerSide=short(NewTracksPerSide);
            FloppyDisk[Id].BytesPerSector=short(NewBytesPerSector);
            if(FloppyDisk[Id].MSATempFile.NotEmpty())
            {
              MSASecsPerTrack=short(NewSectorsPerTrack);
              MSAEndTrack=short(NewTracksPerSide+1);
              MSASides=short(NewSides-1);
              MSAResize=true;
              FloppyDisk[Id].f=fopen(FloppyDisk[Id].MSATempFile,"wb");
            }
            else
              FloppyDisk[Id].f=fopen(FloppyDisk[Id].ImageFile,"wb");
            if(FloppyDisk[Id].f)
            {
              fseek(FloppyDisk[Id].f,0,SEEK_SET);
              int NewDiskSize=HeaderLen+NewBytesPerSector*NewSectorsPerTrack
                *NewTracksPerSide*NewSides;
              if(fwrite(NewDiskBuf,1,NewDiskSize,FloppyDisk[Id].f)
                ==size_t(NewDiskSize))
              {
                TConfigStoreFile CSF(FloppyDisk[Id].ImageFile+".steembpb");
                CSF.SetStr("BPB","Sides",Str(FloppyDisk[Id].Sides));
                CSF.SetStr("BPB","SectorsPerTrack",Str(FloppyDisk[Id].SectorsPerTrack));
                CSF.SetStr("BPB","BytesPerSector",Str(FloppyDisk[Id].BytesPerSector));
                CSF.SetStr("BPB","Sectors",Str(FloppyDisk[Id].SectorsPerTrack
                  *FloppyDisk[Id].TracksPerSide*FloppyDisk[Id].Sides));
                CSF.Close();
                break;
              }
              else
              {
                if((--Countdown)<0)
                {
                  log_write("Error writing to disk image after format! All data lost!");
                  break;
                }
              }
            }
            else
            {
              log_write("Error opening disk image after format! All data lost!");
              break;
            }
          }
          delete[] NewDiskBuf;
        }
      }
    }
    if(FloppyDisk[Id].MSATempFile.NotEmpty()&&FloppyDisk[Id].f)
    {
      // Write ST format MSATempFile to MSA format ImageFile
      WIN_ONLY(if(stem_mousemode!=STEM_MOUSEMODE_WINDOW) 
      SetCursor(LoadCursor(NULL,IDC_WAIT)); )
      FILE *MSA=fopen(FloppyDisk[Id].ImageFile,"r+b");
      if(MSA)
      {
        BYTE Temp;
        fseek(MSA,2,SEEK_SET); //Seek past ID
        if(MSAResize==0)
        {
          fread(&MSASecsPerTrack,2,1,MSA); SWAPBYTES(MSASecsPerTrack);
          fread(&MSASides,2,1,MSA);        SWAPBYTES(MSASides);
          fseek(MSA,2,SEEK_CUR);        // Skip StartTrack
          fread(&MSAEndTrack,2,1,MSA);     SWAPBYTES(MSAEndTrack);
          fseek(MSA,6,SEEK_SET);
          Temp=HIBYTE(MSAStartTrack);   fwrite(&Temp,1,1,MSA);
          Temp=LOBYTE(MSAStartTrack);   fwrite(&Temp,1,1,MSA);
        }
        else
        {
              // Write out MSA file info (in big endian)
          Temp=HIBYTE(MSASecsPerTrack); fwrite(&Temp,1,1,MSA);
          Temp=LOBYTE(MSASecsPerTrack); fwrite(&Temp,1,1,MSA);
          Temp=HIBYTE(MSASides);        fwrite(&Temp,1,1,MSA);
          Temp=LOBYTE(MSASides);        fwrite(&Temp,1,1,MSA);
          Temp=HIBYTE(MSAStartTrack);   fwrite(&Temp,1,1,MSA);
          Temp=LOBYTE(MSAStartTrack);   fwrite(&Temp,1,1,MSA);
          Temp=HIBYTE(MSAEndTrack);     fwrite(&Temp,1,1,MSA);
          Temp=LOBYTE(MSAEndTrack);     fwrite(&Temp,1,1,MSA);
        }
        fseek(MSA,10,SEEK_SET); // Past header
        fseek(FloppyDisk[Id].f,0,SEEK_SET);
        int Len=WORD(MSASecsPerTrack*512);
        // Convert ST format f to MSA format MSA (uncompressed)
        int ReinsertAttempts=0;
        BYTE *MSADataBuf=new BYTE[(MSAEndTrack+1)*(MSASides+1)
          *MSASecsPerTrack*(512+2)];
        BYTE *pD=MSADataBuf;
        for(int t=0;t<=MSAEndTrack;t++)
        {
          for(int s=0;s<=MSASides;s++)
          {
            *(pD++)=HIBYTE(Len);
            *(pD++)=LOBYTE(Len);
            for(int sec=1;sec<=MSASecsPerTrack;sec++)
            {
              FloppyDisk[Id].SeekSector(s,t,sec,false,false);
              if(fread(pD,1,512,FloppyDisk[Id].f)==512)
                // Read sector from ST file
                pD+=512;
              else if(ReinsertAttempts<5)
              {
                ReinsertDisk();
                sec--;
                ReinsertAttempts++;
              }
              else
              { // All else has failed, write an empty sector
                ZeroMemory(pD,512);
                pD+=512;
              }
            }
          }
        }
        fwrite(MSADataBuf,1,(LONG_PTR)(pD)-(LONG_PTR)(MSADataBuf),MSA);
        fclose(MSA);
        delete[] MSADataBuf;
      }
      WIN_ONLY(if(stem_mousemode!=STEM_MOUSEMODE_WINDOW) SetCursor(PCArrow); )
    }
  }
#if USE_PASTI
  if(FloppyDisk[Id].PastiDisk && Id<2) // not sure pasti expects 3 drives!
  {
    if(FloppyDisk[Id].ZipTempFile.Empty()&&FloppyDisk[Id].ReadOnly==0
      &&hPasti && LoseChanges==0)
    {
      struct pastiDISKIMGINFO pdi;
      pdi.mode=PASTI_LDFNAME;
      pdi.fileName=FloppyDisk[Id].ImageFile;
      pdi.fileBuf=FloppyDisk[Id].PastiBuf;
      pdi.bufSize=FloppyDisk[Id].PastiBufLen;
    }
    pasti->Eject(Id,ABSOLUTE_CPU_TIME);
  }
#endif
  if(FloppyDisk[Id].PastiBuf) 
    delete[] FloppyDisk[Id].PastiBuf;
  FloppyDisk[Id].PastiBuf=NULL;
  FloppyDisk[Id].PastiBufLen=0;
  FloppyDisk[Id].PastiDisk=0;
#if defined(SSE_DISK_CAPS)
  if(CAPSIMG_OK && Id<2 && ImageType.Manager==MNGR_CAPS) // not sure caps expects 3 drives!
    Caps.RemoveDisk(Id);
#endif
  if(LoseChanges)
    FloppyDisk[Id].WrittenTo=false; // this is checked by MFM managers
  // use polymorphism for closing (for opening it wouldn't make it simpler)
  if(ImageType.Manager==MNGR_WD1772 && MfmManager)
    MfmManager->Close();
  reading=writing=0; //? TODO
  if(FloppyDisk[Id].f) 
    fclose(FloppyDisk[Id].f);
  FloppyDisk[Id].f=NULL;
  if(FloppyDisk[Id].Format_f) 
    fclose(FloppyDisk[Id].Format_f);
  FloppyDisk[Id].Format_f=NULL;
#if defined(SSE_TOS_PRG_AUTORUN)
  if(ImageType.Manager==MNGR_PRG)
  {
    HardDiskMan.DisableHardDrives=(SSEConfig.old_DisableHardDrives!=0);
    HardDiskMan.update_mount();
    HWND icon_handle=GetDlgItem(DiskMan.Handle,10); // update GEMDOS HD icon
    SendMessage(icon_handle,BM_SETCHECK,!HardDiskMan.DisableHardDrives,0);
  }
#endif
  ImageType.Manager=(BYTE)((pasti_active) ? MNGR_PASTI :
    (OPTION_AUTOSTW ? MNGR_WD1772 : MNGR_STEEM));
  ImageType.Extension=0;
#if defined(SSE_DISK_AUTOSTW)  
  ImageType.RealExtension=ImageType.Extension;
#endif
  UpdateAdat();
#if defined(SSE_DISK_GHOST)
  // This makes sure to update the image before leaving
  if(OPTION_GHOST_DISK && ghost)
  {
    ASSERT(Id<2); // disk 2 will never be ghosted
    GhostDisk[Id].Close();
    ghost=0;
  }
#endif
  m_DiskInDrive=false;
  if(FloppyDisk[Id].ZipTempFile.NotEmpty())    
    DeleteFile(FloppyDisk[Id].ZipTempFile);
  if(FloppyDisk[Id].MSATempFile.NotEmpty())    
    DeleteFile(FloppyDisk[Id].MSATempFile);
  if(FloppyDisk[Id].FormatTempFile.NotEmpty()) 
    DeleteFile(FloppyDisk[Id].FormatTempFile);
  FloppyDisk[Id].ImageFile=FloppyDisk[Id].MSATempFile=FloppyDisk[Id].ZipTempFile
    =FloppyDisk[Id].FormatTempFile=FloppyDisk[Id].DiskName="";
  FloppyDisk[Id].BytesPerSector=FloppyDisk[Id].Sides
    =FloppyDisk[Id].SectorsPerTrack=FloppyDisk[Id].TracksPerSide=0;
  ZeroMemory(FloppyDisk[Id].TrackIsFormatted,sizeof(FloppyDisk[Id].TrackIsFormatted));
  FloppyDisk[Id].FormatMostSectors=0;FloppyDisk[Id].FormatLargestSector=0;
  FloppyDisk[Id].STT_File=FloppyDisk[Id].DIM_File=0;
}


EasyStr TSF314::GetDisk() { 
  return FloppyDisk[Id].GetImageFile(); 
}


bool TSF314::ReinsertDisk() {
  ASSERT(Id<2);
  if(Empty()||FloppyDrive[Id].ImageType.Manager!=MNGR_STEEM)
    return false;
  fclose(FloppyDisk[Id].f);
  if(FloppyDisk[Id].ZipTempFile.NotEmpty()) 
    FloppyDisk[Id].ReadOnly=(FloppyArchiveIsReadWrite==0);
  if(FloppyDisk[Id].MSATempFile.NotEmpty())
    FloppyDisk[Id].f=fopen(FloppyDisk[Id].MSATempFile,"r+b");
  else if(FloppyDisk[Id].ZipTempFile.NotEmpty())
    FloppyDisk[Id].f=fopen(FloppyDisk[Id].ZipTempFile,LPSTR(FloppyDisk[Id].ReadOnly?"rb":"r+b"));
  else
    FloppyDisk[Id].f=fopen(FloppyDisk[Id].ImageFile,LPSTR(FloppyDisk[Id].ReadOnly?"rb":"r+b"));
  if(FloppyDisk[Id].f==NULL)
  {
    DiskMan.EjectDisk(this==&FloppyDrive[0] ? 0 : 1);
    return false;
  }
  return true;
}


//#pragma warning (default: 4701)


///////////////////////////////////// SOUND ///////////////////////////////////

#if defined(SSE_DRIVE_SOUND)

#undef LOGSECTION
#define LOGSECTION LOGSECTION_SOUND

/*
    This is where we emulate the floppy drive sounds. v3.6
    Could be a separate cpp file.
    DirectSound makes it rather easy, you just load your samples
    in secondary buffers and play as needed, one shot or loop,
    the mixing is done by the system.
    Each drive can have its own soundset.
*/


void TSF314::Sound_ChangeVolume() {
/*  Same volume for each buffer
*/
  Sound_Volume=MIN(Sound_Volume,10000);
  for(int i=0;i<NSOUNDS;i++)
  {
    if(Sound_Buffer[Id][i])
      Sound_Buffer[Id][i]->SetVolume(Sound_Volume);
  }
}


void TSF314::Sound_CheckCommand(BYTE cr) {
/*  Called at each WD1772 command, beginning and when spun up (Steem managers).
    If motor wasn't on we play the startup sound.
*/

  if(MuteWhenInactive&&bAppActive==false || Id==num_connected_floppies
    || ImageType.Manager==MNGR_PRG || !m_DiskInDrive) 
    return;
  
  DWORD dwStatus ;
  
  if(!(Fdc.str&FDC_STR_MO) && Sound_Buffer[Id][START])
  {
    Sound_Buffer[Id][START]->GetStatus(&dwStatus);
    if(!(dwStatus&DSBSTATUS_PLAYING))
      Sound_Buffer[Id][START]->Play(0,0,0);
  }

  if(!(cr&BIT_7)&&(ImageType.Manager==MNGR_PASTI||ImageType.Manager==MNGR_CAPS))
  {
    if(track!=old_track)
      Sound_Step();
  }

  if(OPTION_DRIVE_SOUND_SEEK_SAMPLE && Sound_Buffer[Id][SEEK] &&
    ( (cr&(BIT_7+BIT_6+BIT_5+BIT_4))==0x00 
      && track>=DRIVE_SOUND_BUZZ_THRESHOLD // RESTORE
    || (cr&(BIT_7+BIT_6+BIT_5+BIT_4))==0x10 
      &&abs_quick(track-Fdc.dr)>=DRIVE_SOUND_BUZZ_THRESHOLD // SEEK
    || Fdc.old_cr==cr && (cr&(BIT_6+BIT_5)) && !(cr&BIT_7) ) ) // STEPS
  {
    Sound_Buffer[Id][SEEK]->GetStatus(&dwStatus);
    if(!(dwStatus&DSBSTATUS_PLAYING))
      Sound_Buffer[Id][SEEK]->Play(0,0,DSBPLAY_LOOPING); // start SEEK loop
  }
}


void TSF314::Sound_CheckIrq() {
/*  Called at the end of each FDC command, all WD1772 emulations.
    Stop SEEK loop.
    Emit a "STEP" click noise if we were effectively seeking.
*/
  if(Sound_Buffer[Id][SEEK])
  {
    DWORD dwStatus ;
    Sound_Buffer[Id][SEEK]->GetStatus(&dwStatus);
    if(!(Fdc.cr&(BIT_7+BIT_6+BIT_5)) && //RESTORE/SEEK
      (dwStatus&DSBSTATUS_PLAYING))
      Sound_Buffer[Id][SEEK]->Stop();

    if((ImageType.Manager==MNGR_PASTI||ImageType.Manager==MNGR_CAPS)
      && !(MuteWhenInactive&&bAppActive==false) && Id!=num_connected_floppies
      && Fdc.CommandType()==1 && old_track!=track
      && ImageType.Manager!=MNGR_PRG && m_DiskInDrive
      && Sound_Buffer[Id][STEP])
    {
      old_track=track; //record
      Sound_Buffer[Id][STEP]->GetStatus(&dwStatus);
      if(!(dwStatus&DSBSTATUS_PLAYING))
        Sound_Buffer[Id][STEP]->Play(0,0,0);
    }
  }
}


void TSF314::Sound_CheckMotor() {
/*  Called at each emu VBL, start or stop playing motor sound loop if needed.
*/
  if(!Sound_Buffer[Id][MOTOR])
    return;
  DWORD dwStatus ;
  Sound_Buffer[Id][MOTOR]->GetStatus(&dwStatus);
  Dma.UpdateRegs();
  bool motor_on= ((Fdc.str&FDC_STR_MO)
    && !Empty() // but clicks still on
    && ImageType.Manager!=MNGR_PRG
    && Id==Psg.CurrentDrive() //must be selected
    && !(MuteWhenInactive&&bAppActive==false)  // already covered?
    && Id!=num_connected_floppies);
  if(OPTION_DRIVE_SOUND && motor_on && !(dwStatus&DSBSTATUS_PLAYING))
    Sound_Buffer[Id][MOTOR]->Play(0,0,DSBPLAY_LOOPING); // start motor loop
  else if((!OPTION_DRIVE_SOUND||!motor_on) && (dwStatus&DSBSTATUS_PLAYING))
    Sound_Buffer[Id][MOTOR]->Stop();
  // step sound at vbl (this method misses some steps)
  if(motor_on && (Fdc.str&FDC_STR_BSY) 
    &&((ImageType.Manager==MNGR_PASTI||ImageType.Manager==MNGR_CAPS)||!ADAT))
  {
    if((Fdc.cr&0xE0) && old_track!=track)
    {
      //TRACE("vbl %d stepping\n",FRAME);
      Sound_Step();
    }
    if(ImageType.Manager==MNGR_PASTI||ImageType.Manager==MNGR_CAPS)
    { // can't count on FDC_STR_SU?
      if(Fdc.CommandType()==1 //&& (Fdc.str&FDC_STR_SU) 
        && old_track!=track) // started moving
        Sound_CheckCommand(Fdc.cr);
    }
  }
  // check if we must stop SEEK sample
  if(Sound_Buffer[Id][SEEK])
  {
    Sound_Buffer[Id][SEEK]->GetStatus(&dwStatus);
    if((dwStatus&DSBSTATUS_PLAYING) && Id!=Psg.CurrentDrive()
      || Fdc.CommandType()!=1 
      || (Fdc.str&(FDC_STR_BSY|FDC_STR_MO))!=(FDC_STR_BSY|FDC_STR_MO)
      || old_track==track)
      Sound_Buffer[Id][SEEK]->Stop();
  }
  old_track=track; //record
}


void TSF314::Sound_LoadSamples(IDirectSound *DSObj,DSBUFFERDESC *dsbd,
                               WAVEFORMATEX *wfx) {
/*  Called from sound.cpp's DSCreateSoundBuf(), on each run().
    We load each sample in its own secondary buffer, each time, which doesn't 
    seem optimal, but saves memory.
*/
  HRESULT Ret;
  TWavFileFormat WavFileFormat;
  FILE *fp;
  EasyStr path=DriveSoundDir[Id]+SLASH;
  EasyStr pathplusfile;
  for(int i=0;i<NSOUNDS;i++)
  {
    pathplusfile=path;
    pathplusfile+=drive_sound_wav_files[i];
    fp=fopen(pathplusfile.Text,"rb");
    if(fp)
    {
      fread(&WavFileFormat,sizeof(TWavFileFormat),1,fp);
      wfx->nChannels=WavFileFormat.nChannels;
      wfx->nSamplesPerSec=WavFileFormat.nSamplesPerSec;
      wfx->wBitsPerSample=WavFileFormat.wBitsPerSample;
      wfx->nBlockAlign=wfx->nChannels*wfx->wBitsPerSample/8;
      wfx->nAvgBytesPerSec=WavFileFormat.nAvgBytesPerSec;
      dsbd->dwFlags|=DSBCAPS_STATIC ;
      dsbd->dwBufferBytes=WavFileFormat.length;
      Ret=DSObj->CreateSoundBuffer(dsbd,&Sound_Buffer[Id][i],NULL);
      if(Ret==DS_OK)
      {
        LPVOID lpvAudioPtr1;
        DWORD dwAudioBytes1;
        Ret=Sound_Buffer[Id][i]->Lock(0,0,&lpvAudioPtr1,&dwAudioBytes1,NULL,0,
          DSBLOCK_ENTIREBUFFER );
        if(Ret==DS_OK)
          fread(lpvAudioPtr1,1,dwAudioBytes1,fp);
        Ret=Sound_Buffer[Id][i]->Unlock(lpvAudioPtr1,dwAudioBytes1,NULL,0);
      }
      fclose(fp);
    }
#ifdef SSE_DEBUG
    else TRACE_LOG("DriveSound. Can't load sample file %s\n",pathplusfile.Text);
#endif
  }//nxt
  Sound_ChangeVolume();
}


void TSF314::Sound_ReleaseBuffers() {
  // Called from sound.cpp's DSReleaseAllBuffers()
  for(int i=0;i<NSOUNDS;i++)
  {
    if(Sound_Buffer[Id][i])
    {
      Sound_Buffer[Id][i]->Stop();
      Sound_Buffer[Id][i]->Release();
      Sound_Buffer[Id][i]=NULL;
    }
  }
}


void TSF314::Sound_Step() {
  if(!Sound_Buffer[Id][STEP]
    || ImageType.Manager==MNGR_PRG
    || (MuteWhenInactive&&bAppActive==false) 
    || Id==num_connected_floppies)
    return;
  Sound_Buffer[Id][STEP]->SetCurrentPosition(0); // necessay for seek effect
  Sound_Buffer[Id][STEP]->Play(0,0,0);
}


void TSF314::Sound_StopBuffers() {
  for(int i=0;i<NSOUNDS;i++)
    if(Sound_Buffer[Id][i])
      Sound_Buffer[Id][i]->Stop();
}

#endif//sound
