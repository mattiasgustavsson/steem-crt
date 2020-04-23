/*---------------------------------------------------------------------------
PROJECT: Steem SSE
DOMAIN: Rendering
FILE: AviFile.cpp
CONDITION: SSE_VID_RECORD_AVI must be defined
DESCRIPTION: Using CAviFile by P.Gopalakrishna.
Modified for use by Steem SSE. Addition of sound stream.
---------------------------------------------------------------------------*/

//#include "StdAfx.h" // no MFC for Steem

#include <SSE.h>

#if defined(SSE_VID_RECORD_AVI)
#include <windows.h>
#pragma comment(lib, "vfw32.lib")  //  OK for VC, BCC
#include <dsound.h>
#include <debug.h> // TRACE, ASSERT

#define _T(X) X
#define _tcscpy strcpy
#define _tcsncpy strncpy
extern IDirectSoundBuffer *PrimaryBuf,*SoundBuf; // VC9 doesn't know that
unsigned int FormatAviMessage(HRESULT code, char *buf,unsigned int len);
extern BYTE video_freq_at_start_of_vbl;

#include "avifile.h"

#ifndef __countof
#define __countof(x)  ((sizeof(x)/sizeof(x[0])))
#endif

#define LOGSECTION LOGSECTION_VIDEO//SS

#ifdef VC_BUILD
//#pragma warning (disable : 4996)
#endif

#if defined(SSE_VID_RECORD_AVI) && defined(__cplusplus)
#define _T(X) X
#define _tcscpy strcpy
#define _tcsncpy strncpy
CAviFile *pAviFile;
BYTE video_recording;
char *video_recording_codec;
#endif


CAviFile:: CAviFile(LPCSTR lpszFileName /* =_T("Output.avi") */, 
      DWORD dwCodec /* = mmioFOURCC('M','P','G','4') */,
      DWORD dwFrameRate /* = 1 */)
{

  AVIFileInit();

  m_hHeap=NULL;
  m_hAviDC=NULL;
  m_lpBits=NULL;
  m_lSample=NULL;
  m_pAviFile=NULL;
  m_pAviStream=NULL;
  m_pAviCompressedStream=NULL;

  m_dwFCCHandler = dwCodec;
  m_dwFrameRate = dwFrameRate;

  _tcscpy(m_szFileName, lpszFileName);
  _tcscpy(m_szErrMsg, _T("Method Succeeded"));
  m_szErrMsg[__countof(m_szErrMsg)-1] = _T('\0');

  pAppendFrame[0]= &CAviFile::AppendDummy;      // VC8 requires & for Function Pointer; Remove it if your compiler complains;
  pAppendFrame[1]= &CAviFile::AppendFrameFirstTime;
  pAppendFrame[2]= &CAviFile::AppendFrameUsual;

  pAppendFrameBits[0]=&CAviFile::AppendDummy;
  pAppendFrameBits[1]=&CAviFile::AppendFrameFirstTime;
  pAppendFrameBits[2]=&CAviFile::AppendFrameUsual;

  m_nAppendFuncSelector=1;    //0=Dummy 1=FirstTime 2=Usual

#if defined(SSE_VID_RECORD_AVI)
  m_lSampleA=0; // A for audio
  m_pAviStreamA=NULL;
  Initialised=0; // our usual trick to avoid crashes
#endif
}

CAviFile::~CAviFile(void)
{
  ReleaseMemory();

  AVIFileExit();
}

void CAviFile::ReleaseMemory()
{
  m_nAppendFuncSelector=0;    //Point to DummyFunction

  if(m_hAviDC)
  {
    DeleteDC(m_hAviDC);
    m_hAviDC=NULL;
  }
  if(m_pAviCompressedStream)
  {
    AVIStreamRelease(m_pAviCompressedStream);
    m_pAviCompressedStream=NULL;
  }
  if(m_pAviStream)
  {
    AVIStreamRelease(m_pAviStream);
    m_pAviStream=NULL;
  }

#if defined(SSE_VID_RECORD_AVI)
  if(m_pAviStreamA)
  {
    AVIStreamRelease(m_pAviStreamA);
    m_pAviStreamA=NULL;
  }
#endif

  if(m_pAviFile)
  {
    AVIFileRelease(m_pAviFile);
    m_pAviFile=NULL;
  }
  if(m_lpBits)
  {
    HeapFree(m_hHeap,HEAP_NO_SERIALIZE,m_lpBits);
    m_lpBits=NULL;
  }
  if(m_hHeap)
  {
    HeapDestroy(m_hHeap);
    m_hHeap=NULL;
  }
}

