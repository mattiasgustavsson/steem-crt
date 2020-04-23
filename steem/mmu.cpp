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
FILE: mmu.cpp
DESCRIPTION: The MMU (Memory Management Unit) is an Atari custom chip. 
Its memory management capabilities are rudimentary, it's not to be confused 
with a modern MMU. It also handles the video and, on the STE, sound
memory.
On the STE, the Glue and the Mmu have been merged together, producing the
GSTMCU. In Steem SSE, we do as if they were still separate (even though it's
a tempting inheritance case).
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop

#include <conditions.h>
#include <steemh.h>
#include <emulator.h>
#include <draw.h>
#include <options.h>
#include <palette.h>
#include <display.h>
#include <computer.h>
#include <debug_framereport.h>
#include <sound.h>
#if defined(SSE_VID_STVL_SREQ)
#include <interface_stvl.h>
#endif

////////////
// MEMORY //
////////////

/*
0       128 KB
1       512 KB
2         2 MB
3         0
4         7 MB (Steem hack)
5         6 MB (MonSTer)
*/

const MEM_ADDRESS mmu_bank_length_from_config[N_MEMCONF]=
                  {128*1024,512*1024,2*MEGABYTE,0,7*MEGABYTE
#if defined(SSE_MMU_MONSTER_ALT_RAM)                  
                  ,6*MEGABYTE
#endif
};

BYTE *STMem=NULL;
BYTE *Mem_End,*Mem_End_minus_1,*Mem_End_minus_2,*Mem_End_minus_4;
unsigned long mem_len;
MEM_ADDRESS himem;


#define LOGSECTION LOGSECTION_MMU


MEM_ADDRESS mmu_confused_address(MEM_ADDRESS ad) {
/*  MMU Confused is when the CONFIG register doesn't match real memory.
    see memdetect.txt in 3rdparty/doc

R=row, C=column
If a line isn't connected, bit is 0.

On MMU configured for 2MB

STF decoding=
0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
C C C C C C C C C C R R R R R R R R R R X
9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 X

STE decoding=
0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
C R C R C R C R C R C R C R C R C R C R X
9 9 8 8 7 7 6 6 5 5 4 4 3 3 2 2 1 1 0 0 X

512K bank: C9, R9 
128K bank: C9, R9, C8, R8 not connected

On MMU configured for 512K 

STF decoding=
8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
C C C C C C C C C R R R R R R R R R X
8 7 6 5 4 3 2 1 0 8 7 6 5 4 3 2 1 0 X

STE decoding=
8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
C R C R C R C R C R C R C R C R C R X
8 8 7 7 6 6 5 5 4 4 3 3 2 2 1 1 0 0 X

128K bank: R8, C8 not connected

*/
#ifdef SSE_DEBUG
  MEM_ADDRESS ad1=ad; // save for trace
#endif
  //ASSERT(ad<FOUR_MEGS);
  int bank=0;
  if(ad>FOUR_MEGS)
    return 0xffffff;   //bus error
  else if(ad>=Mmu.bank_length[0])
  {
    bank=1;
    ad-=Mmu.bank_length[0];
    if(ad>=Mmu.bank_length[1])
      return 0xfffffe; //gap
  }
  if(SSEConfig.bank_length[bank]==0)
    ad=0xfffffe; //gap
  // MMU configured for 2MB 
  else if(Mmu.bank_length[bank]==MB2)
  {
    if(SSEConfig.bank_length[bank]==KB512)
    { //real memory
      if(IS_STF)
        ad&=~(BIT_20|BIT_10);
      else
        ad&=~(BIT_20|BIT_19);
    }
    else if(SSEConfig.bank_length[bank]==KB128)
    { //real memory
      if(IS_STF)
        ad&=~(BIT_20|BIT_19|BIT_10|BIT_9);
      else
        ad&=~(BIT_20|BIT_19|BIT_18|BIT_17);
    }
  }//2MB
  // MMU configured for 512K
  else if(Mmu.bank_length[bank]==KB512)
  {
    if(SSEConfig.bank_length[bank]==KB128)
    { //real memory
      if(IS_STF)
        ad&=~(BIT_18|BIT_9); // TOS OK, but diagnostic catridge?
      else
        ad&=~(BIT_18|BIT_17);
    }
  }//512K
  if(bank==1&&ad<FOUR_MEGS)
    ad+=SSEConfig.bank_length[0];
#ifdef SSE_DEBUG
  if((ad1!=ad))
    TRACE_LOG("MMU confused ad %X -> %X\n",ad1,ad);
#endif
  return ad;
}


