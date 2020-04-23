/*---------------------------------------------------------------------------
PROJECT: Steem SSE
DOMAIN: Rendering
FILE: dsp.cpp
DESCRIPTION:  Based on C source by Maverick (Fabio Bizzetti) found at
http://membres.multimania.fr/amycoders/tutorials/maverick/sfftm.html
changed in handier C++ classes because it was screaming for object
handling instead of strange code duplication (don't understand what
he meant exactly!)
Used in Steem SSE for STE Microwire emulation. It does give bass and treble
but certainly not close to the STE.
---------------------------------------------------------------------------*/

#include "dsp.h"
#include <math.h>
#include <float.h>

double SampleRate=4096; // we keep this global for the moment (TODO)

#define Pi  3.141592653589793
#define Pi2 6.283185307179586


TIirFilter::TIirFilter() {
}

TIirFilter::~TIirFilter() {
 
}

double TIirFilter::FilterAudio(double Input,double Frequency,double Q,
                               double Gain,unsigned long Type) {
   double Output=Input,S,omega,A,sn,cs,alpha,beta,temp1,temp2,temp3,temp4;
/* -- check if frequency, Q, gain or type has changed.. and, if so, 
   update coefficients */
   if ( ( Frequency != f ) || ( Q != q ) || ( Gain != g ) || ( Type != t ) ) {
      f = Frequency; q = Q; g = Gain; t = Type; /* remember last frequency, q, gain and type */
      switch (Type) {
         case 0:                                               /* no filtering */
            b0 = pow( 10.0, Gain / 20.0 );               /* convert from dB to linear */
            break;
         case 1:                                               /* lowpass */
            Gain = pow( 10.0, Gain / 20.0 );                   /* convert from dB to linear */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            a0 = 1.0 / ( 1.0 + alpha );
            a1 = ( -2.0 * cs ) * a0;
            a2 = ( 1.0 - alpha ) * a0;
            b1 = ( 1.0 - cs ) * a0 * Gain;
            b0 = b1 * 0.5;
            break;
         case 2:                                               /* highpass */
            Gain = pow( 10.0, Gain / 20.0 );                   /* convert from dB to linear */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            a0 = 1.0 / ( 1.0 + alpha );
            a1 = ( -2.0 * cs ) * a0;
            a2 = ( 1.0 - alpha ) * a0;
            b1 = -( 1.0 + cs ) * a0 * Gain;
            b0 = -b1 * 0.5;
            break;
         case 3:                                               /* bandpass */
            Gain = pow( 10.0, Gain / 20.0 );                   /* convert from dB to linear */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            a0 = 1.0 / ( 1.0 + alpha );
            a1 = ( -2.0 * cs ) * a0;
            a2 = ( 1.0 - alpha ) * a0;
            b0 = alpha * a0 * Gain;
            break;
         case 4:                                               /* notch */
            Gain = pow( 10.0, Gain / 20.0 );                   /* convert from dB to linear */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            a0 = 1.0 / ( 1.0 + alpha );
            a1 = ( -2.0 * cs ) * a0;
            a2 = ( 1.0 - alpha ) * a0;
            b0 = a0 * Gain;
            b1 = a1 * Gain;
            break;
         case 5:                                               /* lowshelf */
            /* "shelf slope" 1.0 = max slope, because neither Q nor bandwidth is used in */
            /* those filters (note: true only for lowshelf and highshelf, not peaking). */
           
           S = 1.0; /* used only by lowshelf and highshelf */
            A = pow( 10.0 , ( Gain / 40.0 ) );                 /* Gain is expressed in dB */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            temp1 = A + 1.0; temp2 = A - 1.0; temp3 = temp1 * cs; temp4 = temp2 * cs;
            beta = sn * sqrt( ( A * A + 1.0 ) / S - temp2 * temp2 );
            a0 = 1.0 / ( temp1 + temp4 + beta );
            a1 = ( -2.0 * ( temp2 + temp3 ) ) * a0;
            a2 = ( temp1 + temp4 - beta ) * a0;
            b0 = ( A * ( temp1 - temp4 + beta ) ) * a0;
            b1 = ( 2.0 * A * ( temp2 - temp3 ) ) * a0;
            b2 = ( A * ( temp1 - temp4 - beta ) ) * a0;
            break;
         case 6:                                               /* highshelf */
            /* "shelf slope" 1.0 = max slope, because neither Q nor bandwidth is used in */
            /* those filters (note: true only for lowshelf and highshelf, not peaking). */
           S = 1.0; /* used only by lowshelf and highshelf */
            A = pow( 10.0, Gain / 40.0 ) ;                  /* Gain is expressed in dB */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            temp1 = A + 1.0; temp2 = A - 1.0; temp3 = temp1 * cs; temp4 = temp2 * cs;
            beta = sn * sqrt( ( A * A + 1.0 ) / S - temp2 * temp2 );
            a0 = 1.0 / ( temp1 - temp4 + beta );
            a1 = ( 2.0 * ( temp2 - temp3 ) ) * a0;
            a2 = ( temp1 - temp4 - beta ) * a0;
            b0 = ( A * ( temp1 + temp4 + beta ) ) * a0;
            b1 = ( -2.0 * A * ( temp2 + temp3 ) ) * a0;
            b2 = ( A * ( temp1 + temp4 - beta ) ) * a0;
           break;
         case 7:                                               /* peaking */
            A = pow( 10.0, ( Gain / 40.0 ) );                  /* Gain is expressed in dB */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            temp1 = alpha * A;
            temp2 = alpha / A;
            a0 = 1.0 / ( 1.0 + temp2 );
            a1 = ( -2.0 * cs ) * a0;
            a2 = ( 1.0 - temp2 ) * a0;
            b0 = ( 1.0 + temp1 ) * a0;
            b2 = ( 1.0 - temp1 ) * a0;
            break;
      }
   }
/* -- filter loop: if you don't change the parameters of the filter dynamically, ~only this code will be executed. */
    switch (Type) {
      case 0:                                                  /* no filtering */
         Output = b0*Input;
         break;
      case 1:                                                  /* lowpass */
      case 2:                                                  /* highpass */
         Output = b0*Input + b1*i1 + b0*i2 - a1*o1 - a2*o2;
         break;
      case 3:                                                  /* bandpass */
         Output = b0*Input - b0*i2 - a1*o1 - a2*o2;
         break;
      case 4:                                                  /* notch */
         Output = b0*Input + b1*i1 + b0*i2 - a1*o1 - a2*o2;
         break;
      case 5:                                                  /* low shelving */
      case 6://high=same
        Output = b0*Input + b1*i1 + b2*i2 - a1*o1 - a2*o2;
         break;
      case 7:                                                  /* peaking */
         Output = b0*Input + a1*i1 + b2*i2 - a1*o1 - a2*o2;
         break;
      default:
        break;
   }
   o2=o1; o1=Output; i2=i1; i1=Input; /* update variables for recursion */
   return(Output);
}


