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
FILE: disk_dma.cpp
DESCRIPTION: The DMA (Direct Memory Access) chip was developed by Atari.
It uses two 16-byte FIFO buffers to handle byte transfers between the 
drive  controller (floppy or hard disk) and memory so that the CPU is
freed from the task. Memory access is done by the MMU upon DMA request, 
as arbitrated by the GLUE. The DMA can't address the bus.
The CPU must free the bus for each 16-byte transfer. That's a lot of
transfers when you're using a hard drive, and it slows down the ST's CPU.
We don't very precisely emulate bus arbitration, unlike with the blitter 
emulation, because it's not as important. Disk has the highest bus priority
so that no byte is missed, in Steem there's no failure either.
We count DMA cycles only if option ADAT is set. In some previous versions
of Steem, nothing was counted and it broke nothing. In fact, counting
cycles at imprecise timings like we do is likelier to break programs!
Precision of transfer timing depends on the image type.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <debug.h>
#include <computer.h>
#include <osd.h>
#if USE_PASTI
#include <pasti/pasti.h>
#endif
#ifdef SSE_ACSI
#include <hd_acsi.h>
#include <harddiskman.h>
#endif
#if defined(SSE_DISK_CAPS)
#include <caps/CapsPlug.h> // 3rd party
#endif
#if defined(SSE_DISK_GHOST)
#include <disk_ghost.h>
#endif

#define LOGSECTION LOGSECTION_DMA


TDma::TDma() { // note: no reset pin on that chip
  Request=false;
  BufferInUse=0;
}


void TDma::AddToFifo(BYTE data) {
  // DISK -> FIFO -> RAM = 'read disk' = 'write dma'
  Fifo[BufferInUse][Fifo_idx++]=data;
  if(Fifo_idx==16)
    RequestTransfer();
}


/*  This just transfers one byte between controller's DR and Fifo at request.
    Direction depends on control register, function checks it itself.
    Note that bits 6 (CR_DISABLE) and 7 (CR_DRQ_FDC_OR_HDC) don't count,
    DMA transfer will happen whatever their value.
    This is observed for example in Kick Off 2 (IPF).
    That's pretty strange when you consider that the DMA chip was designed and
    documented by Atari, but then one needs to consider development speed (aka
    the rush to market).
*/

void TDma::Drq() {
  sr|=SR_DRQ;
  if(mcr&CR_WRITE) // RAM -> disk (writing to disk)
  {
    if(!(mcr&CR_HDC_OR_FDC)) //floppy select
      Fdc.dr=GetFifoByte();
#if defined(SSE_ACSI)
    else if(ACSI_EMU_ON)
      AcsiHdc[acsi_dev].DR=GetFifoByte();
#endif
  }
  else // disk->RAM (reading disk)
  {
   if(!Counter)
     ; // ignore
   else
    if(!(mcr&CR_HDC_OR_FDC))
      AddToFifo(Fdc.dr);
#if defined(SSE_ACSI)
    else if(ACSI_EMU_ON)
      AddToFifo(AcsiHdc[acsi_dev].DR);
#endif
  }
  sr&=~SR_DRQ;
}


/*  For writing to disk, we use the FIFO in the other direction,
    because we want it to fill up when empty.
    On a real ST, the first DMA transfer happens right when the
    sector count has been set.
    This could be important in theory, but know no case, to simplify
    we only request byte when drive is ready.
    TODO
*/

BYTE TDma::GetFifoByte() {
  // RAM -> FIFO -> DISK = 'write disk' = 'read dma'
  if(!Fifo_idx)
    RequestTransfer();
  BYTE data=Fifo[BufferInUse][--Fifo_idx];
  return data;  
}