BYTE mmu_confused_peek(MEM_ADDRESS ad,bool cause_exception) {
  MEM_ADDRESS c_ad=mmu_confused_address(ad);
  if(c_ad==0xffffff)
  {   //bus error
    if(cause_exception)
      exception(BOMBS_BUS_ERROR,EA_READ,ad);
    return 0;
  }
  else if(c_ad==0xfffffe)
    //gap in memory
    return 0xff;
  else if(c_ad<mem_len)
    return PEEK(c_ad);
  else
    return 0xff;
}


WORD mmu_confused_dpeek(MEM_ADDRESS ad,bool cause_exception) {
  MEM_ADDRESS c_ad=mmu_confused_address(ad);
  if(c_ad==0xffffff)
  {   //bus error
    if(cause_exception)
      exception(BOMBS_BUS_ERROR,EA_READ,ad);
    return 0;
  }
  else if(c_ad==0xfffffe)
    //gap in memory
    return 0xffff;
  else if(c_ad<mem_len)
    return DPEEK(c_ad);
  else
    return 0xffff;
}


LONG mmu_confused_lpeek(MEM_ADDRESS ad,bool cause_exception) {
  WORD a=mmu_confused_dpeek(ad,cause_exception);
  WORD b=mmu_confused_dpeek(ad+2,cause_exception);
  return MAKELONG(b,a);
}


void mmu_confused_poke_abus(BYTE x) {
  MEM_ADDRESS c_ad=mmu_confused_address(iabus);
  if(c_ad==0xffffff)  //bus error
    exception(BOMBS_BUS_ERROR,EA_WRITE,iabus);
  else if(c_ad==0xfffffe)  //gap in memory
    ;
  else if((c_ad+1)<=mem_len)
    PEEK(c_ad)=x;
}


void mmu_confused_dpoke_abus(WORD x) {
  MEM_ADDRESS c_ad=mmu_confused_address(iabus);
  if(c_ad==0xffffff)  //bus error
    exception(BOMBS_BUS_ERROR,EA_WRITE,iabus);
  else if(c_ad==0xfffffe)  //gap in memory
    ;
  else if((c_ad+2)<=mem_len)
    DPEEK(c_ad)=x;
}

#undef LOGSECTION


//////////
// DISK //
//////////

// the reference can't be in the struct in BCC, has to be global
MEM_ADDRESS &dma_address=Mmu.uDmaCounter.d32;


/////////////////
// SOUND (STE) //
/////////////////

#define LOGSECTION LOGSECTION_SOUND

MEM_ADDRESS &ste_sound_start=Mmu.u_sound_frame_start.d32=0,
  &next_ste_sound_start=Mmu.u_next_sound_frame_start.d32=0,
  &ste_sound_end=Mmu.u_sound_frame_end.d32=0,
  &next_ste_sound_end=Mmu.u_next_sound_frame_end.d32=0;
MEM_ADDRESS &ste_sound_fetch_address=Mmu.u_sound_fetch_address.d32;