double TIirLowPass::FilterAudio(double Input,double Frequency,double Gain) {
  return TIirFilter::FilterAudio(Input,Frequency,1,Gain,1);
}


double TIirHighShelf::FilterAudio(double Input,double Frequency,double Gain) {
//    double rv=TIirFilter::FilterAudio(Input,Frequency,50,Gain,6);
  double rv=TIirFilter::FilterAudio(Input,Frequency,3,Gain,6);
  return rv;
  //return Input; //...
}


double TIirLowShelf::FilterAudio(double Input,double Frequency,double Gain) {
  return TIirFilter::FilterAudio(Input,Frequency,0,Gain,5);
}


double old_gain=0;

double TIirVolume::FilterAudio(double Input,double Gain) {
  //if(Gain<-20) Gain=-20;
  //if(Gain>12) Gain=12;
  /*
  if(Gain-old_gain>1)
    Gain=old_gain+1, old_gain++;
  else if(Gain-old_gain<-1)
    Gain=old_gain-1, old_gain--;
    */
  return TIirFilter::FilterAudio(Input,0,0,Gain,0);
}




#if 0 // doc & original source in C

#include <math.h>


double SampleRate;               /* set it to e.g. 44100.0 (Hz) */

#define Pi  3.141592653589793    /* constant */
#define Pi2 6.283185307179586    /* constant */

