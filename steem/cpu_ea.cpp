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
FILE: cpu_ea.cpp
DESCRIPTION: Functions for MC68000 Effective Address (EA), Read/Write (RW)
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop
#include <computer.h>
#include <interface_stvl.h>

// peek (byte), dpeek (word)

BYTE d8; // meta 8bit register for peek (can be dbush or dbusl)

/*  Peek/poke functions can be used generally, not only for CPU emulation.
    STF and STE have different ROM addresses for both the TOS and, potentially,
    the cartridge.
    ad can be odd.
*/

//peek

BYTE m68k_peek_stf(MEM_ADDRESS ad) {
  abus=(ad&0xfffffe); // without bit 0
  MEM_ADDRESS bit0=(ad&1);
#if defined(BIG_ENDIAN_PROCESSOR)
  MEM_ADDRESS byte_index=bit0;
#else
  MEM_ADDRESS byte_index=bit0^1;
#endif
  MEM_ADDRESS fake_abus=abus+bit0;
  d8=0xff; // default
  // RAM
  if(abus<FOUR_MEGS)
  {
    if(Mmu.Confused)
      d8=mmu_confused_peek(fake_abus,true);
    else if(abus>=MEM_START_OF_USER_AREA||(SUPERFLAG))
    {
      if(abus<himem)
      {
        DEBUG_CHECK_READ_B(fake_abus);
        d8=(PEEK(fake_abus));
      }
      else switch(OPTION_VLE) { // reflect MMU/Shifter bus
      case 1: // OPTION_C2
        Mmu.UpdateVideoCounter((short)LINECYCLES);
        if(Mmu.VideoCounter<himem) // don't crash! return $FF
          d8=PEEK(Mmu.VideoCounter+bit0);
        break;
#if defined(SSE_VID_STVL1)
      case 2: // OPTION_C3
        d8=Stvl.rambus.d8[byte_index];
        break;
#endif
      }
    }
    else
      exception(BOMBS_BUS_ERROR,EA_READ,abus);
  }
  // IO
  else if(abus>=MEM_IO_BASE) // FFXXXX
  {
    DU16 io_word;
    io_word.d16=io_read(abus);
    d8=io_word.d8[byte_index];
  }
  // TOS
  else if(abus>=rom_addr&&abus<rom_addr_end)
  {
    DWORD rombus=fake_abus-rom_addr;
    if(rombus<tos_len)
      d8=ROM_PEEK(rombus);
  }
  // CART
  else if(abus>=0xFA0000&&abus<0xFC0000)
  {
    if(cart)
    {
      DWORD cartbus=fake_abus-0xFA0000;
#if defined(SSE_SOUND_CARTRIDGE)
/*  See m68k_dpeek().
    B.A.T I and II and Music Master use MOVE.W.
    Drumbeat for the Replay 16 cartridge uses MOVE.B. 
*/
      if(SSEConfig.mv16)
        mv16_fetch((WORD)cartbus);
#endif
      d8=CART_PEEK(cartbus);
    }
  }
#if defined(SSE_MMU_MONSTER_ALT_RAM)
  else if(abus>=FOUR_MEGS && abus<Mmu.MonSTerHimem)
  {
    DEBUG_CHECK_READ_B(fake_abus);
    d8=PEEK(fake_abus);
  }
#endif
  else
    exception(BOMBS_BUS_ERROR,EA_READ,abus);
  udbus.d8[byte_index]=d8;
  return d8;
}


BYTE m68k_peek_ste(MEM_ADDRESS ad) {
  abus=(ad&0xfffffe); // without bit 0
  MEM_ADDRESS bit0=(ad&1);
#if defined(BIG_ENDIAN_PROCESSOR)
  MEM_ADDRESS byte_index=bit0;
#else
  MEM_ADDRESS byte_index=bit0^1;
#endif
  MEM_ADDRESS fake_abus=abus+bit0;
  //BYTE 
  d8=0xff; // default
  // RAM
  if(abus<FOUR_MEGS)
  {
    if(Mmu.Confused)
      d8=mmu_confused_peek(fake_abus,true);
    else if(abus>=MEM_START_OF_USER_AREA||(SUPERFLAG))
    {
      if(abus<himem)
      {
        DEBUG_CHECK_READ_B(fake_abus);
        d8=(PEEK(fake_abus));
      }
      else switch(OPTION_VLE) { // reflect MMU/Shifter bus
      case 1: // OPTION_C2
        Mmu.UpdateVideoCounter((short)LINECYCLES);
        if(Mmu.VideoCounter<himem)
          d8=PEEK(Mmu.VideoCounter+bit0);
        break;
#if defined(SSE_VID_STVL1)
      case 2: // OPTION_C3
        d8=Stvl.rambus.d8[byte_index];
        break;
#endif
      }
    }
    else
      exception(BOMBS_BUS_ERROR,EA_READ,abus);
  }
  // IO
  else if(abus>=MEM_IO_BASE) // FFXXXX
  {
    DU16 io_word;
    io_word.d16=io_read(abus);
    d8=io_word.d8[byte_index];
  }
  // TOS
  else if(abus>=0xE00000&&abus<0xEC0000)
  {
    DWORD rombus=fake_abus-rom_addr;
    if(rombus<tos_len)
      d8=ROM_PEEK(rombus);
  }
  // CART
  else if(abus>=Glue.cartbase && abus<Glue.cartend)
  {
    if(cart)
    {
      DWORD cartbus=fake_abus-Glue.cartbase;
      if(Glue.gamecart && cartbus>256*1024)
        cartbus-=(256*1024-64*1024);
#if defined(SSE_SOUND_CARTRIDGE)
      else  if(SSEConfig.mv16)
        mv16_fetch((WORD)cartbus);
#endif
      d8=CART_PEEK(cartbus);
    }
  }
#if defined(SSE_MMU_MONSTER_ALT_RAM)
  else if(abus>=FOUR_MEGS && abus<Mmu.MonSTerHimem)
  {
    DEBUG_CHECK_READ_B(fake_abus);
    d8=PEEK(fake_abus);
  }
#endif
  else if(abus>=0xD00000&&abus<0xD80000)
    ;
  else if(abus>=0xFE0000&&abus<0xFE2000)
    ;
  else
    exception(BOMBS_BUS_ERROR,EA_READ,abus);
  udbus.d8[byte_index]=d8;
  return d8;
}


