/*---------------------------------------------------------------------------
PROJECT: Steem SSE
DOMAIN: Disk image
FILE: CapsPlug.cpp
DESCRIPTION: Extended version of capsplug.cpp by SPS
Used in Windows and in Linux
---------------------------------------------------------------------------*/

// Simple caps plug-in support on Win32, should fit most users
//
// You may want to link with the libray instead, if your product fully complies
// with the license.
// If using the library directly without the plug-in, define CAPS_USER, and include CapsLib.h
// functions are the same with the plug-in, but their name start with "CAPS", not "Caps"
// CAPSInit does not have any parameters, CapsInit gets the library name
//
// Comment out stdafx.h if your project does not use the MSVC precompiled headers
//
// www.caps-project.org

//#include "stdafx.h" 

#include <SSE.h>
#include <debug.h> // for our debug facilities
#include <acc.h>

#ifdef WIN32
#include "ComType.h"
#include "CapsAPI.h"
#include "CapsPlug.h"
HMODULE capi=NULL;

#if defined(BCC_BUILD)
#pragma warn- 8061 
#endif

CapsProc cpr[]= {
  "CAPSInit", NULL,
  "CAPSExit", NULL,
  "CAPSAddImage", NULL,
  "CAPSRemImage", NULL,
  "CAPSLockImage", NULL,
  "CAPSUnlockImage", NULL,
  "CAPSLoadImage", NULL,
  "CAPSGetImageInfo", NULL,
  "CAPSLockTrack", NULL,
  "CAPSUnlockTrack", NULL,
  "CAPSUnlockAllTracks", NULL,
  "CAPSGetPlatformName", NULL,
  "CAPSLockImageMemory", NULL,
  // SSE
  "CAPSGetVersionInfo",NULL,
  "CAPSFdcGetInfo", NULL,
  "CAPSFdcInit", NULL,
  "CAPSFdcReset", NULL,
  "CAPSFdcEmulate", NULL,
  "CAPSFdcRead", NULL,
  "CAPSFdcWrite", NULL,
  "CAPSFdcInvalidateTrack", NULL,
  "CAPSGetInfo",NULL,
  "CAPSSetRevolution",NULL,
  NULL, NULL
};

#if defined(BCC_BUILD)
#pragma warn+ 8061 
#endif

// SSE
enum {ECapsGetVersionInfo=13,ECapsFdcGetInfo,ECapsFdcInit,ECapsFdcReset,ECapsFdcEmulate,
ECapsFdcRead,ECapsFdcWrite,ECapsFdcInvalidateTrack,ECapsGetInfo,
ECapsSetRevolution
};

#endif//WIN32


#ifdef UNIX
#define __cdecl
#include  <caps_linux/capsimg_source_linux_macosx/Core/CommonTypes.h>
#include <caps_linux/capsimg_source_linux_macosx/LibIPF/CapsAPI.h>
#include <caps_linux/capsimg_source_linux_macosx/LibIPF/CapsFDC.h>
#include <caps_linux/capsimg_source_linux_macosx/LibIPF/CapsLib.h>
#undef __cdecl
#include "CapsPlug.h"
#endif

// start caps image support
SDWORD CapsInit(WIN_ONLY(LPCTSTR lib))
{
#ifdef WIN32
  TRACE("Init CAPS library %s\n",lib);
  if (capi)
    return imgeOk;

  capi=SteemLoadLibrary(lib);
  if (!capi)
  {
  //  TRACE("Failed\n");
    return imgeUnsupported;
  }
//  TRACE("OK\n");

  for (int prc=0; cpr[prc].name; prc++)
  {
    cpr[prc].proc=GetProcAddress(capi, cpr[prc].name);
  //  TRACE("init fct %s %p\n",cpr[prc].name,cpr[prc].proc);
  }
  SDWORD res=cpr[0].proc ? CAPSHOOKN(cpr[0].proc)() : imgeUnsupported;
#endif
#ifdef UNIX
  SDWORD res=CAPSInit();
#endif  
  return res;
}

// stop caps image support
SDWORD CapsExit()
{
#ifdef WIN32
  SDWORD res=cpr[1].proc ? CAPSHOOKN(cpr[1].proc)() : imgeUnsupported;

  if (capi) {
    //TRACE_INIT("Releasing CAPS library...\n");
    FreeLibrary(capi);
    capi=NULL;
  }

  for (int prc=0; cpr[prc].name; prc++)
    cpr[prc].proc=NULL;
#endif
#ifdef UNIX
  SDWORD res=CAPSExit();
#endif
  return res;
}