//#define maxfilterunits 2//10        /* set it to the max number of filter units you may use (10 units = use units 0..9) */

/*

Second order IIR filters: 

 Finally, I'm gonna show you the C source of my own second order filter unit.
 Consider it as a flexible filter cell, like e.g. a single unit of a parametric
 equalizer. Of course you can use as many as you wish, either in serie or in
 parallel, to build very complex filter networks. The parameters are the actual
 input sample, the frequency at which the filter has to work 
 (resonance or cut-off, depends by the filter type in use), the Q (if you wanna
 work with bandwidth instead, you will have to calculate the Q like 
 Q=FilterFrequency/BandWidth), the gain of the filter, and the type of filter. 
 The last parameter allows you to change the type of filter without any "click" 
 in the sound output (something that would happen if you instead changed the 
 routine you call each sample), because IIR filters use feedback, and thus have
 to remember some of the previous sample values. Note, the coefficients of 
 the filter will be calculated only if you change the parameters of above.. 
 so if you make no dynamic use of the filter, it will be faster. Anyway, here's 
 my code (note: thanks a lot to Robert Bristow-Johnson for the coefficients 
 to make these great IIR filters, and for all the help he gave me fixing a 
 stupid bug I made when implementing them in a hurry). There's only one 
 function, and it looks like: 

Output = FilterCell(Unit, Input, Frequency, Q, Gain, Type);

Output is the output sample you're gonna get from the filter (and feed e.g. 
to a DAC). 

Unit is the filter unit you are referring to (like in an array). It's a kind
 of object-oriented programming, not using C++ classes though. For a single 
 filter, you would always use only unit 0. But if you want to use more than 
 one filter, then you would have to duplicate the source a number of times 
 necessary to have all the filter units you need (remember: the filters are 
 recursive, so they need to remember some old samples values), or I could 
 have provided 10+ additional parameters (pointers to temporary variables 
 for the past samples, coefficients, etc..) or a pointer to a struct, but 
 my solution has been the simplest and most comfortable one. You have to 
 #define maxfilterunits as the number of units you wanna use. 1 is the 
 minimum, of course (meaning you use only unit 0). 

Input is the input sample you're currently feeding to the IIR filter. For 
example it may come directly from an ADC. 

Frequency is the frequency at which the filter has to work. Depending on the 
type of filter you're using, the meaning of this parameter changes. For 
lowpass/highpass filters it's the cutoff frequency, for low-shelving/high-
shelving it's the center of the "transition band", for bandpass/notch/peaking
 filters it's the resonance frequency. 

Q is the selectivity of the filter, and is equal to the Frequency/Bandwidth. 
The Bandwidth is defined as the distance (in Hz) between the two points with 
3dB of difference with the gain at the frequency at which the filter is set 
to operate (bandpass case); or the distance between the two points at 
dB-Gain/2 (peaking case). The higher the Q, the more selective is the filter. 
Note that high values for Q will slightly change the behaviour of 
lowpass/highpass filters, making them have a superior to flat gain before 
the cut-off frequency. A lowpass filter with high Q becomes a kind of hybrid 
between a bandpass and a lowpass. Use the provided code to get (and the graph)
 the magnitude response of the filters, to get a better idea of how all these
 parameters affect the filtering [to be added]. 

Gain is the gain of the filter. For lowpass/highpass/bandpass and notch it's 
an overall gain (like an amplifier cascaded to the filter), while for 
low-shelving, high-shelving and peaking filters it's the gain in the region 
where the filter is operating. 

Type selects the kind of filter you want to use. 0 means no filtering, 
1 means lowpass, 2 means highpass, 3 means bandpass, 4 means notch, 
5 means low-shelving, 6 means high-shelving, 7 means peaking, and 
8 and above means crash. :D 
*/