WORD m68k_dpeek_stf(MEM_ADDRESS ad) {
  abus=(ad&0xfffffe);
  dbus=0xffff; // default
  if(ad&1)
    exception(BOMBS_ADDRESS_ERROR,EA_READ,abus);
  // RAM
  else if(abus<FOUR_MEGS)
  {
    if(Mmu.Confused)
      dbus=mmu_confused_dpeek(abus,true);
    else if(abus>=MEM_START_OF_USER_AREA||(SUPERFLAG))
    {
      if(abus<himem)
      {
        DEBUG_CHECK_READ_W(abus);
        dbus=DPEEK(abus);
      }
      else switch(OPTION_VLE) { // reflect MMU/Shifter bus
      case 1: // OPTION_C2
        Mmu.UpdateVideoCounter((short)LINECYCLES);
        if(Mmu.VideoCounter<himem)
          dbus=DPEEK(Mmu.VideoCounter);
        break;
#if defined(SSE_VID_STVL1)
      case 2: // OPTION_C3
        dbus=Stvl.rambus.d16;
        break;
#endif
      }
    }
    else
      exception(BOMBS_BUS_ERROR,EA_READ,abus);
  }
  // IO
  else if(abus>=MEM_IO_BASE) // FFXXXX
  {
    dbus=io_read(abus);
  }
  // TOS
  else if(abus>=rom_addr&&abus<rom_addr_end)
  {
    DWORD rombus=abus-rom_addr;
    if(rombus<tos_len)
      dbus=ROM_DPEEK(rombus);
  }
  // CART
  else if(abus>=0xFA0000&&abus<0xFC0000)
  {
    if(cart)
    {
      DWORD cartbus=abus-0xFA0000;
#if defined(SSE_SOUND_CARTRIDGE)
/*  The MV16 cartridge was designed for the game B.A.T.
    It plays samples sent through reading an address on the 
    cartridge (address=data).
*/
      if(SSEConfig.mv16)
        mv16_fetch((WORD)cartbus);
#endif
      dbus=CART_DPEEK(cartbus);
    }
  }
#if defined(SSE_MMU_MONSTER_ALT_RAM)
  else if(abus>=FOUR_MEGS && abus<Mmu.MonSTerHimem)
  {
    DEBUG_CHECK_READ_W(abus);
    dbus=DPEEK(abus);
  }
#endif
  else
    exception(BOMBS_BUS_ERROR,EA_READ,abus);
  return dbus;
}


WORD m68k_dpeek_ste(MEM_ADDRESS ad) {
  abus=(ad&0xfffffe);
  dbus=0xffff; // default
  if(ad&1)
    exception(BOMBS_ADDRESS_ERROR,EA_READ,abus);
  // RAM
  else if(abus<FOUR_MEGS)
  {
    if(Mmu.Confused)
      dbus=mmu_confused_dpeek(abus,true);
    else if(abus>=MEM_START_OF_USER_AREA||(SUPERFLAG))
    {
      if(abus<himem)
      {
        DEBUG_CHECK_READ_W(abus);
        dbus=DPEEK(abus);
      }
      else switch(OPTION_VLE) { // reflect MMU/Shifter bus
      case 1: // OPTION_C2
        Mmu.UpdateVideoCounter((short)LINECYCLES);
        if(Mmu.VideoCounter<himem)
          dbus=DPEEK(Mmu.VideoCounter);
        break;
#if defined(SSE_VID_STVL1)
      case 2: // OPTION_C3
        dbus=Stvl.rambus.d16;
        break;
#endif
      }
    }
    else
      exception(BOMBS_BUS_ERROR,EA_READ,abus);
  }
  // IO
  else if(abus>=MEM_IO_BASE) // FFXXXX
  {
    dbus=io_read(abus);
  }
  // TOS
  else if(abus>=0xE00000&&abus<0xEC0000)
  {
    DWORD rombus=abus-rom_addr;
    if(rombus<tos_len)
      dbus=ROM_DPEEK(rombus);
  }
  // CART
  else if(abus>=Glue.cartbase && abus<Glue.cartend)
  {
    if(cart)
    {
      DWORD cartbus=abus-Glue.cartbase;
      if(Glue.gamecart && cartbus>256*1024)
        cartbus-=(256*1024-64*1024);
#if defined(SSE_SOUND_CARTRIDGE)
      else if(SSEConfig.mv16)
        mv16_fetch((WORD)cartbus);
#endif
      dbus=CART_DPEEK(cartbus);
    }
  }
#if defined(SSE_MMU_MONSTER_ALT_RAM)
  else if(abus>=FOUR_MEGS && abus<Mmu.MonSTerHimem)
  {
    DEBUG_CHECK_READ_W(abus);
    dbus=DPEEK(abus);
  }
#endif
  else if(abus>=0xD00000&&abus<0xD80000)
    ;
  else if(abus>=0xFE0000&&abus<0xFE2000)
    ;
  else
    exception(BOMBS_BUS_ERROR,EA_READ,abus);
  return dbus;
}


DWORD m68k_lpeek(MEM_ADDRESS ad) {
  // only used by utilities
  DWORD l=m68k_dpeek(ad)<<16;
  l|=m68k_dpeek(ad+2);
  return l;
}


// fetch

WORD m68k_fetch_stf(MEM_ADDRESS ad) {
  dbus=0xffff; // default
  if(ad&1)
    exception(BOMBS_ADDRESS_ERROR,EA_FETCH,ad);
  // RAM
  else if(abus<FOUR_MEGS)
  {
    if(Mmu.Confused)
      dbus=mmu_confused_dpeek(abus,true);
    else if(abus>=MEM_START_OF_USER_AREA||(SUPERFLAG))
    {
      if(abus<himem)
        dbus=DPEEK(abus);
      else switch(OPTION_VLE) { // reflect MMU/Shifter bus
      case 1: // OPTION_C2
        Mmu.UpdateVideoCounter((short)LINECYCLES);
        if(Mmu.VideoCounter<himem)
          dbus=DPEEK(Mmu.VideoCounter);
        break;
#if defined(SSE_VID_STVL1)
      case 2: // OPTION_C3
        dbus=Stvl.rambus.d16;
        break;
#endif
      }
    }
    else
      exception(BOMBS_BUS_ERROR,EA_FETCH,ad);
  }
  // IO
  else if(abus>=MEM_IO_BASE) // FFXXXX
  {
    dbus=io_read(abus);
  }
  // TOS
  else if(abus>=rom_addr&&abus<rom_addr_end)
  {
    DWORD rombus=abus-rom_addr;
    if(rombus<tos_len)
      dbus=ROM_DPEEK(rombus);
  }
  // CART
  else if(abus>=0xFA0000&&abus<0xFC0000)
  {
    if(cart)
    {
      DWORD cartbus=abus-0xFA0000;
      dbus=CART_DPEEK(cartbus);
    }
  }
#if defined(SSE_MMU_MONSTER_ALT_RAM)
  else if(abus>=FOUR_MEGS && abus<Mmu.MonSTerHimem)
  {
    dbus=DPEEK(abus);
  }
#endif
  else
    exception(BOMBS_BUS_ERROR,EA_FETCH,ad);
  return dbus;
}


