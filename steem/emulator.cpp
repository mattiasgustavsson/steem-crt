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
FILE: emulator.cpp
DESCRIPTION: Miscellaneous core emulator functions. An important function is
init_timings that sets up all Steem's counters and clocks. Also included are
the code for Steem's agenda system that schedules tasks to be performed at
the end of scanlines, intercept functions.
Some emulation objects are instantiated here.
v401: Function that collects statistics displayed in the infobox.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <computer.h>
#include <draw.h>
#include <mymisc.h>
#include <interface_stvl.h>
#include <harddiskman.h>
#include <gui.h>


#if defined(DEBUG_BUILD) && defined(PEEK_RANGE_TEST)

void RangeError(DWORD &ad,DWORD hi_ad) {
//  ad/=0;
  ad=hi_ad-1;
}

BYTE& PEEK(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,0,0);return *LPBYTE(Mem_End_minus_1-ad); }
WORD& DPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,1,0);return *LPWORD(Mem_End_minus_2-ad); }
DWORD& LPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,3,0);return *LPDWORD(Mem_End_minus_4-ad); }
BYTE* lpPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,0,0);return LPBYTE(Mem_End_minus_1-ad); }
WORD* lpDPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,1,0);return LPWORD(Mem_End_minus_2-ad); }
DWORD* lpLPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(mem_len+MEM_EXTRA_BYTES,3,0);return LPDWORD(Mem_End_minus_4-ad); }

BYTE& ROM_PEEK(DWORD ad){ RANGE_CHECK_MESSAGE(tos_len,0,0);return *LPBYTE(Rom_End_minus_1-ad); }
WORD& ROM_DPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(tos_len,1,0);return *LPWORD(Rom_End_minus_2-ad); }
DWORD& ROM_LPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(tos_len,3,0);return *LPDWORD(Rom_End_minus_4-ad); }
BYTE* lpROM_PEEK(DWORD ad){ RANGE_CHECK_MESSAGE(tos_len,0,0);return LPBYTE(Rom_End_minus_1-ad); }
WORD* lpROM_DPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(tos_len,1,2);return LPWORD(Rom_End_minus_2-ad); }
DWORD* lpROM_LPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(tos_len,3,0);return LPDWORD(Rom_End_minus_4-ad); }

BYTE& CART_PEEK(DWORD ad){ RANGE_CHECK_MESSAGE(128*1024,0,0);return *LPBYTE(Cart_End_minus_1-ad); }
WORD& CART_DPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(128*1024,1,0);return *LPWORD(Cart_End_minus_2-ad); }
DWORD& CART_LPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(128*1024,3,0);return *LPDWORD(Cart_End_minus_4-ad); }
BYTE* lpCART_PEEK(DWORD ad){ RANGE_CHECK_MESSAGE(128*1024,0,0);return LPBYTE(Cart_End_minus_1-ad); }
WORD* lpCART_DPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(128*1024,1,2);return LPWORD(Cart_End_minus_2-ad); }
DWORD* lpCART_LPEEK(DWORD ad){ RANGE_CHECK_MESSAGE(128*1024,3,0);return LPDWORD(Cart_End_minus_4-ad); }

#endif


#if defined(SSE_VID_STVL1)
BYTE &bus_mask=Stvl.bus_mask; // using STVL's
#else
BYTE bus_mask;
#endif
#if !defined(SSE_VID_STVL1)
MEM_ADDRESS abus;
DU16 udbus;
WORD& dbus=udbus.d16;
BYTE& dbusl=udbus.d8[LO];
BYTE& dbush=udbus.d8[HI];
#endif

int cpu_cycles;
int ioaccess;
COUNTER_VAR cpu_timer;
unsigned long hbl_count=0;
int interrupt_depth=0;
DWORD em_width=480,em_height=480;
BYTE em_planes=4;
#ifdef NO_CRAZY_MONITOR
const
#endif
BYTE extended_monitor=0;
DWORD n_cpu_cycles_per_second =(CPU_CLOCK_STE_PAL);
DWORD new_n_cpu_cycles_per_second =(0),n_millions_cycles_per_sec =(8);
//extern "C" 
double cpu_cycles_multiplier=1.0; // used by 6301
int on_rte;
int on_rte_interrupt_depth;
short cpu_cycles_from_hbl_to_timer_b;
BYTE video_freq_idx =(1);
int video_first_draw_line;
int video_last_draw_line;
BYTE screen_res_at_start_of_vbl;
BYTE video_freq_at_start_of_vbl;

const WORD scanline_time_in_cpu_cycles_8mhz[4]={SCANLINE_TIME_IN_CPU_CYCLES_50HZ,
                                                SCANLINE_TIME_IN_CPU_CYCLES_60HZ,
                                                SCANLINE_TIME_IN_CPU_CYCLES_70HZ,
                                                128};

int scanline_time_in_cpu_cycles[4]={SCANLINE_TIME_IN_CPU_CYCLES_50HZ,
                                    SCANLINE_TIME_IN_CPU_CYCLES_60HZ,
                                    SCANLINE_TIME_IN_CPU_CYCLES_70HZ,
                                    128};

const WORD hbl_per_second[4]={313*50,263*60,501*71,501*71}; //HBL_PER_SECOND

int scanline_time_in_cpu_cycles_at_start_of_vbl;
COUNTER_VAR cpu_time_of_last_vbl,shifter_cycle_base;
COUNTER_VAR cpu_timer_at_start_of_hbl;
BYTE screen_res =(0);
short scan_y;
BYTE video_mixed_output=0;
MEM_ADDRESS old_pc;
MEM_ADDRESS pc_high_byte;
WIN_ONLY( CRITICAL_SECTION agenda_cs; )

bool emudetect_called=0;
bool emudetect_write_logs_to_printer=0,emudetect_overscans_fixed=false;
#if defined(SSE_NO_FALCONMODE)
const 
#endif
BYTE emudetect_falcon_mode=EMUD_FALC_MODE_OFF;
BYTE emudetect_falcon_mode_size=0;
bool emudetect_falcon_extra_height=0;
DynamicArray<DWORD> emudetect_falcon_stpal;
DynamicArray<DWORD> emudetect_falcon_pcpal;

TAgenda agenda[MAX_AGENDA_LENGTH];
int agenda_length=0;
unsigned long agenda_next_time=0x7fffffff;
MEM_ADDRESS on_rte_return_address;
LPAGENDAPROC agenda_list[]={
  agenda_fdc_spun_up,
  agenda_fdc_motor_flag_off,
  agenda_fdc_finished,
  agenda_floppy_seek,
  agenda_floppy_readwrite_sector,
  agenda_floppy_read_address,
  agenda_floppy_read_track,
  agenda_floppy_write_track,
  agenda_serial_sent_byte,
  agenda_serial_break_boundary,
  agenda_serial_loopback_byte,
  agenda_midi_replace,
  agenda_check_centronics_interrupt,
  agenda_ikbd_process,
  agenda_keyboard_reset,
  agenda_acia_tx_delay_IKBD,
  agenda_acia_tx_delay_MIDI,
  ikbd_send_joystick_message,
  ikbd_report_abs_mouse,
  agenda_keyboard_replace,
  agenda_fdc_verify,
  agenda_reset,
  (LPAGENDAPROC)1};
