///////////////////////////////////////////////////////////////////////////////
// ArchiveAccessClass.h
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This file declares a class for accessing archives
///////////////////////////////////////////////////////////////////////////////

#ifndef ArchiveAccessClass__H
#define ArchiveAccessClass__H

#include "aaCallbackInStream.h"
#include "windows.h"
#include "../CPP/7Zip/IPassword.h"
#include "../CPP/7Zip/Archive/IArchive.h"

///////////////////////////////////////////////////////////////////////////////
// ArchiveAccessClass declaration

class ArchiveAccessClass: public aaBase {
   public:
      // ArchiveAccessClass constructor: Access file using blockwise transfer, 
      // for reading from stdin or writing to stdout
      // Returns an integer handle to the file (casted class pointer)
      // 0 indicates failure
      // ReadFunction is a callback to a read function
      // StreamID is used to identify this data stream to ReadFunction
      ArchiveAccessClass (ReadCallback ReadFunction, aaHandle StreamHandle, 
                          INT64 _FileSize, int ArchiveType, int* result,
                          bool CloseStreamAfterDestruction = false,
                          wchar_t* _password = NULL);

      // Destructor
      virtual ~ArchiveAccessClass ();

      // Get number of files in archive
      int getFileCount ();

      // Get information on a file in the archive
      int getFileInfo (unsigned intFileNum, FileInArchiveInfo* FileInfo);

      // Extract a file from an archive to memory
      int extractFile (unsigned int FileNum, aaHandle oStreamHandle, 
                       WriteCallback WriteFunc, UINT64* WrittenSize = NULL);

      // Extract multiple files from an archive
      void extractFiles (int FileCount, unsigned int FileNums [], 
                        WriteCallback WriteFunc, StreamRequest RequestStream);

      // Convert handle to class pointer
      static inline ArchiveAccessClass* GetClassPointer (aaHandle handle)
      {
          aaBase* p = reinterpret_cast <aaBase*> (handle);
          ArchiveAccessClass* p2 = dynamic_cast <ArchiveAccessClass*> (p);
          return p2;
      }

      // Set/get the owner of this object
      // Useful for determining the owner of an archive handle
      inline void setOwner (int newOwner) { owner = newOwner; }
      inline int  getOwner () { return owner; }

      // Get password
      inline const wchar_t* getPassword () const { return password; }
   private:
      // Class used for access to archive
      IInArchive* archive;

      // Streams
      AACallbackInStream inStream; 

      wchar_t* password;

      // The owner of this object
      int owner;

      // Runtime-type information
      int magic;
};

#endif