void TMmu::sound_set_control(BYTE io_src_b) {
/*  $FFFF8900 ---- ---- ---- --cc RW Sound DMA Control
    cc:
    00  Sound DMA disabled (reset state)
    01  Sound DMA enabled, disable at end of frame
    11  Sound DMA enabled, repeat frame forever
*/
  if((sound_control & BIT_0)&&(io_src_b & BIT_0)==0)
  {  //Stopping
    TRACE_LOG("%d %d %d STE sound stop ",TIMING_INFO);
    u_sound_frame_start.d32=u_next_sound_frame_start.d32;
    u_sound_frame_end.d32=u_next_sound_frame_end.d32;
    u_sound_fetch_address.d32=u_sound_frame_start.d32;
  }
  else if((io_src_b & BIT_0))
  { //Start playing
    if(OPTION_HACKS && (sound_control&BIT_0) 
      && LINECYCLES>Glue.CurrentScanline.EndCycle) // can be 0
    { // We haven't finished previous frame (timing of sound_fetch() isn't precise)
      // (except if STVL is used)
      // If we're after DE, fetch some samples, it could finish it (Light.prg)
      if(Glue.FetchingLine() // if not, "impossible"to be late (E605 intro)
        && u_sound_frame_end.d32-u_sound_fetch_address.d32<10) // 4 max
      {
        TRACE_LOG("%d %d %d Fetch last samples at %X\n",TIMING_INFO,ste_sound_fetch_address);
        sound_fetch();
      }
    }
    if(sound_control&BIT_0)
    {
      sound_control=(io_src_b&3); // just in case?
      return;
    }
    u_sound_frame_start.d32=u_next_sound_frame_start.d32;
    u_sound_frame_end.d32=u_next_sound_frame_end.d32;
    u_sound_fetch_address.d32=u_sound_frame_start.d32;
    TRACE_LOG("%d %d %d STE sound start loop %d current %x frame %x->%x %d samples ",
      TIMING_INFO,(io_src_b&BIT_1)>>1,u_sound_fetch_address.d32,
      u_sound_frame_start.d32,u_sound_frame_end.d32,
      (u_sound_frame_end.d32-u_sound_frame_start.d32)/2);
    if(u_sound_fetch_address.d32==u_sound_frame_end.d32 
      && (io_src_b&BIT_1)==0) // but if looped it will go (lazer insane)
    {
      // sound_control=(io_src_b&3); // just in case? // well, no!
      TRACE_LOG("STOP\n");
      return; // Froggies bug PC $1723A
    }
#if defined(SSE_VID_STVL_SREQ)
    if(OPTION_C3 && SSEConfig.Stvl>=0x101) // check  version too
      Stvl.sreq=true; // will trigger fetches
    else
#endif
    if(!Glue.FetchingLine())
      sound_fetch(); // fill FIFO
    if(ste_sound_on_this_screen==0)
    {
      // Pad buffer with last byte from VBL to current position
      bool Mono=((shifter_sound_mode&BIT_7)!=0);
      //TRACE_LOG((Mono) ? (char*)"Mono":(char*)"Stereo");
      int freq_idx=0;
      if(video_freq_at_start_of_vbl==60)
        freq_idx=1;
      else if(video_freq_at_start_of_vbl==MONO_HZ)
        freq_idx=2;
      WORD w1,w2;
      Shifter.sound_get_last_sample(&w1,&w2);
      for(int y=-scanlines_above_screen[freq_idx];y<scan_y;y++)
      {
        if(Mono)  //play half as many words
          ste_sound_samples_countdown+=ste_sound_freq
          *scanline_time_in_cpu_cycles_at_start_of_vbl/2;
        else //stereo, 1 word per sample
          ste_sound_samples_countdown+=ste_sound_freq
          *scanline_time_in_cpu_cycles_at_start_of_vbl;
        int loop=int(Mono?2:1);
        while(ste_sound_samples_countdown>=0)
        {
          for(int i=0;i<loop;i++)
          {
            ste_sound_output_countdown+=sound_freq;
            while(ste_sound_output_countdown>=0)
            {
              if(ste_sound_channel_buf_idx>=STE_SOUND_BUFFER_LENGTH)
                break;
              ste_sound_channel_buf[ste_sound_channel_buf_idx++]=w1;
              ste_sound_channel_buf[ste_sound_channel_buf_idx++]=w2;
              ste_sound_output_countdown-=ste_sound_freq;
            }
          }
          ste_sound_samples_countdown-=n_cpu_cycles_per_second;
        }
      }
      ste_sound_on_this_screen=1;
    }//if(ste_sound_on_this_screen==0)
  }
  TRACE_LOG("(Channels %d Freq %d)\n",2-((shifter_sound_mode&BIT_7)>>7),ste_sound_freq);
  LOG_TO(LOGSECTION_SOUND,EasyStr("SOUND: ")+HEXSl(old_pc,6)+" - DMA sound control set to "+(io_src_b & 3)+" from "+(sound_control & 3));
  sound_control=(io_src_b&3);
  mfp_gpip_set_bit(MFP_GPIP_MONO_BIT,
    (COLOUR_MONITOR==1)^((sound_control & BIT_0)!=0));
}