WORD m68k_fetch_ste(MEM_ADDRESS ad) {
  dbus=0xffff; // default
  if(ad&1)
    exception(BOMBS_ADDRESS_ERROR,EA_FETCH,ad);
  // RAM
  else if(abus<FOUR_MEGS)
  {
    if(Mmu.Confused)
      dbus=mmu_confused_dpeek(abus,true);
    else if(abus>=MEM_START_OF_USER_AREA||(SUPERFLAG))
    {
      if(abus<himem)
        dbus=DPEEK(abus);
      else switch(OPTION_VLE) { // reflect MMU/Shifter bus
      case 1: // OPTION_C2
        Mmu.UpdateVideoCounter((short)LINECYCLES);
        if(Mmu.VideoCounter<himem)
          dbus=DPEEK(Mmu.VideoCounter);
        break;
#if defined(SSE_VID_STVL1)
      case 2: // OPTION_C3
        dbus=Stvl.rambus.d16;
        break;
#endif
      }
    }
    else
      exception(BOMBS_BUS_ERROR,EA_FETCH,ad);
  }
  // IO
  else if(abus>=MEM_IO_BASE) // FFXXXX
  {
    dbus=io_read(abus);
  }
  // TOS
  else if(abus>=0xE00000&&abus<0xEC0000)
  {
    DWORD rombus=abus-rom_addr;
    if(rombus<tos_len)
      dbus=ROM_DPEEK(rombus);
  }
  // CART
  else if(abus>=Glue.cartbase && abus<Glue.cartend)
  {
    if(cart)
    {
      DWORD cartbus=abus-Glue.cartbase;
      if(Glue.gamecart && cartbus>256*1024)
        cartbus-=(256*1024-64*1024);
      dbus=CART_DPEEK(cartbus);
    }
  }
#if defined(SSE_MMU_MONSTER_ALT_RAM)
  else if(abus>=FOUR_MEGS && abus<Mmu.MonSTerHimem)
  {
    dbus=DPEEK(abus);
  }
#endif
  else if(abus>=0xD00000&&abus<0xD80000)
    ;
  else if(abus>=0xFE0000&&abus<0xFE2000)
    ;
  else
    exception(BOMBS_BUS_ERROR,EA_FETCH,ad);
  return dbus;
}


void m68k_poke_abus(BYTE x) {
  MEM_ADDRESS fake_abus=(iabus&0xffffff);
  abus=(iabus&0xfffffe);
  if(abus>=MEM_IO_BASE)
    io_write(abus,udbus);
  else if(abus>=himem||Mmu.Confused)
  {
    if(Mmu.Confused)
      mmu_confused_poke_abus(x); 
    else if(abus>=FOUR_MEGS)
    {
#if defined(SSE_MMU_MONSTER_ALT_RAM)
      if(abus<Mmu.MonSTerHimem)
      {
        PEEK(fake_abus)=x;
        DEBUG_CHECK_WRITE_B(fake_abus);
      }
      else
#endif
        exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
    }
  }
  else
  {
#if defined(SSE_VID_CHECK_VIDEO_RAM)
    if(OPTION_C2 && abus>=shifter_draw_pointer && abus<
      shifter_draw_pointer_at_start_of_line+LINECYCLES/2 && Glue.FetchingLine())
      Shifter.Render((short)LINECYCLES,DISPATCHER_CPU);
#endif
    if(abus>=MEM_START_OF_USER_AREA||SUPERFLAG && abus>=MEM_FIRST_WRITEABLE)
      PEEK(fake_abus)=x;
    else
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
    DEBUG_CHECK_WRITE_B(fake_abus);
  }
}


void m68k_dpoke_abus(WORD x) {
  abus=(iabus&0xfffffe);
  if(iabus&1)
    exception(BOMBS_ADDRESS_ERROR,EA_WRITE,abus);
  else if(abus>=MEM_IO_BASE) 
    io_write(abus,udbus);
  else if(abus>=himem||Mmu.Confused) 
  {
    if(Mmu.Confused) 
      mmu_confused_dpoke_abus(x); 
    else if(abus>=FOUR_MEGS) 
    {
#if defined(SSE_MMU_MONSTER_ALT_RAM)
      if(abus<Mmu.MonSTerHimem)
      {
        DPEEK(abus)=x;
        DEBUG_CHECK_WRITE_W(iabus&0xffffff);
      }
      else
#endif
        exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
    }
  }
  else 
  {
#if defined(SSE_VID_CHECK_VIDEO_RAM) 
/*  If we're going to write in video RAM of the current scanline,
    we check whether we need to render before. Some programs write
    just after the memory has been fetched, but Steem renders at
    shift mode changes, and if nothing happens, at the end of the line.
    So if we do nothing it will render wrong memory.
    The test isn't perfect and will cause some "false alerts" but
    we have performance in mind: CPU poke is used a lot, it is rare
    when the address bus is around the current scanline.
    Fixes 3615GEN4 by ULM.
    If STVL was released, we could eliminate this part.
*/
    if(OPTION_C2 && abus>=shifter_draw_pointer && abus<
      shifter_draw_pointer_at_start_of_line+LINECYCLES/2 && Glue.FetchingLine())
      Shifter.Render((short)LINECYCLES,DISPATCHER_CPU);
#endif
    if(abus>=MEM_START_OF_USER_AREA||SUPERFLAG && abus>=MEM_FIRST_WRITEABLE)
      DPEEK(abus)=x;
    else
      exception(BOMBS_BUS_ERROR,EA_WRITE,abus);
    DEBUG_CHECK_WRITE_W(iabus);
  }
}


void m68k_lpoke(MEM_ADDRESS ad,LONG x) {
  // lpoke used only by utility functions, not CPU emulation proper
  iabus=ad;
  m68k_lpoke_abus(x);
}