void CAviFile::SetErrorMessage(LPCTSTR lpszErrorMessage)
{
  _tcsncpy(m_szErrMsg, lpszErrorMessage, __countof(m_szErrMsg)-1);
#if defined(SSE_VID_RECORD_AVI)
  TRACE_LOG("%s\n",m_szErrMsg);
#endif
}
  
HRESULT CAviFile::InitMovieCreation(int nFrameWidth, int nFrameHeight, WORD nBitsPerPixel)
{
  int nMaxWidth=GetSystemMetrics(SM_CXSCREEN),nMaxHeight=GetSystemMetrics(SM_CYSCREEN);
  
  m_hAviDC = CreateCompatibleDC(NULL);
  if(m_hAviDC==NULL)  
  {
    SetErrorMessage("Unable to Create Compatible DC");
    return E_FAIL;
  }
  
  if(nFrameWidth > nMaxWidth) nMaxWidth= nFrameWidth;
  if(nFrameHeight > nMaxHeight) nMaxHeight = nFrameHeight;
  
  m_hHeap=HeapCreate(HEAP_NO_SERIALIZE, nMaxWidth*nMaxHeight*4, 0);
  if(m_hHeap==NULL)
  {
    SetErrorMessage("Unable to Create Heap");
    return E_FAIL;
  }
  
  m_lpBits=HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY|HEAP_NO_SERIALIZE, nMaxWidth*nMaxHeight*4);
  if(m_lpBits==NULL)  
  { 
    SetErrorMessage("Unable to Allocate Memory on Heap");
    return E_FAIL;
  }
#if defined(SSE_VID_RECORD_AVI)
  DeleteFile(m_szFileName);
#endif
  if(FAILED(AVIFileOpen(&m_pAviFile, m_szFileName, OF_CREATE|OF_WRITE, NULL)))
  {
    SetErrorMessage("Unable to Create the Movie File");
    return E_FAIL;
  }
  
  ZeroMemory(&m_AviStreamInfo,sizeof(AVISTREAMINFO));
  m_AviStreamInfo.fccType   = streamtypeVIDEO;
  m_AviStreamInfo.fccHandler  = m_dwFCCHandler;
  m_AviStreamInfo.dwScale   = 1;
  m_AviStreamInfo.dwRate    = m_dwFrameRate;  // Frames Per Second;
  m_AviStreamInfo.dwQuality = (DWORD)-1;        // Default Quality
  m_AviStreamInfo.dwSuggestedBufferSize = nMaxWidth*nMaxHeight*4;
  SetRect(&m_AviStreamInfo.rcFrame, 0, 0, nFrameWidth, nFrameHeight);
  _tcscpy(m_AviStreamInfo.szName, _T("Video Stream"));
  
  if(FAILED(AVIFileCreateStream(m_pAviFile,&m_pAviStream,&m_AviStreamInfo)))
  {
    SetErrorMessage("Unable to Create Video Stream in the Movie File");
    return E_FAIL;
  }

  ZeroMemory(&m_AviCompressOptions,sizeof(AVICOMPRESSOPTIONS));
  m_AviCompressOptions.fccType=streamtypeVIDEO;
  m_AviCompressOptions.fccHandler=m_AviStreamInfo.fccHandler;
#if defined(SSE_VID_RECORD_AVI)
  m_AviCompressOptions.dwFlags=AVICOMPRESSF_KEYFRAMES|AVICOMPRESSF_VALID|AVICOMPRESSF_DATARATE;
#else
  m_AviCompressOptions.dwFlags=AVICOMPRESSF_KEYFRAMES|AVICOMPRESSF_VALID; //|AVICOMPRESSF_DATARATE;
#endif
  m_AviCompressOptions.dwKeyFrameEvery=1;
  //m_AviCompressOptions.dwBytesPerSecond=1000/8;
  //m_AviCompressOptions.dwQuality=100;