bool TMmu::sound_stop() { // called by sound_fetch()
  bool stopped=true;
  u_sound_frame_start.d32=u_next_sound_frame_start.d32;
  u_sound_frame_end.d32=u_next_sound_frame_end.d32;
  u_sound_fetch_address.d32=u_sound_frame_start.d32;
  sound_control&=~BIT_0;
  if(Mfp.reg[MFPR_TACR]==8)
  {
    mfp_timer_counter[0]-=64;
    if(mfp_timer_counter[0]<64)
    {
      mfp_timer_counter[0]=BYTE_00_TO_256(Mfp.reg[MFPR_TADR])*64;
      mfp_interrupt_pend(MFP_INT_TIMER_A,ABSOLUTE_CPU_TIME);
    }
#if defined(SSE_INT_MFP_EVENT_IRQ) // interrupts in event count mode
/*  Besides generating a count pulse, the active transition of the auxiliary
    input signal will also produce an interrupt on the I3 or I4 interrupt
    channel, if the interrupt channel is enabled.
*/
    mfp_interrupt_pend(4,time_of_next_timer_b);
#endif
  }
  mfp_gpip_set_bit(MFP_GPIP_MONO_BIT,
    (COLOUR_MONITOR!=0)^(sound_control&BIT_0)); // IRQ may trigger
  if(sound_control&BIT_1)
  {
    sound_control|=BIT_0; //Playing again immediately
    mfp_gpip_set_bit(MFP_GPIP_MONO_BIT,
      (COLOUR_MONITOR!=0)^(sound_control&BIT_0));
    stopped=false;
  }
  return stopped;
}

//#include <osd.h>
// static for STVL
void TMmu::sound_fetch() {
/*  "During horizontal blanking (transparent to the processor) the DMA sound
    chip fetches samples from memory and provides them to a digital-to-analog
    converter (DAC) at one of several constant rates, programmable as 
    (approximately) 50KHz (kilohertz), 25KHz, 12.5KHz, and 6.25KHz. 
    This rate is called the sample frequency. 
    Each sample is stored as a signed eight-bit quantity, where -128 (80 hex) 
    means full negative displacement of the speaker, and 127 (7F hex) means full 
    positive displacement. In stereo mode, each word represents two samples: the 
    upper byte is the sample for the left channel, and the lower byte is the 
    sample for the right channel. In mono mode each byte is one sample. However, 
    the samples are always fetched a word at a time, so only an even number of 
    mono samples can be played. "
    Approximations:
    Fetching should happen as soon as Video Enable stops, we do it at scanline.
*/
  //TRACE_OSD2("%x %d",shifter_sound_mode,ste_sound_freq);
  for(int i=0;i<4 && Mmu.u_sound_fetch_address.d32<himem
    && ((Mmu.sound_control&BIT_0)!=0 && Shifter.sound_fifo_idx<4);i++)
  {
    Shifter.sound_fifo[Shifter.sound_fifo_idx++]
    =SSEConfig.SteSoundOn ? DPEEK(Mmu.u_sound_fetch_address.d32) : 0;
    //TRACE_LOG("STE snd fetch %x\n",ste_sound_fetch_address);
    Mmu.u_sound_fetch_address.d32+=2;
    Mmu.u_sound_fetch_address.d32&=0x3FFFFE; //v402
    if(Mmu.u_sound_fetch_address.d32==Mmu.u_sound_frame_end.d32) // not >=: lazer insane
    {
      if(Mmu.sound_stop())
        break;
    }
    if(Shifter.sound_fifo_idx>=4) 
      break;
  }//nxt
#if defined(SSE_VID_STVL_SREQ)
  if(OPTION_C3 && SSEConfig.Stvl>=0x101)
    Stvl.sreq=(Shifter.sound_fifo_idx<4 && (Mmu.sound_control&BIT_0));
#endif
}


