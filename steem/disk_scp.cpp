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
FILE: disk_scp.cpp
CONDITION: SSE_DISK_SCP must be defined
DESCRIPTION: SCP disk images are produced with Supercard Pro hardware, 
they're at bit level.
TODO: weak bits not always correctly read
TODO: 8bit images not tested
TODO: writing on disk
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#if defined(SSE_DISK_SCP)

#include <cpu.h>
#include <device_map.h>
#include <run.h>
#include <acc.h>
#include <debug.h>
#include <computer.h>
#include <options.h>
#include <disk_scp.h>

#define N_SIDES FloppyDisk[Id].Sides
#define N_TRACKS FloppyDisk[Id].TracksPerSide
#define LOGSECTION LOGSECTION_IMAGE_INFO
#undef CURRENT_TRACK
#define CURRENT_TRACK (FloppyDrive[Id].track)


TImageSCP::TImageSCP() {
  Init();
}


TImageSCP::~TImageSCP() {
  Close();
}


void TImageSCP::Close() {
  if(fCurrentImage)
  {
    TRACE_LOG("SCP %d close image\n",Id);
    fclose(fCurrentImage);
    if(TimeFromIndexPulse)
      free(TimeFromIndexPulse);
  }
  Init();
}


void TImageSCP::ComputePosition() {
  // when we start reading/writing, where on the disk?
  //ASSERT(TimeFromIndexPulse);
  if(!TimeFromIndexPulse)
    return; //safety
  int cycles_since_ip=(int)(Fdc.current_time-FloppyDrive[Id].time_of_last_ip);
  DWORD units=cycles_since_ip*5; // in SCP units
  Position=0;
  for(DWORD i=0;i<nBits;i++) // slow search
  {
    if(TimeFromIndexPulse[i]>=units)
    {
      Position=i; // can be 0
      break;
    }
  }
  Fdc.Dpll.Reset(ACT); 
  //ASSERT(FloppyDrive[DRIVE].CyclesPerByte());
  // just informative? TODO
  if(FloppyDrive[Id].CyclesPerByte())
    FloppyDisk[Id].current_byte=(WORD)((Fdc.current_time
      -FloppyDrive[Id].time_of_last_ip)/FloppyDrive[Id].CyclesPerByte());
  TRACE_WD("SCP Position %d\n",Position);
}


int TImageSCP::UnitsToNextFlux(DWORD position) {
  // 1 unit = 25 nanoseconds = 1/40 ms
  //ASSERT(position<nBits);
  //ASSERT(position>=0);
  position=position%nBits; // safety
  DWORD 
    //int
    time1=0,time2;
  if(position)
    time1=TimeFromIndexPulse[position-1];
  time2=TimeFromIndexPulse[position];
  //ASSERT( time2>time1     || !time1);
  int units_to_next_flux=time2-time1; 
  // this takes care of weak bits (?)
  //int wobble=(rand()%4)-2; // a bit simplistic? 
  int wobble=(rand()%5)-2;
  units_to_next_flux+=wobble;
  return units_to_next_flux;    
}


WORD TImageSCP::UsToNextFlux(int units_to_next_flux) {
  WORD us_to_next_flux;
  WORD ref_us=((units_to_next_flux/40)+1)&0xFE;  // eg 4
  WORD ref_units=ref_us*40;
  if(units_to_next_flux<ref_units-SCP_DATA_WINDOW_TOLERANCY)
    us_to_next_flux=ref_us-1;
  else if(units_to_next_flux>ref_units+SCP_DATA_WINDOW_TOLERANCY)
    us_to_next_flux=ref_us+1;
  else
    us_to_next_flux=ref_us;
  return us_to_next_flux;
}