/*

*******************************************************************************
                   OPERAND EFFECTIVE ADDRESS CALCULATION TIMES
*******************************************************************************
-------------------------------------------------------------------------------
       <ea>       |    Exec Time    |               Data Bus Usage
------------------+-----------------+------------------------------------------
.B or .W :        |                 |
  Dn              |          0(0/0) |
  An              |          0(0/0) |
  (An)            |          4(1/0) |                              nr           
  (An)+           |          4(1/0) |                              nr           
  -(An)           |          6(1/0) |                   n          nr           
  (d16,An)        |          8(2/0) |                        np    nr           
  (d8,An,Xn)      |         10(2/0) |                   n    np    nr           
  (xxx).W         |          8(2/0) |                        np    nr           
  (xxx).L         |         12(3/0) |                     np np    nr           
  #<data>         |          4(1/0) |                        np                 
.L :              |                 |
  Dn              |          0(0/0) |
  An              |          0(0/0) |
  (An)            |          8(2/0) |                           nR nr           
  (An)+           |          8(2/0) |                           nR nr           
  -(An)           |         10(2/0) |                   n       nR nr           
  (d16,An)        |         12(3/0) |                        np nR nr           
  (d8,An,Xn)      |         14(3/0) |                   n    np nR nr           
  (xxx).W         |         12(3/0) |                        np nR nr           
  (xxx).L         |         16(4/0) |                     np np nR nr           
  #<data>         |          8(2/0) |                     np np                 

*/

/*  To avoid using function pointers for EA, we could also create
    instructions for all EA but it gets heavy!
*/

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//////////////////////////    GET SOURCE     ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
Explanation for TRUE_PC, it's based on microcodes and confirmed by tests, 
so there's no contest!
It's useful in case of crash during the read.

b543	b210         Mode           .B, .W             .L

000	R            Dn              PC                PC
001	R            An              PC                PC
010	R            (An)            PC                PC
011	R            (An)+           PC                PC
100	R            -(An)           PC+2              PC
101	R            (d16, An)       PC                PC
110	R            (d8, An, Xn)    PC                PC
111	000          (xxx).W         PC+2              PC+2
111	001          (xxx).L         PC+4              PC+4
111	010          (d16, PC)       PC                PC
111	011          (d8, PC, Xn)    PC                PC
111	100          #<data>         PC+2              PC+4


Dn		          000	R	  0(0/0)	0(0/0)	Data Register Direct
An		          001	R	  0(0/0)	0(0/0)	Address Register Direct
(An)		        010	R	  4(1/0)	8(2/0)	Address Register Indirect
(An)+		        011	R	  4(1/0)	8(2/0)	Address Register Indirect with Postincrement
–(An)		        100	R	  6(1/0)	10(2/0)	Address Register Indirect with Predecrement
(d16, An)	      101	R	  8(2/0)	12(3/0)	Address Register Indirect with Displacement
(d8, An, Xn)*		110	R	  10(2/0)	14(3/0)	Address Register Indirect with Index
(xxx).W		      111	000	8(2/0)	12(3/0)	Absolute Short
(xxx).L		      111	001	12(3/0)	16(4/0)	Absolute Long
(d16, PC)		    111	010	8(2/0)	12(3/0)	Program Counter Indirect with Displacement
(d8, PC, Xn)*		111	011	10(2/0)	14(3/0)	Program Counter Indirect with Index - 8-Bit Displacement
#<data>		      111	100	4(1/0)	8(2/0)	Immediate
*/

// Dn

void m68k_get_source_000_b() { // .B Dn
  //  Dn              |          0(0/0) |
  m68k_src_b=(BYTE)(Cpu.r[PARAM_M]);
}


void m68k_get_source_000_w() { //.W Dn
  //  Dn              |          0(0/0) |
  m68k_src_w=(WORD)(Cpu.r[PARAM_M]);
}


void m68k_get_source_000_l() { //.L Dn
  //  Dn              |          0(0/0) |
  m68k_src_l=(long)(Cpu.r[PARAM_M]);
}


// An

void m68k_get_source_001_b() { // .B An
  //  An              |          0(0/0) |
  m68k_src_b=(BYTE)(AREG(PARAM_M));
}


void m68k_get_source_001_w() { // .W An
  //  An              |          0(0/0) |
  m68k_src_w=(WORD)(AREG(PARAM_M));
}


void m68k_get_source_001_l() { // .L An
  //  An              |          0(0/0) |
  m68k_src_l=(long)(AREG(PARAM_M));
}


// (An)

void m68k_get_source_010_b() { // .B (An)
  //  (An)            |          4(1/0) |                              nr       
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_src_b=d8;
}


void m68k_get_source_010_w() { // .W (An)
  //  (An)            |          4(1/0) |                              nr       
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nr
  m68k_src_w=dbus;
}


void m68k_get_source_010_l() { // .L (An)
  //  (An)            |          8(2/0) |                           nR nr         
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nR
  m68k_src_lh=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; //nr
  m68k_src_ll=dbus;
}


// (An)+

void m68k_get_source_011_b() { // .B (An)+
  //  (An)+           |          4(1/0) |                              nr           
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_src_b=d8;
  AREG(PARAM_M)++;
  if(PARAM_M==7)
    AREG(PARAM_M)++;
}


void m68k_get_source_011_w() { // .W (An)+
  //  (An)+           |          4(1/0) |                              nr           
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nr
  m68k_src_w=dbus;
  AREG(PARAM_M)+=2;
}


void m68k_get_source_011_l() { // .L (An)+
  //  (An)+           |          8(2/0) |                           nR nr           
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nR
  m68k_src_lh=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; //nr
  m68k_src_ll=dbus;
  AREG(PARAM_M)+=4; // for .L, we assume ++ post read
}


// -(An)

void m68k_get_source_100_b() { // .B -(An)
  //  -(An)           |          6(1/0) |                   n          nr           
  TRUE_PC+=2;
  CPU_BUS_IDLE(2); //n
  AREG(PARAM_M)--;
  if(PARAM_M==7)
    AREG(PARAM_M)--;
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_src_b=d8;
}


void m68k_get_source_100_b_a7() { // .B -(An)
  //  -(An)           |          6(1/0) |                   n          nr           
  TRUE_PC+=2;
  CPU_BUS_IDLE(2); //n
  AREG(PARAM_M)-=2;
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_src_b=d8;
}


void m68k_get_source_100_w() { // .W -(An)
  //  -(An)           |          6(1/0) |                   n          nr           
  TRUE_PC+=2;
  CPU_BUS_IDLE(2); //n
  AREG(PARAM_M)-=2;
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nr
  m68k_src_w=dbus;
}