int aes_calls_since_reset=0;

MEM_ADDRESS line_a_base=0;
MEM_ADDRESS vdi_intout=0;

const int EIGHT_MILLION=8000000;

void init_timings() {
  TRACE_INIT("init_timings()\n");
  // don't do anything to agendas here!
  Fdc.str&=BYTE(~FDC_STR_MO);
  video_first_draw_line=0;	// SS: 0-199 = normal display
  video_last_draw_line=shifter_y;
  if(COLOUR_MONITOR==0) 
    Glue.video_freq=MONO_HZ;
  CALC_VIDEO_FREQ_IDX;
  if(!OPTION_68901)
  {
    CALC_CYCLES_FROM_HBL_TO_TIMER_B(Glue.video_freq);
  }
  screen_res_at_start_of_vbl=screen_res;
  video_freq_at_start_of_vbl=Glue.video_freq;
  scanline_time_in_cpu_cycles_at_start_of_vbl=scanline_time_in_cpu_cycles[video_freq_idx];
  Glue.hbl_pending=true;
  cpu_time_of_start_of_event_plan=0; //0x7f000000; // test overflow
  if(OPTION_C3)
  {
    event_vector=event_dummy;
    time_of_next_event=EIGHT_MILLION;
  }
  else
    Glue.GetNextVideoEvent();
  cpu_cycles=(int)time_of_next_event;
  Glue.CurrentScanline.Cycles=scanline_time_in_cpu_cycles_at_start_of_vbl;
  //ASSERT(Glue.CurrentScanline.Cycles>=224);
  //cpu_timer=time_of_next_event;
  cpu_timer=0;
  cpu_time_of_last_vbl=ABSOLUTE_CPU_TIME;
  time_of_next_timer_b=cpu_time_of_last_vbl+160000;
  scan_y=-scanlines_above_screen[video_freq_idx];
  time_of_last_hbl_interrupt=ABSOLUTE_CPU_TIME;
  time_of_last_vbl_interrupt=ACT;
  cpu_time_of_first_mfp_tick=ABSOLUTE_CPU_TIME;
  shifter_cycle_base=ABSOLUTE_CPU_TIME;
  for(int i=0;i<16;i++)
    mfp_time_of_start_of_last_interrupt[i]=ABSOLUTE_CPU_TIME
      -CYCLES_FROM_START_OF_MFP_IRQ_TO_WHEN_PEND_IS_CLEARED-2;
  mfp_init_timers();
  shifter_draw_pointer=vbase;
  shifter_draw_pointer_at_start_of_line=shifter_draw_pointer;
  shifter_pixel=shifter_hscroll; //start by drawing this pixel
  left_border=BORDER_SIDE;right_border=BORDER_SIDE;
  scanline_drawn_so_far=0;
  cpu_timer_at_start_of_hbl=0;
  shifter_freq_change_idx=0;
  for(int n=0;n<32;n++) 
  {
    shifter_freq_change[n]=Glue.video_freq;
    shifter_freq_change_time[n]=ABSOLUTE_CPU_TIME;
  }
  ikbd_joy_poll_line=0;
  ste_sound_on_this_screen=0;
  ste_sound_output_countdown=0;
  ste_sound_samples_countdown=0;
  ste_sound_channel_buf_idx=0;
#if USE_PASTI
  pasti_update_time=ABSOLUTE_CPU_TIME+EIGHT_MILLION;
#endif
  hbl_count=0;
}


  //Agenda
#undef LOGSECTION
#define LOGSECTION LOGSECTION_AGENDA

int milliseconds_to_hbl(int ms) {
  return ms*HBL_PER_SECOND/1000;
}

// note: critical section disabled for emu thread

void agenda_add(LPAGENDAPROC action,int pause,int param) {
#if defined(SSE_DEBUG)
  //ASSERT(pause>=0);
  //ASSERT(action!=agenda_fdc_motor_flag_off);
  int i;
  for(i=0;i<256 && agenda_list[i]!=(LPAGENDAPROC)1;i++)
    if(agenda_list[i]==action)
      break;
  TRACE_LOG("agenda add #%d #%d %p in %d hbl data $%X\n",agenda_length,i,action,pause,param);
#endif
  if(agenda_length>=MAX_AGENDA_LENGTH)
  {
    log_write("AARRRGGGHH!: Agenda full, can't add!");
    TRACE2("Agenda full\n");
    ASSERT( agenda_length<MAX_AGENDA_LENGTH ); // Debugger msg box -> quit
    return;
  }
  AGENDA_CS( EnterCriticalSection(&agenda_cs); )
  unsigned long target_time=hbl_count+pause;
  int n=0;
  while (n<agenda_length && (signed int)(agenda[n].time-target_time)>0) 
    n++;
  //budge the n, n+1, ... along
  for(int nn=agenda_length;nn>n;nn--)
    agenda[nn]=agenda[nn-1];
  agenda[n].perform=action; // SS pointer to the function to call
  agenda[n].time=target_time;
  agenda[n].param=param;
  agenda_next_time=agenda[agenda_length].time;
  agenda_length++;
  DBG_LOG(EasyStr("TASKS: Agenda length = ")+agenda_length);
  /*ASSERT(agenda_cs.RecursionCount==1);*/
  AGENDA_CS(LeaveCriticalSection(&agenda_cs); )
}


void agenda_delete(LPAGENDAPROC job) {
  AGENDA_CS(EnterCriticalSection(&agenda_cs); )
  for(int n=0;n<agenda_length;n++)
    {
      if(agenda[n].perform==job)
      {
        TRACE_LOG("agenda delete #%d %p\n",n,job);
        for(int nn=n;nn<agenda_length;nn++)
        {
          agenda[nn]=agenda[nn+1];
        }
        agenda_length--;
        n--;
      }
    }
  if(agenda_length)
    agenda_next_time=agenda[agenda_length-1].time;
  else
    agenda_next_time=hbl_count-1; //wait 42 hours
  AGENDA_CS(LeaveCriticalSection(&agenda_cs); )
}


int agenda_get_queue_pos(LPAGENDAPROC job) {
  AGENDA_CS(EnterCriticalSection(&agenda_cs); )
  int n=agenda_length-1;
  for(;n>=0;n--)
    if(agenda[n].perform==job) break;
  AGENDA_CS(LeaveCriticalSection(&agenda_cs); )
  return n;
}

