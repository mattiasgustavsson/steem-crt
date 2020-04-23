///////////////////////////////////////////////////////////////////////////////
// AACallbackInStream.h
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This file declares an InStream using callback functions
///////////////////////////////////////////////////////////////////////////////

/*

Data is accessed using the interfaces ISequentialInStream, IInStream, 
ISequentialOutStream, and IOutStream, defined in IStream.h.

Zip archives require seekable input/output, therefore a subclass of IInStream 
that obtains data using callback functions is needed. Write access would 
require an equivalent subclass of IOutStream.

*/

///////////////////////////////////////////////////////////////////////////////
// Includes

#ifndef AACallbackInStream__H
#define AACallbackInStream__H

#include "ArchiveAccess.h"
#include "../CPP/7Zip/IStream.h"

///////////////////////////////////////////////////////////////////////////////
// Callback stream declaration

class AACallbackInStream: IInStream
{
public:
	// Constructor
#if 0 //SS
	AACallbackInStream (ReadCallback readFunction, aaHandle readStreamID,
		                 INT64 FileSize, bool CloseStreamAfterDestruction = true);
#else
	AACallbackInStream (ReadCallback readFunction, aaHandle readStreamID,
		                 INT64 FileSize, bool CloseStreamAfterDestruction = false);
#endif
	// Destructor
	~AACallbackInStream ();

	// Methods required to implement IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( void);

    virtual ULONG STDMETHODCALLTYPE Release( void);

	// Read size bytes into data, and return number of actually read bytes into
	// processedSize
    STDMETHOD(Read)(void *data, UINT32 size, UINT32 *processedSize);
    STDMETHOD(ReadPart)(void *data, UINT32 size, UINT32 *processedSize);

	// Seek 
    STDMETHOD(Seek)(INT64 offset, UINT32 seekOrigin, UINT64 *newPosition);

private:
	// Read function used for callback
	ReadCallback m_read;

	// Identifies the stream to the read callback
	aaHandle m_StreamID;

	// Current position in datastream for sequential access
	INT64 m_offset, m_FileSize;

   // Close windows handle when stream is closed?
	bool m_CloseWindowsHandle;
};

#endif