///////////////////////////////////////////////////////////////////////////////
// aaCallbackOutStream.h
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This file implements a class for file extraction callbacks from 7-Zip's
// CHandler::extract calls
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "aaCallbackOutStream.h"
#include "ArchiveAccessClass.h"
#include "bgstr.h"

///////////////////////////////////////////////////////////////////////////////
// Constructor

aaCallbackOutStream::aaCallbackOutStream (WriteCallback writeFunction, 
   aaHandle writeStreamID, bool CloseStreamAfterDestruction)
: m_write (writeFunction),
  m_StreamID (writeStreamID),
  m_mode (NArchive::NExtract::NAskMode::kExtract),
  m_ExtractedFileSize (0),
  m_callRequestStream (NULL),
  m_ArchiveHandle (0),
  m_CloseWindowsHandle (CloseStreamAfterDestruction),
  m_FileSize (0),
  m_offset (0)
{
}

///////////////////////////////////////////////////////////////////////////////
// Constructor, streams are requested using callbacks

aaCallbackOutStream::aaCallbackOutStream (aaHandle OwningArchiveHandle, 
    WriteCallback writeFunction, StreamRequest RequestStream)
: m_write (writeFunction),
  m_StreamID (0),
  m_mode (NArchive::NExtract::NAskMode::kExtract),
  m_ExtractedFileSize (0),
  m_callRequestStream (RequestStream),
  m_ArchiveHandle (OwningArchiveHandle),
  m_CloseWindowsHandle (0),
  m_FileSize (0),
  m_offset (0)
{
}

///////////////////////////////////////////////////////////////////////////////
// Destructor

aaCallbackOutStream::~aaCallbackOutStream ()
{
	if (m_CloseWindowsHandle) {
		CloseHandle (reinterpret_cast <HANDLE> (m_StreamID));
	}
}

///////////////////////////////////////////////////////////////////////////////
// Methods required to implement IUnknown

HRESULT STDMETHODCALLTYPE aaCallbackOutStream::QueryInterface( 
   /* [in] */ REFIID riid,
   /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
   (*ppvObject) = this;
    return S_OK;
}

ULONG STDMETHODCALLTYPE aaCallbackOutStream::AddRef( void)
{
    return S_OK;
}

ULONG STDMETHODCALLTYPE aaCallbackOutStream::Release( void)
{
    return S_OK;
}

HRESULT aaCallbackOutStream::GetStream(UINT32 index, 
	ISequentialOutStream **outStream, INT32 askExtractMode)
{
    if (m_callRequestStream != NULL 
    &&  m_mode == NArchive::NExtract::NAskMode::kExtract)
    {
        m_StreamID = m_callRequestStream (m_ArchiveHandle, (aaHandle) index, OpenStream);
        m_offset = 0;
    }
    (*outStream) = this;
    m_mode = askExtractMode;
    return S_OK;
}

HRESULT aaCallbackOutStream::PrepareOperation (INT32 askExtractMode)
{
    return S_OK;
}

HRESULT aaCallbackOutStream::SetOperationResult(INT32 resultEOperationResult)
{
    if (m_callRequestStream != NULL)
        m_callRequestStream (m_ArchiveHandle, m_StreamID, CloseStream);
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Write size bytes to the stream

STDMETHODIMP aaCallbackOutStream::Write (const void *data, UINT32 size, 
                                         UINT32 *processedSize)
{
    if (m_mode == NArchive::NExtract::NAskMode::kExtract) {
        m_ExtractedFileSize += (UINT64) size;
        UINT32 written = 0;
        int result = m_write (m_StreamID, m_offset, size, data, &written); 
        m_offset += written;
        if (processedSize != NULL)
           (*processedSize) = written; 
        return result; 
	} else {
        if (processedSize != NULL)
           (*processedSize) = size; 
        return S_OK;
	}
}

STDMETHODIMP aaCallbackOutStream::WritePart (const void *data, UINT32 size, 
                                             UINT32 *processedSize)
{
	UINT32 written = 0;
	int result = m_write (m_StreamID, m_offset, size, data, &written); 
	m_offset += written;
	if (processedSize != NULL)
		(*processedSize) = written; 
	return result; 
}

///////////////////////////////////////////////////////////////////////////////

long __stdcall aaCallbackOutStream::CryptoGetTextPassword(BSTR *pwd)
{
    ArchiveAccessClass* aac = 
        ArchiveAccessClass::GetClassPointer (m_ArchiveHandle);
    if (aac != NULL) {
       //(*pwd) = aac->password;
       int len = bgstrlen (aac->getPassword (), 1024) + 1;
       (*pwd) = new wchar_t [len];
       bgstrlcpy (*pwd, aac->getPassword (), len);
    } else
        pwd = NULL;
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Methods to implement IArchiveOpenVolumeCallback

// todo: needed for multi-volume archives?

HRESULT STDMETHODCALLTYPE aaCallbackOutStream::GetProperty (PROPID propID, PROPVARIANT *value)
{
   return S_OK;
}

HRESULT STDMETHODCALLTYPE aaCallbackOutStream::GetStream (const wchar_t *name, IInStream **inStream)
{
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Methods needed to implement IOutStream

HRESULT STDMETHODCALLTYPE aaCallbackOutStream::Seek (
   INT64 distanceToMove, UINT32 moveMethod, UINT64 *newPosition)
{
	if (moveMethod == FILE_BEGIN) 
		m_offset = distanceToMove;
	else if (moveMethod == FILE_CURRENT)
		m_offset += distanceToMove;
	else if (moveMethod == FILE_END)
		m_offset = m_FileSize + distanceToMove;
	else
	   return S_FALSE;
  
	if (newPosition != NULL)
		*newPosition = m_offset;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE aaCallbackOutStream::SetSize (UINT64 aNewSize)
{
   m_FileSize = aNewSize;
   return S_OK;
}
