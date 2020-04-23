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
FILE: cpuinit.cpp
DESCRIPTION: Initialisation for CPU jump tables (opcodes and effective 
address). This file contains only one function, cpu_routines_init().
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop
#include <cpu.h>
#include <cpu_op.h>

/*  OPCODE pointers
    Since v4, Steem uses one big opcode table, index is opcode, entry is 
    function pointer.
    This has been tested as a bit faster than the former way (a chain of
    function calls).
    Other advantage: Illegal trapped at once.
*/
void cpu_routines_init() {
  for(DWORD op=0;op<=0xffff;op++) // there are $FFFF + 1 entries
  {
    m68k_call_table[op]=m68k_trap1; // default
    DWORD line=op>>12;
    DWORD b6=(op&(BITS_ba9|BITS_876))>>6;
    DWORD b3=b6&7;
    DWORD b876=(op&BITS_876)>>6; // shifted
    DWORD b543=(op&BITS_543); // not shifted
    DWORD b876543=(op&(BITS_876|BITS_543))>>3; // shifted
    WORD condition_test=(op&0xf00)>>8;
    switch(line) {
    case 0:
      switch(b6) {
      case B6_000000: m68k_call_table[op]=((op & B6_111111)==B6_111100)
                        ? m68k_ori_b_to_ccr : m68k_ori_b;break;
      case B6_000001: m68k_call_table[op]=((op & B6_111111)==B6_111100)
                        ? m68k_ori_w_to_sr : m68k_ori_w;break;
      case B6_000010: m68k_call_table[op]=m68k_ori_l;break;
      case B6_001000: m68k_call_table[op]=((op & B6_111111)==B6_111100)
                        ? m68k_andi_b_to_ccr : m68k_andi_b;break;
      case B6_001001: m68k_call_table[op]=((op & B6_111111)==B6_111100)
                        ? m68k_andi_w_to_sr : m68k_andi_w;break;
      case B6_001010: m68k_call_table[op]=m68k_andi_l;break;
      case B6_010000: m68k_call_table[op]=m68k_subi_b;break;
      case B6_010001: m68k_call_table[op]=m68k_subi_w;break;
      case B6_010010: m68k_call_table[op]=m68k_subi_l;break;
      case B6_011000: m68k_call_table[op]=m68k_addi_b;break;
      case B6_011001: m68k_call_table[op]=m68k_addi_w;break;
      case B6_011010: m68k_call_table[op]=m68k_addi_l;break;
      case B6_100000: 
        if((op&B6_111111)!=B6_111100) //immediate mode is the only one not allowed
          m68k_call_table[op]=m68k_btst;
        break;
      case B6_100001: m68k_call_table[op]=m68k_bchg;break;
      case B6_100010: m68k_call_table[op]=m68k_bclr;break;
      case B6_100011: m68k_call_table[op]=m68k_bset;break;
      case B6_101000: m68k_call_table[op]=((op & B6_111111)==B6_111100)
                        ? m68k_eori_b_to_ccr : m68k_eori_b;break;
      case B6_101001: m68k_call_table[op]=((op & B6_111111)==B6_111100)
                        ? m68k_eori_w_to_sr : m68k_eori_w;break;
      case B6_101010: m68k_call_table[op]=m68k_eori_l;break;
      case B6_110000: m68k_call_table[op]=m68k_cmpi_b;break;
      case B6_110001: m68k_call_table[op]=m68k_cmpi_w;break;
      case B6_110010: m68k_call_table[op]=m68k_cmpi_l;break;
      default:
        if((op&BITS_543)==BITS_543_001)
        {
          switch(b3) {
          case B6_000100: m68k_call_table[op]=m68k_movep_w_to_dN;break;
          case B6_000101: m68k_call_table[op]=m68k_movep_l_to_dN;break;
          case B6_000110: m68k_call_table[op]=m68k_movep_w_from_dN;break;
          case B6_000111: m68k_call_table[op]=m68k_movep_l_from_dN;break;
          }
        }
        else
        {
          switch(b3) {
          case B6_000100: m68k_call_table[op]=m68k_btst_from_dN;break;
          case B6_000101: m68k_call_table[op]=m68k_bchg_from_dN;break;
          case B6_000110: m68k_call_table[op]=m68k_bclr_from_dN;break;
          case B6_000111: m68k_call_table[op]=m68k_bset_from_dN;break;
          }
        }
      }//sw
      break;
    case 1:
      if((op&BITS_876)==BITS_876_001||(op&BITS_876)==BITS_876_111
        &&(op&BITS_ba9)!=BITS_ba9_000&&(op&BITS_ba9)!=BITS_ba9_001)
        ;//trap1
      else
        m68k_call_table[op]=m68k_move_b;
      break;
    case 2:
      if((op&BITS_876)==BITS_876_111&&(op&BITS_ba9)!=BITS_ba9_000
        &&(op&BITS_ba9)!=BITS_ba9_001)
        ;
      else
        m68k_call_table[op]=m68k_move_l;
      break;
    case 3:
      if((op&BITS_876)==BITS_876_111&&(op&BITS_ba9)!=BITS_ba9_000
        &&(op&BITS_ba9)!=BITS_ba9_001)
        ;
      else
        m68k_call_table[op]=m68k_move_w;
      break;
    case 4:
      switch(b6) {
      case B6_000000: m68k_call_table[op]=m68k_negx_b;break;
      case B6_000001: m68k_call_table[op]=m68k_negx_w;break;
      case B6_000010: m68k_call_table[op]=m68k_negx_l;break;
      case B6_000011: m68k_call_table[op]=m68k_move_from_sr;break;
      case B6_001000: m68k_call_table[op]=m68k_clr_b;break;
      case B6_001001: m68k_call_table[op]=m68k_clr_w;break;
      case B6_001010: m68k_call_table[op]=m68k_clr_l;break;
      case B6_001011: m68k_call_table[op]=m68k_trap1;break;
      case B6_010000: m68k_call_table[op]=m68k_neg_b;break;
      case B6_010001: m68k_call_table[op]=m68k_neg_w;break;
      case B6_010010: m68k_call_table[op]=m68k_neg_l;break;
      case B6_010011: 
        if(b543!=BITS_543_001)
          m68k_call_table[op]=m68k_move_to_ccr;
        break;
      case B6_011000: m68k_call_table[op]=m68k_not_b;break;
      case B6_011001: m68k_call_table[op]=m68k_not_w;break;
      case B6_011010: m68k_call_table[op]=m68k_not_l;break;
      case B6_011011: 
        if(b543!=BITS_543_001)
          m68k_call_table[op]=m68k_move_to_sr;
        break;
      case B6_100000: m68k_call_table[op]=m68k_nbcd;break;
      case B6_100001: 
        switch(b543) {
        case BITS_543_000:
          m68k_call_table[op]=m68k_swap;
          break;
        case BITS_543_010: //(An)
        case BITS_543_101: //(d16,An)
        case BITS_543_110: //(d8,An,Xn)
          m68k_call_table[op]=m68k_pea;
          break;
        case BITS_543_111:
          if((op&0x7)<4)
            m68k_call_table[op]=m68k_pea;
          break;
        }//sw
        break;
      case B6_100010:
      case B6_100011: 
        switch(b543) {
        case BITS_543_001:
        case BITS_543_011: 
          break;
        case BITS_543_111:
          if((op&7)>1)
            break; //trap1
        default:
          switch(b6) {
          case B6_100010:m68k_call_table[op]=(b543==BITS_543_000)?
            m68k_ext_w:m68k_movem_w_from_regs;break;
          case B6_100011:m68k_call_table[op]=(b543==BITS_543_000)?
            m68k_ext_l:m68k_movem_l_from_regs;break;
          }
        }
        break;
      case B6_101000: m68k_call_table[op]=m68k_tst_b;break;
      case B6_101001: m68k_call_table[op]=m68k_tst_w;break;
      case B6_101010: m68k_call_table[op]=m68k_tst_l;break;
      case B6_101011: m68k_call_table[op]=((op&B6_111111)==B6_111100)
        ?m68k_trap1:m68k_tas;break;
      case B6_110010:
      case B6_110011:
        switch(b543) {
        case BITS_543_000:
        case BITS_543_001:
        case BITS_543_100:
          break;
        case BITS_543_111:
          if((op&7)>3)
            break;
        default:
          switch(b6) {
          case B6_110010: m68k_call_table[op]=m68k_movem_w_to_regs;break;
          case B6_110011: m68k_call_table[op]=m68k_movem_l_to_regs;break;
          }
        }
        break;
      case B6_111001: //m68k_line_4_stuff
        switch(b543) {
        case BITS_543_000:
        case BITS_543_001: m68k_call_table[op]=m68k_trap;break;
        case BITS_543_010: m68k_call_table[op]=m68k_link;break;
        case BITS_543_011: m68k_call_table[op]=m68k_unlk;break;
        case BITS_543_100: m68k_call_table[op]=m68k_move_to_usp;break;
        case BITS_543_101: m68k_call_table[op]=m68k_move_from_usp;break;
        default:
          switch(op&B6_111111) {
          case B6_110000: m68k_call_table[op]=m68k_reset;break;
          case B6_110001: m68k_call_table[op]=m68k_nop;break;
          case B6_110010: m68k_call_table[op]=m68k_stop;break;
          case B6_110011: m68k_call_table[op]=m68k_rte;break;
          case B6_110101: m68k_call_table[op]=m68k_rts;break;
          case B6_110110: m68k_call_table[op]=m68k_trapv;break;
          case B6_110111: m68k_call_table[op]=m68k_rtr;break;
          }
        }
        break;
      case B6_111010:
        switch(b543) {
        case BITS_543_010: //(An)
        case BITS_543_101: //(d16,An)
        case BITS_543_110: //(d8,An,Xn)
          m68k_call_table[op]=m68k_jsr;
          break;
        case BITS_543_111:
          if((op&0x7)<4)
            m68k_call_table[op]=m68k_jsr;
          break;
        }//sw
        break;
      case B6_111011: 
        switch(b543) {
        case BITS_543_010: //(An)
        case BITS_543_101: //(d16,An)
        case BITS_543_110: //(d8,An,Xn)
          m68k_call_table[op]=m68k_jmp;
          break;
        case BITS_543_111:
          if((op&0x7)<4)
            m68k_call_table[op]=m68k_jmp;
          break;
        }//sw
        break;
      default:
        switch(b3) {
        case B6_000110: 
          if(b543!=BITS_543_001)
            m68k_call_table[op]=m68k_chk;
          break;
        case B6_000111: 
          switch(b543) {
          case BITS_543_010: //(An)
          case BITS_543_101: //(d16,An)
          case BITS_543_110: //(d8,An,Xn)
            m68k_call_table[op]=m68k_lea;
            break;
          case BITS_543_111:
            if((op&0x7)<4)
              m68k_call_table[op]=m68k_lea;
            break;
          }//sw
          break;
        }
      }
      break;
    case 5:
      switch(b876) {
      case 0: m68k_call_table[op]=m68k_addq_b;break;
      case 1: m68k_call_table[op]=m68k_addq_w;break;
      case 2: m68k_call_table[op]=m68k_addq_l;break;
      case 3:
      case 7:
        //TODO test if single dbcc bcc are not more efficient?
        if(b543==BITS_543_001) //dbcc
        {
          switch(condition_test) {
          case 0: m68k_call_table[op]=m68k_dbra;break;
          case 1: m68k_call_table[op]=m68k_dbf;break;
          case 2: m68k_call_table[op]=m68k_dbhi;break;
          case 3: m68k_call_table[op]=m68k_dbls;break;
          case 4: m68k_call_table[op]=m68k_dbcc;break;
          case 5: m68k_call_table[op]=m68k_dbcs;break;
          case 6: m68k_call_table[op]=m68k_dbne;break;
          case 7: m68k_call_table[op]=m68k_dbeq;break;
          case 8: m68k_call_table[op]=m68k_dbvc;break;
          case 9: m68k_call_table[op]=m68k_dbvs;break;
          case 10: m68k_call_table[op]=m68k_dbpl;break;
          case 11: m68k_call_table[op]=m68k_dbmi;break;
          case 12: m68k_call_table[op]=m68k_dbge;break;
          case 13: m68k_call_table[op]=m68k_dblt;break;
          case 14: m68k_call_table[op]=m68k_dbgt;break;
          case 15: m68k_call_table[op]=m68k_dble;break;
          }//sw
        }
        else
        {
          switch(condition_test) {
          case 0: m68k_call_table[op]=m68k_st;break;
          case 1: m68k_call_table[op]=m68k_sf;break;
          case 2: m68k_call_table[op]=m68k_shi;break;
          case 3: m68k_call_table[op]=m68k_sls;break;
          case 4: m68k_call_table[op]=m68k_scc;break;
          case 5: m68k_call_table[op]=m68k_scs;break;
          case 6: m68k_call_table[op]=m68k_sne;break;
          case 7: m68k_call_table[op]=m68k_seq;break;
          case 8: m68k_call_table[op]=m68k_svc;break;
          case 9: m68k_call_table[op]=m68k_svs;break;
          case 10: m68k_call_table[op]=m68k_spl;break;
          case 11: m68k_call_table[op]=m68k_smi;break;
          case 12: m68k_call_table[op]=m68k_sge;break;
          case 13: m68k_call_table[op]=m68k_slt;break;
          case 14: m68k_call_table[op]=m68k_sgt;break;
          case 15: m68k_call_table[op]=m68k_sle;break;
          }//sw
        }
        break;
      case 4: m68k_call_table[op]=m68k_subq_b;break;
      case 5: m68k_call_table[op]=m68k_subq_w;break;
      case 6: m68k_call_table[op]=m68k_subq_l;break;
      }
      break;
    case 6:
      if((BYTE)op)
      { //8-bit displacement
        if((op&0xf00)==0x100)
        { //BSR
          m68k_call_table[op]=m68k_bsr_s;
        }
        else
        {
          switch(condition_test) {
          case 0: m68k_call_table[op]=m68k_bra_s;break;;
          case 1: m68k_call_table[op]=m68k_bf_s;break;;
          case 2: m68k_call_table[op]=m68k_bhi_s;break;;
          case 3: m68k_call_table[op]=m68k_bls_s;break;;
          case 4: m68k_call_table[op]=m68k_bcc_s;break;;
          case 5: m68k_call_table[op]=m68k_bcs_s;break;;
          case 6: m68k_call_table[op]=m68k_bne_s;break;;
          case 7: m68k_call_table[op]=m68k_beq_s;break;;
          case 8: m68k_call_table[op]=m68k_bvc_s;break;;
          case 9: m68k_call_table[op]=m68k_bvs_s;break;;
          case 10: m68k_call_table[op]=m68k_bpl_s;break;;
          case 11: m68k_call_table[op]=m68k_bmi_s;break;;
          case 12: m68k_call_table[op]=m68k_bge_s;break;;
          case 13: m68k_call_table[op]=m68k_blt_s;break;;
          case 14: m68k_call_table[op]=m68k_bgt_s;break;;
          case 15: m68k_call_table[op]=m68k_ble_s;break;;
          }//sw
        }
      }
      else
      {
        if((op&0xf00)==0x100)
        { //BSR
          m68k_call_table[op]=m68k_bsr_l;
        }
        else
        {
          switch(condition_test) {
          case 0: m68k_call_table[op]=m68k_bra_l;break;;
          case 1: m68k_call_table[op]=m68k_bf_l;break;;
          case 2: m68k_call_table[op]=m68k_bhi_l;break;;
          case 3: m68k_call_table[op]=m68k_bls_l;break;;
          case 4: m68k_call_table[op]=m68k_bcc_l;break;;
          case 5: m68k_call_table[op]=m68k_bcs_l;break;;
          case 6: m68k_call_table[op]=m68k_bne_l;break;;
          case 7: m68k_call_table[op]=m68k_beq_l;break;;
          case 8: m68k_call_table[op]=m68k_bvc_l;break;;
          case 9: m68k_call_table[op]=m68k_bvs_l;break;;
          case 10: m68k_call_table[op]=m68k_bpl_l;break;;
          case 11: m68k_call_table[op]=m68k_bmi_l;break;;
          case 12: m68k_call_table[op]=m68k_bge_l;break;;
          case 13: m68k_call_table[op]=m68k_blt_l;break;;
          case 14: m68k_call_table[op]=m68k_bgt_l;break;;
          case 15: m68k_call_table[op]=m68k_ble_l;break;;
          }//sw
        }
      }
      break;
    case 7:
      if((op&BIT_8)==0)
        m68k_call_table[op]=m68k_moveq;
      break;
    case 8:
      switch(b876) {
      case 0: m68k_call_table[op]=m68k_or_b_to_dN;break;
      case 1: m68k_call_table[op]=m68k_or_w_to_dN;break;
      case 2: m68k_call_table[op]=m68k_or_l_to_dN;break;
      case 3: m68k_call_table[op]=m68k_divu;break;
      case 4: m68k_call_table[op]=(b543>BITS_543_001)
                ? m68k_or_b_from_dN : m68k_sbcd;break;
      case 5: m68k_call_table[op]=(b543>BITS_543_001)
                ? m68k_or_w_from_dN : m68k_trap1;break;
      case 6: m68k_call_table[op]=(b543>BITS_543_001)
                ? m68k_or_l_from_dN : m68k_trap1;break;
      case 7: m68k_call_table[op]=m68k_divs;break;
      }
      break;
    case 9:
      switch(b876) {
       case 0: m68k_call_table[op]=m68k_sub_b_to_dN;break;
       case 1: m68k_call_table[op]=m68k_sub_w_to_dN;break;
       case 2: m68k_call_table[op]=m68k_sub_l_to_dN;break;
       case 3: m68k_call_table[op]=m68k_suba_w;break;
       case 4: m68k_call_table[op]=(b543>BITS_543_001)
                 ? m68k_sub_b_from_dN : m68k_subx_b;break;
       case 5: m68k_call_table[op]=(b543>BITS_543_001)
                 ? m68k_sub_w_from_dN : m68k_subx_w;break;
       case 6: m68k_call_table[op]=(b543>BITS_543_001)
                 ? m68k_sub_l_from_dN : m68k_subx_l;break;
       case 7: m68k_call_table[op]=m68k_suba_l;break;
      }
      break;
    case 0xa:
      m68k_call_table[op]=m68k_lineA;
      break;
    case 0xb:
      switch(b876) {
       case 0: m68k_call_table[op]=m68k_cmp_b;break;
       case 1: m68k_call_table[op]=m68k_cmp_w;break;
       case 2: m68k_call_table[op]=m68k_cmp_l;break;
       case 3: m68k_call_table[op]=m68k_cmpa_w;break;
       case 4: m68k_call_table[op]=(b543==BITS_543_001)
                 ? m68k_cmpm_b : m68k_eor_b;break;
       case 5: m68k_call_table[op]=(b543==BITS_543_001)
                 ? m68k_cmpm_w : m68k_eor_w;break;
       case 6: m68k_call_table[op]=(b543==BITS_543_001)
                 ? m68k_cmpm_l : m68k_eor_l;break;
       case 7: m68k_call_table[op]=m68k_cmpa_l;break;
      }
      break;
    case 0xc:
      switch(b876) {
      case 0: m68k_call_table[op]=m68k_and_b_to_dN;break;
      case 1: m68k_call_table[op]=m68k_and_w_to_dN;break;
      case 2: m68k_call_table[op]=m68k_and_l_to_dN;break;
      case 3: m68k_call_table[op]=m68k_mulu;break;
      case 4: m68k_call_table[op]=(b543>BITS_543_001)
                ? m68k_and_b_from_dN : m68k_abcd;break;
      case 5: m68k_call_table[op]=(b543>BITS_543_001)
                ? m68k_and_w_from_dN : m68k_exg_like;break;
      case 6: m68k_call_table[op]=(b543==BITS_543_001)//000 is illegal
                ? m68k_exg_unlike : m68k_and_l_from_dN;break;
      case 7: m68k_call_table[op]=m68k_muls;break;
      }
      break;
    case 0xd:
      switch(b876) {
      case 0: m68k_call_table[op]=m68k_add_b_to_dN;break;
      case 1: m68k_call_table[op]=m68k_add_w_to_dN;break;
      case 2: m68k_call_table[op]=m68k_add_l_to_dN;break;
      case 3: m68k_call_table[op]=m68k_adda_w;break;
      case 4: m68k_call_table[op]=(b543>BITS_543_001)
                ? m68k_add_b_from_dN : m68k_addx_b;break;
      case 5: m68k_call_table[op]=(b543>BITS_543_001)
                ? m68k_add_w_from_dN : m68k_addx_w;break;
      case 6: m68k_call_table[op]=(b543>BITS_543_001)
                ? m68k_add_l_from_dN : m68k_addx_l;break;
      case 7: m68k_call_table[op]=m68k_adda_l;break;
      }
      break;
    case 0xe:
      switch(b876543) {
      case B6_000000: m68k_call_table[op]=m68k_asr_b_to_dM;break;
      case B6_000001: m68k_call_table[op]=m68k_lsr_b_to_dM;break;
      case B6_000010: m68k_call_table[op]=m68k_roxr_b_to_dM;break;
      case B6_000011: m68k_call_table[op]=m68k_ror_b_to_dM;break;
      case B6_001000: m68k_call_table[op]=m68k_asr_w_to_dM;break;
      case B6_001001: m68k_call_table[op]=m68k_lsr_w_to_dM;break;
      case B6_001010: m68k_call_table[op]=m68k_roxr_w_to_dM;break;
      case B6_001011: m68k_call_table[op]=m68k_ror_w_to_dM;break;
      case B6_010000: m68k_call_table[op]=m68k_asr_l_to_dM;break;
      case B6_010001: m68k_call_table[op]=m68k_lsr_l_to_dM;break;
      case B6_010010: m68k_call_table[op]=m68k_roxr_l_to_dM;break;
      case B6_010011: m68k_call_table[op]=m68k_ror_l_to_dM;break;
      case B6_100000: m68k_call_table[op]=m68k_asl_b_to_dM;break;
      case B6_100001: m68k_call_table[op]=m68k_lsl_b_to_dM;break;
      case B6_100010: m68k_call_table[op]=m68k_roxl_b_to_dM;break;
      case B6_100011: m68k_call_table[op]=m68k_rol_b_to_dM;break;
      case B6_101000: m68k_call_table[op]=m68k_asl_w_to_dM;break;
      case B6_101001: m68k_call_table[op]=m68k_lsl_w_to_dM;break;
      case B6_101010: m68k_call_table[op]=m68k_roxl_w_to_dM;break;
      case B6_101011: m68k_call_table[op]=m68k_rol_w_to_dM;break;
      case B6_110000: m68k_call_table[op]=m68k_asl_l_to_dM;break;
      case B6_110001: m68k_call_table[op]=m68k_lsl_l_to_dM;break;
      case B6_110010: m68k_call_table[op]=m68k_roxl_l_to_dM;break;
      case B6_110011: m68k_call_table[op]=m68k_rol_l_to_dM;break;
      default:
        if(b543>=BITS_543_010)
        {
          switch(op&BITS_ba9) {
          case BITS_ba9_000:
          case BITS_ba9_001:
          case BITS_ba9_010:
          case BITS_ba9_011:
            switch(b876) {
            case 3:
              m68k_call_table[op]=m68k_bit_shift_right_to_mem;
              break;
            case 7:
              m68k_call_table[op]=m68k_bit_shift_left_to_mem;
              break;
            }
          }
        }
      }
      //for bit shifting to data registers, immediate mode and dN, mode point 
      //to same function
      if(b543>=BITS_543_100 && m68k_call_table[op]==m68k_trap1)
        m68k_call_table[op]=m68k_call_table[op-BITS_543_100];
      break;
    case 0xf:
      m68k_call_table[op]=m68k_lineF;
      break;
    }//sw
  }//nxt op
  /// SOURCE pointers
  m68k_jsr_get_source_b[0]=m68k_get_source_000_b;
  m68k_jsr_get_source_w[0]=m68k_get_source_000_w;
  m68k_jsr_get_source_l[0]=m68k_get_source_000_l;
  m68k_jsr_get_source_b[1]=m68k_get_source_001_b;
  m68k_jsr_get_source_w[1]=m68k_get_source_001_w;
  m68k_jsr_get_source_l[1]=m68k_get_source_001_l;
  m68k_jsr_get_source_b[2]=m68k_get_source_010_b;
  m68k_jsr_get_source_w[2]=m68k_get_source_010_w;
  m68k_jsr_get_source_l[2]=m68k_get_source_010_l;
  m68k_jsr_get_source_b[3]=m68k_get_source_011_b;
  m68k_jsr_get_source_w[3]=m68k_get_source_011_w;
  m68k_jsr_get_source_l[3]=m68k_get_source_011_l;
  m68k_jsr_get_source_b[4]=m68k_get_source_100_b;
  m68k_jsr_get_source_w[4]=m68k_get_source_100_w;
  m68k_jsr_get_source_l[4]=m68k_get_source_100_l;
  m68k_jsr_get_source_b[5]=m68k_get_source_101_b;
  m68k_jsr_get_source_w[5]=m68k_get_source_101_w;
  m68k_jsr_get_source_l[5]=m68k_get_source_101_l;
  m68k_jsr_get_source_b[6]=m68k_get_source_110_b;
  m68k_jsr_get_source_w[6]=m68k_get_source_110_w;
  m68k_jsr_get_source_l[6]=m68k_get_source_110_l;
  m68k_jsr_get_source_b[7]=m68k_get_source_111_b;
  m68k_jsr_get_source_w[7]=m68k_get_source_111_w;
  m68k_jsr_get_source_l[7]=m68k_get_source_111_l;
  for(int n=0;n<8;n++)
  {
    m68k_jsr_get_source_b_not_a[n]=m68k_jsr_get_source_b[n];
    m68k_jsr_get_source_w_not_a[n]=m68k_jsr_get_source_w[n];
    m68k_jsr_get_source_l_not_a[n]=m68k_jsr_get_source_l[n];
  }
  m68k_jsr_get_source_b_not_a[1]=m68k_trap1;
  m68k_jsr_get_source_w_not_a[1]=m68k_trap1;
  m68k_jsr_get_source_l_not_a[1]=m68k_trap1;
  /// DESTINATION pointers
  m68k_jsr_get_dest_b[0]=m68k_get_dest_000_b;
  m68k_jsr_get_dest_w[0]=m68k_get_dest_000_w;
  m68k_jsr_get_dest_l[0]=m68k_get_dest_000_l;
  m68k_jsr_get_dest_b[1]=m68k_get_dest_001_b;
  m68k_jsr_get_dest_w[1]=m68k_get_dest_001_w;
  m68k_jsr_get_dest_l[1]=m68k_get_dest_001_l;
  m68k_jsr_get_dest_b[2]=m68k_get_dest_010_b;
  m68k_jsr_get_dest_w[2]=m68k_get_dest_010_w;
  m68k_jsr_get_dest_l[2]=m68k_get_dest_010_l;
  m68k_jsr_get_dest_b[3]=m68k_get_dest_011_b;
  m68k_jsr_get_dest_w[3]=m68k_get_dest_011_w;
  m68k_jsr_get_dest_l[3]=m68k_get_dest_011_l;
  m68k_jsr_get_dest_b[4]=m68k_get_dest_100_b;
  m68k_jsr_get_dest_w[4]=m68k_get_dest_100_w;
  m68k_jsr_get_dest_l[4]=m68k_get_dest_100_l;
  m68k_jsr_get_dest_b[5]=m68k_get_dest_101_b;
  m68k_jsr_get_dest_w[5]=m68k_get_dest_101_w;
  m68k_jsr_get_dest_l[5]=m68k_get_dest_101_l;
  m68k_jsr_get_dest_b[6]=m68k_get_dest_110_b;
  m68k_jsr_get_dest_w[6]=m68k_get_dest_110_w;
  m68k_jsr_get_dest_l[6]=m68k_get_dest_110_l;
  m68k_jsr_get_dest_b[7]=m68k_get_dest_111_b;
  m68k_jsr_get_dest_w[7]=m68k_get_dest_111_w;
  m68k_jsr_get_dest_l[7]=m68k_get_dest_111_l;
  for(int n=0;n<8;n++)
  {
    m68k_jsr_get_dest_b_not_a[n]=m68k_jsr_get_dest_b[n];
    m68k_jsr_get_dest_w_not_a[n]=m68k_jsr_get_dest_w[n];
    m68k_jsr_get_dest_l_not_a[n]=m68k_jsr_get_dest_l[n];
  }
  m68k_jsr_get_dest_b_not_a[1]=m68k_trap1;
  m68k_jsr_get_dest_w_not_a[1]=m68k_trap1;
  m68k_jsr_get_dest_l_not_a[1]=m68k_trap1;
  for(int n=0;n<8;n++)
  {
    m68k_jsr_get_dest_b_not_a_or_d[n]=m68k_jsr_get_dest_b_not_a[n];
    m68k_jsr_get_dest_w_not_a_or_d[n]=m68k_jsr_get_dest_w_not_a[n];
    m68k_jsr_get_dest_l_not_a_or_d[n]=m68k_jsr_get_dest_l_not_a[n];
  }
  m68k_jsr_get_dest_b_not_a_or_d[0]=m68k_trap1;
  m68k_jsr_get_dest_w_not_a_or_d[0]=m68k_trap1;
  m68k_jsr_get_dest_l_not_a_or_d[0]=m68k_trap1;

#if defined(SSE_VC_INTRINSICS)
/*  Check CPU caps.
    If it knows POPCNT, we'll use it instead of the C code in acc.cpp.
*/
  int cpuInfo[4];
  __cpuid(cpuInfo, 1);
  BOOL popcount=(BITTEST(cpuInfo[2],23));
  count_bits_set_in_word=(popcount)
    ? count_bits_set_in_word2 : count_bits_set_in_word1;
#endif
}