WORD TImageSCP::GetMfmData(WORD position) {
/*  We use the same interface for SCP as for STW so that integration
    with the Disk manager, WD1772 emu etc. is straightforward.
    But precise emulation doesn't send MFM data word by word (16bit).
    Instead it sends bytes and AM signals according to bit sequences,
    as analysed in (3rd party-inspired) Fdc.ShiftBit().
    
    TSF314.Read() -> SCP.GetMfmData() -> Fdc.Dpll.GetNextBit() 
    -> SCP.GetNextTransition()
*/
  WORD mfm_data=0;
  if(!TimeFromIndexPulse) //safety, SCP track in ram?
    return mfm_data;
  // must compute new starting point?
  if(position!=0xFFFF)
    ComputePosition();
  // we manage timing here, maybe we should do that in Fdc instead
  COUNTER_VAR a1=Fdc.Dpll.ctime,a2,tm=0;
  // clear dsr signals
  Fdc.Amd.aminfo&=~(CAPSFDC_AI_DSRREADY|CAPSFDC_AI_DSRAM|CAPSFDC_AI_DSRMA1);
  // loop until break
  for(int i=0; ;i++) 
  {
    int bit=Fdc.Dpll.GetNextBit(tm,DRIVE); //tm isn't used...
    //ASSERT(bit==0 || bit==1); // 0 or 1, clock and data
    TRACE_MFM("%d",bit); // full flow of bits
    if(Fdc.ShiftBit(bit)) // true if byte ready to transfer
      break;
  }//nxt i
  //Fdc.Mfm.data_last_bit=(mfm_data&1); // no use
  a2=Fdc.Dpll.ctime;
  COUNTER_VAR delay_in_cycles=(a2-a1);
#ifdef SSE_DEBUG  // only report DPLL if there's some adjustment
  if(Fdc.Dpll.increment!=128|| Fdc.Dpll.phase_add||Fdc.Dpll.phase_sub
    ||Fdc.Dpll.freq_add||Fdc.Dpll.freq_sub)
  {
    //ASSERT( !(Fdc.Dpll.freq_add && Fdc.Dpll.freq_sub) ); 
    //ASSERT( !(Fdc.Dpll.phase_add && Fdc.Dpll.phase_sub) );
    TRACE_MFM(" DPLL (%d,%d,%d) ",Fdc.Dpll.increment,Fdc.Dpll.phase_add-Fdc.Dpll.phase_sub,Fdc.Dpll.freq_add-Fdc.Dpll.freq_sub);
  }
  //ASSERT(delay_in_cycles>0);
  TRACE_MFM(" %d cycles\n",delay_in_cycles);
#endif
  Fdc.update_time=Fdc.current_time+delay_in_cycles; 
  if(Fdc.update_time-ACT<=0) // safety
    Fdc.update_time=ACT+16;
  //ASSERT(!mfm_data); // see note at top of function
  mfm_data=Fdc.Mfm.encoded; // correct?
  return mfm_data;
}


int TImageSCP::GetNextTransition(WORD& us_to_next_flux) {
  int t=UnitsToNextFlux(Position);
  us_to_next_flux=UsToNextFlux(t); // in parameter
  IncPosition();
  t/=5; // in cycles
  return t; 
}


void TImageSCP::IncPosition() {
  //ASSERT( Position>=0 );
  //ASSERT( Position<nBits );
  Position++;
  if(Position>=nBits)
  {
    TRACE_WD("\nSCP Position %d triggers IP side %d track %d rev %d/%d\n",Position,CURRENT_SIDE,CURRENT_TRACK,rev+1,file_header.IFF_NUMREVS);
    Position=0;
/*  If a sector is spread over IP, we make sure that our event
    system won't start a new byte before returning to current
    byte. 
*/
    FloppyDrive[DRIVE].IndexPulse(true); // true for "image triggered"
    // provided there are >1 revs...    
    if(file_header.IFF_NUMREVS>1)
    {
      // we step revs 0->1 each IP, we'll reload 0 during rev
      // Notice we do no computing, the first bit of the new rev
      // is relative to last bit of previous rev, or we are very
      // lucky.
      LoadTrack(CURRENT_SIDE,CURRENT_TRACK,true);
    }      
  }
}


void TImageSCP::Init() {
  fCurrentImage=NULL;
  TimeFromIndexPulse=NULL;
  N_SIDES=2;
  N_TRACKS=83; //max
  nBytes=DISK_BYTES_PER_TRACK; //not really pertinent (TODO?)
}