void agenda_acia_tx_delay_IKBD(int) {
  acia[ACIA_IKBD].tx_flag=0; //finished transmitting
  if(acia[ACIA_IKBD].tx_irq_enabled) 
    acia[ACIA_IKBD].irq=true;
  mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(acia[ACIA_IKBD].irq||acia[ACIA_MIDI].irq));
}


void agenda_acia_tx_delay_MIDI(int) {
  acia[ACIA_MIDI].tx_flag=0; //finished transmitting
  if(OPTION_C1)
  {
    acia[ACIA_MIDI].sr|=BIT_1; // TDRE
    if((acia[ACIA_MIDI].cr&BIT_5)&&!(acia[ACIA_MIDI].cr&BIT_6)) // IRQ transmit enabled
    {
      acia[ACIA_MIDI].sr|=BIT_7; // IRQ
      mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,
        !((acia[ACIA_IKBD].sr&BIT_7)||(acia[ACIA_MIDI].sr&BIT_7)));
    }
    return;
  }
  if(acia[ACIA_MIDI].tx_irq_enabled) 
    acia[ACIA_MIDI].irq=true;
  mfp_gpip_set_bit(MFP_GPIP_ACIA_BIT,!(acia[ACIA_IKBD].irq||acia[ACIA_MIDI].irq));
}

#undef LOGSECTION


#ifndef NO_CRAZY_MONITOR

void call_a000() {
  //TRACE_INIT("call_a000()\n");
  on_rte_return_address=(pc);
  //now save regs a0,a1,d0 ?
  on_rte=ON_RTE_LINE_A;
  DPEEK(0)=0xa000; //SS ?
//    m68k_interrupt(LPEEK(BOMBS_LINE_A*4));
  UPDATE_SR;
  WORD saved_sr=SR;
  if(!SUPERFLAG) 
    change_to_supervisor_mode();
  m68k_PUSH_L(0);
#if defined(SSE_DEBUGGER_PSEUDO_STACK)
  Debug.PseudoStackPush(pc);
#endif
  m68k_PUSH_W(saved_sr);
  SET_PC(LPEEK(BOMBS_LINE_A*4));
//  DBG_LOG(EasyStr("interrupt - increasing interrupt depth from ")+interrupt_depth+" to "+(interrupt_depth+1));
  CLEAR_T;
  interrupt_depth++;
  memcpy(save_r,Cpu.r,16*4);
  on_rte_interrupt_depth=interrupt_depth;
}


void extended_monitor_hack() {
  em_width&=-16;
  if(line_a_base==0)
  {
    line_a_base=areg[0];
    LPEEK(0)=ROM_LPEEK(0);
    memcpy(Cpu.r,save_r,15*4);
  }
  int real_planes=em_planes;
  if(screen_res==1) 
    real_planes=2;
//  log_write(EasyStr("doing the em hack - line-A base at ")+HEXSl(line_a_base,6));
  m68k_dpoke(line_a_base-12,WORD(em_width));            //V_REZ_HZ -12  WORD  Horizontal pixel resolution.
  m68k_dpoke(line_a_base-4,WORD(em_height));            //V_REZ_VT -4  WORD  Vertical pixel resolution.
  m68k_dpoke(line_a_base-2,WORD(em_width*real_planes/8)); //BYTES_LIN -2  WORD  Bytes per screen line.
  m68k_dpoke(line_a_base,WORD(real_planes));              //PLANES 0  WORD  Number of planes in the current resolution
  m68k_dpoke(line_a_base+2,WORD(em_width*real_planes/8)); //WIDTH 2  WORD  Width of the destination form in bytes
  int h=8;if(em_planes==1)h=16; //height of a character
  m68k_dpoke(line_a_base-40,WORD(em_width*real_planes*h/8)); //V_CEL_WR -40  WORD  Number of bytes between character cells
  m68k_dpoke(line_a_base-44,WORD(em_width/8-1));        //V_CEL_MX -44  WORD  Number of text columns - 1.
  m68k_dpoke(line_a_base-42,WORD(em_height/h-1));      //V_CEL_MY -42  WORD  Number of text rows - 1.
  if(!vdi_intout)
    Tos.HackMemoryForExtendedMonitor();
  if(vdi_intout)
  {
//    log_write("Changing intout[0] etc.");
    m68k_dpoke(line_a_base-692,WORD(em_width-1));
    m68k_dpoke(line_a_base-690,WORD(em_height-1));
//    log_write(EasyStr("Wrote the new dimensions to $")+HEXSl(line_a_base-692,6));
    m68k_dpoke(vdi_intout,WORD(em_width-1));
    m68k_dpoke(vdi_intout+2,WORD(em_height-1));
//    log_write(EasyStr("Wrote the new dimensions to $")+HEXSl(vdi_intout,6));
  }
}

#endif


void emudetect_init() {
  emudetect_falcon_stpal.Resize(256);
  emudetect_falcon_pcpal.Resize(256);
}


void emudetect_reset() {
  emudetect_called=0;
  emudetect_write_logs_to_printer=0;
  emudetect_falcon_stpal.DeleteAll();
  emudetect_falcon_pcpal.DeleteAll();
#if !defined(SSE_NO_FALCONMODE)
  emudetect_falcon_mode=EMUD_FALC_MODE_OFF;
#endif
  emudetect_falcon_mode_size=1;
}


void emudetect_falcon_palette_convert(int n) {
  if(BytesPerPixel==1||emudetect_called==0) 
    return; // 256 mode could cause slow down, can't make it work.
  DWORD val=emudetect_falcon_stpal[n];
  emudetect_falcon_pcpal[n]=colour_convert(DWORD_B(&val,0),DWORD_B(&val,1),DWORD_B(&val,3));
}