#if defined(SSE_SOUND_CARTRIDGE)
/*  To play the digital sound of the MV16 cartridge, we hack Steem's STE
    dma sound emulation, trading stereo 8bit for mono 16bit.
    This reduces overhead (need no other sound buffer).
    The sound is played as is, without filter (just like STE sound!).
    https://www.youtube.com/watch?v=2cYJGTL0QrE actual hardware
    https://www.youtube.com/watch?v=nxAHqYP9hAg crack - PSG
    This also handles the Replay 16 cartridge and the Centronics cartridge
    used by Wings of Death and Lethal Xcess.
*/

void mv16_fetch(WORD data) {
#define last_write Microwire.StartTime //recycle an int, starts at 0
  if(SSEConfig.mr16)
    data>>=1;
  int cycles=(ACT-last_write)&0xFFF; //fff for when last_write is 0!
  ste_sound_samples_countdown+=ste_sound_freq*cycles; // this implies jitter
  while(ste_sound_samples_countdown>/*=*/0)
  {
    ste_sound_output_countdown+=sound_freq;
    while(ste_sound_output_countdown>=0)
    {
      if(ste_sound_channel_buf_idx>=STE_SOUND_BUFFER_LENGTH)
        break;
      ste_sound_channel_buf[ste_sound_channel_buf_idx++]=ste_sound_last_word;
      ste_sound_output_countdown-=ste_sound_freq;
    }
    ste_sound_output_countdown+=sound_freq;
    ste_sound_samples_countdown-=n_cpu_cycles_per_second; 
  }
  last_write=ACT;
  ste_sound_on_this_screen=true;
  ste_sound_last_word=data;
}

#undef last_write 

#endif


////////////////
// SSE struct //
////////////////

void TMmu::Reset(bool Cold) {
  // the MMU has no reset pin so it's rather virtual here...
  ExtraBytesForHscroll=0;
  MonSTerHimem=0;
  vbase=Mmu.VideoCounter=shifter_draw_pointer=0;//v4 "reset:  all zeros"
  sound_control=0;
  ste_sound_start=0,next_ste_sound_start=0;
  ste_sound_end=0,next_ste_sound_end=0;
  ste_sound_fetch_address=ste_sound_start;
  if(Cold)
  {
    if(IS_STE)
      OPTION_WS=4; // STE is always in WS1
    else WIN_ONLY( if(OPTION_RANDOM_WU) )
      OPTION_WS=(rand()%4)+1; // choose a wakeup
    linewid=0; // schematics
    Restore();
    Glue.Update();
  }
}


void TMmu::Restore() {
  linewid0=ExtraBytesForHscroll=0;
  //     0  1  2  3  4  5
  // dl  0  3  4  5  6  3
  // ws  0  2  4  3  1  2
  //res  0  2  0  0 -2  2
  //sync 0  2  2  0  0  2
  ResMod[2]=ResMod[3]=FreqMod[3]=FreqMod[4]=0;
  WU[1]=WU[2]=WU[5]=WS[1]=WS[5]=ResMod[1]=ResMod[5]=FreqMod[1]=FreqMod[2]=FreqMod[5]=2;
  WU[3]=WU[4]=WS[4]=1;
  WS[2]=DL[2]=4;
  WS[3]=DL[1]=DL[5]=3;
  ResMod[4]=-2;
  DL[3]=5;
  DL[4]=6;
  if(!(OPTION_WS>=1 && OPTION_WS<=4))
    OPTION_WS=(rand()%4)+1;
  u_next_sound_frame_start.d8[B3]=u_next_sound_frame_end.d8[B3]=0; // 24bit
}


