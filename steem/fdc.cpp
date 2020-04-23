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
FILE: fdc.cpp
DESCRIPTION: High level emulations of the Western Digital WD1772 floppy
disk controller.
There are two internal emulations, one based on agendas (ST/MSA/DIM, scanline
precision) and one based on events (STW/HFE/SCP, cycle precision).
Steem also interfaces with two more WD1772/disk emulations through this 
object: Pasti (STX) and SPS (IPF/CTR).
Interception for ghost disks (STG) is also done here.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <computer.h>
#include <run.h>
#include <debug.h>
#include <gui.h>
#include <display.h>
#include <osd.h>

#if USE_PASTI
#include <pasti/pasti.h>
HINSTANCE hPasti=NULL;
COUNTER_VAR pasti_update_time;
const struct pastiFUNCS *pasti=NULL;
char pasti_file_exts[PASTI_FILE_EXTS_BUFFERSIZE];
WORD pasti_store_byte_access;
bool pasti_active=false;
#else
const bool pasti_active=false;
#endif


#define DRIVE_HBLS_PER_ROTATION  (ADAT?FloppyDrive[DRIVE].HblsPerRotation():(313*50/5))
#define DRIVE_HBLS_OF_INDEX_PULSE (DRIVE_HBLS_PER_ROTATION/((ADAT)?50:20)) // 4ms =200ms/50

int floppy_mediach[2]={0,0};

BYTE num_connected_floppies=2;
bool floppy_access_started_ff=false;
BYTE floppy_access_ff_counter=0;
BYTE floppy_irq_flag=0;
const BYTE fdc_step_time_to_hbls[4]={94,188,32,47};
WORD floppy_write_track_bytes_done;
BYTE fdc_spinning_up=0;
bool floppy_instant_sector_access=true; // default, let's not discourage people!
bool floppy_access_ff=false;


#define LOGSECTION LOGSECTION_FDC

BYTE floppy_current_drive() {
  if((psg_reg[PSGR_PORT_A]&BIT_1)==0)  // Drive A
    return 0;
  else if((psg_reg[PSGR_PORT_A]&BIT_2)==0) // Drive B
    return 1;
  return 0; // Neither, guess A 
}


BYTE floppy_current_side() {
  return (psg_reg[PSGR_PORT_A]&BIT_0)==0;
}


void write_to_dma(BYTE Val,int Num=1) {
  int n=(Num<=0)? 1 : Num;
  for(int i=0;i<n;i++)
    Dma.AddToFifo(Val);
}


bool floppy_handle_file_error(int floppyno,bool Write,int sector,
                              int PosInSector,bool FromFormat) {
  ASSERT(!(floppyno&0xfffffffe));
  static DWORD last_reinsert_time[2]={0,0};
  TSF314 *floppy=&FloppyDrive[floppyno];
  log_write(EasyStr("File error - re-inserting disk ")+LPSTR(floppyno?"B":"A"));
  bool WorkingNow=0;
  if(timer>=last_reinsert_time[floppyno]+2000&&floppy->DiskInDrive())
  {
    // Over 2 seconds since last failure
    FILE *Dest=NULL;
    if(FromFormat)
    {
      if(FloppyDisk[floppyno].ReopenFormatFile())
        Dest=FloppyDisk[floppyno].Format_f;
      else if(floppy->ReinsertDisk())
        Dest=FloppyDisk[floppyno].f;
    }
    if(Dest)
    {
      if(FloppyDisk[floppyno].SeekSector(floppy_current_side(),
        FloppyDrive[floppyno].track,sector,FromFormat)==0) 
      {
        fseek(Dest,PosInSector,SEEK_CUR);
        BYTE temp;
        if(Write)
        {
          temp=Dma.GetFifoByte();
          WorkingNow=(fwrite(&temp,1,1,Dest)>0);
        }
        else
        {
          WorkingNow=(fread(&temp,1,1,Dest)>0);
          if(DMA_ADDRESS_IS_VALID_W && Dma.Counter) 
            write_to_dma(temp,0);
        }
      }
    }
    else
      GUIDiskErrorEject(floppyno);
  }
  last_reinsert_time[floppyno]=timer;
  return !WorkingNow;
}


bool floppy_track_index_pulse_active() {
  // This function is used by Steem native and by WD1772 manager
  if(ADAT)
  {
    bool active=(Fdc.StatusType && FloppyDrive[DRIVE].motor
      && FloppyDrive[DRIVE].BytePosition()<=125); //4ms = +-125 bytes
    return active;
  }
  else if(Fdc.StatusType && num_connected_floppies!=DRIVE)
    return ((hbl_count) % DRIVE_HBLS_PER_ROTATION)
      >=(DWORD)(DRIVE_HBLS_PER_ROTATION-DRIVE_HBLS_OF_INDEX_PULSE);
  return 0;
}


void fdc_type1_check_verify() {
  if(ADAT)
  {
    if(Fdc.cr&Fdc.CR_V)
    {
      WORD HBLsToNextID;
      BYTE NextIDNum;
      FloppyDisk[DRIVE].NextID(NextIDNum,HBLsToNextID); // C++ references
      HBLsToNextID+=(WORD)milliseconds_to_hbl(15); // head settling
/*  When you boot a ST with no disk into the drive, it will hang for
    a long time then show the desktop with two drive icons.
*/
      if(FloppyDrive[DRIVE].Empty()
#if defined(SSE_MEGASTE) && defined(SSE_DISK_HD) // assume DD/HD mismatch times out
        ||((IS_MEGASTE
          &&((FloppyDisk[DRIVE].Density==2)^((MegaSte.FdHd&BIT_1)==BIT_1))))
#endif            
        )
      {
        TRACE_LOG("No disk %c verify times out\n",'A'+DRIVE);
      }
      else
      {
        Fdc.IndexCounter=0; // 5 REVS to find match
        agenda_add(agenda_fdc_verify,HBLsToNextID,NextIDNum);
      }
    }
    else
      // no verify: delay 'finish' to avoid nasty bugs (European Demos)
      agenda_add(agenda_fdc_verify,2,1); 
    return;
  }
  if((Fdc.cr&Fdc.CR_V)==0)
    return;
  // This reads an ID field and checks that track number matches floppy drive head
  // It will fail on an unformatted track or if there is no disk of course
  int floppyno=floppy_current_drive();
  TSF314 *floppy=&FloppyDrive[floppyno];
  if(FloppyDrive[floppyno].track>FLOPPY_MAX_TRACK_NUM||floppy->Empty())
    Fdc.str|=FDC_STR_SE;
  else if(FloppyDisk[floppyno].TrackIsFormatted[floppy_current_side()]
    [FloppyDrive[floppyno].track]==0) 
  {
    // If track is formatted then it is okay to seek to it, otherwise do this:
    if(FloppyDrive[floppyno].track>=FloppyDisk[floppyno].TracksPerSide) 
      Fdc.str|=FDC_STR_SE;
    if(floppy_current_side()>=FloppyDisk[floppyno].Sides) 
      Fdc.str|=FDC_STR_SE;
  }
  DEBUG_ONLY(if(Fdc.str & FDC_STR_SE) DBG_LOG("     Verify failed (track not formatted)"); )
}


void fdc_command(BYTE cm) {
  DBG_LOG(Str("FDC: ")+HEXSl(old_pc,6)+" - executing command $"+HEXSl(cm,2));
  if(Fdc.str&FDC_STR_BSY)
  {
/*  WD1770/1772 5 1/4 " Floppy Disk Controller/Formatter:
    "This register should not be loaded when the device is busy unless
    the new command is a force interrupt." 
    Later versions of the doc state:
    "This register is not loaded when the device is busy unless the new
    command is a force interrupt."
    The first version is more accurate. It is always possible to change the
    registers but you shouldn't.
    While the disk is still spinning up, it's possible to load a new command (from Hatari).
    Command is decoded after spinup, after 15ms, or almost at once.   
    Ignoring the command is a simplification (a hack, hence the option).
    Froggies: ignore $17 on $80 (motor on)
    Overdrive: replace $00 (motor off) with $13
*/
    if(fdc_spinning_up || !OPTION_HACKS)
    {
      TRACE_LOG("CR %X->%X\n",Fdc.cr,cm);
    }
    else if((cm&0xF0)!=0xD0) // Not force interrupt
    {
      DBG_LOG("     Command ignored, FDC is busy!");
      TRACE_LOG("Command %X ignored\n",cm);
      //Fdc.cr=cm; // should but we can't afford in our emu
      return;
    }
  }
  if(Fdc.InterruptCondition!=8) // see note in IORead()
    mfp_gpip_set_bit(MFP_GPIP_FDC_BIT,true); // Turn off IRQ output
  Fdc.InterruptCondition=0;
  agenda_delete(agenda_fdc_finished);
  floppy_irq_flag=0;
  Fdc.cr=cm;
  agenda_delete(agenda_fdc_motor_flag_off);
  if((Fdc.str&FDC_STR_BSY) && Fdc.CommandType(cm)<4) 
    return;
  // AFAIK the FDC turns the motor on automatically whenever it receives a command.
  // Normally the FDC delays execution until the motor has reached top speed but
  // there is a bit in type 1, 2 and 3 commands that will make them execute
  // while the motor is in the middle of spinning up (BIT_3).
  bool delay_exec=0;
  // no motor if no drive (drive 1 when 0 is max)
  if(num_connected_floppies!=DRIVE && !(Fdc.str&FDC_STR_MO)) 
  {
    if((cm&0xF0)!=0xd0&&!(cm&BIT_3))  // Not force interrupt, delay not disabled
      delay_exec=true; // Delay command until after spinup
    Fdc.str=FDC_STR_BSY|FDC_STR_MO;
    fdc_spinning_up=int(delay_exec?2:1);
    FloppyDrive[DRIVE].motor=true;
    TRACE_LOG("Motor on drive %c\n",'A'+DRIVE);
    if(FloppyDrive[DRIVE].Empty())
      ;  // no IP
    else if(floppy_instant_sector_access)
      agenda_add(agenda_fdc_spun_up,milliseconds_to_hbl(100),delay_exec);
    else
    { //  Set up agenda for next IP
      Fdc.IndexCounter=0;
      DWORD delay=FloppyDrive[DRIVE].HblsNextIndex();
      agenda_add(agenda_fdc_spun_up,delay,delay_exec);
    }
  }
  if(delay_exec==0) 
  {
#if defined(SSE_DRIVE_SOUND)
    FloppyDrive[DRIVE].Sound_CheckCommand(Fdc.cr);
#endif
    fdc_execute();
  }
}


void agenda_fdc_spun_up(int do_exec) {
/*  On the WD1772 all commands, except the Force Interrupt Command,
 are programmed via the h Flag to delay for spindle motor start 
 up time. If the h Flag is not set and the MO signal is low when 
 a command is received, the WD1772 forces MO to a logic 1 and 
 waits 6 revolutions before executing the command. 
 At 300 RPM, this guarantees a one second spindle start up time.
 ->
 We count IP, only if there's a spinning selected drive
 Not emulated: if program changes drive and the new drive is spinning
 too, but IP occurs at different time!
*/
  if(ADAT)
  {
    //ASSERT(Fdc.IndexCounter<6);
    if(Psg.CurrentDrive()!=TYM2149::NO_VALID_DRIVE&&FloppyDrive[DRIVE].motor) //TODO wd's line
      Fdc.IndexCounter++;
    if(Fdc.IndexCounter<6) // not finished
    {
      agenda_add(agenda_fdc_spun_up,DRIVE_HBLS_PER_ROTATION,do_exec);
      return;
    }
  }
#if defined(SSE_DRIVE_SOUND)
  FloppyDrive[DRIVE].Sound_CheckCommand(Fdc.cr);
#endif
  fdc_spinning_up=0;
  TRACE_LOG("FDC Drive spun\n");
  if(do_exec) 
    fdc_execute();
}