double FilterCellA(unsigned long Unit, double Input, double Frequency, double Q, double Gain, unsigned long Type) {
   static double i1[maxfilterunits], i2[maxfilterunits], o1[maxfilterunits], o2[maxfilterunits];  /* temporary variables */
   static double a0[maxfilterunits], a1[maxfilterunits], a2[maxfilterunits];  /* coefficients */
   static double b0[maxfilterunits], b1[maxfilterunits], b2[maxfilterunits];  /* coefficients */
   static double f[maxfilterunits];         /* last Frequency used */
   static double q[maxfilterunits];         /* last Q used */
   static double g[maxfilterunits];         /* last Gain used */
   static unsigned long t[maxfilterunits];  /* last Type used */
/* --------------------------------------------------------------- */
   double Output,S,omega,A,sn,cs,alpha,beta,temp1,temp2,temp3,temp4;
/* -- check if frequency, Q, gain or type has changed.. and, if so, update coefficients */
   if ( ( Frequency != f[Unit] ) || ( Q != q[Unit] ) || ( Gain != g[Unit] ) || ( Type != t[Unit] ) ) {
      f[Unit] = Frequency; q[Unit] = Q; g[Unit] = Gain; t[Unit] = Type; /* remember last frequency, q, gain and type */
      switch (Type) {
         case 0:                                               /* no filtering */
            b0[Unit] = pow( 10.0, Gain / 20.0 );               /* convert from dB to linear */
            break;
         case 1:                                               /* lowpass */
            Gain = pow( 10.0, Gain / 20.0 );                   /* convert from dB to linear */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            a0[Unit] = 1.0 / ( 1.0 + alpha );
            a1[Unit] = ( -2.0 * cs ) * a0[Unit];
            a2[Unit] = ( 1.0 - alpha ) * a0[Unit];
            b1[Unit] = ( 1.0 - cs ) * a0[Unit] * Gain;
            b0[Unit] = b1[Unit] * 0.5;
            break;
         case 2:                                               /* highpass */
            Gain = pow( 10.0, Gain / 20.0 );                   /* convert from dB to linear */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            a0[Unit] = 1.0 / ( 1.0 + alpha );
            a1[Unit] = ( -2.0 * cs ) * a0[Unit];
            a2[Unit] = ( 1.0 - alpha ) * a0[Unit];
            b1[Unit] = -( 1.0 + cs ) * a0[Unit] * Gain;
            b0[Unit] = -b1[Unit] * 0.5;
            break;
         case 3:                                               /* bandpass */
            Gain = pow( 10.0, Gain / 20.0 );                   /* convert from dB to linear */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            a0[Unit] = 1.0 / ( 1.0 + alpha );
            a1[Unit] = ( -2.0 * cs ) * a0[Unit];
            a2[Unit] = ( 1.0 - alpha ) * a0[Unit];
            b0[Unit] = alpha * a0[Unit] * Gain;
            break;
         case 4:                                               /* notch */
            Gain = pow( 10.0, Gain / 20.0 );                   /* convert from dB to linear */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            a0[Unit] = 1.0 / ( 1.0 + alpha );
            a1[Unit] = ( -2.0 * cs ) * a0[Unit];
            a2[Unit] = ( 1.0 - alpha ) * a0[Unit];
            b0[Unit] = a0[Unit] * Gain;
            b1[Unit] = a1[Unit] * Gain;
            break;
         case 5:                                               /* lowshelf */
            /* "shelf slope" 1.0 = max slope, because neither Q nor bandwidth is used in */
            /* those filters (note: true only for lowshelf and highshelf, not peaking). */
            S = 1.0; /* used only by lowshelf and highshelf */
            A = pow( 10.0 , ( Gain / 40.0 ) );                 /* Gain is expressed in dB */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            temp1 = A + 1.0; temp2 = A - 1.0; temp3 = temp1 * cs; temp4 = temp2 * cs;
            beta = sn * sqrt( ( A * A + 1.0 ) / S - temp2 * temp2 );
            a0[Unit] = 1.0 / ( temp1 + temp4 + beta );
            a1[Unit] = ( -2.0 * ( temp2 + temp3 ) ) * a0[Unit];
            a2[Unit] = ( temp1 + temp4 - beta ) * a0[Unit];
            b0[Unit] = ( A * ( temp1 - temp4 + beta ) ) * a0[Unit];
            b1[Unit] = ( 2.0 * A * ( temp2 - temp3 ) ) * a0[Unit];
            b2[Unit] = ( A * ( temp1 - temp4 - beta ) ) * a0[Unit];
            break;
         case 6:                                               /* highshelf */
            /* "shelf slope" 1.0 = max slope, because neither Q nor bandwidth is used in */
            /* those filters (note: true only for lowshelf and highshelf, not peaking). */
            S = 1.0; /* used only by lowshelf and highshelf */
            A = pow( 10.0, ( Gain / 40.0 ) );                  /* Gain is expressed in dB */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            temp1 = A + 1.0; temp2 = A - 1.0; temp3 = temp1 * cs; temp4 = temp2 * cs;
            beta = sn * sqrt( ( A * A + 1.0 ) / S - temp2 * temp2 );
            a0[Unit] = 1.0 / ( temp1 - temp4 + beta );
            a1[Unit] = ( 2.0 * ( temp2 - temp3 ) ) * a0[Unit];
            a2[Unit] = ( temp1 - temp4 - beta ) * a0[Unit];
            b0[Unit] = ( A * ( temp1 + temp4 + beta ) ) * a0[Unit];
            b1[Unit] = ( -2.0 * A * ( temp2 + temp3 ) ) * a0[Unit];
            b2[Unit] = ( A * ( temp1 + temp4 - beta ) ) * a0[Unit];
            break;
         case 7:                                               /* peaking */
            A = pow( 10.0, ( Gain / 40.0 ) );                  /* Gain is expressed in dB */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            temp1 = alpha * A;
            temp2 = alpha / A;
            a0[Unit] = 1.0 / ( 1.0 + temp2 );
            a1[Unit] = ( -2.0 * cs ) * a0[Unit];
            a2[Unit] = ( 1.0 - temp2 ) * a0[Unit];
            b0[Unit] = ( 1.0 + temp1 ) * a0[Unit];
            b2[Unit] = ( 1.0 - temp1 ) * a0[Unit];
            break;
      }
   }
/* -- filter loop: if you don't change the parameters of the filter dynamically, ~only this code will be executed. */
   switch (Type) {
      case 0:                                                  /* no filtering */
         Output = b0[Unit]*Input;
         break;
      case 1:                                                  /* lowpass */
      case 2:                                                  /* highpass */
         Output = b0[Unit]*Input + b1[Unit]*i1[Unit] + b0[Unit]*i2[Unit] - a1[Unit]*o1[Unit] - a2[Unit]*o2[Unit];
         break;
      case 3:                                                  /* bandpass */
         Output = b0[Unit]*Input - b0[Unit]*i2[Unit] - a1[Unit]*o1[Unit] - a2[Unit]*o2[Unit];
         break;
      case 4:                                                  /* notch */
         Output = b0[Unit]*Input + b1[Unit]*i1[Unit] + b0[Unit]*i2[Unit] - a1[Unit]*o1[Unit] - a2[Unit]*o2[Unit];
         break;
      case 5:                                                  /* low shelving */
      case 6:                                                  /* high shelving */
         Output = b0[Unit]*Input + b1[Unit]*i1[Unit] + b2[Unit]*i2[Unit] - a1[Unit]*o1[Unit] - a2[Unit]*o2[Unit];
         break;
      case 7:                                                  /* peaking */
         Output = b0[Unit]*Input + a1[Unit]*i1[Unit] + b2[Unit]*i2[Unit] - a1[Unit]*o1[Unit] - a2[Unit]*o2[Unit];
         break;
   }
   o2[Unit]=o1[Unit]; o1[Unit]=Output; i2[Unit]=i1[Unit]; i1[Unit]=Input; /* update variables for recursion */
   return(Output);
}



