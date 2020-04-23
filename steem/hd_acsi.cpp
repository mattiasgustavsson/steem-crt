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

DOMAIN: hard drive
FILE: hd_acsic.cpp
CONDITION: SSE_ACSI must be defined
DESCRIPTION: A sector-based emulation of Atari ACSI (Atari  Computer System
Interface) hard disks like the SH204 or the Megafile 60.
ACSI was much as SCSI, not as good, cheaper, but effective.
This emualtion is based on "Atari ACSI/DMA Integration guide", June 28, 1991
(no copy/paste of code this time, it's for some fun).
Emulation is straightforward: just fetch sector #n, all seem to be 512 bytes. 
It was more difficult to adapt the GUI (hard disk manager...).

TODO: extend to "ICD" or such SCSI commands
---------------------------------------------------------------------------*/


#include "pch.h"
#pragma hdrstop

#if defined(SSE_ACSI)

#include <run.h>
#include <mymisc.h>
#include <osd.h>
#include <steemh.h>
#include <acc.h>
#include <hd_acsi.h>
#include <debug.h>
#include <computer.h>


#define BLOCK_SIZE 512 // fortunately it seems constant

BYTE acsi_dev=0; // active device


TAcsiHdc::TAcsiHdc() {
  hard_disk_image=NULL;
  Active=false;
}


TAcsiHdc::~TAcsiHdc() {
  CloseImageFile();
}


void TAcsiHdc::CloseImageFile() {
  if(hard_disk_image)
    fclose(hard_disk_image);
  hard_disk_image=NULL;
  Active=false;
}


void TAcsiHdc::Format() { 
/*  For fun. Fill full image with $6C.
    We do this sector by sector because otherwise it can be really slow
    and Steem looks hanged ("not responding") for a while, and because 
    we dont want to add agendas on the other hand.
*/
  BYTE sector[BLOCK_SIZE];
  memset(sector,0x6c,BLOCK_SIZE);
  //ASSERT(hard_disk_image);
  fseek(hard_disk_image,0,SEEK_SET); //restore
  //ASSERT(nSectors>0);
  for(int i=0;i<nSectors;i++)
    fwrite(sector,BLOCK_SIZE,1,hard_disk_image); //fill sectors
}


bool TAcsiHdc::Init(int num, char *path) {
  //ASSERT(num<MAX_ACSI_DEVICES);
  CloseImageFile();
  //ASSERT(inquiry_string);
  memset(inquiry_string,0,32);
  hard_disk_image=fopen(path,"rb+");
  Active=(hard_disk_image!=NULL); // file is there or not
  if(Active) // note it could be anything, even HD6301V1ST.img ot T102.img
  {
    int l=GetFileLength(hard_disk_image); //in bytes - int is enough
    nSectors=l/BLOCK_SIZE;
   //ASSERT(!(l%BLOCK_SIZE) && nSectors>=20480 && device_num>=0 && device_num<MAX_ACSI_DEVICES); // but we take it?
    device_num=num&7;
    char *filename=GetFileNameFromPath(path);
    char *dot=strrchr(filename,'.');
    int nchars=(int)(dot ? (dot-filename) : 23);
    //ASSERT(nchars>0);
    //ASSERT(inquiry_string);
    strncpy(inquiry_string+8,filename,nchars);
#ifdef SSE_DEBUG     //395, examine MBR
    TRACE_HDC("ACSI %d ID %s phys sectors %d",device_num,inquiry_string+8,nSectors);
    fseek(hard_disk_image, 0x1c2, SEEK_SET);
    //BYTE siz[4];
    int siz;
    BYTE p_flg;
    BYTE p_id[3];
    BYTE p_st[4];
    fread(&siz, 4, 1, hard_disk_image);
    SWAP_BIG_ENDIAN_DWORD(siz);
    TRACE_HDC(" MBR %d sectors (%d MB)\nPartitions",siz,siz/(2*1024));
    for (int i = 0;i < 4;i++)
    {
      fread(&p_flg, 1, 1, hard_disk_image);
      fread(&p_id, 1, 3, hard_disk_image);
      fread(&p_st, 1, 4, hard_disk_image);
      fread(&siz, 4, 1, hard_disk_image);
      SWAP_BIG_ENDIAN_DWORD(siz);
      TRACE_HDC(" %d:%X %s %X%X%X%X %d",i,p_flg,p_id,p_st[0],p_st[1],p_st[2],p_st[3],siz);
    }
    TRACE_HDC("\n");
#endif
    acsi_dev=device_num;
  }
  //TRACE_INIT("ACSI %d open %s %d sectors %d MB\n",device_num,path,nSectors,nSectors/(2*1024));
  return (Active!=0);
}


BYTE TAcsiHdc::IORead() {
  BYTE ior_byte=0;
  if((Dma.mcr&0xFF)==0x8a) // "read status"
    ior_byte=STR;
  //TRACE_HD("ACSI PC %X read %X %X = %X\n",old_pc,Line,Dma.mcr,ior_byte);
  Irq(false);
  Active=1;
  return ior_byte;
}


void TAcsiHdc::IOWrite(BYTE Line,BYTE io_src_b) {
  if(!hard_disk_image)
    return;
  //TRACE_HD("ACSI PC %X write %X = %X\n",old_pc,Line,io_src_b);
  bool do_irq=false;
  // take new command only if A1 is low, it's our ID and we're ready
  // A1 in ACSI doc is A0 in DMA doc
  if(!Line && (io_src_b>>5)==device_num && cmd_ctr==7)
  {
    cmd_ctr=0;
    io_src_b&=0x1f;
    acsi_dev=device_num; // we have the bus
    //ASSERT(acsi_dev<MAX_ACSI_DEVICES);
  }
  if(cmd_ctr<6) // getting command
  {
    cmd_block[cmd_ctr]=io_src_b;
    //ASSERT( Line || !cmd_ctr); //asserts on buggy drivers, but it should work if there's only 1 device
    cmd_ctr++;
    do_irq=true;
  }
  if(cmd_ctr==6) // command in
  {
    TRACE_HDC("ACSI %d command §%X (%X %X %X %X %X)\n",device_num,cmd_block[0],cmd_block[1],cmd_block[2],cmd_block[3],cmd_block[4],cmd_block[5]);
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
    Dma.Datachunk=0;
#endif
    STR=0; // all fine
    switch(*cmd_block) {
      case 0x00: //ready
        break;
      case 0x03: //request sense 
        DR=error_code;
        Dma.Drq();
        DR=0;
        Dma.Drq();
        Dma.Drq();
        Dma.Drq();
        break;
      case 0x04: //format
        Format();
        break;
      case 0x08: //read
        ReadWrite(false,cmd_block[4]);
        break;
      case 0x0a: //write
        ReadWrite(true,cmd_block[4]);
        break;
      case 0x0b: //seek
        Seek();
        break;
      case 0x12: //inquiry
        Inquiry();
        break;
      case 0x15: //SCSI mode select
        {
          TRACE_HDC("Mode select (%d) %d %x\n",cmd_block[4],Dma.Counter,dma_address);
          for(int i=0;i<cmd_block[4];i++)
            Dma.Drq(); //do nothing with it?
        }
        break;
      default: //other commands
        STR=2;
        error_code=0x20; //invalid opcode
    }//sw
#ifdef SSE_DEBUG
    if(STR&2)
      TRACE_HDC("ACSI error STR %X error code %X\n",STR,error_code);
#endif
    cmd_ctr++;
    //ASSERT(cmd_ctr<8);
    HDDisplayTimer=timer+HD_TIMER; // simplistic
    // some delay... 1MB/s 512bytes/ 0.5ms
    if(!floppy_instant_sector_access&&(*cmd_block==8 || *cmd_block==0xa)&&!STR)
    {
      time_of_irq=ACT+cmd_block[4]*4000;
      Active=2; // signal for ior
      HDDisplayTimer+=cmd_block[4]/2;
    }
    else
      Active=1;
  }
  if(do_irq)
    Irq(true);
}


void TAcsiHdc::Inquiry() {//drivers display this so we have a cool name
  TRACE_HDC("Inquiry: %s\n",inquiry_string+8); //strange...
  for(int i=0;i<32;i++)
  {
    DR=inquiry_string[i];
    Dma.Drq();
  }
}


void TAcsiHdc::Irq(bool state) {
  mfp_gpip_set_bit(MFP_GPIP_FDC_BIT,!state); // MFP GPIP is "active low"
}


void TAcsiHdc::ReadWrite(bool write,BYTE block_count) {
  //ASSERT(block_count);
  TRACE_HDC("%s sectors %d-%d (%d)\n",write?"Write":"Read",SectorNum(),SectorNum()+block_count-1,block_count);
#if defined(SSE_STATS)
  Stats.nHdsector+=block_count;
#endif
  size_t ok=Seek();
// read or write done in one pass, no delay, which messes DMA timings and
// can cause glitches - TODO
  bool tmp=floppy_instant_sector_access;
  NOT_ADVANCED_BEGIN
    if(OPTION_HACKS)//v402
      floppy_instant_sector_access=true;
  ADVANCED_END 
  for(int i=0;ok&&i<block_count;i++)
  {
    for(int j=0;ok&&j<BLOCK_SIZE;j++)
    {
      //ASSERT(hard_disk_image);
      if(write)
      {
        Dma.Drq(); // get byte write from DMA
        ok=fwrite(&DR,1,1,hard_disk_image);
      }
      else if((ok=fread(&DR,1,1,hard_disk_image))!=0) // fails when driver tests size
        Dma.Drq(); // put byte on DMA - if there is one
    }//j
  }//i
  if(!ok)
    STR=2;
  NOT_ADVANCED_BEGIN
    floppy_instant_sector_access=tmp;
  ADVANCED_END 
}


void TAcsiHdc::Reset() {
  cmd_ctr=7; // "ready"; we don't restore
}


int TAcsiHdc::SectorNum() {
  int block_number=(cmd_block[1]<<16) + (cmd_block[2]<<8) + cmd_block[3];
  //block_number&=0x1FFFFF; //limit is?
  return block_number;
}


bool TAcsiHdc::Seek() {
 int block_number=SectorNum();
 if(fseek(hard_disk_image,block_number*BLOCK_SIZE,SEEK_SET))
   STR=2;
 return (STR!=2); // that would mean "OK"
}

#endif//#if defined(SSE_ACSI)