void fdc_execute() {
  // We need to do something here to make the command take more time
  // if the disk spinning up (fdc_spinning_up).
  int floppyno=floppy_current_drive();
  TSF314 *floppy=&FloppyDrive[floppyno];
  floppy_irq_flag=FLOPPY_IRQ_YES;
  int hbls_to_interrupt=64;
/*
The 177x accepts 11 commands.  Western Digital divides these commands
into four categories, labeled I,II, III, and IV.

COMMAND SUMMARY
     +------+----------+-------------------------+
     !	    !	       !	   BITS 	 !
     ! TYPE ! COMMAND  !  7  6	5  4  3  2  1  0 !
     +------+----------+-------------------------+
     !	 1  ! Restore  !  0  0	0  0  h  v r1 r0 !
     !	 1  ! Seek     !  0  0	0  1  h  v r1 r0 !
     !	 1  ! Step     !  0  0	1  u  h  v r1 r0 !
     !	 1  ! Step-in  !  0  1	0  u  h  v r1 r0 !
     !	 1  ! Step-out !  0  1	1  u  h  v r1 r0 !
     !	 2  ! Rd sectr !  1  0	0  m  h  E  0  0 !
     !	 2  ! Wt sectr !  1  0	1  m  h  E  P a0 !
     !	 3  ! Rd addr  !  1  1	0  0  h  E  0  0 !
     !	 3  ! Rd track !  1  1	1  0  h  E  0  0 !
     !	 3  ! Wt track !  1  1	1  1  h  E  P  0 !
     !	 4  ! Forc int !  1  1	0  1 i3 i2 i1 i0 !
     +------+----------+-------------------------+
*/

  if((Fdc.cr & BIT_7)==0) 
  {
    // Type 1 commands
/*
Type I commands are Restore, Seek, Step, Step In, and Step Out.

The following table is a bit map of the values to store in the Command
Register.
Command      Bit 7     B6     B5     B4     B3     B2     B1     Bit 0
--------     -----     --     --     --     --     --     --     -----
Restore      0         0      0      0      h      V      r1     r0
Seek         0         0      0      1      h      V      r1     r0
Step         0         0      1      u      h      V      r1     r0
Step in      0         1      0      u      h      V      r1     r0
Step out     0         1      1      u      h      V      r1     r0

Flags:

u (Update Track Register) - If this flag is set, the 177x will update
the track register after executing the command.  If this flag is
cleared, the 177x will not update the track register.

h (Motor On) - If the value of this bit is 1, the controller will
disable the motor spin-up sequence.  Otherwise, if the motor is off
when the chip receives a command, the chip will turn the motor on and
wait 6 revolutions before executing the command.  At 300 RPM, the
6-revolution wait guarantees a one-second start time. If the 177x is
idle for 9 consecutive disk revolutions, it turns off the drive motor.
If the 177x receives a command while the motor is on, the controller
executes the command immediately.

V (Verify) - If this flag is set, the head settles after command
execution.  The settling time is 15 000 cycles for the 1772 and 30 000
cycles for the 1770.  The FDDC will then verify the track position of
the head.  The 177x reads the first ID field it finds and compares the
track number in that ID field against the Track Register.  If the
track numbers match but the ID field CRC is invalid, the 177x sets the
CRC Error bit in the status register and reads the next ID field.  If
the 177x does not find a sector with valid track number AND valid CRC
within 5 disk rotations, the chip sets the Seek Error bit in the
status register.

r (Step Time) - This bit pair determines the time between track steps
according to the following table:

r1       r0            1770                                        1772
--       --            ----                                        ----
0        0             6000 CPU clock cycles                       6000 cycles
0        1             12000 cycles                                12000 cycles
1        0             20 000 cycles                               2000 cycles
1        1             30 000 cycles                               3000 cycles

  SS 
  ST: 1772 r1 r0 11 -> 3000 cycles
  commands starting with 1 = seek  h  V r1 r0

13                                 0  0  1  1  seek
17                                 0  1  1  1  seek with Verify

53  = step in with update track register

  in Fdc.h, we have this table that seems accurate:
  int fdc_step_time_to_hbls[4]={94,188,32,47};

*/
    hbls_to_interrupt=fdc_step_time_to_hbls[Fdc.cr & (BIT_0 | BIT_1)];
    switch(Fdc.cr&0xF0) {
    case 0x00: //restore to track 0
/*
Restore:
If the FDDC receives this command when the drive head is at track
zero, the chip sets its Track Register to $00 and ends the command.
If the head is not at track zero, the FDDC steps the head carriage
until the head arrives at track 0.  The 177x then sets its Track
Register to $00 and ends the command.  If the chip's track-zero input
does not activate after 255 step pulses AND the V bit is set in the
command word, the 177x sets the Seek Error bit in the status register
and ends the command.
*/
      if((Fdc.cr&Fdc.CR_V) && floppy->Empty()) //no disk
        Fdc.str=FDC_STR_SE|FDC_STR_MO|FDC_STR_BSY;
      else
      {
/*
If the head is not at track zero, the FDDC steps the head carriage
until the head arrives at track 0.  The 177x then sets its Track
Register to $00 and ends the command.  If the chip's track-zero input
does not activate after 255 step pulses AND the V bit is set in the
command word, the 177x sets the Seek Error bit in the status register
and ends the command.
*/
        Fdc.tr=255,Fdc.dr=0; // like in CAPSimg
        floppy_irq_flag=0;
        if(FloppyDrive[floppyno].track==0&&DRIVE!=num_connected_floppies)
          Fdc.tr=0;
        agenda_add(agenda_floppy_seek,1,0); //1 scanline
        Fdc.str=FDC_STR_MO|FDC_STR_BSY;
        Fdc.StatusType=1;
        DBG_LOG(Str("FDC: Restored drive ")+char('A'+floppyno)+" to track 0");
      }
      break;
    case 0x10: //seek to track number in data register
/*
Seek:
The CPU must load the desired track number into the Data Register
before issuing this command.  The Seek command causes the 177x to step
the head to the desired track number and update the Track Register.

*/
      agenda_add(agenda_floppy_seek,2,0);
      Fdc.str=FDC_STR_MO|FDC_STR_BSY;
      floppy_irq_flag=0;
      DBG_LOG(Str("FDC: Seeking drive ")+char('A'+floppyno)+" to track "+Fdc.dr+" hbl_count="+hbl_count);
      Fdc.StatusType=1;
      break;
    default: {//step, step in, step out
/*
Step:
The 177x issues one step pulse to the mechanism, then delays one step
time according to the r flag.

Step in:
The 177x issues one step pulse in the direction toward Track 76 and
waits one step time according to the r flag.  [Transcriber's Note:
Western Digital assumes in this paragraph that disks do not have more
than 77 tracks.]

Step out:
The 177x issues one step pulse in the direction toward Track 0 and
waits one step time according to the r flag.

The chip steps the drive head in the same direction it last stepped
unless the command changes the direction.  Each step pulse takes 4
cycles.  The 177x begins outputting a direction signal to the drive 24
cycles before the first stepping pulse.
*/
#if defined(SSE_DRIVE_SOUND)
      if(OPTION_DRIVE_SOUND && Psg.CurrentDrive()!=TYM2149::NO_VALID_DRIVE)
        FloppyDrive[DRIVE].Sound_Step();
#endif
      Fdc.str=FDC_STR_MO|FDC_STR_BSY;
      char d=1; //step direction, default is inwards
      if(floppy->Empty()||Psg.CurrentDrive()==TYM2149::NO_VALID_DRIVE) //Japtro
      {
        if(Fdc.cr&Fdc.CR_V)
          Fdc.str|=FDC_STR_SE;
      }
      else
      {
        switch(Fdc.cr&(BIT_7+BIT_6+BIT_5)) {
        case 0x20: 
          if(Fdc.Lines.direction==0) 
            d=-1; 
          break;
        case 0x60: 
          d=-1; 
          break;
        }
        Fdc.Lines.direction=(d==1);
        if(Fdc.cr & BIT_4) //U flag, update track register
          Fdc.tr+=d;
        if(d==-1&&FloppyDrive[floppyno].track==0)   //trying to step out from track 0
          Fdc.tr=0; //here we set the track register
        else 
        { //can step
          FloppyDrive[floppyno].track+=d;
          DBG_LOG(Str("FDC: Stepped drive ")+char('A'+floppyno)+" to track "+FloppyDrive[floppyno].track);
          if(ADAT)
            floppy_irq_flag=0; // IRQ set in fdc_type1_check_verify()
          fdc_type1_check_verify();
        }
        Fdc.StatusType=1;
        //          TRACE_LOG("Step %d (TR %d CYL %d)\n",d,Fdc.tr,FloppyDrive[floppyno].track);
      }
    }//case
    }//sw
  }
  else 
  {
    Fdc.StatusType=0;
    Fdc.str&=BYTE(~FDC_STR_WP);
    LOG_ONLY(int n_sectors=Dma.Counter; )
    switch(Fdc.cr&(BIT_7+BIT_6+BIT_5+BIT_4)) {
      // Type 2
    case 0x80:case 0xa0:LOG_ONLY(n_sectors=1; ) // Read/write single sector
    case 0x90:case 0xb0:                         // Read/write multiple sectors
/*
Type II commands are Read Sector and Write Sector.

Command          Bit 7     B6     B5     B4     B3     B2     B1     Bit 0
------------     -----     --     --     --     --     --     --     -----
Read Sector      1         0      0      m      h      E      0      0
Write Sector     1         0      1      m      h      E      P      a0

Flags:

m (Multiple Sectors) - If this bit = 0, the 177x reads or writes
("accesses") only one sector.  If this bit = 1, the 177x sequentially
accesses sectors up to and including the last sector on the track.  A
multiple-sector command will end prematurely when the CPU loads a
Force Interrupt command into the Command Register.

h (Motor On) - If the value of this bit is 1, the controller will
disable the motor spin-up sequence.  Otherwise, if the motor is off
when the chip receives a command, the chip will turn the motor on and
wait 6 revolutions before executing the command.  At 300 RPM, the
6-revolution wait guarantees a one- second start time.  If the 177x is
idle for 9 consecutive disk revolutions, it turns off the drive motor.
If the 177x receives a command while the motor is on, the controller
executes the command immediately.

E (Settling Delay) - If this flag is set, the head settles before
command execution.  The settling time is 15 000 cycles for the 1772
and 30 000 cycles for the 1770.

P (Write Precompensation) - On the 1770-02 and 1772-00, a 0 value in
this bit enables automatic write precompensation.  The FDDC delays or
advances the write bit stream by one-eighth of a cycle according to
the following table.

Previous          Current bit           Next bit
bits sent         sending               to be sent       Precompensation
---------         -----------           ----------       ---------------
x       1         1                     0                Early
x       0         1                     1                Late
0       0         0                     1                Early
1       0         0                     0                Late

Programmers typically enable precompensation on the innermost tracks,
where bit shifts usually occur and bit density is maximal.  A 1 value
for this flag disables write precompensation.

a0 (Data Address Mark) - If this bit is 0, the 177x will write a
normal data mark.  If this bit is 1, the 177x will write a deleted
data mark.

Read Sector:
The controller waits for a sector ID field that has the correct track
number, sector number, and CRC.  The controller then checks for the
Data Address Mark, which consists of 43 copies of the second byte of
the CRC.  If the controller does not find a sector with correct ID
field and address mark within 5 disk revolutions, the command ends.
Once the 177x finds the desired sector, it loads the bytes of that
sector into the data register.  If there is a CRC error at the end of
the data field, the 177x sets the CRC Error bit in the Status Register
and ends the command regardless of the state of the "m" flag.

Write Sector:
The 177x waits for a sector ID field with the correct track number,
sector number, and CRC.  The 177x then counts off 22 bytes from the
CRC field.  If the CPU has not loaded a byte into the Data Register
before the end of this 22-byte delay, the 177x ends the command.
Assuming that the CPU has heeded the 177x's data request, the
controller writes 12 bytes of zeroes.  The 177x then writes a normal
or deleted Data Address Mark according to the a0 flag of the command.
Next, the 177x writes the byte which the CPU placed in the Data
Register, and continues to request and write data bytes until the end
of the sector.  After the 177x writes the last byte, it calculates and
writes the 16-bit CRC.  The chip then writes one $ff byte.  The 177x
interrupts the CPU 24 cycles after it writes the second byte of the
CRC.
*/
      if(floppy->Empty()||FloppyDrive[floppyno].track>FLOPPY_MAX_TRACK_NUM
        || (floppyno==num_connected_floppies))
      { //no drive etc.
        TRACE_LOG("Drive empty or track %d overshoot\n",FloppyDrive[floppyno].track);

        Fdc.str=FDC_STR_MO|FDC_STR_BSY;
        floppy_irq_flag=FLOPPY_IRQ_ONESEC;  //end command after 1 second
        break;
      }
#ifdef ENABLE_LOGFILE
      {
        Str RW="Reading",Secs="one sector";
        if(Fdc.cr&0x20) RW="Writing";
        if(n_sectors>1) Secs="multiple sectors";
        DBG_LOG(Str("FDC: ")+RW+" "+Secs+" from drive "+char('A'+floppyno)+
          " track "+FloppyDrive[floppyno].track+
          " side "+floppy_current_side()+
          " sector "+Fdc.sr+
          " into address "+HEXSl(dma_address,6)+
          " Dma.Counter="+Dma.Counter);
      }
#endif
      // compute crc for debug info 
      Fdc.CrcLogic.Reset();
      Fdc.CrcLogic.Add(0xFB); //dam
      if(floppy_instant_sector_access) 
      {
        agenda_add(agenda_floppy_readwrite_sector,hbls_to_interrupt,MAKELONG(0,Fdc.cr));
        Fdc.str=FDC_STR_MO|FDC_STR_BSY;
        floppy_irq_flag=0;
      }
      else
      {
        TWD1772IDField IDList[30];
        int SectorIdx=-1;
        //this should work with STW too as this has been modded:
        int nSects=FloppyDisk[floppyno].GetIDFields(floppy_current_side(),
          FloppyDrive[floppyno].track,IDList);
        for(int n=0;n<nSects;n++) 
        {
          if(IDList[n].track==Fdc.tr && IDList[n].num==Fdc.sr) 
          {
            SectorIdx=n;
            break;
          }
        }
        if(SectorIdx>-1&&nSects>0)
        {
          BYTE num=Fdc.sr;
          WORD CurrentByte=FloppyDrive[DRIVE].BytePosition();
          WORD BytesToNextID=FloppyDisk[DRIVE].BytesToID(num);
          //TRACE_FDC("Current %d Start ID %d in %d, at %d ",CurrentByte,num,BytesToNextID,BytesToNextID+CurrentByte);
          // 12+3: to ID; 16: will be counted in agenda function
          BYTE pre_data_gap=FloppyDisk[DRIVE].PreDataGap()-12-3-16;
          //TRACE_FDC("gap %d\n",pre_data_gap);
          WORD start=(CurrentByte+BytesToNextID+pre_data_gap)
            %FloppyDisk[DRIVE].TrackBytes;
          DWORD HBLOfSectorStart=hbl_count
            +FloppyDrive[DRIVE].BytesToHbls(BytesToNextID+pre_data_gap);
          //TRACE_FDC("hbl now %d then %d, diff %d\n",hbl_count,HBLOfSectorStart,HBLOfSectorStart-hbl_count);
          HBLOfSectorStart-=2+1; //see below, Steem's way, + hbl_count is ++ right after comparison
          agenda_delete(agenda_floppy_readwrite_sector);
          agenda_add(agenda_floppy_readwrite_sector,
            (HBLOfSectorStart-hbl_count)+2,MAKELONG(0,start));
          floppy_irq_flag=0;
          Fdc.str=FDC_STR_MO|FDC_STR_BSY;
        }
        else 
        {
          TRACE_LOG("SectorIdx %d nSects %d\n",SectorIdx,nSects);
          floppy_irq_flag=FLOPPY_IRQ_ONESEC;
          Fdc.str=FDC_STR_MO|FDC_STR_BSY;
        }
      }
      break;
      // Type 3
/*
Type III commands are Read Address, Read Track, and Write Track.

Command          Bit 7     B6     B5     B4     B3     B2     B1     Bit 0
------------     -----     --     --     --     --     --     --     -----
Read Address     1         1      0      0      h      E      0      0      C0
Read Track       1         1      1      0      h      E      0      0      E0
Write Track      1         1      1      1      h      E      P      0      F0

Flags:

h (Motor On) - If the value of this bit is 1, the controller will
disable the motor spin-up sequence.  Otherwise, if the motor is off
when the chip receives a command, the chip will turn the motor on and
wait 6 revolutions before executing the command.  At 300 RPM, the
6-revolution wait guarantees a one- second start time.  If the 177x is
idle for 9 consecutive disk revolutions, it turns off the drive motor.
If the 177x receives a command while the motor is on, the controller
executes the command immediately.

E (Settling Delay) - If this flag is set, the head settles before
command execution.  The settling time is 15 000 cycles for the 1772
and 30 000 cycles for the 1770.

P (Write Precompensation) - On the 1770-02 and 1772-00, a 0 value in
this bit enables automatic write precompensation.  The FDDC delays or
advances the write bit stream by one-eighth of a cycle according to
the following table.

Previous          Current bit           Next bit
bits sent         sending               to be sent       Precompensation
---------         -----------           ----------       ---------------
x       1         1                     0                Early
x       0         1                     1                Late
0       0         0                     1                Early
1       0         0                     0                Late

Programmers typically enable precompensation on the innermost tracks,
where bit shifts usually occur and bit density is maximal.  A 1 value
for this flag disables write precompensation.
*/
    case 0xc0: //read address
/*
Read Address:
The 177x reads the next ID field it finds, then sends the CPU the
following six bytes via the Data Register:
Byte #     Meaning                |     Sector length code     Sector length
------     ------------------     |     ------------------     -------------
1          Track                  |     0                      128
2          Side                   |     1                      256
3          Sector                 |     2                      512
4          Sector length code     |     3                      1024
5          CRC byte 1             |
6          CRC byte 2             |

[Transcriber's Note:  | is the vertical bar character.]

The 177x copies the track address into the Sector Register.  The chip
sets the CRC Error bit in the status register if the CRC is invalid.
*/
      DBG_LOG(Str("FDC: Type III Command - read address to ")+HEXSl(dma_address,6)+"from drive "+char('A'+floppyno));
      if(floppy->Empty())
        floppy_irq_flag=0;  //never cause interrupt, timeout
      else
      {
        TWD1772IDField IDList[30];
        DWORD nSects=FloppyDisk[floppyno].GetIDFields(floppy_current_side(),
          FloppyDrive[floppyno].track,IDList);
        if(nSects &&!(FloppyDrive[floppyno].single_sided&&CURRENT_SIDE==1))
        {
          WORD HBLsToNextSector=0;
          BYTE NextIDNum; // it's index 0 - #sectors-1
          FloppyDisk[DRIVE].NextID(NextIDNum,HBLsToNextSector); //references
          agenda_delete(agenda_floppy_read_address);
          agenda_add(agenda_floppy_read_address,HBLsToNextSector,NextIDNum);
          floppy_irq_flag=0;
          Fdc.str=FDC_STR_MO|FDC_STR_BSY;
        }
        else 
        {
          Fdc.str=FDC_STR_MO|FDC_STR_SE|FDC_STR_BSY;  //sector not found
          floppy_irq_flag=0;  //never cause interrupt, timeout
        }
      }
      break;
    case 0xe0:  //read track
/*
Read Track:
This command dumps a raw track, including gaps, ID fields, and data,
into the Data Register.  The FDDC starts reading with the leading edge
of the first index pulse it finds, and stops reading with the next
index pulse.  During this command, the FDDC does not check CRCs.  The
address mark detector is on during the entire command.  (The address
mark detector detects ID, data and index address marks during read and
write operations.)  Because the address mark detector is always on,
write splices or noise may cause the chip to look for an address mark.
[Transcriber's Note: I do not know how the programmer can tell that
the AM detector has found an address mark.]  The chip may read gap
bytes incorrectly during write-splice time because of synchronization.
*/
      DBG_LOG(Str("FDC: Type III Command - read track to ")+HEXSl(dma_address,6)+" from drive "+char('A'+floppyno)+
                  " Dma.Counter="+Dma.Counter);
      Fdc.str=FDC_STR_MO|FDC_STR_BSY;
      floppy_irq_flag=0;
      if(floppy->DiskInDrive()) 
      {
        agenda_delete(agenda_floppy_read_track);
        DWORD DiskPosition=hbl_count % DRIVE_HBLS_PER_ROTATION;
        agenda_add(agenda_floppy_read_track,DRIVE_HBLS_PER_ROTATION-
          DiskPosition,0);
      }
      break;
    case 0xf0:  //write (format) track
/*
Write Track:
This command is the means of formatting disks.  The drive head must be
over the correct track BEFORE the CPU issues the Write Track command.
Writing starts with the leading edge of the first index pulse which
the 177x finds.  The 177x stops writing when it encounters the next
index pulse.  The 177x sets the Data Request bit immediately after
receiving the Write Track command, but does not start writing until
the CPU loads the Data Register.  If the CPU does not send the 177x a
byte within three byte times after the first index pulse, the 177x
ends the command.  The 177x will write all data values from $00 to $f4
(inclusive) and from $f8 to $ff (inclusive) unaltered.  Data values
$f5, $f6, and $f7, however, have special meanings.  The value $f5
means to write an $a1 to the disk.  The $a1 value which the 177x
writes to the disk will lack an MFM clock transition between bits 4
and 5.  This missing clock transition indicates that the next normally
written byte will be an address mark.  In addition, a Data Register
value of $f5 will reset the 177x's CRC generator.  A Data Register
value of $f6 will not reset the CRC generator but will write a pre-
address-mark value of $c2 to the disk.  The written $c2 will lack an
MFM clock transition between bits 3 and 4.  A Data Register value of
$f7 will write a two-byte CRC to the disk.
*/
      DBG_LOG(Str("FDC: - Type III Command - write track from address ")+HEXSl(dma_address,6)+" to drive "+char('A'+floppyno));
      floppy_irq_flag=0;
      Fdc.str=FDC_STR_MO|FDC_STR_BSY;
      if(floppy->DiskInDrive()&&FloppyDisk[floppyno].ReadOnly==0
        &&FloppyDrive[floppyno].track<=FLOPPY_MAX_TRACK_NUM) 
      {
        if(FloppyDisk[floppyno].Format_f==NULL) 
          FloppyDisk[floppyno].OpenFormatFile();
        if(FloppyDisk[floppyno].Format_f) 
        {
          agenda_delete(agenda_floppy_write_track);
          DWORD DiskPosition=hbl_count % DRIVE_HBLS_PER_ROTATION;
          agenda_add(agenda_floppy_write_track,DRIVE_HBLS_PER_ROTATION
            -DiskPosition,0);
          floppy_write_track_bytes_done=0;
        }
      }
      break;
    case 0xd0:        //force interrupt
/*
The Type IV command is Force Interrupt.

Force Interrupt:
Programmers use this command to stop a multiple-sector read or write
command or to ensure Type I status in the Status Register.  The format
of this command is %1101(I3)(I2)00.  If flag I2 is set, the 177x will
acknowledge the command at the next index pulse.  If flag I3 is set,
the 177x will immediately stop what it is doing and generate an
interrupt.  If neither I2 nor I3 are set, the 177x will not interrupt
the CPU, but will immediately stop any command in progress.  After the
CPU issues an immediate interrupt command ($d8), it MUST write $d0
(Force Interrupt, I2 clear, I3 clear) to the Command Register in order
to shut off the 177x's interrupt output.  After any Force Interrupt
command, the CPU must wait 16 cycles before issuing any other command.
If the CPU does not wait, the 177x will ignore the previous Force
Interrupt command.  Because the 177x is microcoded, it will
acknowledge Force Interrupt commands only between micro- instructions.
*/
      DBG_LOG(Str("FDC: ")+HEXSl(old_pc,6)+" - Force interrupt: t="+hbl_count);
/*  On the common $90-$D0 sequence, the command is still active when it is 
    interrupted, and STR isn't 'type I'. 
*/
      if(!(Fdc.str&FDC_STR_BSY))
        Fdc.StatusType=2; // PYM/BPOC doesn't like 'track0' 
      agenda_delete(agenda_floppy_seek);
      agenda_delete(agenda_floppy_readwrite_sector);
      agenda_delete(agenda_floppy_read_address);
      agenda_delete(agenda_floppy_read_track);
      agenda_delete(agenda_floppy_write_track);
      agenda_delete(agenda_fdc_verify);
      agenda_delete(agenda_fdc_spun_up); // Holocaust/Blood real start
      fdc_spinning_up=0; // v4, was the real problem with Froggies
      agenda_delete(agenda_fdc_finished);
      Fdc.str=(Fdc.str&FDC_STR_MO);
      if(Fdc.cr&b1100) 
      {
/*  "The lower four bits of the command determine the conditional 
interrupt as follows:
- i0,i1 = Not used with the WD1772
- i2 = Every Index Pulse
- i3 = Immediate Interrupt
The conditional interrupt is enabled when the corresponding bit 
positions of the command (i3-i0) are set to a 1. When the 
condition for interrupt is met the INTRQ line goes high signifying
 that the condition specified has occurred. If i3-i0 are all set
 to zero (Hex D0), no interrupt occurs but any command presently
 under execution is immediately terminated. When using the immediate
 interrupt condition (i3 = 1) an interrupt is immediately generated
 and the current command terminated. Reading the status or writing
 to the Command Register does not automatically clear the interrupt.
 The Hex D0 is the only command that enables the immediate interrupt
 (Hex D8) to clear on a subsequent load Command Register or Read 
 Status Register operation. Follow a Hex D8 with D0 command."

  -> D4 (IRQ every index pulse) wasn't implemented yet in Steem. 
*/          
        if(Fdc.cr&b1000) // D8
        {
          Fdc.InterruptCondition=8;
          agenda_fdc_finished(0); // Interrupt CPU immediately
        }
        else // D4
        {
          Fdc.InterruptCondition=4; // IRQ at every index pulse
          floppy_irq_flag=0;
          WORD hbls_to_next_ip=FloppyDrive[DRIVE].HblsNextIndex();
          agenda_add(agenda_fdc_finished,hbls_to_next_ip,0);
        }
      }
      else
      { //D0
        floppy_irq_flag=0;
        Fdc.InterruptCondition=0;
        agenda_add(agenda_fdc_motor_flag_off,DRIVE_HBLS_PER_ROTATION*9,0); //no IP ctr...
      }
      break;
    }//sw
  }//if
  if(!ADAT && (Fdc.str&FDC_STR_MO) && !floppy->Empty())
    agenda_add(agenda_fdc_motor_flag_off,milliseconds_to_hbl(1800),0);
  // Don't need to add agenda if it has happened
  if(floppy_irq_flag && (floppy_irq_flag!=FLOPPY_IRQ_NOW)) 
  {
    if(floppy_irq_flag==FLOPPY_IRQ_ONESEC) 
    {
      TRACE_LOG("Error - 5REV - IRQ in %d HBLs\n",DRIVE_HBLS_PER_ROTATION*5);
      agenda_add(agenda_fdc_finished,DRIVE_HBLS_PER_ROTATION*5,0);
    }
    else
      agenda_add(agenda_fdc_finished,int(hbls_to_interrupt),0);
  }
}