void ASMCALL emudetect_falcon_draw_scanline(int border1,int picture,
                                            int border2,int hscroll) {
  if(emudetect_called==0||draw_lock==0) 
    return;
  int st_line_bytes=emudetect_falcon_mode_size*320*emudetect_falcon_mode;
  MEM_ADDRESS source=shifter_draw_pointer&0xffffff;
  if(source+st_line_bytes>mem_len) 
    return;
  int wh_mul=1;
  if(emudetect_falcon_mode_size==1
    &&draw_blit_source_rect.right>320+BORDER_SIDE+BORDER_SIDE)
    wh_mul=2;
  // border always multiple of 4 so write using longs only
  DWORD bord_col=0xffffffff;
  if(emudetect_falcon_mode==1) bord_col=emudetect_falcon_pcpal[0];
#if !defined(SSE_VID_32BIT_ONLY)
  if(BytesPerPixel==1) 
    border1/=4,border2/=4,bord_col|=(bord_col&0xffff)<<16;
  if(BytesPerPixel==2) 
    border1/=2,border2/=2;
#endif
  int DestInc=4;
#if !defined(SSE_VID_32BIT_ONLY)
  if(BytesPerPixel==3) 
    DestInc=3;
#endif
  for(int y=0;y<wh_mul;y++)
  {
    LPDWORD plDest=LPDWORD(draw_dest_ad);
    for(int x=0;x<wh_mul;x++)
    {
      for(int n=border1;n>0;n--)
      {
        *plDest=bord_col;
        plDest=LPDWORD(LPBYTE(plDest)+DestInc);
      }
    }
#if !defined(SSE_VID_32BIT_ONLY)
    LPBYTE pbDest=LPBYTE(plDest);
    LPWORD pwDest=LPWORD(plDest);
#endif
    source+=hscroll*emudetect_falcon_mode;
    if(emudetect_falcon_mode==1)
    {
      DWORD col;
      for(int n=picture;n>0;n--)
      {
        col=emudetect_falcon_pcpal[PEEK(source++)];
        for(int x=0;x<wh_mul;x++)
        {
          switch(BytesPerPixel) {
#if !defined(SSE_VID_32BIT_ONLY)
          case 1: *(pbDest++)=BYTE(col); break;
          case 2: *(pwDest++)=WORD(col); break;
          case 3: *(plDest)=col; plDest=LPDWORD(LPBYTE(plDest)+3); break;
#endif
          case 4: *(plDest++)=col; break;
          }
        }
      }
    }
    else if(emudetect_falcon_mode==2)
    {
      DWORD src;
      for(int n=picture;n>0;n--)
      {
        src=DPEEK(source);source+=2;
        int red=(src & MAKEBINW(b11111000,b00000000))>>11;
        int green=(src & MAKEBINW(b00000111,b11100000))>>5;
        int blue=(src & MAKEBINW(b00000000,b00011111));
        for(int x=0;x<wh_mul;x++)
        {
          switch(BytesPerPixel) {
#if !defined(SSE_VID_32BIT_ONLY)
          case 1: *(pbDest++)=0; break;
          case 2:
            if(rgb555)
              *(pwDest++)=WORD((red<<(15-5))|((green & ~1)<<(10-6))|blue);
            else
              *(pwDest++)=WORD((red<<(16-5))|(green<<(11-6))|blue);
            break;
#endif
          case 3:case 4:
            *(plDest)=((red<<(24-5))|(green<<(16-6))|(blue<<(8-5)))<<rgb32_bluestart_bit;
            plDest=LPDWORD(LPBYTE(plDest)+DestInc);
            break;
          }
        }
      }
    }
#if !defined(SSE_VID_32BIT_ONLY)
    if(BytesPerPixel==1) 
      plDest=LPDWORD(pbDest);
    if(BytesPerPixel==2) 
      plDest=LPDWORD(pwDest);
#endif
    for(int x=0;x<wh_mul;x++)
    {
      for(int n=border2;n>0;n--)
      {
        *plDest=bord_col;
        plDest=LPDWORD(LPBYTE(plDest)+DestInc);
      }
    }
    draw_dest_ad+=draw_line_length;
  }
}


#if defined(SSE_DISK_CAPS) 
TCaps Caps; //this object includes a controller and 2 drives
#endif
#if defined(SSE_DISK_GHOST)
// Each drive has its own optional ghost image
// Most will use A: but e.g. Lethal Xcess could save on B:
TGhostDisk GhostDisk[2];
#endif
#if defined(SSE_DISK_STW)
TImageSTW ImageSTW[3];
#endif
#if defined(SSE_DISK_SCP)
TImageSCP ImageSCP[3];
#endif
#if defined(SSE_DISK_HFE)
TImageHFE ImageHFE[3];
#endif
THardDiskManager HardDiskMan;
#if defined(SSE_ACSI)
TAcsiHardDiskManager AcsiHardDiskMan;
#endif


#if defined(SSE_MEGASTF_RTC)
/*  Partial emulation of the Ricoh Real Time Clock of the Mega ST.
    The time will always read as your PC time, whatever you write.
-------+-----+-----------------------------------------------------+----------
##############Realtime Clock                                       ###########
-------+-----+-----------------------------------------------------+----------
$FFFC21|byte |S_Units                                              |???
$FFFC23|byte |S_Tens                                               |???
$FFFC25|byte |M_Units                                              |???
$FFFC27|byte |M_Tens                                               |???
$FFFC29|byte |H_Units                                              |???
$FFFC2B|byte |H_Tens                                               |???
$FFFC2D|byte |Weekday                                              |???
$FFFC2F|byte |Day_Units                                            |???
$FFFC31|byte |Day_Tens                                             |???
$FFFC33|byte |Mon_Units                                            |???
$FFFC35|byte |Mon_Tens                                             |???
$FFFC37|byte |Yr_Units                                             |???
$FFFC39|byte |Yr_Tens                                              |???
$FFFC3B|byte |Cl_Mod                                               |???
$FFFC3D|byte |Cl_Test                                              |???
$FFFC3F|byte |Cl_Reset                                             |???
*/

// masks of valid bits, non-valid read as zero (unimportant)

// and high byte?

BYTE trp5c15_mask[2][16]={0xF,0x7,0xF,0x7,0xF,0x3,0x7,0xF,0x3,0xF,0x1,0xF,0xF,0xD,0xF,0xF,
                          0x7,0x1,0xF,0x7,0xF,0x3,0x7,0xF,0x3,0x0,0x1,0x3,0x0,0xD,0xF,0xF};

TRp5c15 MegaRtc;

#define LOGSECTION LOGSECTION_IKBD

