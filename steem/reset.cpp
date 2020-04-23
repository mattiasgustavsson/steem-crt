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
FILE: reset.cpp
DESCRIPTION: Functions to reset the emulator to a startup state.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <computer.h>
#include <osd.h>
#include <draw.h>
#include <interface_stvl.h>
#include <gui.h>
#include <palette.h>
#include <harddiskman.h>
#include <loadsave.h>

/*
Atari ST Boot Up Operation
The Motorola 68000 on boot up requires initial
values to load into its supervisor stack pointer
and reset vector address. These come in the
form of two long words at address 0x000000 to
0x000007.

Boot up sequence
Figure 9 - Atari ST boot up sequence
(1)
. Load SSP with long word value from 0xFC0000.
. Load PC with long word value from 0xFC0004 (Garbage value, memory not yet
sized).
. CPU Supervisor Mode Interrupts disabled (IPL=7).
. RESET instruction to reset all peripheral chips.
. Check for magic number 0xFA52235F on cartridge port, if present jump to
diagnostic cartridge.
(2).
. Test for warm start, if memvalid (0x000420) and memval2 (0x00043A) contain
the Magic numbers 0x7520191F3 and 0x237698AA respectively, then load the
memconf (0xFF8001) contents with data from memctrl (0x000424).
(3)
. If the resvalid (0x000426) contains the Magic number 0x31415926, jump to reset
vector taken from Resvector (0x00042A).
(4)
. YM2149 sound chip initialized (Floppy deselected).
. The vertical synchronization frequency in syncmode (0xFF820A) is adjusted to
50Hz or 60Hz depending on region.
. Shifter palette initialized.
. Shifter Base register (0xFF8201 and 0xFF8203) are initialized to 0x010000.
. The following steps 5 to 8 are only done on a coldstart to initialize memory.
(5)
. Write 0x000a (2 Mbyte & 2 Mbyte) to the MMU Memory Configuration Register
0xff8001).
(6)
. Write Pattern to 0x000000 - 0x000lff.
. Read Pattern from 0x000200 - 0x0003ff.
. If Match then Bank0 contains 128 Kbyte; goto step 7.
. Read Pattern from 0x000400 - 0x0005ff.
. If Match then Bank0 contains 512 Kbyte; goto step 7.
. Read Pattern from 0x000000 - 0x0001ff.
. If Match then Bank0 contains 2 Mbyte; goto step 7.
. panic: RAM error in Bank0.
(7)
. Write Pattern to 0x200000 - 0x200lff.
. Read Pattern from 0x200200 - 0x2003ff.
. If Match then Bank1 contains 128 Kbyte; goto step 8.
. Read Pattern from 0x200400 - 0x2005ff.
. If Match then Bank1 contains 512 Kbyte; goto step 8.
. Read Pattern from 0x200000 - 0x2001ff.
. If Match then Bank1 contains 2 Mbyte; goto step 8.
. note: Bank1 not fitted.
(8)
. Write Configuration to MMU Memory Configuration Register (0xff8001).
. Note Total Memory Size (Top of RAM) for future reference in phystop
(0x00042E).
. Set magic values in memvalid (0x000420) and memval2 (0x00043A).
(9)
. Clear the first 64 Kbytes of RAM from top of operating system variables
(0x00093A) to Shifter base address (0x010000).
. Initialize operating system variables.
. Change and locate Shifter Base register to 32768 bytes from top of physical ram.
. Initialize interrupt CPU vector table.
. Initialize BIOS.
. Initialize MFP.
(10)
. Cartridge port checked, if software with bit 2 set in CA_INIT then start.
(11)
. Identify type of monitor attached for mode of operation for the Shifter video chip
and initialize.
(12)
. Cartridge port checked, if software with CA_INIT clear (execute prior to display
memory and interrupt vector initialization) then start.
(13)
. CPU Interrupt level (IPL) lowered to 3 (HBlank interrupts remain masked).
(14)
. Cartridge port checked, if software with bit 1 set in CA_INIT (Execute prior to
GEMDOS initialization) then start.
(15)
. The GEMDOS Initialization routines are completed.
(16)
. Attempt boot from floppy disk if operating system variable _bootdev (0x000446)
smaller than 2 (for floppy disks) is. Before a boot attempt is made bit 3 in
CA_INIT (Execute prior to boot disk) checked, if set, start cartridge.
. The ACSI Bus is examined for devices, if successful search and load boot sector.
. If system variable _cmdload (0x000482) is 0x0000, skip step 17.
(17)
. Turn screen cursor on
. Start any program in AUTO folder of boot device
. Start COMMAND.PRG for a shell
(18)
. Start any program in AUTO folder of boot device
. AES (in the ROM) starts.

It is important to have these two different reset signals, as some parts of the design only
need to be reset on power up to known states. One of these components was the clock
signal component. It was important for the CPU that the clock was running while a reset
is issued, and that the reset was active for at least 132 clock cycles [27].
*/