void agenda_fdc_motor_flag_off(int revs_to_wait) {
/*
 "If after finishing the command, the device remains idle for
 9 revolutions, the MO signal goes back to a logic 0."
  ->
  To ensure 9 revs, the controller counts 10 IP.
  We do the same as before, using our new variable IndexCounter
  We don't use 'revs_to_wait'

  We count IP, only if there's a spinning selected drive
  Not emulated: if program changes drive and the new drive is spinning
  too, but IP occurs at different time!

  Cases Symic; Treasure Trap -FOF

*/
  if(ADAT)
  {
    if(Psg.CurrentDrive()!=TYM2149::NO_VALID_DRIVE && FloppyDrive[DRIVE].motor
      &&!FloppyDrive[DRIVE].Empty())
      Fdc.IndexCounter++;
    if(Fdc.IndexCounter<10 && FloppyDrive[DRIVE].ImageType.Manager==MNGR_STEEM)
    {
      agenda_add(agenda_fdc_motor_flag_off,DRIVE_HBLS_PER_ROTATION,revs_to_wait);
      return;
    }
    Fdc.IndexCounter=0; 
  }
  Fdc.str&=BYTE(~FDC_STR_MO);
  TRACE_LOG("Motor off\n");
  FloppyDrive[DRIVE].motor=false;
}


void agenda_fdc_verify(int) {
  if(Fdc.StatusType && (Fdc.cr&Fdc.CR_V))
  {
    // This reads an ID field and checks that track number matches FloppyDrive's track
    // It will fail on an unformatted track or if there is no disk of course
    int floppyno=floppy_current_drive();
    //TSF314 *floppy=&FloppyDrive[floppyno];
    if(FloppyDrive[floppyno].track>FLOPPY_MAX_TRACK_NUM)
      Fdc.str|=FDC_STR_SE;
    else if(FloppyDrive[floppyno].track!=Fdc.tr) // track not correct
      Fdc.str|=FDC_STR_SE;
    else if(FloppyDisk[floppyno].TrackIsFormatted[floppy_current_side()]
      [FloppyDrive[floppyno].track]==0)
    {
      // If track is formatted then it is okay to seek to it, otherwise do this:
      if(FloppyDrive[floppyno].track>=FloppyDisk[floppyno].TracksPerSide)
        Fdc.str|=FDC_STR_SE;
      if(floppy_current_side()>=FloppyDisk[floppyno].Sides)
        Fdc.str|=FDC_STR_SE;
    }
#if defined(SSE_DEBUG)
    DEBUG_ONLY(if(Fdc.str & FDC_STR_SE) DBG_LOG("     Verify failed (track not formatted)"); )
    if(Fdc.str & FDC_STR_SE)
      TRACE_LOG("Verify error TR %d CYL %d\n",Fdc.tr,FloppyDrive[floppyno].track);
#endif
  }
  agenda_fdc_finished(0);
}


void agenda_fdc_finished(int) {
#if defined(DEBUG_BUILD)
  if(TRACE_ENABLED(LOGSECTION_FDC)) 
    Dma.UpdateRegs(true); // -> "IRQ" trace (including 'busy')
#endif
#if defined(SSE_DRIVE_SOUND) 
  if(OPTION_DRIVE_SOUND&&(!ADAT||OPTION_DRIVE_SOUND_SEEK_SAMPLE))
    FloppyDrive[DRIVE].Sound_CheckIrq();
#endif//snd
  DBG_LOG("FDC: Finished command, GPIP bit low.");
  if(floppy_irq_flag==FLOPPY_IRQ_ONESEC)
    Fdc.str|=FDC_STR_SE;
  floppy_irq_flag=FLOPPY_IRQ_NOW;
  mfp_gpip_set_bit(MFP_GPIP_FDC_BIT,0); // Sets bit in GPIP low (and it stays low)
  Fdc.str&=BYTE(~FDC_STR_BSY); // Turn off busy bit
  Fdc.str&=BYTE(~FDC_STR_T0); // This is lost data bit for non-type 1 commands
#if defined(SSE_DISK_GHOST)
  if(!(OPTION_GHOST_DISK && Fdc.Lines.CommandWasIntercepted)) // spurious Lost Data
#endif
    if(Fdc.StatusType)
    {
      if(CURRENT_TRACK==0 && num_connected_floppies!=DRIVE)
        Fdc.str|=FDC_STR_T0;
      Fdc.StatusType=2;
    }
/*  Command $D4 will trigger an IRQ at each index pulse.
    Motor keeps running.
    It's a way to measure rotation time (Panzer)
*/
  if(Fdc.InterruptCondition==4)
  {
//    TRACE_LOG("D4 prepare next IP at %d\n",hbl_count+DRIVE_HBLS_PER_ROTATION);
    agenda_add(agenda_fdc_finished,DRIVE_HBLS_PER_ROTATION,0);    
  } 
  else if(ADAT)
  {
    //  Set up agenda for next IP
    agenda_delete(agenda_fdc_motor_flag_off); //3.6.2
    Fdc.IndexCounter=0;
    DWORD delay=(ADAT) ? (FloppyDrive[DRIVE].HblsNextIndex()) : 2;
    agenda_add(agenda_fdc_motor_flag_off,delay,0);
  }
}


void agenda_floppy_seek(int) {
  int floppyno=floppy_current_drive();
/*
"SEEK
This command assumes that the track register contains the track number of the
current position of the Read/Write head and the Data Register contains the
desired track number. The WD1770 will update the Track Register and issue
stepping pulses in the appropiate direction until the contents of the Track
Register are equal to the contents of the Data Register (the desired Track
location). A verification operation takes place if the V flag is on. The h
bit allows the Motor On option at the start of the command. An interrupt is
generated at the completion of the command. Note: When using mutiple drives,
the track register must be updated for the drive selected before seeks are
issued."
  ->
  Seek works with DR and TR, not DR and disk track.
*/
  if(ADAT)
  {
    if(Fdc.tr==Fdc.dr)
    {
      DBG_LOG(Str("FDC: Finished seeking to track ")+Fdc.dr+" hbl_count="+hbl_count);
      fdc_type1_check_verify();
      return;
    }
    else 
    {
      if(Fdc.tr>Fdc.dr)
      {
        Fdc.tr--;
        if(FloppyDrive[floppyno].track)
          FloppyDrive[floppyno].track--;    
        if(!FloppyDrive[floppyno].track && floppyno!=num_connected_floppies)
        {
          if(!(Fdc.cr&0xF0)) // condition?
            Fdc.tr=0; // this is  how RESTORE works
          Fdc.str|=FDC_STR_T0;
        }
/*  
If the TR00* input does not go active low after 255 stepping pulses,
the WD1772 terminates operation, interrupts, and sets the Seek Error
status bit, providing the v flag is set.
->
If RESTORE fails to set TR00 in the status byte, TOS knows there's no
2nd drive.
*/
        else if(!(Fdc.cr&0xF0) && !Fdc.tr)
        {
          //ASSERT(floppyno==num_connected_floppies); //1
          if(Fdc.cr&Fdc.CR_V)
            Fdc.str|=FDC_STR_SE;
          agenda_fdc_finished(0); //IRQ
        }
      }
      else
      {
        //ASSERT(Fdc.tr<Fdc.dr);
        Fdc.tr++;
        if(FloppyDrive[floppyno].track<FLOPPY_MAX_TRACK_NUM)
          FloppyDrive[floppyno].track++;
      }
    }
  }
  else // !ADAT or not defined
  {
/*  In Steem 3.2, seek was directly using disk track instead of TR
    It still works that way in fast mode.
    It was already using an agenda for each step.
*/
    if(FloppyDrive[floppyno].track==Fdc.dr)
    {
      DBG_LOG(Str("FDC: Finished seeking to track ")+Fdc.dr+" hbl_count="+hbl_count);
      Fdc.tr=Fdc.dr;
      fdc_type1_check_verify();
      agenda_fdc_finished(0);
      return;
    }
    if(FloppyDrive[floppyno].track>Fdc.dr)
      FloppyDrive[floppyno].track--;
    else if(FloppyDrive[floppyno].track<Fdc.dr)
      FloppyDrive[floppyno].track++;
  }
  int hbls_to_interrupt=fdc_step_time_to_hbls[Fdc.cr & (BIT_0 | BIT_1)];
  if(floppy_instant_sector_access)
    hbls_to_interrupt>>=5;
#if defined(SSE_DRIVE_SOUND)
  if(OPTION_DRIVE_SOUND)
    FloppyDrive[DRIVE].Sound_Step();
#endif
  agenda_add(agenda_floppy_seek,hbls_to_interrupt,0);
}


void agenda_floppy_readwrite_sector(int Data) {
  int floppyno=floppy_current_drive();
  TSF314 *floppy=&FloppyDrive[floppyno];
  bool FromFormat=0;
  if(FloppyDrive[floppyno].track<=FLOPPY_MAX_TRACK_NUM)
    FromFormat=FloppyDisk[floppyno].TrackIsFormatted[floppy_current_side()]
      [FloppyDrive[floppyno].track];
  int Command=Fdc.cr; //shouldn't change...
  BYTE WriteProtect=0;
  if((Command&0x20)&&FloppyDisk[floppyno].ReadOnly) // Write
    WriteProtect=FDC_STR_WP;
  if(floppy->Empty())
  {
    TRACE_LOG("No disk %c\n",floppyno+'A');
    Fdc.str=BYTE(WriteProtect | FDC_STR_MO | /*FDC_STR_SE | */FDC_STR_BSY);
    agenda_add(agenda_fdc_finished,DRIVE_HBLS_PER_ROTATION
      *int((Glue.video_freq==MONO_HZ) ? 11:5),0);
    return; // Don't loop
  }
  Fdc.str=BYTE(FDC_STR_BSY | FDC_STR_MO | WriteProtect);
  floppy_irq_flag=0;
  if(FLOPPY_ACCESS_FF(floppyno))
    floppy_access_ff_counter=FLOPPY_FF_VBL_COUNT;
  if(!ADAT)
  {
    agenda_delete(agenda_fdc_motor_flag_off);
    agenda_add(agenda_fdc_motor_flag_off,milliseconds_to_hbl(1800),0);
  }
instant_sector_access_loop:
  int Part=LOWORD(Data);
  int SectorStage=(Part % 71); // 0=seek, 1-64=read/write, 65=end of sector, 66-70=gap
  if(SectorStage==0) 
  {
    if(FloppyDisk[floppyno].SeekSector(floppy_current_side(),
      FloppyDrive[floppyno].track,Fdc.sr,FromFormat)) 
    {
      // Error seeking sector, it doesn't exist
      TRACE_LOG("H%d T%d RNF %d\n",floppy_current_side(),FloppyDrive[floppyno].track,Fdc.sr);
      floppy_irq_flag=FLOPPY_IRQ_ONESEC;  //end command after 1 second
    }
#ifdef ONEGAME
    else{
      if (Command & 0x20){ // Write
        OGWriteSector(floppy_current_side(),FloppyDrive[floppyno].track,Fdc.sr,floppy->BytesPerSector);
      }
    }
#endif
  }
  else if(SectorStage<=64) 
  {
    FILE *f=(FILE*)(FromFormat ? FloppyDisk[floppyno].Format_f:FloppyDisk[floppyno].f);
    int BytesPerStage=16; // constant
    int PosInSector=(SectorStage-1)*BytesPerStage;
    BYTE Temp=0;
    if(Command&0x20)
    { // Write
      if(FloppyDisk[floppyno].ReadOnly) 
      {
        floppy_irq_flag=FLOPPY_IRQ_NOW; //interrupt with write-protect flag
        FDCCantWriteDisplayTimer=timer+3000;
      }
      else
      {
        FloppyDisk[floppyno].WrittenTo=true;
        if(FloppyDisk[floppyno].IsZip()) 
          FDCCantWriteDisplayTimer=timer+5000; // Writing will be lost!
        // byte per byte, we write 16 bytes
        for(int bb=BytesPerStage;bb>0;bb--)
        {
          Temp=Dma.GetFifoByte(); // from RAM to disk
          Fdc.CrcLogic.Add(Temp); // debug info
          if(f==NULL||fwrite(&Temp,1,1,f)==0)
          {
            if(floppy_handle_file_error(floppyno,true,Fdc.sr,PosInSector,
              FromFormat)) 
            {
              TRACE_LOG("fwrite error\n");
              floppy_irq_flag=FLOPPY_IRQ_ONESEC;  //end command after 1 second
              break;
            }
          }
          PosInSector++;
        }
      }
    }
    else
    { // Read
      for(int bb=BytesPerStage;bb>0;bb--) 
      {	// int BytesPerStage=16;
        if(f==NULL||fread(&Temp,1,1,f)==0)
        {
          if(floppy_handle_file_error(floppyno,0,Fdc.sr,PosInSector,
            FromFormat))
          {
            TRACE_LOG("fread error\n");
            floppy_irq_flag=FLOPPY_IRQ_ONESEC;  //end command after 1 second
            break;
          }
        }
        if(Dma.Counter) //game Sabotage
          Dma.AddToFifo(Temp); // disk to RAM
        Fdc.CrcLogic.Add(Temp);
        PosInSector++;
      }
    }
    if(PosInSector>=int(FromFormat?FloppyDisk[floppyno].FormatLargestSector
      :FloppyDisk[floppyno].BytesPerSector))
      Part=64; // Done sector, last part
  }
  else if(SectorStage==65)
  {
    DBG_LOG(Str("FDC: Finished reading/writing sector ")+Fdc.sr+" of track "+FloppyDrive[floppyno].track+" of side "+floppy_current_side());
    floppy_irq_flag=FLOPPY_IRQ_NOW;
    if(Command & BIT_4)
    { // Multiple sectors
      Fdc.sr++;
      floppy_irq_flag=0;
      {
        if(!Fdc.sr)
          Fdc.sr++; // CAPS: 255 wraps to 1 (due to add with carry)
        TRACE_LOG("SR->%d\n",Fdc.sr);
        // The controller must find next sector
        if(!ADAT)
          agenda_add(agenda_floppy_readwrite_sector,1,MAKELONG(++Part,Command));
        else
          agenda_add(agenda_floppy_readwrite_sector,FloppyDisk[DRIVE].SectorGap(),
          MAKELONG(++Part,Command));
        return;
      }
    }
  }
  Part++;
  switch(floppy_irq_flag) {
  case FLOPPY_IRQ_NOW:
    DBG_LOG("FDC: Finished read/write sector");
    TRACE_FDC("CRC %X\n",Fdc.CrcLogic.crc);
    Fdc.str=BYTE(WriteProtect|FDC_STR_MO);
    agenda_fdc_finished(0);
    return; // Don't loop
  case FLOPPY_IRQ_ONESEC:
    // sector not found
    Fdc.str=BYTE(WriteProtect|FDC_STR_MO| /*FDC_STR_SE | */FDC_STR_BSY);
    TRACE_LOG("Sector not found\n");
    agenda_add(agenda_fdc_finished,DRIVE_HBLS_PER_ROTATION
      *int((Glue.video_freq==MONO_HZ) ? 11 : 5),0);
    return; // Don't loop
  }
  if(floppy_instant_sector_access)
  {  // Fast disk
    Data=MAKELONG(Part,Command); // Part has been ++, Command is unchanged
    goto instant_sector_access_loop; // goto considered harmful
  }
  else
  { //Slow disk
/*  Correct drift due to hbl system imprecision. 
    With packs of 16 bytes it can accumulate, and at the end of the sector, 
    we're off by a couple of HBL, enough to miss next ID. */
    //ASSERT(Part!=64||FloppyDrive[DRIVE].ImageType.Extension==EXT_STT); // 32 -> 65
    WORD bytes=(Part==65) ? 19 : 16;
    WORD start=HIWORD(Data);
    if(Part<65)
    {
      WORD current_byte=FloppyDrive[DRIVE].BytePosition();
      WORD theory_byte=start+((WORD)Part-1)*16;
#if 1 // limit correction!
      short drift=current_byte-theory_byte;
      if(drift<0)
        bytes+=2;
      else if(drift>0)
        bytes-=2;
#else
      if(current_byte-theory_byte)
      {
        //TRACE_LOG("Drift start %d theory %d byte %d Part %d\n",start,start+(Part-1)*16,FloppyDrive[DRIVE].BytePosition(),Part);
        bytes-=2*(current_byte-theory_byte);
      }
#endif
      if(Part==64)
        bytes+=3; //v4: crc, ff
    }
    agenda_add(agenda_floppy_readwrite_sector,FloppyDrive[DRIVE].BytesToHbls(bytes),
      MAKELONG(Part,start)); 
  }
}


void agenda_floppy_read_address(int idx) {
  int floppyno=floppy_current_drive();
  //TSF314 *floppy=&FloppyDrive[floppyno];
  TWD1772IDField IDList[30];
  int nSects=FloppyDisk[floppyno].GetIDFields(floppy_current_side(),
    FloppyDrive[floppyno].track,IDList);
  if(idx<nSects) 
  {
    DBG_LOG(Str("FDC: Reading address for sector ")+IDList[idx].num+" on track "+
                FloppyDrive[floppyno].track+", side "+floppy_current_side()+" hbls="+hbl_count);
    TRACE_LOG("Read address T%d S%d s#%d t%d CRC %02X%02X\n", IDList[idx].track,
      IDList[idx].side,IDList[idx].num,IDList[idx].len,IDList[idx].CRC[0],IDList[idx].CRC[1]);
    // timing of this is not really correct
    Dma.AddToFifo(IDList[idx].track);
    Dma.AddToFifo(IDList[idx].side);
    Dma.AddToFifo(IDList[idx].num);
    Dma.AddToFifo(IDList[idx].len);
    Dma.AddToFifo(IDList[idx].CRC[0]);
    Dma.AddToFifo(IDList[idx].CRC[1]);
    Fdc.str&=BYTE(~FDC_STR_WP);
    Fdc.str|=FDC_STR_MO;
    //The Track Address of the ID field is written into the sector register so 
    //that a comparison can be made by the user.
    Fdc.sr=IDList[idx].track;
    agenda_fdc_finished(0);
    if(!ADAT) 
    {
      agenda_delete(agenda_fdc_motor_flag_off);
      agenda_add(agenda_fdc_motor_flag_off,milliseconds_to_hbl(1800),0);
    }
  }
}