void m68k_get_source_100_l() { // .L -(An)
  //  -(An)           |         10(2/0) |                   n       nR nr           
  CPU_BUS_IDLE(2); //n
  AREG(PARAM_M)-=4;
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nR
  m68k_src_lh=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; //nr
  m68k_src_ll=dbus;
}


// (d16, An)

void m68k_get_source_101_b() { //.B (d16, An)
  //  (d16,An)        |          8(2/0) |                        np    nr           
  iabus=AREG(PARAM_M)+(signed short)IRC;
  PREFETCH; //np
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_src_b=d8;
}


void m68k_get_source_101_w() { // .W (d16, An)
  //  (d16,An)        |          8(2/0) |                        np    nr           
  iabus=AREG(PARAM_M)+(signed short)IRC;
  PREFETCH; //np
  CPU_BUS_ACCESS_READ; //nr
  m68k_src_w=dbus;
}


void m68k_get_source_101_l() { // .L (d16, An)
  //  (d16,An)        |         12(3/0) |                        np nR nr           
  iabus=AREG(PARAM_M)+(signed short)IRC;
  PREFETCH; //np
  CPU_BUS_ACCESS_READ; //nR
  m68k_src_lh=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; //nr
  m68k_src_ll=dbus;
}


// (d8,An,Xn)

void m68k_get_source_110_b() { // .B (d8,An,Xn)
  //  (d8,An,Xn)      |         10(2/0) |                   n    np    nr           
  CPU_BUS_IDLE(2); //n
  if(IRC&BIT_b)   //.l
    iabus=AREG(PARAM_M)+(signed char)(IRC)+(int)Cpu.r[IRC>>12];
  else          //.w
    iabus=AREG(PARAM_M)+(signed char)(IRC)+(signed short)Cpu.r[IRC>>12];
  PREFETCH; //np
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_src_b=d8;
}


void m68k_get_source_110_w() { // .W (d8,An,Xn)
  //  (d8,An,Xn)      |         10(2/0) |                   n    np    nr           
  CPU_BUS_IDLE(2); //n
  if(IRC&BIT_b)   //.l
    iabus=AREG(PARAM_M)+(signed char)(IRC)+(int)Cpu.r[IRC>>12];
  else          //.w
    iabus=AREG(PARAM_M)+(signed char)(IRC)+(signed short)Cpu.r[IRC>>12];
  PREFETCH; //np
  CPU_BUS_ACCESS_READ; //nr
  m68k_src_w=dbus;
}


void m68k_get_source_110_l() { // .L (d8,An,Xn)
  //  (d8,An,Xn)      |         14(3/0) |                   n    np nR nr           

  CPU_BUS_IDLE(2); //n
  if(IRC&BIT_b)  //.l
    iabus=AREG(PARAM_M)+(signed char)IRC+(int)Cpu.r[IRC>>12];
  else         //.w
    iabus=AREG(PARAM_M)+(signed char)IRC+(signed short)Cpu.r[IRC>>12];
  PREFETCH; //np
  CPU_BUS_ACCESS_READ; //nR
  m68k_src_lh=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; //nr
  m68k_src_ll=dbus;
}


// (xxx).W, (xxx).L, (d16, PC), (d8, PC, Xn), #<data>

void m68k_get_source_111_b() {
  switch(IRD&0x7) {
  case 0:  // .B (xxx).W
    //  (xxx).W         |          8(2/0) |                        np    nr           
    TRUE_PC+=2;
    iabus=(signed int)(signed short)IRC; // sign extension
    PREFETCH; //np
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_src_b=d8;
    break;
  case 1:  // .B (xxx).L
    //  (xxx).L         |         12(3/0) |                     np np    nr    
    TRUE_PC+=4;
    iabush=IRC;
    PREFETCH; //np
    iabusl=IRC;
    PREFETCH; //np
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_src_b=d8;
    break;
  case 2:  // .B (d16, PC)
    //  (d16,An)        |          8(2/0) |                        np    nr           
    iabus=pc+(signed short)IRC;
    PREFETCH; //np
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_src_b=d8;
    break;
  case 3:  // .B (d8, PC, Xn)
    //  (d8,An,Xn)      |         10(2/0) |                   n    np    nr           
    CPU_BUS_IDLE(2); //n
    m68k_iriwo=IRC;
    if(m68k_iriwo&BIT_b)   //.l
      iabus=pc+(signed char)(m68k_iriwo)+(int)Cpu.r[m68k_iriwo>>12];
    else          //.w
      iabus=pc+(signed char)(m68k_iriwo)+(signed short)Cpu.r[m68k_iriwo>>12];
    PREFETCH; //np
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_src_b=d8;
    break;       //what do bits 8,9,a  of extra word do?  (not always 0)
  case 4:  // .B #<data>
    //  #<data>         |          4(1/0) |                        np                 
    TRUE_PC+=2; // after prefetch?
    m68k_src_b=LOBYTE(IRC);
    PREFETCH; //np
    break;
  default:
    m68k_trap1();
  }//sw
}


void m68k_get_source_111_w() {
  switch(IRD&0x7) {
  case 0:  // .W (xxx).W
    //  (xxx).W         |          8(2/0) |                        np    nr           
    TRUE_PC+=2;
    iabus=(signed short)IRC; // sign extension
    PREFETCH; //np
    CPU_BUS_ACCESS_READ; //nr
    m68k_src_w=dbus;
    break;
  case 1:  // .W (xxx).L
    //  (xxx).L         |         12(3/0) |                     np np    nr    
    TRUE_PC+=4;
    iabush=IRC;
    PREFETCH; //np
    iabusl=IRC;
    PREFETCH; //np
    CPU_BUS_ACCESS_READ; //nr
    m68k_src_w=dbus;
    break;
  case 2:  // .W (d16, PC)
    //  (d16,An)        |          8(2/0) |                        np    nr           
    iabus=pc+(signed short)IRC;
    PREFETCH; //np
    CPU_BUS_ACCESS_READ; //nr
    m68k_src_w=dbus;
    break;
  case 3: // .W (d8, PC, Xn)
    //  (d8,An,Xn)      |         10(2/0) |                   n    np    nr           
    CPU_BUS_IDLE(2); //n
    m68k_iriwo=IRC;
    if(m68k_iriwo&BIT_b)   //.l
      iabus=pc+(signed char)(m68k_iriwo)+(int)Cpu.r[m68k_iriwo>>12];
    else          //.w
      iabus=pc+(signed char)(m68k_iriwo)+(signed short)Cpu.r[m68k_iriwo>>12];
    PREFETCH; //np
    CPU_BUS_ACCESS_READ; //nr
    m68k_src_w=dbus;
    break;       //what do bits 8,9,a  of extra word do?  (not always 0)
  case 4:  // .W #<data>
    //  #<data>         |          4(1/0) |                        np                 
    TRUE_PC+=2;
    m68k_src_w=IRC;
    PREFETCH; //np    
    break;
  default:
    m68k_trap1();
  }//sw
}