WORD TRp5c15::Read(MEM_ADDRESS addr) {
  int bank=reg[0][0xD]&1;
  int regn=(addr-0xFFFC20)/2;
  WORD x=reg[bank][regn];
  if(!bank) // bank 1 used for tests, but diagnostic cartridge also uses 0
  {         // which we don't emulate
    time_t t=time(NULL);
    struct tm *lpTime=localtime(&t);
    switch(regn) {
    case 0x0: // S_Units                                              |???
      x=(WORD)lpTime->tm_sec%10;
      break;
    case 0x1: // S_Tens                                               |???
      x=(WORD)lpTime->tm_sec/10;
      break;
    case 0x2: // M_Units                                              |???
      x=(WORD)lpTime->tm_min%10;
      break;
    case 0x3: // M_Tens                                               |???
      x=(WORD)lpTime->tm_min/10;
      break;
    case 0x4: // H_Units                                              |???
      x=(WORD)lpTime->tm_hour%10;
      break;
    case 0x5: // H_Tens                                               |???
      x=(WORD)lpTime->tm_hour/10;
      break;
    case 0x6: // Weekday                                              |???
      x=(WORD)lpTime->tm_wday;
      break;
    case 0x7: // Day_Units                                            |???
      x=(WORD)lpTime->tm_mday%10;
      break;
    case 0x8: // Day_Tens                                             |???
      x=(WORD)lpTime->tm_mday/10;
      break;
    case 0x9: // Mon_Units                                            |???
      x=(WORD)(lpTime->tm_mon+1)%10;
      break;
    case 0xA: // Mon_Tens                                             |???
      x=(WORD)(lpTime->tm_mon+1)/10;
      break;
    case 0xB: // Yr_Units                                             |???
      x=(WORD)(lpTime->tm_year+1900-1980)%10;
      break;
    case 0xC: // Yr_Tens                                              |???
      x=(WORD)(lpTime->tm_year+1900-1980)/10;
      break;
    case 0xD: // Cl_Mod                                               |???
    case 0xE: // Cl_Test                                              |???
    case 0xF: // Cl_Reset                                             |???
      break;
    }
  }
  x&=trp5c15_mask[bank][regn];
  TRACE_LOG("PC %X read addr %X reg %d-%X = %d\n",old_pc,addr,bank,regn,x);
  return x;
}


void TRp5c15::Write(MEM_ADDRESS addr,BYTE io_src_b) {
  int bank=reg[0][0xD]&1;
  int regn=(addr-0xFFFC20)/2;
  reg[bank][regn]=io_src_b&trp5c15_mask[bank][regn];
  if(regn>=0xD&&regn<=0xF)
    reg[1-bank][regn]=reg[bank][regn];
  TRACE_LOG("PC %X write addr %X reg %d-%X = %d\n",old_pc,addr,bank,regn,reg[bank][regn]);
}

#undef LOGSECTION

#endif


#if defined(SSE_STATS)
/*  v4.0.1's unique feature, it was much fun doing it and it can be
    quite revealing, like FPS.
    The report is meant to be rich text, but we keep a plain version too.
    RTF format is verbose so that looks like a lot of code but the idea is simple.
    While running, object Stats is updated at low overhead cost.
    When stopping, if the Status page is open, the report is saved as an RTF
    file then loaded in the page.
    When the Status page gets open, the report is remade (also while running,
    but then some stats are not updated).
*/

TStats Stats;