void agenda_floppy_read_track(int part) {
  WORD &CRC=Fdc.CrcLogic.crc;
  static short BytesRead;
  int floppyno=floppy_current_drive();
  TSF314 *floppy=&FloppyDrive[floppyno];
  bool Error=0;
  bool FromFormat=0;
  if(FloppyDrive[floppyno].track<=FLOPPY_MAX_TRACK_NUM)
    FromFormat=FloppyDisk[floppyno].TrackIsFormatted[floppy_current_side()]
      [FloppyDrive[floppyno].track];
  if(floppy->Empty()) 
    return; // Stop, timeout
  int RealPart=HIWORD(part); //starts at 0
  part=LOWORD(part);
  if(part==0) 
    BytesRead=0;
  Fdc.str|=FDC_STR_BSY;
  if(FLOPPY_ACCESS_FF(floppyno))
    floppy_access_ff_counter=FLOPPY_FF_VBL_COUNT;
  if(!ADAT)
  {
    agenda_delete(agenda_fdc_motor_flag_off);
    agenda_add(agenda_fdc_motor_flag_off,milliseconds_to_hbl(1800),0);
  }
  int TrackBytes=FloppyDisk[floppyno].GetRawTrackData(floppy_current_side(),
    FloppyDrive[floppyno].track);
  if(TrackBytes) // STT
  {
    fseek(FloppyDisk[floppyno].f,BytesRead,SEEK_CUR);
    int ReinsertAttempts=0;
    BYTE Temp;
    for(int n=0;n<16;n++)
    {
      if(BytesRead>=TrackBytes) 
        break;
      if(fread(&Temp,1,1,FloppyDisk[floppyno].f)==0) 
      {
        if(ReinsertAttempts++>2) 
        {
          Error=true;
          break;
        }
        if(floppy->ReinsertDisk())
        {
          TrackBytes=FloppyDisk[floppyno].GetRawTrackData(floppy_current_side(),
            FloppyDrive[floppyno].track);
          fseek(FloppyDisk[floppyno].f,BytesRead,SEEK_CUR);
          n--;
        }
      }
      else
      {
        if(DMA_ADDRESS_IS_VALID_W && Dma.Counter)
          Dma.AddToFifo(Temp);
        BytesRead++;
      }
    }
  }
  else // ST, MSA, DIM
  {
    int DDBytes=FloppyDisk[floppyno].TrackBytes; //v4
    TWD1772IDField IDList[30];
    int nSects=FloppyDisk[floppyno].GetIDFields(floppy_current_side(),
      FloppyDrive[floppyno].track,IDList);
    if(nSects==0)
    {
      // Unformatted track, read in random values
      TrackBytes=DDBytes;
      for(int bb=0;bb<16;bb++) 
      {
        write_to_dma(rand()&0xFF);
        BytesRead++;
      }
    }
    else
    {
      // Find out if it is a high density track
      TrackBytes=0;
      for(int n=0;n<nSects;n++) 
        TrackBytes+=22+12+3+1+6+22+12+3+1 + (128 << IDList[n].len) + 26;
      if(TrackBytes>DDBytes)
        TrackBytes=DDBytes*2;
      else
        TrackBytes=DDBytes;
      if(part/154<nSects) 
      {
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
        Dma.Datachunk++;
#endif
        int IDListIdx=part/154;
        BYTE SectorNum=IDList[IDListIdx].num;
        int SectorBytes=(128 << IDList[IDListIdx].len);
        BYTE pre_sect[200];
        int i=0;
        //for(int n=0;n<(ADAT?nSects<FloppyDisk[floppyno].PostIndexGap():22);n++) //?
        for(int n=0;n<(SectorNum?22:FloppyDisk[floppyno].PostIndexGap());n++)  //v4 60,22,10 - 22
          pre_sect[i++]=0x4e;  // gap 1 & 3
/*
Gap 2 Pre ID                    12+3        12+3         3+3     00+A1
*/
        for(int n=0;n<(nSects<11?12:3);n++) 
          pre_sect[i++]=0x00; // gap 2 PLL Lockup time
        for(int n=0;n<3;n++) 
          pre_sect[i++]=0xa1;   // gap 2 Marker
        pre_sect[i++]=0xfe;                         // Start of address mark
        pre_sect[i++]=IDList[IDListIdx].track;
        pre_sect[i++]=IDList[IDListIdx].side;
        pre_sect[i++]=IDList[IDListIdx].num;
        pre_sect[i++]=IDList[IDListIdx].len;
        pre_sect[i++]=IDList[IDListIdx].CRC[0];
        pre_sect[i++]=IDList[IDListIdx].CRC[1];
        for(int n=0;n<22;n++) 
          pre_sect[i++]=0x4e; // gap 3a
        for(int n=0;n<12;n++) 
          pre_sect[i++]=0x00; // gap 3b
        for(int n=0;n<3;n++) 
          pre_sect[i++]=0xa1;  // gap 3b Marker
/*
Data Address Mark                  1           1           1      FB
*/
        pre_sect[i++]=0xfb;                        // Start of data
        int num_bytes_to_write=16;
        int byte_idx=(part%154)*16;
        // Write the gaps/address before the sector
        if(byte_idx<i) 
        {
          while(num_bytes_to_write>0)
          {
            write_to_dma(pre_sect[byte_idx++]);
            num_bytes_to_write--;
            BytesRead++;
            if(byte_idx>=i)
              break;
          }
        }
        byte_idx-=i;
        // Write the sector
/*
Data                             512         512         512
*/
        if(num_bytes_to_write>0&&byte_idx>=0&&byte_idx<SectorBytes)
        {
          if(byte_idx==0)
          {
            CRC=0xffff;
            fdc_add_to_crc(CRC,0xa1);
            fdc_add_to_crc(CRC,0xa1);
            fdc_add_to_crc(CRC,0xa1);
            fdc_add_to_crc(CRC,0xfb);
          }
          if(FloppyDisk[floppyno].SeekSector(floppy_current_side(),
            FloppyDrive[floppyno].track,SectorNum,FromFormat)) 
          {
            // Can't seek to sector!
            TRACE_LOG("Argh! Can't seek to sector!\n");
            while(num_bytes_to_write>0) 
            {
              write_to_dma(0x00);
              fdc_add_to_crc(CRC,0x00);
              num_bytes_to_write--;
              BytesRead++;
              byte_idx++;
              if(byte_idx>=SectorBytes)
                break;
            }
          }
          else
          {
            FILE *f=(FILE*)(FromFormat 
              ? FloppyDisk[floppyno].Format_f:FloppyDisk[floppyno].f);
            fseek(f,byte_idx,SEEK_CUR);
            BYTE Temp;
            for(;num_bytes_to_write>0;num_bytes_to_write--)
            {
              if(fread(&Temp,1,1,f)==0) 
              {
                if(floppy_handle_file_error(floppyno,0,SectorNum,byte_idx,
                  FromFormat)) 
                {
                  Fdc.str=FDC_STR_MO|FDC_STR_SE|FDC_STR_BSY;
                  Error=true;
                  num_bytes_to_write=0;
                  break;
                }
              }
              fdc_add_to_crc(CRC,Temp);
              if(DMA_ADDRESS_IS_VALID_W && Dma.Counter)
                Dma.AddToFifo(Temp);
              BytesRead++;
              byte_idx++;
              if(byte_idx>=SectorBytes) 
                break;
            }
          }
        }
        byte_idx-=SectorBytes;
        // Write CRC
/*
CRC                                2           2           2
*/
        if(num_bytes_to_write>0&&byte_idx>=0&&byte_idx<2)
        {
          if(byte_idx==0)
          {
            write_to_dma(HIBYTE(CRC));          // End of Data Field (CRC)
            byte_idx++;
            BytesRead++;
            num_bytes_to_write--;
          }
          if(byte_idx==1&&num_bytes_to_write>0)
          {
            write_to_dma(LOBYTE(CRC));          // End of Data Field (CRC)
            byte_idx++;
            BytesRead++;
            num_bytes_to_write--;
          }
        }
        byte_idx-=2;//?
        // Write Gap 4
/*
Gap 4 Post Data                   40          40           1      4E
*/
        BYTE gap4bytes=FloppyDisk[DRIVE].PostDataGap(); // (nSects>=11?1:24);
        if(num_bytes_to_write>0&&byte_idx>=0&&byte_idx<gap4bytes) 
        {
          while(num_bytes_to_write>0)
          {
            write_to_dma(0x4e);
            byte_idx++;
            num_bytes_to_write--;
            BytesRead++;
            if(byte_idx>=gap4bytes) 
            {
              // Move to next sector (-1 because we ++ below)
              part=(IDListIdx+1)*154-1;
              break;
            }
          }
        }
      }
      else
      {
        // End of track, read in 0x4e
        BYTE gap5bytes=(nSects>=11?20:16); //ProCopy 1.5 Analyze
        // isn't it a bug anyway? More gap with 11 than 9-10 ???
        write_to_dma(0x4e,gap5bytes);
        BytesRead+=gap5bytes;
      }
    }
  }
  part++;
  if(BytesRead>=TrackBytes) 
  {//finished reading in track
    Fdc.str=FDC_STR_MO;  //all fine!
    agenda_fdc_finished(0);
    DBG_LOG(Str("FDC: Read track finished, t=")+hbl_count);
  }
  else if(Error==0)
  {   //read more of the track
    int bytes_per_second=FloppyDisk[DRIVE].TrackBytes*5;
    int hbls_per_second=HBL_PER_SECOND;
    int n_hbls=hbls_per_second/(bytes_per_second/16);
/*  Correct HBL drift for Read Track.
    Timing is important for ProCopy Analyze.
*/
    WORD current_byte=FloppyDrive[DRIVE].BytePosition();
    WORD theory_byte=(WORD)RealPart*16;
    if(current_byte>theory_byte) //only one way
    {
//      TRACE_LOG("TRACK READ Drift theory %d byte %d #hbl %d\n",RealPart*16,FloppyDrive[DRIVE].BytePosition(),n_hbls);
      n_hbls--;
    }
    RealPart++;
    agenda_add(agenda_floppy_read_track,n_hbls,MAKELONG(part,RealPart)); 
  }
}


/*
DISK FORMATTING:
----------------

The 177x formats disks according to the IBM 3740 or System/34
standard.  See the Write Track command for the CPU formatting method.
The recommended physical format for 256-byte sectors is as follows.

Number of Bytes     Value of Byte      Comments
---------------     -------------      --------
60                  $4e                Gap 1 and Gap 3.  Start and end of index
                                       pulse.
12                  $00                Gap 3.  Start of bytes repeated for each
                                       sector.
3                   $a1                Gap 3.  Start of ID field.  See section
                                       on Write Track command.
1                   $fe                ID address mark
1                   track #            $00 through $4c (0 through 76)
1                   side #             0 or 1
1                   sector #           $01 through $10 (1 through 16)
1                   length code        See section on Read Address command.
2                   CRC                End of ID field.  See section on Write
                                       Track command.
22                  $4e                Gap 2.
12                  $00                Gap 2.  During Write Sector commands the
                                       drive starts writing at the start
                                       of this.
3                   $a1                Gap 2.  Start of data field.  See
                                       section on Write Track command.
1                   $fb                data address mark
256                 data               Values $f5, $f6, and $f7 invalid.  See
                                       section on Write Track command.  IBM
                                       uses $e5.
2                   CRC                End of data field.  See section on Write
                                       Track command.
24                  $4e                Gap 4.  End of bytes repeated for each
                                       sector.  During Write Sector
                                       commands the drive stops writing shortly
                                       after the beginning of this.
668                 $4e                Continue writing until the 177x
                                       generates an interrupt.  The listed byte
                                       count is approximate.

Variations in the recommended formats are possible if the following
requirements are met:
(1)  Sector size must be 128, 256, 512, or 1024 bytes.
(2)  All address mark indicators ($a1) must be 3 bytes long.
(3)  The $4e section of Gap 2 must be 22 bytes long.  The $00 section of Gap 2
     must be 12 bytes long.
(4)  The $4e section of Gap 3 must be at least 24 bytes long.  The $00 section
     of Gap 3 must be at least 8 bytes long.
(5)  Gaps 1 and 4 must be at least 2 bytes long.  These gaps should be longer
     to allow for PLL lock time, motor speed variations, and write splice time.

The 177x does not require an Index Address Mark.
*/


void agenda_floppy_write_track(int part) {
  static int SectorLen,nSector=-1;
  int floppyno=floppy_current_drive();
  TSF314 *floppy=&FloppyDrive[floppyno];
  BYTE Data;
  int TrackBytes=TrackBytes=FloppyDisk[floppyno].TrackBytes; // Double density format only
  if(FloppyDisk[floppyno].ReadOnly||FloppyDisk[floppyno].STT_File)
  { // STT is no solution for Format
    Fdc.str=FDC_STR_MO|FDC_STR_WP;
    agenda_fdc_finished(0);
    return;
  }
  if(floppy->Empty()) 
  {
    Fdc.str=FDC_STR_MO|FDC_STR_SE|FDC_STR_BSY;
    return;
  }
  FloppyDisk[floppyno].WrittenTo=true;
  bool Error=0;
  Fdc.str|=FDC_STR_BSY;
  if(FLOPPY_ACCESS_FF(floppyno))
    floppy_access_ff_counter=FLOPPY_FF_VBL_COUNT;
  if(!ADAT)
  {
    agenda_delete(agenda_fdc_motor_flag_off);
    agenda_add(agenda_fdc_motor_flag_off,milliseconds_to_hbl(1800),0);
  }
  if(part==0)
  { // Find data/address marks
    // Find address marks and read SectorLen and nSector from the address
    // Must have [[0xa1] 0xa1] 0xa1 0xfe or [[0xc2] 0xc2] 0xc2 0xfe
    // Find data marks and increase part
    // Must have [[0xa1] 0xa1] 0xa1 0xfb or [[0xc2] 0xc2] 0xc2 0xfb
    for(int n=0;n<16;n++)
    {
      Data=Dma.GetFifoByte();
      floppy_write_track_bytes_done++;
      if(Data==0xa1||Data==0xf5||Data==0xc2||Data==0xf6)
      { // Start of gap 3
        int Timeout=10;
        do {
          Data=Dma.GetFifoByte();
          floppy_write_track_bytes_done++;
        } while((Data==0xa1||Data==0xf5||Data==0xc2||Data==0xf6)
          &&(--Timeout)>0);
        if(Data==0xfe)
        { // Found address mark
          if(dma_address+4<himem) 
          {
            nSector=PEEK(dma_address+2);
            switch (PEEK(dma_address+3)) {
            case 0:  SectorLen=128;break;
            case 1:  SectorLen=256;break;
            case 2:  SectorLen=512;break;
            case 3:  SectorLen=1024;break;
            default: Error=true;
            }
            if(Error) 
            {
              DBG_LOG(Str("FDC: Format data with invalid sector length (")+PEEK(dma_address+3)+"). Skipping this ID field.");
              Error=0;
            }
          }
        }
        else if(Data==0xfb)
        {
          part++; // Read next SectorLen bytes of data
          break;
        }
      }
    }
  }
  else
  {
    bool IgnoreSector=true;
    if(nSector>=0)
      IgnoreSector=FloppyDisk[floppyno].SeekSector(floppy_current_side(),
        FloppyDrive[floppyno].track,nSector,true);
    if(IgnoreSector) 
    {
      LOG_ONLY(if(nSector<0) DBG_LOG("FDC: Format sector data with no address, it will be lost in the ether"); )
      LOG_ONLY(if(nSector>=0) DBG_LOG("FDC: Format can't write sector, sector number too big for this type of image"); )
      dma_address+=SectorLen;
      floppy_write_track_bytes_done+=(WORD)SectorLen;
      part=0;
      nSector=-1;
    }
    else
    {
      fseek(FloppyDisk[floppyno].Format_f,(part-1)*16,SEEK_CUR);
      FloppyDisk[floppyno].FormatMostSectors=MAX(nSector,FloppyDisk[floppyno].FormatMostSectors);
      FloppyDisk[floppyno].FormatLargestSector=MAX(SectorLen,FloppyDisk[floppyno].FormatLargestSector);
      FloppyDisk[floppyno].TrackIsFormatted[floppy_current_side()]
        [FloppyDrive[floppyno].track]=true;
      for(int bb=0;bb<16;bb++) 
      {
        Dma.GetFifoByte();
        if(fwrite(&Data,1,1,FloppyDisk[floppyno].Format_f)==0) 
        {
          Error=true;
          if(FloppyDisk[floppyno].ReopenFormatFile())
          {
            FloppyDisk[floppyno].SeekSector(floppy_current_side(),
              FloppyDrive[floppyno].track,nSector,true);
            fseek(FloppyDisk[floppyno].Format_f,(part-1)*16+bb,SEEK_CUR);
            Error=(fwrite(&Data,1,1,FloppyDisk[floppyno].Format_f)==0);
          }
        }
        if(Error) 
          break;
        floppy_write_track_bytes_done++;
      }
      part++;
      if((part-1)*16>=SectorLen) 
      {
        nSector=-1;
        part=0;
      }
    }
  }
  if(floppy_write_track_bytes_done>TrackBytes) 
  {
    DBG_LOG(Str("FDC: Format finished, wrote ")+floppy_write_track_bytes_done+" bytes");
    Fdc.str=FDC_STR_MO;  //all fine!
    agenda_fdc_finished(0);
    fflush(FloppyDisk[floppyno].Format_f);
  }
  else if(Error)
  {
    DBG_LOG("FDC: Format aborted, can't write to format file");
    Fdc.str=FDC_STR_MO|FDC_STR_SE|FDC_STR_BSY;
  }
  else
  { //write more of the track
    int bytes_per_second=FloppyDisk[DRIVE].TrackBytes*5;
    int hbls_per_second=HBL_PER_SECOND;
    int n_hbls=hbls_per_second/(bytes_per_second/16);
    agenda_add(agenda_floppy_write_track,n_hbls,part);
  }
}


void fdc_add_to_crc(WORD &crc,BYTE data) {
  // The CRC polynomial is x^16+x^12+x^5+1.
  for(int i=0;i<8;i++)
    crc=WORD((crc<<1)^((((crc>>8)^(data<<i))&0x0080)?0x1021:0));
}


#if USE_PASTI

void PASTI_CALLCONV pasti_handle_return(struct pastiIOINFO *pPIOI) {
//  LOG_TO(LOGSECTION_PASTI,Str("PASTI: Handling return, update cycles=")+pPIOI->updateCycles+" irq="+pPIOI->intrqState+" Xfer="+pPIOI->haveXfer);
  pasti_update_time=ABSOLUTE_CPU_TIME+pPIOI->updateCycles;
  BOOL old_irq=(Mfp.reg[MFPR_GPIP] & BIT_5)==0; // 0=irq on
  if(old_irq!=pPIOI->intrqState) 
  {
#if defined(SSE_DISK_GHOST)
    if(!Fdc.Lines.CommandWasIntercepted)
#endif
      mfp_gpip_set_bit(MFP_GPIP_FDC_BIT,!pPIOI->intrqState);
    if(FLOPPY_ACCESS_FF(DRIVE))
      floppy_access_ff_counter=FLOPPY_FF_VBL_COUNT;
  }
  if(pPIOI->haveXfer)
  {
    dma_address=pPIOI->xferInfo.xferSTaddr; //SS not the regs
    if(pPIOI->xferInfo.memToDisk) 
    {
      //LOG_TO(LOGSECTION_PASTI,Str("PASTI: DMA transfer ")+pPIOI->xferInfo.xferLen+" bytes from address=$"+HEXSl(dma_address,6)+" to pasti buffer");
      for(DWORD i=0;i<pPIOI->xferInfo.xferLen;i++)
        if(DMA_ADDRESS_IS_VALID_R)
          LPBYTE(pPIOI->xferInfo.xferBuf)[i]=Dma.GetFifoByte();
    }
    else
    {
      //LOG_TO(LOGSECTION_PASTI,Str("PASTI: DMA transfer ")+pPIOI->xferInfo.xferLen+" bytes from pasti buffer to address=$"+HEXSl(dma_address,6));
      for(DWORD i=0;i<pPIOI->xferInfo.xferLen;i++) 
      {
        if(DMA_ADDRESS_IS_VALID_W) 
        {
          Dma.AddToFifo(LPBYTE(pPIOI->xferInfo.xferBuf)[i]);
          //DEBUG_CHECK_WRITE_B(dma_address);
        }
      }
    }
  }
#if defined(SSE_DEBUG)
  if(TRACE_ENABLED(LOGSECTION_FDC)&&!old_irq&&old_irq!=(BYTE)pPIOI->intrqState) 
    Dma.UpdateRegs(true);
#endif
  if(pPIOI->brkHit)
  {
    if(runstate==RUNSTATE_RUNNING) 
    {
      runstate=RUNSTATE_STOPPING;
      SET_WHY_STOP("Pasti breakpoint");
    }
    DEBUG_ONLY(if(debug_in_trace) SET_WHY_STOP("Pasti breakpoint"); )
  }
#if defined(SSE_DRIVE_SOUND)
  Dma.UpdateRegs();
  if(OPTION_DRIVE_SOUND && pPIOI->intrqState&&!old_irq)
    FloppyDrive[DRIVE].Sound_CheckIrq();
#endif
}