bool TImageSCP::LoadTrack(BYTE side,BYTE track,bool reload) {
  //ASSERT(Id==0||Id==1);
  bool ok=false;
  //ASSERT( side<2 && track<N_TRACKS ); // unique side may be 1
  if(side>=2 || track>=N_TRACKS)
    return ok; //no crash
  BYTE trackn=track;
  if(N_SIDES==2) // general case
    trackn=track*2+side; 
  if(track_header.TDH_TRACKNUM==trackn //already loaded
    && !rev && (!reload||file_header.IFF_NUMREVS==1))
    return true;
  if(TimeFromIndexPulse) 
    free(TimeFromIndexPulse);
  TimeFromIndexPulse=NULL;
  int offset=file_header.IFF_THDOFFSET[trackn]; // base = start of file
  if(fCurrentImage) // image exists
  {  
    fseek(fCurrentImage,offset,SEEK_SET);
    int size=sizeof(TSCP_track_header);
    fread(&track_header,size,1,fCurrentImage);
    // Determine which track rev to load (we go through all available revs)
    if(reload)
      rev++;
    else
      rev=0;
    rev%=file_header.IFF_NUMREVS;
    WORD* flux_to_flux_units_table_16bit=(WORD*)calloc(track_header.\
      TDH_TABLESTART[rev].TDH_LENGTH,sizeof(WORD));
    TimeFromIndexPulse=(DWORD*)calloc(track_header.TDH_TABLESTART[rev].\
      TDH_LENGTH,sizeof(DWORD));
    //ASSERT(flux_to_flux_units_table_16bit && TimeFromIndexPulse);
    if(flux_to_flux_units_table_16bit && TimeFromIndexPulse)
    {
      fseek(fCurrentImage,offset+track_header.TDH_TABLESTART[rev].TDH_OFFSET,
        SEEK_SET);
      // read only a 8bit table if specified
      size_t encoding_bytes = (file_header.IFF_ENCODING == 8) ? 1 : 2;
      fread(flux_to_flux_units_table_16bit,encoding_bytes,track_header.\
        TDH_TABLESTART[rev].TDH_LENGTH,fCurrentImage);
      // randomise distance from IP of whole track (War Heli)
      // int units_from_ip=reload?0:(rand()%0xb0); // too much? (Audio Sculpture)
      //int units_from_ip=reload?0:(rand()%0x20);
      int units_from_ip=reload?0:(rand()%0x16);
      nBits=0;
      // convert to time after IP, one data per bit (SLOW)
      WORD data=0;
      // ASSERT(!(side==0&&track==0&&i==track_header.TDH_TABLESTART[rev].TDH_LENGTH-1));// last data is 0!
      // probably doesn't work but won't break normal images
      if(file_header.IFF_RESOLUTION)
      {
        for(DWORD i=0;i<track_header.TDH_TABLESTART[rev].TDH_LENGTH;i++)
        {
          data=flux_to_flux_units_table_16bit[i]*(file_header.IFF_RESOLUTION+1);
          //ASSERT(units_from_ip+data>=units_from_ip);
          units_from_ip+=(data)?data:0xFFFF;
          //ASSERT(units_from_ip<0x7FFFFFFF); // max +- 200,000,000, OK
          if(data)
            TimeFromIndexPulse[nBits++]=units_from_ip;
        }
      }
      else
      {
        for(DWORD i=0;i<track_header.TDH_TABLESTART[rev].TDH_LENGTH;i++)
        {
          data=flux_to_flux_units_table_16bit[i];
          SWAP_BIG_ENDIAN_WORD(data); // reverse endianess first
          //ASSERT(units_from_ip + flux_to_flux_units_table_16bit[i] >= units_from_ip);
          units_from_ip+=(data) ? data : 0xFFFF;
          //ASSERT(units_from_ip < 0x7FFFFFFF); // max +- 200,000,000, OK
          if(data)
            TimeFromIndexPulse[nBits++] = units_from_ip;
        }
      }
      // check if we end on a 0 data (means we need info from next track!)
      // eg finale-overlander_rev5_smd340
      if(!data) 
      {
        BYTE nextrev=(rev+1)%file_header.IFF_NUMREVS;
        fseek(fCurrentImage,offset+track_header.TDH_TABLESTART[nextrev].TDH_OFFSET,
        SEEK_SET);
        // we don't read more than what we can take, should be enough!
        fread(flux_to_flux_units_table_16bit,encoding_bytes,track_header.\
        TDH_TABLESTART[rev].TDH_LENGTH,fCurrentImage);
        {
          for(DWORD i=0;!data;i++) // exit as soon as we have the transition
          {
            data=flux_to_flux_units_table_16bit[i];
            if(file_header.IFF_RESOLUTION)
              data*=(file_header.IFF_RESOLUTION+1);
            else
            {
              SWAP_BIG_ENDIAN_WORD(data);
            }
            units_from_ip+=(data) ? data : 0xFFFF;
            if(data)
              TimeFromIndexPulse[nBits++] = units_from_ip;
          }
        }
        TRACE_LOG("SCP %d-%d last flux %d\n",side,track,TimeFromIndexPulse[nBits-1]);
      }
      free(flux_to_flux_units_table_16bit);
      ok=true;
    }
    FloppyDisk[Id].current_side=side;
    FloppyDisk[Id].current_track=track;
    // debug info!
    TRACE_LOG("SCP LoadTrack side %d track %d %c%c%c %d rev %d/%d INDEX TIME %d (%f ms) TRACK LENGTH %d bits %d last bit unit %d DATA OFFSET %d  checksum %X\n",side,track,track_header.TDH_ID[0],track_header.TDH_ID[1],track_header.TDH_ID[2],track_header.TDH_TRACKNUM,rev+1,file_header.IFF_NUMREVS,track_header.TDH_TABLESTART[rev].TDH_DURATION,(float)track_header.TDH_TABLESTART[rev].TDH_DURATION*25/1000000,track_header.TDH_TABLESTART[rev].TDH_LENGTH, nBits,TimeFromIndexPulse[nBits-1],track_header.TDH_TABLESTART[rev].TDH_OFFSET,track_header.track_data_checksum);
  }
  return ok;
}