#if defined(SSE_VID_RECORD_AVI)
  if(FAILED(AVIMakeCompressedStream(&m_pAviCompressedStream,m_pAviStream,&m_AviCompressOptions,NULL)))
  {
    TRACE_LOG("Fall back to MSVC codec\n");
    m_AviCompressOptions.fccHandler=mmioFOURCC('M','S','V','C');
  }
#endif

  if(FAILED(AVIMakeCompressedStream(&m_pAviCompressedStream,m_pAviStream,&m_AviCompressOptions,NULL)))
  {
    // One reason this error might occur is if you are using a Codec that is not 
    // available on your system. Check the mmioFOURCC() code you are using and make
    // sure you have that codec installed properly on your machine.
    SetErrorMessage("Unable to Create Compressed Stream: Check your CODEC options");
    return E_FAIL;
  }
  
  BITMAPINFO bmpInfo;
  ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
  bmpInfo.bmiHeader.biPlanes    = 1;
  bmpInfo.bmiHeader.biWidth   = nFrameWidth;
  bmpInfo.bmiHeader.biHeight    = nFrameHeight;
  bmpInfo.bmiHeader.biCompression = BI_RGB;
  bmpInfo.bmiHeader.biBitCount  = nBitsPerPixel;
  bmpInfo.bmiHeader.biSize    = sizeof(BITMAPINFOHEADER);
  bmpInfo.bmiHeader.biSizeImage = bmpInfo.bmiHeader.biWidth*bmpInfo.bmiHeader.biHeight*bmpInfo.bmiHeader.biBitCount/8;
  
  if(FAILED(AVIStreamSetFormat(m_pAviCompressedStream,0,(LPVOID)&bmpInfo, bmpInfo.bmiHeader.biSize)))
  {
    // One reason this error might occur is if your bitmap does not meet the Codec requirements.
    // For example, 
    //   your bitmap is 32bpp while the Codec supports only 16 or 24 bpp; Or
    //   your bitmap is 274 * 258 size, while the Codec supports only sizes that are powers of 2; etc...
    // Possible solution to avoid this is: make your bitmap suit the codec requirements,
    // or Choose a codec that is suitable for your bitmap.
    SetErrorMessage("Unable to Set Video Stream Format");
    return E_FAIL;
  }
  
#if defined(SSE_VID_RECORD_AVI)
  // Add audio stream if there's one (uncompressed)
  if(SoundBuf)
  {
    // we take the WAVEFORMATEX currently used in DirectSound
    DWORD dwSizeAllocated=0, dwSizeWritten; 
    SoundBuf->GetFormat(NULL,dwSizeAllocated,&dwSizeWritten);
    //ASSERT( dwSizeWritten==sizeof(WAVEFORMATEX) );
    SoundBuf->GetFormat(&wfx,dwSizeWritten,&dwSizeWritten);
    //ASSERT( wfx.nChannels );
    ZeroMemory(&m_AviStreamInfoA,sizeof(AVISTREAMINFO));
    m_AviStreamInfoA.fccType    = streamtypeAUDIO;
    m_AviStreamInfoA.fccHandler = m_dwFCCHandler;
    m_AviStreamInfoA.dwScale    =  wfx.nBlockAlign;;
    m_AviStreamInfoA.dwRate   = wfx.nSamplesPerSec*wfx.nBlockAlign; 
    m_AviStreamInfoA.dwSampleSize = wfx.nBlockAlign;
    
    _tcscpy(m_AviStreamInfoA.szName, _T("Audio Stream"));
    
    if(FAILED(AVIFileCreateStream(m_pAviFile,&m_pAviStreamA,&m_AviStreamInfoA)))
    {
      SetErrorMessage("Unable to Create Audio Stream in the Movie File");
      return E_FAIL;
    }

    if(FAILED(AVIStreamSetFormat(m_pAviStreamA,0,&wfx,sizeof(WAVEFORMATEX))))
    {
      SetErrorMessage("Failed to set format for audio");
      return E_FAIL;
    }
  }

  Initialised=true;
#endif

  return S_OK;  // Everything went Fine
}