double FilterCellB(unsigned long Unit, double Input, double Frequency, double Q, double Gain, unsigned long Type) {
   static double i1[maxfilterunits], i2[maxfilterunits], o1[maxfilterunits], o2[maxfilterunits];  /* temporary variables */
   static double a0[maxfilterunits], a1[maxfilterunits], a2[maxfilterunits];  /* coefficients */
   static double b0[maxfilterunits], b1[maxfilterunits], b2[maxfilterunits];  /* coefficients */
   static double f[maxfilterunits];         /* last Frequency used */
   static double q[maxfilterunits];         /* last Q used */
   static double g[maxfilterunits];         /* last Gain used */
   static unsigned long t[maxfilterunits];  /* last Type used */
/* --------------------------------------------------------------- */
   double Output,S,omega,A,sn,cs,alpha,beta,temp1,temp2,temp3,temp4;
/* -- check if frequency, Q, gain or type has changed.. and, if so, update coefficients */
   if ( ( Frequency != f[Unit] ) || ( Q != q[Unit] ) || ( Gain != g[Unit] ) || ( Type != t[Unit] ) ) {
      f[Unit] = Frequency; q[Unit] = Q; g[Unit] = Gain; t[Unit] = Type; /* remember last frequency, q, gain and type */
      switch (Type) {
         case 0:                                               /* no filtering */
            b0[Unit] = pow( 10.0, Gain / 20.0 );               /* convert from dB to linear */
            break;
         case 1:                                               /* lowpass */
            Gain = pow( 10.0, Gain / 20.0 );                   /* convert from dB to linear */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            a0[Unit] = 1.0 / ( 1.0 + alpha );
            a1[Unit] = ( -2.0 * cs ) * a0[Unit];
            a2[Unit] = ( 1.0 - alpha ) * a0[Unit];
            b1[Unit] = ( 1.0 - cs ) * a0[Unit] * Gain;
            b0[Unit] = b1[Unit] * 0.5;
            break;
         case 2:                                               /* highpass */
            Gain = pow( 10.0, Gain / 20.0 );                   /* convert from dB to linear */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            a0[Unit] = 1.0 / ( 1.0 + alpha );
            a1[Unit] = ( -2.0 * cs ) * a0[Unit];
            a2[Unit] = ( 1.0 - alpha ) * a0[Unit];
            b1[Unit] = -( 1.0 + cs ) * a0[Unit] * Gain;
            b0[Unit] = -b1[Unit] * 0.5;
            break;
         case 3:                                               /* bandpass */
            Gain = pow( 10.0, Gain / 20.0 );                   /* convert from dB to linear */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            a0[Unit] = 1.0 / ( 1.0 + alpha );
            a1[Unit] = ( -2.0 * cs ) * a0[Unit];
            a2[Unit] = ( 1.0 - alpha ) * a0[Unit];
            b0[Unit] = alpha * a0[Unit] * Gain;
            break;
         case 4:                                               /* notch */
            Gain = pow( 10.0, Gain / 20.0 );                   /* convert from dB to linear */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            a0[Unit] = 1.0 / ( 1.0 + alpha );
            a1[Unit] = ( -2.0 * cs ) * a0[Unit];
            a2[Unit] = ( 1.0 - alpha ) * a0[Unit];
            b0[Unit] = a0[Unit] * Gain;
            b1[Unit] = a1[Unit] * Gain;
            break;
         case 5:                                               /* lowshelf */
            /* "shelf slope" 1.0 = max slope, because neither Q nor bandwidth is used in */
            /* those filters (note: true only for lowshelf and highshelf, not peaking). */
            S = 1.0; /* used only by lowshelf and highshelf */
            A = pow( 10.0 , ( Gain / 40.0 ) );                 /* Gain is expressed in dB */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            temp1 = A + 1.0; temp2 = A - 1.0; temp3 = temp1 * cs; temp4 = temp2 * cs;
            beta = sn * sqrt( ( A * A + 1.0 ) / S - temp2 * temp2 );
            a0[Unit] = 1.0 / ( temp1 + temp4 + beta );
            a1[Unit] = ( -2.0 * ( temp2 + temp3 ) ) * a0[Unit];
            a2[Unit] = ( temp1 + temp4 - beta ) * a0[Unit];
            b0[Unit] = ( A * ( temp1 - temp4 + beta ) ) * a0[Unit];
            b1[Unit] = ( 2.0 * A * ( temp2 - temp3 ) ) * a0[Unit];
            b2[Unit] = ( A * ( temp1 - temp4 - beta ) ) * a0[Unit];
            break;
         case 6:                                               /* highshelf */
            /* "shelf slope" 1.0 = max slope, because neither Q nor bandwidth is used in */
            /* those filters (note: true only for lowshelf and highshelf, not peaking). */
            S = 1.0; /* used only by lowshelf and highshelf */
            A = pow( 10.0, ( Gain / 40.0 ) );                  /* Gain is expressed in dB */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            temp1 = A + 1.0; temp2 = A - 1.0; temp3 = temp1 * cs; temp4 = temp2 * cs;
            beta = sn * sqrt( ( A * A + 1.0 ) / S - temp2 * temp2 );
            a0[Unit] = 1.0 / ( temp1 - temp4 + beta );
            a1[Unit] = ( 2.0 * ( temp2 - temp3 ) ) * a0[Unit];
            a2[Unit] = ( temp1 - temp4 - beta ) * a0[Unit];
            b0[Unit] = ( A * ( temp1 + temp4 + beta ) ) * a0[Unit];
            b1[Unit] = ( -2.0 * A * ( temp2 + temp3 ) ) * a0[Unit];
            b2[Unit] = ( A * ( temp1 + temp4 - beta ) ) * a0[Unit];
            break;
         case 7:                                               /* peaking */
            A = pow( 10.0, ( Gain / 40.0 ) );                  /* Gain is expressed in dB */
            omega = ( Pi2 * Frequency ) / SampleRate;
            sn = sin( omega ); cs = cos( omega );
            alpha = sn / ( 2.0 * Q );
            temp1 = alpha * A;
            temp2 = alpha / A;
            a0[Unit] = 1.0 / ( 1.0 + temp2 );
            a1[Unit] = ( -2.0 * cs ) * a0[Unit];
            a2[Unit] = ( 1.0 - temp2 ) * a0[Unit];
            b0[Unit] = ( 1.0 + temp1 ) * a0[Unit];
            b2[Unit] = ( 1.0 - temp1 ) * a0[Unit];
            break;
      }
   }
/* -- filter loop: if you don't change the parameters of the filter dynamically, ~only this code will be executed. */
   switch (Type) {
      case 0:                                                  /* no filtering */
         Output = b0[Unit]*Input;
         break;
      case 1:                                                  /* lowpass */
      case 2:                                                  /* highpass */
         Output = b0[Unit]*Input + b1[Unit]*i1[Unit] + b0[Unit]*i2[Unit] - a1[Unit]*o1[Unit] - a2[Unit]*o2[Unit];
         break;
      case 3:                                                  /* bandpass */
         Output = b0[Unit]*Input - b0[Unit]*i2[Unit] - a1[Unit]*o1[Unit] - a2[Unit]*o2[Unit];
         break;
      case 4:                                                  /* notch */
         Output = b0[Unit]*Input + b1[Unit]*i1[Unit] + b0[Unit]*i2[Unit] - a1[Unit]*o1[Unit] - a2[Unit]*o2[Unit];
         break;
      case 5:                                                  /* low shelving */
      case 6:                                                  /* high shelving */
         Output = b0[Unit]*Input + b1[Unit]*i1[Unit] + b2[Unit]*i2[Unit] - a1[Unit]*o1[Unit] - a2[Unit]*o2[Unit];
         break;
      case 7:                                                  /* peaking */
         Output = b0[Unit]*Input + a1[Unit]*i1[Unit] + b2[Unit]*i2[Unit] - a1[Unit]*o1[Unit] - a2[Unit]*o2[Unit];
         break;
   }
   o2[Unit]=o1[Unit]; o1[Unit]=Output; i2[Unit]=i1[Unit]; i1[Unit]=Input; /* update variables for recursion */
   return(Output);
}
#endif