///////////
// VIDEO //
///////////

MEM_ADDRESS &vbase=Mmu.u_vbase.d32;
MEM_ADDRESS shifter_draw_pointer_at_start_of_line;

/*  Video Address Counter:

    STF - read only, writes ignored
    ff 8205   R               |xxxxxxxx|   Video Address Counter High
    ff 8207   R               |xxxxxxxx|   Video Address Counter Mid
    ff 8209   R               |xxxxxxxx|   Video Address Counter Low

    STE - read & write
    FF8204 ---- ---- --xx xxxx (High)
    FF8206 ---- ---- xxxx xxxx
    FF8208 ---- ---- xxxx xxx- (Low)
*/

#undef LOGSECTION
#define LOGSECTION LOGSECTION_VIDEO

void TMmu::UpdateVideoCounter(short CyclesIn) {
  MEM_ADDRESS vc;
  if(bad_drawing)
  {  // Fake SDP, eg extended monitor
    if(scan_y<0)
      vc=vbase;
    else if(scan_y<shifter_y)
    {
      int line_len=(160/res_vertical_scale);
      vc=vbase+scan_y*line_len+MIN(CyclesIn/2,line_len) & ~1;
    }
    else
      vc=vbase+32000;
  }
  else if(Glue.FetchingLine()) // lines where the counter actually moves
  {
    if(OPTION_C2)
      Glue.CheckSideOverscan(); // this updates Bytes and StartCycle
    int bytes_to_count=Glue.CurrentScanline.Bytes;
    // 8 cycles latency before MMU starts prefetching
    int starts_counting=(Glue.CurrentScanline.StartCycle+MMU_PREFETCH_LATENCY)/2;
    // can't be odd though (hires)
    starts_counting&=-2;
    // compute vc
    int c=CyclesIn/2-starts_counting;
    vc=shifter_draw_pointer_at_start_of_line;
    if(!bytes_to_count)
      ; // 0-byte lines
    else if(c>=bytes_to_count)
    {
      vc+=bytes_to_count;
      if(IS_STE && CyclesIn>=Glue.CurrentScanline.EndCycle&&!no_LW)
        vc+=LINEWID*2;
    }
    else if(c>=0)
    {
      c&=-2;
      vc+=c;
    }
  }
  else if(Glue.vsync)
    vc=vbase; // during VSYNC, VCOUNT=VBASE
  else // lines witout fetching (before or after frame)
    vc=shifter_draw_pointer_at_start_of_line;
#if defined(SSE_MMU_MONSTER_ALT_RAM)
  if(mem_len<14*0x100000) 
#else
  if(mem_len<=FOUR_MEGS) 
#endif
    vc&=0x3FFFFE;
  VideoCounter=vc; // update member variable
}


MEM_ADDRESS TMmu::ReadVideoCounter(short CyclesIn) {
  //ASSERT(!OPTION_C3);
  UpdateVideoCounter(CyclesIn);
  return VideoCounter;
}


/*  This function is only called by io_write() in device_map.cpp, if
    lle is not active.
    Now that video counter reckoning (Mmu.VideoCounter) is separated from 
    rendering (shifter_draw_pointer), a lot of cases that seemed complicated
    are simplified. Most hacks could be removed.

    Also, a simple test program allowed us to demystify writes to the video
    counter. It is actually straightforward, here's the rule:
    The byte in the MMU register is replaced with the byte on the bus, that's
    it, even if the counter is running at the time (Display Enable), and
    whatever words are in the Shifter.
    It's logical after all. The video counter resides in the MMU and the
    Shifter never sees it.
*/

