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
FILE: cpu_op.h
DESCRIPTION: Declarations of CPU opcode functions.
Declarations were generated by VS2015, before that we just included files
in the correct order!
---------------------------------------------------------------------------*/

#pragma once
#ifndef CPU_OP_H
#define CPU_OP_H

void m68k_ori_b();

void m68k_andi_b();

void m68k_eori_b();

void m68k_subi_b();

void m68k_addi_b();

void m68k_ori_w();

void m68k_andi_w();

void m68k_eori_w();

void m68k_subi_w();

void m68k_addi_w();

void m68k_ori_l();

void m68k_andi_l();

void m68k_eori_l();

void m68k_subi_l();

void m68k_addi_l();

void m68k_ori_b_to_ccr();

void m68k_andi_b_to_ccr();

void m68k_eori_b_to_ccr();

void m68k_ori_w_to_sr();

void m68k_andi_w_to_sr();

void m68k_eori_w_to_sr();

void m68k_btst();

void m68k_btst_from_dN();

void m68k_bchg();

void m68k_bchg_from_dN();

void m68k_bset();

void m68k_bset_from_dN();

void m68k_bclr();

void m68k_bclr_from_dN();

void m68k_cmpi_b();

void m68k_cmpi_w();

void m68k_cmpi_l();

void m68k_movep_w_to_dN();

void m68k_movep_l_to_dN();

void m68k_movep_w_from_dN();

void m68k_movep_l_from_dN();

void m68k_negx_b();

void m68k_negx_w();

void m68k_negx_l();

void m68k_clr_b();

void m68k_clr_w();

void m68k_clr_l();

void m68k_neg_b();

void m68k_neg_w();

void m68k_neg_l();

void m68k_not_b();

void m68k_not_w();

void m68k_not_l();

void m68k_tst_b();

void m68k_tst_w();

void m68k_tst_l();

void m68k_tas();

void m68k_move_from_sr();

void m68k_move_to_ccr();

void m68k_move_to_sr();

void m68k_nbcd();

void m68k_swap();

void m68k_pea();

void m68k_ext_w();

void m68k_movem_w_from_regs();

void m68k_ext_l();

void m68k_movem_l_from_regs();

void m68k_movem_l_to_regs();

void m68k_movem_w_to_regs();

void m68k_jsr();

void m68k_jmp();

void m68k_chk();

void m68k_lea();

void m68k_trap();

void m68k_link();

void m68k_unlk();

void m68k_move_to_usp();

void m68k_move_from_usp();

void m68k_reset();

void m68k_nop();

void m68k_stop();

void m68k_rte();

void m68k_rts();

void m68k_trapv();

void m68k_rtr();

void m68k_addq_b();

void m68k_addq_w();

void m68k_addq_l();

void m68k_subq_b();

void m68k_subq_w();

void m68k_subq_l();

void m68k_dbra();

void m68k_dbf();

void m68k_dbhi();

void m68k_dbls();

void m68k_dbcc();

void m68k_dbcs();

void m68k_dbne();

void m68k_dbeq();

void m68k_dbvc();

void m68k_dbvs();

void m68k_dbpl();

void m68k_dbmi();

void m68k_dbge();

void m68k_dblt();

void m68k_dbgt();

void m68k_dble();

void m68k_st();

void m68k_sf();

void m68k_shi();

void m68k_sls();

void m68k_scc();

void m68k_scs();

void m68k_sne();

void m68k_seq();

void m68k_svc();

void m68k_svs();

void m68k_spl();

void m68k_smi();

void m68k_sge();

void m68k_slt();

void m68k_sgt();

void m68k_sle();

void m68k_or_b_to_dN();

void m68k_or_w_to_dN();

void m68k_or_l_to_dN();

void m68k_divu();

void m68k_sbcd();

void m68k_or_b_from_dN();

void m68k_or_w_from_dN();

void m68k_or_l_from_dN();

void m68k_divs();

void m68k_sub_b_to_dN();

void m68k_sub_w_to_dN();

void m68k_sub_l_to_dN();

void m68k_suba_w();

void m68k_subx_b();

void m68k_sub_b_from_dN();

void m68k_subx_w();

void m68k_sub_w_from_dN();

void m68k_subx_l();

void m68k_sub_l_from_dN();

void m68k_suba_l();

void m68k_cmp_b();

void m68k_cmp_w();

void m68k_cmp_l();

void m68k_cmpa_w();

void m68k_cmpm_b();

void m68k_eor_b();

void m68k_cmpm_w();

void m68k_eor_w();

void m68k_cmpm_l();

void m68k_eor_l();

void m68k_cmpa_l();

void m68k_and_b_to_dN();

void m68k_and_w_to_dN();

void m68k_and_l_to_dN();

void m68k_mulu();

void m68k_abcd();

void m68k_and_b_from_dN();

void m68k_exg_like();

void m68k_and_w_from_dN();

void m68k_exg_unlike();

void m68k_and_l_from_dN();

void m68k_muls();

void m68k_add_b_to_dN();

void m68k_add_w_to_dN();

void m68k_add_l_to_dN();

void m68k_adda_w();

void m68k_addx_b();

void m68k_add_b_from_dN();

void m68k_addx_w();

void m68k_add_w_from_dN();

void m68k_addx_l();

void m68k_add_l_from_dN();

void m68k_adda_l();

void m68k_bit_shift_to_dm_get_source();

void m68k_asr_b_to_dM();

void m68k_lsr_b_to_dM();

void m68k_roxr_b_to_dM();

void m68k_ror_b_to_dM();

void m68k_asr_w_to_dM();

void m68k_lsr_w_to_dM();

void m68k_roxr_w_to_dM();

void m68k_ror_w_to_dM();

void m68k_asr_l_to_dM();

void m68k_lsr_l_to_dM();

void m68k_roxr_l_to_dM();

void m68k_ror_l_to_dM();

void m68k_asl_b_to_dM();

void m68k_lsl_b_to_dM();

void m68k_roxl_b_to_dM();

void m68k_rol_b_to_dM();

void m68k_asl_w_to_dM();

void m68k_lsl_w_to_dM();

void m68k_roxl_w_to_dM();

void m68k_rol_w_to_dM();

void m68k_asl_l_to_dM();

void m68k_lsl_l_to_dM();

void m68k_roxl_l_to_dM();

void m68k_rol_l_to_dM();

void m68k_bit_shift_right_to_mem();

void m68k_bit_shift_left_to_mem();

void m68k_move_b();

void m68k_move_l();

void m68k_move_w();

void m68k_bra_s();

void m68k_bf_s();

void m68k_bhi_s();

void m68k_bls_s();

void m68k_bcc_s();

void m68k_bcs_s();

void m68k_bne_s();

void m68k_beq_s();

void m68k_bvc_s();

void m68k_bvs_s();

void m68k_bpl_s();

void m68k_bmi_s();

void m68k_bge_s();

void m68k_blt_s();

void m68k_bgt_s();

void m68k_ble_s();

void m68k_bsr_s();

void m68k_bra_l();

void m68k_bf_l();

void m68k_bhi_l();

void m68k_bls_l();

void m68k_bcc_l();

void m68k_bcs_l();

void m68k_bne_l();

void m68k_beq_l();

void m68k_bvc_l();

void m68k_bvs_l();

void m68k_bpl_l();

void m68k_bmi_l();

void m68k_bge_l();

void m68k_blt_l();

void m68k_bgt_l();

void m68k_ble_l();

void m68k_bsr_l();

void m68k_moveq();

void m68k_lineA();

void m68k_lineF();

#endif//#ifndef CPU_OP_H