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
FILE: cpu_op.cpp
DESCRIPTION: High level emulation of the Motorola MC68000 instructions.
This uses one function for each opcode type. Instructions are executed step
by step, exceptions trigger a longjmp.
Even now that a low-level emu is possible, a hig-level emu still makes sense:
performance, traceability.
Timings given in comments are adapted from Yacht (see 3rdparty/doc).
See Motorola doc for what the instructions do.
This is Steem's biggest source file.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop
#include "computer.h"


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  LINE-0 IMMEDIATE ROUTINES    //////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
-------------------------------------------------------------------------------
 EORI, ORI, ANDI, |    Exec Time    |               Data Bus Usage
    SUBI, ADDI    |  INSTR     EA   | 1st Operand |  2nd OP (ea)  |   INSTR
------------------+-----------------+-------------+---------------+------------
#<data>,<ea> :    |                 |             |               |
  .B or .W :      |                 |             |               |
    Dn            |  8(2/0)  0(0/0) |          np |               | np          
    (An)          | 12(2/1)  4(1/0) |          np |            nr | np nw	      
    (An)+         | 12(2/1)  4(1/0) |          np |            nr | np nw	      
    -(An)         | 12(2/1)  6(1/0) |          np | n          nr | np nw	      
    (d16,An)      | 12(2/1)  8(2/0) |          np |      np    nr | np nw	      
    (d8,An,Xn)    | 12(2/1) 10(2/0) |          np | n    np    nr | np nw	      
    (xxx).W       | 12(2/1)  8(2/0) |          np |      np    nr | np nw	      
    (xxx).L       | 12(2/1) 12(3/0) |          np |   np np    nr | np nw	      
*/

