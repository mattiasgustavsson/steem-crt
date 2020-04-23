/*---------------------------------------------------------------------------
PROJECT: Steem SSE
DOMAIN: Rendering
FILE: dsp.h
DESCRIPTION: Declarations for audio filter classes used for STE Microwire
emulation.
---------------------------------------------------------------------------*/

#pragma once
#ifndef DSP_H
#define DSP_H

/*
Each time we need a filter, an object must be created. For example
for stereo we must create the same filter twice.
*/

extern double SampleRate;

class TIirFilter {
public:  
  TIirFilter();
  ~TIirFilter() ;
protected:
  double FilterAudio(double Input,double Frequency,double Q,double Gain,
    unsigned long Type);
  double i1, i2, o1, o2;  /* temporary variables */
  double a0, a1, a2;  /* coefficients */
  double b0, b1, b2;  /* coefficients */
  double f;         /* last Frequency used */
  double q;         /* last Q used */
  double g;         /* last Gain used */
  unsigned long t;  /* last Type used */

};

class TIirLowPass : TIirFilter {
public:
  double FilterAudio(double Input,double Frequency,double Gain);
};

class TIirLowShelf : TIirFilter {
public:
  double FilterAudio(double Input,double Frequency,double Gain);
};

class TIirHighShelf : TIirFilter {
public:
  double FilterAudio(double Input,double Frequency,double Gain);
};

class TIirVolume : TIirFilter {
public:
  double FilterAudio(double Input,double Gain);
};

#endif//#ifndef DSP_H