/*
  1. power_on() is called by initialise() at start, if we don't restore
  a state snaphsot, reset_st() isn't called
  2. power_on() is called by reset_st() when player rights click on reset
  or when restoring a state snapshot (cold reset)
*/

void power_on() {
  TRACE_INIT("power_on\n");
  if(STMem)
    ZeroMemory(STMem+MEM_EXTRA_BYTES,mem_len);
  on_rte=ON_RTE_RTE;
  SET_PC(rom_addr);
  //we updated a7 but then cleared it here, fixes TOS1.0 4MB
  //for (int n=0;n<16;n++) r[n]=0; //registers not reset!
  other_sp=0;
  ioaccess=0;
  //vbase=mem_len-0x8000;
  if(extended_monitor)
    Tos.HackMemoryForExtendedMonitor();
  DEBUG_ONLY(if(debug_wipe_log_on_reset) logfile_wipe(); )
  log_write("************************* Power On ************************");
  osd_init_run(0);
  video_mixed_output=0;
  floppy_mediach[0]=0;
  floppy_mediach[1]=0;
  os_gemdos_vector=0;
  os_bios_vector=0;
  os_xbios_vector=0;
#ifndef DISABLE_STEMDOS
  stemdos_reset();
#endif
  interrupt_depth=0;
  hbl_count=0;
  agenda_length=0;
  // some replication because reset_st() not called on start
  LPEEK(0)=ROM_LPEEK(0);
  LPEEK(4)=ROM_LPEEK(4);
  SR=0x2700;
  UPDATE_FLAGS;
  areg[7]=LPEEK(0);
  SET_PC(LPEEK(4));
  emudetect_reset();
  snapshot_loaded=0;
  for(BYTE floppyno=0;floppyno<2;floppyno++)
  {
    FloppyDrive[floppyno].track=0;
    FloppyDrive[floppyno].Id=floppyno;
    FloppyDrive[floppyno].motor=false;
  }
  psg_reg_select=0;
  psg_reg_data=0;
  Dma.mcr=0; // see reset_peripherals()
  Fdc.tr=Fdc.sr=Fdc.dr=0;
  SSEConfig.SwitchSTModel(ST_MODEL);
  ZeroMemory(&Mfp.reg[MFPR_TADR],4);
  reset_peripherals(true);
  init_screen();
  init_timings();
  disable_input_vbl_count=50*3; // 3 seconds
#if defined(SSE_VID_DD_3BUFFER_WIN)
  Disp.VSyncTiming=0;
#endif
  if(MONO && SSEOptions.MonochromeDisableBorder)
    border=0;
  else
    border=border_last_chosen;
#if defined(SSE_HD6301_LL) && defined(SSE_IKBD_RTC)
  if(OPTION_C1)
  {
    if(OPTION_BATTERY6301)
    {
      ikbd_set_clock_to_correct_time();
      hd6301_poke(0x88, 0xaa); //init
    }
    else
      hd6301_poke(0x88, 0x00); //not init
  }
#endif
  Tos.CheckSTTypeAndTos();
}


#define LOGSECTION LOGSECTION_ALWAYS

void reset_peripherals(bool Cold) {
  DBG_LOG("***** reset peripherals ****");
  Debug.Reset(Cold);
  Cpu.ProcessingState=TMC68000::NORMAL;
  if(extended_monitor)
  {
    if(em_planes==1)
    {
      screen_res=2;
      Glue.video_freq=MONO_HZ;
      video_freq_idx=2;
    }
    else
    {
      screen_res=0;
      screen_res=Shifter.m_ShiftMode; //?
      Glue.video_freq=50;
      video_freq_idx=0;
    }
    Glue.m_ShiftMode=screen_res; // important for extended monitor
    Tos.HackMemoryForExtendedMonitor();
  }
  else if(COLOUR_MONITOR) //TODO
  {
    screen_res=0;
    Glue.video_freq=60;
    video_freq_idx=1;
  }
  else
  {
    screen_res=2;
    Glue.video_freq=MONO_HZ;
    video_freq_idx=2;
  }
  SSEConfig.OverscanOn=FALSE;
  Cpu.Reset(Cold);
  Glue.Reset(Cold);
  Mmu.Reset(Cold);
  Shifter.Reset(Cold);
  OptionBox.UpdateSTVideoPage();
  Fdc.Reset();
  Mfp.Reset(Cold);
  if(!OPTION_68901)
  {
    CALC_CYCLES_FROM_HBL_TO_TIMER_B(Glue.video_freq);
  }
  Dma.sr=1;  // reset: one
  if(Cold) //v402 
    Dma.mcr=0; // reset: not affected
  Dma.Counter=0; // reset: all zeros
  Dma.ByteCount=0;
#if USE_PASTI
  if(hPasti)
  {
//    LOG_TO(LOGSECTION_PASTI,"PASTI: Reset, calling HwReset()");
    pasti->HwReset(Cold);
  }
#endif
#if defined(SSE_DISK_CAPS)
  if(CAPSIMG_OK)
    Caps.Reset();
#endif
#if defined(SSE_ACSI) 
  if(ACSI_EMU_ON)
    for(int i=0;i<TAcsiHdc::MAX_ACSI_DEVICES;i++)
      AcsiHdc[i].Reset();
#endif
  if(Cold) // the MC6850 has no reset pin
    ACIA_Reset(ACIA_IKBD,true);
  if(!SSEConfig.Mega||Cold) // interesting remark by MasterOfGizmo, left without an answer at AF
    ikbd_reset(true); // Always cold reset, soft reset is different
#if defined(SSE_HD6301_LL)
  Ikbd.ResetChip(Cold);
#endif
  if(Cold)
    ACIA_Reset(ACIA_MIDI,true);
  MIDIPort.Reset();
  ParallelPort.Reset();
  SerialPort.Reset();
  RS232_CalculateBaud(((Mfp.reg[MFPR_UCR]&BIT_7)!=0),
    Mfp.get_timer_control_register(3),true);
  RS232_VBL();
  ZeroMemory(&Blitter,sizeof(Blitter));
  if(runstate==RUNSTATE_RUNNING)
    prepare_next_event();
#if defined(SSE_DRIVE_SOUND)
  if(Cold)
  {
    FloppyDrive[0].Sound_StopBuffers();
    FloppyDrive[1].Sound_StopBuffers();
  }
#endif
#if defined(SSE_VID_STVL1)
  acc_cycles=0;
  if(hStvl)
    video_logic_reset(&Stvl,Cold);
  draw_mem_line_ptr=draw_mem;
#endif
  Psg.Reset();
  palette_convert_all(); // C3, if changing monitor triggers CPU op reset
#if defined(SSE_MEGASTE)
  MegaSte.MemCache.Reset(); // if necessary
  MegaSte.FdHd=0;
#endif
}