bool TImageSCP::Open(char *path) {
  bool ok=false;
  //if(!SSEConfig.IsInit)    return ok;
  Close(); // make sure previous image is correctly closed
  fCurrentImage=fopen(path,"rb");
  if(fCurrentImage) // image exists
  {
    // we read only the header
    if(fread(&file_header,sizeof(TSCP_file_header),1,fCurrentImage))
    {
      if(!strncmp(DISK_EXT_SCP,(char*)&file_header.IFF_ID,3)) // it's SCP
      {
        // compute N_SIDES and N_TRACKS
        if(file_header.IFF_HEADS)
        {
          N_SIDES=1;
          N_TRACKS=file_header.IFF_END-file_header.IFF_START+1;
        }
        else
          N_TRACKS=(file_header.IFF_END-file_header.IFF_START+1)/2;
#if defined(SSE_DEBUG)
          TRACE_LOG("SCP %d sides %d tracks %d IFF_VER %X IFF_DISKTYPE %X IFF_NUMREVS %d IFF_START %d IFF_END %d IFF_FLAGS %d IFF_ENCODING %d IFF_HEADS %d IFF_RESOLUTION %X IFF_CHECKSUM %X\n",Id,N_SIDES,N_TRACKS,file_header.IFF_VER,file_header.IFF_DISKTYPE,file_header.IFF_NUMREVS,file_header.IFF_START,file_header.IFF_END,file_header.IFF_FLAGS,file_header.IFF_ENCODING,file_header.IFF_HEADS,file_header.IFF_RESOLUTION,file_header.IFF_CHECKSUM);
#endif
        track_header.TDH_TRACKNUM=0xFF;
        ok=true; //TODO some checks?
      }//cmp
    }//read
  }
  if(!ok)
    Close();
  else 
    FloppyDrive[Id].MfmManager=this;
  return ok;
}


//#pragma warning(disable:4100)//unreferenced formal parameter

void TImageSCP::SetMfmData(WORD /*position*/, WORD /*mfm_data*/) {
  // :) TODO
}

//#pragma warning(default:4100)


#endif//#if defined(SSE_DISK_SCP)