// add image container
SDWORD CapsAddImage()
{
#ifdef WIN32  
  SDWORD res=cpr[2].proc ? CAPSHOOKN(cpr[2].proc)() : -1;
#endif
#ifdef UNIX
  SDWORD res=CAPSAddImage();
#endif
  return res;
}

// delete image container
SDWORD CapsRemImage(SDWORD id)
{
#ifdef WIN32  
  SDWORD res=cpr[3].proc ? CAPSHOOKN(cpr[3].proc)(id) : -1;
#endif  
#ifdef UNIX
  SDWORD res=CAPSRemImage(id);
#endif
  return res;
}

// lock image
SDWORD CapsLockImage(SDWORD id, PCHAR name)
{
#ifdef WIN32  
  SDWORD res=cpr[4].proc ? CAPSHOOKN(cpr[4].proc)(id, name) : imgeUnsupported;
#endif  
#ifdef UNIX
  SDWORD res=CAPSLockImage(id,name);
#endif
  return res;
}

// unlock image
SDWORD CapsUnlockImage(SDWORD id)
{
#ifdef WIN32  
  SDWORD res=cpr[5].proc ? CAPSHOOKN(cpr[5].proc)(id) : imgeUnsupported;
#endif  
#ifdef UNIX
  SDWORD res=CAPSUnlockImage(id);
#endif
  return res;
}

// load and decode complete image
SDWORD CapsLoadImage(SDWORD id, UDWORD flag)
{
#ifdef WIN32  
  SDWORD res=cpr[6].proc ? CAPSHOOKN(cpr[6].proc)(id, flag) : imgeUnsupported;
#endif  
#ifdef UNIX
  SDWORD res=CAPSLoadImage(id,flag);
#endif
  return res;
}

// get image information
SDWORD CapsGetImageInfo(PCAPSIMAGEINFO pi, SDWORD id)
{
#ifdef WIN32  
  SDWORD res=cpr[7].proc ? CAPSHOOKN(cpr[7].proc)(pi, id) : imgeUnsupported;
#endif  
#ifdef UNIX
  SDWORD res=CAPSGetImageInfo(pi,id);
#endif
  return res;
}

// load and decode track, or return with the cache
SDWORD CapsLockTrack(PCAPSTRACKINFO pi, SDWORD id, UDWORD cylinder, UDWORD head, UDWORD flag)
{
#ifdef WIN32  
  SDWORD res=cpr[8].proc ? CAPSHOOKN(cpr[8].proc)(pi, id, cylinder, head, flag) : imgeUnsupported;
#endif  
#ifdef UNIX
  SDWORD res=CAPSLockTrack(pi,id,cylinder, head, flag);
#endif
  return res;
}

// remove track from cache
SDWORD CapsUnlockTrack(SDWORD id, UDWORD cylinder, UDWORD head)
{
#ifdef WIN32  
  SDWORD res=cpr[9].proc ? CAPSHOOKN(cpr[9].proc)(id, cylinder, head) : imgeUnsupported;
#endif  
#ifdef UNIX
  SDWORD res=CAPSUnlockTrack(id, cylinder, head);
#endif
  return res;
}

// remove all tracks from cache
SDWORD CapsUnlockAllTracks(SDWORD id)
{
#ifdef WIN32  
  SDWORD res=cpr[10].proc ? CAPSHOOKN(cpr[10].proc)(id) : imgeUnsupported;
#endif
#ifdef UNIX
  SDWORD res=CAPSUnlockAllTracks(id);
#endif
  return res;
}

// get platform name
PCHAR CapsGetPlatformName(UDWORD pid)
{
#ifdef WIN32  
  PCHAR res=cpr[11].proc ? CAPSHOOKS(cpr[11].proc)(pid) : NULL;
#endif
#ifdef UNIX
  PCHAR res=CAPSGetPlatformName(pid);
#endif
  return res;
}

// lock memory mapped image
SDWORD CapsLockImageMemory(SDWORD id, PUBYTE buffer, UDWORD length, UDWORD flag)
{
#ifdef WIN32  
  SDWORD res=cpr[12].proc ? CAPSHOOKN(cpr[12].proc)(id, buffer, length, flag) : imgeUnsupported;
#endif
#ifdef UNIX
  SDWORD res=CAPSLockImageMemory(id, buffer, length, flag);
#endif
  return res;
}