HRESULT CAviFile::AppendFrameFirstTime(HBITMAP  hBitmap)
{
  BITMAP Bitmap;

  GetObject(hBitmap, sizeof(BITMAP), &Bitmap);

  if(SUCCEEDED(InitMovieCreation( Bitmap.bmWidth, 
                  Bitmap.bmHeight, 
                  Bitmap.bmBitsPixel)))
  {
    m_nAppendFuncSelector=2;    //Point to the UsualAppend Function

    return AppendFrameUsual(hBitmap);
  }

  ReleaseMemory();

  return E_FAIL;
}

HRESULT CAviFile::AppendFrameUsual(HBITMAP hBitmap)
{
  BITMAPINFO  bmpInfo;

  bmpInfo.bmiHeader.biBitCount=0;
  bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
  
  GetDIBits(m_hAviDC,hBitmap,0,0,NULL,&bmpInfo,DIB_RGB_COLORS);

  bmpInfo.bmiHeader.biCompression=BI_RGB; 

  GetDIBits(m_hAviDC,hBitmap,0,bmpInfo.bmiHeader.biHeight,m_lpBits,&bmpInfo,DIB_RGB_COLORS);

  if(FAILED(AVIStreamWrite(m_pAviCompressedStream,m_lSample++,1,m_lpBits,bmpInfo.bmiHeader.biSizeImage,0,NULL,NULL)))
  {
    SetErrorMessage(_T("Unable to Write Video Stream to the output Movie File"));

    ReleaseMemory();

    return E_FAIL;
  }

    return S_OK;
}

HRESULT CAviFile::AppendDummy(HBITMAP)
{
  return E_FAIL;
}

HRESULT CAviFile::AppendNewFrame(HBITMAP hBitmap)
{
  return (this->*pAppendFrame[m_nAppendFuncSelector])((HBITMAP)hBitmap);
}

HRESULT CAviFile::AppendNewFrame(int nWidth, int nHeight, LPVOID pBits,WORD nBitsPerPixel)
{
  return (this->*pAppendFrameBits[m_nAppendFuncSelector])(nWidth,nHeight,pBits,nBitsPerPixel);
}

HRESULT CAviFile::AppendFrameFirstTime(int nWidth, int nHeight, LPVOID pBits,WORD nBitsPerPixel)
{
  if(SUCCEEDED(InitMovieCreation(nWidth, nHeight, nBitsPerPixel)))
  {
    m_nAppendFuncSelector=2;    //Point to the UsualAppend Function

    return AppendFrameUsual(nWidth, nHeight, pBits, nBitsPerPixel);
  }

  ReleaseMemory();

  return E_FAIL;
}

HRESULT CAviFile::AppendFrameUsual(int nWidth, int nHeight, LPVOID pBits,WORD nBitsPerPixel)
{
  DWORD dwSize=nWidth*nHeight*nBitsPerPixel/8;

  if(FAILED(AVIStreamWrite(m_pAviCompressedStream,m_lSample++,1,pBits,dwSize,0,NULL,NULL)))
  {
    SetErrorMessage(_T("Unable to Write Video Stream to the output Movie File"));

    ReleaseMemory();

    return E_FAIL;
  }

    return S_OK;
}

HRESULT CAviFile::AppendDummy(int , int , LPVOID ,WORD )
{
  return E_FAIL;
}


#if defined(SSE_VID_RECORD_AVI)

HRESULT CAviFile::AppendSound(LPVOID pAudBuf,DWORD numbytes) {
  unsigned long numsamps = numbytes*8 / wfx.wBitsPerSample;
  //ASSERT( numsamps*wfx.wBitsPerSample/8==numbytes );

  // very poor trick to avoid slow sound when encoding is too demanding
  if(video_freq_at_start_of_vbl
    && numbytes>wfx.nAvgBytesPerSec/video_freq_at_start_of_vbl)
    numbytes=wfx.nAvgBytesPerSec/video_freq_at_start_of_vbl;

  if(FAILED(AVIStreamWrite(m_pAviStreamA,m_lSampleA,numsamps,pAudBuf,
    numbytes,0,NULL,NULL) ))
  {
    SetErrorMessage(_T("Unable to Write Sound Stream to the output Movie File"));
    //    ReleaseMemory();
    return E_FAIL;
  }
  m_lSampleA+=numsamps;
  return S_OK;
}

#undef _tcscpy
#undef _tcsncpy
#endif

#endif //defined(SSE_VID_RECORD_AVI)
