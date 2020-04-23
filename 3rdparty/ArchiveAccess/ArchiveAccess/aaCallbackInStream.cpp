///////////////////////////////////////////////////////////////////////////////
// AACallbackInStream.h
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This file declares an InStream using callback functions
///////////////////////////////////////////////////////////////////////////////

#include "windows.h"

#include "aaCallbackInStream.h"

#include <limits>
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Constructor

AACallbackInStream::AACallbackInStream (ReadCallback readFunction, 
    aaHandle ReadStreamID, INT64 FileSize, bool CloseStreamAfterDestruction)
{
  //OutputDebugString(L"AACallbackInStream\n");
  //if(!CloseStreamAfterDestruction)
    //OutputDebugString(L"No need to close!\n");
	m_read = readFunction;
	m_StreamID = ReadStreamID;
	m_offset = 0;
	m_FileSize = FileSize;
#if 0 //SS - but then it doesn't work...
  OutputDebugString(L"Force m_CloseWindowsHandle!\n");
  m_CloseWindowsHandle = true;
#else
	m_CloseWindowsHandle = CloseStreamAfterDestruction;
#endif
}


///////////////////////////////////////////////////////////////////////////////
// Destructor

AACallbackInStream::~AACallbackInStream ()
{
  //OutputDebugString(L"~AACallbackInStream\n");
#if 0 //SS - but then it doesn't work...
  // the Handle is better closed by the calling program (Steem)
  if (true) {
#else
	if (m_CloseWindowsHandle) {
#endif
		CloseHandle (reinterpret_cast <HANDLE> (m_StreamID));
	}
}

///////////////////////////////////////////////////////////////////////////////
// Methods inherited from ISequentialInStream

STDMETHODIMP AACallbackInStream::Read (void *data, UINT32 size, 
									 UINT32 *pProcessedSize)
{
  //OutputDebugString(L"Read\n");
   UINT32 processedSize;
   HRESULT result = m_read (m_StreamID, m_offset, size, data, &processedSize); 
   m_offset += processedSize;
   if (pProcessedSize != NULL)
      *pProcessedSize = processedSize;

   // Returning S_OK has been tested with all archive types and works.
   // Returning result here does not work with gz archives
   return S_OK;   // true 
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP AACallbackInStream::ReadPart (void *data, UINT32 size, 
										 UINT32 *processedSize)
{
	return Read(data, size, processedSize);
}

///////////////////////////////////////////////////////////////////////////////
// Method inherited from IInStream

STDMETHODIMP AACallbackInStream::Seek (INT64 distanceToMove, UINT32 moveMethod,
									 UINT64 *newPosition)
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

///////////////////////////////////////////////////////////////////////////////
// Methods required to implement IUnknown

HRESULT STDMETHODCALLTYPE AACallbackInStream::QueryInterface( 
   /* [in] */ REFIID riid,
   /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	return 0;
}

ULONG STDMETHODCALLTYPE AACallbackInStream::AddRef( void)
{
	return 0;
}


ULONG STDMETHODCALLTYPE AACallbackInStream::Release( void)
{
	return 0;
}