void m68k_get_source_111_l() {
  switch(IRD&0x7) {
  case 0:  // .L (xxx).W
    //  (xxx).W         |         12(3/0) |                        np nR nr           
    TRUE_PC+=2;
    iabus=(signed short)IRC;
    PREFETCH; //np
    CPU_BUS_ACCESS_READ; //nR
    m68k_src_lh=dbus;
    iabus+=2;
    CPU_BUS_ACCESS_READ; //nr
    m68k_src_ll=dbus;
    break;
  case 1:  // .L (xxx).L
    //  (xxx).L         |         16(4/0) |                     np np nR nr           
    TRUE_PC+=4;
    iabush=IRC;
    PREFETCH; //np
    iabusl=IRC;
    PREFETCH; //np
    CPU_BUS_ACCESS_READ; //nR
    m68k_src_lh=dbus;
    iabus+=2;
    CPU_BUS_ACCESS_READ; //nr
    m68k_src_ll=dbus;
    break;
  case 2:  // .L (d16, PC)
    //  (d16,An)        |         12(3/0) |                        np nR nr           
    iabus=pc+(signed short)IRC;
    PREFETCH; //np
    CPU_BUS_ACCESS_READ; //nR
    m68k_src_lh=dbus;
    iabus+=2;
    CPU_BUS_ACCESS_READ; //nr
    m68k_src_ll=dbus;
    break;
  case 3:  // .L (d8, PC, Xn)
    //  (d8,An,Xn)      |         14(3/0) |                   n    np nR nr           
    CPU_BUS_IDLE(2); //n
    m68k_iriwo=IRC;
    if(m68k_iriwo&BIT_b)  //.l
      iabus=pc+(signed char)(m68k_iriwo)+(int)Cpu.r[m68k_iriwo>>12];
    else          //.w
      iabus=pc+(signed char)(m68k_iriwo)+(signed short)Cpu.r[m68k_iriwo>>12];
    PREFETCH; //np
    CPU_BUS_ACCESS_READ; //nR
    m68k_src_lh=dbus;
    iabus+=2;
    CPU_BUS_ACCESS_READ; //nr
    m68k_src_ll=dbus;
    break;       //what do bits 8,9,a  of extra word do?  (not always 0)
  case 4:  // .B #<data>
    //  #<data>         |          8(2/0) |                     np np                 
    TRUE_PC+=4;
    m68k_src_lh=IRC;
    PREFETCH; //np
    m68k_src_ll=IRC;
    PREFETCH; //np
    break;
  default:
    m68k_trap1();
  }//sw
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//////////////////////////    GET DEST       ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void m68k_get_dest_000_b() {
  //  Dn              |          0(0/0) |
  m68k_dst_b=cpureg[PARAM_M].d8[B0];
}


void m68k_get_dest_000_w() {
  //  Dn              |          0(0/0) |
  m68k_dst_w=cpureg[PARAM_M].d16[LO];
}


void m68k_get_dest_000_l() {
  //  Dn              |          0(0/0) |
  m68k_dst_l=cpureg[PARAM_M].d32;
}


void m68k_get_dest_001_b() {
  //  An              |          0(0/0) |
  m68k_dst_b=cpureg[PARAM_M+8].d8[B0];
}


void m68k_get_dest_001_w() {
  //  An              |          0(0/0) |
  m68k_dst_w=cpureg[PARAM_M+8].d16[LO];
}


void m68k_get_dest_001_l() {
  //  An              |          0(0/0) |
  m68k_dst_l=cpureg[PARAM_M+8].d32;
}


void m68k_get_dest_010_b() {
  //  (An)            |          4(1/0) |                              nr           
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_dst_b=d8;
}


void m68k_get_dest_010_w() {
  //  (An)            |          4(1/0) |                              nr           
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nr
  m68k_dst_w=dbus;
}


void m68k_get_dest_010_l() {
  //  (An)            |          8(2/0) |                           nR nr           
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nR
  m68k_dst_lh=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; //nr
  m68k_dst_ll=dbus;
}


void m68k_get_dest_011_b() {
  //  (An)+           |          4(1/0) |                              nr           
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_dst_b=d8;
  AREG(PARAM_M)++;
  if(PARAM_M==7)
    AREG(PARAM_M)++;
}


void m68k_get_dest_011_w() {
  //  (An)+           |          4(1/0) |                              nr           
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nr
  m68k_dst_w=dbus;
  AREG(PARAM_M)+=2;
}


void m68k_get_dest_011_l() {
  //  (An)+           |          8(2/0) |                           nR nr           
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nR
  m68k_dst_lh=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; //nr
  m68k_dst_ll=dbus;
  AREG(PARAM_M)+=4;
}


void m68k_get_dest_100_b() {
  //  -(An)           |          6(1/0) |                   n          nr           
  TRUE_PC+=2;
  CPU_BUS_IDLE(2); //n
  AREG(PARAM_M)--;
  if(PARAM_M==7)
    AREG(PARAM_M)--;
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_dst_b=d8;
}


void m68k_get_dest_100_w() {
  //  -(An)           |          6(1/0) |                   n          nr           
  TRUE_PC+=2;
  CPU_BUS_IDLE(2); //n
  AREG(PARAM_M)-=2;
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ;
  m68k_dst_w=dbus;
}


void m68k_get_dest_100_l() {
  //  -(An)           |         10(2/0) |                   n       nR nr           
  CPU_BUS_IDLE(2); //n
  AREG(PARAM_M)-=4;
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nR
  m68k_dst_lh=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; //nr
  m68k_dst_ll=dbus;
}


void m68k_get_dest_101_b() {
  //  (d16,An)        |          8(2/0) |                        np    nr           
  iabus=AREG(PARAM_M)+(signed short)IRC;
  PREFETCH; //np
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_dst_b=d8;
}


