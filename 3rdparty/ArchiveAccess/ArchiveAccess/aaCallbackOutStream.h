///////////////////////////////////////////////////////////////////////////////
// aaCallbackOutStream.h
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This file declares a class for file extraction callbacks from 7-Zip's
// CHandler::extract calls
///////////////////////////////////////////////////////////////////////////////

/*
7-Zip's c layer's extract function will only extract a single for each call.
Therefore, the caller can prepare anything necessary to write data, including
opening streams and so on. Therefore, no special class for outStreams is
required; the call to GetStream simply returns this class, which in turn calls
the callback function.
*/

///////////////////////////////////////////////////////////////////////////////
// Includes

#ifndef aaCallbackOutStream__H
#define aaCallbackOutStream__H

#include "../CPP/7Zip/Archive/IArchive.h"
#include "../CPP/7Zip/IPassword.h"

#include "../CPP/Common/Defs.h"
#include "ArchiveAccess.h"

///////////////////////////////////////////////////////////////////////////////
// Callback stream declaration

class aaCallbackOutStream: public IArchiveExtractCallback, 
                           public IOutStream,
                           public ICryptoGetTextPassword
{
public:
    // Constructor, read data from existing stream
    aaCallbackOutStream (WriteCallback writeFunction, aaHandle writeStreamID,
        bool CloseStreamAfterDestruction = false);

    // Constructor, streams are requested using callbacks
    aaCallbackOutStream (aaHandle OwningArchiveHandle, 
        WriteCallback writeFunction, StreamRequest RequestStream);

    // Destructor
    ~aaCallbackOutStream ();

    // Methods required to implement IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( void);

    virtual ULONG STDMETHODCALLTYPE Release( void);

    STDMETHOD(GetStream)(UINT32 index, ISequentialOutStream **outStream, 
                         INT32 askExtractMode);
    STDMETHOD(PrepareOperation)(INT32 askExtractMode);
    STDMETHOD(SetOperationResult)(INT32 resultEOperationResult);

    // Methods to implement IProgress
    STDMETHOD(SetTotal)(UINT64 total) { return S_OK; }
    STDMETHOD(SetCompleted)(const UINT64 *completeValue) { return S_OK; }

    // Methods to implement IArchiveOpenVolumeCallback
    STDMETHOD(GetProperty)(PROPID propID, PROPVARIANT *value);
    STDMETHOD(GetStream)(const wchar_t *name, IInStream **inStream);

    // Methods needed to implement IOutStream
    STDMETHOD(Seek)(INT64 distanceToMove, UINT32 moveMethod, UINT64 *newPosition);
    STDMETHOD(SetSize)(UINT64 aNewSize);

    // Write size bytes to the stream
    STDMETHOD(Write)(const void *data, UINT32 size, UINT32 *processedSize);
    STDMETHOD(WritePart)(const void *data, UINT32 size, UINT32 *processedSize);

    inline UINT64 GetExtractedByteCount () 
	{ return m_ExtractedFileSize; }

    //virtual long __stdcall CryptoGetTextPassword(unsigned short ** );
    STDMETHOD(CryptoGetTextPassword)(BSTR *password);

    // Get current offset
    inline UINT64 GetOffset () 
	{ return m_offset; }
private:
    // Write function used for callback
    WriteCallback m_write;

    // Identifies the stream to the read callback
    aaHandle m_StreamID;

    // Write mode, see NAskMode in IArchive.h
    int m_mode;

    // Number of bytes extracted to file so far
    UINT64 m_ExtractedFileSize;

    // Callback for requesting streams
    StreamRequest m_callRequestStream;

    // Handle to owning archive, needed for RequestStream & getting passwords
    aaHandle m_ArchiveHandle;

    // Close windows file handle on destruction
	 bool m_CloseWindowsHandle;

    // Total file size and current offset
    INT64 m_FileSize, m_offset;
};

#endif