#undef LOGSECTION
#define LOGSECTION LOGSECTION_PASTI


void PASTI_CALLCONV pasti_motor_proc(BOOL state) {
  TRACE_FDC("(pasti) ");
  FloppyDrive[DRIVE].Motor(state!=FALSE); // for the trace
}


void PASTI_CALLCONV pasti_log_proc(const char * LOG_ONLY(text)) {
#if defined(DEBUG_BUILD)
  TRACE_LOG("Pasti: %s\n",(char*)text);
  LOG_TO(LOGSECTION_PASTI,Str("PASTI: ")+text);
#endif
}


void PASTI_CALLCONV pasti_warn_proc(const char *text) {
  Alert((char*)text,"Pasti Warning",0);
}


#if defined(SSE_DISK_GHOST)
/*  Little function to update the value of some register inside
    pasti, hopefully without triggering anything.
*/

void pasti_update_reg(unsigned addr,unsigned data) {
  struct pastiIOINFO pioi;
  pioi.stPC=pc; 
  pioi.cycles=ABSOLUTE_CPU_TIME;    
  pioi.addr=addr;
  pioi.data=data;
  pasti->Io(PASTI_IOWRITE,&pioi);
}

#endif

#endif//#if USE_PASTI

#undef LOGSECTION

#define LOGSECTION LOGSECTION_FDC


void TWD1772::Reset() {
  str=2;
  InterruptCondition=0;
#if defined(SSE_DISK_GHOST)
  Lines.CommandWasIntercepted=0;
#endif
  prg_phase=WD_READY;
  StatusType=1;  //Fdc.StatusType=2;???
  Lines.motor=false;
  //tr=sr=dr=0; //? not clear in doc // no: So Watt
  floppy_irq_flag=0;
  fdc_spinning_up=0;
}


#if defined(SSE_DISK_GHOST)
/*  Ghost disks are described in doc/sse/stg.txt and implemented in
    disk_ghost.
    Here's the part that does the interception.
*/

bool TWD1772::CheckGhostDisk(BYTE drive, BYTE io_src_b) {
  TWD1772IDField myIDField;
  myIDField.track=FloppyDrive[drive].track; // not TR
  myIDField.side=Psg.SelectedSide;
  myIDField.num=sr; 
  WORD nbytes=512;
/*  Simplest case: the game writes sectors using the "single sector"
    way.
    We don't have "len" info, we guess.
    The sector counter could be set for multiple command calls (eg 9
    for all sectors), so we only envision 512/1024.
*/
  if((io_src_b&0xF0)==0xA0 || (io_src_b&0xF0)==0x80)
  {
    switch(Dma.Counter) {
    case 0:
      nbytes=0;
      break;
    case 2:
      nbytes=1024;
      myIDField.len=3;
      break;
    default:
      myIDField.len=2;
    }//sw
  }
  // WRITE 1 SECTOR
  if(nbytes && (io_src_b&0xF0)==0xA0)
  {
    cr=io_src_b; //update this...
    if(FloppyDrive[drive].CheckGhostDisk(true))
    {
      // bytes ST memory -> our buffer
      for(int i=0;i<nbytes;i++)
        *(GhostDisk[drive].SectorData+i)=Dma.GetFifoByte();
      GhostDisk[drive].WriteSector(&myIDField);
      str=FDC_STR_MO;
      Lines.CommandWasIntercepted=1;
      agenda_fdc_finished(0);
    }
  }
  // READ 1 SECTOR
  if(nbytes && (io_src_b&0xF0)==0x80)
  {   
    // sector is in ghost image?
    if(FloppyDrive[drive].CheckGhostDisk(false)
      && GhostDisk[drive].ReadSector(&myIDField))
    {
      cr=io_src_b; //update this...
      str=FDC_STR_MO;
      for(int i=0;i<nbytes;i++)
        Dma.AddToFifo(*(GhostDisk[drive].SectorData+i));
      Lines.CommandWasIntercepted=1;
      agenda_fdc_finished(0); 
    }
  }
/*  For multiples sectors:
    - TOS 1.0 used this instead of R/W 1 sector
    - We don't IRQ, we hope the program will use D0.
    - We assume sectors are 512 bytes, even if it's not the case it could work.
*/
  if((io_src_b&0xF0)==0xB0 || (io_src_b&0xF0)==0x90)
  {
    myIDField.len=2;
    nbytes=myIDField.nBytes(); 
  }
  // WRITE MULTIPLE SECTORS
  if(nbytes && (io_src_b&0xF0)==0xB0)
  {
    if(FloppyDrive[drive].CheckGhostDisk(true))
    {
      cr=io_src_b; //update this...
      // for all sectors
      const int k=Dma.Counter;
      for(int j=0;j<k;j++)
      {
        // bytes ST memory -> our buffer
        for(int i=0;i<nbytes;i++)
        {
          *(GhostDisk[drive].SectorData+i)=Dma.GetFifoByte();
        }//nxt i
        GhostDisk[drive].WriteSector(&myIDField); // write 1 sector
        myIDField.num=++sr;
      }//nxt j
      str=FDC_STR_MO; // but no IRQ
      Lines.CommandWasIntercepted=1;
    }
  }//multi-write
  // READ MULTIPLE SECTORS
  if(nbytes && (io_src_b&0xF0)==0x90)
  { 
    if(FloppyDrive[drive].CheckGhostDisk(false))
    {
      // for all sectors
      const int k=Dma.Counter;
      for(int j=0;j<k;j++)
      {
        // sector is in ghost image?
        if(FloppyDrive[drive].ghost 
          && GhostDisk[drive].ReadSector(&myIDField))
        {
          cr=io_src_b; //update this...
          for(int i=0;i<nbytes;i++)
          {
            Dma.AddToFifo( *(GhostDisk[drive].SectorData+i) );
          } //nxt i
          str=FDC_STR_MO;
          Lines.CommandWasIntercepted=1;
          myIDField.num=++sr; // update both sr and ID field's num
        }
      }//nxt j
    }
  }//multi-read
  // FAKE FORMAT
  if((io_src_b&0xF0)==0xF0)
  {
    str=FDC_STR_MO;
    Lines.CommandWasIntercepted=1;
    //Dma.BaseAddress+=6250;    
    dma_address+=6250; 
    Dma.Counter-=6250/512;
    agenda_fdc_finished(0);
  }
#if USE_PASTI
/*  Pasti keeps its own variables for all DMA/FDC emulation. This is what
    makes mixing difficult. Updating here is the best way I've found so
    far. TODO
*/
  if(Lines.CommandWasIntercepted && hPasti
    && FloppyDrive[drive].ImageType.Manager==MNGR_PASTI)
  {
    //ASSERT(!OPTION_PASTI_JUST_STX||FloppyDrive[drive].ImageType.Extension==EXT_STX);
    pasti_update_reg(0xff8609,Mmu.uDmaCounter.d8[B2]); //    (Dma.BaseAddress&0xff0000)>>16);
    pasti_update_reg(0xff860b,Mmu.uDmaCounter.d8[B1]); //     (Dma.BaseAddress&0xff00)>>8);
    pasti_update_reg(0xff860d,Mmu.uDmaCounter.d8[B0]); //      (Dma.BaseAddress&0xff));
  }
#endif
  return (bool) Lines.CommandWasIntercepted;
}

#endif//ghost


/*  DmaIO -> WD1772IO
    IO is complicated because there are various WD1772 emulations running,
    we must call the correct one!
*/

BYTE TWD1772::IORead(BYTE Line) {
  //ASSERT( Line<=3 );
  BYTE &drive=DRIVE;
  BYTE ior_byte;  
  // Steem handling
  switch(Line) {
  default://case 0: // str
#if defined(SSE_DISK_GHOST)
    if(Lines.CommandWasIntercepted)
      ior_byte=str; // just "motor on" $80, but need agenda???
    else
#endif
    {
      // IP
      if(floppy_track_index_pulse_active())
        str|=FDC_STR_IP;
      else
        str&=BYTE(~FDC_STR_IP);
      if(StatusType) // type I command status
      {
        // WP
        // disk has just been changed (30 VBL set at SetDisk())
        if(floppy_mediach[drive])
        {
          str&=(~FDC_STR_WP);
          if(floppy_mediach[drive]/10!=1) 
            str|=FDC_STR_WP;
        }
        // Permanent status if disk is in
        else if(FloppyDisk[drive].ReadOnly && !FloppyDrive[drive].Empty())
          str|=FDC_STR_WP;
        // SU
        if(fdc_spinning_up) //should be up-to-date for WD1772 emu too
          str&=BYTE(~FDC_STR_SU);
        else
          str|=FDC_STR_SU;
        // TR0: compute (again) now TODO
        Lines.track0=(FloppyDrive[drive].track==0
          && (num_connected_floppies!=DRIVE)); //update line...
        if(Lines.track0)
          str|=FDC_STR_T0;
        else
          str&=~FDC_STR_T0;
      } 
      if((Mfp.reg[MFPR_GPIP] & BIT_5)==0) // IRQ is currently raised
      {
        LOG_ONLY( DEBUG_ONLY( if (stem_runmode==STEM_MODE_CPU) ) LOG_TO(LOGSECTION_FDC,Str("FDC: ")+HEXSl(old_pc,6)+
          " - Reading status register as "+Str(itoa(str,d2_t_buf,2)).LPad(8,'0')+
          " ($"+HEXSl(str,2)+"), clearing IRQ"); )
          floppy_irq_flag=0;
/*
"When using the immediate interrupt condition (i3 = 1) an interrupt
 is immediately generated and the current command terminated. 
 Reading the status or writing to the Command Register does not
 automatically clear the interrupt. The Hex D0 is the only command 
 that enables the immediate interrupt (Hex D8) to clear on a subsequent 
 load Command Register or Read Status Register operation. 
 Follow a Hex D8 with D0 command."
 -> More precisely: with D8, for both read str (here) and write cr:
 "clear IRQ if no condition", then "clear condition",
*/
        if(InterruptCondition!=8)
          mfp_gpip_set_bit(MFP_GPIP_FDC_BIT,true); // Turn off IRQ output
        InterruptCondition=0;
      }
      Lines.irq=0;
      ior_byte=str;
    }
    break;
  case 1:
    ior_byte=tr;
    TRACE_FDC("FDC tr R %d PC %X\n",ior_byte,old_pc);
    break;
  case 2:
    ior_byte=sr;
    TRACE_FDC("FDC sr R %d PC %X\n",ior_byte,old_pc);
    break;
  case 3:
    ior_byte=dr;
    TRACE_FDC("FDC dr R %d PC %X\n",ior_byte,old_pc);
    break;
  }//sw
  // CAPS handling
#if defined(SSE_DISK_CAPS)
  if(FloppyDrive[drive].ImageType.Manager==MNGR_CAPS)
    ior_byte=(BYTE)Caps.ReadWD1772(Line);
#endif
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
  if(!Line&&(TRACE_MASK3&TRACE_CONTROL_FDCSTR))
  {
    if(hPasti&&(pasti_active||FloppyDrive[drive].ImageType.Extension==EXT_STX))
    {
      Dma.UpdateRegs();
      ior_byte=Fdc.str;
    }
    TRACE_FDC("PC %X FDC str %X ",old_pc,ior_byte);
    TraceStatus();
    TRACE_FDC("\n");
  }
#endif
  return ior_byte;
}


void TWD1772::IOWrite(BYTE Line,BYTE io_src_b) {
  //ASSERT( Line<=3 );
  BYTE drive=DRIVE;
  switch(Line) {
  case 0: // Write CR
    old_cr=cr;
    if(io_src_b==0xFF)
      break; // ignore - other conditions handled in respective managers
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
    Dma.Datachunk=0;
#endif
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
    if(TRACE_MASK3 & TRACE_CONTROL_FDCREGS)
    {
      if( FloppyDrive[drive].ImageType.Manager==MNGR_WD1772 
        && (cr&0xF0)==0x90 && !(str&FDC_STR_RNF))
        TRACE_LOG("\n");
      BYTE drive_char= (psg_reg[PSGR_PORT_A]&6)==6? '?' : 'A'+drive;
   //TRACE_FDC(" %d %d ",ACT,FloppyDrive[drive].BytePosition());
      TRACE_FDC("%d FDC(%d) CR $%02X %c:%d STR %X TR %d CYL %d SR %d DR %d DMA $%X #%d PC $%X\n",
        ACT,FloppyDrive[drive].ImageType.Manager,io_src_b,drive_char,CURRENT_SIDE,str,tr,CURRENT_TRACK,sr,dr,dma_address,Dma.Counter,old_pc);
    }
    Dma.last_act=ACT;
#endif
#if defined(SSE_DEBUGGER_BOOT_CHECKSUM)
    // Gives the checksum of bootsector ($1234 means executable).
    if(FloppyDrive[drive].SectorChecksum&&(TRACE_MASK3&TRACE_CONTROL_DISKBOOT))
      TRACE_FDC("%c: bootsector checksum $%X (%x)\n",'A'+drive,FloppyDrive[drive].SectorChecksum,0x1234-FloppyDrive[drive].SectorChecksum);
#endif
#if defined(SSE_STATS)
    if(FloppyDrive[drive].SectorChecksum)
      Stats.boot_checksum[drive][CURRENT_SIDE]=FloppyDrive[drive].SectorChecksum;
#endif
    FloppyDrive[drive].SectorChecksum=0;
#if defined(SSE_STATS)
    if((io_src_b&0xF0)==0xF0 || (io_src_b&0xE0)==0xA0) // write commands
      Stats.mskSpecial|=Stats.WRITE_DISK;
#endif
#if defined(SSE_DISK_GHOST)
    Lines.CommandWasIntercepted=0; // reset this at each new whatever command 
    if(OPTION_GHOST_DISK)
    {
      if((FloppyDisk[drive].ReadOnly) || // why not?
        (FloppyDrive[drive].ImageType.Extension==EXT_STX)
        || FloppyDrive[drive].ImageType.Manager==MNGR_CAPS
        || FloppyDrive[drive].ImageType.Extension==EXT_SCP)
      {
        CheckGhostDisk(drive,io_src_b); // updates CommandWasIntercepted
      }
    }
#endif//ghost
#if defined(SSE_DRIVE_SOUND)
    if(OPTION_DRIVE_SOUND)
#if defined(SSE_DISK_GHOST)
      if(!Lines.CommandWasIntercepted) //would mess registers, and is instant
#endif
      {
        Dma.UpdateRegs();
        FloppyDrive[drive].old_track=FloppyDrive[drive].track; //record
        FloppyDrive[drive].Sound_CheckCommand(0xF0); // only latch
      }
#endif//sound
    TimeOut=0;
#if defined(SSE_DISK_GHOST)
    if((OPTION_GHOST_DISK && Fdc.Lines.CommandWasIntercepted))
      ;
    else
#endif
    if(FloppyDrive[drive].ImageType.Manager==MNGR_STEEM && !pasti_active)
      fdc_command(io_src_b); // for ST, MSA, DIM, STT
#if defined(SSE_DISK_STW)
    else if(FloppyDrive[drive].ImageType.Manager==MNGR_WD1772)
      WriteCR(io_src_b); // for STW, SCP, HFE
#endif
#if defined(SSE_TOS_PRG_AUTORUN)
    else if(FloppyDrive[drive].ImageType.Manager==MNGR_PRG) 
      mfp_gpip_set_bit(MFP_GPIP_FDC_BIT,0); //hack to avoid long timeout
#endif
    break;
  case 1: // Write TR
#ifdef DEBUG_BUILD
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
    if(TRACE_MASK3 & TRACE_CONTROL_FDCREGS)
      TRACE_FDC("FDC tr W %d PC %X\n",io_src_b,old_pc);
#endif
    LOG_TO(LOGSECTION_FDC,EasyStr("FDC: ")+HEXSl(old_pc,6)+" - Setting FDC track register to "+io_src_b);
#endif
    // original doc states "This register should not be loaded when the device 
    // is busy", not that it won't
    tr=io_src_b;
    break;
  case 2: // Write SR
#ifdef DEBUG_BUILD
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
    if(TRACE_MASK3 & TRACE_CONTROL_FDCREGS)
      TRACE_FDC("FDC sr W %d PC %X\n",io_src_b,old_pc);
#endif
    LOG_TO(LOGSECTION_FDC,EasyStr("FDC: ")+HEXSl(old_pc,6)+" - Setting FDC sector register to "+io_src_b);
#endif
    // original doc states "This register should not be loaded when the device 
    // is busy", not that it won't
    sr=io_src_b;
#if defined(SSE_STATS) // safer to check all here at SR
    if(tr>Stats.nTrack[drive])
      Stats.nTrack[drive]=tr;
    if(CURRENT_SIDE>Stats.nSide[drive])
      Stats.nSide[drive]=CURRENT_SIDE;
    if(sr>Stats.nSector[drive])
      Stats.nSector[drive]=sr;
#endif
    break;
  case 3: // Write DR
#ifdef DEBUG_BUILD
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
    if(TRACE_MASK3 & TRACE_CONTROL_FDCREGS)
      TRACE_FDC("FDC dr W %d PC %X\n",io_src_b,old_pc);
#endif
    LOG_TO(LOGSECTION_FDC,EasyStr("FDC: ")+HEXSl(old_pc,6)+" - Setting FDC data register to "+io_src_b);
#endif
    dr=io_src_b;
    break;
  }//sw
#if defined(SSE_DISK_CAPS)
  if(FloppyDrive[drive].ImageType.Manager==MNGR_CAPS)
#if defined(SSE_DISK_GHOST)
    if(OPTION_GHOST_DISK && Lines.CommandWasIntercepted)
    {
      TRACE_FDC("Ghost - Caps doesn't get command %x\n",io_src_b);
      Caps.fdc.r_command=cr; //update this...
    }
    else
#endif
      Caps.WriteWD1772(Line,io_src_b);
#endif
}


BYTE TWD1772::CommandType(int command) {
  // return type 1-4 of this FDC command
  if(command==-1) //default: current command
    command=cr;
  BYTE type;
  if(!(command&BIT_7))
    type=1;
  else if(!(command&BIT_6))
    type=2;
  else if((command&0xF0)==0xD0) //1101
    type=4;
  else
    type=3;
  return type;
}


bool TWD1772::WritingToDisk() {// could do this at DMA level?
  return((cr&0xF0)==0xF0 || (cr&0xE0)==0xA0 || (cr&0xE0)==0xB0) ;
}