void TStats::Report() {
  EasyStr stats_file=RunDir+SLASH+STEEM_STATS_FILENAME; 
  FILE *fp=fopen(stats_file, "w");
  if(fp)
  {
#if defined(SSE_STATS_RTF)
    fprintf(fp,"{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0\\froman Times;}{\\f1\\fswiss\
 Arial;}{<\\f2<\\fmodern Courier New;}}");
    fprintf(fp,"{\\colortbl;\\red0\\green0\\blue0;\\red0\\green0\\blue255;\
\\red0\\green255\\blue255;\\red0\\green255\\blue0;\\red255\\green0\\blue255;\
\\red255\\green0\\blue0;\\red255\\green255\\blue0;\\red255\\green255\\blue255;\
\\red0\\green0\\blue128;\\red0\\green128\\blue128;\\red0\\green128\\blue0;\
\\red128\\green0\\blue128;\\red128\\green0\\blue0;\\red128\\green128\\blue0;\
\\red128\\green128\\blue128;\\red192\\green192\\blue192;}");
    char nl[]="\\par\n";
//    char sSte[]="\\b (STE)\\b0";
    char sSte[]="\\cf6 (STE) \\cf0 "; //red
#else
    char nl[]="\n";
    char sSte[]="(STE) ";
#endif
    char s1frame[]="1 frame";
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b STATISTICS\\b0 %s",nl);
#else
    fprintf(fp,"STATISTICS%s",nl);
#endif
    int th=run_time/1000;
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b Time\\b0: %d.%02ds  \\b CPU\\b0: %d%%  \\b Slow-downs\\b0: "
      PRICV "%s",th,run_time-th*1000,tCpuUsage,nSlowdown,nl);
    fprintf(fp,"\\b Disk\\b0 %s",nl);
#else
    fprintf(fp,"Time: %d.%ds  CPU: %02d%%%s",th,run_time-th*1000,tCpuUsage,nl);
    fprintf(fp,"Disk%s",nl);
#endif
    for(BYTE d=0;d<2;d++)
    {
      if(FloppyDrive[d].m_DiskInDrive)
        fprintf(fp,"%c: %s %s%s",'A'+d,GetFileNameFromPath(FloppyDisk[d].ImageFile.Text),
        FloppyDisk[d].RealDiskInZip.Text,nl);
      if(nSector[d])
        fprintf(fp,"%c: max side %d, max track %d, max sector %d #sectors %d%s",
        'A'+d,nSide[d],nTrack[d],nSector[d],nSector2[d],nl);
      if(boot_checksum[d][0])
        fprintf(fp,"%c0: boot checksum %X%s",'A'+d,boot_checksum[d][0],nl);
      if(boot_checksum[d][1])
        fprintf(fp,"%c1: boot checksum %X%s",'A'+d,boot_checksum[d][1],nl);
    }
    if(nHdsector)
      fprintf(fp,"HD sectors: " PRICV "%s",nHdsector,nl);
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b OS\\b0 %s",nl);
#else
    fprintf(fp,"OS%s",nl);
#endif
    if(nPrg)
      fprintf(fp,"Programs: " PRICV "%s",nPrg,nl);
    fprintf(fp,"System calls%sGEMDOS: " PRICV " (intercepted: " PRICV ") BIOS: "
PRICV " (" PRICV ") XBIOS: " PRICV " (" PRICV ") VDI: " PRICV " (" PRICV ") AES: "
PRICV "%s",
 nl,nGemdos,nGemdosi,nBios,nBiosi,nXbios,nXbiosi,nVdi,nVdii,nAes,nl);

#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b Input\\b0%s",nl);
#else
    fprintf(fp,"Input%s",nl);
#endif
    if(nKeyIn)
      fprintf(fp,"Keys: " PRICV "%s",nKeyIn,nl);
    if(nMousex||nMousey)
      fprintf(fp,"Mouse X: " PRICV " Y: " PRICV "%s",nMousex,nMousey,nl);
    if(nJoy0)
      fprintf(fp,"Joystick 0: " PRICV "%s",nJoy0,nl);
    if(nClick0)
      fprintf(fp,"Left click/fire 0: " PRICV "%s",nClick0/2,nl);
    if(nJoy1)
      fprintf(fp,"Joystick 1: " PRICV "%s",nJoy1,nl);
    if(nClick1)
      fprintf(fp,"Right click/fire 1: " PRICV "%s",nClick1/2,nl);
    if(mskSpecial)
    {
#if defined(SSE_STATS_RTF)
      fprintf(fp,"\\b Special\\b0%s",nl);
#else
      fprintf(fp,"Special%s",nl);
#endif
      if(mskSpecial&IKBD_22)
        fprintf(fp,"IKBD reprogramming%s",nl);
      if(mskSpecial&EMU_DETECT)
        fprintf(fp,"Emu detect%s",nl);
      if(mskSpecial&WRITE_DISK)
        fprintf(fp,"Write disk%s",nl);
    }
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b Video\\b0 %s",nl);
#else
    fprintf(fp,"Video%s",nl);
#endif
    fprintf(fp,"Frames: " PRICV "%s",nFrame,nl);
    fprintf(fp,"Frequency: %dHz Resolution: %d%s",Glue.previous_video_freq,
      screen_res,nl);
    if(video_mixed_output)
      fprintf(fp,"Mixed resolutions%s",nl);
    if(OPTION_OSD_FPSINFO && nFps) // it's optional because of the load
      fprintf(fp,"ST FPS: %d%s",nFps,nl);
    fprintf(fp,"Overscan mask: %X, %s: %X%s",mskOverscan,s1frame,
      mskOverscan1,nl);
    if(mskOverscan1) // detail overscan tricks of last frame
    {
      fprintf(fp,"Overscan %s: ",s1frame);
      if(mskOverscan1&TRICK_TOP_OVERSCAN)
        fprintf(fp,"Top ");
      if(mskOverscan1&(TRICK_BOTTOM_OVERSCAN|TRICK_BOTTOM_OVERSCAN_60HZ))
        fprintf(fp,"Bottom ");
      if(mskOverscan1&(TRICK_LINE_PLUS_26|TRICK_LINE_PLUS_20))
        fprintf(fp,"Left ");
      if(mskOverscan1&TRICK_LINE_PLUS_44)
        fprintf(fp,"Right ");
      if(mskOverscan1&TRICK_0BYTE_LINE)
        fprintf(fp,"0 byte ");
      if(mskOverscan1&(TRICK_LINE_MINUS_106|TRICK_4BIT_SCROLL|TRICK_LINE_PLUS_2
        |TRICK_LINE_MINUS_106|TRICK_LINE_MINUS_2))
        fprintf(fp,"Sync-scroll");
      fprintf(fp,"%s",nl);
    }
    if(nLinePlus16)
      fprintf(fp,"%s+16 pixels: " PRICV "%s",sSte,nLinePlus16,nl);
    if(nTimerb)
      fprintf(fp,"Timer B: " PRICV "%s",nTimerb,nl);
    if(nTimerbtick)
      fprintf(fp,"Timer B ticks (%s): " PRICV "%s",s1frame,nTimerbtick,nl);
    if(nPal)
      fprintf(fp,"Palette writes (%s): %d%s",s1frame,nPal,nl);
    if(nReadvc)
      fprintf(fp,"Read video counter: " PRICV ", %s: %d%s",nReadvc,s1frame,
        nReadvc1,nl);
#ifdef WIN32 //todo        
    // counting colours is not as easy!
    // we read back our DirectX backbuffer and count different pixels (slow)
    if(nFrame && (Disp.Method==DISPMETHOD_DD||Disp.Method==DISPMETHOD_D3D))
    { 
      HRESULT DErr;
#if defined(SSE_VID_DD)
      Disp.OurBackSur=(OPTION_3BUFFER_WIN&&Disp.DDBackSur2&&Disp.SurfaceToggle)
        ? Disp.DDBackSur2 : Disp.DDBackSur;
      DErr=Disp.OurBackSur->Lock(NULL,&Disp.DDBackSurDesc,
        DDLOCK_READONLY,NULL);
      //LONG len=Disp.DDBackSurDesc.lPitch*Disp.DDBackSurDesc.dwHeight;
      DWORD dwpitch=Disp.DDBackSurDesc.lPitch/4;
      DWORD *pDataStart=(DWORD*)Disp.DDBackSurDesc.lpSurface;
#elif defined(SSE_VID_D3D)
      IDirect3DSurface9 *BackBuff=NULL;
      DErr=Disp.pD3DDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&BackBuff);
      D3DLOCKED_RECT LockedRect;
      //LONG len=0;
      DWORD *pDataStart=NULL,dwpitch=0;
      if(DErr==DD_OK)
      {
        // locking the texture again may fail, the backbuffer seems to be always OK
        DErr=BackBuff->LockRect(&LockedRect,NULL,0);
      //  len=LockedRect.Pitch*Disp.SurfaceHeight;
        dwpitch=LockedRect.Pitch/4;
        pDataStart=(DWORD*)LockedRect.pBits;
      }
#endif
      if(DErr==DD_OK)
      {
//        DWORD *pDataEnd=pDataStart+len/4;
        DWORD *pData=pDataStart;
        WORD nColour=0;
        DWORD ix=0;
        DWORD *pPixel=new DWORD[4096]; // assume 32bit, 4096 col. max
        ZeroMemory(pPixel,4096*sizeof(DWORD)); // normally useless...
        LONG h=draw_blit_source_rect.bottom-draw_blit_source_rect.top;
        LONG w=draw_blit_source_rect.right-draw_blit_source_rect.left;
        for(LONG y=0;y<h;y++)
        {
          for(LONG x=0;x<w;x++)
          {
            DWORD data=(*(pData+y*dwpitch+x) & 0x00FFFFFF); // assume X8R8G8B8
            WORD i;
            for(i=0;i<nColour;i++) // for first pixel, i=nColour=0
              if(pPixel[i]==data)
                break;
            if(i==nColour)
            {
              pPixel[nColour]=data;
              //TRACE2("%d,%d %d colour %d = %X\n",x,y,ix,i,data);
              nColour++; // can be OSD!
            }
            ix++;
          }//nxt x
        }//nxt y
        delete [] pPixel;
#if defined(SSE_VID_DD)
        VERIFY(!Disp.OurBackSur->Unlock(NULL));
#elif defined(SSE_VID_D3D)
       // VERIFY(!Disp.pD3DTexture->UnlockRect(0));
        VERIFY(!BackBuff->UnlockRect());
        VERIFY(!BackBuff->Release());
#endif
#if defined(SSE_STATS_RTF)
        if(nColour>16) // in blue
          fprintf(fp,"Colours (%s): \\cf2 %d\\cf0%s",s1frame,nColour,nl);
        else
#endif
          fprintf(fp,"Colours (%s): %d%s",s1frame,nColour,nl);
      }
      else
        fprintf(fp,"Can't get backsurface ERR %d%s",DErr,nl);
    }