void m68k_get_dest_101_w() {
  //  (d16,An)        |          8(2/0) |                        np    nr           
  iabus=AREG(PARAM_M)+(signed short)IRC;
  PREFETCH; //np
  CPU_BUS_ACCESS_READ; //nr
  m68k_dst_w=dbus;
}


void m68k_get_dest_101_l() {
  //  (d16,An)        |         12(3/0) |                        np nR nr           
  iabus=AREG(PARAM_M)+(signed short)IRC;
  PREFETCH; //np
  CPU_BUS_ACCESS_READ; //nR
  m68k_dst_lh=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; //nr
  m68k_dst_ll=dbus;
}


void m68k_get_dest_110_b() {
  //  (d8,An,Xn)      |         10(2/0) |                   n    np    nr           
  CPU_BUS_IDLE(2); //n 
  m68k_iriwo=IRC;
  if(m68k_iriwo&BIT_b)   //.l
    iabus=AREG(PARAM_M)+(signed char)(m68k_iriwo)+(int)Cpu.r[m68k_iriwo>>12];
  else          //.w
    iabus=AREG(PARAM_M)+(signed char)(m68k_iriwo)
    +(signed short)Cpu.r[m68k_iriwo>>12];
  PREFETCH; //np
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_dst_b=d8;
}


void m68k_get_dest_110_w() {
  //  (d8,An,Xn)      |         10(2/0) |                   n    np    nr           
  CPU_BUS_IDLE(2); //n
  m68k_iriwo=IRC;
  if(m68k_iriwo&BIT_b)  //.l
    iabus=AREG(PARAM_M)+(signed char)(m68k_iriwo)+(int)Cpu.r[m68k_iriwo>>12];
  else         //.w
    iabus=AREG(PARAM_M)+(signed char)(m68k_iriwo)
    +(signed short)Cpu.r[m68k_iriwo>>12];
  PREFETCH; //np
  CPU_BUS_ACCESS_READ; //nr
  m68k_dst_w=dbus;
}


void m68k_get_dest_110_l() {
  //  (d8,An,Xn)      |         14(3/0) |                   n    np nR nr           
  CPU_BUS_IDLE(2); //n
  m68k_iriwo=IRC;
  if(m68k_iriwo&BIT_b)   //.l
    iabus=AREG(PARAM_M)+(signed char)(m68k_iriwo)+(int)Cpu.r[m68k_iriwo>>12];
  else          //.w
    iabus=AREG(PARAM_M)+(signed char)(m68k_iriwo)
    +(signed short)Cpu.r[m68k_iriwo>>12];
  PREFETCH; //np
  CPU_BUS_ACCESS_READ; //nR
  m68k_dst_lh=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; //nr
  m68k_dst_ll=dbus;
}


void m68k_get_dest_111_b() {
  switch(IRD&0x7) {
  case 0:
    //  (xxx).W         |          8(2/0) |                        np    nr           
    iabus=(signed short)IRC; //cast important for sign extension!
    PREFETCH; //np
    TRUE_PC+=2;
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_dst_b=d8;
    break;
  case 1:
    //  (xxx).L         |         12(3/0) |                     np np    nr           
    iabush=IRC;
    PREFETCH; //np
    iabusl=IRC;
    PREFETCH; //np
    TRUE_PC+=4;
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_dst_b=d8;
    break;
  default:
    m68k_trap1();
  }
}


void m68k_get_dest_111_w() {
  switch(IRD&0x7) {
  case 0:
    //  (xxx).W         |          8(2/0) |                        np    nr           
    iabus=(signed short)IRC;
    PREFETCH; //np
    TRUE_PC+=2;
    CPU_BUS_ACCESS_READ; //nr
    m68k_dst_w=dbus;
    break;
  case 1:
    //  (xxx).L         |         12(3/0) |                     np np    nr           
    iabush=IRC;
    PREFETCH; //np
    iabusl=IRC;
    PREFETCH; //np
    TRUE_PC+=4;
    CPU_BUS_ACCESS_READ; //nr
    m68k_dst_w=dbus;
    break;
  default:
    m68k_trap1();
  }
}


void m68k_get_dest_111_l() {
  switch(IRD&0x7) {
  case 0:
    //  (xxx).W         |         12(3/0) |                        np nR nr           
    iabus=(signed short)IRC;
    PREFETCH; //np
    TRUE_PC+=2;
    CPU_BUS_ACCESS_READ; //nR
    m68k_dst_lh=dbus;
    iabus+=2;
    CPU_BUS_ACCESS_READ; //nr
    m68k_dst_ll=dbus;
    break;
  case 1:
    //  (xxx).L         |         16(4/0) |                     np np nR nr           
    iabush=IRC;
    PREFETCH; //np
    iabusl=IRC;
    PREFETCH; //np
    TRUE_PC+=4;
    CPU_BUS_ACCESS_READ; //nR
    m68k_dst_lh=dbus;
    iabus+=2;
    CPU_BUS_ACCESS_READ; //nr
    m68k_dst_ll=dbus;
    break;
  default:
    m68k_trap1();
  }
}


// read DEST

BYTE m68k_read_dest_b() { //only used by tst.b, cmpi.b
  BYTE x=0;
  switch(IRD&BITS_543) {
  case BITS_543_000:
    x=cpureg[PARAM_M].d8[B0];
    break;
  case BITS_543_001:
    m68k_trap1();
    break;
  case BITS_543_010:
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ_B; //nr
    x=d8;
    break;
  case BITS_543_011:
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ_B; //nr
    x=d8;
    AREG(PARAM_M)++;
    if(PARAM_M==7)
      areg[7]++;
    break;
  case BITS_543_100:
    AREG(PARAM_M)--;
    if(PARAM_M==7)
      areg[7]--;
    CPU_BUS_IDLE(2); //n
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ_B; //nr
    x=d8;
    break;
  case BITS_543_101:
    //  (d16,An)        | 101 | reg |   8(2/0)   |              np    nr     
    iabus=AREG(PARAM_M)+(signed short)IRC;
    PREFETCH; //np
    CPU_BUS_ACCESS_READ_B; //nr
    x=d8;
    break;
  case BITS_543_110:
    //  (d8,An,Xn)      |         10(2/0) |                   n    np    nr           
    CPU_BUS_IDLE(2); //n
    m68k_iriwo=IRC;
    PREFETCH; //np
    if(m68k_iriwo&BIT_b)   //.l
      iabus=AREG(PARAM_M)+(signed char)(m68k_iriwo)+(int)Cpu.r[m68k_iriwo>>12];
    else          //.w
      iabus=AREG(PARAM_M)+(signed char)(m68k_iriwo)
      +(signed short)Cpu.r[m68k_iriwo>>12];
    CPU_BUS_ACCESS_READ_B; //nr
    x=d8;
    break;
  case BITS_543_111:
    switch(IRD&0x7) {
    case 0:
      //  (xxx).W         | 111 | 000 |   8(2/0)   |              np    nr       
      iabus=(unsigned long)((signed long)((signed short)IRC));
      PREFETCH; //np
      CPU_BUS_ACCESS_READ_B; //nr
      x=d8;
      break;
    case 1:
      //  (xxx).L         | 111 | 001 |  12(3/0)   |           np np    nr    
      iabush=IRC;
      PREFETCH; //np
      iabusl=IRC;
      PREFETCH; //np
      CPU_BUS_ACCESS_READ_B; //nr
      x=d8;
      break;
    default:
      m68k_trap1();
    }
  }
  m68k_dst_b=x;
  return x;
}