/*  This is yet another WD1772 emulation, specially written to handle
    STW disk images. 
    A goal from the start was to be able to use it for another format
    like SCP as well, hence we work with a spinning drive and a flow of
    bits or bytes.
    Since v3.7.2, this emu is used for HFE disk image support too, so
    this "rewrite" proved much useful.
    We follow Western Digital flow charts, with some additions and (gasp!) 
    corrections. 
    Because we use Dma.Drq() for each byte, the agenda system is too gross.
    Each time an operation takes time, we set up an event that sends here.
    Otherwise we use recursion to hop to the next phase, it's better than
    goto.
    So it doesn't work with emulation cycles like CapsImg but with timed
    events like Pasti. Still not sure what's the best approach here.
    For a better support of SCP format, we integrated a "data separator"
    inspired by their competitor SPS/CAPS/Kryoflux, as well as a "DPLL"
    inspired by MESS.
*/


#if defined(SSE_DEBUG)

char* wd_phase_name[]={  // Coooool! note change if change enum !!!!!
    "WD_READY",
    "WD_TYPEI_SPINUP",
    "WD_TYPEI_SPUNUP", // spunup must be right after spinup
    "WD_TYPEI_SEEK",
    "WD_TYPEI_STEP_UPDATE",
    "WD_TYPEI_STEP",
    "WD_TYPEI_STEP_PULSE",
    "WD_TYPEI_CHECK_VERIFY",
    "WD_TYPEI_HEAD_SETTLE",
    "WD_TYPEI_FIND_ID",
    "WD_TYPEI_READ_ID", // read ID must be right after find ID
    "WD_TYPEI_TEST_ID", // test ID must be right after read ID
    "WD_TYPEII_SPINUP",
    "WD_TYPEII_SPUNUP", // spunup must be right after spinup
    "WD_TYPEII_HEAD_SETTLE", //head settle must be right after spunup
    "WD_TYPEII_FIND_ID",
    "WD_TYPEII_READ_ID", // read ID must be right after find ID
    "WD_TYPEII_TEST_ID", // test ID must be right after read ID
    "WD_TYPEII_FIND_DAM",
    "WD_TYPEII_READ_DATA",
    "WD_TYPEII_READ_CRC",
    "WD_TYPEII_CHECK_MULTIPLE",
    "WD_TYPEII_WRITE_DAM",
    "WD_TYPEII_WRITE_DATA",
    "WD_TYPEII_WRITE_CRC",
    "WD_TYPEIII_SPINUP",
    "WD_TYPEIII_SPUNUP", // spunup must be right after spinup
    "WD_TYPEIII_HEAD_SETTLE", //head settle must be right after spunup
    "WD_TYPEIII_IP_START", // start read/write
    "WD_TYPEIII_FIND_ID",
    "WD_TYPEIII_READ_ID", // read ID must be right after find ID
    "WD_TYPEIII_TEST_ID",
    "WD_TYPEIII_READ_DATA",
    "WD_TYPEIII_WRITE_DATA",
    "WD_TYPEIII_WRITE_DATA2", // CRC is 1 byte in RAM -> 2 bytes on disk
    "WD_TYPEIV_4", // $D4
    "WD_TYPEIV_8", // $D8
    "WD_MOTOR_OFF",
  };


/* STR Bits:
Bit 7 - Motor On.  This bit is high when the drive motor is on, and
low when the motor is off.

Bit 6 - Write Protect.  This bit is not used during reads.  During
writes, this bit is high when the disk is write protected.

Bit 5 - Spin-up / Record Type.  For Type I commands, this bit is low
during the 6-revolution motor spin-up time.  This bit is high after
spin-up.  For Type II and Type III commands, Bit 5 low indicates a
normal data mark.  Bit 5 high indicates a deleted data mark.

Bit 4 - Record Not Found.  This bit is set if the 177x cannot find the
track, sector, or side which the CPU requested.  Otherwise, this bit
is clear.

Bit 3 - CRC Error.  This bit is high if a sector CRC on disk does not
match the CRC which the 177x computed from the data.  The CRC
polynomial is x^16+x^12+x^5+1.  If the stored CRC matches the newly
calculated CRC, the CRC Error bit is low.  If this bit and the Record
Not Found bit are set, the error was in an ID field.  If this bit is
set but Record Not Found is clear, the error was in a data field.

Bit 2 - Track Zero / Lost Data.  After Type I commands, this bit is 0
if the mechanism is at track zero.  This bit is 1 if the head is not
at track zero.  After Type II or III commands, this bit is 1 if the
CPU did not respond to Data Request (Status bit 1) in time for the
177x to maintain a continuous data flow.  This bit is 0 if the CPU
responded promptly to Data Request.

Bit 1 - Index / Data Request.  On Type I commands, this bit is high
during the index pulse that occurs once per disk rotation.  This bit
is low at all times other than the index pulse.  For Type II and III
commands, Bit 1 high signals the CPU to handle the data register in
order to maintain a continuous flow of data.  Bit 1 is high when the
data register is full during a read or when the data register is empty
during a write.  "Worst case service time" for Data Request is 23.5
cycles.

Bit 0 - Busy.  This bit is 1 when the 177x is busy.  This bit is 0
when the 177x is free for CPU commands.
*/


void TWD1772::TraceStatus() {
  // this embarrassing part proves my ignorance!
  int type=CommandType(Fdc.cr);
  TRACE_LOG("( ");
  if(str&0x80)
    TRACE_LOG("MO "); //Motor On
  if(str&0x40)
    TRACE_LOG("WP "); // write protect
  if(str&0x20)
    if(type==1)
      TRACE_LOG("SU "); // Spin-up (meaning up to speed)
    else
      TRACE_LOG("RT "); //Record Type (1=deleted data)
  if(str&0x10)
    if(type==1)
      TRACE_LOG("SE "); //Seek Error
    else
      TRACE_LOG("RNF "); //Record Not Found
  if(str&0x08)
    TRACE_LOG("CRC "); //CRC Error
  if(str&0x04)
    if(type==1) 
      TRACE_LOG("T0 "); // track zero
    else
      TRACE_LOG("LD "); //Lost Data, normally impossible on ST
  if(str&0x02)
    if(type==1 || type==4) 
      TRACE_LOG("IP "); // index
    else
      TRACE_LOG("DRQ "); // data request
  if(str&0x01)
    TRACE_LOG("BSY "); // busy
    TRACE_LOG(") "); 
}


void TWD1772IDField::Trace() {
  TRACE_WD("ID T%d S%d N%d L%d CRC%X%X\n",track,side,num,len,CRC[0],CRC[1]);
}

#endif

/*  MFM. Correct field must be filled in before calling a function:
    data -> Encode() -> clock and encoded word available 
    encoded word -> Decode() -> data & clock available 
    If mode is FORMAT_CLOCK, the clock byte will have a missing bit
    for bytes $A1 and $C2.
    The STW format could have been done without MFM encoding but it is
    necessary for the HFE format anyway.
*/

void TWD1772MFM::Decode() {
  WORD encoded_shift=encoded;
  data=clock=0; //BYTEs
  for(int i=0;i<8;i++)
  {
    clock|= (encoded_shift&0x8000) != 0;
    if(i<7)
      clock<<=1;
    encoded_shift<<=1;
    data|= (encoded_shift&0x8000) != 0;
    if(i<7)
      data<<=1,encoded_shift<<=1;
  }
}


void TWD1772MFM::Encode(int mode) {
  // 1. compute the clock
  clock=0;
  BYTE previous=data_last_bit;
  BYTE current;
  data_last_bit=data&1;
  BYTE data_shift=data;
  for(int i=0;i<8;i++)
  {
    current=data_shift&0x80;
    if(!previous && !current)
      clock|=1;
    if(i<7)
      clock<<=1;
    data_shift<<=1;
    previous=current;
  }
  if(mode==FORMAT_CLOCK)
    if(data==0xA1) // -> $4489
      clock&=~4; // missing bit 2 of clock -> bit 5 of encoded word
    else if(data==0xC2) // -> $5224
      clock&=~2; // missing bit 1 of clock -> bit 4 of encoded word
  // 2. mix clock & data to create a word
  data_shift=data;
  BYTE clock_shift=clock;
  encoded=0;
  for(int i=0;i<8;i++)
  {
    encoded|=(clock_shift&0x80) != 0;
    encoded<<=1; clock_shift<<=1;
    encoded|=(data_shift&0x80) != 0;
    if(i<7)
      encoded<<=1; 
    data_shift<<=1;
  }   
}


bool TWD1772Crc::Check(TWD1772IDField *IDField) {
  bool ok=IDField->CRC[0]==HIBYTE(crc) && IDField->CRC[1]==LOBYTE(crc);
#ifdef DEBUG_BUILD
  if(!ok)
    TRACE_WD("CRC error - computed: %X - read: %X%X\n",crc,IDField->CRC[0],IDField->CRC[1]);
#endif
  return ok;
}


// reset am detector; read returns only on AM detected or clocks elapsed
void TWD1772AmDetector::Enable() {
  Enabled=true;
  nA1=0;
#if defined(SSE_WD1772_LL)
  aminfo|=(CAPSFDC_AI_AMDETENABLE|CAPSFDC_AI_CRCENABLE);
  aminfo&=~(CAPSFDC_AI_CRCACTIVE|CAPSFDC_AI_AMACTIVE);
  amisigmask=CAPSFDC_AI_DSRAM;
#endif
}

void TWD1772AmDetector::Reset() {
#if defined(SSE_WD1772_LL)
  amdatadelay=2;
  ammarkdist=ammarktype=amdataskip=0; 
  amdecode=aminfo=amisigmask=dsr=0; // dword
  dsrcnt=0; // int
#endif
  Enable();
}


void TWD1772::Drq(bool state) {
  Lines.drq=state;
  if(state)
    str|=FDC_STR_DRQ;
  else
    str&=~FDC_STR_DRQ;
  if(state)
    Dma.Drq();
}


void TWD1772::Irq(bool state) {
  Amd.Reset();
  Amd.Enabled=false;
  if(state && !Lines.irq) // so not on "force interrupt"
  {
    IndexCounter=10;
    //TRACE_WD("%d IP for motor off\n",IndexCounter);
    prg_phase=WD_MOTOR_OFF; // will be changed for $D4
    str&=~FDC_STR_BSY;
    if(CommandType()==2 || CommandType()==3)
      str&=~FDC_STR_DRQ;
#if defined(SSE_DRIVE_SOUND)
    if(OPTION_DRIVE_SOUND && OPTION_DRIVE_SOUND_SEEK_SAMPLE)
      FloppyDrive[DRIVE].Sound_CheckIrq();
#endif
#if defined(SSE_DEBUG)
    Dma.UpdateRegs(true);
#else
    Dma.UpdateRegs();
#endif
  }
  Lines.irq=state;
  mfp_gpip_set_bit(MFP_GPIP_FDC_BIT,!state); // MFP GPIP is "active low"
  // reset drive R/W flags
  FloppyDrive[DRIVE].reading=FloppyDrive[DRIVE].writing=0;
}


void TWD1772::Motor(bool state) {
#ifdef DEBUG_BUILD
  if(state!=(bool)FloppyDrive[DRIVE].motor)
    TRACE_WD("WD motor %d\n",state);
#endif
  Lines.motor=state;
  if(state)
    str|=FDC_STR_MO;
  else
    str&=~FDC_STR_MO;
  // only on currently selected drive, if any:
  if(Psg.CurrentDrive()!=TYM2149::NO_VALID_DRIVE)
    FloppyDrive[DRIVE].Motor(state); 
#ifdef SSE_DEBUG
  else TRACE_WD("WD motor %d: no drive\n",state);
#endif
}


void TWD1772::NewCommand(BYTE command) {
  //ASSERT( IMAGE_STW || IMAGE_SCP || IMAGE_HFE);
  cr=command;
  // reset drive R/W flags
  FloppyDrive[DRIVE].reading=FloppyDrive[DRIVE].writing=0;
  BYTE type=CommandType(command);
  current_time=ACT;
  switch(type) {
  case 1: // I
    // Set Busy, Reset CRC, Seek error, DRQ, INTRQ
    str|=FDC_STR_BSY;
    str&=~(FDC_STR_CRC|FDC_STR_SE |FDC_STR_WP);
    Drq(0); // this takes care of status bit
    if(InterruptCondition!=8)
      Irq(0);
    InterruptCondition=0;
    StatusType=1;
    // should we wait for spinup (H=0)?
    if(!(cr&CR_H) && !Lines.motor)
    {
      // Set MO wait 6 index pulses
      Motor(true); // this will create the event at each IP until motor off
      IndexCounter=6;
      //TRACE_WD("%d IP to spin up\n",IndexCounter);
      prg_phase=WD_TYPEI_SPINUP;
      fdc_spinning_up=true;
    }
    else
    {
      // Doc doesn't state motor is started if it wasn't spinning yet and h=1
      // We assume.
      Motor(true); // but does it make sense?
      fdc_spinning_up=false;
      prg_phase=WD_TYPEI_SPUNUP;
      str|=FDC_STR_SU; // eg ST NICCC 2
      update_time=ACT+256; // 256 My Socks Are Weapons, Suretrip II
    }
    break;
  case 2: // II
    // Set Busy, Reset CRC, DRQ, LD, RNF, WP, Record Type
    str|=FDC_STR_BSY;
    str&=~(FDC_STR_LD|FDC_STR_RNF|FDC_STR_WP|FDC_STR_RT);
    Drq(0);
    if(InterruptCondition!=8)
      Irq(0);
    InterruptCondition=0;
    StatusType=0;
    if(!(cr&CR_H) && !Lines.motor)
    {
      Motor(true); 
      IndexCounter=6;
      //TRACE_WD("%d IP to spin up\n",IndexCounter);
      prg_phase=WD_TYPEII_SPINUP;
      fdc_spinning_up=true;
    }
    else
    {
      Motor(true); 
      fdc_spinning_up=false;
      prg_phase=WD_TYPEII_SPUNUP;
      current_time=ACT;
      OnUpdate();
    }
    break;
  case 3: // III
    str|=FDC_STR_BSY;
    str&=~(FDC_STR_LD|FDC_STR_RNF|FDC_STR_WP|FDC_STR_RT); // we add WP
    Drq(0);
    // we add this:
    if(InterruptCondition!=8)
      Irq(0);
    InterruptCondition=0;
    StatusType=0;

    // we treat the motor / H business as for type II, not as on flow chart
    if(!(cr&CR_H) && !Lines.motor)
    {
      Motor(true); 
      IndexCounter=6;
      //TRACE_WD("%d IP to spin up\n",IndexCounter);
      prg_phase=WD_TYPEIII_SPINUP;
      fdc_spinning_up=true;
    }
    else
    {
      Motor(true); 
      prg_phase=WD_TYPEIII_SPUNUP;
      fdc_spinning_up=false;
      current_time=ACT;
      OnUpdate();
    }
    break;
  case 4: // IV
    Motor(true); // also type IV triggers motor
    if(str&FDC_STR_BSY)
      str&=~FDC_STR_BSY;
    else // read str is type I if FDC wasn't busy when interrupted (doc)
    {
      StatusType=1;
      str&=~(FDC_STR_CRC|FDC_STR_LD|FDC_STR_RT|FDC_STR_RNF);
    }

    if(cr&CR_I3) // immediate, D8
    {
      InterruptCondition=8;
      Irq(true); 
      prg_phase=WD_MOTOR_OFF;
      IndexCounter=10;
      //TRACE_WD("%d IP for motor off\n",IndexCounter);
    }
    else if(cr&CR_I2) // each IP, D4
    {
      prg_phase=WD_TYPEIV_4;
      InterruptCondition=4;
      IndexCounter=1;
    }
    else // D0, just stop motor in 9 rev
    {
      // no IRQ!
      if(InterruptCondition!=8)
        Irq(false); // but could have to clear it (Wipe-Out)
      prg_phase=WD_MOTOR_OFF;
      IndexCounter=10;
      InterruptCondition=0;
      //TRACE_WD("%d IP for motor off\n",IndexCounter);
    }
    break;
  }//sw
  prepare_next_event(); //395
}


/*  Drive calls this function at IP if it's selected.
    Whether the WD1772 is waiting for it or not. 
    The WD1772 doesn't know which drive (id) it is operating but we do!
*/

void TWD1772::OnIndexPulse(int id,bool image_triggered) {
  IndexCounter--; // We set counter then decrement until 0

  if(FLOPPY_ACCESS_FF(id)) 
    floppy_access_ff_counter=FLOPPY_FF_VBL_COUNT;

#ifdef DEBUG_BUILD
  if(prg_phase>WD_READY && prg_phase<=WD_MOTOR_OFF) // anticrash
    TRACE_WD("%c: IP #%d (%s) (%s) cr %X tr %d sr %d dr %d str %X\n",'A'+id,IndexCounter,wd_phase_name[prg_phase],image_triggered?"triggered":"event",cr,tr,sr,dr,str);
#endif

  if(!IndexCounter)
  {
    if(!image_triggered)
      current_time=time_of_next_event;

    //TRACE2("prg_phase %d\n",prg_phase);
    switch(prg_phase) {
    case WD_TYPEI_SPINUP:
      str|=FDC_STR_SU;
    case WD_TYPEII_SPINUP:
    case WD_TYPEIII_SPINUP:
      prg_phase++; // we assume next phase is spunup for this optimisation
      fdc_spinning_up=false;
      OnUpdate();
      break;

    case WD_TYPEI_FIND_ID:
    case WD_TYPEI_READ_ID:
    case WD_TYPEII_FIND_ID:
    case WD_TYPEII_READ_ID:
    case WD_TYPEIII_FIND_ID: // not in doc
    case WD_TYPEIII_READ_ID:
      TRACE_WD("Find ID timeout\n");
      str|=FDC_STR_SE; // RNF is same bit as SE, OSD will be set by Dma.UpdateRegs
      Irq(true);      
      break;

    case WD_TYPEIII_IP_START: 
      IndexCounter=1;
      n_format_bytes=0;
      //TRACE_WD("%d IP for stop track operation\n",IndexCounter);
      if(cr&CR_TYPEIII_WRITE)
      {
        TRACE_WD("Format track %c:S%d T%d (DMA %d sectors)\n",'A'+DRIVE,CURRENT_SIDE,CURRENT_TRACK,Dma.Counter);
        prg_phase=WD_TYPEIII_WRITE_DATA;
        F7_escaping=false;
        OnUpdate(); // hop
      }
      else
      {
        TRACE_WD("Read track %c:S%d F%d  (DMA %d sectors)\n",'A'+DRIVE,CURRENT_SIDE,CURRENT_TRACK,Dma.Counter);
        prg_phase=WD_TYPEIII_READ_DATA;
        Amd.Reset();
        Read();
      }
      break;

    case WD_TYPEIII_WRITE_DATA:
    case WD_TYPEIII_WRITE_DATA2: //! 
    case WD_TYPEIII_READ_DATA:
      FloppyDrive[DRIVE].reading=FloppyDrive[DRIVE].writing=0; 
#if defined(SSE_DEBUG) 
#if defined(SSE_DEBUGGER_TRACE_CONTROL)
      TRACE_WD("%d bytes transferred\n",Dma.Datachunk*16);
#else
      TRACE_WD("\n"); //stop list of sector nums
#endif
#endif
      Irq(true);
      break;

    case WD_TYPEIV_4: // $D4: raise IRQ at each IP until new command
      //ASSERT( !Lines.irq );
      Irq(true);
      //ASSERT( prg_phase==WD_TYPEIV_4 );
      prg_phase=WD_TYPEIV_4;
      IndexCounter=1;
      TRACE_WD("%d IP for $D4 interrupt\n",IndexCounter);
      break;

    case WD_MOTOR_OFF:
      Motor(false);
#if defined(SSE_DISK_GHOST)
      Lines.CommandWasIntercepted=0;
#endif
      prg_phase=WD_READY;
      break;

    default: // drive is spinning, WD isn't counting
      if(!image_triggered)
        OnUpdate(); //just in case... ???
      break; 
    }//sw
  
  }//if
  else
  {
    if(!image_triggered)
      OnUpdate(); // to trigger Read() or Write() if needed: Delirious 3
  }

//  TRACE_WD("Fdc IP %d byte %d\n",IndexCounter,FloppyDisk[DRIVE].current_byte);
}