#endif//WIN32
    if(nBlit)
      fprintf(fp,"%sBlit: " PRICV " (hog: " PRICV ", TOS: " PRICV 
        "), %s: %d%s",sSte,nBlit,nBlith,nBlitT,s1frame,nBlit1,nl);
    if(nExtendedPal)
      fprintf(fp,"%s4096 colour palette: " PRICV " (TOS: " PRICV ")%s",sSte,
      nExtendedPal,nExtendedPalT,nl);
    if(nVscroll)
      fprintf(fp,"%sVertical scroll: " PRICV "%s",sSte,nVscroll,nl);
    if(nHscroll)
      fprintf(fp,"%sHorizontal scroll: " PRICV "%s",sSte,nHscroll,nl);
    if(nScreensplit)
      fprintf(fp,"%sScreen split/scroll: " PRICV ", %s: %d%s",sSte,nScreensplit,
        s1frame,nScreensplit1,nl);
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b Sound\\b0 %s",nl);
#else
    fprintf(fp,"Sound%s",nl);
#endif
    if(nPsgSound)
      fprintf(fp,"PSG: " PRICV "%s",nPsgSound,nl);
    for(int mode=0;mode<4;mode++)
      if(mskDigitalSound&(1<<mode))
        fprintf(fp,"%sDigital Sound: %dHz %s%s",sSte,ste_sound_mode_to_freq
         [mode],((mskDigitalSound)&(1<<(mode+4)))?"Mono":"Stereo",nl);
    if(nMicrowire)
      fprintf(fp,"%sMicrowire: " PRICV " (TOS: " PRICV ")%s",sSte,nMicrowire,
        nMicrowireT,nl);
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b I/O\\b0 %s",nl);
#else
    fprintf(fp,"I/O%s",nl);
#endif
    for(int i=0;i<3;i++)
    {
      if(nPorti[i])
        fprintf(fp,"%s in: " PRICV "%s",STPort[i].Name.Text,nPorti[i],nl);
      if(nPorto[i])
        fprintf(fp,"%s out: " PRICV "%s",STPort[i].Name.Text,nPorto[i],nl);
    }
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b Exceptions\\b0 %s",nl);
#else
    fprintf(fp,"Exceptions%s",nl);
#endif
    if(nStop)
      fprintf(fp,"Stop: " PRICV "%s",nStop,nl);
    for(int i=0;i<12;i++)
      if(nException[i])
        fprintf(fp,"Exception %d: " PRICV "%s",i,nException[i],nl);
    if(nHbi)
      fprintf(fp,"Horizontal sync: " PRICV ", %s: %d%s",nHbi,s1frame,nHbi1,nl);
    fprintf(fp,"Vertical sync: " PRICV "%s",nVbi,nl);
    for(BYTE irq=0;irq<15;irq++)
      if(nMfpIrq[irq] || Mfp.IrqInfo[irq].IsTimer 
        && nMfpTimeout[Mfp.IrqInfo[irq].Timer])
        if(Mfp.IrqInfo[irq].IsTimer)
          fprintf(fp,"Mfp irq %d: " PRICV " (" PRICV " timeouts, %dHz)%s",irq,
            nMfpIrq[irq],nMfpTimeout[Mfp.IrqInfo[irq].Timer],
            fTimer[Mfp.IrqInfo[irq].Timer],nl); // freq at last timeout
        else
          fprintf(fp,"Mfp irq %d: " PRICV "%s",irq,nMfpIrq[irq],nl);
    if(nSpurious)
      fprintf(fp,"Spurious: " PRICV "%s",nSpurious,nl);
    for(BYTE trap=0;trap<15;trap++)
      if(nTrap[trap])
        fprintf(fp,"Trap #%d: " PRICV "%s",trap,nTrap[trap],nl);
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b %sSTATE OF CHIPS%s\\f2 68000\\b0 %s",nl,nl,nl);
#else
    fprintf(fp,"%sSTATE OF CHIPS%s68000%s",nl,nl,nl);
#endif
#if defined(SSE_MEGASTE)
    if(IS_MEGASTE)
    {
      fprintf(fp,"%dMHz Cache:%c%s",8<<(MegaSte.MemCache.ScuReg&BIT_0),
        (MegaSte.MemCache.ScuReg&BIT_1) ? 'Y' : 'N',nl);
    }
#endif
    UPDATE_SR;
    fprintf(fp,"PC:%08X IR:%04X SR:%04X IPL:%d%s",pc,IRD,SR,
      ipl_timing[ipl_timing_index].ipl,nl);
    for(int i=0;i<4;i++)
      fprintf(fp,"D%d:%08X ",i,Cpu.r[i]);
    fprintf(fp,"%s",nl);
    for(int i=4;i<8;i++)
      fprintf(fp,"D%d:%08X ",i,Cpu.r[i]);
    fprintf(fp,"%s",nl);
    for(int i=0;i<4;i++)
      fprintf(fp,"A%d:%08X ",i,areg[i]);
    fprintf(fp,"%s",nl);
    for(int i=4;i<8;i++)
      fprintf(fp,"A%d:%08X ",i,areg[i]);
    fprintf(fp,"%sUSP:%08X SSP:%08X%s",nl,USP,SSP,nl);
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b 68901\\b0 %s",nl);
#else
    fprintf(fp,"68901%s",nl);
#endif
    for(BYTE reg=0;reg<24;reg++)
    {
      fprintf(fp,"%02d:%02X ",reg,Mfp.reg[reg]);
      if((reg&7)==7)
        fprintf(fp,"%s",nl);
    }
#if defined(SSE_HD6301_LL)
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b 6301\\b0 %s",nl);
#else
    fprintf(fp,"6301%s",nl);
#endif
    if(OPTION_C1)
    {
      fprintf(fp,"PC:%04X OP:%02X CCR:%02X SP:%04X D:%04X X:%04X%s",
        hd6301_peek(-1),hd6301_peek(-6),hd6301_peek(-2),hd6301_peek(-3),
        hd6301_peek(-4),hd6301_peek(-5),nl);
      for(BYTE reg=0;reg<16;reg++)
      {
        fprintf(fp,"%02d:%02X ",reg,hd6301_peek(reg));
        if((reg&7)==7)
          fprintf(fp,"%s",nl);
      }
    }
    else
      fprintf(fp,"NOT ACTIVE%s",nl);
#endif
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b 6850\\b0 %s",nl);
#else
    fprintf(fp,"6850%s",nl);
#endif
    for(int i=0;i<2;i++)
      fprintf(fp,"ACIA %d CR:%02X SR:%02X RDR:%02X TDR:%02X%s",
      acia[i].Id,acia[i].cr,acia[i].sr,acia[i].rdr,acia[i].tdr,nl);
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b YM2149\\b0 %s",nl);
#else
    fprintf(fp,"YM2149%s",nl);
