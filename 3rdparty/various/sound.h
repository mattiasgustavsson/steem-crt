#pragma once
#ifndef SOUND_H
#define SOUND_H

struct TWavFileFormat { // from ?
  char riff[4];		    // 4 bytes
  long filesize;	    // 4 bytes
  char wave[4];		    // 4 bytes
  char fmt[4];		    // 4 bytes
  long chunkSize;       // 4 bytes
  short wFormatTag;     // 2 bytes
  short nChannels;      // 2 bytes
  long nSamplesPerSec;  // 4 bytes
  long nAvgBytesPerSec; // 4 bytes
  short nBlockAlign;    // 2 bytes
  short wBitsPerSample; // 2 bytes
  char data[4];		    // 4 bytes
  long length;			// 4 bytes
} ;

#endif//#ifndef SOUND_H