#if defined(SSE_DISK_SCP) 
// helper function
// reset rev if we're on last rev and not reading

void check_scp_rev(int drive) {
  if(IMAGE_SCP && ImageSCP[drive].rev && ImageSCP[drive].rev
    ==(ImageSCP[drive].file_header.IFF_NUMREVS-1))
  {
    ImageSCP[drive].LoadTrack(CURRENT_SIDE,CURRENT_TRACK);
  }
}

#endif


void TWD1772::OnUpdate() {

  update_time=current_time+n_cpu_cycles_per_second;

  if(TimeOut>1 && fdc_spinning_up) //? TODO
  {
    str|=FDC_STR_RNF;
    TimeOut=0;
    Irq(true); // 'IDLE' //?
    return;
  }
  BYTE &drive=DRIVE; // for tracing

  if(FloppyDrive[drive].ImageType.Manager!=MNGR_WD1772)
    return;

  switch(prg_phase)  {
    
  case WD_TYPEI_SPUNUP:
    // we come here after 6 IP or directly
#if defined(SSE_DRIVE_SOUND)
    FloppyDrive[drive].Sound_CheckCommand(cr);
#endif
    if(cr&CR_STEP) // is command a step, step-in, step-out?
    {
      // if step-in or step-out, update DIRC line
      if((cr&CR_STEP)==CR_STEP_IN) 
        Lines.direction=true;
      else if((cr&CR_STEP)==CR_STEP_OUT) 
        Lines.direction=false; 
      // goto B or C according to flag u
      prg_phase=(cr&CR_U)?WD_TYPEI_STEP_UPDATE:WD_TYPEI_STEP;
      OnUpdate();
    }
    else  // else it's seek/restore
    {
      prg_phase=WD_TYPEI_SEEK; // goto 'A'
      if((cr&CR_SEEK)==CR_RESTORE) // restore?
      {
        tr=0xFF;
        Lines.track0=(FloppyDrive[drive].track==0 && num_connected_floppies!=drive);
        if(Lines.track0)
          tr=0;
        dr=0;
      }    
      OnUpdate(); // some recursion is always cool   
    }
    break;

  case WD_TYPEI_SEEK: // 'A'
    dsr=dr;
    if(tr==dsr)
      prg_phase=WD_TYPEI_CHECK_VERIFY;
    else
    {
      Lines.direction=(dsr>tr);
      prg_phase=WD_TYPEI_STEP_UPDATE;
    }
    OnUpdate(); // some recursion is always cool
    break;

  case WD_TYPEI_STEP_UPDATE: // 'B'
    if(Lines.direction)
      tr++;
    else
      tr--;
    prg_phase=WD_TYPEI_STEP;
    OnUpdate(); // some recursion is always cool
    break;

  case WD_TYPEI_STEP: // 'C'
    Lines.track0=(FloppyDrive[drive].track==0 && num_connected_floppies!=drive);
    if(Lines.track0 && !Lines.direction)
    {
      tr=0;
      prg_phase=WD_TYPEI_CHECK_VERIFY;
      OnUpdate(); // some recursion is always cool
    }
    else
    {
      StepPulse();
/*
Delay according to r1,r0 field

Command      Bit 7     B6     B5     B4     B3     B2     B1     Bit 0
--------     -----     --     --     --     --     --     --     -----
Restore      0         0      0      0      h      V      r1     r0
Seek         0         0      0      1      h      V      r1     r0
Step         0         0      1      u      h      V      r1     r0
Step in      0         1      0      u      h      V      r1     r0
Step out     0         1      1      u      h      V      r1     r0

r1       r0            1772
--       --            ----
0        0             6000 cycles (?)
0        1             12000 cycles
1        0             2000 cycles
1        1             3000 cycles
*/
      switch(cr&(CR_R1|CR_R0)) {
      case 0:
        update_time=MsToCycles(6);
        break;
      case 1:
        update_time=MsToCycles(12);
        break;
      case 2:
        update_time=MsToCycles(2);
        break;
      case 3:
        update_time=MsToCycles(3);
        break;
      }//sw
      update_time+=current_time;
      prg_phase=WD_TYPEI_STEP_PULSE;
    }
    break;

  case WD_TYPEI_STEP_PULSE:
    // goto 'D' if command is step, 'A' otherwise
    prg_phase=(cr&CR_STEP)?WD_TYPEI_CHECK_VERIFY:WD_TYPEI_SEEK;
    OnUpdate(); // some recursion is always cool
    break;

  case WD_TYPEI_CHECK_VERIFY: // 'D'
    // update str bit 2 (reflects status of the TR00 signal)
    if(Lines.track0)
      str|=FDC_STR_T0;
    else
      str&=~FDC_STR_T0;

    if(cr&CR_V)
    {
      if(FloppyDrive[drive].ImageType.Manager==MNGR_WD1772
        && FloppyDrive[drive].m_DiskInDrive
#if defined(SSE_MEGASTE) && defined(SSE_DISK_HD) // assume DD/HD mismatch times out
        &&(!(IS_MEGASTE
          &&((FloppyDisk[drive].Density==2)^((MegaSte.FdHd&BIT_1)==BIT_1))))
#endif    
         )
      {
        FloppyDrive[drive].MfmManager->LoadTrack(CURRENT_SIDE,CURRENT_TRACK);
        prg_phase=WD_TYPEI_HEAD_SETTLE; 
        update_time=current_time + MsToCycles(15);
      }
      else if(!ADAT) // like Steem native, not really correct
      {              // but we come fast to GEM
        str|=FDC_STR_SE;
        Irq(true);
      }
    }
    else
      Irq(true); // this updates status bits
    break;

  case WD_TYPEI_HEAD_SETTLE:
    // flow chart is missing head settling
    prg_phase=WD_TYPEI_FIND_ID;
    Amd.Reset();
    n_format_bytes=0;
    Read(); // drive will send word (clock, byte) and set event
    IndexCounter=6; 
//    TRACE_WD("%d IP to find ID %d\n",IndexCounter,sr);
    break;

  case WD_TYPEI_FIND_ID:
  case WD_TYPEII_FIND_ID:
  case WD_TYPEIII_FIND_ID:
    CrcLogic.Add(dsr);
#if defined(SSE_WD1772_LL)
    // wait for AM
    if(Amd.aminfo & CAPSFDC_AI_DSRAM)
    {
      //ASSERT(IMAGE_SCP);
      // AM detected, read returns on dsr ready
      Amd.amisigmask=CAPSFDC_AI_DSRREADY;
      Amd.nA1=3;
      CrcLogic.Reset(); 
      Amd.Enabled=false; // read IDs OK
      //TRACE_WD("Phase %d AM at %d\n",prg_phase,FloppyDisk[drive].current_byte);
    }
    else
#endif
      if(Amd.Enabled && (dsr==0xA1 && !(Mfm.clock&BIT_5)
#if defined(SSE_WD1772_LL) 
      || (Amd.aminfo&CAPSFDC_AI_DSRMA1)
#endif
      ))
    {
      //ASSERT(Amd.Enabled);
      Amd.nA1++;
      CrcLogic.Reset(); // only special $A1 resets the CRC logic
#if defined(SSE_WD1772_LL)
      Amd.amisigmask=CAPSFDC_AI_DSRREADY;
#endif
//      TRACE_WD("Phase %d A1 at %d\n",prg_phase,FloppyDisk[drive].current_byte);
    }
    // note: it is strictly 3 A1 syncs
    else if( (dsr&0xFF)>=0xFC && Amd.nA1==3) // CAPS: $FC->$FF
    {
      TRACE_WD("%X found at %d\n",dsr,FloppyDisk[drive].current_byte);
      n_format_bytes=0; //reset
      prg_phase++; // in type I or type II or III
      Amd.Enabled=false; // read IDs OK
    }
    else if(Amd.nA1)
      Amd.Reset(); //?
    Read(); // this sets up next event
    break;

  case WD_TYPEI_READ_ID:
  case WD_TYPEII_READ_ID:
  case WD_TYPEIII_READ_ID:
    // fill in ID field
    *(((BYTE*)&IDField)+n_format_bytes)=dsr; //no padding!!!!!!
    if(n_format_bytes<4)
      CrcLogic.Add(dsr);
    if(prg_phase==WD_TYPEIII_READ_ID)
    {
      dr=dsr;
      Drq(true); // read address
    }
    n_format_bytes++;
    if(n_format_bytes==sizeof(TWD1772IDField))
    {
      n_format_bytes=0; 
      prg_phase++; // in type I, II, III
#ifdef SSE_DEBUG
      TRACE_WD("At %d:",FloppyDisk[drive].current_byte); // position
      IDField.Trace();
#endif
      OnUpdate(); // some recursion is always cool
    }
    else
      Read();
    break;

  case WD_TYPEI_TEST_ID:
#ifdef SSE_DEBUG____
    TRACE_WD("At %d:",FloppyDisk[drive].current_byte); // position
    IDField.Trace();
#endif
    //test track and CRC
    if(IDField.track==tr && CrcLogic.Check(&IDField))
    {
      CrcLogic.Reset();
      Irq(true); // verify OK
    }
    else // they should all have correct track, will probably time out
    {
      prg_phase=WD_TYPEI_FIND_ID;
#if defined(SSE_DISK_SCP)
      check_scp_rev(drive);
#endif
      if(IDField.track==tr)
        str|=FDC_STR_CRC; // set CRC error if track field was OK
      CrcLogic.Add(dsr); //unimportant
      Amd.Enabled=true; 
      Read(); // this sets up next event
    }
    break;

  case WD_TYPEII_SPUNUP:
  case WD_TYPEIII_SPUNUP:
    prg_phase++;
//    TRACE("cr&CR_E %d\n",cr&CR_E);
    if(cr&CR_E) // head settle delay programmed
      update_time=current_time + MsToCycles(15);
    else
      OnUpdate(); // some recursion is always cool
    break;

  case WD_TYPEII_HEAD_SETTLE: // we come directly or after 15ms delay
    // check Write Protect for command write sector
    // note: Lines.write_protect is undefined!
    if((cr&CR_TYPEII_WRITE) && (FloppyDisk[drive].ReadOnly) ) 
    {
      str|=FDC_STR_WP;
      Irq(true);
    }
    else // read, or write OK
    {
      IndexCounter=5; 
      //TRACE_WD("%d IP to find ID %d\n",IndexCounter,sr);
      prg_phase=WD_TYPEII_FIND_ID; // goto '1'
      Amd.Reset();
      n_format_bytes=0;
      Read();
    }
    break;

  case WD_TYPEII_TEST_ID:
    //ASSERT(!n_format_bytes);
#ifdef SSE_DEBUG___
    TRACE_WD("At %d:",FloppyDisk[drive].current_byte); // position
    IDField.Trace();
#endif
    if(IDField.track==tr && IDField.num==sr)
    {
      ByteCount=IDField.nBytes();
      if(CrcLogic.Check(&IDField))
      {
        CrcLogic.Reset();
        prg_phase=(cr&CR_TYPEII_WRITE) ? WD_TYPEII_WRITE_DAM :
          WD_TYPEII_FIND_DAM;
      }
      else
      {
        str|=FDC_STR_CRC;
        CrcLogic.Add(dsr);
        prg_phase=WD_TYPEII_FIND_ID; 
      }
    }
    else // it's no error (yet), the Fdc must browse the IDs
    {
      prg_phase=WD_TYPEII_FIND_ID;
#if defined(SSE_DISK_SCP)
      check_scp_rev(drive);
#endif
    }
    Amd.Reset();
    Read();
    break;

  case WD_TYPEII_FIND_DAM:
    CrcLogic.Add(dsr); //before eventual reset
    n_format_bytes++;
    if(n_format_bytes<27)
      ; // CAPS: first bytes aren't even read
    else if(n_format_bytes==27)
    {
      TRACE_MFM("Enable AMD\n");
      Amd.Enable();
#if defined(SSE_WD1772_LL)
      Amd.amisigmask=CAPSFDC_AI_DSRREADY|CAPSFDC_AI_DSRMA1;
#endif
    }
    else if(n_format_bytes==44+ Amd.nA1) //timed out
    {
      TRACE_WD("DAM time out %d in at %d\n",n_format_bytes,FloppyDisk[drive].current_byte);
      n_format_bytes=0;
      prg_phase=WD_TYPEII_FIND_ID;
      Amd.Enable();
    }
#if defined(SSE_WD1772_LL)
    //if not A1 mark, restart
    else if((Amd.aminfo & CAPSFDC_AI_DSRAM) && !(Amd.aminfo & CAPSFDC_AI_DSRMA1))
    {
      //ASSERT(IMAGE_SCP);
      Amd.Enable();
      n_format_bytes=0;
    }
    // wait for AM
    else if ((Amd.aminfo & CAPSFDC_AI_DSRAM))
    {
      TRACE_WD("AM found at byte %d (%d in), reset CRC\n",dsr,FloppyDisk[drive].current_byte,n_format_bytes);
      // AM detected, read returns on dsr ready
      Amd.amisigmask=CAPSFDC_AI_DSRREADY;
      CrcLogic.Reset();
      Amd.nA1=3;
    }
#endif
    else if(dsr==0xA1 && !(Mfm.clock&BIT_5) //stw
#if defined(SSE_WD1772_LL)
      || (Amd.aminfo&CAPSFDC_AI_DSRMA1)
#endif
      ) 
    {
      TRACE_WD("%X found at byte %d, reset CRC\n",dsr,FloppyDisk[drive].current_byte);
      CrcLogic.Reset();
      Amd.nA1++; 
    }
    else if(Amd.nA1==3 && ((dsr&0xFE)==0xF8||(dsr&0xFE)==0xFA)) // DAM found
    {
      TRACE_WD("TR%d SR%d %X found at byte %d (%d after ID)\n",tr,sr,dsr,FloppyDisk[drive].current_byte,n_format_bytes);
      n_format_bytes=0; // for CRC later
      Amd.Enabled=false;
      prg_phase=WD_TYPEII_READ_DATA;
      if((dsr&0xFE)==0xF8)
        str|=FDC_STR_RT; // "record type" set when "deleted data" DAM
    }
    else if(Amd.nA1==3) // address mark but then no FB...
    {
      TRACE_WD("%x found after AM: keep looking\n",dsr);
      Amd.Enable();
    }
    Read();    
    break;

  case WD_TYPEII_READ_DATA:
    CrcLogic.Add(dsr);
    dr=dsr;
    Drq(true); // DMA never fails to take the byte
    ByteCount--;
    //ASSERT(!n_format_bytes);
    if(!ByteCount)
      prg_phase=WD_TYPEII_READ_CRC;
    Read();
    break;

  case WD_TYPEII_READ_CRC:
    IDField.CRC[n_format_bytes]=dsr; // and we don't add to CRC
    if(n_format_bytes) //1
    {
      if(!CrcLogic.Check(&IDField))
      {
        TRACE_WD("Read sector %c:%d-%d-%d CRC error\n",'A'+drive,CURRENT_SIDE,IDField.track,IDField.num);
        str|=FDC_STR_CRC; // caught by Dma.UpdateRegs() for OSD
        Irq(true);
      }
      else
      {
        TRACE_WD("Read sector %c:%d-%d-%d OK CRC %X%X\n",'A'+drive,CURRENT_SIDE,IDField.track,IDField.num,IDField.CRC[0],IDField.CRC[1]);
        prg_phase=WD_TYPEII_CHECK_MULTIPLE;
        OnUpdate(); // some recursion is always cool
      }
    }
    else
    {
      n_format_bytes++;
      Read(); // next CRC byte
    }
    break;

  case WD_TYPEII_CHECK_MULTIPLE:
#if defined(SSE_DISK_SCP)
    check_scp_rev(drive);
#endif
    if(cr&CR_M)
    {
      FloppyDrive[drive].writing=0;
      sr++;
      if(!sr)
        sr++; // CAPS: 255 wraps to 1 (due to add with carry)
      prg_phase=WD_TYPEII_HEAD_SETTLE; // goto '4'
      OnUpdate();
    }
    else
      Irq(true);
    break;

  case WD_TYPEII_WRITE_DAM:
    n_format_bytes++;
    if(n_format_bytes<23-1) //22 or 23?
      Read();    
    else if(n_format_bytes<23-1+12) // write 12 $0
    {
      Lines.write_gate=1; // those lines don't matter for now
      Lines.write=1;
      Mfm.data=0;
      Mfm.Encode(); 
      //TRACE_FDC("write %X at byte %d\n",Mfm.data,FloppyDisk[drive].current_byte);
      CrcLogic.Add(Mfm.data); // shouldn't matter
      Write();
    }
    else if(n_format_bytes<23-1+12+3) // write 3x $A1 (missing in flow chart)
    {
      Mfm.data=0xA1;
      Mfm.Encode(TWD1772MFM::FORMAT_CLOCK); 
      //TRACE_FDC("write %X at byte %d, reset CRC\n",Mfm.data,FloppyDisk[drive].current_byte);
      CrcLogic.Add(Mfm.data); // before reset   
      CrcLogic.Reset();
      Write();
    }
    else if(n_format_bytes==23-1+12+3) // write DAM acording to A0 field
    {
      //ASSERT(!(cr&CR_A0)); //Amateur versions (>1.50?) of ProCopy use $A1 to copy
      Mfm.data= (cr&CR_A0) ? 0xF9 : 0xFB;
      Mfm.Encode(); 
      TRACE_WD("tr %d sr %d write %X at byte %d, %d in\n",tr,sr,Mfm.data,FloppyDisk[drive].current_byte,n_format_bytes);
      CrcLogic.Add(Mfm.data); // after eventual reset (TODO)        
      Write();     
    }
    else
    {
      n_format_bytes=0;
      prg_phase=WD_TYPEII_WRITE_DATA;
      OnUpdate(); // some recursion is always cool  
    }
    break;

  case WD_TYPEII_WRITE_DATA:
    Drq(true); // normally first DRQ happened much earlier, we simplify
    dsr=dr;
    CrcLogic.Add(dsr);
    Mfm.data=dsr;
    Mfm.Encode();
    ByteCount--;
    //ASSERT(!n_format_bytes);
    if(!ByteCount)
    {
      prg_phase=WD_TYPEII_WRITE_CRC;
      TRACE_WD("CRC: %X\n",CrcLogic.crc);
    }
    Write();
    break;

  case WD_TYPEII_WRITE_CRC: // CRC + final $FF (?)
    n_format_bytes++;
    if(n_format_bytes==1)
      Mfm.data=CrcLogic.crc>>8;
    else if(n_format_bytes==2)
      Mfm.data=(CrcLogic.crc&0xFF);
    else
    {
      n_format_bytes=0;
      prg_phase=WD_TYPEII_CHECK_MULTIPLE;
      Mfm.data=0xFF;
      Lines.write_gate=0; //early
      Lines.write=0; // early
    }
    Mfm.Encode();
    Write();
    break;

  case WD_TYPEIII_HEAD_SETTLE: // we come directly or after 15ms delay
    Amd.Reset();
#if defined(SSE_WD1772_LL)
    Amd.aminfo&=~CAPSFDC_AI_CRCENABLE;
    Amd.amisigmask=CAPSFDC_AI_DSRREADY;
#endif

    if((cr&0xF0)==CR_TYPEIII_READ_ADDRESS)
    {
      IndexCounter=5; //not documented, see OnIndexPulse()
      prg_phase=WD_TYPEIII_FIND_ID;
      n_format_bytes=0;
      Read();
    }
    // check Write Protect for command write track
     // Lines.write_protect is undefined!
    else if((cr&CR_TYPEIII_WRITE) && (FloppyDisk[drive].ReadOnly) )
    {
      TRACE_WD("Can't write on disk\n");
      str|=FDC_STR_WP;
      Irq(true);
    }
    else // for read & write track, we start at next IP
    {
      IndexCounter=1;
  //    TRACE_WD("%d IP for read or write track\n",IndexCounter);
      prg_phase=WD_TYPEIII_IP_START;
      //ASSERT( FloppyDrive[drive].motor );
    }
    break;

  case WD_TYPEIII_TEST_ID:
    if(!CrcLogic.Check(&IDField))
      str|=FDC_STR_CRC;
    sr=IDField.track;
    Irq(true);
    break;

  case WD_TYPEIII_READ_DATA: 

    // "the Address Mark Detector is on for the duration of the command"
    if(dsr==0xA1 && !(Mfm.clock&BIT_5)
#if defined(SSE_WD1772_LL)
      || (Amd.aminfo&CAPSFDC_AI_DSRMA1)
#endif
      )
    {
#if defined(SSE_DISK_SCP) // don't need this hack with the SCP version
      if(!IMAGE_SCP)// CAPS-like code produces the $14, it's quite intricate
#endif
      if(CrcLogic.crc!=0xCDB4)
        dsr=0x14;
      CrcLogic.Reset();
    }
    else
      CrcLogic.Add(dsr); 
   
    dr=dsr;
    Drq(true);
    Read();
    break;

  case WD_TYPEIII_WRITE_DATA:  
    // The most interesting part of STW support, and novelty in ST emulation!
    Drq(true);
    if((dr&0xFE)==0xFE
#if defined(SSE_DEBUGGER_TRACE_CONTROL) 
      && !(TRACE_MASK3&(TRACE_CONTROL_FDCMFM|TRACE_CONTROL_FDCBYTES))
#endif
      )
      n_format_bytes=4; // so we'll trace all written IDs
    // analyse byte in for MFM markers
    if(dr==0xF5 && !F7_escaping) //Write A1 in MFM with missing clock Init CRC
    {
      Mfm.data=dsr=0xA1;
      Mfm.Encode(TWD1772MFM::FORMAT_CLOCK);
      CrcLogic.Reset();
      Write();
      if(n_format_bytes)
      {
        TRACE_WD("$%x-",dsr);
        n_format_bytes--;
      }
    }
    else if(dr==0xF6 && !F7_escaping) //Write C2 in MFM with missing clock
    {
      Mfm.data=dsr=0xC2;
      Mfm.Encode(TWD1772MFM::FORMAT_CLOCK);
      CrcLogic.Add(Mfm.data);
      Write();
      if(n_format_bytes)
      {
        TRACE_WD("$%x-",dsr);
        n_format_bytes--;
      }
    }
/*  The format code $F7 may be used inside an ID field. The CRC bytes are added
    to the CRC, so that this is correct. This implies that at the receipt of
    $F7, the WD1772 saves the current value of the CRC (at least the lower 
    byte), before it is modified by the upper byte.
    A byte following F7 isn't interpreted as a format byte.*/
    else if(dr==0xF7 && !F7_escaping) //Write 2 CRC Bytes
    {
      Mfm.data=dsr=CrcLogic.crc>>8; // write 1st byte
      dr=CrcLogic.crc&0xFF; // save 2nd byte
      CrcLogic.Add(Mfm.data);
      Mfm.Encode();
      F7_escaping=true;
      Write();
      if(n_format_bytes)
      {
        TRACE_WD("%d-",dsr);
        n_format_bytes--;
      }
      prg_phase=WD_TYPEIII_WRITE_DATA2; // for 2nd byte
    }
    else // other bytes ($0, $E5...)
    {
      Mfm.data=dsr=dr;
      Mfm.Encode();
      CrcLogic.Add(dsr);
      F7_escaping=false;
      Write();
      if(n_format_bytes&&dr!=0xFE)
      {
#ifdef DEBUG_BUILD
        TRACE_WD("%d ",dsr);
        if(n_format_bytes==1)
          TRACE_WD("\n");
#endif
        n_format_bytes--;
      }
    }
    break;

  case WD_TYPEIII_WRITE_DATA2:
    // write 2nd byte of CRC
    Mfm.data=dsr=dr; // as saved
    CrcLogic.Add(Mfm.data);
    Mfm.Encode();
    Write(); 
    if(n_format_bytes)
    {
      TRACE_WD("%d-",dsr);
      n_format_bytes--;
    }
    prg_phase=WD_TYPEIII_WRITE_DATA; // go back
    break;

  default:
    update_time=current_time+n_cpu_cycles_per_second; 
   break;
  }//sw
  //ASSERT(prg_phase!=WD_TYPEII_HEAD_SETTLE);
  //prepare_next_event();  
}