void TMmu::WriteVideoCounter(MEM_ADDRESS addr,BYTE io_src_b) {
  short CyclesIn=(short)LINECYCLES;
  // some STF programs write to those addresses, it just must be ignored.
  if(IS_STF)
    return;
  // some bits will stay at 0 in the STE whatever you write
#if defined(SSE_MMU_MONSTER_ALT_RAM)
  if(addr==0xFF8205&&mem_len<14*0x100000)
#else
  if(mem_len<=FOUR_MEGS && addr==0xFF8205)
#endif
    io_src_b&=0x3F;
  else if(addr==0xFF8209)
    io_src_b&=0xFE;
  bool fl=Glue.FetchingLine();
  if(fl)
    Shifter.Render(CyclesIn,DISPATCHER_WRITE_SDP);
  UpdateVideoCounter(CyclesIn);
  const MEM_ADDRESS former_video_counter=VideoCounter;
  // it can be written but it would be overwritten at once during VSYNC
  if(!Glue.vsync) // change appropriate byte
    DWORD_B(&VideoCounter,(0xff8209-addr)/2)=io_src_b;
  // update shifter_draw_pointer_at_start_of_line
  shifter_draw_pointer_at_start_of_line-=former_video_counter;
  shifter_draw_pointer_at_start_of_line+=VideoCounter;
  // updating the video counter while video memory is still being fetched
  // could cause display artefacts because of shifter latency
  // a bit hacky, unlike with STVL
  // OK TalkTalk2 bees, Tekila, 20 Years STE Megademo/ex reset screen
  if(!fl
    ||CyclesIn<Glue.CurrentScanline.StartCycle+SHIFTER_RASTER_PREFETCH_TIMING
    || CyclesIn>Glue.CurrentScanline.EndCycle+SHIFTER_RASTER_PREFETCH_TIMING)
    shifter_draw_pointer=VideoCounter;
  // writing the counter inhibits LINEWID addition
  if(CyclesIn==Glue.CurrentScanline.EndCycle+6)
    no_LW=true;
}

#if defined(SSE_MEGASTE)
// Mega STE CPU naive implementation of the 16K cache 

TMemCache::TMemCache() {
  pAdlist=NULL;
  pIsCached=NULL;
}


TMemCache::~TMemCache() {
  if(pAdlist)
    free(pAdlist);
  if(pIsCached)
    free(pIsCached);
}  


void TMemCache::Add(MEM_ADDRESS ad) {
  if(ScuReg!=3 || ad<8 || ad>himem || pAdlist==NULL) // only ~16MHz and cache enabled
    return;
  ad>>=1;
  if(!pIsCached[ad])
  {
    pAdlist[iPos]=ad;
    iPos++;
    if(iPos==8*1024)
      iPos=0;
    pIsCached[ad]=true;
    pIsCached[pAdlist[iPos]]=false; // FIFO
  }
}


bool TMemCache::Check(MEM_ADDRESS ad,WORD &data) {
  if(ScuReg!=3 || ad<8 || ad>himem || pIsCached==NULL)
    return false;
  // We use a lookup table, a lookup function is too slow
  if(pIsCached[ad>>1])
  {
    data=DPEEK(ad); // in the emulator, we read the memory at no time cost
    return true;
  }
  return false;
}


void TMemCache::Reset() { // too slow for live emu, called at reset and model change
  if(!IS_MEGASTE)
    return;
  iPos=0;
  if(pAdlist)
    ZeroMemory(pAdlist,8*1024*sizeof(MEM_ADDRESS)); // 8K * address tag size
  if(pIsCached)
    ZeroMemory(pIsCached,1024*1024*4/2*sizeof(bool));
}


void TMemCache::Ready(bool enabling) {
  // enabled for Mega STE respectless of cache use, the goal here is not
  // to waste memory when using another model
  if(enabling && pAdlist==NULL) 
  {
    pAdlist=(MEM_ADDRESS*)calloc(8*1024,sizeof(MEM_ADDRESS));
    pIsCached=(bool*)calloc(1024*1024*4/2,sizeof(bool)); // 2MB
    Reset();
  }
  else if(!enabling && pAdlist)
  {
    free(pAdlist);
    pAdlist=NULL;
    free(pIsCached);
    pIsCached=NULL;
  }
}

#endif//#if defined(SSE_MEGASTE)
