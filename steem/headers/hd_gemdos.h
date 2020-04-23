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
FILE: hd_gemdos.h
DESCRIPTION: Declarations for Steem's virtual hard drive emulation.
struct Tstemdos_file_struct, Tstemdos_fsnext_struct_type
---------------------------------------------------------------------------*/

#pragma once
#ifndef STEMDOS_DECLA_H
#define STEMDOS_DECLA_H

#include <dirsearch.h>

#define SVmemvalid 0x420
#define SVmemctrl 0x424
#define SVphystop 0x42e
#define SV_membot 0x432
#define SV_memtop 0x436
#define SVmemval2 0x43a
#define SVscreenpt 0x45e
#define SVsshiftmd 0x44c
#define SV_v_bas_ad 0x44e
#define SV_drvbits 0x4c2

extern bool mount_flag[26];
extern EasyStr mount_path[26];
extern bool stemdos_comline_read_is_rb;

#ifndef DISABLE_STEMDOS

#define GEMDOS_VECTOR LPEEK(0x84)

#define STEMDOS_RTE_GETDRIVE 0x02
#define STEMDOS_RTE_GETDIR 0x01
#define STEMDOS_RTE_DUP 0x03
#define STEMDOS_RTE_FCREATE 0x10
#define STEMDOS_RTE_FOPEN 0x20
#define STEMDOS_RTE_GET_DTA_FOR_FSFIRST 0x30
#define STEMDOS_RTE_FCLOSE 0x40
#define STEMDOS_RTE_DFREE 0x50
#define STEMDOS_RTE_MKDIR 0x60
#define STEMDOS_RTE_RMDIR 0x70
#define STEMDOS_RTE_FDELETE 0x80
#define STEMDOS_RTE_FATTRIB 0x90
#define STEMDOS_RTE_RENAME 0xa0
#define STEMDOS_RTE_PEXEC 0xb0
#define STEMDOS_RTE_MFREE 0xc0
#define STEMDOS_RTE_MFREE2 0xd0
#define STEMDOS_RTE_SUBACTION 0xf
#define STEMDOS_RTE_MAINACTION 0xf0

#define STEMDOS_FILE_IS_STEMDOS 0
#define STEMDOS_FILE_IS_GEMDOS 1
#define STEMDOS_FILE_ASKING 2

#define MAX_STEMDOS_FSNEXT_STRUCTS 100
#define MAX_STEMDOS_PEXEC_LIST 76 //Change loadsave_emu.cpp if change this! 


#pragma pack(push, 8)

struct Tstemdos_file_struct {
  Str filename;
  FILE *f;
  DWORD attrib;
  int owner_program;
  WORD date,time;
  bool open;
};


struct Tstemdos_fsnext_struct_type {
  EasyStr path;
  EasyStr NextFile;
  MEM_ADDRESS dta;
  DWORD start_hbl;
  int attr;
};

#pragma pack(pop)

void stemdos_intercept_trap_1();
void stemdos_rte();
void stemdos_set_drive_reset();
void stemdos_update_drvbits();
bool stemdos_check_mount(BYTE a);
void stemdos_reset();
BYTE stemdos_get_boot_drive();
void stemdos_check_paths();
bool stemdos_any_files_open();
void stemdos_control_c(); //control-c pressed
void stemdos_close_all_files();
extern void stemdos_init();
void stemdos_get_PC_path();
void stemdos_add_to_Pexec_list(MEM_ADDRESS);
bool stemdos_Pterm();
void stemdos_open_file(int);
void stemdos_close_file(Tstemdos_file_struct*);
void stemdos_read(int h,MEM_ADDRESS);
void stemdos_seek(int h,MEM_ADDRESS);
void stemdos_Fdatime(int h,MEM_ADDRESS);
void stemdos_Dfree(int dr,MEM_ADDRESS);
void stemdos_mkdir();
void stemdos_rmdir();
void stemdos_Fdelete();
void stemdos_rename();
void stemdos_Fattrib();
void stemdos_Pexec();
void stemdos_fsfirst(MEM_ADDRESS),stemdos_fsnext();
int stemdos_get_file_path();
void stemdos_parse_path(); //remove \..\ etc.
void stemdos_trap_1_Fdup();
void stemdos_trap_1_Mfree(MEM_ADDRESS ad);
void stemdos_trap_1_Fgetdta();
void stemdos_trap_1_Fclose(WORD h);
void stemdos_trap_1_Pexec_basepage();
void stemdos_finished();
void stemdos_final_rte(); //clear stack from original GEMDOS call
char* StrUpperNoSpecial(char*);
WORD STfile_read_word(FILE*f);
LONG STfile_read_long(FILE*f);
void STfile_read_to_ST_memory(FILE*f,MEM_ADDRESS ad,int n_bytes);

extern BYTE stemdos_boot_drive;
extern bool stemdos_intercept_datetime;
extern EasyStr mount_gemdos_path[26];
extern MEM_ADDRESS original_return_address;
extern int stemdos_rte_action;
extern Tstemdos_file_struct stemdos_file[46];
extern Tstemdos_file_struct stemdos_new_file;
extern BYTE stemdos_std_handle_forced_to[6]; //h>=6 && h<46
extern MEM_ADDRESS stemdos_dfree_buffer;
extern WORD stemdos_Fattrib_flag;
extern Tstemdos_fsnext_struct_type stemdos_fsnext_struct[MAX_STEMDOS_FSNEXT_STRUCTS];
extern WORD stemdos_command;
extern WORD stemdos_attr;
extern EasyStr stemdos_filename;
extern EasyStr stemdos_rename_to_filename;
extern EasyStr PC_filename;
extern FILE *stemdos_Pexec_file;
extern MEM_ADDRESS stemdos_Pexec_com,stemdos_Pexec_env;
extern WORD stemdos_Pexec_mode;
extern int stemdos_Pexec_list_ptr;
extern MEM_ADDRESS stemdos_Pexec_list[MAX_STEMDOS_PEXEC_LIST];
extern bool stemdos_ignore_next_pexec4;
extern MEM_ADDRESS stemdos_dta;
extern short stemdos_save_sr;
extern BYTE stemdos_current_drive;
extern bool STfile_read_error;

#ifdef SSE_DEBUG
extern short nMallocs;
#endif

#endif//#ifndef DISABLE_STEMDOS

#endif//#ifndef STEMDOS_DECLA_H