WORD m68k_read_dest_w() { //only used by tst.w, cmpi.w
  WORD x=0;
  switch(IRD&BITS_543) {
  case BITS_543_000:
    x=LOWORD(Cpu.r[PARAM_M]);
    break;
  case BITS_543_001:
    m68k_trap1();
    break;
  case BITS_543_010:
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ; //nr
    x=dbus;
    break;
  case BITS_543_011:
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ; //nr
    x=dbus;
    AREG(PARAM_M)+=2;
    break;
  case BITS_543_100:
    CPU_BUS_IDLE(2); //n
    AREG(PARAM_M)-=2;
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ; //nr
    x=dbus;
    break;
  case BITS_543_101:
    //  (d16,An)        | 101 | reg |   8(2/0)   |              np    nr     
    iabus=AREG(PARAM_M)+(signed short)IRC;
    PREFETCH; //np
    CPU_BUS_ACCESS_READ; //nr
    x=dbus;
    break;
  case BITS_543_110:
    //  (d8,An,Xn)      |         10(2/0) |                   n    np    nr           
    CPU_BUS_IDLE(2); //n
    m68k_iriwo=IRC;
    PREFETCH; //np
    if(m68k_iriwo&BIT_b)   //.l
      iabus=AREG(PARAM_M)+(signed char)(m68k_iriwo)+(int)Cpu.r[m68k_iriwo>>12];
    else          //.w
      iabus=AREG(PARAM_M)+(signed char)(m68k_iriwo)
      +(signed short)Cpu.r[m68k_iriwo>>12];
    CPU_BUS_ACCESS_READ; //nr
    x=dbus;
    break;
  case BITS_543_111:
    switch(IRD&0x7) {
    case 0:
      //  (xxx).W         | 111 | 000 |   8(2/0)   |              np    nr       
      iabus=(unsigned long)((signed long)((signed short)IRC));
      PREFETCH; //np
      CPU_BUS_ACCESS_READ; //nr
      x=dbus;
      break;
    case 1:
      //  (xxx).L         | 111 | 001 |  12(3/0)   |           np np    nr    
      iabush=IRC;
      PREFETCH; //np
      iabusl=IRC;
      PREFETCH; //np
      CPU_BUS_ACCESS_READ; //nr
      x=dbus;
      break;
    default:
      m68k_trap1();
    }
  }
  m68k_dst_w=x;
  return x;
}


LONG m68k_read_dest_l() { //only used by tst.l, cmpi.l
  DU32 ux;
  DWORD &x=ux.d32;
  WORD &xh=ux.d16[HI];
  WORD &xl=ux.d16[LO];
  switch(IRD&BITS_543) {
  case BITS_543_000:
    x=Cpu.r[PARAM_M];
    break;
  case BITS_543_001:
    m68k_trap1();
    break;
  case BITS_543_010:
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ; //nR
    xh=dbus;
    iabus+=2;
    CPU_BUS_ACCESS_READ; //nr
    xl=dbus;
    break;
  case BITS_543_011:
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ; //nR
    xh=dbus;
    iabus+=2;
    CPU_BUS_ACCESS_READ; //nr
    xl=dbus;
    AREG(PARAM_M)+=4;
    break;
  case BITS_543_100:
    CPU_BUS_IDLE(2); //n
    AREG(PARAM_M)-=4;
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ; //nR
    xh=dbus;
    iabus+=2;
    CPU_BUS_ACCESS_READ; //nr
    xl=dbus;
    break;
  case BITS_543_101:
    //  (d16,An)        | 101 | reg |  12(3/0)   |              np nR nr          
    iabus=AREG(PARAM_M)+(signed short)IRC;
    PREFETCH; //np
    CPU_BUS_ACCESS_READ; //nR
    xh=dbus;
    iabus+=2;
    CPU_BUS_ACCESS_READ; //nr
    xl=dbus;
    break;
  case BITS_543_110:
    //  (d8,An,Xn)      | 110 | reg |  14(3/0)   |         n    np nR nr           
    CPU_BUS_IDLE(2); //n
    m68k_iriwo=IRC;
    PREFETCH; //np
    if(m68k_iriwo&BIT_b)   //.l
      iabus=AREG(PARAM_M)+(signed char)(m68k_iriwo)+(int)Cpu.r[m68k_iriwo>>12];
    else          //.w
      iabus=AREG(PARAM_M)+(signed char)(m68k_iriwo)
      +(signed short)Cpu.r[m68k_iriwo>>12];
    CPU_BUS_ACCESS_READ; //nR
    xh=dbus;
    iabus+=2;
    CPU_BUS_ACCESS_READ; //nr
    xl=dbus;
    break;
  case BITS_543_111:
    switch(IRD&0x7) {
    case 0:
      //  (xxx).W         | 111 | 000 |  12(3/0)   |              np nR nr      
      iabus=(unsigned long)((signed long)((signed short)IRC));
      PREFETCH; //np
      CPU_BUS_ACCESS_READ; //nR
      xh=dbus;
      iabus+=2;
      CPU_BUS_ACCESS_READ; //nr
      xl=dbus;
      break;
    case 1:
      //(xxx).L         | 111 | 001 |  16(4/0)   |           np np nR nr           
      iabush=IRC;
      PREFETCH; //np
      iabusl=IRC;
      PREFETCH; //np
      CPU_BUS_ACCESS_READ; //nR
      xh=dbus;
      iabus+=2;
      CPU_BUS_ACCESS_READ; //nr
      xl=dbus;
      break;
    default:
      m68k_trap1();
      break;
    }
    break;
  }
  m68k_dst_l=x;
  return x;
}