#undef LOGSECTION


void reset_st(DWORD flags) {
  // first get out of instruction function if needs be
#ifndef STEEM_CRT
  if(runstate==RUNSTATE_RUNNING&&!(flags&RESET_STAGE2))
  {
    if(OPTION_EMUTHREAD) // schedule call to longjmp from proper thread
      agenda_add(agenda_reset,2,flags);
    else
      exception(0,0,flags);
    return;
  }
#endif
  TRACE_INIT("reset_st, flags %X\n",flags);
  bool Stop=((flags & RESET_NOSTOP)==0);
  bool Warm=((flags & RESET_WARM)!=0);
  bool ChangeSettings=((flags & RESET_NOCHANGESETTINGS)==0);
  bool Backup=((flags & RESET_NOBACKUP)==0);
  if(runstate==RUNSTATE_RUNNING && Stop) 
    runstate=RUNSTATE_STOPPING;
  if(Backup) 
    GUISaveResetBackup();
  if(Warm)
  {
    if(Cpu.ProcessingState==TMC68000::HALTED)
    {
      CLICK_PLAY_BUTTON(); // ST can be reset on HALT
    }
    Cpu.ProcessingState=TMC68000::NORMAL;
    reset_peripherals(0);
#ifndef DISABLE_STEMDOS
    stemdos_set_drive_reset();
#endif
    if(runstate==RUNSTATE_STOPPED)
    {
      // Hack alert! Show user reset has happened
      MEM_ADDRESS old_x2=vbase;
      vbase=0;
      draw_end();
      draw(0);
      vbase=old_x2;
    }
    else
      vbase=0;
  }
  else // cold
  {
    if(ChangeSettings) 
      GUIColdResetChangeSettings();
    power_on();
    palette_convert_all();
    if(ResChangeResize) 
      StemWinResize();
    draw_end();
    draw(true); // black now
    ComputerRestore();
#if defined(SSE_STATS)
    Stats.nException[0]++; // just been cleared
#endif
  }
  areg[7]=LPEEK(0); // until changed by TOS
  SET_PC(LPEEK(4));
  SR=0x2700;
  UPDATE_FLAGS;
  DEBUG_ONLY(debug_reset(); )
  video_freq_at_start_of_vbl=Glue.video_freq;
  PasteIntoSTAction(STPASTE_STOP);
  CheckResetIcon();
  CheckResetDisplay();
  //scan_y=0;
#ifndef NO_CRAZY_MONITOR
  line_a_base=0;
  vdi_intout=0;
  aes_calls_since_reset=0;
  if(extended_monitor)
  {
    extended_monitor=1; //first stage of extmon init
    m68k_lpoke(0x42E,mem_len); //phystop
  }
#endif
#ifndef DISABLE_STEMDOS
  stemdos_reset();
#endif
  TRACE2("RESET ");
  Debug.TraceGeneralInfos(TDebug::RESET);
}


/*  exception() executes a C longjmp, must be called from the thread where
    it was set up - we use agenda, not event, in part because we have a 
    parameter.
*/

void agenda_reset(int flags) { // from GUI
  exception(0,0,flags);
}