void TWD1772::Read() {
  if(Psg.CurrentDrive()!=TYM2149::NO_VALID_DRIVE)
  {
    FloppyDrive[DRIVE].Read(); // this gets data and creates event
    Mfm.Decode();
    if(!IMAGE_SCP) // dsr shouldn't be messed with...
      dsr=Mfm.data;
#ifdef SSE_DEBUG // for SCP it's not perfectly aligned
    TRACE_MFM("%s #%d MFM %04X c $%02X d $%02X\n",wd_phase_name[prg_phase],FloppyDisk[DRIVE].current_byte,Mfm.encoded,Mfm.clock,dsr);
#endif
  }
  else  
    update_time=current_time+n_cpu_cycles_per_second;
}


void TWD1772::StepPulse() {
/*
  // useless now, normally it lasts some us, but we're not going to
  // set up events for that
  Lines.step=true; 
  Lines.step=false;
*/
  if(Psg.CurrentDrive()!=TYM2149::NO_VALID_DRIVE)
    FloppyDrive[Psg.SelectedDrive].Step(Lines.direction);
}


void TWD1772::Write() {
/*  Data must be MFM-encoded before.
    We don't do it here because we don't know if we code for special
    format bytes or not.
*/
#ifdef SSE_DEBUG
  TRACE_MFM("%s #%d MFM %04X c $%02X d $%02X\n",wd_phase_name[prg_phase],FloppyDisk[DRIVE].current_byte,Mfm.encoded,Mfm.clock,Mfm.data);
  //ASSERT(Psg.CurrentDrive()!=TYM2149::NO_VALID_DRIVE);
#endif
  if(Psg.CurrentDrive()!=TYM2149::NO_VALID_DRIVE)
    FloppyDrive[DRIVE].Write(); // this writes data and creates event  
  else  
    update_time=current_time+n_cpu_cycles_per_second;
}


// TWD1772IO -> WriteCR when manager=MNGR_WD1772


void  TWD1772::WriteCR(BYTE io_src_b) {
  //ASSERT(IMAGE_STW||IMAGE_SCP||IMAGE_HFE);
  //ASSERT( FloppyDrive[DRIVE].ImageType.Manager==MNGR_WD1772 );
  if(CommandType(io_src_b)==2 || CommandType(io_src_b)==3) //or no condition?
  {
    if(FloppyDrive[DRIVE].ImageType.Manager==MNGR_WD1772
      && FloppyDrive[DRIVE].m_DiskInDrive)
      FloppyDrive[DRIVE].MfmManager->LoadTrack(CURRENT_SIDE,CURRENT_TRACK);
  }

/*  cr will accept a new command when busy bit is clear or when command
    is 'Force Interrupt'.
    Suska: 'Force Interrupt' isn't accepted if busy is clear. $FF is ignored.
    Not documented (from Hatari?): also when the drive is still spinning up
*/

  if(!(str&FDC_STR_BSY)||(io_src_b&0xF0)==0xD0||fdc_spinning_up)
  {
   //agenda_delete(agenda_fdc_motor_flag_off); // and others? // what is it doing here?
// = circle around wrong motor off agenda if drive empty in Fdc
    NewCommand(io_src_b); // one more function, more readable
  }
  else
  {
    TRACE_WD("FDC command %X ignored\n",io_src_b);
    //cr=io_src_b;//should...
  }
}


#if defined(SSE_WD1772_LL)

/*  This is the correct algorithm for the WD1772 DPLL (digital phase-locked 
    loop) system, as described in patent US 4808884 A.
    It allows us to read low-level (flux level) disk images such as SCP.
    Thx to Olivier Galibert for some inspiration, otherwise the code
    would be a real MESS.
*/

int TWD1772Dpll::GetNextBit(COUNTER_VAR &tm, BYTE drive) {

  int aa=0;

  WORD timing_in_us;

  while(ctime-latest_transition>=0)
  {
#if defined(SSE_DISK_SCP) // add formats here ;)
    aa=ImageSCP[drive].GetNextTransition(timing_in_us); // cycles to next 1
#endif
    TRACE_MFM("(%d)",timing_in_us);
    if(OPTION_HACKS && timing_in_us>12) // Powerdrome 0-79-10
    {
      short rnd=(rand()%5)+1-3; // -2 +2us
      TRACE_MFM("(NFA%d)",rnd);
      aa+=rnd*8;
    }
    latest_transition+=aa;
  }
  COUNTER_VAR when=latest_transition;
  //ASSERT(!(when==-1 || when-ctime<0));

  for(;;) {
    COUNTER_VAR etime = ctime+delays[slot];

    if(transition_time == 0xffff && etime-when >= 0)
      transition_time = counter;

    if(slot < 8) { //SS I don't understand this, why only <8?
      BYTE mask = 1 << slot;
      if(phase_add & mask)
        counter += 226;
      else if(phase_sub & mask)
        counter += 30;
      else
        counter += increment;

      if((freq_add & mask) && increment < 140)
        increment++;
      else if((freq_sub & mask) && increment > 117)
        increment--;
    } else
      counter += increment;

    slot++;
    tm = etime;
    if(counter & 0x800)
      break;
  }

  int bit = transition_time != 0xffff;
  
  if(bit) {
    static const BYTE pha[8] = { 0xf, 0x7, 0x3, 0x1, 0, 0, 0, 0 };
    static const BYTE phs[8] = { 0, 0, 0, 0, 0x1, 0x3, 0x7, 0xf };
    static const BYTE freqa[4][8] = {
      { 0xf, 0x7, 0x3, 0x1, 0, 0, 0, 0 },
      { 0x7, 0x3, 0x1, 0, 0, 0, 0, 0 },
      { 0x7, 0x3, 0x1, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0 }
    };
    static const BYTE freqs[4][8] = {
      { 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0x1, 0x3, 0x7 },
      { 0, 0, 0, 0, 0, 0x1, 0x3, 0x7 },
      { 0, 0, 0, 0, 0x1, 0x3, 0x7, 0xf },
    };

    int cslot = transition_time >> 8;
    //ASSERT( cslot<8 );
    phase_add = pha[cslot];
    phase_sub = phs[cslot];
    int way = transition_time & 0x400 ? 1 : 0;
    if(history & 0x80)
      history = way ? 0x80 : 0x83;
    else if(history & 0x40)
      history = way ? history & 2 : (history & 2) | 1;
    freq_add = freqa[history & 3][cslot];
    freq_sub = freqs[history & 3][cslot];
    history = way ? (history >> 1) | 2 : history >> 1;
  } else
    phase_add = phase_sub = freq_add = freq_sub = 0;

  counter &= 0x7ff;
  ctime = tm;
  transition_time = 0xffff;
  slot = 0;
  //ASSERT( bit==0 || bit==1 );
  return bit;
}


void TWD1772Dpll::Reset(COUNTER_VAR when) {
  counter = 0;
  increment = 128;
  transition_time = 0xffff;
  history = 0x80;
  slot = 0;
  latest_transition= ctime = when;
  phase_add = 0x00;
  phase_sub = 0x00;
  freq_add  = 0x00;
  freq_sub  = 0x00;
  write_position = 0;
  write_start_time = -1;
  SetClock(1); // clock WD1772 = clock CPU, 16 cycles = 2 microseconds
}


void TWD1772Dpll::SetClock(const int &period)
{
  for(int i=0; i<42; i++)
    delays[i] = period*(i+1);
}


/*  This is the correct algorithm for the WD1772 data separator.
    It interprets the bit flow from disk images such as SCP, coming
    from the DPLL.
    
    Fluxes -> DPLL -> data separator -> dsr

    Thx to Istvan Fabian for some inspiration otherwise Steem would have lower
    CAPS to read disk images (like those that use the $C2 sync mark).
    Note: my comments in this function marked by SS
*/

bool TWD1772::ShiftBit(int bit) {

  bool byte_ready=false;

  // shift read bit into am decoder
  DWORD amdecode=Amd.amdecode<<1;
  if (bit)
    amdecode|=1;
  Amd.amdecode=amdecode;

  // get am info, clear AM found, A1 and C2 mark signals
  DWORD aminfo=Amd.aminfo & ~(CAPSFDC_AI_AMFOUND|CAPSFDC_AI_MARKA1|CAPSFDC_AI_MARKC2);

  // bitcell distance of last mark detected
  if (Amd.ammarkdist)
    Amd.ammarkdist--;

  // am detector if enabled
  if(Amd.Enabled) {
    //if (aminfo & CAPSFDC_AI_AMDETENABLE) { //SS TODO
    //ASSERT(aminfo & CAPSFDC_AI_AMDETENABLE); //SS asserts...
    // not a mark in shifter/decoder
    BYTE amt=0;

    // check if shifter/decoder has a mark
    // the real hardware probably has two shifters (clocked and fed by data separator) connected to a decoder
    // each bit of the shifter/decoder is for shifter0#0, shifter1#0...shifter0#7, shifter1#7, 
    // only 2 comparisons is needed per cell instead of 8 (A1/0A, C2/14 and 0A/A1, 14/C2)
    switch (amdecode  & 0xffff) {
      // A1 mark only enabled if not overlapped with another A1
      case 0x4489:
        if (!Amd.ammarkdist || Amd.ammarktype!=1)
          amt=1;
        break;

        // C2 mark always enabled
      case 0x5224:
        amt=2;
        break;
    }

    // process mark if found
    if (amt) {
      // we just read the last data bit of a mark, delay by a clock bit to read from decoder#1
      Amd.amdatadelay=1;

      // if overlapped with a different mark
      if (Amd.ammarkdist && Amd.ammarktype!=amt) {
        // dsr value is invalid
        Amd.amdataskip++;

        // delay by an additional data bit (data, clock)
        Amd.amdatadelay+=2;
      }

      // if dsr is empty dsr shouldn't be flushed, dsr value is invalid
      if (!Amd.dsrcnt)
        Amd.amdataskip++;

      // force dsr to flush its current value, since data values start from next data bit
      Amd.dsrcnt=7;

      // 16 bitcells must be read before next mark, otherwise marks are overlapped (8 clock+data bits)
      Amd.ammarkdist=16;

      // save last mark type; only used when marks overlap
      Amd.ammarktype=amt;

      // set mark signal, the shifter/decoder must be connected to the crc logic, not dsr
      if (amt == 1) {
        aminfo|=CAPSFDC_AI_MARKA1|CAPSFDC_AI_MA1ACTIVE;


        // if CRC is enabled, first A1 mark activates the CRC logic
        if (aminfo & CAPSFDC_AI_CRCENABLE) {
          // if CRC logic is not activate yet reset CRC value and counter, 16 cells already available as mark
          if (!(aminfo & CAPSFDC_AI_CRCACTIVE)) {
            aminfo|=CAPSFDC_AI_CRCACTIVE;
            //pc->crc=~0; //SS keep ours
            CrcLogic.crccnt=16; 
          }
        }

      } else
        aminfo|=CAPSFDC_AI_MARKC2;
      TRACE_MFM(" mark %X ",(amdecode  & 0xffff));
    }
  }


  // process CRC if activated
  if (aminfo & CAPSFDC_AI_CRCACTIVE) {
    // process new value at every 16 cells (8 clock/data)
    if (!(CrcLogic.crccnt & 0xf)) { 
      // reset CRC process if less than 3 consecutive A1 marks detected
      if (CrcLogic.crccnt>48 || (aminfo & CAPSFDC_AI_MARKA1)) {
        // 3 consecutive A1 marks found: set AM found signal, disable AM detector
        if (CrcLogic.crccnt == 48) {
          aminfo|=CAPSFDC_AI_AMFOUND|CAPSFDC_AI_AMACTIVE;
          aminfo&=~CAPSFDC_AI_AMDETENABLE;
        }
      } else
        aminfo&=~(CAPSFDC_AI_CRCACTIVE|CAPSFDC_AI_AMACTIVE);
    }

    CrcLogic.crccnt++;
  }



  // wait for data clock cycle plus bitcell delay
  if (!Amd.amdatadelay) {
    // set next delay
    // just read a clock bit here, next cell is data, that gets processed at the clock bit after that
    Amd.amdatadelay=1;

    // clear dsr signals
    aminfo&=~(CAPSFDC_AI_DSRREADY|CAPSFDC_AI_DSRAM|CAPSFDC_AI_DSRMA1);

    // shift data bit into dsr, this is a clock bit, data bit is at decoder#1
    Amd.dsr=( ((Amd.dsr<<1) | ((amdecode>>1) & 1)) ) & 0xff;

    // process data if 8 bits are dsr now, otherwise just increase dsr counter
    if (++Amd.dsrcnt == 8) {
      // reset dsr counter
      Amd.dsrcnt=0;

      // if AM found set dsr signal
      if (aminfo & CAPSFDC_AI_AMACTIVE) {
        TRACE_MFM(" -AM- ");
        aminfo&=~CAPSFDC_AI_AMACTIVE;
        aminfo|=CAPSFDC_AI_DSRAM;
      }

      // if A1 mark found set dsr signal
      if (aminfo & CAPSFDC_AI_MA1ACTIVE) {
        aminfo&=~CAPSFDC_AI_MA1ACTIVE;
        aminfo|=CAPSFDC_AI_DSRMA1;
      }

      // set dsr ready signal, unless data is invalid
      if (!Amd.amdataskip)
      {
        aminfo|=CAPSFDC_AI_DSRREADY;
        dsr=(Amd.dsr&0xFF);
      }
      else
        Amd.amdataskip--;
    }
  } else
  {
    Amd.amdatadelay--;
#ifdef SSE_DEBUG
    Mfm.encoded=(WORD)Amd.amdecode; //wrong byte/clock order for 1st $A1
#endif
  }

  // save new am info
  Amd.aminfo=aminfo;

  // if a byte is complete, break and signal new byte
  if (Amd.aminfo & CAPSFDC_AI_DSRREADY) {
    byte_ready=true;
  }
  return byte_ready;
}

#endif