void TDma::UpdateRegs(
#if defined(SSE_DEBUG)
                      bool trace_them
#endif
                      ) {
/*  This is used for debugging and for pasti drive led
    and may have some other uses.
    We update both DMA and FDC registers (so we use old variable names for the
    latter) - we could have a TWD1772::UpdateRegs() as well.
*/
  if(Request) // OSD sabotaging data! eg Arctic Fox STX C3, nasty
    return;
#if USE_PASTI
  if(hPasti && (pasti_active || FloppyDrive[DRIVE].ImageType.Extension==EXT_STX)
#if defined(SSE_DISK_GHOST)
    &&!(OPTION_GHOST_DISK&&Fdc.Lines.CommandWasIntercepted)
#endif
    )
  {
    pastiPEEKINFO ppi;
    pasti->Peek(&ppi);
    Fdc.cr=ppi.commandReg;
    Fdc.str=ppi.statusReg;
    Fdc.tr=ppi.trackReg;
    Fdc.sr=ppi.sectorReg;
    Fdc.dr=ppi.dataReg;
    FloppyDrive[0].track=ppi.drvaTrack;
    FloppyDrive[1].track=ppi.drvbTrack;
    Counter=(WORD)ppi.dmaCount;
    dma_address=ppi.dmaBase;
    mcr=(WORD)ppi.dmaControl;
    sr=(BYTE)ppi.dmaStatus;
  }
#endif
#if defined(SSE_DISK_CAPS)
  if(CAPSIMG_OK && FloppyDrive[DRIVE].ImageType.Manager==MNGR_CAPS)
  {
    int ext=0;
    Fdc.cr=(BYTE)CapsFdcGetInfo(cfdciR_Command, &Caps.fdc,ext);
    Fdc.str=(BYTE)CapsFdcGetInfo(cfdciR_ST, &Caps.fdc,ext);
/*  when disk A is non IPF and disk B is IPF, we may get bogus
    motor on, how to fix that? - rare anyway
    TODO
*/
    if(!(Fdc.str&FDC_STR_MO)) // assume this drive!
      FloppyDrive[DRIVE].motor=false;
    Fdc.tr=(BYTE)CapsFdcGetInfo(cfdciR_Track, &Caps.fdc,ext);
    Fdc.sr=(BYTE)CapsFdcGetInfo(cfdciR_Sector, &Caps.fdc,ext);
    Fdc.dr=(BYTE)CapsFdcGetInfo(cfdciR_Data, &Caps.fdc,ext);
    FloppyDrive[0].track=(BYTE)Caps.Drive[0].track;
    FloppyDrive[1].track=(BYTE)Caps.Drive[1].track;
  } 
#endif
#if defined(SSE_DEBUG)
  if(trace_them)
  {
    if(mcr&CR_HDC_OR_FDC)
      TRACE_HDC("HDC IRQ\n");
    else
    {
      //ASSERT(Fdc.str);
      TRACE_FDC("%c:" PRICV " FDC(%d) IRQ CR %X STR %X ",'A'+DRIVE,ACT,FloppyDrive[DRIVE].ImageType.Manager,Fdc.cr,Fdc.str);
      Fdc.TraceStatus();
      TRACE_FDC("TR %d (CYL %d) sr %d DR %d\n",Fdc.tr,FloppyDrive[DRIVE].track,Fdc.sr,Fdc.dr);
    }
  }
#endif
#if defined(SSE_DEBUGGER_OSD_CONTROL) //finally done this
  if(OSD_MASK1 & OSD_CONTROL_FDC)
  {
    if((Fdc.str&0x10))
      TRACE_OSD("RNF"); 
    else if((Fdc.str&0x08))
      TRACE_OSD("CRC"); 
  }
#endif
}


/*  "The count should indicate the number of 512 bytes chunks that 
    the DMA will have to transfer
    "This register is decrement by one each time 512 bytes has been
    transferred. When the sector count register reaches zero the DMA
    will stop to transfer data. Only the lower 8 bits are used."
    -> it's always 512 bytes, regardless of sector size
*/

void TDma::IncAddress() {
  if(Counter&0xFF) 
  {
    dma_address+=2;
    ByteCount+=2;
    if(ByteCount>=512)
    {
#if defined(SSE_STATS) // safer to check all here at SR
      if(!(mcr&CR_HDC_OR_FDC)) //floppy select
        Stats.nSector2[DRIVE]++;
#endif
      ByteCount=0;
      Counter--;
      if(Counter&0xFF)
        sr|=SR_COUNT;  // DMA sector count not 0
      else
        sr&=~SR_COUNT;
    }
  }
}


void TDma::RequestTransfer() {
  // we make this function to avoid code duplication
  Request=true; 
  Fifo_idx= (mcr&CR_WRITE) ? 16 : 0;
  BufferInUse=!BufferInUse; // toggle 16byte buffer
  TransferBytes();
}