#endif
    for(BYTE reg=0;reg<16;reg++)
    {
      fprintf(fp,"%02d:%02X ",reg,psg_reg[reg]);
      if((reg&7)==7)
        fprintf(fp,"%s",nl);
    }
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b WD1772\\b0 %s",nl);
#else
    fprintf(fp,"WD1772%s",nl);
#endif
    fprintf(fp,"CR:%02X STR:%02X TR:%02X SR:%02X DR:%02X DSR:%02X CRC:%04X%s",
      Fdc.cr,Fdc.str,Fdc.tr,Fdc.sr,Fdc.dr,Fdc.dsr,Fdc.CrcLogic.crc,nl);
#if defined(SSE_STATS_RTF)
    fprintf(fp,(IS_STE) ? "\\b GSTMCU\\b0 %s" : "\\b MMU\\b0 %s",nl);
#else
    fprintf(fp,(IS_STE) ? "GSTMCU%s" : "MMU%s",nl);
#endif
    DU32 usdp;
    usdp.d32=Mmu.VideoCounter;
    fprintf(fp,"001:%02X 201:%02X 203:%02X 205:%02X 207:%02X 209:%02X%s",
      Mmu.MemConfig,Mmu.u_vbase.d8[B2],Mmu.u_vbase.d8[B1],usdp.d8[B2],
      usdp.d8[B1],usdp.d8[B0],nl);
    fprintf(fp,"609:%02X 60B:%02X 60D:%02X%s",
      Mmu.uDmaCounter.d8[B2],Mmu.uDmaCounter.d8[B1],Mmu.uDmaCounter.d8[B0],nl);
    if(IS_STE)
    {
      fprintf(fp,"20D:%02X 20F:%02X 901:%02X 903:%02X 905:%02X 907:%02X%s",
        Mmu.u_vbase.d8[B0],Mmu.linewid,Mmu.sound_control,
        Mmu.u_next_sound_frame_start.d8[B2],Mmu.u_next_sound_frame_start.d8[B1],
        Mmu.u_next_sound_frame_start.d8[B0],nl);
      fprintf(fp,"909:%02X 90B:%02X 90D:%02X 90F:%02X 911:%02X 913:%02X%s",
        Mmu.u_sound_fetch_address.d8[B2],Mmu.u_sound_fetch_address.d8[B1],
        Mmu.u_sound_fetch_address.d8[B2],Mmu.u_next_sound_frame_end.d8[B2],
        Mmu.u_next_sound_frame_end.d8[B1],Mmu.u_next_sound_frame_end.d8[B0],nl);
    }
    if(IS_STF)
#if defined(SSE_STATS_RTF)
      fprintf(fp,"\\b GLU\\b0 %s",nl);
#else
      fprintf(fp,"GLU%s",nl);
#endif
    fprintf(fp,"20A:%02X 260:%02X%s",Glue.m_SyncMode,Glue.m_ShiftMode,nl);
    if(IS_STE)
      fprintf(fp,"265:%02X 900:%02X %s",Glue.hscroll,Glue.gamecart,nl);
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b Shifter\\b0 %s",nl);
#else
    fprintf(fp,"Shifter%s",nl);
#endif
    for(BYTE reg=0;reg<16;reg++)
    {
      fprintf(fp,"%02d:%04X ",reg,STpal[reg]);
      if((reg&3)==3)
        fprintf(fp,"%s",nl);
    }
    if(IS_STE)
      fprintf(fp,"16:%X 17:%X 18:%X 19:%04X 20:%04X%s",Shifter.m_ShiftMode,
        shifter_hscroll,shifter_sound_mode,Microwire.Data,Microwire.Mask,nl);
    else
      fprintf(fp,"16:%X%s",Shifter.m_ShiftMode,nl);
    //fprintf(fp,"16:%X%s",Shifter.m_ShiftMode);// TEST SEH exception in other module
    //int a=0; int b=5/a;  printf("yoho %d",b);// TEST SEH exception
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\b DMA\\b0 %s",nl);
#else
    fprintf(fp,"DMA%s",nl);
#endif
    fprintf(fp,"MCR:%02X SR:%02X CTR:%02X CNT:%02X FIFO:%d-%d%s",Dma.mcr,
      Dma.sr,Dma.Counter,Dma.ByteCount,Dma.BufferInUse,Dma.Fifo_idx,nl);
    for(int fifo_n=0;fifo_n<2;fifo_n++)
    {
      for(int i=0;i<8;i++) // as words but our fifos contain bytes
        fprintf(fp,"%02X%02X ",Dma.Fifo[fifo_n][i*2],Dma.Fifo[fifo_n][i*2+1]);
      fprintf(fp,"%s",nl);
    }
    if(SSEConfig.Blitter)
    {
#if defined(SSE_STATS_RTF)
      fprintf(fp,"\\b Blitter\\b0 %s",nl);
#else
      fprintf(fp,"Blitter%s",nl);
#endif
      for(int reg=0;reg<16;reg++)
      {
        fprintf(fp,"%02d:%04X ",reg,Blitter.HalfToneRAM[reg]);
        if((reg&3)==3)
          fprintf(fp,"%s",nl);
      }
      fprintf(fp,"16:%04X 17:%04X 18:%04X 19:%04X%s",(WORD)Blitter.SrcXInc,
        (WORD)Blitter.SrcYInc,Blitter.SrcAdr.d16[HI],Blitter.SrcAdr.d16[LO],nl);
      fprintf(fp,"20:%04X 21:%04X 22:%04X 23:%04X%s",Blitter.EndMask[0],
        Blitter.EndMask[1],Blitter.EndMask[2],(WORD)Blitter.DestXInc,nl);
      fprintf(fp,"24:%04X 25:%04X 26:%04X 27:%04X%s",(WORD)Blitter.DestYInc,
        Blitter.DestAdr.d16[HI],Blitter.DestAdr.d16[LO],Blitter.XCount,nl);
      BYTE hibyte=(BYTE)(Blitter.LineNumber|(Blitter.Smudge<<5)|(Blitter.Hog<<6)
        |(Blitter.Busy<<7));
      BYTE lobyte=(BYTE)(Blitter.Skew|(Blitter.NFSR<<6)|(Blitter.FXSR<<7));
      fprintf(fp,"28:%04X 29:%04X 30:%04X%s",Blitter.YCount,
        (Blitter.Hop<<8)|Blitter.Op,(hibyte<<8)|lobyte,nl);
    }
#if defined(SSE_STATS_RTF)
    fprintf(fp,"\\f0 %s }",nl);
#endif
    fclose(fp);
  }
}

#endif//#if defined(SSE_STATS)

