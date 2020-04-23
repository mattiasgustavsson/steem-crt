//////////////////////////////////////////////////////////////////////////////
// aaArchiveCreator.h
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This is a base class for classes that create new archives
///////////////////////////////////////////////////////////////////////////////

#ifndef aaArchiveCreator__H
#define aaArchiveCreator__H

#include "Common/Defs.h"
#include "../CPP/7Zip/Archive/zip/ZipOut.h"
#include "../CPP/7Zip/UI/Common/Update.h"


#include "ArchiveAccess.h"
#include "aaCallbackOutStream.h"
#include "../CPP/Common/MyVector.h"

///////////////////////////////////////////////////////////////////////////////
// Class Declaration

class aaArchiveCreator: public aaBase {
public:
   // Create new archive using streams
   aaArchiveCreator (const TCHAR* ArchiveFileName, int ArchiveType, 
                     wchar_t* _password);

   // Destructor
   virtual ~aaArchiveCreator ();

   // Add file to new archive
   /*virtual int 
   addFile (FileInArchiveInfo* file, ReadCallback ReadFunc, int iStreamID) = 0;*/

   // Add file to archive, given its name
   void addFile (const TCHAR* FileName);

   // Add files as requested by addFile
   bool UpdateArchive();

   // Copy count bytes from input file into output stream
   /*bool CopyFile (ReadCallback ReadFunc, int iStreamID, 
                  UINT64 ReadStartOffset, UINT64 count);*/

   // Convert handle to class pointer
   static inline aaArchiveCreator* GetClassPointer (aaHandle handle)
   {
       return reinterpret_cast <aaArchiveCreator*> (handle);
   }
protected:
   // Pointer to 7-Zip archive handler
   IOutArchive* outArchive;
   CObjectVector<CDirItem> DirItems;
};

#endif