void TDma::TransferBytes() { //Execute the DMA transfer. Count cycles if needed.
  //ASSERT( Request );
  //ASSERT(!(mcr&CR_RESERVED)); // reserved bits
  // like for the blitter, we must save cpu's internals (TODO)
  MEM_ADDRESS save_iabus=iabus;
  MEM_ADDRESS save_abus=abus;
  WORD save_dbus=dbus;
  BYTE save_mask=BUS_MASK;
/*  Computing the checksum if it's bootsector.
    We do it here in DMA because it should work with native, Pasti, CAPS,
    MFM. Both sides for Freeboot.
*/
  if(Fdc.cr==0x80 && !Fdc.tr && Fdc.sr==1 && !(mcr&CR_WRITE))
  {
    for(int i=0;i<16;i+=2)
    {
      FloppyDrive[DRIVE].SectorChecksum+=Fifo[!BufferInUse][i]<<8; 
      FloppyDrive[DRIVE].SectorChecksum+=Fifo[!BufferInUse][i+1]; 
    }
  }
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
  Datachunk++; 
  if(TRACE_MASK3 & TRACE_CONTROL_FDCBYTES)
#ifdef SSE_ACSI
    if((mcr&CR_HDC_OR_FDC))
      TRACE_LOG("#%03d (%d) %s %06X: ",Datachunk, AcsiHdc[acsi_dev].SectorNum(),(mcr&0x100)?"from":"to",dma_address);
    else
#endif
    {
      if(TRACE_MASK3 & TRACE_CONTROL_FDCREGS) // + timing
      //TRACE_LOG("#%03d (%d-%02d-%02d) %s %06X: ",Datachunk,floppy_current_side(),FloppyDrive[DRIVE].track,Fdc.CommandType()==2?Fdc.sr:0,(mcr&0x100)?"from":"to",dma_address);
        TRACE_LOG("%d #%03d (%d-%02d-%02d) %s %06X: ",ACT-Dma.last_act,Datachunk,floppy_current_side(),FloppyDrive[DRIVE].track,Fdc.CommandType()==2?Fdc.sr:0,(mcr&0x100)?"from":"to",dma_address);
      else
        TRACE_LOG("#%03d (%d-%02d-%02d) %s %06X: ",Datachunk,floppy_current_side(),FloppyDrive[DRIVE].track,Fdc.CommandType()==2?Fdc.sr:0,(mcr&0x100)?"from":"to",dma_address);
    }
  Dma.last_act=ACT; // for timing above
#endif
  bool count_cycles=(OPTION_COUNT_DMA_CYCLES!=0)
#if 0 // testing also if fast!
    && ((!(mcr&CR_HDC_OR_FDC) && ADAT) // floppy select
#if defined(SSE_ACSI)
    || (ACSI_EMU_ON && (mcr&CR_HDC_OR_FDC) && !floppy_instant_sector_access)) // HD select
#endif
#endif
    ;
  BYTE save_blitter_request=Blitter.Request;
  Blitter.Request=false; // disk has higher priority (Antiques MFM)
  //if(count_cycles) CPU_BUS_IDLE(4); // is there arbitration delay?
  for(int i=0;i<8;i++) // burst, 8 word packets
  {
    if(!(mcr&0x100)&& DMA_ADDRESS_IS_VALID_W) // disk -> RAM
    {
      dbush=Fifo[!BufferInUse][i*2];
      dbusl=Fifo[!BufferInUse][i*2+1];
      if(count_cycles)
      {
        iabus=dma_address; //temp
//        ASSERT(!Stvl.busy); // with CAPS disks
        DMA_BUS_ACCESS_WRITE;
      }
      else
        DPEEK(dma_address)=dbus;
    }
    else if((mcr&0x100) && DMA_ADDRESS_IS_VALID_R) // RAM -> disk
    {
      if(count_cycles)
      {
        iabus=dma_address; //temp
        DMA_BUS_ACCESS_READ;
      }
      else
      {
        dbush=PEEK(dma_address);
        dbusl=PEEK(dma_address+1);
      }
      Fifo[BufferInUse][14-i*2]=dbusl; // reverse order!
      Fifo[BufferInUse][14-i*2+1]=dbush;
    }
    else 
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
      if(TRACE_MASK3 & TRACE_CONTROL_FDCBYTES)
#endif
        TRACE_LOG("!!!");
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
    if(TRACE_MASK3 & TRACE_CONTROL_FDCBYTES)
      TRACE_LOG("%02X %02X ",Fifo[(mcr&CR_WRITE)?BufferInUse:!BufferInUse][(mcr&CR_WRITE)?14-i*2:i*2],
        Fifo[(mcr&CR_WRITE)?BufferInUse:!BufferInUse][(mcr&CR_WRITE)?14-i*2+1:i*2+1]);
#endif
#if USE_PASTI    
  if(hPasti&&(pasti_active||FloppyDrive[DRIVE].ImageType.Extension==EXT_STX))
    dma_address+=2;
  else
#endif
    IncAddress();
  }//nxt
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
  if(TRACE_MASK3 & TRACE_CONTROL_FDCBYTES)
    TRACE_LOG("\n");
#endif
  Blitter.Request=save_blitter_request;
  //if(count_cycles) CPU_BUS_IDLE(4); // is there arbitration delay?
#if defined(SSE_DEBUGGER)
  // write the DMA transfer in history
  pc_history_y[pc_history_idx]=scan_y;
  pc_history_c[pc_history_idx]=(short)LINECYCLES;
  pc_history[pc_history_idx++]=0x12346789;
  if(pc_history_idx>=HISTORY_SIZE)
    pc_history_idx=0;
  if(!count_cycles) for(int i=0;i<8;i++) // for Debugger monitor, intercept DMA traffic
  {
    if(!(mcr&0x100)&& DMA_ADDRESS_IS_VALID_W) // disk -> RAM
    {DEBUG_CHECK_WRITE_W(dma_address-16+i*2)}
    else if((mcr&0x100) && DMA_ADDRESS_IS_VALID_R) // RAM -> disk
    {DEBUG_CHECK_READ_W(dma_address-16+i*2)}
  }
#endif
  iabus=save_iabus;
  abus=save_abus;
  dbus=save_dbus;
  BUS_MASK=save_mask;
  Request=FALSE;
}