// SSE
// library version
SDWORD CapsGetVersionInfo(PVOID pversioninfo, UDWORD flag) {
#ifdef WIN32  
  UDWORD res=cpr[ECapsGetVersionInfo].proc ? 
    CAPSHOOKN(cpr[ECapsGetVersionInfo].proc)(pversioninfo,flag) : imgeUnsupported;
#endif
#ifdef UNIX
  UDWORD res=CAPSGetVersionInfo(pversioninfo,flag);
#endif
  return res;
}

// get WD1772 status
UDWORD CapsFdcGetInfo(SDWORD iid, PCAPSFDC pc, SDWORD ext) {
#ifdef WIN32  
  UDWORD res=cpr[ECapsFdcGetInfo].proc ? 
    CAPSHOOKN(cpr[ECapsFdcGetInfo].proc)(iid, pc, ext) : imgeUnsupported;
#endif
#ifdef UNIX
  UDWORD res=CAPSFdcGetInfo(iid, pc, ext);
#endif
  return res;
}

// init WD1772
SDWORD CapsFdcInit(PCAPSFDC pc) {
#ifdef WIN32  
  SDWORD res=cpr[ECapsFdcInit].proc ? 
    CAPSHOOKN(cpr[ECapsFdcInit].proc)(pc) : imgeUnsupported;
#endif
#ifdef UNIX
  SDWORD res=CAPSFdcInit(pc);
#endif
  return res;
}

// reset WD1772
void CapsFdcReset(PCAPSFDC pc) {
#ifdef WIN32  
  if(cpr[ECapsFdcReset].proc)
    CAPSHOOKN(cpr[ECapsFdcReset].proc)(pc);
#endif    
#ifdef UNIX
  CAPSFdcReset(pc);
#endif
}

// run WD1772 cycles
void CapsFdcEmulate(PCAPSFDC pc, UDWORD cyclecnt) {
#ifdef WIN32  
  if(cpr[ECapsFdcEmulate].proc)
    CAPSHOOKN(cpr[ECapsFdcEmulate].proc)(pc,cyclecnt);
#endif    
#ifdef UNIX
  CAPSFdcEmulate(pc,cyclecnt);
#endif
}

// read WD1772 register
UDWORD CapsFdcRead(PCAPSFDC pc, UDWORD address) {
#ifdef WIN32  
  UDWORD res=cpr[ECapsFdcRead].proc ? 
    CAPSHOOKN(cpr[ECapsFdcRead].proc)(pc,address) : imgeUnsupported;
#endif
#ifdef UNIX
  UDWORD res=CAPSFdcRead(pc,address);
#endif
  return res;
}

// write WD1772 register
void CapsFdcWrite(PCAPSFDC pc, UDWORD address, UDWORD data) {
#ifdef WIN32  
  if(cpr[ECapsFdcWrite].proc)
    CAPSHOOKN(cpr[ECapsFdcWrite].proc)(pc,address,data);
#endif    
#ifdef UNIX
  CAPSFdcWrite(pc,address,data);
#endif
}

// cause the plugin to reload the track
SDWORD CapsFdcInvalidateTrack(PCAPSFDC pc, SDWORD drive) {
#ifdef WIN32  
  SDWORD res=cpr[ECapsFdcInvalidateTrack].proc ? 
    CAPSHOOKN(cpr[ECapsFdcInvalidateTrack].proc)(pc,drive) : imgeUnsupported;
#endif
#ifdef UNIX
  SDWORD res=CAPSFdcInvalidateTrack(pc,drive);
#endif
  return res;
}

// get track info
SDWORD CapsGetInfo(PVOID pinfo, SDWORD id, UDWORD cylinder, UDWORD head, 
                   UDWORD inftype, UDWORD infid) {
#ifdef WIN32                     
  SDWORD res=cpr[ECapsGetInfo].proc ? 
    CAPSHOOKN(cpr[ECapsGetInfo].proc)(pinfo,id,cylinder,head,inftype,infid) :
    imgeUnsupported;
#endif  
#ifdef UNIX
  SDWORD res=CAPSGetInfo(pinfo,id,cylinder,head,inftype,infid);
#endif
  return res;
}


// choose rev of disk image
SDWORD CapsSetRevolution(SDWORD id, UDWORD value) {
#ifdef WIN32  
  SDWORD res=cpr[ECapsSetRevolution].proc ? 
    CAPSHOOKN(cpr[ECapsSetRevolution].proc)(id,value) :
    imgeUnsupported;
#endif
#ifdef UNIX
  SDWORD res=CAPSSetRevolution(id,value);
#endif
  return res;
}