void m68k_ori_b() {
  // prefetch rule (ijor):
  //"Just before the start of any instruction two words (no more and no less) are 
  // already fetched. One word will be in IRD and another one in IRC."
  // IRD has the opcode (now decoded)
  // use source operand that is in IRC (it was already fetched)
  m68k_src_b=(BYTE)IRC; // high order byte can be anything, it is discarded
  // fetch next word into IRC and increment our pc
  // generally, fetching happens as soon as the operand is used, but not always
  PREFETCH; //np 
  // Effective Address (EA): access and read an operand, here the destination
  m68k_GET_DEST_B_NOT_A; // EA 
  // the 68000 mostly uses Arithmetic Unit au to do the fetches, register pc is 
  // mostly updated at some other point
  // in Steem, we mostly use pc to do the fetches, we keep a true pc updated because
  // in case of crash, this is the value pushed on the exception stack frame
  TRUE_PC=pc+2; 
  // to emulate tvn latch (for interrupts), we record the current time in cycles
  // it will be examined in m68kProcess() after the instruction completes
  CHECK_IPL; 
  // fetch next word into IRC, don't increment pc
  // We don't increment pc now because we will do it in m68kProcess() for the
  // next instruction. That way, pc now points to the start of the instruction, which
  // is easier for several emulation parts, including the Debugger. Doesn't matter,
  // pc is our variable, TRUE_PC is the CPU register and it has already been incremented
  PREFETCH_FINAL; //np
  // compute the actual operation; everything is byte-sized
  resultb=(m68k_dst_b|m68k_src_b);
  // update CCR flags using macros (but not the SR variable at this point)
  SR_CHECK_AND_B; 
  // instructions could be split further to handle this (DEST_IS_DATA_REGISTER) test
  // -> m68k_ori_b_r, m68k_ori_b_m etc. (many instructions), not sure we gain performance
  // (code size/speed)
  if(DEST_IS_DATA_REGISTER) 
    REGB(PARAM_M)=resultb; // only lower order byte of the register is affected
  else
  {
    // Motorola 68000 quirk: both bytes of data bus will take the same value!
    // done in the timing function, here we always update lower byte for simplicity
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_andi_b() { // the only difference with ori_b is the operation
  m68k_src_b=(BYTE)IRC;
  PREFETCH; //np
  m68k_GET_DEST_B_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=(m68k_dst_b&m68k_src_b);
  SR_CHECK_AND_B;
  if(DEST_IS_DATA_REGISTER)
    REGB(PARAM_M)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_eori_b() {
  m68k_src_b=(BYTE)IRC;
  PREFETCH; //np
  m68k_GET_DEST_B_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=m68k_dst_b^m68k_src_b;
  SR_CHECK_AND_B;
  if(DEST_IS_DATA_REGISTER)
    REGB(PARAM_M)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_subi_b() {
  m68k_src_b=(BYTE)IRC;
  PREFETCH; //np
  m68k_GET_DEST_B_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=m68k_dst_b-m68k_src_b;
  SR_SUB_B(SR_X);
  if(DEST_IS_DATA_REGISTER)
    REGB(PARAM_M)=resultb;
  else
  {
    dbus=(BYTE)result;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_addi_b() {
  m68k_src_b=(BYTE)IRC;
  PREFETCH; //np
  m68k_GET_DEST_B_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=m68k_dst_b+m68k_src_b;
  SR_ADD_B;
  if(DEST_IS_DATA_REGISTER)
    REGB(PARAM_M)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_ori_w() {
  m68k_src_w=IRC;
  PREFETCH; //np
  m68k_GET_DEST_W_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  // everything is word-sized, resultl stands for result low, there is also resulth
  resultl=(m68k_dst_w|m68k_src_w); 
  SR_CHECK_AND_W;
  if(DEST_IS_DATA_REGISTER)
    REGW(PARAM_M)=resultl;
  else
  {
    dbus=resultl;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


void m68k_andi_w() {
  m68k_src_w=IRC;
  PREFETCH; //np
  m68k_GET_DEST_W_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=(m68k_dst_w&m68k_src_w);
  SR_CHECK_AND_W;
  if(DEST_IS_DATA_REGISTER)
    REGW(PARAM_M)=resultl;
  else
  {
    dbus=resultl;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


void m68k_eori_w() {
  m68k_src_w=IRC;
  PREFETCH; //np
  m68k_GET_DEST_W_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=m68k_dst_w^m68k_src_w;
  SR_CHECK_AND_W;
  if(DEST_IS_DATA_REGISTER)
    REGW(PARAM_M)=resultl;
  else
  {
    dbus=resultl;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


void m68k_subi_w() {
  m68k_src_w=IRC;    
  PREFETCH; //np
  m68k_GET_DEST_W_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=m68k_dst_w-m68k_src_w;
  SR_SUB_W(SR_X);
  if(DEST_IS_DATA_REGISTER)
     REGW(PARAM_M)=resultl;
  else
  {
    dbus=resultl;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


void m68k_addi_w() {
  m68k_src_w=IRC;
  PREFETCH; //np
  m68k_GET_DEST_W_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  m68k_old_dest=m68k_dst_w;
  resultl=m68k_dst_w+m68k_src_w;
  SR_ADD_W;
  if(DEST_IS_DATA_REGISTER)
     REGW(PARAM_M)=resultl;
  else
  {
    dbus=resultl;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


/*
  .L :                              |             |               |
    Dn            | 16(3/0)  0(0/0) |       np np |               | np       nn	
    (An)          | 20(3/2)  8(2/0) |       np np |         nR nr | np nw nW    
    (An)+         | 20(3/2)  8(2/0) |       np np |         nR nr | np nw nW    
    -(An)         | 20(3/2) 10(2/0) |       np np | n       nR nr | np nw nW    
    (d16,An)      | 20(3/2) 12(3/0) |       np np |      np nR nr | np nw nW    
    (d8,An,Xn)    | 20(3/2) 14(3/0) |       np np | n    np nR nr | np nw nW    
    (xxx).W       | 20(3/2) 12(3/0) |       np np |      np nR nr | np nw nW    
    (xxx).L       | 20(3/2) 16(4/0) |       np np |   np np nR nr | np nw nW    
*/

void m68k_ori_l() {
  // we load m68k_src_l with two prefetches
  m68k_GET_IMMEDIATE_L_WITH_TIMING; //np np
  m68k_GET_DEST_L_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  // real 68000 ALU works with 16bit data, we take advantage of our 32bit (or 64bit) ALU
  result=(m68k_dst_l|m68k_src_l);
  SR_CHECK_AND_L;
  if(DEST_IS_DATA_REGISTER)
  {
    CPU_BUS_IDLE(4); //nn
    REGL(PARAM_M)=result;
  }
  else
    // when it reads the destination, the CPU ends with the lower word, when it writes
    // the result, it starts from where it was, we do the same with this macro,
    // using iabus as it is after EA
    WRITE_LONG_BACKWARDS; //nw nW
}


void m68k_andi_l() {
  m68k_GET_IMMEDIATE_L_WITH_TIMING; //np np
  m68k_GET_DEST_L_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  result=(m68k_dst_l&m68k_src_l);
  SR_CHECK_AND_L;
  if(DEST_IS_DATA_REGISTER)
  {
    CPU_BUS_IDLE(4); //nn
    REGL(PARAM_M)=result;
  }
  else
    WRITE_LONG_BACKWARDS; //nw nW
}


void m68k_eori_l() {
  m68k_GET_IMMEDIATE_L_WITH_TIMING; //np np
  m68k_GET_DEST_L_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  result=m68k_dst_l^m68k_src_l;
  SR_CHECK_AND_L;
  if(DEST_IS_DATA_REGISTER)
  {
    CPU_BUS_IDLE(4); //nn
    REGL(PARAM_M)=result;
  }
  else
    WRITE_LONG_BACKWARDS; //nw nW
}


void  m68k_subi_l() {
  m68k_GET_IMMEDIATE_L_WITH_TIMING; //np np
  m68k_GET_DEST_L_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  result=m68k_dst_l-m68k_src_l;
  SR_SUB_L(SR_X);
  if(DEST_IS_DATA_REGISTER)
  {
    CPU_BUS_IDLE(4); //nn
    REGL(PARAM_M)=result;
  }
  else
    WRITE_LONG_BACKWARDS; //nw nW
}


void m68k_addi_l() {
  m68k_GET_IMMEDIATE_L_WITH_TIMING; //np np
  m68k_GET_DEST_L_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  result=m68k_dst_l+m68k_src_l;
  SR_ADD_L;
  if(DEST_IS_DATA_REGISTER)
  {
    CPU_BUS_IDLE(4); //nn
    REGL(PARAM_M)=result;
  }
  else
    WRITE_LONG_BACKWARDS; //nw nW
}


/*
-------------------------------------------------------------------------------
 ORI, ANDI, EORI  |    Exec Time    |               Data Bus Usage
  to CCR, to SR   |      INSTR      | 1st Operand |          INSTR
------------------+-----------------+-------------+----------------------------
#<data>,CCR       |                 |             |               
  .B :            | 20(3/0)         |          np |              nn nn np np   
ijor:
Writing to SR is, internally, quite different than writing to a regular register.
The most important difference is that the prefetch must be flushed and filled again.
This is because the instruction might have changed mode from SUPER to USER 
(other way around is not possible, it would trigger a privilege violation). 
And some systems use a different memory map for each mode. 
Then the prefetch must be filled again, now with the new mode. Yes, just in case.

Not only that, it can't refill the prefetch until the write to SR and the possible
mode change has been completed internally. So they can't overlap.

The first np fetched the opcode for the next instruction or whatever was at that 
location, which in this case is completely wasted because it will be flushed. 
And the wasteful bus cycle is because the instruction just reuses the same microcode 
as every other immediate instruction for that part.
*/

void m68k_ori_b_to_ccr() {
  m68k_src_b=(BYTE)IRC;
  PREFETCH; //np // useless prefetch
  CPU_BUS_IDLE(8); //nn nn 
  // each time the program reads SR, we must build it from boolean flags before
  UPDATE_SR; // upper byte
  CCR|=m68k_src_b;
#ifndef SSE_LEAN_AND_MEAN
  // SR isn't a word register, some bits just don't exist (and read as 0)
  // but we need precise SR only when the program reads it
  SR&=SR_VALID_BITMASK;
#endif
  // each time the program changes SR, we must derive the boolean flags after
  UPDATE_FLAGS;
  // refetch, on the ST same memory but it could be different on other system
  PREFETCH_ONLY; //np 
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_andi_b_to_ccr() {
  m68k_src_b=(BYTE)IRC;
  PREFETCH; //np
  CPU_BUS_IDLE(8); //nn nn 
  UPDATE_SR;
  CCR&=m68k_src_b;
  UPDATE_FLAGS;
  PREFETCH_ONLY; //np
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_eori_b_to_ccr() {
  m68k_src_b=(BYTE)IRC;
  PREFETCH; //np
  CPU_BUS_IDLE(8); //nn nn 
  UPDATE_SR;
  CCR^=m68k_src_b;
#ifndef SSE_LEAN_AND_MEAN
  SR&=SR_VALID_BITMASK;
#endif
  UPDATE_FLAGS;
  PREFETCH_ONLY; //np
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_ori_w_to_sr() {
  m68k_src_w=IRC;
  PREFETCH; //np
  if(SUPERFLAG)
  {
    CPU_BUS_IDLE(8); //nn nn 
    UPDATE_SR;
    SR|=m68k_src_w;
#ifndef SSE_LEAN_AND_MEAN
    SR&=SR_VALID_BITMASK;
#endif
    UPDATE_FLAGS;
    PREFETCH_ONLY; //np
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
  else
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
}


void m68k_andi_w_to_sr() {
  m68k_src_w=IRC;
  PREFETCH; //np
  if(SUPERFLAG)
  {
    UPDATE_SR;
    DEBUG_ONLY(int debug_old_sr=SR; )
    CPU_BUS_IDLE(8); //nn nn 
    SR&=m68k_src_w;
#ifndef SSE_LEAN_AND_MEAN
    SR&=SR_VALID_BITMASK;
#endif
    UPDATE_FLAGS;
    PREFETCH_ONLY; //np
    CHECK_IPL;
    PREFETCH_FINAL; //np
    DETECT_CHANGE_TO_USER_MODE;
    CHECK_STOP_ON_USER_CHANGE
  }
  else
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
}


void m68k_eori_w_to_sr() {
  m68k_src_w=IRC;
  PREFETCH; //np
  if(SUPERFLAG) 
  {
    UPDATE_SR;
    DEBUG_ONLY( int debug_old_sr=SR; )
    CPU_BUS_IDLE(8); //nn nn 
    SR^=m68k_src_w;
#ifndef SSE_LEAN_AND_MEAN
    SR&=SR_VALID_BITMASK;
#endif
    UPDATE_FLAGS;
    PREFETCH_ONLY; //np
    CHECK_IPL;
    PREFETCH_FINAL; //np
    DETECT_CHANGE_TO_USER_MODE
    CHECK_STOP_ON_USER_CHANGE;
  }
  else
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
}


/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       BTST       |  INSTR     EA   | 1st Operand |  2nd OP (ea)  |   INSTR
------------------+-----------------+-------------+---------------+------------
Dn,<ea> :         |                 |             |               |
  .B :            |                 |             |               |
    (An)          |  4(1/0)  4(1/0) |             |            nr | np          
    (An)+         |  4(1/0)  4(1/0) |             |            nr | np          
    -(An)         |  4(1/0)  6(1/0) |             | n          nr | np          
    (d16,An)      |  4(1/0)  8(2/0) |             |      np    nr | np          
    (d8,An,Xn)    |  4(1/0) 10(2/0) |             | n    np    nr | np          
    (xxx).W       |  4(1/0)  8(2/0) |             |      np    nr | np          
    (xxx).L       |  4(1/0) 12(3/0) |             |   np np    nr | np          
Dn,Dm :           |                 |             |               |
  .L :            |  6(1/0)  0(0/0) |             |               | np n        
#<data>,<ea> :    |                 |             |               |             
  .B :            |                 |             |               |
    (An)          |  8(2/0)  4(1/0) |          np |            nr | np          
    (An)+         |  8(2/0)  4(1/0) |          np |            nr | np          
    -(An)         |  8(2/0)  6(1/0) |          np | n          nr | np          
    (d16,An)      |  8(2/0)  8(2/0) |          np |      np    nr | np          
    (d8,An,Xn)    |  8(2/0) 10(2/0) |          np | n    np    nr | np          
    (xxx).W       |  8(2/0)  8(2/0) |          np |      np    nr | np          
    (xxx).L       |  8(2/0) 12(3/0) |          np |   np np    nr | np          
#<data>,Dn :      |                 |             |               |
  .L :            | 10(2/0)  0(0/0) |          np |               | np n        
*/

void m68k_btst() {
  m68k_src_b=(BYTE)IRC;
  PREFETCH; //np
  if(DEST_IS_DATA_REGISTER)
  {
/*
#<data>,Dn :      |                 |             |               |
  .L :            | 10(2/0)  0(0/0) |          np |               | np n     
*/
    CHECK_IPL;
    PREFETCH_FINAL; //np
    CPU_BUS_IDLE(2); //n
    m68k_src_b&=31;
    PSWZ=(((Cpu.r[PARAM_M]>>m68k_src_b)&1)==0);
  }
  else
  {
/*
#<data>,<ea> :    |                 |             |               |             
  .B :            |                 |             |               |
    (An)          |  8(2/0)  4(1/0) |          np |            nr | np          
    (An)+         |  8(2/0)  4(1/0) |          np |            nr | np          
    -(An)         |  8(2/0)  6(1/0) |          np | n          nr | np          
    (d16,An)      |  8(2/0)  8(2/0) |          np |      np    nr | np          
    (d8,An,Xn)    |  8(2/0) 10(2/0) |          np | n    np    nr | np          
    (xxx).W       |  8(2/0)  8(2/0) |          np |      np    nr | np          
    (xxx).L       |  8(2/0) 12(3/0) |          np |   np np    nr | np          
*/
    m68k_ap=(short)(m68k_src_b & 7);
    m68k_GET_SOURCE_B_NOT_A; //EA
    CHECK_IPL;
    PREFETCH_FINAL; //np
    PSWZ=(((m68k_src_b>>m68k_ap)&1)==0);
  }
}


void m68k_btst_from_dN() {
  if(DEST_IS_DATA_REGISTER)
  {  //btst to data register
/*
Dn,Dm :           |                 |             |               |
  .L :            |  6(1/0)  0(0/0) |             |               | np n
*/
    CHECK_IPL;
    PREFETCH_FINAL; //np
    CPU_BUS_IDLE(2); //n
    PSWZ=(((Cpu.r[PARAM_M]>>(31&Cpu.r[PARAM_N]))&1)==0);
  }
  else
  { // btst memory
/*
Dn,<ea> :         |                 |             |               |
  .B :            |                 |             |               |
    (An)          |  4(1/0)  4(1/0) |             |            nr | np
    (An)+         |  4(1/0)  4(1/0) |             |            nr | np
    -(An)         |  4(1/0)  6(1/0) |             | n          nr | np
    (d16,An)      |  4(1/0)  8(2/0) |             |      np    nr | np
    (d8,An,Xn)    |  4(1/0) 10(2/0) |             | n    np    nr | np
    (xxx).W       |  4(1/0)  8(2/0) |             |      np    nr | np
    (xxx).L       |  4(1/0) 12(3/0) |             |   np np    nr | np
 # ?
*/
    m68k_GET_SOURCE_B_NOT_A; //EA  //even immediate mode is allowed!!!!
    CHECK_IPL;
    PREFETCH_FINAL; //np
    PSWZ=(((m68k_src_b>>(7&Cpu.r[PARAM_N]))&1)==0);
  }
}


/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
    BCHG, BSET    |  INSTR     EA   | 1st Operand |  2nd OP (ea)  |      INSTR
------------------+-----------------+-------------+---------------+------------
Dn,<ea> :         |                 |             |               |
  .B :            |                 |             |               |             
    (An)          |  8(1/1)  4(1/0) |             |            nr | np    nw    
    (An)+         |  8(1/1)  4(1/0) |             |            nr | np    nw    
    -(An)         |  8(1/1)  6(1/0) |             | n          nr | np    nw    
    (d16,An)      |  8(1/1)  8(2/0) |             |      np    nr | np    nw    
    (d8,An,Xn)    |  8(1/1) 10(2/0) |             | n    np    nr | np    nw    
    (xxx).W       |  8(1/1)  8(2/0) |             |      np    nr | np    nw    
    (xxx).L       |  8(1/1) 12(3/0) |             |   np np    nr | np    nw    
Dn,Dm :           |                 |             |               |
  .L :            |                 |             |               |
    if Dn<16      |  6(1/0)  0(0/0) |             |               | np       n  
    if Dn>15      |  8(1/0)  0(0/0) |             |               | np       nn 
#<data>,<ea> :    |                 |             |               |
  .B :            |                 |             |               |
    (An)          | 12(2/1)  4(1/0) |          np |            nr | np    nw    
    (An)+         | 12(2/1)  4(1/0) |          np |            nr | np    nw    
    -(An)         | 12(2/1)  6(1/0) |          np | n          nr | np    nw    
    (d16,An)      | 12(2/1)  8(2/0) |          np |      np    nr | np    nw    
    (d8,An,Xn)    | 12(2/1) 10(2/0) |          np | n    np    nr | np    nw    
    (xxx).W       | 12(2/1)  8(2/0) |          np |      np    nr | np    nw    
    (xxx).L       | 12(2/1) 12(3/0) |          np |   np np    nr | np    nw    
#<data>,Dn :      |                 |             |               |
  .L :            |                 |             |               |
    if data<16    | 10(2/0)  0(0/0) |          np |               | np       n  
    if data>15    | 12(2/0)  0(0/0) |          np |               | np       nn 
*/

void m68k_bchg() {
  m68k_src_b=(BYTE)IRC;
  PREFETCH; //np
  if(DEST_IS_DATA_REGISTER)
  {
/*
#<data>,Dn :      |                 |             |               |
  .L :            |                 |             |               |
    if data<16    | 10(2/0)  0(0/0) |          np |               | np       n  
    if data>15    | 12(2/0)  0(0/0) |          np |               | np       nn 
*/
    m68k_dst_l=REGL(PARAM_M);
    CHECK_IPL;
    PREFETCH_FINAL;  //np
    m68k_src_b&=31;
    if(m68k_src_b>15)
      CPU_BUS_IDLE(4); //nn
    else
      CPU_BUS_IDLE(2); //n
    m68k_src_l=1<<m68k_src_b;
    result=m68k_dst_l^m68k_src_l;
    PSWZ=((m68k_dst_l&m68k_src_l)==0);
    REGL(PARAM_M)=result;
  }
  else
  {
/*
#<data>,<ea> :    |                 |             |               |
  .B :            |                 |             |               |
    (An)          | 12(2/1)  4(1/0) |          np |            nr | np    nw    
    (An)+         | 12(2/1)  4(1/0) |          np |            nr | np    nw    
    -(An)         | 12(2/1)  6(1/0) |          np | n          nr | np    nw    
    (d16,An)      | 12(2/1)  8(2/0) |          np |      np    nr | np    nw    
    (d8,An,Xn)    | 12(2/1) 10(2/0) |          np | n    np    nr | np    nw    
    (xxx).W       | 12(2/1)  8(2/0) |          np |      np    nr | np    nw    
    (xxx).L       | 12(2/1) 12(3/0) |          np |   np np    nr | np    nw    
*/
    m68k_src_b&=7;
    m68k_GET_DEST_B_NOT_A; // EA
    TRUE_PC=pc+2;
    CHECK_IPL;
    PREFETCH_FINAL; //np
    m68k_src_b=(BYTE)(1<<m68k_src_b);
    resultb=m68k_dst_b^m68k_src_b;
    PSWZ=((m68k_dst_b&m68k_src_b)==0);
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_bchg_from_dN() {
  if(DEST_IS_DATA_REGISTER)
  {
/*
Dn,Dm :           |                 |             |               |
  .L :            |                 |             |               |
    if Dn<16      |  6(1/0)  0(0/0) |             |               | np       n
    if Dn>15      |  8(1/0)  0(0/0) |             |               | np       nn
*/
    m68k_src_b=(cpureg[PARAM_N].d8[B0]&31);
    CHECK_IPL;
    PREFETCH_FINAL; //np
    if(m68k_src_b>15)
      CPU_BUS_IDLE(4); //nn
    else
      CPU_BUS_IDLE(2); //n
    PSWZ=(((Cpu.r[PARAM_M]>>m68k_src_b)&1)==0);
    Cpu.r[PARAM_M]^=(1<<m68k_src_b);
  }
  else
  {
/*
Dn,<ea> :         |                 |             |               |
  .B :            |                 |             |               |
    (An)          |  8(1/1)  4(1/0) |             |            nr | np    nw
    (An)+         |  8(1/1)  4(1/0) |             |            nr | np    nw
    -(An)         |  8(1/1)  6(1/0) |             | n          nr | np    nw
    (d16,An)      |  8(1/1)  8(2/0) |             |      np    nr | np    nw
    (d8,An,Xn)    |  8(1/1) 10(2/0) |             | n    np    nr | np    nw
    (xxx).W       |  8(1/1)  8(2/0) |             |      np    nr | np    nw
    (xxx).L       |  8(1/1) 12(3/0) |             |   np np    nr | np    nw
*/
    m68k_GET_DEST_B_NOT_A; // EA
    TRUE_PC=pc+2;
    CHECK_IPL;
    PREFETCH_FINAL; //np
    resultb=m68k_dst_b^(signed char)(1<<(7&Cpu.r[PARAM_N]));
    PSWZ=(((m68k_dst_b>>(7&Cpu.r[PARAM_N]))&1)==0);
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_bset() {
  m68k_src_b=(BYTE)IRC;
  PREFETCH; //np
  if(DEST_IS_DATA_REGISTER)
  {
/*
#<data>,Dn :      |                 |             |               |
  .L :            |                 |             |               |
    if data<16    | 10(2/0)  0(0/0) |          np |               | np       n  
    if data>15    | 12(2/0)  0(0/0) |          np |               | np       nn 
*/
    m68k_dst_l=REGL(PARAM_M);
    CHECK_IPL;
    PREFETCH_FINAL; //np
    m68k_src_b&=31;
    if(m68k_src_b>15)
      CPU_BUS_IDLE(4); //nn
    else
      CPU_BUS_IDLE(2); //n
    m68k_src_l=1<<m68k_src_b;
    result=m68k_dst_l|m68k_src_l;
    PSWZ=((m68k_dst_l&m68k_src_l)==0);
    REGL(PARAM_M)=result;
  }
  else
  {
/*
#<data>,<ea> :    |                 |             |               |
  .B :            |                 |             |               |
    (An)          | 12(2/1)  4(1/0) |          np |            nr | np    nw    
    (An)+         | 12(2/1)  4(1/0) |          np |            nr | np    nw    
    -(An)         | 12(2/1)  6(1/0) |          np | n          nr | np    nw    
    (d16,An)      | 12(2/1)  8(2/0) |          np |      np    nr | np    nw    
    (d8,An,Xn)    | 12(2/1) 10(2/0) |          np | n    np    nr | np    nw    
    (xxx).W       | 12(2/1)  8(2/0) |          np |      np    nr | np    nw    
    (xxx).L       | 12(2/1) 12(3/0) |          np |   np np    nr | np    nw    
*/
    m68k_src_b&=7;
    m68k_GET_DEST_B_NOT_A; // EA
    TRUE_PC=pc+2;
    CHECK_IPL;
    PREFETCH_FINAL; //np
    m68k_src_b=(BYTE)(1<<m68k_src_b);
    resultb=m68k_dst_b|m68k_src_b;
    PSWZ=((m68k_dst_b&m68k_src_b)==0);
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_bset_from_dN() {
  if(DEST_IS_DATA_REGISTER)
  {
/*
Dn,Dm :           |                 |             |               |
  .L :            |                 |             |               |
    if Dn<16      |  6(1/0)  0(0/0) |             |               | np       n  
    if Dn>15      |  8(1/0)  0(0/0) |             |               | np       nn 
*/
    CHECK_IPL;
    PREFETCH_FINAL; //np
    m68k_src_b=(cpureg[PARAM_N].d8[B0]&31);
    if(m68k_src_b>15)
      CPU_BUS_IDLE(4); //nn
    else
      CPU_BUS_IDLE(2); //n
    PSWZ=(((Cpu.r[PARAM_M]>>(m68k_src_b))&1)==0);
    Cpu.r[PARAM_M]|=(1<<m68k_src_b);
  }
  else
  {
/*
Dn,<ea> :         |                 |             |               |
  .B :            |                 |             |               |             
    (An)          |  8(1/1)  4(1/0) |             |            nr | np    nw    
    (An)+         |  8(1/1)  4(1/0) |             |            nr | np    nw    
    -(An)         |  8(1/1)  6(1/0) |             | n          nr | np    nw    
    (d16,An)      |  8(1/1)  8(2/0) |             |      np    nr | np    nw    
    (d8,An,Xn)    |  8(1/1) 10(2/0) |             | n    np    nr | np    nw    
    (xxx).W       |  8(1/1)  8(2/0) |             |      np    nr | np    nw    
    (xxx).L       |  8(1/1) 12(3/0) |             |   np np    nr | np    nw    
*/
    m68k_GET_DEST_B_NOT_A; // EA
    TRUE_PC=pc+2;
    CHECK_IPL;
    PREFETCH_FINAL; //np
    resultb=m68k_dst_b|((signed char)(1<<(7&Cpu.r[PARAM_N])));
    PSWZ=(((m68k_dst_b>>(7&Cpu.r[PARAM_N]))&1)==0);
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
     BCLR         |  INSTR     EA   | 1st Operand |  2nd OP (ea)  |   INSTR
------------------+-----------------+-------------+---------------+------------
Dn,<ea> :         |                 |             |               |
  .B :            |                 |             |               |
    (An)          |  8(1/1)  4(1/0) |             |            nr | np    nw    
    (An)+         |  8(1/1)  4(1/0) |             |            nr | np    nw    
    -(An)         |  8(1/1)  6(1/0) |             | n          nr | np    nw    
    (d16,An)      |  8(1/1)  8(2/0) |             |      np    nr | np    nw    
    (d8,An,Xn)    |  8(1/1) 10(2/0) |             | n    np    nr | np    nw    
    (xxx).W       |  8(1/1)  8(2/0) |             |      np    nr | np    nw    
    (xxx).L       |  8(1/1) 12(3/0) |             |   np np    nr | np    nw    
Dn,Dm :           |                 |             |               |
  .L :            |                 |             |               |
    if Dn<16      |  8(1/0)  0(0/0) |             |               | np nn       
    if Dn>15      | 10(1/0)  0(0/0) |             |               | np nn n     
#<data>,<ea> :    |                 |             |               |
  .B :            |                 |             |               |
    (An)          | 12(2/1)  4(1/0) |          np |            nr | np    nw    
    (An)+         | 12(2/1)  4(1/0) |          np |            nr | np    nw    
    -(An)         | 12(2/1)  6(1/0) |          np | n          nr | np    nw    
    (d16,An)      | 12(2/1)  8(2/0) |          np |      np    nr | np    nw    
    (d8,An,Xn)    | 12(2/1) 10(2/0) |          np | n    np    nr | np    nw    
    (xxx).W       | 12(2/1)  8(2/0) |          np |      np    nr | np    nw    
    (xxx).L       | 12(2/1) 12(3/0) |          np |   np np    nr | np    nw    
#<data>,Dn :      |                 |             |               |
  .L :            |                 |             |               |
    if data<16    | 12(2/0)  0(0/0) |          np |               | np nn       
    if data>15    | 14(2/0)  0(0/0) |          np |               | np nn n     
*/

void m68k_bclr() {
  m68k_src_b=(BYTE)IRC;
  PREFETCH; //np
  if(DEST_IS_DATA_REGISTER)
  {
/*
#<data>,Dn :      |                 |             |               |
  .L :            |                 |             |               |
    if data<16    | 12(2/0)  0(0/0) |          np |               | np nn       
    if data>15    | 14(2/0)  0(0/0) |          np |               | np nn n     
*/
    m68k_dst_l=REGL(PARAM_M);
    CHECK_IPL;
    PREFETCH_FINAL; //np
    m68k_src_b&=31;
    if(m68k_src_b>15)
      CPU_BUS_IDLE(6); //nn n
    else
      CPU_BUS_IDLE(4); //nn
    m68k_src_l=1<<m68k_src_b;
    result=m68k_dst_l&(~m68k_src_l);
    PSWZ=((m68k_dst_l&m68k_src_l)==0);
    REGL(PARAM_M)=result;
  }
  else
  {
/*
#<data>,<ea> :    |                 |             |               |
  .B :            |                 |             |               |
    (An)          | 12(2/1)  4(1/0) |          np |            nr | np    nw    
    (An)+         | 12(2/1)  4(1/0) |          np |            nr | np    nw    
    -(An)         | 12(2/1)  6(1/0) |          np | n          nr | np    nw    
    (d16,An)      | 12(2/1)  8(2/0) |          np |      np    nr | np    nw    
    (d8,An,Xn)    | 12(2/1) 10(2/0) |          np | n    np    nr | np    nw    
    (xxx).W       | 12(2/1)  8(2/0) |          np |      np    nr | np    nw    
    (xxx).L       | 12(2/1) 12(3/0) |          np |   np np    nr | np    nw    
*/
    m68k_src_b&=7;
    m68k_GET_DEST_B_NOT_A; // EA
    TRUE_PC=pc+2;
    CHECK_IPL;
    PREFETCH_FINAL; //np
    m68k_src_b=(BYTE)(1<<m68k_src_b);
    resultb=m68k_dst_b&(~m68k_src_b);
    PSWZ=((m68k_dst_b&m68k_src_b)==0);
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_bclr_from_dN() {
  if(DEST_IS_DATA_REGISTER)
  {
/*
Dn,Dm :           |                 |             |               |
  .L :            |                 |             |               |
    if Dn<16      |  8(1/0)  0(0/0) |             |               | np nn       
    if Dn>15      | 10(1/0)  0(0/0) |             |               | np nn n     
*/
    CHECK_IPL;
    PREFETCH_FINAL; //np
    m68k_src_b=(cpureg[PARAM_N].d8[B0]&31);
    if(m68k_src_b>=16)
      CPU_BUS_IDLE(6); //nn n
    else
      CPU_BUS_IDLE(4); //nn
    PSWZ=(((Cpu.r[PARAM_M]>>(m68k_src_b))&1)==0);
    Cpu.r[PARAM_M]&=(long)~((long)(1<<m68k_src_b)); //length = .l
  }
  else
  {
/*
Dn,<ea> :         |                 |             |               |
  .B :            |                 |             |               |
    (An)          |  8(1/1)  4(1/0) |             |            nr | np    nw    
    (An)+         |  8(1/1)  4(1/0) |             |            nr | np    nw    
    -(An)         |  8(1/1)  6(1/0) |             | n          nr | np    nw    
    (d16,An)      |  8(1/1)  8(2/0) |             |      np    nr | np    nw    
    (d8,An,Xn)    |  8(1/1) 10(2/0) |             | n    np    nr | np    nw    
    (xxx).W       |  8(1/1)  8(2/0) |             |      np    nr | np    nw    
    (xxx).L       |  8(1/1) 12(3/0) |             |   np np    nr | np    nw    
*/
    m68k_GET_DEST_B_NOT_A; // EA
    TRUE_PC=pc+2;
    CHECK_IPL;
    PREFETCH_FINAL; //np
    resultb=m68k_dst_b & ((signed char)~(1<<(7&Cpu.r[PARAM_N])));
    PSWZ=(((m68k_dst_b>>(7&Cpu.r[PARAM_N]))&1)==0);
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       CMPI       |  INSTR     EA   | 1st Operand |  2nd OP (ea)  |   INSTR
------------------+-----------------+-------------+-------------+--------------
#<data>,<ea> :    |                 |             |               |
  .B or .W :      |                 |             |               |
    Dn            |  8(2/0)  0(0/0) |          np |               | np          
    (An)          |  8(2/0)  4(1/0) |          np |            nr | np          
    (An)+         |  8(2/0)  4(1/0) |          np |            nr | np          
    -(An)         |  8(2/0)  6(1/0) |          np | n          nr | np          
    (d16,An)      |  8(2/0)  8(2/0) |          np |      np    nr | np          
    (d8,An,Xn)    |  8(2/0) 10(2/0) |          np | n    np    nr | np          
    (xxx).W       |  8(2/0)  8(2/0) |          np |      np    nr | np          
    (xxx).L       |  8(2/0) 12(3/0) |          np |   np np    nr | np          
*/

void m68k_cmpi_b() {
  m68k_src_b=(BYTE)IRC;
  PREFETCH; //np
  m68k_dst_b=m68k_read_dest_b(); //EA
  resultb=m68k_dst_b-m68k_src_b;
  SR_SUB_B(0);
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_cmpi_w() {
  m68k_src_w=IRC;
  PREFETCH; //np
  m68k_dst_w=m68k_read_dest_w(); //EA
  resultl=m68k_dst_w-m68k_src_w;
  SR_SUB_W(0);
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_cmpi_l() {
/*
  .L :            |                 |             |               |
    Dn            | 14(3/0)  0(0/0) |       np np |               | np       n  
    (An)          | 12(3/0)  8(2/0) |       np np |         nR nr | np          
    (An)+         | 12(3/0)  8(2/0) |       np np |         nR nr | np          
    -(An)         | 12(3/0) 10(2/0) |       np np | n       nR nr | np          
    (d16,An)      | 12(3/0) 12(3/0) |       np np |      np nR nr | np          
    (d8,An,Xn)    | 12(3/0) 14(3/0) |       np np | n    np nR nr | np          
    (xxx).W       | 12(3/0) 12(3/0) |       np np |      np nR nr | np          
    (xxx).L       | 12(3/0) 16(4/0) |       np np |   np np nR nr | np       
*/
  m68k_GET_IMMEDIATE_L_WITH_TIMING; //np np
  m68k_dst_l=m68k_read_dest_l(); //EA
  result=m68k_dst_l-m68k_src_l;
  SR_SUB_L(0);
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(DEST_IS_REGISTER)
    CPU_BUS_IDLE(2); //n
}


/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
      MOVEP       |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
(d16,Ay),Dx :     |                 |
  .W :            | 16(4/0)         |                np    nR    nr np          
*/

void m68k_movep_w_to_dN() {
  iabus=AREG(PARAM_M)+(signed short)IRC;
  PREFETCH; //np
  CPU_BUS_ACCESS_READ_B; //nR
  cpureg[PARAM_N].d8[HI]=d8;
  iabus+=2;
  CPU_BUS_ACCESS_READ_B; //nr
  cpureg[PARAM_N].d8[LO]=d8;
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_movep_l_to_dN() {
//  .L :            | 24(6/0)         |                np nR nR nr nr np          
  iabus=AREG(PARAM_M)+(signed short)IRC;
  PREFETCH; //np 
  CPU_BUS_ACCESS_READ_B; //nR
  cpureg[PARAM_N].d8[B3]=d8;
  iabus+=2;
  CPU_BUS_ACCESS_READ_B; //nR
  cpureg[PARAM_N].d8[B2]=d8;
  iabus+=2;
  CPU_BUS_ACCESS_READ_B; //nr
  cpureg[PARAM_N].d8[B1]=d8;
  iabus+=2;
  CPU_BUS_ACCESS_READ_B; //nr
  cpureg[PARAM_N].d8[B0]=d8;
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_movep_w_from_dN() {
/*
Dx,(d16,Ay) :     |                 |
  .W :            | 16(2/2)         |                np    nW    nw np          
*/
  iabus=AREG(PARAM_M)+(short)IRC;
  PREFETCH; //np 
  dbus=cpureg[PARAM_N].d8[HI];
  CPU_BUS_ACCESS_WRITE_B; //nW
  iabus+=2;
  dbus=cpureg[PARAM_N].d8[LO];
  CPU_BUS_ACCESS_WRITE_B; //nw
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_movep_l_from_dN() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
      MOVEP       |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
Dx,(d16,Ay) :     |                 |
  .L :            | 24(2/4)         |                np nW nW nw nw np          

NOTES :
  .Read and write operations are done from the MSB to the LSB on 2 words if 
  using ".w" (first read/write word at Ay+d16 then word at Ay+d16+2)and on 
  4 words if using ".l" (first read/write word at Ay+d16 then word at Ay+d16+2 
  then word at Ay+d16+4 and finally word at Ay+d16+6).
*/
  iabus=AREG(PARAM_M)+(signed short)IRC;
  PREFETCH; //np 
  dbus=cpureg[PARAM_N].d8[B3];
  CPU_BUS_ACCESS_WRITE_B; //nW
  iabus+=2;
  dbus=cpureg[PARAM_N].d8[B2];
  CPU_BUS_ACCESS_WRITE_B; //nW
  iabus+=2;
  dbus=cpureg[PARAM_N].d8[B1];
  CPU_BUS_ACCESS_WRITE_B; //nw
  iabus+=2;
  dbus=cpureg[PARAM_N].d8[B0];
  CPU_BUS_ACCESS_WRITE_B; //nw
  CHECK_IPL;
  PREFETCH_FINAL;  //np
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////        LINE-4 ROUTINES        //////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
-------------------------------------------------------------------------------
        CLR,      |    Exec Time    |               Data Bus Usage
  NEGX, NEG, NOT  |  INSTR     EA   |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
*/

void m68k_negx_b() {
/*
  .B or .W :      |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(2/0) |   np np    nr |               np nw       
*/
  m68k_GET_DEST_B_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=-m68k_dst_b;
  if(PSWX)
    resultb--;
  if(resultb)
    CLEAR_Z;
  PSWV=((m68k_dst_b&resultb&MSB_B)!=0);
  PSWX=PSWC=(((m68k_dst_b|resultb)&MSB_B)!=0);
  PSWN=(resultb<0);
  if(DEST_IS_DATA_REGISTER)
    REGB(PARAM_M)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_negx_w() {
/*
  .B or .W :      |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(2/0) |   np np    nr |               np nw       
*/
  m68k_GET_DEST_W_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=-m68k_dst_w;
  if(PSWX)
    resultl--;
  if(resultl)
    CLEAR_Z;
  PSWV=((m68k_dst_w&resultl&MSB_W)!=0);
  PSWX=PSWC=(((m68k_dst_w|resultl)&MSB_W)!=0);
  PSWN=(resultl<0);
  if(DEST_IS_DATA_REGISTER)
     REGW(PARAM_M)=resultl;
  else
  {
    dbus=resultl;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


void m68k_negx_l() {
/*
  .L :            |                 |               | 
    Dn            |  6(1/0)  0(0/0) |               |               np       n  
    (An)          | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    (An)+         | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    -(An)         | 12(1/2) 10(2/0) | n       nR nr |               np nw nW    
    (d16,An)      | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (d8,An,Xn)    | 12(1/2) 14(3/0) | n    np nR nr |               np nw nW    
    (xxx).W       | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (xxx).L       | 12(1/2) 16(4/0) |   np np nR nr |               np nw nW    
*/
  m68k_GET_DEST_L_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  result=-m68k_dst_l;
  if(PSWX)
    result--;
  if(result)
    CLEAR_Z;
  PSWV=((m68k_dst_l&result&MSB_L)!=0);
  PSWX=PSWC=(((m68k_dst_l|result)&MSB_L)!=0);
  PSWN=(result<0);
  if(DEST_IS_DATA_REGISTER)
  {
    CPU_BUS_IDLE(2); //n
    REGL(PARAM_M)=result;
  }
  else
    WRITE_LONG_BACKWARDS; //nw nW
}


void m68k_clr_b() {
/*
<ea> :            |                 |               | 
  .B or .W :      |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(2/0) |   np np    nr |               np nw       
*/
  m68k_GET_DEST_B_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=0;
  CLEAR_NVC_SET_Z;
  if(DEST_IS_DATA_REGISTER)
    REGB(PARAM_M)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_clr_w() {
/*
<ea> :            |                 |               | 
  .B or .W :      |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(2/0) |   np np    nr |               np nw       
*/
  m68k_GET_DEST_W_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=0;
  CLEAR_NVC_SET_Z;
  if(DEST_IS_DATA_REGISTER)
     REGW(PARAM_M)=resultl;
  else
  {
    dbus=(WORD)result;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


void m68k_clr_l() {
/*
  .L :            |                 |               | 
    Dn            |  6(1/0)  0(0/0) |               |               np       n  
    (An)          | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    (An)+         | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    -(An)         | 12(1/2) 10(2/0) | n       nR nr |               np nw nW    
    (d16,An)      | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (d8,An,Xn)    | 12(1/2) 14(3/0) | n    np nR nr |               np nw nW    
    (xxx).W       | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (xxx).L       | 12(1/2) 16(4/0) |   np np nR nr |               np nw nW    
*/
  m68k_GET_DEST_L_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  result=0;
  CLEAR_NVC_SET_Z;
  if(DEST_IS_DATA_REGISTER)
  {
    CPU_BUS_IDLE(2); //n
    REGL(PARAM_M)=result;
  }
  else
    WRITE_LONG_BACKWARDS; //nw nW
}


void m68k_neg_b() {
/*
  .B or .W :      |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(2/0) |   np np    nr |               np nw       
*/
  m68k_GET_DEST_B_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=-m68k_dst_b;
  PSWV=((m68k_dst_b&resultb&MSB_B)!=0);
  PSWX=PSWC=(((m68k_dst_b|resultb)&MSB_B)!=0);
  SR_CHECK_Z_AND_N_B;
  if(DEST_IS_DATA_REGISTER)
    REGB(PARAM_M)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_neg_w() {
/*
  .B or .W :      |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(2/0) |   np np    nr |               np nw       
*/
  m68k_GET_DEST_W_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=-m68k_dst_w;
  PSWV=((m68k_dst_w&resultl&MSB_W)!=0);
  PSWX=PSWC=(((m68k_dst_w|resultl)&MSB_W)!=0);
  SR_CHECK_Z_AND_N_W;
  if(DEST_IS_DATA_REGISTER)
     REGW(PARAM_M)=resultl;
  else
  {
    dbus=resultl;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


void m68k_neg_l() {
/*
  .L :            |                 |               | 
    Dn            |  6(1/0)  0(0/0) |               |               np       n  
    (An)          | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    (An)+         | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    -(An)         | 12(1/2) 10(2/0) | n       nR nr |               np nw nW    
    (d16,An)      | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (d8,An,Xn)    | 12(1/2) 14(3/0) | n    np nR nr |               np nw nW    
    (xxx).W       | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (xxx).L       | 12(1/2) 16(4/0) |   np np nR nr |               np nw nW    
*/
  m68k_GET_DEST_L_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  result=-m68k_dst_l;
  PSWV=((m68k_dst_l&result&MSB_L)!=0);
  PSWX=PSWC=(((m68k_dst_l|result)&MSB_L)!=0);
  SR_CHECK_Z_AND_N_L;
  if(DEST_IS_DATA_REGISTER)
  {
    CPU_BUS_IDLE(2); //n
    REGL(PARAM_M)=result;
  }
  else
    WRITE_LONG_BACKWARDS; //nw nW
}


void m68k_not_b() {
/*
  .B or .W :      |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(2/0) |   np np    nr |               np nw       
*/
  m68k_GET_DEST_B_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=~m68k_dst_b;
  SR_CHECK_AND_B;
  if(DEST_IS_DATA_REGISTER)
    REGB(PARAM_M)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_not_w() {
/*
  .B or .W :      |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(2/0) |   np np    nr |               np nw       
*/
  m68k_GET_DEST_W_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=~m68k_dst_w;
  SR_CHECK_AND_W;
  if(DEST_IS_DATA_REGISTER)
    REGW(PARAM_M)=resultl;
  else
  {
    dbus=resultl;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


void m68k_not_l() {
/*
  .L :            |                 |               | 
    Dn            |  6(1/0)  0(0/0) |               |               np       n  
    (An)          | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    (An)+         | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    -(An)         | 12(1/2) 10(2/0) | n       nR nr |               np nw nW    
    (d16,An)      | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (d8,An,Xn)    | 12(1/2) 14(3/0) | n    np nR nr |               np nw nW    
    (xxx).W       | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (xxx).L       | 12(1/2) 16(4/0) |   np np nR nr |               np nw nW    
*/
  m68k_GET_DEST_L_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  result=~m68k_dst_l;
  SR_CHECK_AND_L;
  if(DEST_IS_DATA_REGISTER)
  {
    CPU_BUS_IDLE(2); //n
    REGL(PARAM_M)=result;
  }
  else
    WRITE_LONG_BACKWARDS; //nw nW
}


/*
-------------------------------------------------------------------------------
	                |     Exec Time   |               Data Bus Usage             
       TST        |  INSTR     EA   |  1st OP (ea)  |          INSTR           
------------------+-----------------+---------------+--------------------------
*/

void m68k_tst_b() {
/*
  .B or .W :      |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  4(1/0)  4(1/0) |            nr |               np          
    (An)+         |  4(1/0)  4(1/0) |            nr |               np          
    -(An)         |  4(1/0)  6(1/0) | n          nr |               np          
    (d16,An)      |  4(1/0)  8(2/0) |      np    nr |               np          
    (d8,An,Xn)    |  4(1/0) 10(2/0) | n    np    nr |               np          
    (xxx).W       |  4(1/0)  8(2/0) |      np    nr |               np          
    (xxx).L       |  4(1/0) 12(3/0) |   np np    nr |               np          
*/
  resultb=m68k_read_dest_b(); // EA
  SR_CHECK_AND_B;
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_tst_w() {
/*
  .B or .W :      |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  4(1/0)  4(1/0) |            nr |               np          
    (An)+         |  4(1/0)  4(1/0) |            nr |               np          
    -(An)         |  4(1/0)  6(1/0) | n          nr |               np          
    (d16,An)      |  4(1/0)  8(2/0) |      np    nr |               np          
    (d8,An,Xn)    |  4(1/0) 10(2/0) | n    np    nr |               np          
    (xxx).W       |  4(1/0)  8(2/0) |      np    nr |               np          
    (xxx).L       |  4(1/0) 12(3/0) |   np np    nr |               np          
*/
  resultl=m68k_read_dest_w(); // EA
  SR_CHECK_AND_W;
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_tst_l() {
/*
  .L              |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  4(1/0)  8(2/0) |         nR nr |               np          
    (An)+         |  4(1/0)  8(2/0) |         nR nr |               np          
    -(An)         |  4(1/0) 10(2/0) | n       nR nr |               np          
    (d16,An)      |  4(1/0) 12(3/0) |      np nR nr |               np          
    (d8,An,Xn)    |  4(1/0) 14(3/0) | n    np nR nr |               np          
    (xxx).W       |  4(1/0) 12(3/0) |      np nR nr |               np          
    (xxx).L       |  4(1/0) 16(4/0) |   np np nR nr |               np          
*/
  result=m68k_read_dest_l(); // EA
  SR_CHECK_AND_L;
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_tas() {
/*
-------------------------------------------------------------------------------
	                |    Exec Time    |               Data Bus Usage
       TAS        |  INSTR     EA   |  1st OP (ea)  |          INSTR            
------------------+-----------------+---------------+--------------------------
<ea> :            |                 |               |
  .B :            |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          | 10(1/1)  4(1/0) |            nr |          n nw np          
    (An)+         | 10(1/1)  4(1/0) |            nr |          n nw np          
    -(An)         | 10(1/1)  6(1/0) | n          nr |          n nw np          
    (d16,An)      | 10(1/1)  8(2/0) |      np    nr |          n nw np          
    (d8,An,Xn)    | 10(1/1) 10(2/0) | n    np    nr |          n nw np          
    (xxx).W       | 10(1/1)  8(1/0) |               |          n nw np          
    (xxx).L       | 10(1/1) 12(2/0) |               |          n nw np          
NOTES :
  .M68000UM is probably wrong with instruction timming when <ea> is different 
   from Dn. It reads "14(3/0)" but according to the same book the read-modify-
   write bus cycle used by this instruction is only 10 clock cycles long which 
   seems coherent with the microwords decomposition in USP4325121. Last, 
   evaluation on real hardware confirms the 10 cycles timing.
*/
  // no blitter during read-modify-write bus cycle
  // no disk DMA either but it's possible in Steem TODO
  BYTE save=Blitter.Request;
  Blitter.Request=0;
  m68k_GET_DEST_B_NOT_A; // EA
  TRUE_PC=pc+2;
  resultb=m68k_dst_b;
  SR_CHECK_AND_B; // before the write contrary to general case
  resultb|=MSB_B;
  if(DEST_IS_REGISTER)
  {
    REGB(PARAM_M)=resultb;
    Blitter.Request=save;
  }
  else
  {
    // not really correct, at low level it's a 6 cycle + WS bus access
    CPU_BUS_IDLE(2); //n
    dbus=resultb;
    Blitter.Request=save; // right after write is OK
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_move_from_sr() {
/*
-------------------------------------------------------------------------------
       MOVE       |    Exec Time    |               Data Bus Usage
     from SR      |  INSTR     EA   |  2nd Op (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
SR,<ea> :         |                 |               |
  .W :            |                 |               |
    Dn            |  6(1/0)  0(0/0) |               |               np       n  
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np nw       
*/
  m68k_GET_DEST_W_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  UPDATE_SR;
  if(DEST_IS_REGISTER)
  {
    CPU_BUS_IDLE(2); //n
    REGW(PARAM_M)=SR;
  }
  else
  {
    dbus=SR;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


/*
-------------------------------------------------------------------------------
       MOVE       |    Exec Time    |               Data Bus Usage
  to CCR, to SR   |  INSTR     EA   |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
*/

void m68k_move_to_ccr() {
/*
  .W :            |                 |               |
    Dn            | 12(1/0)  0(0/0) |               |         nn np np          
    (An)          | 12(1/0)  4(1/0) |            nr |         nn np np          
    (An)+         | 12(1/0)  4(1/0) |            nr |         nn np np          
    -(An)         | 12(1/0)  6(1/0) | n          nr |         nn np np          
    (d16,An)      | 12(1/0)  8(2/0) |      np    nr |         nn np np          
    (d8,An,Xn)    | 12(1/0) 10(2/0) | n    np    nr |         nn np np          
    (xxx).W       | 12(1/0)  8(2/0) |      np    nr |         nn np np          
    (xxx).L       | 12(1/0) 12(3/0) |   np np    nr |         nn np np          
    #<data>       | 12(1/0)  4(1/0) |      np       |         nn np np          
*/
  m68k_GET_SOURCE_W; // EA - word operation!
  UPDATE_SR; // upper byte
  CCR=(BYTE)m68k_src_w;
#ifndef SSE_LEAN_AND_MEAN
  SR&=SR_VALID_BITMASK;
#endif
  UPDATE_FLAGS;
  CPU_BUS_IDLE(4); //nn
  PREFETCH_ONLY; //np
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_move_to_sr() {
  if(SUPERFLAG)
  {
/*
  .W :            |                 |               |
    Dn            | 12(1/0)  0(0/0) |               |         nn np np          
    (An)          | 12(1/0)  4(1/0) |            nr |         nn np np          
    (An)+         | 12(1/0)  4(1/0) |            nr |         nn np np          
    -(An)         | 12(1/0)  6(1/0) | n          nr |         nn np np          
    (d16,An)      | 12(1/0)  8(2/0) |      np    nr |         nn np np          
    (d8,An,Xn)    | 12(1/0) 10(2/0) | n    np    nr |         nn np np          
    (xxx).W       | 12(1/0)  8(2/0) |      np    nr |         nn np np          
    (xxx).L       | 12(1/0) 12(3/0) |   np np    nr |         nn np np          
    #<data>       | 12(1/0)  4(1/0) |      np       |         nn np np    

    notice only 1 nn to update SR, there are 2 in the ORI etc instructions
    but we have 2 np here
*/
    m68k_GET_SOURCE_W; //EA
    CPU_BUS_IDLE(4); //nn
#ifdef DEBUG_BUILD
    UPDATE_SR;
    int debug_old_sr=SR;
#endif
    SR=m68k_src_w;
#ifndef SSE_LEAN_AND_MEAN
    SR&=SR_VALID_BITMASK;
#endif
    UPDATE_FLAGS;
    PREFETCH_ONLY; //np
    CHECK_IPL;
    PREFETCH_FINAL; //np
    DETECT_CHANGE_TO_USER_MODE;
    CHECK_STOP_ON_USER_CHANGE;
  }
  else
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
}


void m68k_nbcd() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       NBCD       |      INSTR      |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
<ea> :            |                 |               |
  .B :            |                 |               |
    Dn            |  6(1/0)  0(0/0) |               |               np       n  
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np nw       
*/
  m68k_GET_DEST_B_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  BYTE m=m68k_dst_b,n=0;
  if(m&0xff) 
    n=0xa0;
  if(m&0xf)
    n=0x9a;
  if(PSWX)
    n=0x99;
  resultb=n-m;
  PSWX=PSWC=(m!=0);
  if(resultb)
    CLEAR_Z;
  if(DEST_IS_REGISTER)
  {
    CPU_BUS_IDLE(2); //n
    REGB(PARAM_M)=resultb;
  }
  else 
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_swap() {
/*
-------------------------------------------------------------------------------
	                |    Exec Time    |               Data Bus Usage
       SWAP       |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
Dn :              |                 |
  .W :            |  4(1/0)         |                               np          
*/
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resulth=cpureg[PARAM_M].d16[LO]; // low->high
  resultl=cpureg[PARAM_M].d16[HI]; // high->low
  SR_CHECK_AND_L;
  Cpu.r[PARAM_M]=result;
}


void m68k_pea() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
        PEA       |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
<ea> :            |                 | 
  .L :            |                 |
    (An)          | 12(1/2)         |                               np nS ns    
    (d16,An)      | 16(2/2)         |                          np   np nS ns    
    (d8,An,Xn)    | 20(2/2)         |                        n np n np nS ns    
    (xxx).W       | 16(2/2)         |                               np nS ns np 
    (xxx).L       | 20(3/2)         |                          np   np nS ns np 
*/
  switch(IRD&BITS_543) {
  case BITS_543_010: // (An)
    effective_address=AREG(PARAM_M);
    CHECK_IPL;
    PREFETCH_FINAL; //np
    m68k_PUSH_L_WITH_TIMING(ueffective_address); //nS ns
    break;
  case BITS_543_101: // (d16,An)
    effective_address=AREG(PARAM_M)+(signed short)IRC;
    PREFETCH; //np 
    CHECK_IPL;
    PREFETCH_FINAL; //np
    m68k_PUSH_L_WITH_TIMING(ueffective_address); //nS ns
    break;
  case BITS_543_110: // (d8,An,Xn)
    CPU_BUS_IDLE(2); //n
    m68k_iriwo=IRC;
    PREFETCH; //np
    CPU_BUS_IDLE(2); //n
    if(m68k_iriwo & BIT_b)   //.l
      effective_address=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
      +(int)Cpu.r[m68k_iriwo>>12];
    else          //.w
      effective_address=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
      +(signed short)Cpu.r[m68k_iriwo>>12];
    CHECK_IPL;
    PREFETCH_FINAL; //np
    m68k_PUSH_L_WITH_TIMING(ueffective_address); //nS ns
    break;
  case BITS_543_111:
    switch(IRD&0x7) {
    case 0: // (xxx).W
      effective_address=(signed long)(signed short)IRC;
      PREFETCH; //np
      TRUE_PC+=2;
      m68k_PUSH_L_WITH_TIMING(ueffective_address); //nS ns
      CHECK_IPL;
      PREFETCH_FINAL; //np
      break;
    case 1: // (xxx).L
      effective_address_h=IRC;
      PREFETCH; //np
      effective_address_l=IRC;
      PREFETCH; //np
      TRUE_PC+=4;
      m68k_PUSH_L_WITH_TIMING(ueffective_address); //nS ns
      CHECK_IPL;
      PREFETCH_FINAL; //np
      break;
    case 2: // (d16,PC) 
      effective_address=(pc+(signed short)IRC);
      PREFETCH; //np
      CHECK_IPL;
      PREFETCH_FINAL; //np
      m68k_PUSH_L_WITH_TIMING(ueffective_address); //nS ns
      break;
    case 3: // (d8,PC,Xn)
      CPU_BUS_IDLE(2); //n
      m68k_iriwo=IRC;
      if(m68k_iriwo & BIT_b)  //.l
        effective_address=(pc+(signed char)LOBYTE(m68k_iriwo)
          +(int)Cpu.r[m68k_iriwo>>12]);
      else         //.w
        effective_address=(pc+(signed char)LOBYTE(m68k_iriwo)
          +(signed short)Cpu.r[m68k_iriwo>>12]);
      PREFETCH; //np
      CPU_BUS_IDLE(2); //n
      CHECK_IPL;
      PREFETCH_FINAL; //np
      m68k_PUSH_L_WITH_TIMING(ueffective_address); //nS ns
      break;
    }
    break;
  }//sw
}


void m68k_ext_w() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
        EXT       |      INSTR      |  1st Operand  |          INSTR
------------------+-----------------+---------------+--------------------------
Dn :              |                 |               |
  .W :            |  4(1/0)         |               |               np          
*/
  resultl=(signed short)REGB(PARAM_M);
  SR_CHECK_AND_W;
  cpureg[PARAM_M].d16[LO]=resultl;
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_movem_w_from_regs() {
  if((IRD&BITS_543)==BITS_543_100)
  { //MOVEM predecrement R->M -(An)
  //-(An)         |  8+4m(2/m)      |                np (nw)*       np    
    m68k_src_w=IRC; 
    PREFETCH; //np
    // The register written to memory should be the original one, so
    // predecrement afterwards.
    iabus=AREG(PARAM_M);
    short mask=1;
    for(int n=0;n<16;n++)
    {
      if(m68k_src_w & mask)
      {
        iabus-=2;
        dbus=REGW(15-n);
        CPU_BUS_ACCESS_WRITE; // (nw)*
      }
      mask<<=1;
    }
    AREG(PARAM_M)=iabus;
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
  else
  { // MOVEM other cases
/*
  .W              |                 | 
    (An)          |  8+4m(2/m)      |                np (nw)*       np          
    (d16,An)      | 12+4m(3/m)      |             np np (nw)*       np          
    (d8,An,Xn)    | 14+4m(3/m)      |          np n  np (nw)*       np          
    (xxx).W       | 12+4m(3/m)      |             np np (nw)*       np          
    (xxx).L       | 16+4m(4/m)      |          np np np (nw)*       np        
*/
    m68k_src_w=IRC;  // registers
    PREFETCH; //np
    switch(IRD&BITS_543) {
    case BITS_543_010: // (An)
      iabus=AREG(PARAM_M);
      break;
    case BITS_543_101: // (d16,An)
      iabus=AREG(PARAM_M)+(signed short)IRC;
      PREFETCH; //np
      break;
    case BITS_543_110: 
      // (d8,An,Xn)    | 14+4m(3/m)      |          np n  np (nw)*       np
      m68k_iriwo=IRC; 
      CPU_BUS_IDLE(2); //n
      PREFETCH; //np
      if(m68k_iriwo&BIT_b)  //.l
        iabus=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
        +(int)Cpu.r[m68k_iriwo>>12];
      else         //.w
        iabus=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
        +(signed short)Cpu.r[m68k_iriwo>>12];
      break;
    case BITS_543_111:
      switch(IRD&0x7) {
      case 0:
        iabus=(unsigned long)((signed long)((signed short)IRC));
        PREFETCH; //np
        break;
      case 1:
        iabush=IRC;
        PREFETCH; //np
        iabusl=IRC;
        PREFETCH; //np
        break;
      }
      break;
    }
    short mask=1;
    for(int n=0;n<16;n++)
    {
      if(m68k_src_w & mask)
      {
        dbus=REGW(n);
        CPU_BUS_ACCESS_WRITE; // (nw)*
        iabus+=2;
      }
      mask<<=1;
    }
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_ext_l() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
        EXT       |      INSTR      |  1st Operand  |          INSTR
------------------+-----------------+---------------+--------------------------
Dn :              |                 |               |
  .W :            |  4(1/0)         |               |               np          
  .L :            |  4(1/0)         |               |               np          

*/
  result=(signed long)REGW(PARAM_M);
  SR_CHECK_AND_L;
  Cpu.r[PARAM_M]=result;
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_movem_l_from_regs() {
  if((IRD&BITS_543)==BITS_543_100) 
  {    // MOVEM.L -(An)
/*

-------------------------------------------------------------------------------
	                |    Exec Time    |               Data Bus Usage
      MOVEM       |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
R --> M           |                 | 
  .L              |                 |                             
    -(An)         |  8+8m(2/2m)     |                np (nw nW)*    np   
*/
    m68k_src_w=IRC;
    PREFETCH; //np
    // The register written to memory should be the original one, so
    // predecrement afterwards.
    iabus=AREG(PARAM_M);
    TRUE_PC=pc+2;
    short mask=1;
    for(int n=0;n<16;n++)
    {
      if(m68k_src_w & mask)
      {
        iabus-=2;
        dbus=cpureg[15-n].d16[LO];
        CPU_BUS_ACCESS_WRITE; // (nw)*
        iabus-=2;
        dbus=cpureg[15-n].d16[HI];
        CPU_BUS_ACCESS_WRITE; // (nW)*
      }
      mask<<=1;
    }
    AREG(PARAM_M)=iabus;
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
  else
  { // MOVEM.L to memory other cases
/*
R --> M           |                 | 
  .L              |                 |                             
    (An)          |  8+8m(2/2m)     |                np (nW nw)*    np          
    (d16,An)      | 12+8m(3/2m)     |             np np (nW nw)*    np          
    (d8,An,Xn)    | 14+8m(3/2m)     |        n    np np (nW nw)*    np          
    (xxx).W       | 12+8m(3/2m)     |             np np (nW nw)*    np          
    (xxx).L       | 16+8m(4/2m)     |          np np np (nW nw)*    np      
*/
    m68k_src_w=IRC; // register mask - prefetch may be delayed
    switch(IRD&BITS_543) {
    case BITS_543_010: 
      // (An)          |  8+8m(2/2m)     |                np (nW nw)*    np
      PREFETCH; //np
      iabus=AREG(PARAM_M);
      break;
    case BITS_543_101: 
      // (d16,An)      | 12+8m(3/2m)     |             np np (nW nw)*    np     
      PREFETCH; //np
      iabus=AREG(PARAM_M)+(signed short)IRC;
      PREFETCH; //np
      break;
    case BITS_543_110: 
      // (d8,An,Xn)    | 14+8m(3/2m)     |        n    np np (nW nw)*    np 
      CPU_BUS_IDLE(2); //n 
      PREFETCH; //np
      m68k_ap=IRC; 
      if(m68k_ap&BIT_b)  //.l
        iabus=AREG(PARAM_M)+(signed char)LOBYTE(m68k_ap)
        +(int)Cpu.r[m68k_ap>>12];
      else         //.w
        iabus=AREG(PARAM_M)+(signed char)LOBYTE(m68k_ap)
        +(signed short)Cpu.r[m68k_ap>>12];
      PREFETCH; //np
      break;
    case BITS_543_111:
      switch(IRD&0x7) {
      case 0:
        //(xxx).W       | 12+8m(3/2m)     |             np np (nW nw)*    np
        PREFETCH; //np
        iabus=(unsigned long)((signed long)((signed short)IRC));
        PREFETCH; //np
        break;
      case 1:
        //(xxx).L       | 16+8m(4/2m)     |          np np np (nW nw)*    np
        PREFETCH; //np
        iabush=IRC;
        PREFETCH; //np
        iabusl=IRC;
        PREFETCH; //np
        break;
      }
      break;
    }
    TRUE_PC=pc+2;
    short mask=1;
    for(int n=0;n<16;n++)
    {
      if(m68k_src_w&mask)
      {
        dbus=cpureg[n].d16[HI];
        CPU_BUS_ACCESS_WRITE; //(nW )*
        iabus+=2;
        dbus=cpureg[n].d16[LO];
        CPU_BUS_ACCESS_WRITE; //( nw)*
        iabus+=2;
      }
      mask<<=1;
    }//nxt
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }//movem regs->mem
}



void m68k_movem_l_to_regs() {
/*
  .L              |                 | 
    (An)          | 12+8m(3+2m/0)   |                np nR (nr nR)* np          
    (An)+         | 12+8m(3+2m/0)   |                np nR (nr nR)* np          
    (d16,An)      | 16+8m(4+2m/0)   |             np np nR (nr nR)* np          
    (d8,An,Xn)    | 18+8m(4+2m/0)   |          np n  np nR (nr nR)* np          
    (xxx).W       | 16+8m(4+2m/0)   |             np np nR (nr nR)* np          
    (xxx).L       | 20+8m(5+2m/0)   |          np np np nR (nr nR)* np          
*/
  bool postincrement=false;
  m68k_src_w=IRC; 
  PREFETCH; //np
  switch(IRD&BITS_543) {
  case BITS_543_011: //(An)+  //$4cdf
    CHECK_IPL; // see Motorola Application Note 1012, such exceptions are rare
    postincrement=true;
    //no break
  case BITS_543_010: //(An)
    iabus=AREG(PARAM_M);
    break;
  case BITS_543_101://(d16,An)
    iabus=AREG(PARAM_M)+(signed short)IRC;
    PREFETCH; //np
    break;
  case BITS_543_110:
    //(d8,An,Xn)    | 18+8m(4+2m/0)   |          np n  np nR (nr nR)* np
    CPU_BUS_IDLE(2); //n //390
    m68k_iriwo=IRC; 
    if(m68k_iriwo&BIT_b)  //.l
      iabus=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
      +(int)Cpu.r[m68k_iriwo>>12];
    else         //.w
      iabus=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
      +(signed short)Cpu.r[m68k_iriwo>>12];
    PREFETCH; //np
    break;
  case BITS_543_111:
    switch(IRD&0x7) {
    case 0:
      //(xxx).W       | 16+8m(4+2m/0)   |             np np nR (nr nR)* np          
      iabus=(unsigned long)((signed long)((signed short)IRC));
      PREFETCH; //np
      break;
    case 1:
      //(xxx).L       | 20+8m(5+2m/0)   |          np np np nR (nr nR)* np    
      iabush=IRC;
      PREFETCH; //np
      iabusl=IRC;
      PREFETCH; //np
      break;
    case 2:
      //(d16,An)      | 16+8m(4+2m/0)   |             np np nR (nr nR)* np          
      iabus=pc+(signed short)IRC;
      PREFETCH; //np
      break;
    case 3:
      //(d8,An,Xn)    | 18+8m(4+2m/0)   |          np n  np nR (nr nR)* np         
      CPU_BUS_IDLE(2); //n
      m68k_iriwo=IRC;
      if(m68k_iriwo&BIT_b)  //.l
        iabus=pc+(signed char)LOBYTE(m68k_iriwo)+(int)Cpu.r[m68k_iriwo>>12];
      else         //.w
        iabus=pc+(signed char)LOBYTE(m68k_iriwo)
        +(signed short)Cpu.r[m68k_iriwo>>12];
      PREFETCH; //np
      break;
    }
    break;
  }
  CPU_BUS_ACCESS_READ; //nR - extra word read (discarded)
  TRUE_PC=pc+2;
  short mask=1;
  for(int n=0;n<16;n++)
  {
    if(m68k_src_w & mask)
    {
      iabus+=2;
      CPU_BUS_ACCESS_READ; //nr
      cpureg[n].d16[LO]=dbus;
      iabus-=2;
      CPU_BUS_ACCESS_READ; //nR
      cpureg[n].d16[HI]=dbus;
      iabus+=4;
    }
    mask<<=1;
  }
  if(postincrement)
    AREG(PARAM_M)=iabus;
  else
    CHECK_IPL;
  PREFETCH_FINAL;
}


void  m68k_movem_w_to_regs() {
/*
  .W              |                 | 
    (An)          | 12+4m(3+m/0)    |                np (nr)*    nr np          
    (An)+         | 12+4m(3+m/0)    |                np (nr)*    nr np          
    (d16,An)      | 16+4m(4+m/0)    |             np np (nr)*    nr np          
    (d8,An,Xn)    | 18+4m(4+m/0)    |          np n  np (nr)*    nr np          
    (xxx).W       | 16+4m(4+m/0)    |             np np (nr)*    nr np          
    (xxx).L       | 20+4m(5+m/0)    |          np np np (nr)*    nr np  
*/
  bool postincrement=false;
  m68k_src_w=IRC; 
  PREFETCH; //np
  switch(IRD&BITS_543) {
  case BITS_543_011://(An)+         | 12+4m(3+m/0)    |                np (nr)*    nr np
    CHECK_IPL;
    postincrement=true;
    //no break
  case BITS_543_010://(An)          | 12+4m(3+m/0)    |                np (nr)*    nr np
    iabus=AREG(PARAM_M);
    break;
  case BITS_543_101://(d16,An)      | 16+4m(4+m/0)    |             np np (nr)*    nr np
    iabus=AREG(PARAM_M)+(signed short)IRC;
    PREFETCH; //np
    break;
  case BITS_543_110://(d8,An,Xn)    | 18+4m(4+m/0)    |          np n  np (nr)*    nr np 
    CPU_BUS_IDLE(2); //n //390
    m68k_iriwo=IRC; 
    if(m68k_iriwo&BIT_b)  //.l
      iabus=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
      +(int)Cpu.r[m68k_iriwo>>12];
    else         //.w
      iabus=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
      +(signed short)Cpu.r[m68k_iriwo>>12];
    PREFETCH; //np
    break;
  case BITS_543_111:
    switch(IRD&0x7) {
    case 0://(xxx).W       | 16+4m(4+m/0)    |             np np (nr)*    nr np
      iabus=(unsigned long)((signed long)((signed short)IRC));
      PREFETCH; //np
      break;
    case 1://(xxx).L       | 20+4m(5+m/0)    |          np np np (nr)*    nr np  
      iabush=IRC;
      PREFETCH; //np
      iabusl=IRC;
      PREFETCH; //np
      break;
    case 2://(d16,An)      | 16+4m(4+m/0)    |             np np (nr)*    nr np
      iabus=pc+(signed short)IRC;
      PREFETCH; //np
      break;
    case 3:
      CPU_BUS_IDLE(2); //n
      m68k_iriwo=IRC;
      if(m68k_iriwo&BIT_b)  //.l
        iabus=pc+(signed char)LOBYTE(m68k_iriwo)+(int)Cpu.r[m68k_iriwo>>12];
      else         //.w
        iabus=pc+(signed char)LOBYTE(m68k_iriwo)
        +(signed short)Cpu.r[m68k_iriwo>>12];
      PREFETCH; //np
      break;
    }
    break;
  }
  TRUE_PC=pc+2;
  short mask=1;
  for(int n=0;n<16;n++)
  {
    if(m68k_src_w & mask)
    {
      CPU_BUS_ACCESS_READ; //(nr)*
      Cpu.r[n]=(signed long)((signed short)dbus); // sign extension
      iabus+=2;
    }
    mask<<=1;
  }
  CPU_BUS_ACCESS_READ; //nr - extra word read (discarded)
  if(postincrement)
    AREG(PARAM_M)=iabus;
  else
    CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_jsr() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       JSR        |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------
<ea> :            |                 |                
    (An)          | 16(2/2)         |                      np nS ns np          
    (d16,An)      | 18(2/2)         |                 n    np nS ns np          
    (d8,An,Xn)    | 22(2/2)         |                 n nn np nS ns np          
    (xxx).W       | 18(2/2)         |                 n    np nS ns np          
    (xxx).L       | 20(3/2)         |                   np np nS ns np          
*/
  DU32 uoldpc; // because we use pc to fetch, not au
  MEM_ADDRESS &oldpc=uoldpc.d32;
  switch (IRD&BITS_543) {
  case BITS_543_010:
    // (An)          | 16(2/2)         |                      np nS ns np          
    effective_address=AREG(PARAM_M);
    oldpc=pc;
    NEW_PC(effective_address);
    PREFETCH_ONLY;
    break;
  case BITS_543_101:
    // (d16,An)      | 18(2/2)         |                 n    np nS ns np          
    CPU_BUS_IDLE(2); //n
    // we use IRC, compute PC and prefetch 1st word, we don't prefetch
    // what was after IRC
    effective_address=AREG(PARAM_M)+(signed short)IRC;
    oldpc=pc+2; // but stacked pc is updated to point at the correct return address
    NEW_PC(effective_address);
    PREFETCH_ONLY;
    break;
  case BITS_543_110:
    // (d8,An,Xn)    | 22(2/2)         |                 n nn np nS ns np          
    CPU_BUS_IDLE(6); //n nn
    m68k_iriwo=IRC;
    if(m68k_iriwo & BIT_b)  //.l
      effective_address=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
      +(int)Cpu.r[m68k_iriwo>>12];
    else         //.w
      effective_address=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
      +(signed short)Cpu.r[m68k_iriwo>>12];
    oldpc=pc+2;
    NEW_PC(effective_address);
    PREFETCH_ONLY;
    break;
  case BITS_543_111:
    switch(IRD&0x7) {
    case 0:
      // (xxx).W       | 18(2/2)         |                 n    np nS ns np          
      CPU_BUS_IDLE(2); //n
      effective_address=(signed long)(signed short)IRC;
      oldpc=pc+2;
      NEW_PC(effective_address);
      PREFETCH_ONLY;
      break;
    case 1:
      // (xxx).L       | 20(3/2)         |                   np np nS ns np     
      effective_address_h=IRC;
      PREFETCH; //np
      effective_address_l=IRC;
      oldpc=pc+2;
      NEW_PC(effective_address);
      PREFETCH_ONLY;
      break;
    case 2:
      // (d16,An)      | 18(2/2)         |                 n    np nS ns np
      CPU_BUS_IDLE(2); //n
      effective_address=(pc+(signed short)IRC);
      oldpc=pc+2;
      NEW_PC(effective_address);
      PREFETCH_ONLY;
      break;
    case 3:
      // (d8,An,Xn)    | 22(2/2)         |                 n nn np nS ns np      
      CPU_BUS_IDLE(6); //n nn
      m68k_iriwo=IRC;
      if(m68k_iriwo & BIT_b)  //.l
        effective_address=(pc+(signed char)LOBYTE(m68k_iriwo)
          +(int)Cpu.r[m68k_iriwo>>12]);
      else         //.w
        effective_address=(pc+(signed char)LOBYTE(m68k_iriwo)
          +(signed short)Cpu.r[m68k_iriwo>>12]);
      oldpc=pc+2;
      NEW_PC(effective_address);
      PREFETCH_ONLY;
      break;       //what do bits 8,9,a  of extra word do?  (not always 0)
    }
    break;
  }
  m68k_PUSH_L_WITH_TIMING(uoldpc); // nS ns
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  Debug.PseudoStackPush(uoldpc.d32);
#endif
  CHECK_IPL;
  PREFETCH_FINAL;
  intercept_os();
}


void m68k_jmp() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       JMP        |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------
<ea> :            |                 |                          
    (An)          |  8(2/0)         |                      np       np          
    (d16,An)      | 10(2/0)         |                 n    np       np          
    (d8,An,Xn)    | 14(2/0)         |                 n nn np       np          
    (xxx).W       | 10(2/0)         |                 n    np       np          
    (xxx).L       | 12(3/0)         |                   np np       np          
NOTES :
  .M68000UM is probably wrong with bus read accesses for JMP (d8,An,Xn),Dn 
   instruction. It reads "14(3/0)" but, according to USP4325121 and with a 
   little common sense, 2 bus read accesses are far enough.
*/
  switch(IRD&BITS_543) {
  case BITS_543_010:
    //(An)          |  8(2/0)         |                      np       np  
    effective_address=AREG(PARAM_M);
    break;
  case BITS_543_101:
    //(d16,An)      | 10(2/0)         |                 n    np       np 
    CPU_BUS_IDLE(2); //n
    effective_address=AREG(PARAM_M)+(signed short)IRC;
    break;
  case BITS_543_110:
    //(d8,An,Xn)    | 14(2/0)         |                 n nn np       np
    CPU_BUS_IDLE(6); //n nn
    m68k_iriwo=IRC;
    if(m68k_iriwo & BIT_b)  //.l
      effective_address=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
      +(int)Cpu.r[m68k_iriwo>>12];
    else         //.w
      effective_address=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
      +(signed short)Cpu.r[m68k_iriwo>>12];
    break;
  case BITS_543_111:
    switch(IRD&0x7) {
    case 0:
      //(xxx).W       | 10(2/0)         |                 n    np       np          
      CPU_BUS_IDLE(2); //n
      effective_address=(signed long)(signed short)IRC;
      break;
    case 1:
      //(xxx).L       | 12(3/0)         |                   np np       np          
      effective_address_h=IRC;
      PREFETCH; //np
      effective_address_l=IRC;
      break;
    case 2:
      //(d16,An)      | 10(2/0)         |                 n    np       np 
      CPU_BUS_IDLE(2); //n
      effective_address=(pc+(signed short)IRC);
      break;
    case 3:
      //(d8,An,Xn)    | 14(2/0)         |                 n nn np       np
      CPU_BUS_IDLE(6); //n nn
      m68k_iriwo=IRC;
      if(m68k_iriwo & BIT_b)  //.l
        effective_address=(pc+(signed char)LOBYTE(m68k_iriwo)
          +(int)Cpu.r[m68k_iriwo>>12]);
      else         //.w
        effective_address=(pc+(signed char)LOBYTE(m68k_iriwo)
          +(signed short)Cpu.r[m68k_iriwo>>12]);
      break;       //what do bits 8,9,a  of extra word do?  (not always 0)
    }
    break;
  }
  m68kSetPC(effective_address,true);
  intercept_os();
}


/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
        CHK       |  INSTR     EA   |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
no trap :         |                 |               |
  <ea>,Dn :       |                 |              /  
    .W :          |                 |             /  
      Dn          | 10(1/0)  0(0/0) |            |                  np nn n     
      (An)        | 10(1/0)  4(1/0) |         nr |                  np nn n     
      (An)+       | 10(1/0)  4(1/0) |         nr |                  np nn n     
      -(An)       | 10(1/0)  6(1/0) | n       nr |                  np nn n     
      (d16,An)    | 10(1/0)  8(2/0) |      np nr |                  np nn n     
      (d8,An,Xn)  | 10(1/0) 10(2/0) | n    np nr |                  np nn n     
      (xxx).W     | 10(1/0)  8(2/0) |      np nr |                  np nn n     
      (xxx).L     | 10(1/0) 12(2/0) |   np np nr |                  np nn n     
      #<data>     | 10(1/0)  4(1/0) |      np    |                  np nn n     
trap :            |                 |            |
  <ea>,Dn :       |                 |            |  
    .W :          |                 |            |
      Dn > Src :  |                 |            |
        Dn        | 38(5/3)  0(0/0) |            |np    nn ns nS ns np np np n np 
        (An)      | 38(5/3)  4(1/0) |         nr |np    nn ns nS ns np np np n np 
        (An)+     | 38(5/3)  4(1/0) |         nr |np    nn ns nS ns np np np n np 
        -(An)     | 38(5/3)  6(1/0) | n       nr |np    nn ns nS ns np np np n np 
        (d16,An)  | 38(5/3)  8(2/0) |      np nr |np    nn ns nS ns np np np n np 
        (d8,An,Xn)| 38(5/3) 10(2/0) | n    np nr |np    nn ns nS ns np np np n np 
        (xxx).W   | 38(5/3)  8(2/0) |      np nr |np    nn ns nS ns np np np n np 
        (xxx).L   | 38(5/3) 12(2/0) |   np np nr |np    nn ns nS ns np np np n np 
        #<data>   | 38(5/3)  4(1/0) |      np    |np    nn ns nS ns np np np n np 
      Dn <0 :     |                 |            |
        Dn        | 40(5/3)  0(0/0) |            |np n- nn ns nS ns np np np n np 
        (An)      | 40(5/3)  4(1/0) |         nr |np n- nn ns nS ns np np np n np 
        (An)+     | 40(5/3)  4(1/0) |         nr |np n- nn ns nS ns np np np n np 
        -(An)     | 40(5/3)  6(1/0) | n       nr |np n- nn ns nS ns np np np n np 
        (d16,An)  | 40(5/3)  8(2/0) |      np nr |np n- nn ns nS ns np np np n np 
        (d8,An,Xn)| 40(5/3) 10(2/0) | n    np nr |np n- nn ns nS ns np np np n np 
        (xxx).W   | 40(5/3)  8(2/0) |      np nr |np n- nn ns nS ns np np np n np 
        (xxx).L   | 40(5/3) 12(2/0) |   np np nr |np n- nn ns nS ns np np np n np 
        #<data>   | 40(5/3)  4(1/0) |      np    |np n- nn ns nS ns np np np n np 
*/

void m68k_chk() {
  m68k_GET_SOURCE_W; //EA
  CHECK_IPL;
  PREFETCH_FINAL; //np 
  if(Cpu.r[PARAM_N]&0x8000)
  {
#if defined(SSE_STATS)
    Stats.nException[6]++;
#endif
    Cpu.ProcessingState=TMC68000::EXCEPTION;
    CPU_BUS_IDLE(2);
    SET_N;
    CPU_BUS_IDLE(4);
    m68k_finish_exception(BOMBS_CHK*4); //ns nS ns nV nv np n np 
  }
  else if((signed short)cpureg[PARAM_N].d16[LO]>(signed short)m68k_src_w)
  {
#if defined(SSE_STATS)
    Stats.nException[6]++;
#endif
    Cpu.ProcessingState=TMC68000::EXCEPTION;
    CLEAR_N;
    CPU_BUS_IDLE(4);
    m68k_finish_exception(BOMBS_CHK*4); //ns nS ns nV nv np n np 
  }
  else // no trap
    CPU_BUS_IDLE(6); //nn n
}


void m68k_lea() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
        LEA       |      INSTR      |                  INSTR                   
------------------+-----------------+------------------------------------------
<ea>,An :         |                 |               
  .L :            |                 |               
    (An)          |  4(1/0)         |                               np          
    (d16,An)      |  8(2/0)         |                          np   np          
    (d8,An,Xn)    | 12(2/0)         |                        n np n np          
    (xxx).W       |  8(2/0)         |                          np   np          
    (xxx).L       | 12(3/0)         |                       np np   np          
*/
  switch(IRD&BITS_543) {
  case BITS_543_010: //(An)
    effective_address=AREG(PARAM_M);
    break;
  case BITS_543_101: //(d16,An)
    effective_address=AREG(PARAM_M)+(signed short)IRC;
    PREFETCH; //np
    break;
  case BITS_543_110: //(d8,An,Xn)
    CPU_BUS_IDLE(2); //n
    m68k_iriwo=IRC;
    PREFETCH; //np
    CPU_BUS_IDLE(2); //n
    if(m68k_iriwo & BIT_b)  //.l
      effective_address=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
      +(int)Cpu.r[m68k_iriwo>>12];
    else         //.w
      effective_address=AREG(PARAM_M)+(signed char)LOBYTE(m68k_iriwo)
      +(signed short)Cpu.r[m68k_iriwo>>12];
    break;
  case BITS_543_111:
    switch(IRD&0x7) {
    case 0: //(xxx).W
      effective_address=(signed long)(signed short)IRC; //m68k_fetchW();
      PREFETCH; //np
      TRUE_PC+=2;
      break;
    case 1: //(xxx).L
      effective_address_h=IRC; //=m68k_fetchW()<<16;
      PREFETCH; //np
      effective_address_l=IRC; //|=m68k_fetchW();
      PREFETCH; //np
      TRUE_PC+=4;
      break;
    case 2: //(d16,PC) 
      effective_address=(pc+(signed short)IRC);
      PREFETCH; //np
      break;
    case 3: //(d8,PC,Xn)
    {
      CPU_BUS_IDLE(2); //n
      m68k_iriwo=IRC; //m68k_fetchW();
      MEM_ADDRESS au=pc;
      PREFETCH; //np
      CPU_BUS_IDLE(2); //n
      if(m68k_iriwo & BIT_b)  //.l
        effective_address=(au+(signed char)LOBYTE(m68k_iriwo)
          +(int)Cpu.r[m68k_iriwo>>12]);
      else         //.w
        effective_address=(au+(signed char)LOBYTE(m68k_iriwo)
          +(signed short)Cpu.r[m68k_iriwo>>12]);
      break;       //what do bits 8,9,a  of extra word do?  (not always 0)
    }//case
    }//sw
    break;
  }//sw
  AREG(PARAM_N)=effective_address;
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_trap() {
  int trap=(IRD&0xf);
#if defined(SSE_STATS)
  Stats.nTrap[trap]++;
#endif
  MEM_ADDRESS VectorAd=0x80+(trap*4);
  MEM_ADDRESS Vector=LPEEK(VectorAd);
  switch(trap) {
  case 1: //GEMDOS
#if defined(SSE_STATS)
    Stats.nGemdos++;
#endif
    if(os_gemdos_vector==0)
      if(Vector>=rom_addr)
        os_gemdos_vector=Vector;
    break;
#if defined(SSE_STATS)
  case 2:
    switch(Cpu.r[0]) { // check magic number
    case 0x73:
      Stats.nVdi++;
      break;
    case 0xC8:
      Stats.nAes++;
      break;
    }//sw
    break;
#endif
  case 13: // BIOS
#if defined(SSE_STATS)
    Stats.nBios++;
#endif
    if(os_bios_vector==0)
      if(Vector>=rom_addr)
        os_bios_vector=Vector;
    break;
  case 14: // XBIOS
#if defined(SSE_STATS)
    Stats.nXbios++;
#endif
    if(os_xbios_vector==0)
      if(Vector>=rom_addr)
        os_xbios_vector=Vector;
    break;
  }//sw
  Cpu.ProcessingState=TMC68000::EXCEPTION;
  CPU_BUS_IDLE(4); // nn
  m68k_finish_exception(VectorAd); //ns nS ns nV nv np n np 
  intercept_os();
  debug_check_break_on_irq(BREAK_IRQ_TRAP_IDX);
}


void m68k_link() {
/*
Pushes the contents of the specified address register onto the stack. Then
loads the updated stack pointer into the address register. Finally, adds the
displacement value to the stack pointer.
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       LINK       |      INSTR      |  2nd Operand  |   INSTR
------------------+-----------------+---------------+--------------------------
An,#<data> :      |                 |
  .W :            | 16(2/2)         |                      np nS ns np          
*/
  m68k_src_w=IRC;
  PREFETCH; //np
  CHECK_IPL;
  m68k_PUSH_L_WITH_TIMING((DU32&)cpureg[8+PARAM_M]); // nS ns
  AREG(PARAM_M)=sp;
  sp+=(signed short)m68k_src_w;
  PREFETCH_FINAL; //np
}


void m68k_unlk() {
/*
-------------------------------------------------------------------------------
	                |    Exec Time    |               Data Bus Usage
      UNLNK       |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
An :              | 12(3/0)         |                         nU nu np          
*/
  iabus=sp=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nU
  m68k_src_lh=dbus;
  iabus+=2;
  CHECK_IPL;
  CPU_BUS_ACCESS_READ; //nu
  m68k_src_ll=dbus;
  PREFETCH_FINAL; //np
  sp+=4;
  AREG(PARAM_M)=m68k_src_l;
}


void m68k_move_to_usp() {
  if(SUPERFLAG) 
  {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
     MOVE USP     |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------
 An,USP :         |  4(1/0)         |                               np          
*/
    CHECK_IPL;
    PREFETCH_FINAL; //np
    other_sp=AREG(PARAM_M);
  }
  else
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
}


void m68k_move_from_usp() {
  if(SUPERFLAG) 
  {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
     MOVE USP     |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------
 USP,An :         |  4(1/0)         |                               np          
*/
    CHECK_IPL;
    PREFETCH_FINAL; //np
    AREG(PARAM_M)=other_sp;
  }
  else
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
}


void m68k_reset() {
  if(SUPERFLAG) 
  {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       RESET      |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------
                  | 132(1/0)        |                     nn (??-)* np         
NOTES :
  .(??)* is for 124 cycles when the CPU will asserts the /RSTO signal in order 
   to reset all external devices. Normally, the CPU won't use the bus during 
   these cycles.
  .RESET instruction has nothing to do with /RESET exception. 
*/
    ioaccess=0;
    //TRACE_OSD("reset");
    reset_peripherals(0);
    for(int i=0;i<(128/2);i++)
      CPU_BUS_IDLE(2); //n
    CHECK_IPL;
    PREFETCH_FINAL; //np
#ifdef DEBUG_BUILD
    if(stop_on_next_reset)
      stop_on_next_reset=2;
#endif
  }
  else
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
}


void m68k_nop() {
/*
-------------------------------------------------------------------------------
	                |    Exec Time    |               Data Bus Usage
       NOP        |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
                  |  4(1/0)         |                               np          
*/
  CHECK_IPL;
  PREFETCH_FINAL; //np
}


void m68k_stop() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       STOP       |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------
                  |  4(0/0)         |                               n          
*/
#ifndef DEBUG_BUILD 
  // Privilege is constantly checked and PC on the stack will be different if
  // the exception triggers after the first iteration (Audio Sculpture as 
  // discussed at atari-forum)
  if(Cpu.ProcessingState==Cpu.STOPPED)
  {
    pc-=2;
    PSWI=((SR&SR_IPL)>>8);
  }
  else
  {
    pc+=2;
#if defined(SSE_STATS)
    Stats.nStop++;
#endif
  }
  if(SUPERFLAG)
  {
    CHECK_IPL; // IPL is checked before SR is changed at 1st iteration
    CPU_BUS_IDLE(4);
    m68k_src_w=IRC; // we fetch nothing
    SR=m68k_src_w;
#ifndef SSE_LEAN_AND_MEAN
    SR&=SR_VALID_BITMASK;
#endif
    BYTE former_pswi=PSWI;
    UPDATE_FLAGS;
    if(Cpu.ProcessingState!=Cpu.STOPPED)
      PSWI=former_pswi; // little trick to avoid copying pswI all the time
    DETECT_CHANGE_TO_USER_MODE;
    Cpu.ProcessingState=Cpu.STOPPED;
  }
  else
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
#else
  // different handling of pc for Debugger
  if(Cpu.ProcessingState==Cpu.STOPPED)
    PSWI=((SR&SR_IPL)>>8);
#if defined(SSE_STATS)
  else
    Stats.nStop++;
#endif
  if(SUPERFLAG)
  {
    UPDATE_SR;
    CHECK_IPL;
    CPU_BUS_IDLE(4);
    m68k_src_w=IRC;
    int debug_old_sr=-1;
    if(Cpu.ProcessingState!=Cpu.STOPPED)
      debug_old_sr=SR;
    SR=m68k_src_w;
#ifndef SSE_LEAN_AND_MEAN
    SR&=SR_VALID_BITMASK;
#endif
    BYTE former_pswi=PSWI;
    UPDATE_FLAGS;
    if(Cpu.ProcessingState!=Cpu.STOPPED)
      PSWI=former_pswi; // trick
    Cpu.ProcessingState=Cpu.STOPPED;
    DETECT_CHANGE_TO_USER_MODE;
    if(debug_old_sr!=-1)
    {
      CHECK_STOP_ON_USER_CHANGE;
    }
    pc=old_pc; // for Debugger
  }
  else
  {
    if(Cpu.ProcessingState==Cpu.STOPPED)
      old_pc+=4; // internal trick for the stack
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
  }
#endif
}


void m68k_rte() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
     RTE, RTR     |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
                  | 20(5/0)         |                      nU nu nu np np
*/
  bool dont_intercept_os=false;
  if(SUPERFLAG) 
  {
#ifdef DEBUG_BUILD
    UPDATE_SR;
    int debug_old_sr = SR;
#endif
    iabus=sp+2;
    CPU_BUS_ACCESS_READ; //nU
    effective_address_h=dbus;
    iabus-=2;
    CPU_BUS_ACCESS_READ; //nu
    WORD sr0=dbus;
    iabus+=4;
    CPU_BUS_ACCESS_READ; //nu
    effective_address_l=dbus;
    SR=sr0;
#ifndef SSE_LEAN_AND_MEAN
    SR&=SR_VALID_BITMASK;
#endif
    UPDATE_FLAGS;
    sp=iabus+2;
    DETECT_CHANGE_TO_USER_MODE;
    m68kSetPC(effective_address,true); //np np
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
    Debug.PseudoStackPop(effective_address);
#endif
#ifdef ENABLE_LOGFILE
    LOG_TO(LOGSECTION_INTERRUPTS,Str("INTERRUPT: ")+HEXSl(old_pc,6)+" - RTE to "+HEXSl(pc,6)+" SR="+HEXSl(SR,4)+
                                  " at "+ABSOLUTE_CPU_TIME+" idepth="+interrupt_depth);
#endif
#ifdef DEBUG_BUILD
    if(stemdos_command==0x48)
    {
#define LOGSECTION LOGSECTION_STEMDOS
      TRACE_LOG("Malloc #%d returns %x\n",nMallocs,Cpu.r[0]);
      stemdos_command=0;
#undef LOGSECTION
    }
#endif
    if(on_rte) 
    {
      //TRACE("on rte %d\n",on_rte);
      if(on_rte_interrupt_depth==interrupt_depth) {  //is this the RTE we want?
        switch(on_rte) {
#ifdef DEBUG_BUILD
        case ON_RTE_STOP:
          if(runstate==RUNSTATE_RUNNING) {
            runstate=RUNSTATE_STOPPING;
            SET_WHY_STOP(HEXSl(old_pc,6)+": RTE");
          }
          on_rte=ON_RTE_RTE;
          break;
#endif
#ifndef DISABLE_STEMDOS
        case ON_RTE_STEMDOS:
          stemdos_rte();
          ioaccess&=~IOACCESS_INTERCEPT_OS;
          dont_intercept_os=true;
          break;
#endif
#ifndef NO_CRAZY_MONITOR
        case ON_RTE_LINE_A:
          on_rte=ON_RTE_RTE;
          SET_PC(on_rte_return_address);
          extended_monitor_hack();
          break;
        case ON_RTE_EMHACK:
          on_rte=ON_RTE_RTE;
          extended_monitor_hack();
          break;
#endif
        }
      }
    }
//    DBG_LOG(EasyStr("RTE - decreasing interrupt depth from ")+interrupt_depth+" to "+(interrupt_depth-1));
    interrupt_depth--;
    if(!dont_intercept_os)
      intercept_os();
    CHECK_STOP_ON_USER_CHANGE;
  }//supervisor
  else
    exception(BOMBS_PRIVILEGE_VIOLATION,EA_INST,0);
}


void m68k_rts() {
/*
-------------------------------------------------------------------------------
	                |    Exec Time    |               Data Bus Usage
       RTS        |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
                  | 16(4/0)         |                   nU nu    np np          
*/
  iabus= sp;
  CPU_BUS_ACCESS_READ; //nU
  effective_address_h=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; //nu
  effective_address_l=dbus;
  iabus+=2;
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  Debug.PseudoStackPop(effective_address);
#endif
  sp=iabus;
  m68kSetPC(effective_address,true);
  intercept_os();
#if defined(SSE_DEBUGGER)
  if(on_rte==ON_RTS_STOP)
  {
    if(runstate==RUNSTATE_RUNNING) {
      runstate=RUNSTATE_STOPPING;
      SET_WHY_STOP(HEXSl(old_pc,6)+": RTS");
    }
    on_rte=ON_RTE_RTE;
  }
#endif
}


void m68k_trapv() {
/*
-------------------------------------------------------------------------------
	          |    Exec Time    |               Data Bus Usage
      TRAPV       |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
no trap           |  4(1/0)         |                                 np          
   trap           | 34(5/3)         |          nn ns nS ns nV nv np n np          
*/
  if(PSWV) 
  {
#if defined(SSE_STATS)
    Stats.nException[7]++;
#endif
    Cpu.ProcessingState=TMC68000::EXCEPTION;
    CPU_BUS_IDLE(4); //nn
    m68k_finish_exception(BOMBS_TRAPV * 4); //ns nS ns nV nv np n np 
  }
  else
  {
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_rtr() {
/*
-------------------------------------------------------------------------------
	                |    Exec Time    |               Data Bus Usage
     RTE, RTR     |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
                  | 20(5/0)         |                      nU nu nu np np       
NOTES :
  .M68000UM is probably wrong with bus read accesses for RTR instruction. 
   It reads "20(2/0)" but, according to USP4325121 and with a little common 
   sense, 2 bus read accesses aren't enough for reading status register and 
   program counter values from the stack and prefetching.
*/
  iabus=sp+2;
  CPU_BUS_ACCESS_READ; //nU
  effective_address_h=dbus;
  iabus-=2;
  CPU_BUS_ACCESS_READ; //nu
  BYTE ccr0=dbusl;
  iabus+=4;
  CPU_BUS_ACCESS_READ; //nu
  effective_address_l=dbus;
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  Debug.PseudoStackPop(effective_address);
#endif
  UPDATE_SR; // upper byte
  CCR=ccr0;
#ifndef SSE_LEAN_AND_MEAN
  SR&=SR_VALID_BITMASK;
#endif
  UPDATE_FLAGS;
  sp=iabus+2;
  m68kSetPC(effective_address,true);
  intercept_os();
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////////////////  LINE 5 - ADDQ, SUBQ, SCC, DBCC     //////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
    ADDQ, SUBQ    |  INSTR     EA   |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
*/

void m68k_addq_b() {
/*
  .B              |                 |               |
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np nw       
*/
  m68k_src_b=(BYTE)PARAM_N;
  if(m68k_src_b==0)
    m68k_src_b=8;
  m68k_GET_DEST_B_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=m68k_dst_b+m68k_src_b;
  SR_ADD_B;
  if(DEST_IS_DATA_REGISTER)
     REGB(PARAM_M)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_addq_w() {
/*
  .W :            |                 |               |
    Dn            |  4(1/0)  0(0/0) |               |               np          
    An            |  8(1/0)  0(0/0) |               |               np       nn 
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np nw       
NOTES :
  .M68000UM is probably wrong with instruction timing for ADDQ.W #<data>,An 
   instruction. It reads "4(1/0)" but, according to USP4325121, this 
   instruction is based on the same microwords than ADDQ.L #<data>,Dn and 
   ADDQ.L #<data>,An that have a "8(3/0)" timing. That makes sense because of 
   the calculation of a 32 bit address on 16 bits ALU. In addition, evaluation 
   on real hardware confirms the 8 cycles timing and, last, it makes addq and 
   subbq coherent in term of timing.
*/
  m68k_src_w=(WORD)PARAM_N;
  if(m68k_src_w==0)
    m68k_src_w=8;
  if(DEST_IS_ADDRESS_REGISTER)
  { //addq.w to address register
    CHECK_IPL;
    PREFETCH_FINAL; //np
    CPU_BUS_IDLE(4); //nn
    AREG(PARAM_M)+=m68k_src_w;
  }
  else
  {
    m68k_GET_DEST_W_NOT_A; //EA
    TRUE_PC=pc+2;
    CHECK_IPL;
    PREFETCH_FINAL; //np
    resultl=m68k_dst_w+m68k_src_w;
    SR_ADD_W;
    if(DEST_IS_DATA_REGISTER)
      REGW(PARAM_M)=resultl;
    else
    {
      dbus=resultl;
      CPU_BUS_ACCESS_WRITE; //nw
    }
  }
}


void m68k_addq_l() {
/*
  .L :            |                 |               |
    Dn            |  8(1/0)  0(0/0) |               |               np       nn 
    An            |  8(1/0)  0(0/0) |               |               np       nn 
    (An)          | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    (An)+         | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    -(An)         | 12(1/2) 10(2/0) | n       nR nr |               np nw nW    
    (d16,An)      | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (d8,An,Xn)    | 12(1/2) 14(3/0) | n    np nR nr |               np nw nW    
    (xxx).W       | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (xxx).L       | 12(1/2) 16(4/0) |   np np nR nr |               np nw nW    
*/
  m68k_src_l=(LONG)PARAM_N;
  if(m68k_src_l==0)
    m68k_src_l=8;
  if(DEST_IS_ADDRESS_REGISTER)
  { //addq.l to address register
    CHECK_IPL;
    PREFETCH_FINAL; //np
    CPU_BUS_IDLE(4); //nn
    AREG(PARAM_M)+=m68k_src_l;
  }
  else
  {
    m68k_GET_DEST_L_NOT_A; // EA
    TRUE_PC=pc+2;
    CHECK_IPL;
    PREFETCH_FINAL; //np
    result=m68k_dst_l+m68k_src_l;
    SR_ADD_L;
    if(DEST_IS_DATA_REGISTER)
    {
      CPU_BUS_IDLE(4); //nn
      REGL(PARAM_M)=result;
    }
    else
      WRITE_LONG_BACKWARDS; //nw nW
  }
}


void m68k_subq_b() {
/*
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np nw       
*/
  m68k_src_b=(BYTE)PARAM_N;
  if(m68k_src_b==0)
    m68k_src_b=8;
  m68k_GET_DEST_B_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=m68k_dst_b-m68k_src_b;
  SR_SUB_B(SR_X);
  if(DEST_IS_DATA_REGISTER)
     REGB(PARAM_M)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_subq_w() {
/*
    Dn            |  4(1/0)  0(0/0) |               |               np          
    An            |  8(1/0)  0(0/0) |               |               np       nn 
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw       
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw       
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw       
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw       
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw       
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np nw       
*/
  m68k_src_w=PARAM_N;
  if(m68k_src_w==0)
    m68k_src_w=8;
  if(DEST_IS_ADDRESS_REGISTER)
  { //subq.w to address register
    CHECK_IPL;
    PREFETCH_FINAL; //np
    CPU_BUS_IDLE(4); //nn
    AREG(PARAM_M)-=m68k_src_w;
  }
  else
  {
    m68k_GET_DEST_W_NOT_A; // EA
    TRUE_PC=pc+2;
    CHECK_IPL;
    PREFETCH_FINAL; //np
    resultl=m68k_dst_w-m68k_src_w;
    SR_SUB_W(SR_X);
    if(DEST_IS_DATA_REGISTER)
      REGW(PARAM_M)=resultl;
    else
    {
      dbus=resultl;
      CPU_BUS_ACCESS_WRITE; //nw
    }
  }
}


void m68k_subq_l() {
/*
  .L :            |                 |               |
    Dn            |  8(1/0)  0(0/0) |               |               np       nn 
    An            |  8(1/0)  0(0/0) |               |               np       nn 
    (An)          | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    (An)+         | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    -(An)         | 12(1/2) 10(2/0) | n       nR nr |               np nw nW    
    (d16,An)      | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (d8,An,Xn)    | 12(1/2) 14(3/0) | n    np nR nr |               np nw nW    
    (xxx).W       | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (xxx).L       | 12(1/2) 16(4/0) |   np np nR nr |               np nw nW    
*/
  m68k_src_l=(LONG)PARAM_N;
  if(m68k_src_l==0)
    m68k_src_l=8;
  if(DEST_IS_ADDRESS_REGISTER)
  { //subq.l to address register
    CHECK_IPL;
    PREFETCH_FINAL; //np
    CPU_BUS_IDLE(4); //nn
    AREG(PARAM_M)-=m68k_src_l;
  }
  else
  {
    m68k_GET_DEST_L_NOT_A; //EA
    TRUE_PC=pc+2;
    CHECK_IPL;
    PREFETCH_FINAL; //np
    result=m68k_dst_l-m68k_src_l;
    SR_SUB_L(SR_X);
    if(DEST_IS_DATA_REGISTER)
    {
      CPU_BUS_IDLE(4); //nn
      REGL(PARAM_M) = result;
    }
    else
      WRITE_LONG_BACKWARDS; //nw nW
  }
}


/* Motorola
If Condition False 
Then (Dn - 1 -> Dn; If Dn  <> -1 Then PC + dn -> PC) 
*/

/* Yacht
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       DBcc       |      INSTR      |                   INSTR
------------------+-----------------+------------------------------------------
Dn,<label> :      |                 |                               
  branch taken    | 10(2/0)         |                      n np       np        
  branch not taken|                 |                                           
    cc true       | 12(2/0)         |                      n     n np np        
    counter exp   | 14(3/0)         |                      n np    np np        
NOTES :                                                                         
  .DBcc does not exist in USP4325121. Only does a primitive instruction DCNT    
   that didn't reach the production version of the Cpu CPU.          
   Nervertheless, looking at DCNT and Bcc micro/nanowords can lead to a         
   realistic idea of how DBcc should work. At least, this re-enginering of DBcc 
   matches the timing written in M68000UM for this instruction.                 
FLOWCHART :                                                                     
                   cc                  n                 /cc                    
                   +-------------------+-------------------+                    
                   n                             z         np       /z          
                   |                             +---------+---------+          
                   +---------------------------->np                  |          
                                                 |                   |          
                                                 np<-----------------+          
*/

// optimisation:
// for dbcc, scc, bcc, we multiply functions to minimise tests inside the instruction

void m68k_dbra() {
  CPU_BUS_IDLE(2); //n 
#if !defined(SSE_LEAN_AND_MEAN)
  if(!CC_T)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else
#endif
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dbf() {
  CPU_BUS_IDLE(2); //n 
#if !defined(SSE_LEAN_AND_MEAN)
  if(!CC_F)
#endif
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
#if !defined(SSE_LEAN_AND_MEAN)
  else
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
#endif
}


void m68k_dbhi() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_HI)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dbls() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_LS)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dbcc() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_CC)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dbcs() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_CS)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dbne() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_NE)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dbeq() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_EQ)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dbvc() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_VC)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dbvs() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_VS)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dbpl() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_PL)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dbmi() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_MI)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dbge() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_GE)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dblt() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_LT)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dbgt() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_GT)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_dble() {
  CPU_BUS_IDLE(2); //n 
  if(!CC_LE)
  { //  /cc
    cpureg[PARAM_M].d16[LO]--;
    if(cpureg[PARAM_M].d16[LO]!=-1)
    {
      // /z
      m68k_src_w=IRC;
      // counter not expired, branch taken
      // | 10(2/0)         |                      n np       np        
      MEM_ADDRESS new_pc=(pc+(signed short)m68k_src_w);
      m68kSetPC(new_pc,true); //np np
    }
    else
    {
      // z
      // counter expired, branch not taken
      // | 14(3/0)         |                      n np    np np     
      PREFETCH; //np // fetch useless operand
      PREFETCH_ONLY;
      CHECK_IPL;
      PREFETCH_FINAL; //np
    }
  }
  else 
  {
    //  cc
    //  | 12(2/0)         |                      n     n np np  
    CPU_BUS_IDLE(2); //n
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
        Scc       |  INSTR     EA   |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
<ea> :            |                 |               |
  .B :            |                 |               |
    cc false      |                 |               |
      Dn          |  4(1/0)  0(0/0) |               |               np          
      (An)        |  8(1/1)  4(1/0) |            nr |               np    nw    
      (An)+       |  8(1/1)  4(1/0) |            nr |               np    nw    
      -(An)       |  8(1/1)  6(1/0) | n          nr |               np    nw    
      (d16,An)    |  8(1/1)  8(2/0) |      np    nr |               np    nw    
      (d8,An,Xn)  |  8(1/1) 10(2/0) | n    np    nr |               np    nw    
      (xxx).W     |  8(1/1)  8(2/0) |      np    nr |               np    nw    
      (xxx).L     |  8(1/1) 12(3/0) |   np np    nr |               np    nw    
    cc true       |                 |               |
      Dn          |  6(1/0)  0(0/0) |               |               np       n  
      (An)        |  8(1/1)  4(1/0) |            nr |               np    nw    
      (An)+       |  8(1/1)  4(1/0) |            nr |               np    nw    
      -(An)       |  8(1/1)  6(1/0) | n          nr |               np    nw    
      (d16,An)    |  8(1/1)  8(2/0) |      np    nr |               np    nw    
      (d8,An,Xn)  |  8(1/1) 10(2/0) | n    np    nr |               np    nw    
      (xxx).W     |  8(1/1)  8(2/0) |      np    nr |               np    nw    
      (xxx).L     |  8(1/1) 12(3/0) |   np np    nr |               np    nw    
*/

void m68k_st() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=(BYTE)0xFF;
  if(DEST_IS_REGISTER)
  {
    CPU_BUS_IDLE(2); //n
    REGB(PARAM_M)=resultb;
  }
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_sf() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=0;
  if(DEST_IS_REGISTER)
    REGB(PARAM_M)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_shi() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_HI) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_sls() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_LS) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_scc() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_CC) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_scs() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_CS) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_sne() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_NE) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_seq() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_EQ) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_svc() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_VC) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_svs() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_VS) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_spl() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_PL) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_smi() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_MI) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_sge() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_GE) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_slt() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_LT) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_sgt() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_GT) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


void m68k_sle() {
  m68k_GET_DEST_B; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(CC_LE) //cc true
  {
    resultb=(BYTE)0xFF;
    if(DEST_IS_REGISTER)
    {
      CPU_BUS_IDLE(2); //n
      REGB(PARAM_M)=resultb;
    }
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
  else // cc false
  {
    resultb=0;
    if(DEST_IS_REGISTER)
      REGB(PARAM_M)=resultb;
    else
    {
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  Line 8 - or, div, sbcd   //////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
      AND, OR     |  INSTR     EA   |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
*/

void m68k_or_b_to_dN() {
/*
  .B or .W :      |                 |               |
    Dn            |  4(1/0)  0(0/0) |               |               np  
    (An)          |  4(1/0)  4(1/0) |            nr |               np	
    (An)+         |  4(1/0)  4(1/0) |            nr |               np		     
    -(An)         |  4(1/0)  6(1/0) | n          nr |               np		     
    (d16,An)      |  4(1/0)  8(2/0) |      np    nr |               np		     
    (d8,An,Xn)    |  4(1/0) 10(2/0) | n    np    nr |               np		     
    (xxx).W       |  4(1/0)  8(2/0) |      np    nr |               np		     
    (xxx).L       |  4(1/0) 12(3/0) |   np np    nr |               np		     
    #<data>       |  4(1/0)  4(1/0) |      np       |               np		    
*/
  m68k_GET_SOURCE_B_NOT_A; //EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  m68k_dst_b=REGB(PARAM_N);
  resultb=m68k_dst_b|m68k_src_b;
  SR_CHECK_AND_B;
  REGB(PARAM_N)=resultb;
}


void m68k_or_w_to_dN() {
/*
  .B or .W :      |                 |               |
    Dn            |  4(1/0)  0(0/0) |               |               np       
    (An)          |  4(1/0)  4(1/0) |            nr |               np		     
    (An)+         |  4(1/0)  4(1/0) |            nr |               np		     
    -(An)         |  4(1/0)  6(1/0) | n          nr |               np		     
    (d16,An)      |  4(1/0)  8(2/0) |      np    nr |               np		     
    (d8,An,Xn)    |  4(1/0) 10(2/0) | n    np    nr |               np		     
    (xxx).W       |  4(1/0)  8(2/0) |      np    nr |               np		     
    (xxx).L       |  4(1/0) 12(3/0) |   np np    nr |               np		     
    #<data>       |  4(1/0)  4(1/0) |      np       |               np		    
*/
  m68k_GET_SOURCE_W_NOT_A; //EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  m68k_dst_w=REGW(PARAM_N);
  resultl=m68k_dst_w|m68k_src_w;
  SR_CHECK_AND_W;
  REGW(PARAM_N)=resultl;
}


void m68k_or_l_to_dN() {
/*
  .L :            |                 |               |
    Dn            |  8(1/0)  0(0/0) |               |               np       nn
    (An)          |  6(1/0)  8(2/0) |         nR nr |               np       n	
    (An)+         |  6(1/0)  8(2/0) |         nR nr |               np       n	
    -(An)         |  6(1/0) 10(2/0) | n       nR nr |               np       n	
    (d16,An)      |  6(1/0) 12(3/0) |      np nR nr |               np       n	
    (d8,An,Xn)    |  6(1/0) 14(3/0) | n    np nR nr |               np       n	
    (xxx).W       |  6(1/0) 12(3/0) |      np nR nr |               np       n	
    (xxx).L       |  6(1/0) 16(4/0) |   np np nR nr |               np       n	
    #<data>       |  8(1/0)  8(2/0) |   np np       |               np       nn
*/
  m68k_GET_SOURCE_L_NOT_A; // EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  m68k_dst_l=REGL(PARAM_N);
  if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
    CPU_BUS_IDLE(4); //nn
  else 
    CPU_BUS_IDLE(2); //n
  result=m68k_dst_l|m68k_src_l;
  SR_CHECK_AND_L;
  REGL(PARAM_N)=result;  
}


void m68k_divu() {
  m68k_GET_SOURCE_W_NOT_A; // EA
  if(m68k_src_w==0)
  { // div by 0
#if defined(SSE_STATS)
    Stats.nException[5]++;
#endif
    Cpu.ProcessingState=TMC68000::EXCEPTION;
    CLEAR_VC;
    //Divide by Zero      | 38(4/3)+ |           nn nn    ns nS ns nV nv np n np
    CPU_BUS_IDLE(4); //nn
    CPU_BUS_IDLE(4); //nn
    m68k_finish_exception(BOMBS_DIVISION_BY_ZERO * 4); //ns nS ns nV nv np n np 
  }
  else
  {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       DIVU       |  INSTR     EA   |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
<ea>,Dn :        /                  |               |
  .W :          |                   |               |
    Dn          | 76+?(1/0)  0(0/0) |               | nn n [seq.] n nn np       
    (An)        | 76+?(1/0)  4(1/0) |            nr | nn n [seq.] n nn np       
    (An)+       | 76+?(1/0)  4(1/0) |            nr | nn n [seq.] n nn np       
    -(An)       | 76+?(1/0)  6(1/0) | n          nr | nn n [seq.] n nn np       
    (d16,An)    | 76+?(1/0)  8(2/0) |      np    nr | nn n [seq.] n nn np       
    (d8,An,Xn)  | 76+?(1/0) 10(2/0) | n    np    nr | nn n [seq.] n nn np       
    (xxx).W     | 76+?(1/0)  8(2/0) |      np    nr | nn n [seq.] n nn np       
    (xxx).L     | 76+?(1/0) 12(2/0) |   np np    nr | nn n [seq.] n nn np       
    #<data>     | 76+?(1/0)  8(2/0) |   np np       | nn n [seq.] n nn np       
NOTES :
  .Overflow always cost 10 cycles (n nn np).
  .[seq.] refers to 15 consecutive blocks to be chosen in the following 3 :
   (nn), (nnn-) or (nnn-n). 
   (see following flowchart for details).
  .Best case : 76 cycles (nn n [nn]*15 n nn np)
  .Worst case : 136 (140 ?) cycles.
FLOWCHART :
                                       n                                        
                                       n             Divide by zero             
                  +--------------------+--------------------+                   
  Overflow        n                                         n                   
      +-----------+----------+                              n                   
     np                      n<---------------------+-+-+   nw                  
                             n                      | | |   nw                  
               No more bits  |  pMSB=0       pMSB=1 | | |   nw                  
                       +-----+-----+-----------+    | | |   np                  
                       n    MSB=0  n- MSB=1    |    | | |   np                  
                       np      +---+---+       +----+ | |   np                  
                               |       n              | |   np                  
                               |       +--------------+ |                       
                               +------------------------+                       
  .for each iteration of the loop : shift dividend to the left by 1 bit then    
   substract divisor to the MSW of new dividend, discard after test if result   
   is negative keep it if positive.                                             
  .MSB = most significant bit : bit at the far left of the dividend             
  .pMSB = previous MSB : MSB used in the previous iteration of the loop                                                                                              
*/
    unsigned long q;
    unsigned long dividend = (unsigned long) (Cpu.r[PARAM_N]);
    unsigned short divisor = (unsigned short) m68k_src_w;
    // using ijor's timings (in 3rdparty\pasti)
    int cycles_for_instr=getDivu68kCycles(dividend,divisor)-4; // -prefetch
    for(int i=0;i<(cycles_for_instr>>1);i++)
      CPU_BUS_IDLE(2); //n
    q=(unsigned long)((unsigned long)dividend)
      /(unsigned long)((unsigned short)divisor);
    if(q&0xffff0000)
    {
      CLEAR_C;
      SET_NV;
    }
    else
    {
      CLEAR_VC;
      PSWN=((q&MSB_W)!=0);
      PSWZ=(q==0);
      Cpu.r[PARAM_N]=((((unsigned long)Cpu.r[PARAM_N])
        %((unsigned short)m68k_src_w))<<16)+q;
    }
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
    ABCD, SBCD    |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------ 
*/

void m68k_sbcd() {
  bool toreg=(DEST_IS_DATA_REGISTER);
  if(toreg)
  {
/*
Dy,Dx :           |                 |
  .B :            |  6(1/0)         |                               np       n
*/
    CHECK_IPL;
    PREFETCH_FINAL; //np
    CPU_BUS_IDLE(2); //n
    m68k_src_b=REGB(PARAM_M);
    m68k_dst_b=REGB(PARAM_N);
  }
  else
  {
/*
-(Ay),-(Ax) :     |                 |
  .B :            | 18(3/1)         |                 n    nr    nr np nw       
*/
    CPU_BUS_IDLE(2); //n
    AREG(PARAM_M)--;
    if(PARAM_M==7)
      AREG(PARAM_M)--;
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_src_b=d8;
    AREG(PARAM_N)--;
    if(PARAM_N==7)
      AREG(PARAM_N)--;
    iabus=AREG(PARAM_N);
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_dst_b=d8;
    TRUE_PC=pc+2;
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }//if
  // computing of result lifted from WinUAE
  BYTE src=m68k_src_b;
  BYTE dst=m68k_dst_b;
  BYTE lo_nibble=(dst&0xF)-(src&0xF);
  if(PSWX)
    lo_nibble--;
  CLEAR_XNC;
  if(lo_nibble&0xF0)
  {
    lo_nibble-=6;
    SET_C; //internal
  }
  WORD hi_nibble=(dst&0xF0)-(src&0xF0);
  if(PSWC)
    hi_nibble-=0x10;
  CLEAR_C; //clear internal bit
  if(hi_nibble&0xF00)
  {
    hi_nibble-=0x60;
    SET_XNC;
  }
  resultb=(hi_nibble&0xF0)+(lo_nibble&0xF);
  if(resultb)
    CLEAR_Z;
  if(toreg)
    //REGB(PARAM_M)=resultb; //argh! Ikari Warriors
    REGB(PARAM_N)=resultb; 
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_or_b_from_dN() {
/*
Dn,<ea> :         |                 |               | 
  .B or .W :      |                 |               | 
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw	      
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw	      
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw	      
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw	      
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw	      
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw	      
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np nw	      
*/
  m68k_src_b=REGB(PARAM_N);
  m68k_GET_DEST_B_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=m68k_dst_b|m68k_src_b;
  SR_CHECK_AND_B;
  dbus=resultb;
  CPU_BUS_ACCESS_WRITE_B; //nw
}


void m68k_or_w_from_dN() {
/*
Dn,<ea> :         |                 |               | 
  .B or .W :      |                 |               | 
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw	      
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw	      
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw	      
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw	      
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw	      
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw	      
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np nw	      
*/
  m68k_src_w=REGW(PARAM_N);
  m68k_GET_DEST_W_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=m68k_dst_w|m68k_src_w;
  SR_CHECK_AND_W;
  dbus=resultl;
  CPU_BUS_ACCESS_WRITE; //nw
}


void m68k_or_l_from_dN() {
/*
  .L :            |                 |                              
    (An)          | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    (An)+         | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    -(An)         | 12(1/2) 10(2/0) | n       nR nr |               np nw nW    
    (d16,An)      | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (d8,An,Xn)    | 12(1/2) 14(3/0) | n    np nR nr |               np nw nW    
    (xxx).W       | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (xxx).L       | 12(1/2) 16(4/0) |   np np nR nr |               np nw nW    
*/
  m68k_src_l=REGL(PARAM_N);
  m68k_GET_DEST_L_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  result=m68k_dst_l|m68k_src_l;
  SR_CHECK_AND_L;
  WRITE_LONG_BACKWARDS; //nw nW
}


void m68k_divs() {
  m68k_GET_SOURCE_W_NOT_A; //EA
  if(m68k_src_w==0)
  {
#if defined(SSE_STATS)
    Stats.nException[5]++;
#endif
    Cpu.ProcessingState=TMC68000::EXCEPTION;
    CLEAR_VC;
    //Divide by Zero      | 38(4/3)+ |           nn nn    ns nS ns nV nv np n np
    CPU_BUS_IDLE(4); //nn
    CPU_BUS_IDLE(4); //nn
    m68k_finish_exception(BOMBS_DIVISION_BY_ZERO * 4); //ns nS ns nV nv np n np 
  }
  else
  {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       DIVS       |  INSTR     EA   |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
<ea>,Dn :       /                  /            /
 .W :         /                  /          /
  Dn        |120+?(1/0)  0(0/0)|          | nn nnn (n)n [seq] nnn-nnn n(n(n))np 
  (An)      |120+?(1/0)  4(1/0)|        nr| nn nnn (n)n [seq] nnn-nnn n(n(n))np 
  (An)+     |120+?(1/0)  4(1/0)|        nr| nn nnn (n)n [seq] nnn-nnn n(n(n))np 
  -(An)     |120+?(1/0)  6(1/0)|n       nr| nn nnn (n)n [seq] nnn-nnn n(n(n))np 
  (d16,An)  |120+?(1/0)  8(2/0)|     np nr| nn nnn (n)n [seq] nnn-nnn n(n(n))np 
  (d8,An,Xn)|120+?(1/0) 10(2/0)|n    np nr| nn nnn (n)n [seq] nnn-nnn n(n(n))np 
  (xxx).W   |120+?(1/0)  8(2/0)|     np nr| nn nnn (n)n [seq] nnn-nnn n(n(n))np 
  (xxx).L   |120+?(1/0) 12(2/0)|  np np nr| nn nnn (n)n [seq] nnn-nnn n(n(n))np 
  #<data>   |120+?(1/0)  8(2/0)|  np np   | nn nnn (n)n [seq] nnn-nnn n(n(n))np 
NOTES :
  .Overflow cost 16 or 18 cycles depending on dividend sign (n nn nn (n)n np).
  .[seq.] refers to 15 consecutive blocks to be chosen in the following 2 :
   (nnn-n) or (nn-n).
   (see following flowchart for details).
  .Best case : 120-122 cycles depending on dividend sign.
  .Worst case : 156 (158 ?) cycles.
FLOWCHART :                                   
                                         n                                      
                                         n                       Divide by zero 
                                +--------+--------------------------------+     
                                n                                         n     
                                n                                         n     
                  Dividend<0    n    Dividend>0                           nw    
                          +-----+-----+                                   nw    
                          n           |                                   nw    
                          +---------->n                                   np    
                                      |    Overflow                       np    
        +-----------------+      +----+----+                              np    
        | +----------+    +----->n         np                             np    
        | |          +---------->n                                              
        | |                      n-                                             
        | | MSB=1      MSB=0     n   No more bits                               
        | | +-----------+--------+--------+                                     
        | +-+           |                 n                                     
        +---------------+   divisor<0     n        divisor>0                    
                               +----------+-----------+                         
                               n         dividend<0   n   dividend>0            
                               n           +----------+----+                    
                               np          n               np                   
                                           n                                    
                                           np                                   
  .for each iteration of the loop : shift quotient to the left by 1 bit.             
  .MSB = most significant bit : bit at the far left of the quotient.                  
*/
    signed long q;
    signed long dividend = (signed long) (Cpu.r[PARAM_N]);
    signed short divisor = (signed short) m68k_src_w;
    // using ijor's timings (in 3rdparty\pasti)
    int cycles_for_instr=getDivs68kCycles(dividend,divisor)-4; // -prefetch
    for(int i=0;i<(cycles_for_instr>>1);i++)
      CPU_BUS_IDLE(2); //n
    // signaled by Dio, X86 crashes on div overflow
#if defined(BCC_BUILD) || (defined(VC_BUILD) && _MSC_VER < 1500) || defined(MINGW_BUILD)
    if(dividend==(-2147483647 - 1))
      q=dividend; //-32768-1; 
    else
      q=(signed long)((signed long)dividend)/(signed long)((signed short)divisor);
#else
    if(dividend==INT_MIN)
      q=dividend; //-32768-1; 
    else
      q=(signed long)((signed long)dividend)/(signed long)((signed short)divisor);
#endif
    if(q<-32768 || q>32767)
    {
      CLEAR_C;
      SET_NV;
    }
    else
    {
      CLEAR_VC;
      PSWN=((q&MSB_W)!=0);
      PSWZ=(q==0);
      Cpu.r[PARAM_N]=((((signed long)Cpu.r[PARAM_N])
        %((signed short)m68k_src_w))<<16)|((long)LOWORD(q));
    }
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  line 9 - sub            ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void m68k_sub_b_to_dN() {
/*
<ea>,Dn :         |                 |              \              |
  .B or .W :      |                 |               |             |
    Dn            |  4(1/0)  0(0/0) |               |             | np          
    (An)          |  4(1/0)  4(1/0) |            nr |             | np          
    (An)+         |  4(1/0)  4(1/0) |            nr |             | np          
    -(An)         |  4(1/0)  6(1/0) | n          nr |             | np          
    (d16,An)      |  4(1/0)  8(2/0) |      np    nr |             | np          
    (d8,An,Xn)    |  4(1/0) 10(2/0) | n    np    nr |             | np          
    (xxx).W       |  4(1/0)  8(2/0) |      np    nr |             | np          
    (xxx).L       |  4(1/0) 12(3/0) |   np np    nr |             | np          
    #<data>       |  4(1/0)  4(1/0)        np                     | np          
*/
  m68k_GET_SOURCE_B_NOT_A; // EA
  CHECK_IPL;
  m68k_dst_b=REGB(PARAM_N);
  resultb=m68k_dst_b-m68k_src_b;
  SR_SUB_B(SR_X);
  REGB(PARAM_N)=resultb;
  PREFETCH_FINAL; //np
}


void m68k_sub_w_to_dN() {
/*
<ea>,Dn :         |                 |              \              |
  .B or .W :      |                 |               |             |
    Dn            |  4(1/0)  0(0/0) |               |             | np          
    An            |  4(1/0)  0(0/0) |               |             | np          
    (An)          |  4(1/0)  4(1/0) |            nr |             | np          
    (An)+         |  4(1/0)  4(1/0) |            nr |             | np          
    -(An)         |  4(1/0)  6(1/0) | n          nr |             | np          
    (d16,An)      |  4(1/0)  8(2/0) |      np    nr |             | np          
    (d8,An,Xn)    |  4(1/0) 10(2/0) | n    np    nr |             | np          
    (xxx).W       |  4(1/0)  8(2/0) |      np    nr |             | np          
    (xxx).L       |  4(1/0) 12(3/0) |   np np    nr |             | np          
    #<data>       |  4(1/0)  4(1/0)        np                     | np          
*/
  m68k_GET_SOURCE_W;   //EA - A is allowed
  CHECK_IPL;
  m68k_dst_w=REGW(PARAM_N);
  resultl=m68k_dst_w-m68k_src_w;
  SR_SUB_W(SR_X);
  REGW(PARAM_N)=resultl;
  PREFETCH_FINAL; //np
}


void m68k_sub_l_to_dN() {
/*
  .L :            |                 |               |             |
    Dn            |  8(1/0)  0(0/0) |               |             | np       nn 
    An            |  8(1/0)  0(0/0) |               |             | np       nn 
    (An)          |  6(1/0)  8(2/0) |         nR nr |             | np       n  
    (An)+         |  6(1/0)  8(2/0) |         nR nr |             | np       n  
    -(An)         |  6(1/0) 10(2/0) | n       nR nr |             | np       n  
    (d16,An)      |  6(1/0) 12(3/0) |      np nR nr |             | np       n  
    (d8,An,Xn)    |  6(1/0) 14(3/0) | n    np nR nr |             | np       n  
    (xxx).W       |  6(1/0) 12(3/0) |      np nR nr |             | np       n  
    (xxx).L       |  6(1/0) 16(4/0) |   np np nR nr |             | np       n  
    #<data>       |  8(1/0)  8(2/0) |   np np       |               np       nn 
*/
  m68k_GET_SOURCE_L; //EA - A is allowed
  CHECK_IPL;
  PREFETCH_FINAL;
  if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
    CPU_BUS_IDLE(4); //nn
  else 
    CPU_BUS_IDLE(2); //n
  m68k_dst_l=REGL(PARAM_N);
  result=m68k_dst_l-m68k_src_l;
  SR_SUB_L(SR_X);
  REGL(PARAM_N)=result;
}


void m68k_suba_w() {
/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
    ADDA, SUBA    |  INSTR     EA   |  1st OP (ea)  |          INSTR            
------------------+-----------------+---------------+-------------------------- 
<ea>,An :         |                 |               |
  .B or .W :      |                 |               |
    Dn            |  8(1/0)  0(0/0) |               |               np       nn 
    An            |  8(1/0)  0(0/0) |               |               np       nn 
    (An)          |  8(1/0)  4(1/0) |            nr |               np       nn 
    (An)+         |  8(1/0)  4(1/0) |            nr |               np       nn 
    -(An)         |  8(1/0)  6(1/0) | n          nr |               np       nn 
    (d16,An)      |  8(1/0)  8(2/0) |      np    nr |               np       nn 
    (d8,An,Xn)    |  8(1/0) 10(2/0) | n    np    nr |               np       nn 
    (xxx).W       |  8(1/0)  8(2/0) |      np    nr |               np       nn 
    (xxx).L       |  8(1/0) 12(3/0) |   np np    nr |               np       nn 
    #<data>       |  8(1/0)  4(1/0) |      np       |               np       nn 
*/
  m68k_GET_SOURCE_W; //EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  CPU_BUS_IDLE(4); //nn
  m68k_src_l=(signed long)((signed short)m68k_src_w); // sign extension
  AREG(PARAM_N)-=m68k_src_l;
}


void m68k_subx_b() {
/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
    ADDX, SUBX    |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------ 
*/
  bool toreg=(DEST_IS_DATA_REGISTER);
  if(toreg)
  {
/*
Dy,Dx :           |                 |
  .B or .W :      |  4(1/0)         |                               np
*/
    m68k_src_b=REGB(PARAM_M);
    m68k_dst_b=REGB(PARAM_N);
  }
  else
  {
/*
-(Ay),-(Ax) :     |                 |
  .B or .W :      | 18(3/1)         |              n nr    nr       np nw       
*/
    CPU_BUS_IDLE(2); //n
    AREG(PARAM_M)--;
    if(PARAM_M==7)
      AREG(PARAM_M)--;
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_src_b=d8;
    AREG(PARAM_N)--;
    if(PARAM_N==7)
      AREG(PARAM_N)--;
    iabus=AREG(PARAM_N);
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_dst_b=d8;
  }//if
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=m68k_dst_b-m68k_src_b;
  if(PSWX)
    resultb--;
  PSWV=(( ( ((~m68k_src_b)&( m68k_dst_b)&(~resultb))|  
        (( m68k_src_b)&(~m68k_dst_b)&( resultb)) ) & MSB_B)!=0);  
  PSWX=PSWC=(( ( (( m68k_src_b)&(~m68k_dst_b)) | (( resultb)&(~m68k_dst_b))| 
        (( m68k_src_b)&( resultb)) ) & MSB_B)!=0); 
  if(resultb)
    CLEAR_Z;
  PSWN=(resultb<0);
  if(toreg)
    REGB(PARAM_N)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_sub_b_from_dN() {
/*
Dn,<ea> :         |                 |              /              |
  .B or .W :      |                 |             |               |
    (An)          |  8(1/1)  4(1/0) |             |            nr | np nw       
    (An)+         |  8(1/1)  4(1/0) |             |            nr | np nw       
    -(An)         |  8(1/1)  6(1/0) |             | n          nr | np nw       
    (d16,An)      |  8(1/1)  8(2/0) |             |      np    nr | np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) |             | n    np    nr | np nw       
    (xxx).W       |  8(1/1)  8(2/0) |             |      np    nr | np nw       
    (xxx).L       |  8(1/1) 12(3/0) |             |   np np    nr | np nw       

*/
  m68k_src_b=cpureg[PARAM_N].d8[B0];
  m68k_GET_DEST_B_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=m68k_dst_b-m68k_src_b;
  SR_SUB_B(SR_X);
  dbus=resultb;
  CPU_BUS_ACCESS_WRITE_B; //nw
}


void m68k_subx_w() {
/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
    ADDX, SUBX    |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------ 
*/
  bool toreg=(DEST_IS_DATA_REGISTER);
  if(toreg)
  {
/*
Dy,Dx :           |                 |
  .B or .W :      |  4(1/0)         |                               np
*/
    m68k_src_w=REGW(PARAM_M);
    m68k_dst_w=REGW(PARAM_N);
  }
  else
  {
/*
-(Ay),-(Ax) :     |                 |
  .B or .W :      | 18(3/1)         |              n nr    nr       np nw       
*/
    CPU_BUS_IDLE(2); //n
    AREG(PARAM_M)-=2;
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ; //nr
    m68k_src_w=dbus;
    AREG(PARAM_N)-=2;
    iabus=AREG(PARAM_N);
    CPU_BUS_ACCESS_READ; //n
    m68k_dst_w=dbus;
  }//if
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=m68k_dst_w-m68k_src_w;
  if(PSWX)
    resultl--;
  PSWV=(( ( ((~m68k_src_w)&( m68k_dst_w)&(~resultl))|
        (( m68k_src_w)&(~m68k_dst_w)&( resultl)) ) & MSB_W)!=0);
  PSWX=PSWC=(( ( (( m68k_src_w)&(~m68k_dst_w)) | (( resultl)&(~m68k_dst_w))|
        (( m68k_src_w)&( resultl)) ) & MSB_W)!=0); 
  if(resultl)
    CLEAR_Z;
  PSWN=(resultl<0);
  if(toreg)
    REGW(PARAM_N)=resultl;
  else
  {
    dbus=resultl;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


void m68k_sub_w_from_dN() {
/*
Dn,<ea> :         |                 |              /              |
  .B or .W :      |                 |             |               |
    (An)          |  8(1/1)  4(1/0) |             |            nr | np nw       
    (An)+         |  8(1/1)  4(1/0) |             |            nr | np nw       
    -(An)         |  8(1/1)  6(1/0) |             | n          nr | np nw       
    (d16,An)      |  8(1/1)  8(2/0) |             |      np    nr | np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) |             | n    np    nr | np nw       
    (xxx).W       |  8(1/1)  8(2/0) |             |      np    nr | np nw       
    (xxx).L       |  8(1/1) 12(3/0) |             |   np np    nr | np nw       
*/
  m68k_src_w=cpureg[PARAM_N].d16[LO];
  m68k_GET_DEST_W_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=m68k_dst_w-m68k_src_w;
  SR_SUB_W(SR_X);
  dbus=resultl;
  CPU_BUS_ACCESS_WRITE; //nw
}


void m68k_subx_l() { 
/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
    ADDX, SUBX    |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------ 
*/
  bool toreg=(DEST_IS_DATA_REGISTER);
  if(toreg)
  {
/*
Dy,Dx :           |                 |
  .L :            |  8(1/0)         |                               np       nn
*/
    CHECK_IPL;
    PREFETCH_FINAL; //np
    CPU_BUS_IDLE(4); //nn
    m68k_src_l=Cpu.r[PARAM_M];
    m68k_dst_l=REGL(PARAM_N);
  }
  else
  {
/*
-(Ay),-(Ax) :     |                 |
  .L :            | 30(5/2)         |              n nr nR nr nR nw np    nW    
*/
    CPU_BUS_IDLE(2); //n 
    AREG(PARAM_M)-=2;
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ; //nr
    m68k_src_ll=dbus;
    AREG(PARAM_M)-=2;
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ; //nR
    m68k_src_lh=dbus;
    AREG(PARAM_N)-=2;
    iabus=AREG(PARAM_N);
    CPU_BUS_ACCESS_READ; //nr
    m68k_dst_ll=dbus;
    AREG(PARAM_N)-=2;
    iabus=AREG(PARAM_N);
    CPU_BUS_ACCESS_READ; //nR
    m68k_dst_lh=dbus;
    TRUE_PC=pc+2;
  }//if
  result=m68k_dst_l-m68k_src_l;
  if(PSWX)
    result--;
  PSWV=(( ( ((~m68k_src_l)&( m68k_dst_l)&(~result))| 
        (( m68k_src_l)&(~m68k_dst_l)&( result)) ) & MSB_L)!=0);
  PSWX=PSWC=(( ( (( m68k_src_l)&(~m68k_dst_l)) | (( result)&(~m68k_dst_l))| 
        (( m68k_src_l)&( result)) ) & MSB_L)!=0); 
  if(result)
    CLEAR_Z;
  PSWN=(result<0);
  if(toreg)
  {
    REGL(PARAM_N)=result;
  }
  else
  {
    dbus=resultl;
    iabus+=2;
    CPU_BUS_ACCESS_WRITE; //nw
    CHECK_IPL;
    PREFETCH_FINAL; //np between 2 writes
    dbus=resulth;
    iabus-=2;
    CPU_BUS_ACCESS_WRITE; //nW
  }
}


void m68k_sub_l_from_dN() { 
/*
  .L :            |                 |             |               |
    (An)          | 12(1/2)  8(2/0) |             |         nR nr | np nw nW    
    (An)+         | 12(1/2)  8(2/0) |             |         nR nr | np nw nW    
    -(An)         | 12(1/2) 10(2/0) |             | n       nR nr | np nw nW    
    (d16,An)      | 12(1/2) 12(3/0) |             |      np nR nr | np nw nW    
    (d8,An,Xn)    | 12(1/2) 14(3/0) |             | n    np nR nr | np nw nW    
    (xxx).W       | 12(1/2) 12(3/0) |             |      np nR nr | np nw nW    
    (xxx).L       | 12(1/2) 16(4/0) |             |   np np nR nr | np nw nW    
*/
  m68k_src_l=Cpu.r[PARAM_N];
  m68k_GET_DEST_L_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  result=m68k_dst_l-m68k_src_l;
  SR_SUB_L(SR_X);
  WRITE_LONG_BACKWARDS; //nw nW
}


void m68k_suba_l() {
/*
  .L :            |                 |               |
    Dn            |  8(1/0)  0(0/0) |               |               np       nn 
    An            |  8(1/0)  0(0/0) |               |               np       nn 
    (An)          |  6(1/0)  8(2/0) |         nR nr |               np       n  
    (An)+         |  6(1/0)  8(2/0) |         nR nr |               np       n  
    -(An)         |  6(1/0) 10(2/0) | n       nR nr |               np       n  
    (d16,An)      |  6(1/0) 12(3/0) |      np nR nr |               np       n  
    (d8,An,Xn)    |  6(1/0) 14(3/0) | n    np nR nr |               np       n  
    (xxx).W       |  6(1/0) 12(3/0) |      np nR nr |               np       n  
    (xxx).L       |  6(1/0) 16(4/0) |   np np nR nr |               np       n  
    #<data>       |  8(1/0)  8(2/0) |   np np       |               np       nn 
*/
  m68k_GET_SOURCE_L; // EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
    CPU_BUS_IDLE(4); //nn
  else 
    CPU_BUS_IDLE(2); //n
  AREG(PARAM_N)-=m68k_src_l;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  line b - cmp, eor       ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
       CMP        |  INSTR     EA   |  1st OP (ea)  |          INSTR            
------------------+-----------------+---------------+-------------------------- 
*/

void m68k_cmp_b() {
/*
<ea>,Dn :         |                 |               | 
  .B or .W :      |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  4(1/0)  4(1/0) |            nr |               np          
    (An)+         |  4(1/0)  4(1/0) |            nr |               np          
    -(An)         |  4(1/0)  6(1/0) | n          nr |               np          
    (d16,An)      |  4(1/0)  8(2/0) |      np    nr |               np          
    (d8,An,Xn)    |  4(1/0) 10(2/0) | n    np    nr |               np          
    (xxx).W       |  4(1/0)  8(2/0) |      np    nr |               np          
    (xxx).L       |  4(1/0) 12(3/0) |   np np    nr |               np          
    #<data>       |  4(1/0)  4(1/0) |      np       |               np   
*/
  m68k_GET_SOURCE_B_NOT_A; //EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  m68k_dst_b=REGB(PARAM_N);
  resultb=m68k_dst_b-m68k_src_b;
  SR_SUB_B(0);
}


void m68k_cmp_w() {
/*
<ea>,Dn :         |                 |               | 
  .B or .W :      |                 |               | 
    Dn            |  4(1/0)  0(0/0) |               |               np          
    An            |  4(1/0)  0(0/0) |               |               np          
    (An)          |  4(1/0)  4(1/0) |            nr |               np          
    (An)+         |  4(1/0)  4(1/0) |            nr |               np          
    -(An)         |  4(1/0)  6(1/0) | n          nr |               np          
    (d16,An)      |  4(1/0)  8(2/0) |      np    nr |               np          
    (d8,An,Xn)    |  4(1/0) 10(2/0) | n    np    nr |               np          
    (xxx).W       |  4(1/0)  8(2/0) |      np    nr |               np          
    (xxx).L       |  4(1/0) 12(3/0) |   np np    nr |               np          
    #<data>       |  4(1/0)  4(1/0) |      np       |               np   
*/
  m68k_GET_SOURCE_W; //EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  m68k_dst_w=REGW(PARAM_N);
  resultl=m68k_dst_w-m68k_src_w;
  SR_SUB_W(0);
}


void m68k_cmp_l() {
/*
  .L :            |                 |               | 
    Dn            |  6(1/0)  0(0/0) |               |               np       n  
    An            |  6(1/0)  0(0/0) |               |               np       n  
    (An)          |  6(1/0)  8(1/0) |         nR nr |               np       n  
    (An)+         |  6(1/0)  8(1/0) |         nR nr |               np       n  
    -(An)         |  6(1/0) 10(1/0) | n       nR nr |               np       n  
    (d16,An)      |  6(1/0) 12(2/0) |      np nR nr |               np       n  
    (d8,An,Xn)    |  6(1/0) 14(2/0) | n    np nR nr |               np       n  
    (xxx).W       |  6(1/0) 12(2/0) |      np nR nr |               np       n  
    (xxx).L       |  6(1/0) 16(3/0) |   np np nR nr |               np       n  
    #<data>       |  6(1/0)  8(2/0) |   np np       |               np       n  
*/
  m68k_GET_SOURCE_L; //EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  CPU_BUS_IDLE(2); //n
  m68k_dst_l=REGL(PARAM_N);
  result=m68k_dst_l-m68k_src_l;
  SR_SUB_L(0);
}


void m68k_cmpa_w() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage             
       CMPA       |  INSTR     EA   |  1st OP (ea)  |          INSTR           
------------------+-----------------+---------------+--------------------------
<ea>,An :         |                 |               | 
   .W :           |                 |               | 
    Dn            |  6(1/0)  0(0/0) |               |               np       n  
    An            |  6(1/0)  0(0/0) |               |               np       n  
    (An)          |  6(1/0)  4(1/0) |            nr |               np       n  
    (An)+         |  6(1/0)  4(1/0) |            nr |               np       n  
    -(An)         |  6(1/0)  6(1/0) | n          nr |               np       n  
    (d16,An)      |  6(1/0)  8(2/0) |      np    nr |               np       n  
    (d8,An,Xn)    |  6(1/0) 10(2/0) | n    np    nr |               np       n  
    (xxx).W       |  6(1/0)  8(2/0) |      np    nr |               np       n  
    (xxx).L       |  6(1/0) 12(3/0) |   np np    nr |               np       n  
    #<data>       |  6(1/0)  4(1/0) |      np       |               np       n  
*/
  m68k_GET_SOURCE_W; //EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  CPU_BUS_IDLE(2); //n
  m68k_src_l=(signed long)((signed short)m68k_src_w); // sign extension
  m68k_dst_l=AREG(PARAM_N);
  result=m68k_dst_l-m68k_src_l;
  SR_SUB_L(0);
}


void m68k_cmpm_b() {
/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
       CMPM       |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------ 
(Ay)+,(Ax)+       |                 |
  .B or .W :      | 12(3/0)         |                      nr    nr np          
*/
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_src_b=d8;
  AREG(PARAM_M)++;
  if(ry==7)
    AREG(PARAM_M)++;
  iabus=AREG(PARAM_N);
  CHECK_IPL;
  CPU_BUS_ACCESS_READ_B; //nr
  m68k_dst_b=d8;
  AREG(PARAM_N)++;
  if(rx==7)
    AREG(PARAM_N)++;
  resultb=m68k_dst_b-m68k_src_b;
  SR_SUB_B(0);
  PREFETCH_FINAL; //np
}


void m68k_eor_b() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage              
        EOR       |  INSTR     EA   |  1st OP (ea)  |          INSTR           
------------------+-----------------+---------------+--------------------------
Dn,<ea> :         |                 |               |
  .B or .W :      |                 |               |
    Dn            |  4(1/0)  0(0/0) |               |               np		     
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw	   
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw	      
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw	      
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw	      
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw	      
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw	      
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np nw	
*/
  m68k_src_b=REGB(PARAM_N);
  m68k_GET_DEST_B_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=m68k_dst_b^m68k_src_b;
  SR_CHECK_AND_B;
  if(DEST_IS_DATA_REGISTER)
    REGB(PARAM_M)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B;  //nw
  }
}


void m68k_cmpm_w() {
/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
       CMPM       |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------ 
(Ay)+,(Ax)+       |                 |
  .B or .W :      | 12(3/0)         |                      nr    nr np          
*/
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nr
  m68k_src_w=dbus;
  AREG(PARAM_M)+=2;
  iabus=AREG(PARAM_N);
  CHECK_IPL;
  CPU_BUS_ACCESS_READ; //nr
  m68k_dst_w=dbus;
  AREG(PARAM_N)+=2;
  resultl=m68k_dst_w-m68k_src_w;
  SR_SUB_W(0);
  PREFETCH_FINAL; //np
}


void m68k_eor_w() {
/*
Dn,<ea> :         |                 |               |
  .B or .W :      |                 |               |
    Dn            |  4(1/0)  0(0/0) |               |               np		     
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw	   
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw	      
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw	      
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw	      
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw	      
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw	      
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np nw	  
*/
  m68k_src_w=REGW(PARAM_N);
  m68k_GET_DEST_W_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=m68k_dst_w^m68k_src_w;
  SR_CHECK_AND_W;
  if(DEST_IS_DATA_REGISTER)
    REGW(PARAM_M)=resultl;
  else
  {
    dbus=resultl;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


void m68k_cmpm_l() {
  //   .L :            | 20(5/0)         |                   nR nr nR nr np   
  iabus=AREG(PARAM_M);
  CPU_BUS_ACCESS_READ; //nR
  m68k_src_lh=dbus;
  iabus+=2;
  CPU_BUS_ACCESS_READ; //nr
  m68k_src_ll=dbus;
  AREG(PARAM_M)+=4;
  iabus=AREG(PARAM_N);
  CPU_BUS_ACCESS_READ; //nR
  m68k_dst_lh=dbus;
  iabus+=2;
  CHECK_IPL;
  CPU_BUS_ACCESS_READ; //nr
  m68k_dst_ll=dbus;
  AREG(PARAM_N)+=4;
  result=m68k_dst_l-m68k_src_l;
  SR_SUB_L(0);
  PREFETCH_FINAL; //np
}


void m68k_eor_l() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage              
        EOR       |  INSTR     EA   |  1st OP (ea)  |          INSTR           
------------------+-----------------+---------------+--------------------------
Dn,<ea> :         |                 |               |
  .L :            |                 |               |                           
    Dn            |  8(1/0)  0(0/0) |               |               np       nn 
    (An)          | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    (An)+         | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    -(An)         | 12(1/2) 10(2/0) | n       nR nr |               np nw nW    
    (d16,An)      | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (d8,An,Xn)    | 12(1/2) 14(3/0) | n    np nR nr |               np nw nW    
    (xxx).W       | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (xxx).L       | 12(1/2) 16(4/0) |   np np nR nr |               np nw nW    
*/
  m68k_src_l=REGL(PARAM_N);
  m68k_GET_DEST_L_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  result=m68k_dst_l^m68k_src_l;
  SR_CHECK_AND_L;
  if(DEST_IS_DATA_REGISTER)
  {
    CPU_BUS_IDLE(4); //nn
    REGL(PARAM_M)=result;
  }
  else
    WRITE_LONG_BACKWARDS; //nw nW
}


void m68k_cmpa_l() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage             
       CMPA       |  INSTR     EA   |  1st OP (ea)  |          INSTR           
------------------+-----------------+---------------+--------------------------
<ea>,An :         |                 |               | 
  .L :            |                 |               | 
    Dn            |  6(1/0)  0(0/0) |               |               np       n  
    An            |  6(1/0)  0(0/0) |               |               np       n  
    (An)          |  6(1/0)  8(1/0) |         nR nr |               np       n  
    (An)+         |  6(1/0)  8(1/0) |         nR nr |               np       n  
    -(An)         |  6(1/0) 10(1/0) | n       nR nr |               np       n  
    (d16,An)      |  6(1/0) 12(2/0) |      np nR nr |               np       n  
    (d8,An,Xn)    |  6(1/0) 14(2/0) | n    np nR nr |               np       n  
    (xxx).W       |  6(1/0) 12(2/0) |      np nR nr |               np       n  
    (xxx).L       |  6(1/0) 16(3/0) |   np np nR nr |               np       n  
    #<data>       |  6(1/0)  8(2/0) |   np np       |               np       n  
*/
  m68k_GET_SOURCE_L; //EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  CPU_BUS_IDLE(2); //n
  m68k_dst_l=AREG(PARAM_N);
  result=m68k_dst_l-m68k_src_l;
  SR_SUB_L(0);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////  Line C - and, abcd, exg, mul   ///////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void m68k_and_b_to_dN() {
/*
<ea>,Dn :         |                 |               |
  .B or .W :      |                 |               |
    Dn            |  4(1/0)  0(0/0) |               |               np       
    (An)          |  4(1/0)  4(1/0) |            nr |               np		     
    (An)+         |  4(1/0)  4(1/0) |            nr |               np		     
    -(An)         |  4(1/0)  6(1/0) | n          nr |               np		     
    (d16,An)      |  4(1/0)  8(2/0) |      np    nr |               np		     
    (d8,An,Xn)    |  4(1/0) 10(2/0) | n    np    nr |               np		     
    (xxx).W       |  4(1/0)  8(2/0) |      np    nr |               np		     
    (xxx).L       |  4(1/0) 12(3/0) |   np np    nr |               np		     
    #<data>       |  4(1/0)  4(1/0) |      np       |               np		     

*/
  m68k_GET_SOURCE_B_NOT_A; // EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  m68k_dst_b=REGB(PARAM_N);
  resultb=(m68k_src_b&m68k_dst_b);
  SR_CHECK_AND_B;
  REGB(PARAM_N)=resultb;
}


void m68k_and_w_to_dN() {
/*
<ea>,Dn :         |                 |               |
  .B or .W :      |                 |               |
    Dn            |  4(1/0)  0(0/0) |               |               np       
    (An)          |  4(1/0)  4(1/0) |            nr |               np		     
    (An)+         |  4(1/0)  4(1/0) |            nr |               np		     
    -(An)         |  4(1/0)  6(1/0) | n          nr |               np		     
    (d16,An)      |  4(1/0)  8(2/0) |      np    nr |               np		     
    (d8,An,Xn)    |  4(1/0) 10(2/0) | n    np    nr |               np		     
    (xxx).W       |  4(1/0)  8(2/0) |      np    nr |               np		     
    (xxx).L       |  4(1/0) 12(3/0) |   np np    nr |               np		     
    #<data>       |  4(1/0)  4(1/0) |      np       |               np		     

*/
  m68k_GET_SOURCE_W_NOT_A; //EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  m68k_dst_w=REGW(PARAM_N);
  resultl=(m68k_src_w&m68k_dst_w);
  SR_CHECK_AND_W;
  REGW(PARAM_N)=resultl;
}


void m68k_and_l_to_dN() {
/*
  .L :            |                 |               |
    Dn            |  8(1/0)  0(0/0) |               |               np       nn
    (An)          |  6(1/0)  8(2/0) |         nR nr |               np       n	
    (An)+         |  6(1/0)  8(2/0) |         nR nr |               np       n	
    -(An)         |  6(1/0) 10(2/0) | n       nR nr |               np       n	
    (d16,An)      |  6(1/0) 12(3/0) |      np nR nr |               np       n	
    (d8,An,Xn)    |  6(1/0) 14(3/0) | n    np nR nr |               np       n	
    (xxx).W       |  6(1/0) 12(3/0) |      np nR nr |               np       n	
    (xxx).L       |  6(1/0) 16(4/0) |   np np nR nr |               np       n	
    #<data>       |  8(1/0)  8(2/0) |   np np       |               np       nn

*/
  m68k_GET_SOURCE_L_NOT_A; //EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  m68k_dst_l=REGL(PARAM_N);
  if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
    CPU_BUS_IDLE(4); //nn
  else 
    CPU_BUS_IDLE(2); //n
  result=(m68k_src_l&m68k_dst_l);
  SR_CHECK_AND_L;
  REGL(PARAM_N)=result;
}


void m68k_mulu() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
    MULU,MULS     |  INSTR     EA   |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
<ea>,Dn :        /                  |               |
  .W :          /                   |               |
    Dn         | 38+2m(1/0)  0(0/0) |               |               np       n* 
    (An)       | 38+2m(1/0)  4(1/0) |            nr |               np       n* 
    (An)+      | 38+2m(1/0)  4(1/0) |            nr |               np       n* 
    -(An)      | 38+2m(1/0)  6(1/0) | n          nr |               np       n* 
    (d16,An)   | 38+2m(1/0)  8(2/0) |      np    nr |               np       n* 
    (d8,An,Xn) | 38+2m(1/0) 10(2/0) | n    np    nr |               np       n* 
    (xxx).W    | 38+2m(1/0)  8(2/0) |      np    nr |               np       n* 
    (xxx).L    | 38+2m(1/0) 12(2/0) |   np np    nr |               np       n* 
    #<data>    | 38+2m(1/0)  8(2/0) |      np       |               np       n* 
NOTES :
  .for MULU 'm' = the number of ones in the source
    - Best case 38 cycles with $0
    - Worst case : 70 cycles with $FFFF 
  .for MULS 'm' = concatenate the 16-bit pointed by <ea> with a zero as the LSB
   'm' is the resultant number of 10 or 01 patterns in the 17-bit source.
    - Best case : 38 cycles with $0 or $FFFF
    - Worst case : 70 cycles with $5555
  .in both cases : 'n*' should be replaced by 17+m consecutive 'n'
FLOWCHART :                                                                     
                            np                                                  
         LSB=1              |           LSB=0                                   
               +------------+------------+                                      
               |                         |                                      
            +->n------------------------>n<----+                                
            |                            |     | LSB=0 or                       
            |           No more bits     |     | 2LSB=00,11                     
   LSB=1 or +---------------+------------+-----+                                
   2LSB=01,10               |                                                   
                            n                                                   
  .LSB = less significant bit : bit at the far right of the source.             
  .2LSB = 2 less significant bits : 2 last bits at the far right of the source. 

*/
  m68k_GET_SOURCE_W_NOT_A; // EA
  CHECK_IPL;
  PREFETCH_FINAL; //np 
  for(int i=0;i<(34/2);i++)
    CPU_BUS_IDLE(2); //n
  for(WORD Val=m68k_src_w;Val;Val>>=1)
  {
    if(Val&1)
      CPU_BUS_IDLE(2); //n
  }
  m68k_dst_w=REGW(PARAM_N);
  result=(unsigned long)(unsigned short)m68k_dst_w
    *(unsigned long)((unsigned short)m68k_src_w);
  SR_CHECK_AND_L;
  REGL(PARAM_N)=result;
}


void m68k_abcd() {
/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
    ABCD, SBCD    |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------ 
*/
  bool toreg=(DEST_IS_DATA_REGISTER);
  if(toreg)
  {
/*
Dy,Dx :           |                 |
  .B :            |  6(1/0)         |                               np       n
*/
    CHECK_IPL;
    PREFETCH_FINAL; //np
    CPU_BUS_IDLE(2); //n
    m68k_src_b=REGB(PARAM_M);
    m68k_dst_b=REGB(PARAM_N);
  }
  else
  {
/*
-(Ay),-(Ax) :     |                 |
  .B :            | 18(3/1)         |                 n    nr    nr np nw       
*/
    CPU_BUS_IDLE(2); //n
    AREG(PARAM_M)--;
    if(ry==7)
      AREG(PARAM_M)--;
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_src_b=d8;
    AREG(PARAM_N)--;
    if(rx==7)
      AREG(PARAM_N)--;
    iabus=AREG(PARAM_N);
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_dst_b=d8;
    TRUE_PC=pc+2;
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }//if
/*
  http://en.wikipedia.org/wiki/Binary-coded_decimal#Addition_with_BCD
  each decimal digit is coded on 4bit

    0-9  0-9   +   0-9  0-9   
    ---- ----      ---- ----  

  The Steem way works for normal operands but when an operand
  is illegal (nibble>9), the result isn't the same as on a Cpu,
  that uses the "+6" trick.
  http://tams-www.informatik.uni-hamburg.de/applets/hades/webdemos/20-arithmetic/10-adders/bcd-adder.html
  In the 68000, the 'DAA' decimal adjust accumulator is integrated into ABCD.
  It is correct in WinUAE, so we use the same way now.
*/
  BYTE src=m68k_src_b;
  BYTE dst=m68k_dst_b;
  BYTE lo_nibble=(src&0xF)+(dst&0xF);
  if(PSWX)
    lo_nibble++;
  CLEAR_XNC;
  if(lo_nibble>9)
    lo_nibble+=6;
  WORD hi_nibble=(src&0xF0)+(dst&0xF0)+(lo_nibble&0xF0);
  if(hi_nibble>0x90)
  {
    hi_nibble+=0x60;
    SET_XC;
  }
  resultb=(hi_nibble&0xF0)+(lo_nibble&0xF);
  if(resultb)
    CLEAR_Z;
  else if(resultb<0)
    SET_N;
  if(toreg)
    REGB(PARAM_N)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_and_b_from_dN() {
/*
Dn,<ea> :         |                 |               | 
  .B or .W :      |                 |               | 
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw	      
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw	      
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw	      
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw	      
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw	      
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw	      
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np nw	      
*/
  m68k_GET_DEST_B_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  m68k_src_b=REGB(PARAM_N);
  PREFETCH_FINAL; //np
  resultb=(m68k_dst_b&m68k_src_b);
  SR_CHECK_AND_B;
  dbus=resultb;
  CPU_BUS_ACCESS_WRITE_B; //nw
}


void m68k_exg_like() {
/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
        EXG       |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------ 
  .L :            |                 |
    Dx,Dy         |  6(1/0)         |                               np       n  
    Ax,Ay         |  6(1/0)         |                               np       n  
*/
  switch(IRD&BITS_543) {
  case BITS_543_000: //EXG D D
    CHECK_IPL;
    PREFETCH_FINAL; //np
    CPU_BUS_IDLE(2); //n
    compare_buffer=Cpu.r[PARAM_N];
    Cpu.r[PARAM_N]=Cpu.r[PARAM_M];
    Cpu.r[PARAM_M]=compare_buffer;
    break;
  case BITS_543_001: //EXG A A
    CHECK_IPL;
    PREFETCH_FINAL; //np
    CPU_BUS_IDLE(2); //n
    compare_buffer=AREG(PARAM_N);
    AREG(PARAM_N)=AREG(PARAM_M);
    AREG(PARAM_M)=compare_buffer;
    break;
  }//sw
}


void m68k_and_w_from_dN() {
/*
Dn,<ea> :         |                 |               | 
  .B or .W :      |                 |               | 
    (An)          |  8(1/1)  4(1/0) |            nr |               np nw	      
    (An)+         |  8(1/1)  4(1/0) |            nr |               np nw	      
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np nw	      
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np nw	      
    (d8,An,Xn)    |  8(1/1) 10(2/0) | n    np    nr |               np nw	      
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np nw	      
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np nw	      
*/
  m68k_GET_DEST_W_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  m68k_src_w=REGW(PARAM_N);
  PREFETCH_FINAL; //np
  resultl=(m68k_dst_w&m68k_src_w);
  SR_CHECK_AND_W;
  dbus=resultl;
  CPU_BUS_ACCESS_WRITE; //nw
}


void m68k_exg_unlike() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
        EXG       |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
  .L :            |                 |
    Dx,Ay         |  6(1/0)         |                               np       n
*/
  CHECK_IPL;
  PREFETCH_FINAL; //np
  CPU_BUS_IDLE(2); //n
  compare_buffer=AREG(PARAM_M);
  AREG(PARAM_M)=REGL(PARAM_N);
  REGL(PARAM_N)=compare_buffer;
}


void m68k_and_l_from_dN() {
/*
Dn,<ea> :         |                 |               | 
  .L :            |                 |                              
    (An)          | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    (An)+         | 12(1/2)  8(2/0) |         nR nr |               np nw nW    
    -(An)         | 12(1/2) 10(2/0) | n       nR nr |               np nw nW    
    (d16,An)      | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (d8,An,Xn)    | 12(1/2) 14(3/0) | n    np nR nr |               np nw nW    
    (xxx).W       | 12(1/2) 12(3/0) |      np nR nr |               np nw nW    
    (xxx).L       | 12(1/2) 16(4/0) |   np np nR nr |               np nw nW    
*/
  m68k_GET_DEST_L_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  m68k_src_l=Cpu.r[PARAM_N];
  PREFETCH_FINAL; //np
  result=(m68k_dst_l&m68k_src_l);
  SR_CHECK_AND_L;
  WRITE_LONG_BACKWARDS; //nw nW
}


void m68k_muls() {
  // see mulu for YACHT chart
  m68k_GET_SOURCE_W_NOT_A; // EA
  CHECK_IPL;
  PREFETCH_FINAL; //np 
/*
  .for MULS 'm' = concatenate the 16-bit pointed by <ea> with a zero as the LSB
   'm' is the resultant number of 10 or 01 patterns in the 17-bit source.
    - Best case : 38 cycles with $0 or $FFFF
    - Worst case : 70 cycles with $5555
*/
  for(int i=0;i<(34/2);i++)
    CPU_BUS_IDLE(2); //n
  int LastLow=0;
  int Val=WORD(m68k_src_w);
  for(int n=0;n<16;n++)
  {
    if((Val&1)!=LastLow)
      CPU_BUS_IDLE(2); //n*
    LastLow=(Val&1);
    Val>>=1;
  }
  result=((signed long)((signed short)LOWORD(Cpu.r[PARAM_N])))
    *((signed long)((signed short)m68k_src_w));
  //SR_CHECK_Z_AND_N_L;
  SR_CHECK_AND_L;
  Cpu.r[PARAM_N]=result;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  line D - add.b/add.w/add.l ////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void m68k_add_b_to_dN() {
/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
     ADD, SUB     |  INSTR     EA   | 1st Operand |  2nd OP (ea)  |   INSTR     
------------------+-----------------+-------------+---------------+------------ 
<ea>,Dn :         |                 |              \              |
  .B or .W :      |                 |               |             |
    Dn            |  4(1/0)  0(0/0) |               |             | np          
    An            |  4(1/0)  0(0/0) |               |             | np          
    (An)          |  4(1/0)  4(1/0) |            nr |             | np          
    (An)+         |  4(1/0)  4(1/0) |            nr |             | np          
    -(An)         |  4(1/0)  6(1/0) | n          nr |             | np          
    (d16,An)      |  4(1/0)  8(2/0) |      np    nr |             | np          
    (d8,An,Xn)    |  4(1/0) 10(2/0) | n    np    nr |             | np          
    (xxx).W       |  4(1/0)  8(2/0) |      np    nr |             | np          
    (xxx).L       |  4(1/0) 12(3/0) |   np np    nr |             | np          
    #<data>       |  4(1/0)  4(1/0)        np                     | np          
*/
  m68k_GET_SOURCE_B_NOT_A; //EA
  CHECK_IPL;
  PREFETCH_FINAL; //np
  m68k_dst_b=REGB(PARAM_N);
  resultb=m68k_dst_b+m68k_src_b;
  SR_ADD_B;
  REGB(PARAM_N)=resultb;
}


void m68k_add_w_to_dN() { // add.w ea,dn or adda.w ea,an
/*
  .B or .W :      |                 |               |             |
    Dn            |  4(1/0)  0(0/0) |               |             | np          
    An            |  4(1/0)  0(0/0) |               |             | np          
    (An)          |  4(1/0)  4(1/0) |            nr |             | np          
    (An)+         |  4(1/0)  4(1/0) |            nr |             | np          
    -(An)         |  4(1/0)  6(1/0) | n          nr |             | np          
    (d16,An)      |  4(1/0)  8(2/0) |      np    nr |             | np          
    (d8,An,Xn)    |  4(1/0) 10(2/0) | n    np    nr |             | np          
    (xxx).W       |  4(1/0)  8(2/0) |      np    nr |             | np          
    (xxx).L       |  4(1/0) 12(3/0) |   np np    nr |             | np          
    #<data>       |  4(1/0)  4(1/0)        np                     | np          
*/
  m68k_GET_SOURCE_W; //EA  //A is allowed
  CHECK_IPL;
  PREFETCH_FINAL; //np
  m68k_dst_w=REGW(PARAM_N);
  resultl=m68k_dst_w+m68k_src_w;
  SR_ADD_W;
  REGW(PARAM_N)=resultl;
}


void m68k_add_l_to_dN() { // add.l ea,dn or adda.l ea,an
/*
  .L :            |                 |               |             |
    Dn            |  8(1/0)  0(0/0) |               |             | np       nn 
    An            |  8(1/0)  0(0/0) |               |             | np       nn 
    (An)          |  6(1/0)  8(2/0) |         nR nr |             | np       n  
    (An)+         |  6(1/0)  8(2/0) |         nR nr |             | np       n  
    -(An)         |  6(1/0) 10(2/0) | n       nR nr |             | np       n  
    (d16,An)      |  6(1/0) 12(3/0) |      np nR nr |             | np       n  
    (d8,An,Xn)    |  6(1/0) 14(3/0) | n    np nR nr |             | np       n  
    (xxx).W       |  6(1/0) 12(3/0) |      np nR nr |             | np       n  
    (xxx).L       |  6(1/0) 16(4/0) |   np np nR nr |             | np       n  
    #<data>       |  8(1/0)  8(2/0) |   np np       |               np       nn 
*/
  m68k_GET_SOURCE_L; //EA  //A is allowed
  CHECK_IPL;
  PREFETCH_FINAL; //np
  m68k_dst_l=REGL(PARAM_N);
  if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
    CPU_BUS_IDLE(4); //nn
  else 
    CPU_BUS_IDLE(2); //n
  result=m68k_dst_l+m68k_src_l;
  SR_ADD_L;
  REGL(PARAM_N)=result;
}


void m68k_adda_w() {
/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
    ADDA, SUBA    |  INSTR     EA   |  1st OP (ea)  |          INSTR            
------------------+-----------------+---------------+-------------------------- 
<ea>,An :         |                 |               |
  .B or .W :      |                 |               |
    Dn            |  8(1/0)  0(0/0) |               |               np       nn 
    An            |  8(1/0)  0(0/0) |               |               np       nn 
    (An)          |  8(1/0)  4(1/0) |            nr |               np       nn 
    (An)+         |  8(1/0)  4(1/0) |            nr |               np       nn 
    -(An)         |  8(1/0)  6(1/0) | n          nr |               np       nn 
    (d16,An)      |  8(1/0)  8(2/0) |      np    nr |               np       nn 
    (d8,An,Xn)    |  8(1/0) 10(2/0) | n    np    nr |               np       nn 
    (xxx).W       |  8(1/0)  8(2/0) |      np    nr |               np       nn 
    (xxx).L       |  8(1/0) 12(3/0) |   np np    nr |               np       nn 
    #<data>       |  8(1/0)  4(1/0) |      np       |               np       nn 
*/
  m68k_GET_SOURCE_W; // EA
  CHECK_IPL;
  m68k_src_l=(signed long)((signed short)m68k_src_w); // sign extension
  PREFETCH_FINAL; //np
  CPU_BUS_IDLE(4); //nn
  AREG(PARAM_N)+=m68k_src_l;
}


void m68k_addx_b() {
/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
    ADDX, SUBX    |      INSTR      |                  INSTR                    
------------------+-----------------+------------------------------------------ 
*/
  bool toreg=(DEST_IS_DATA_REGISTER);
  if(toreg)
  {
/*
Dy,Dx :           |                 |
  .B or .W :      |  4(1/0)         |                               np
*/
    m68k_src_b=REGB(PARAM_M);
    m68k_dst_b=REGB(PARAM_N);
  }
  else
  {
/*
-(Ay),-(Ax) :     |                 |
  .B or .W :      | 18(3/1)         |              n nr    nr       np nw
*/
    CPU_BUS_IDLE(2); //n
    AREG(PARAM_M)--;
    if(ry==7)
      AREG(PARAM_M)--;
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_src_b=d8;
    AREG(PARAM_N)--;
    if(PARAM_N==7)
      AREG(PARAM_N)--;
    iabus=AREG(PARAM_N);
    CPU_BUS_ACCESS_READ_B; //nr
    m68k_dst_b=d8;
  }//if
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=m68k_dst_b+m68k_src_b;
  if(PSWX)
    resultb++;
  PSWV=(( ( (( m68k_src_b)&( m68k_dst_b)&(~resultb))| 
        ((~m68k_src_b)&(~m68k_dst_b)&( resultb)) ) & MSB_B)!=0);
  PSWX=PSWC=(( ( (( m68k_src_b)&( m68k_dst_b)) | ((~resultb)&( m68k_dst_b))|
        (( m68k_src_b)&(~resultb)) ) & MSB_B)!=0);
  if(resultb)
    CLEAR_Z;
  PSWN=(resultb<0);
  if(toreg)
    REGB(PARAM_N)=resultb;
  else
  {
    dbus=resultb;
    CPU_BUS_ACCESS_WRITE_B; //nw
  }
}


void m68k_add_b_from_dN() {
/* ADD.B
Dn,<ea> :         |                 |              /              |
  .B or .W :      |                 |             |               |
    (An)          |  8(1/1)  4(1/0) |             |            nr | np nw
    (An)+         |  8(1/1)  4(1/0) |             |            nr | np nw
    -(An)         |  8(1/1)  6(1/0) |             | n          nr | np nw
    (d16,An)      |  8(1/1)  8(2/0) |             |      np    nr | np nw
    (d8,An,Xn)    |  8(1/1) 10(2/0) |             | n    np    nr | np nw
    (xxx).W       |  8(1/1)  8(2/0) |             |      np    nr | np nw
    (xxx).L       |  8(1/1) 12(3/0) |             |   np np    nr | np nw
*/
  m68k_src_b=REGB(PARAM_N);
  m68k_GET_DEST_B_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultb=m68k_dst_b+m68k_src_b;
  SR_ADD_B;
  dbus=resultb;
  CPU_BUS_ACCESS_WRITE_B; //nw
}


void m68k_addx_w() {
  bool toreg=(DEST_IS_DATA_REGISTER);
  if(toreg)
  {
/*
Dy,Dx :           |                 |
  .B or .W :      |  4(1/0)         |                               np
*/
    m68k_src_w=REGW(PARAM_M);//LOWORD(Cpu.r[PARAM_M]);
    m68k_dst_w=REGW(PARAM_N);//=&(Cpu.r[PARAM_N]);
  }
  else
  {
/*
-(Ay),-(Ax) :     |                 |
  .B or .W :      | 18(3/1)         |              n nr    nr       np nw
*/
    CPU_BUS_IDLE(2); //n
    AREG(PARAM_M)-=2;
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ; //nr
    m68k_src_w=dbus;
    AREG(PARAM_N)-=2;
    iabus=AREG(PARAM_N);
    CPU_BUS_ACCESS_READ; //nr
    m68k_dst_w=dbus;
  }//if
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=m68k_dst_w+m68k_src_w;
  if(PSWX)
    resultl++;
  PSWV=(( ( (( m68k_src_w)&( m68k_dst_w)&(~resultl))|
        ((~m68k_src_w)&(~m68k_dst_w)&( resultl)) ) & MSB_W)!=0);
  PSWX=PSWC=(( ( (( m68k_src_w)&( m68k_dst_w)) |
        ((~resultl)&( m68k_dst_w))|
        (( m68k_src_w)&(~resultl)) ) & MSB_W)!=0);
  if(resultl)
    CLEAR_Z;
  PSWN=(resultl<0);
  if(toreg)
    REGW(PARAM_N)=resultl;
  else
  {
    dbus=resultl;
    CPU_BUS_ACCESS_WRITE; //nw
  }
}


void m68k_add_w_from_dN() {
/*
Dn,<ea> :         |                 |              /              |
  .B or .W :      |                 |             |               |
    (An)          |  8(1/1)  4(1/0) |             |            nr | np nw       
    (An)+         |  8(1/1)  4(1/0) |             |            nr | np nw       
    -(An)         |  8(1/1)  6(1/0) |             | n          nr | np nw       
    (d16,An)      |  8(1/1)  8(2/0) |             |      np    nr | np nw       
    (d8,An,Xn)    |  8(1/1) 10(2/0) |             | n    np    nr | np nw       
    (xxx).W       |  8(1/1)  8(2/0) |             |      np    nr | np nw       
    (xxx).L       |  8(1/1) 12(3/0) |             |   np np    nr | np nw       
*/
  m68k_src_w=REGW(PARAM_N);
  m68k_GET_DEST_W_NOT_A; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  resultl=m68k_dst_w+m68k_src_w;
  SR_ADD_W;
  dbus=resultl;
  CPU_BUS_ACCESS_WRITE; //nw
}


void m68k_addx_l() {
/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
    ADDX, SUBX    |      INSTR      |                  INSTR
------------------+-----------------+------------------------------------------
*/
  bool toreg=(DEST_IS_DATA_REGISTER);
  if(toreg)
  {
/*
Dy,Dx :           |                 |
  .L :            |  8(1/0)         |                               np       nn
*/
    CHECK_IPL;
    PREFETCH_FINAL; //np
    CPU_BUS_IDLE(4); //nn
    m68k_src_l=REGL(PARAM_M);
    m68k_dst_l=REGL(PARAM_N);
  }
  else
  {
/*
-(Ay),-(Ax) :     |                 |
  .L :            | 30(5/2)         |              n nr nR nr nR nw np    nW
*/
    CPU_BUS_IDLE(2); //n
    AREG(PARAM_M)-=2;
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ; //nr
    m68k_src_ll=dbus;
    AREG(PARAM_M)-=2;
    iabus=AREG(PARAM_M);
    CPU_BUS_ACCESS_READ; //nR
    m68k_src_lh=dbus;
    AREG(PARAM_N)-=2;
    iabus=AREG(PARAM_N);
    CPU_BUS_ACCESS_READ; //nr
    m68k_dst_ll=dbus;
    AREG(PARAM_N)-=2;
    iabus=AREG(PARAM_N);
    CPU_BUS_ACCESS_READ; //nR
    m68k_dst_lh=dbus;
  }//if
  TRUE_PC=pc+2;
  result=m68k_dst_l+m68k_src_l;
  if(PSWX)
    result++;
  PSWV=(( ( (( m68k_src_l)&( m68k_dst_l)&(~result))|
        ((~m68k_src_l)&(~m68k_dst_l)&( result)) ) & MSB_L)!=0);
  PSWX=PSWC=(( ( (( m68k_src_l)&( m68k_dst_l)) | ((~result)&( m68k_dst_l))|
        (( m68k_src_l)&(~result)) ) & MSB_L)!=0);
  if(result) 
    CLEAR_Z;
  PSWN=(result<0);
  if(toreg)
    REGL(PARAM_N)=result;
  else
  {
    iabus+=2;
    dbus=resultl;
    CPU_BUS_ACCESS_WRITE; //nw
    CHECK_IPL;
    PREFETCH_FINAL; //np 
    dbus=resulth;
    iabus-=2;
    CPU_BUS_ACCESS_WRITE; //nW
  }
}


void m68k_add_l_from_dN() {
/*  ADD Dn,<ea>
  .L :            |                 |             |               |
    (An)          | 12(1/2)  8(2/0) |             |            nr | np nw nW    
    (An)+         | 12(1/2)  8(2/0) |             |            nr | np nw nW    
    -(An)         | 12(1/2) 10(2/0) |             | n       nR nr | np nw nW    
    (d16,An)      | 12(1/2) 12(3/0) |             |      np nR nr | np nw nW    
    (d8,An,Xn)    | 12(1/2) 14(3/0) |             | n    np nR nr | np nw nW    
    (xxx).W       | 12(1/2) 12(3/0) |             |      np nR nr | np nw nW    
    (xxx).L       | 12(1/2) 16(4/0) |             |   np np nR nr | np nw nW    
*/
  m68k_src_l=Cpu.r[PARAM_N];
  m68k_GET_DEST_L_NOT_A; //EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  result=m68k_dst_l+m68k_src_l;
  SR_ADD_L;
  WRITE_LONG_BACKWARDS; //nw nW
}


void m68k_adda_l() {
/*
------------------------------------------------------------------------------- 
                  |    Exec Time    |               Data Bus Usage              
    ADDA, SUBA    |  INSTR     EA   |  1st OP (ea)  |          INSTR            
------------------+-----------------+---------------+-------------------------- 
<ea>,An :         |                 |               |
  .L :            |                 |               |
    Dn            |  8(1/0)  0(0/0) |               |               np       nn 
    An            |  8(1/0)  0(0/0) |               |               np       nn 
    (An)          |  6(1/0)  8(2/0) |         nR nr |               np       n  
    (An)+         |  6(1/0)  8(2/0) |         nR nr |               np       n  
    -(An)         |  6(1/0) 10(2/0) | n       nR nr |               np       n  
    (d16,An)      |  6(1/0) 12(3/0) |      np nR nr |               np       n  
    (d8,An,Xn)    |  6(1/0) 14(3/0) | n    np nR nr |               np       n  
    (xxx).W       |  6(1/0) 12(3/0) |      np nR nr |               np       n  
    (xxx).L       |  6(1/0) 16(4/0) |   np np nR nr |               np       n  
    #<data>       |  8(1/0)  8(2/0) |   np np       |               np       nn 
*/
  m68k_GET_SOURCE_L;  // EA
  CHECK_IPL;
  PREFETCH_FINAL;  //np
  if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
    CPU_BUS_IDLE(4); //nn
  else
    CPU_BUS_IDLE(2); //n
  AREG(PARAM_N)+=m68k_src_l;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  line e - bit shift      ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
*******************************************************************************
  Line 1110
    ASL, ASR, LSL, LSR, ROL, ROR, ROXL, ROXR
*******************************************************************************
-------------------------------------------------------------------------------
     ASL, ASR,    |    Exec Time    |               Data Bus Usage              
     LSL, LSR,    |                 |
     ROL, ROR,    |                 |
    ROXL, ROXR    |  INSTR     EA   |  1st OP (ea)  |          INSTR           
------------------+-----------------+---------------+--------------------------
Dx,Dy :           |                 |               | 
  .B or .W :      |  6+2m(1/0)      |               |               np    n* n  
  .L :            |  8+2m(1/0)      |               |               np    n* nn 
#<data>,Dy :      |                 |               |
  .B or .W :      |  6+2m(1/0)      |               |               np    n  n* 
  .L :            |  8+2m(1/0)      |               |               np    nn n* 
<ea> :            |                 |               |
  .B or .W :      |                 |               |
    (An)          |  8(1/1)  4(1/0) |            nr |               np    nw    
    (An)+         |  8(1/1)  4(1/0) |            nr |               np    nw    
    -(An)         |  8(1/1)  6(1/0) | n          nr |               np    nw    
    (d16,An)      |  8(1/1)  8(2/0) |      np    nr |               np    nw    
    (d8,An)       |  8(1/1) 10(2/0) | n    np    nr |               np    nw    
    (xxx).W       |  8(1/1)  8(2/0) |      np    nr |               np    nw    
    (xxx).L       |  8(1/1) 12(3/0) |   np np    nr |               np    nw    
NOTES :
  .'m' is the shift count.
  .'n*' should be replaced by m consecutive 'n'
*/

inline void m68k_bit_shift_to_dm_get_source() {
  if(IRD&BIT_5)
    m68k_src_w=(WORD)(Cpu.r[PARAM_N]&63);
  else
  {
    m68k_src_w=(WORD)PARAM_N;
    if(!m68k_src_w)
      m68k_src_w=8;
  }
}
#define m68k_BIT_SHIFT_TO_dM_GET_SOURCE m68k_bit_shift_to_dm_get_source()


#define INSTRUCTION_TIME_SHIFT(n) \
{  for(int i=0;i<(m68k_src_w+n/2);i++) \
    CPU_BUS_IDLE(2); \
}


void m68k_asr_b_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; // EA
  CHECK_IPL;
  m68k_dst_b=REGB(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  if(m68k_src_w>31)
    m68k_src_w=31;
  CLEAR_VC;
  if(m68k_src_w)
  {
    if(m68k_dst_b & (BYTE)(1<<MIN(m68k_src_w-1,7)))
      SET_XC;
    else
      CLEAR_X;
  }
  resultb=m68k_dst_b>>m68k_src_w;
  SR_CHECK_Z_AND_N_B;
  REGB(PARAM_M)=resultb;
}


void m68k_lsr_b_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; // EA
  CHECK_IPL;
  m68k_dst_b=REGB(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  if(m68k_src_w>31)
    m68k_src_w=31;
  CLEAR_VC;
  if(m68k_src_w)
  {
    if(m68k_src_w>8)
      CLEAR_X;
    else
    {
      if(m68k_dst_b&(BYTE)(1<<(m68k_src_w-1)))
        SET_XC;
      else
        CLEAR_X;
    }
  }
  resultb=((BYTE)m68k_dst_b)>>m68k_src_w;
  SR_CHECK_Z_AND_N_B;
  REGB(PARAM_M)=resultb;
}


void m68k_roxr_b_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_b=REGB(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  CLEAR_V;
  PSWC=PSWX;
  resultb=m68k_dst_b;
  for(int n=0;n<m68k_src_w;n++)
  {
    BYTE old_x=PSWX;
    PSWX=PSWC=(resultb&1);
    resultb=((BYTE)resultb)>>1;
    if(old_x)
      resultb|=MSB_B;
  }
  SR_CHECK_Z_AND_N_B;
  REGB(PARAM_M)=resultb;
}


void m68k_ror_b_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_b=REGB(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  CLEAR_VC;
  resultb=m68k_dst_b;
  for(int n=0;n<m68k_src_w;n++)
  {
    PSWC=(resultb&1);
    resultb=((BYTE)resultb)>>1;
    if(PSWC)
      resultb|=MSB_B;
  }
  SR_CHECK_Z_AND_N_B;
  REGB(PARAM_M)=resultb;
}


void m68k_asr_w_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_w=REGW(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  if(m68k_src_w>31)
    m68k_src_w=31;
  CLEAR_VC;
  resultl=m68k_dst_w;
  if(m68k_src_w)
  {
    if(m68k_dst_w &(WORD)(1<<MIN(m68k_src_w-1,15)))
      SET_XC;
    else
      CLEAR_X;
    resultl=m68k_dst_w>>m68k_src_w;
  }
  SR_CHECK_Z_AND_N_W;
  REGW(PARAM_M)=resultl;
}


void m68k_lsr_w_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_w=REGW(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  if(m68k_src_w>31)
    m68k_src_w=31;
  CLEAR_VC;
  if(m68k_src_w)
  {
    if(m68k_src_w>16)
      CLEAR_X;
    else
    {
      if(m68k_dst_w & (WORD)(1<<(m68k_src_w-1)))
        SET_XC;
      else
        CLEAR_X;
    }
  }
  resultl=((WORD)m68k_dst_w)>>m68k_src_w;
  SR_CHECK_Z_AND_N_W;
  REGW(PARAM_M)=resultl;
}


void m68k_roxr_w_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_w=REGW(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  CLEAR_V;
  PSWC=PSWX;
  resultl=m68k_dst_w;
  for(int n=0;n<m68k_src_w;n++)
  {
    BYTE old_x=PSWX;
    PSWX=PSWC=(resultl&1);
    resultl=((WORD)resultl)>>1;
    if(old_x)
      resultl|=MSB_W;
  }
  SR_CHECK_Z_AND_N_W;
  REGW(PARAM_M)=resultl;
}


void m68k_ror_w_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_w=REGW(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  CLEAR_VC;
  resultl=m68k_dst_w;
  for(int n=0;n<m68k_src_w;n++)
  {
    PSWC=(resultl&1);
    resultl=((WORD)resultl)>>1;
    if(PSWC)
      resultl|=MSB_W;
  }
  SR_CHECK_Z_AND_N_W;
  REGW(PARAM_M)=resultl;
}


void m68k_asr_l_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_l=REGL(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(4);
  CLEAR_VC;
  result=m68k_dst_l;
  if(m68k_src_w)
  {
    // If shift by 31 or more then test MSB as this is copied to the whole long
    if(m68k_dst_l & (1<<MIN(m68k_src_w-1,31)))
      SET_XC;
    else
      CLEAR_X;
    // Because MSB->LSB, MSB has been copied to all other bits so 1 extra shift
    // will make no difference.
    if(m68k_src_w>31) 
      m68k_src_w=31;
    result=m68k_dst_l>>m68k_src_w;
  }
  SR_CHECK_Z_AND_N_L;
  REGL(PARAM_M)=result;
}


void m68k_lsr_l_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_l=REGL(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(4);
  CLEAR_VC;
  if(m68k_src_w)
  {
    if(m68k_src_w>32)
      CLEAR_X;
    else
      PSWX=PSWC=((m68k_dst_l&(DWORD)(1<<(m68k_src_w-1)))!=0);
  }
  result=((DWORD)m68k_dst_l)>>m68k_src_w;
  if(m68k_src_w>31)
    result=0;
  SR_CHECK_Z_AND_N_L;
  REGL(PARAM_M)=result;
}


void m68k_roxr_l_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_l=REGL(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(4);
  CLEAR_V;
  PSWC=PSWX;
  result=m68k_dst_l;
  for(int n=0;n<m68k_src_w;n++)
  {
    BYTE old_x=PSWX;
    PSWX=PSWC=(result&1);
    result=((DWORD)result)>>1;
    if(old_x)
      result|=MSB_L;
  }
  SR_CHECK_Z_AND_N_L;
  REGL(PARAM_M)=result;
}


void m68k_ror_l_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_l=REGL(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(4);
  CLEAR_VC;
  result=m68k_dst_l;
  for(int n=0;n<m68k_src_w;n++)
  {
    PSWC=(result&1);
    result=((DWORD)result)>>1;
    if(PSWC)
      result|=MSB_L;
  }
  SR_CHECK_Z_AND_N_L;
  REGL(PARAM_M)=result;
}


void m68k_asl_b_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_b=REGB(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  if(m68k_src_w>31)
    m68k_src_w=31;
  CLEAR_VC;
  if(m68k_src_w)
  {
    CLEAR_X;
    if(m68k_src_w<=8)
    {
      if(m68k_dst_b & (BYTE)(MSB_B>>(m68k_src_w-1)))
        SET_XC;
    }
    if(m68k_src_w<=7)
    {
      signed char mask=(signed char)(((signed char)(MSB_B))>>(m68k_src_w));
      // mask:  m  m-1 m-2 m-3 ... m-r m-r-1 ...
      //        1   1   1   1       1   0    0...
      if((mask&(m68k_dst_b))!=0 && ((mask&(m68k_dst_b))^mask)!=0)
        SET_V;
    }
    else if(m68k_dst_b)
      SET_V;
  }
  resultb=m68k_dst_b<<m68k_src_w;
  SR_CHECK_Z_AND_N_B;
  REGB(PARAM_M)=resultb;
}


void m68k_lsl_b_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_b=REGB(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  if(m68k_src_w>31)
    m68k_src_w=31;
  CLEAR_VC;
  if(m68k_src_w)
  {
    CLEAR_X;
    if(m68k_src_w<=8)
    {
      if(m68k_dst_b&(BYTE)(MSB_B>>(m68k_src_w-1)))
        SET_XC;
    }
  }
  resultb=((BYTE)m68k_dst_b)<<m68k_src_w;
  SR_CHECK_Z_AND_N_B;
  REGB(PARAM_M)=resultb;
}


void m68k_roxl_b_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_b=REGB(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  CLEAR_VC;
  if(PSWX)
    SET_C;
  resultb=m68k_dst_b;
  for(int n=0;n<m68k_src_w;n++)
  {
    BYTE old_x=PSWX;
    PSWX=PSWC=((resultb&MSB_B)!=0);
    resultb=((BYTE)resultb)<<1;
    if(old_x)
      resultb|=1;
  }
  SR_CHECK_Z_AND_N_B;
  REGB(PARAM_M)=resultb;
}


void m68k_rol_b_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_b=REGB(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  CLEAR_VC;
  resultb=m68k_dst_b;
  for(int n=0;n<m68k_src_w;n++)
  {
    PSWC=((resultb&MSB_B)!=0);
    resultb=((BYTE)resultb)<<1;
    if(PSWC)
      resultb|=1;
  }
  SR_CHECK_Z_AND_N_B;
  REGB(PARAM_M)=resultb;
}


void m68k_asl_w_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_w=REGW(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  if(m68k_src_w>31)
    m68k_src_w=31;
  CLEAR_VC;
  if(m68k_src_w)
  {
    CLEAR_X;
    if(m68k_src_w<=16)
    {
      if(m68k_dst_w&(WORD)(MSB_W>>(m68k_src_w-1)))
        SET_XC;
    }
    if(m68k_src_w<=15)
    {
      signed short mask=(signed short)(((signed short)(MSB_W))>>(m68k_src_w));
      if((mask&(m68k_dst_w))!=0&&((mask&(m68k_dst_w))^mask)!=0)
        SET_V;
    }
    else if(m68k_dst_w)
      SET_V;
  }
  resultl=m68k_dst_w<<m68k_src_w;
  SR_CHECK_Z_AND_N_W;
  REGW(PARAM_M)=resultl;
}


void m68k_lsl_w_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_w=REGW(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  if(m68k_src_w>31)
    m68k_src_w=31;
  CLEAR_VC;
  if(m68k_src_w)
  {
    CLEAR_X;
    if(m68k_src_w<=16)
    {
      if(m68k_dst_w&(WORD)(MSB_W>>(m68k_src_w-1)))
        SET_XC;
    }
  }
  resultl=((WORD)m68k_dst_w)<<m68k_src_w;
  SR_CHECK_Z_AND_N_W;
  REGW(PARAM_M)=resultl;
}


void m68k_roxl_w_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_w=REGW(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  CLEAR_V;
  PSWC=PSWX;
    
  resultl=m68k_dst_w;
  for(int n=0;n<m68k_src_w;n++)
  {
    BYTE old_x=PSWX;
    PSWX=PSWC=((resultl&MSB_W)!=0);
    resultl=((WORD)resultl)<<1;
    if(old_x)
      resultl|=1;
  }
  SR_CHECK_Z_AND_N_W;
  REGW(PARAM_M)=resultl;
}


void m68k_rol_w_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_w=REGW(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(2);
  CLEAR_VC;
  resultl=m68k_dst_w;
  for(int n=0;n<m68k_src_w;n++)
  {
    PSWC=((resultl&MSB_W)!=0);
    resultl=((WORD)resultl)<<1;
    if(PSWC)
      resultl|=1;
  }
  SR_CHECK_Z_AND_N_W;
  REGW(PARAM_M)=resultl;
}


void m68k_asl_l_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  CHECK_IPL;
  m68k_dst_l=REGL(PARAM_M);
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(4);
  CLEAR_VC;
  if(m68k_src_w)
  {
    CLEAR_X;
    if(m68k_src_w<=32)
    {
      if(m68k_dst_l&(LONG)(MSB_L>>(m68k_src_w-1)))
        SET_XC;
    }
    if(m68k_src_w<=31)
    {
      signed long mask=(((signed long)(MSB_L))>>(m68k_src_w));
      if((mask&(m68k_dst_l))!=0 && ((mask&(m68k_dst_l))^mask)!=0)
        SET_V;
    }
    else if(m68k_dst_l)
      SET_V;
  }
  result=m68k_dst_l<<m68k_src_w;
  if(m68k_src_w>31)
    result=0;
  SR_CHECK_Z_AND_N_L;
  REGL(PARAM_M)=result;
}


void m68k_lsl_l_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  m68k_dst_l=REGL(PARAM_M);
  CHECK_IPL;
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(4);
  CLEAR_VC;
  if(m68k_src_w)
  {
    CLEAR_X;
    if(m68k_src_w<=32)
    {
      if(m68k_dst_l&(LONG)(MSB_L>>(m68k_src_w-1)))
        SET_XC;
    }
  }
  result=((DWORD)m68k_dst_l)<<m68k_src_w;
  if(m68k_src_w>31)
    result=0;
  SR_CHECK_Z_AND_N_L;
  REGL(PARAM_M)=result;
}


void m68k_roxl_l_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  m68k_dst_l=REGL(PARAM_M);
  CHECK_IPL;
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(4);
  CLEAR_V;
  PSWC=PSWX;
  result=m68k_dst_l;
  for(int n=0;n<m68k_src_w;n++)
  {
    BYTE old_x=PSWX;
    PSWX=PSWC=((result&MSB_L)!=0);
    result=((DWORD)result)<<1;
    if(old_x)
      result|=1;
  }
  SR_CHECK_Z_AND_N_L;
  REGL(PARAM_M)=result;
}


void m68k_rol_l_to_dM() {
  m68k_BIT_SHIFT_TO_dM_GET_SOURCE; //EA
  m68k_dst_l=REGL(PARAM_M);
  CHECK_IPL;
  PREFETCH_FINAL; //np
  INSTRUCTION_TIME_SHIFT(4);
  CLEAR_VC;
  result=m68k_dst_l;
  for(int n=0;n<m68k_src_w;n++)
  {
    PSWC=((result&MSB_L)!=0);
    result=((DWORD)result)<<1;
    if(PSWC)
      result|=1;
  }
  SR_CHECK_Z_AND_N_L;
  REGL(PARAM_M)=result;
}


void m68k_bit_shift_right_to_mem() {
  m68k_GET_DEST_W_NOT_A_OR_D; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  switch(IRD&BITS_ba9) {
  case BITS_ba9_000: // arithmetic shift ASR
    CLEAR_V;
    PSWX=PSWC=(m68k_dst_w&1);
    resultl=m68k_dst_w>>1;
    break;
  case BITS_ba9_001: // logical shift LSR
    CLEAR_V;
    PSWX=PSWC=(m68k_dst_w&1);
    resultl=((WORD)m68k_dst_w)>>1;
    break;
  case BITS_ba9_010: // rotate with extend RORX
  {
    CLEAR_V;
    BYTE old_x=PSWX;
    if(m68k_dst_w&1)
      SET_XC;
    else
      CLEAR_XC; // StarRay
    resultl=((WORD)m68k_dst_w)>>1;
    if(old_x)
      resultl|=MSB_W;
    break;
  }
  case BITS_ba9_011: // rotate without extend ROR
    CLEAR_V;
    PSWC=(m68k_dst_w&1);
    resultl=((WORD)m68k_dst_w)>>1;
    if(PSWC)
      resultl|=MSB_W;
    break;
  }
  SR_CHECK_Z_AND_N_W;
  dbus=resultl;
  CPU_BUS_ACCESS_WRITE; //nw
}


void m68k_bit_shift_left_to_mem() {
  m68k_GET_DEST_W_NOT_A_OR_D; // EA
  TRUE_PC=pc+2;
  CHECK_IPL;
  PREFETCH_FINAL; //np
  switch(IRD&BITS_ba9) {
  case BITS_ba9_000: //asl
    PSWX=PSWC=((m68k_dst_w&(WORD)(MSB_W))!=0);
    PSWV=((m68k_dst_w&0xc000)==0x8000||(m68k_dst_w&0xc000)==0x4000);
    resultl=m68k_dst_w<<1;
    break;
  case BITS_ba9_001: //LSL
    CLEAR_V;
    PSWX=PSWC=((m68k_dst_w&MSB_W)!=0);
    resultl=((WORD)m68k_dst_w)<<1;
    break;
  case BITS_ba9_010: //ROXL
  {
    CLEAR_V;
    BYTE old_x=PSWX;
    PSWX=PSWC=((m68k_dst_w&MSB_W)!=0);
    resultl=((WORD)m68k_dst_w)<<1;
    if(old_x)
      resultl|=1;
    break;
  }
  case BITS_ba9_011: //ROL
    CLEAR_V;
    PSWC=(m68k_dst_w&MSB_W)!=0;
    resultl=((WORD)m68k_dst_w)<<1;
    if(PSWC)
      resultl|=1;
    break;
  }
  SR_CHECK_Z_AND_N_W;
  dbus=resultl;
  CPU_BUS_ACCESS_WRITE; //nw
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////  HIGH NIBBLE ROUTINES    ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


/*
*******************************************************************************
  Line 0001         &          Line 0010          &          Line 0011
    MOVE.B                       MOVE.L, MOVEA.L               MOVE.W, MOVEA.W
*******************************************************************************

-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       MOVE       |      INSTR      |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
<ea>,Dn :         |                 |               |
  .B or .W :      |                 |               |
    Dn            |  4(1/0)         |               |               np		   
    An            |  4(1/0)         |               |               np		   
    (An)          |  8(2/0)         |            nr |               np		   
    (An)+         |  8(2/0)         |            nr |               np		   
    -(An)         | 10(2/0)         | n          nr |               np		   
    (d16,An)      | 12(3/0)         |      np    nr |               np		   
    (d8,An,Xn)    | 14(3/0)         | n    np    nr |               np		   
    (xxx).W       | 12(3/0)         |      np    nr |               np		   
    (xxx).L       | 16(4/0)         |   np np    nr |               np		 
    #<data>       |  8(2/0)         |      np       |               np		 
  .L :            |                 |               |            
    Dn            |  4(1/0)         |               |               np		     
    An            |  4(1/0)         |               |               np		    
    (An)          | 12(3/0)         |         nR nr |               np		     
    (An)+         | 12(3/0)         |         nR nr |               np		      
    -(An)         | 14(3/0)         | n       nR nr |               np		      
    (d16,An)      | 16(4/0)         |      np nR nr |               np		      
    (d8,An,Xn)    | 18(4/0)         | n    np nR nr |               np		      
    (xxx).W       | 16(4/0)         |      np nR nr |               np		      
    (xxx).L       | 20(5/0)         |   np np nR nr |               np		      
    #<data>       | 12(3/0)         |   np np       |               np		      
<ea>,(An) :       |                 |               |
  .B or .W :      |                 |               |
    Dn            |  8(1/1)         |               |            nw np          
    An            |  8(1/1)         |               |            nw np          
    (An)          | 12(2/1)         |            nr |            nw np          
    (An)+         | 12(2/1)         |            nr |            nw np          
    -(An)         | 14(2/1)         | n          nr |            nw np          
    (d16,An)      | 16(3/1)         |      np    nr |            nw np          
    (d8,An,Xn)    | 18(3/1)         | n    np    nr |            nw np          
    (xxx).W       | 16(3/1)         |      np    nr |            nw np          
    (xxx).L       | 20(4/1)         |   np np    nr |            nw np          
    #<data>       | 12(2/1)         |      np       |            nw np          
  .L :            |                 |               |
    Dn            | 12(1/2)         |               |         nW nw np		 
    An            | 12(1/2)         |               |         nW nw np		 
    (An)          | 20(3/2)         |         nR nr |         nW nw np		 
    (An)+         | 20(3/2)         |         nR nr |         nW nw np		 
    -(An)         | 22(3/2)         | n       nR nr |         nW nw np		 
    (d16,An)      | 24(4/2)         |      np nR nr |         nW nw np		 
    (d8,An,Xn)    | 26(4/2)         | n    np nR nr |         nW nw np		 
    (xxx).W       | 24(4/2)         |      np nR nr |         nW nw np		 
    (xxx).L       | 28(5/2)         |   np np nR nr |         nW nw np		 
    #<data>       | 20(3/2)         |   np np       |         nW nw np		 
<ea>,(An)+ :      |                 |               |
  .B or .W :      |                 |               |
    Dn            |  8(1/1)         |               |            nw np          
    An            |  8(1/1)         |               |            nw np          
    (An)          | 12(2/1)         |            nr |            nw np          
    (An)+         | 12(2/1)         |            nr |            nw np          
    -(An)         | 14(2/1)         | n          nr |            nw np          
    (d16,An)      | 16(3/1)         |      np    nr |            nw np          
    (d8,An,Xn)    | 18(3/1)         | n    np    nr |            nw np          
    (xxx).W       | 16(3/1)         |      np    nr |            nw np          
    (xxx).L       | 20(4/1)         |   np np    nr |            nw np          
    #<data>       | 12(2/1)         |      np       |            nw np          
  .L :            |                 |               |                           
    Dn            | 12(1/2)         |               |         nW nw np          
    An            | 12(1/2)         |               |         nW nw np          
    (An)          | 20(3/2)         |         nR nr |         nW nw np          
    (An)+         | 20(3/2)         |         nR nr |         nW nw np          
    -(An)         | 22(3/2)         | n       nR nr |         nW nw np          
    (d16,An)      | 24(4/2)         |      np nR nr |         nW nw np          
    (d8,An,Xn)    | 26(4/2)         | n    np nR nr |         nW nw np          
    (xxx).W       | 24(4/2)         |      np nR nr |         nW nw np          
    (xxx).L       | 28(5/2)         |   np np nR nr |         nW nw np          
    #<data>       | 20(3/2)         |   np np       |         nW nw np          
<ea>,-(An) :      |                 |               |
  .B or .W :      |                 |               |
    Dn            |  8(1/1)         |               |                  np nw    
    An            |  8(1/1)         |               |                  np nw    
    (An)          | 12(2/1)         |            nr |                  np nw    
    (An)+         | 12(2/1)         |            nr |                  np nw    
    -(An)         | 14(2/1)         | n          nr |                  np nw    
    (d16,An)      | 16(3/1)         |      np    nr |                  np nw    
    (d8,An,Xn)    | 18(3/1)         | n    np    nr |                  np nw    
    (xxx).W       | 16(3/1)         |      np    nr |                  np nw    
    (xxx).L       | 20(4/1)         |   np np    nr |                  np nw    
    #<data>       | 12(2/1)         |      np       |                  np nw    
  .L :            |                 |               |                           
    Dn            | 12(1/2)         |               |                  np nw nW 
    An            | 12(1/2)         |               |                  np nw nW 
    (An)          | 20(3/2)         |         nR nr |                  np nw nW 
    (An)+         | 20(3/2)         |         nR nr |                  np nw nW 
    -(An)         | 22(3/2)         | n       nR nr |                  np nw nW 
    (d16,An)      | 24(4/2)         |      np nR nr |                  np nw nW 
    (d8,An,Xn)    | 26(4/2)         | n    np nR nr |                  np nw nW 
    (xxx).W       | 24(4/2)         |      np nR nr |                  np nw nW 
    (xxx).L       | 28(5/2)         |   np np nR nr |                  np nw nW 
    #<data>       | 20(3/2)         |   np np       |                  np nw nW 
<ea>,(d16,An) :   |                 |               |
  .B or .W :      |                 |               |
    Dn            | 12(2/1)         |               |      np    nw np        
    An            | 12(2/1)         |               |      np    nw np		      
    (An)          | 16(3/1)         |            nr |      np    nw np		      
    (An)+         | 16(3/1)         |            nr |      np    nw np		      
    -(An)         | 18(3/1)         | n          nr |      np    nw np		      
    (d16,An)      | 20(4/1)         |      np    nr |      np    nw np		      
    (d8,An,Xn)    | 22(4/1)         | n    np    nr |      np    nw np		      
    (xxx).W       | 20(4/1)         |      np    nr |      np    nw np		      
    (xxx).L       | 24(5/1)         |   np np    nr |      np    nw np		      
    #<data>       | 16(3/1)         |      np       |      np    nw np		      
  .L :            |                 |               |
    Dn            | 16(2/2)         |               |      np nW nw np		      
    An            | 16(2/2)         |               |      np nW nw np		      
    (An)          | 24(4/2)         |         nR nr |      np nW nw np          
    (An)+         | 24(4/2)         |         nR nr |      np nW nw np          
    -(An)         | 26(4/2)         | n       nR nr |      np nW nw np          
    (d16,An)      | 28(5/2)         |      np nR nr |      np nW nw np          
    (d8,An,Xn)    | 30(5/2)         | n    np nR nr |      np nW nw np          
    (xxx).W       | 28(5/2)         |      np nR nr |      np nW nw np          
    (xxx).L       | 32(6/2)         |   np np nR nr |      np nW nw np     
    #<data>       | 24(4/2)         |   np np       |      np nW nw np		   
<ea>,(d8,An,Xn) : |                 |               |
  .B or .W :      |                 |               |
    Dn            | 14(2/1)         |               | n    np    nw np		   
    An            | 14(2/1)         |               | n    np    nw np		   
    (An)          | 18(3/1)         |            nr | n    np    nw np		   
    (An)+         | 18(3/1)         |            nr | n    np    nw np		   
    -(An)         | 20(3/1)         | n          nr | n    np    nw np		   
    (d16,An)      | 22(4/1)         |      np    nr | n    np    nw np		   
    (d8,An,Xn)    | 24(4/1)         | n    np    nr | n    np    nw np		   
    (xxx).W       | 22(4/1)         |      np    nr | n    np    nw np		   
    (xxx).L       | 26(5/1)         |   np np    nr | n    np    nw np		   
    #<data>       | 18(3/1)         |      np       | n    np    nw np		   
  .L :            |                 |               |
    Dn            | 18(2/2)         |               | n    np nW nw np		   
    An            | 18(2/2)         |               | n    np nW nw np		   
    (An)          | 26(4/2)         |         nR nr | n    np nW nw np          
    (An)+         | 26(4/2)         |         nR nr | n    np nW nw np          
    -(An)         | 28(4/2)         | n       nR nr | n    np nW nw np          
    (d16,An)      | 30(5/2)         |      np nR nr | n    np nW nw np          
    (d8,An,Xn)    | 32(5/2)         | n    np nR nr | n    np nW nw np          
    (xxx).W       | 30(5/2)         |      np nR nr | n    np nW nw np          
    (xxx).L       | 34(6/2)         |   np np nR nr | n    np nW nw np      
    #<data>       | 26(4/2)         |   np np       | n    np nW nw np		    
<ea>,(xxx).W :    |                 |               |
  .B or .W :      |                 |               |
    Dn            | 12(2/1)         |               |      np    nw np		    
    An            | 12(2/1)         |               |      np    nw np		    
    (An)          | 16(3/1)         |            nr |      np    nw np		    
    (An)+         | 16(3/1)         |            nr |      np    nw np		    
    -(An)         | 18(3/1)         | n          nr |      np    nw np		    
    (d16,An)      | 20(4/1)         |      np    nr |      np    nw np		    
    (d8,An,Xn)    | 22(4/1)         | n    np    nr |      np    nw np		    
    (xxx).W       | 20(4/1)         |      np    nr |      np    nw np		    
    (xxx).L       | 24(5/1)         |   np np    nr |      np    nw np		    
    #<data>       | 16(3/1)         |      np       |      np    nw np		    
  .L :            |                 |               |
    Dn            | 16(2/2)         |               |      np nW nw np		    
    An            | 16(2/2)         |               |      np nW nw np		    
    (An)          | 24(4/2)         |         nR nr |      np nW nw np      
    (An)+         | 24(4/2)         |         nR nr |      np nW nw np          
    -(An)         | 26(4/2)         | n       nR nr |      np nW nw np          
    (d16,An)      | 28(5/2)         |      np nR nr |      np nW nw np          
    (d8,An,Xn)    | 30(5/2)         | n    np nR nr |      np nW nw np          
    (xxx).W       | 28(5/2)         |      np nR nr |      np nW nw np          
    (xxx).L       | 32(6/2)         |   np np nR nr |      np nW nw np      
    #<data>       | 24(4/2)         |   np np       |      np nW nw np		    
<ea>,(xxx).L :    |                 |               |
  .B or .W :      |                 |               |
    Dn            | 16(3/1)         |               |   np np    nw np		    
    An            | 16(3/1)         |               |   np np    nw np		    
    (An)          | 20(4/1)         |            nr |      np    nw np np	  
    (An)+         | 20(4/1)         |            nr |      np    nw np np	  
    -(An)         | 22(4/1)         | n          nr |      np    nw np np	  
    (d16,An)      | 24(5/1)         |      np    nr |      np    nw np np	  
    (d8,An,Xn)    | 26(5/1)         | n    np    nr |      np    nw np np	  
    (xxx).W       | 24(5/1)         |      np    nr |      np    nw np np	  
    (xxx).L       | 28(6/1)         |   np np    nr |      np    nw np np	  
    #<data>       | 20(4/1)         |      np       |   np np    nw np		    
  .L :            |                 |               |
    Dn            | 20(3/2)         |               |   np np nW nw np		    
    An            | 20(3/2)         |               |   np np nW nw np		    
    (An)          | 28(5/2)         |         nR nr |      np nW nw np np   
    (An)+         | 28(5/2)         |         nR nr |      np nW nw np np   
    -(An)         | 30(5/2)         | n       nR nr |      np nW nw np np       
    (d16,An)      | 32(6/2)         |      np nR nr |      np nW nw np np       
    (d8,An,Xn)    | 34(6/2)         | n    np nR nr |      np nW nw np np       
    (xxx).W       | 32(6/2)         |      np nR nr |      np nW nw np np       
    (xxx).L       | 36(7/2)         |   np np nR nr |      np nW nw np np       
    #<data>       | 28(5/2)         |   np np       |   np np nW nw np          

-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
      MOVEA       |      INSTR      |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
<ea>,An :         |                 |               |
  .W :            |                 |               |
    Dn            |  4(1/0)         |               |               np		      
    An            |  4(1/0)         |               |               np		      
    (An)          |  8(2/0)         |            nr |               np        
    (An)+         |  8(2/0)         |            nr |               np        
    -(An)         | 10(2/0)         | n          nr |               np        
    (d16,An)      | 12(3/0)         |      np    nr |               np        
    (d8,An,Xn)    | 14(3/0)         | n    np    nr |               np        
    (xxx).W       | 12(3/0)         |      np    nr |               np        
    (xxx).L       | 16(4/0)         |   np np    nr |               np        
    #<data>       |  8(2/0)         |      np       |               np		      
  .L :            |                 |               |
    Dn            |  4(1/0)         |               |               np		      
    An            |  4(1/0)         |               |               np		      
    (An)          | 12(3/0)         |         nR nr |               np        
    (An)+         | 12(3/0)         |         nR nr |               np        
    -(An)         | 14(3/0)         | n       nR nr |               np          
    (d16,An)      | 16(4/0)         |      np nR nr |               np          
    (d8,An,Xn)    | 18(4/0)         | n    np nR nr |               np          
    (xxx).W       | 16(4/0)         |      np nR nr |               np        
    (xxx).L       | 20(5/0)         |   np np nR nr |               np        
    #<data>       | 12(3/0)         |   np np       |               np		      
*/


void m68k_move_b() {
  // Source
  m68k_GET_SOURCE_B_NOT_A; // EA
  resultb=m68k_src_b;
  SR_CHECK_AND_B;
  // Destination
  TRUE_PC=pc+2;
  WORD ird876=(IRD&BITS_876);
  if(ird876==BITS_876_000) // to Dn
  {
    CHECK_IPL;
    REGB(PARAM_N)=resultb;
    PREFETCH_FINAL; //np
  }
  else //to memory
  {
    switch(ird876) {
    case BITS_876_010: // (An)
      if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
        CHECK_IPL;
      iabus= AREG(PARAM_N);
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
      if(!SOURCE_IS_REGISTER_OR_IMMEDIATE)
        CHECK_IPL;
      PREFETCH_FINAL; //np
      break;
    case BITS_876_011: // (An)+
    {
      CHECK_IPL;
      iabus=AREG(PARAM_N);
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
      AREG(PARAM_N)++;
      if(rx==7)
        AREG(PARAM_N)++;
      PREFETCH_FINAL; //np
    }
    break;
    case BITS_876_100: // -(An) np nw
    {
      CHECK_IPL;
      PREFETCH_FINAL; //np
      iabus=AREG(PARAM_N);
      iabus--;
      if(rx==7)
        iabus--;
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
      AREG(PARAM_N)=iabus; // updated after write
      break;
    }
    case BITS_876_101: // (d16, An) //np    nw np
      if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
        CHECK_IPL;
      iabus=AREG(PARAM_N)+(signed short)IRC;
      PREFETCH; //np
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
      if(!SOURCE_IS_REGISTER_OR_IMMEDIATE)
        CHECK_IPL;
      PREFETCH_FINAL; //np
      break;
    case BITS_876_110: // (d8, An, Xn) //n    np    nw np	
      CPU_BUS_IDLE(2); //n
      m68k_iriwo=IRC;
      if(m68k_iriwo&BIT_b)   //.l
        iabus=AREG(PARAM_N)+(signed char)LOBYTE(m68k_iriwo)
        +(int)Cpu.r[m68k_iriwo>>12];
      else      //.w
        iabus=AREG(PARAM_N)+(signed char)LOBYTE(m68k_iriwo)
        +(signed short)Cpu.r[m68k_iriwo>>12];
      PREFETCH; //np
      dbus=resultb;
      CPU_BUS_ACCESS_WRITE_B; //nw
      CHECK_IPL;
      PREFETCH_FINAL; //np
      break;
    case BITS_876_111:
      switch(IRD&BITS_ba9) {
      case BITS_ba9_000: // (xxx).W //np    nw np
        iabus=(unsigned long)((signed long)((signed short)IRC));
        PREFETCH; //np
        dbus=resultb;
        CPU_BUS_ACCESS_WRITE_B; //nw
        CHECK_IPL;
        PREFETCH_FINAL; //np
        break;
      case BITS_ba9_001: // (xxx).L 
                         // register or immediate: np np    nw np
                         // other:                 np    nw np np 
/*
<ea>,(xxx).L :    |                 |               |
  .B or .W :      |                 |               |
    Dn            | 16(3/1)         |               |   np np    nw np		    
    An            | 16(3/1)         |               |   np np    nw np		    
    (An)          | 20(4/1)         |            nr |      np    nw np np	  
    (An)+         | 20(4/1)         |            nr |      np    nw np np	  
    -(An)         | 22(4/1)         | n          nr |      np    nw np np	  
    (d16,An)      | 24(5/1)         |      np    nr |      np    nw np np	  
    (d8,An,Xn)    | 26(5/1)         | n    np    nr |      np    nw np np	  
    (xxx).W       | 24(5/1)         |      np    nr |      np    nw np np	  
    (xxx).L       | 28(6/1)         |   np np    nr |      np    nw np np	  
    #<data>       | 20(4/1)         |      np       |   np np    nw np		    

*/
        if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
        {
          TRUE_PC+=2;
          iabush=IRC;
          PREFETCH; //np
          iabusl=IRC;
          PREFETCH; //np
          dbus=resultb;
          CPU_BUS_ACCESS_WRITE_B; //nw
          CHECK_IPL;
          PREFETCH_FINAL; //np
        }
        else
        {
          iabush=IRC;
          PREFETCH; //np
          iabusl=IRC;
          pc+=2; // prefetch is delayed until after write!
          dbus=resultb;
          CPU_BUS_ACCESS_WRITE_B; //nw
          PREFETCH_ONLY; //np
          CHECK_IPL;
          PREFETCH_FINAL; //np
        }
        break;
      }
    }
  }// to memory
}


void m68k_move_l() {
  // Source
  m68k_GET_SOURCE_L; // EA
  // Destination
  TRUE_PC=pc+2;
  WORD ird876=(IRD&BITS_876);
  if(ird876==BITS_876_000) // Dn
  {
    CHECK_IPL;
    result=m68k_src_l;
    SR_CHECK_AND_L;
    REGL(PARAM_N)=result;
    PREFETCH_FINAL; //np
  }
  else if(ird876==BITS_876_001) // An
  {
    CHECK_IPL;
    AREG(PARAM_N)=m68k_src_l; // MOVEA, no flag update
    PREFETCH_FINAL; //np
  }
  else //to memory
  {
    result=m68k_src_l;
    SR_CHECK_AND_L;
    switch(ird876) {
    case BITS_876_010: // (An)
      iabus=AREG(PARAM_N);
      dbus=resulth;
      CPU_BUS_ACCESS_WRITE; //nW
      CHECK_IPL;
      iabus+=2;
      dbus=resultl;
      CPU_BUS_ACCESS_WRITE; //nw
      PREFETCH_FINAL; //np
      break;
    case BITS_876_011: // (An)+
    {
      iabus=AREG(PARAM_N);
      dbus=resulth;
      CPU_BUS_ACCESS_WRITE; //nW
      iabus+=2;
      CHECK_IPL;
      dbus=resultl;
      CPU_BUS_ACCESS_WRITE; //nw
      iabus+=2;
      AREG(PARAM_N)=iabus;
      PREFETCH_FINAL; //np
      break;
    }
    case BITS_876_100: // -(An) //np nw nW
    {
      CHECK_IPL;
      PREFETCH_FINAL; //np
      iabus=AREG(PARAM_N)-2;
      dbus=resultl;
      CPU_BUS_ACCESS_WRITE; //nw
      iabus-=2;
      dbus=resulth;
      CPU_BUS_ACCESS_WRITE; //nW
      AREG(PARAM_N)=iabus; // update after write
      break;
    }
    case BITS_876_101: // (d16, An) //np nW nw np
      iabus=AREG(PARAM_N)+(signed short)IRC;
      PREFETCH; //np
      dbus=resulth;
      CPU_BUS_ACCESS_WRITE; //nW
      if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
        CHECK_IPL;
      iabus+=2;
      dbus=resultl;
      CPU_BUS_ACCESS_WRITE; //nw
      if(!SOURCE_IS_REGISTER_OR_IMMEDIATE) //?
        CHECK_IPL;
      PREFETCH_FINAL;
      break;
    case BITS_876_110: // (d8, An, Xn) //n    np nW nw np
      CPU_BUS_IDLE(2); //n
      m68k_iriwo=IRC;
      if(m68k_iriwo&BIT_b)   //.l
        iabus=AREG(PARAM_N)+(signed char)LOBYTE(m68k_iriwo)
        +(int)Cpu.r[m68k_iriwo>>12];
      else          //.w
        iabus=AREG(PARAM_N)+(signed char)LOBYTE(m68k_iriwo)
        +(signed short)Cpu.r[m68k_iriwo>>12];
      PREFETCH; //np
      dbus=resulth;
      CPU_BUS_ACCESS_WRITE; //nW
      iabus+=2;
      dbus=resultl;
      CPU_BUS_ACCESS_WRITE; //nw
      CHECK_IPL;
      PREFETCH_FINAL;
      break;
    case BITS_876_111:
      switch(IRD&BITS_ba9) {
      case BITS_ba9_000: // (xxx).W //np nW nw np
        iabus=(unsigned long)((signed long)((signed short)IRC)); // sign extension
        PREFETCH; //np
        dbus=resulth;
        CPU_BUS_ACCESS_WRITE; //nW
        iabus+=2;
        dbus=resultl;
        CPU_BUS_ACCESS_WRITE; //nw
        CHECK_IPL;
        PREFETCH_FINAL;
        break;
      case BITS_ba9_001: // (xxx).L
                         // register or immediate:  np np nW nw np
                         // other:                  np nW nw np np (refetch IR)
        if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
        {
          TRUE_PC+=2;
          iabush=IRC;
          PREFETCH; //np
          iabusl=IRC;
          PREFETCH; //np
          dbus=resulth;
          CPU_BUS_ACCESS_WRITE; //nW
          iabus+=2;
          dbus=resultl;
          CPU_BUS_ACCESS_WRITE; //nw
          CHECK_IPL;
          PREFETCH_FINAL;
        }
        else
        {
          iabush=IRC;
          PREFETCH; //np
          iabusl=IRC;
          pc+=2; // prefetch is delayed until after write!
          dbus=resulth;
          CPU_BUS_ACCESS_WRITE; //nW
          //CHECK_IPL;
          iabus+=2;
          dbus=resultl;
          CPU_BUS_ACCESS_WRITE; //nw
          PREFETCH_ONLY; //np
          CHECK_IPL;
          PREFETCH_FINAL; //np
        }
        break;
      }
    }
  }// to memory
}


void m68k_move_w() {
  // Source
  m68k_GET_SOURCE_W; // EA
  // Destination
  TRUE_PC=pc+2;
  WORD ird876=(IRD&BITS_876);
  if(ird876==BITS_876_000) // Dn
  {
    CHECK_IPL;
    resultl=m68k_src_w;
    SR_CHECK_AND_W;
    REGW(PARAM_N)=resultl;
    PREFETCH_FINAL;
  }
  else if(ird876==BITS_876_001) // An
  {
    CHECK_IPL;
    // MOVEA, no flag update, sign extension
    AREG(PARAM_N)=(signed long)((signed short)m68k_src_w);
    PREFETCH_FINAL;
  }
  else //to memory
  {
    resultl=m68k_src_w;
    SR_CHECK_AND_W;
    switch(ird876) {
    case BITS_876_010: // (An)
      if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
        CHECK_IPL;
      iabus=AREG(PARAM_N);
      dbus=resultl;
      CPU_BUS_ACCESS_WRITE; //nw
      if(!SOURCE_IS_REGISTER_OR_IMMEDIATE)
        CHECK_IPL;
      PREFETCH_FINAL;
      break;
    case BITS_876_011:  // (An)+
    {
      CHECK_IPL;
      iabus=AREG(PARAM_N);
      dbus=resultl;
      CPU_BUS_ACCESS_WRITE; //nw
      iabus+=2;
      AREG(PARAM_N)=iabus;
      PREFETCH_FINAL;
      break;
    }
    case BITS_876_100: // -(An)
    {
      CHECK_IPL;
      PREFETCH_FINAL; //np
      iabus=AREG(PARAM_N)-2;
      dbus=resultl;
      CPU_BUS_ACCESS_WRITE; //nw
      AREG(PARAM_N)=iabus; // updated after write!
      break;
    }
    case BITS_876_101: // (d16, An) //np    nw np
      iabus=AREG(PARAM_N)+(signed short)IRC;
      PREFETCH; //np
      if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
        CHECK_IPL;
      dbus=resultl;
      CPU_BUS_ACCESS_WRITE; //nw
      if(!SOURCE_IS_REGISTER_OR_IMMEDIATE)
        CHECK_IPL;
      PREFETCH_FINAL;
      break;
    case BITS_876_110: // (d8, An, Xn) //n    np    nw np
      CPU_BUS_IDLE(2); //n
      m68k_iriwo=IRC;
      if(m68k_iriwo&BIT_b)  //.l
        iabus=AREG(PARAM_N)+(signed char)LOBYTE(m68k_iriwo)
        +(int)Cpu.r[m68k_iriwo>>12];
      else          //.w
        iabus=AREG(PARAM_N)+(signed char)LOBYTE(m68k_iriwo)
        +(signed short)Cpu.r[m68k_iriwo>>12];
      PREFETCH; //np
      dbus=resultl;
      CPU_BUS_ACCESS_WRITE; //nw
      CHECK_IPL;
      PREFETCH_FINAL;
      break;
    case BITS_876_111:
      switch (IRD&BITS_ba9) {
      case BITS_ba9_000: // (xxx).W //np    nw np
        iabus=(unsigned long)((signed long)((signed short)IRC)); // sign extension
        PREFETCH; //np
        dbus=resultl;
        CPU_BUS_ACCESS_WRITE; //nw
        CHECK_IPL;
        PREFETCH_FINAL;
        break;
      case BITS_ba9_001: // (xxx).L
                         // register or immediate: np np    nw np
                         // other:                 np    nw np np (refetch IR)
        if(SOURCE_IS_REGISTER_OR_IMMEDIATE)
        {
          TRUE_PC+=2;
          iabush=IRC;
          PREFETCH; //np
          iabusl=IRC;
          PREFETCH; //np
          dbus=resultl;
          CPU_BUS_ACCESS_WRITE; //nw
          CHECK_IPL;
          PREFETCH_FINAL;
        }
        else
        {
          iabush=IRC;
          PREFETCH; //np
          iabusl=IRC;
          pc+=2; // prefetch is delayed until after write!
          dbus=resultl;
          CPU_BUS_ACCESS_WRITE; //nw
          PREFETCH_ONLY;
          CHECK_IPL;
          PREFETCH_FINAL;
        }
        break;
      }
    }
  }// to memory
}


/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
        Bcc       |      INSTR      |                   INSTR
------------------+-----------------+------------------------------------------
<label> :         |                 |
 .B or .S :       |                 |
  branch taken    | 10(2/0)         |                 n          np np          
  branch not taken|  8(1/0)         |                nn             np          
*/

void m68k_bra_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
#if !defined(SSE_LEAN_AND_MEAN)
  if(CC_T) 
#endif
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
#if !defined(SSE_LEAN_AND_MEAN)
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
#endif
}


void m68k_bf_s() {
#if !defined(SSE_LEAN_AND_MEAN)
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_F) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
#endif
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bhi_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_HI) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bls_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_LS) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bcc_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_CC) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bcs_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_CS) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bne_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD)); //todo in (cc)
  if(CC_NE) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_beq_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_EQ) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bvc_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_VC) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bvs_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_VS) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bpl_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_PL) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bmi_s() { // MInus
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_MI) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bge_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_GE) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_blt_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_LT) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bgt_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_GT) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_ble_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  if(CC_LE) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}

/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
        BSR       |      INSTR      |                   INSTR
------------------+-----------------+------------------------------------------
<label> :         |                 |
  .B .S or .W :   | 18(2/2)         |                 n    nS ns np np          
*/

void m68k_bsr_s() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed char)IRD));
  CPU_BUS_IDLE(2); //n
  DU32 mypc;
  mypc.d32=pc;
  m68k_PUSH_L_WITH_TIMING(mypc); //nS ns
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  Debug.PseudoStackPush(pc);
#endif
  m68kSetPC(new_pc,true); //np np
}

/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
        Bcc       |      INSTR      |                   INSTR
------------------+-----------------+------------------------------------------
<label> :         |                 |
 .W :             |                 |
  branch taken    | 10(2/0)         |                 n          np np          
  branch not taken| 12(2/0)         |                nn          np np          
*/

void m68k_bra_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
#if !defined(SSE_LEAN_AND_MEAN)
  if(CC_T) 
#endif
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
#if !defined(SSE_LEAN_AND_MEAN)
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
#endif
}


void m68k_bf_l() {
#if !defined(SSE_LEAN_AND_MEAN)
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_F) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else 
#endif
  { // branch not taken
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bhi_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_HI) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bls_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_LS) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bcc_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_CC) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bcs_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_CS) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bne_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_NE) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_beq_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_EQ) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bvc_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_VC) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bvs_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_VS) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bpl_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_PL) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bmi_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_MI) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bge_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_GE) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_blt_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_LT) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_bgt_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_GT) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}


void m68k_ble_l() {
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  if(CC_LE) 
  { // branch taken
    CPU_BUS_IDLE(2); //n
    m68kSetPC(new_pc,true); //np np
  }
  else // branch not taken
  {
    CPU_BUS_IDLE(4); //nn
    PREFETCH; //np // fetch useless operand
    CHECK_IPL;
    PREFETCH_FINAL; //np
  }
}

/*
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
        BSR       |      INSTR      |                   INSTR
------------------+-----------------+------------------------------------------
<label> :         |                 |
  .B .S or .W :   | 18(2/2)         |                 n    nS ns np np          
*/

void m68k_bsr_l() {
  CPU_BUS_IDLE(2); //n
  DU32 mypc;
  TRUE_PC=pc+2; // beyond operand
  mypc.d32=TRUE_PC;
  m68k_PUSH_L_WITH_TIMING(mypc); // nS ns
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  Debug.PseudoStackPush(mypc.d32);
#endif
  MEM_ADDRESS new_pc=(pc+(signed long)((signed short)IRC));
  m68kSetPC(new_pc,true); //np np
}


/*
*******************************************************************************
  Line 0111
    MOVEQ
*******************************************************************************
-------------------------------------------------------------------------------
                  |    Exec Time    |               Data Bus Usage
       MOVEQ      |      INSTR      |  1st OP (ea)  |          INSTR
------------------+-----------------+---------------+--------------------------
#<data>,Dn :      |                 |               
  .L :            |  4(1/0)         |                               np          

*/

void m68k_moveq() {
  CHECK_IPL;
  result=(signed long)((signed char)IRD); //sign extension
  SR_CHECK_AND_L;
  REGL(PARAM_N)=result;
  PREFETCH_FINAL; //np
}


void m68k_lineA() {
#if defined(SSE_STATS)
  Stats.nException[10]++;
#endif
  Cpu.ProcessingState=TMC68000::EXCEPTION;
  pc-=2;  //pc not incremented for illegal instruction
  CPU_BUS_IDLE(4); //nn
  m68k_finish_exception(BOMBS_LINE_A * 4); //ns nS ns nV nv np n np 
  debug_check_break_on_irq(BREAK_IRQ_LINEA_IDX);
}


void m68k_lineF() {
#if defined(SSE_STATS)
  Stats.nException[11]++;
#endif
  Cpu.ProcessingState=TMC68000::EXCEPTION;
  pc-=2;  //pc not incremented for illegal instruction
#ifdef ONEGAME
  if (ird==0xffff){
    OGIntercept();
    return;
  }
#endif
  CPU_BUS_IDLE(4);
  m68k_finish_exception(BOMBS_LINE_F * 4); //ns nS ns nV nv np n np 
  debug_check_break_on_irq(BREAK_IRQ_LINEF_IDX);
}
