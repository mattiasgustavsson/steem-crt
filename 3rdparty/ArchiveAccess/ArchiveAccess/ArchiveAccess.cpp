///////////////////////////////////////////////////////////////////////////////
// ArchiveAccess.cpp
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This file implements some access functions to file archives
///////////////////////////////////////////////////////////////////////////////

/*

7-Zip is a library that contains read/write access functions for dlls.
It is implemented in C++, so there is no easy way to use these functions
in Delphi. The purpose of ArchiveAccess is to provide access to 7-zip's
classes using plain C. These functions can be used easily in Delphi.

*/

/*
VS 2008 for SSE
don't ignore default libraries
link against dll
removed linker dependency:
libcmt.lib
*/

///////////////////////////////////////////////////////////////////////////////
// Includes

#include <windows.h>
#include "ArchiveAccess.h"
#include "ArchiveAccessClass.h"
#include "aaArchiveCreator.h"
//#include "aaTarArchiveCreator.h"
//#include "aaZipArchiveCreator.h"
#include "bgstr.h"


extern "C" {
#include "tar.h"
}

///////////////////////////////////////////////////////////////////////////////
// Globals

int g_CodePage = -1;

///////////////////////////////////////////////////////////////////////////////
// virtual aaBase destructor

aaBase::~aaBase () {}

///////////////////////////////////////////////////////////////////////////////
// Try to determine archive type by looking at the magic number stored in the
// file

int CALL_CONV aaDetermineArchiveType (ReadCallback ReadFunction, 
   aaHandle StreamHandle, INT64 _FileSize, int* flags)
{
    int ArchiveType = ArchiveFormatUnknown;
    if (flags != NULL)
        (*flags) = ArchiveFlagOrdinary;

    // Read a few bytes from file start
    unsigned int BytesRead;
    const int BufSize = 512;
    unsigned char buf[BufSize];
    ReadFunction (StreamHandle, 0, BufSize, buf, &BytesRead);

    // Test for old zip signature, PK00PK
    if (buf[0] == 0x50
    &&  buf[1] == 0x4B
    &&  buf[2] == 0x30
    &&  buf[3] == 0x30
    &&  buf[4] == 0x50
    &&  buf[5] == 0x4B
    &&  buf[6] == 0x03
    &&  buf[7] == 0x04)
    {
        // According to the Zip file format specification, version 6.2, 
        // a file is encrypted if any of the bits 0, 6 or 13 in the word at 
        // offset 10 are set
        unsigned short zipGeneralPurposeBits = ((unsigned short*) buf)[5];
        if ((zipGeneralPurposeBits & (1 | (1 << 6) | (1 << 13))) != 0 
        &&  flags != NULL)
            (*flags) = ArchiveFlagEncrypted;
        ArchiveType = ArchiveFormatZip;
    } else

    if (buf[0] == 0x50
    &&  buf[1] == 0x4B
    &&  buf[2] == 0x03
    &&  buf[3] == 0x04)
    {
        // According to the Zip file format specification, version 6.2, 
        // a file is encrypted if any of the bits 0, 6 or 13 in the word at 
        // offset 6 are set
        unsigned short zipGeneralPurposeBits = ((unsigned short*) buf)[3];
        if ((zipGeneralPurposeBits & (1 | (1 << 6) | (1 << 13))) != 0 
        &&  flags != NULL)
            (*flags) = ArchiveFlagEncrypted;
        ArchiveType = ArchiveFormatZip;
    } else 

    // Test for rar
    if (buf[0] == 'R' 
    &&  buf[1] == 'a' 
    &&  buf[2] == 'r' 
    &&  buf[3] == '!')
    {
        if (flags != NULL) {
            if (buf[10] & 0x0008)
                (*flags) |= ArchiveFlagSolid;
/*            if (buf[10] & MHD_PROTECT
            ||  buf[10] & MHD_PASSWORD)
                (*flags) |= ArchiveFlagEncrypted;*/
        }
        ArchiveType = ArchiveFormatRAR;
    } else 

    // Test for gz
    if (buf[0] == 31
    &&  buf[1] == 139
    &&  buf[2] == 0x08)
    {
        ArchiveType = ArchiveFormatGZ;
    } else 

    // Test for arj
    if (buf[0] == 0x60
    &&  buf[1] == 0xea
    ) 
    {
        ArchiveType = ArchiveFormatArj;
    } else 

    // Test for cab
    #ifdef CabSupport
    if   ((buf[0] == 'M'
       &&  buf[1] == 'S'
       &&  buf[2] == 'C'
       &&  buf[3] == 'F')
/*    ||    (buf[0] == 'I'
       &&  buf[1] == 'S'
       &&  buf[2] == 'c'
       &&  buf[3] == '(')*/ ) // latter type is not supported?
    {
        ArchiveType = ArchiveFormatCab;
    } else
    #endif

    // Test for 7z
    if (buf[0] == '7'
    &&  buf[1] == 'z'
    &&  buf[2] == 0xBC
    &&  buf[3] == 0xAF
    &&  buf[4] == 0x27
    &&  buf[5] == 0x1c)
    {
        ArchiveType = ArchiveFormat7Zip;
    } else

    // Test for bzip
    if (buf[0] == 'B'
    &&  buf[1] == 'Z'
    &&  buf[2] == 'h') 
    {
        ArchiveType = ArchiveFormatBZip2;
    } else

    // Test for tar
    if (is_tar (buf, BufSize))
    {
        ArchiveType = ArchiveFormatTar;
    }

    // Check for encrypted files
    if (flags != NULL 
    &&  ArchiveType != ArchiveFormatUnknown
    &&  ArchiveType != ArchiveFormatGZ) 
    {
      int result = 0;
      aaHandle tmpArcHandle = 
		  aaOpenArchive (ReadFunction, StreamHandle, _FileSize, ArchiveType, 
		                 &result, NULL);
      int FileCount = aaGetFileCount (tmpArcHandle);
      for (int i = 0; i < FileCount; i++) {
         FileInArchiveInfo FileInfo;
         aaGetFileInfo (tmpArcHandle, i, &FileInfo);
         if (FileInfo.IsEncrypted) {
            (*flags) = ArchiveFlagEncrypted;
         }
      }
      aaCloseArchive (tmpArcHandle);
    }
   
    return ArchiveType;
}

///////////////////////////////////////////////////////////////////////////////
// Callback function for reading data

HRESULT CALL_CONV readFileCallback (aaHandle StreamHandle, INT64 offset, 
   UINT32 count, void* buf, UINT32* processedSize)
{
   unsigned long ReadBytes;
   HANDLE handle = reinterpret_cast <HANDLE> (StreamHandle);
   //long offsHi = (long) (offset >> 32);
   //long offsLo = (long) (offset & ((1 << 32) -1));

   //SetFilePointer (handle, offsLo, &offsHi, FILE_BEGIN);
   SetFilePointerEx (handle, *(LARGE_INTEGER*)&offset, NULL, FILE_BEGIN);
   int result = ReadFile(handle, buf, count, &ReadBytes, NULL);

   if (processedSize != NULL)
       (*processedSize) = ReadBytes;

    if (result != 0) // count != ReadBytes is no error here
        return S_OK;
    else
        return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Callback function for writing data

HRESULT CALL_CONV writeFileCallback (aaHandle StreamHandle, INT64 offset, 
   UINT32 count, const void* buf, UINT32* processedSize)
{
    int result = 1;
    unsigned long procSize;
    HANDLE handle = reinterpret_cast <HANDLE> (StreamHandle);
    long offsHi = (long) (offset >> 32);
    long offsLo = (long) (offset & ((1 << 32) -1));

    SetFilePointer (handle, offsLo, &offsHi, FILE_BEGIN);
    result = WriteFile (handle, buf, count, &procSize, NULL);
    
    if (processedSize != NULL)
        (*processedSize) = procSize;
    
    if (result != 0 && count == procSize)
        return S_OK;
    else
        return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Access file using a file system
// Returns an integer handle to the file (casted class pointer)

aaHandle CALL_CONV aaOpenArchiveFile (const TCHAR* ArchiveFileName, int* result,
                               wchar_t* password)
{
    HANDLE FileHandle = CreateFile (ArchiveFileName, FILE_READ_DATA, 
                                    FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, NULL);
    if (FileHandle == (void*) -1) {
        if (result != NULL)
            *result = S_FALSE;
        return 0;
    }

    unsigned long FileSizeHi;
    UINT32 FileSizeLo = GetFileSize (FileHandle, &FileSizeHi);
    INT64 FileSize = (FileSizeHi << 32) + FileSizeLo;
    aaHandle StreamID = reinterpret_cast <aaHandle> (FileHandle);
    int ArchiveType = aaDetermineArchiveType (readFileCallback, StreamID, FileSize);

    if (ArchiveType != ArchiveFormatUnknown) {
        ArchiveAccessClass* aac = 
           new ArchiveAccessClass (readFileCallback, StreamID, FileSize, 
                                   ArchiveType, result, false, password);
        return (aaHandle) (aac);
    } else
        return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Access file using callbacks
// Returns an integer handle to the file (casted class pointer) or zero in
// case of an error

aaHandle CALL_CONV aaOpenArchive (ReadCallback ReadFunction, 
   aaHandle StreamHandle, INT64 _FileSize, int ArchiveType, int* result,
   wchar_t* password)
{

    //OutputDebugString(L"Open archive\n");

    if (ArchiveType == ArchiveFormatUnknown) 
        ArchiveType = aaDetermineArchiveType (ReadFunction, StreamHandle, _FileSize);

    if (ArchiveType != ArchiveFormatUnknown) {
        ArchiveAccessClass* aac = 
           new ArchiveAccessClass (ReadFunction, StreamHandle, _FileSize, 
                                   ArchiveType, result, false, password);
        return (aaHandle) (aac);
    } else
        return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Set/get the owner of an ArchiveHandle
// Useful for determining the owner of an archive handle in callbacks

bool CALL_CONV aaSetOwner (aaHandle ArchiveHandle, int newOwner)
{
    ArchiveAccessClass* aac = 
        ArchiveAccessClass::GetClassPointer (ArchiveHandle);
    if (aac != NULL) {
        aac->setOwner (newOwner);
        return true;
    } else 
        return false;
}

int  CALL_CONV aaGetOwner (aaHandle ArchiveHandle)
{
    ArchiveAccessClass* aac = 
        ArchiveAccessClass::GetClassPointer (ArchiveHandle);
    if (aac != NULL) {
        return aac->getOwner ();
    } else 
        return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Get number of files in archive

int CALL_CONV aaGetFileCount (aaHandle ArchiveHandle)
{
    ArchiveAccessClass* aac = 
        ArchiveAccessClass::GetClassPointer (ArchiveHandle);
    if (aac != NULL) {
        return aac->getFileCount ();
    } else 
        return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Get information on a file in the archive
// FileInfo is a pointer to a record that is filled by the procedure, but 
// created by the caller

int CALL_CONV aaGetFileInfo (aaHandle ArchiveHandle, int FileNum, 
                             FileInArchiveInfo* FileInfo)
{
    ArchiveAccessClass* aac = 
        ArchiveAccessClass::GetClassPointer (ArchiveHandle);
    if (aac != NULL) {
         return aac->getFileInfo (FileNum, FileInfo);
    } else 
        return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Extract bytes from an archive to memory

int CALL_CONV aaExtract (aaHandle ArchiveHandle, unsigned int FileNum, 
   aaHandle WriteStreamHandle, WriteCallback WriteFunc, UINT64* WrittenSize)
{
    ArchiveAccessClass* aac = 
        ArchiveAccessClass::GetClassPointer (ArchiveHandle);
    if (aac != NULL) {
        return aac->extractFile (FileNum, WriteStreamHandle, WriteFunc, WrittenSize);
    } else 
        return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Extract multiple files from an archive
// todo: how to report errors? Should the return value simply give the number
// of files uncompressed without errors?

void CALL_CONV aaExtractMultiple (aaHandle ArchiveHandle, int FileCount, 
    unsigned int FileNums [], WriteCallback WriteFunc,
    StreamRequest RequestStream)
{
    ArchiveAccessClass* aac = 
        ArchiveAccessClass::GetClassPointer (ArchiveHandle);
    if (aac != NULL) {
        aac->extractFiles (FileCount, FileNums, WriteFunc, RequestStream);
    } /*else 
        return ArchiveAccessErrorInvalidHandle;*/
}

///////////////////////////////////////////////////////////////////////////////
// Extract all files from an archive
// todo return value?

void CALL_CONV aaExtractAll (aaHandle ArchiveHandle, WriteCallback WriteFunc, 
                          StreamRequest RequestStream)
{
    unsigned int count = aaGetFileCount (ArchiveHandle);
    unsigned int* indices = new unsigned int[count];
    for (unsigned int i = 0; i < count; i++)
        indices[i] = i;
    aaExtractMultiple (ArchiveHandle, count, indices, WriteFunc, RequestStream);
}

///////////////////////////////////////////////////////////////////////////////
// Create new archive using streams

/*
aaHandle aaCreateArchive (WriteCallback WriteFunction, int oStreamID, 
   int ArchiveType, wchar_t* _password = NULL)
{
   if (ArchiveType != ArchiveFormatUnknown) {
        aaTarArchiveCreator* TarAC = 
           new aaTarArchiveCreator (WriteFunction, oStreamID, false, _password);
        return (aaHandle) (TarAC);
    } else
        return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Create new archive using a file system

aaHandle aaCreateArchiveFile (const char* ArchiveFileName, int ArchiveType, 
                              wchar_t* _password)
{
    HANDLE FileHandle = CreateFile (ArchiveFileName, FILE_WRITE_DATA, 
                                    FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL, NULL);
    if (FileHandle == (void*) -1) {
        return 0;
    }

    int StreamID = reinterpret_cast <int> (FileHandle);
    aaArchiveCreator* AC = NULL;

    if (ArchiveType == ArchiveFormatTar) {
        AC = new aaTarArchiveCreator (writeFileCallback, StreamID, true, _password);
    } else if (ArchiveType == ArchiveFormatZip) {
        AC = new aaZipArchiveCreator (writeFileCallback, StreamID, true, _password);
    } 

    return (aaHandle) (AC);
}

///////////////////////////////////////////////////////////////////////////////
// Add file

int CALL_CONV aaAddFromStream (aaHandle ArchiveHandle, FileInArchiveInfo* file, 
                               ReadCallback ReadFunc, int iStreamID)
{
    aaArchiveCreator* nac = 
        aaArchiveCreator::GetClassPointer (ArchiveHandle);
    if (nac != NULL) {
        return nac->addFile(file, ReadFunc, iStreamID);
    } else 
        return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Add file to new archive

DLL_EXPORT int CALL_CONV aaAddFile (aaHandle ArchiveHandle, const char* FileName)
{
   FileInArchiveInfo FileInfo; 

   // Store only relative paths
   if (FileName[1] == ':' && FileName[2] == '\\')
      bgstrlcpy (FileInfo.path, FileName+3, FileInArchiveInfoStringSize-3);
   else if (FileName[0] == '/')
      bgstrlcpy (FileInfo.path, FileName+2, FileInArchiveInfoStringSize-2);
   else
      bgstrlcpy (FileInfo.path, FileName, FileInArchiveInfoStringSize);
   
   FileInfo.attributes = GetFileAttributes (FileName);
   FileInfo.IsDir = (FileInfo.attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
   if (FileInfo.attributes == 0xFFFFFFFF) {
      // Error: File does not exist
      return false;
   } else if (!FileInfo.IsDir) {
      // Add normal file
      HANDLE FileHandle = CreateFile (FileName, FILE_READ_DATA, 
                                      FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL, NULL);

      if (FileHandle != 0) {
         // Get file info
         unsigned long FileSizeHi;
         UINT32 FileSizeLo = GetFileSize (FileHandle, &FileSizeHi);
         FileInfo.UncompressedFileSize = (FileSizeHi << 32) + FileSizeLo;
         _FILETIME CreationTime, LastAccessTime;
         GetFileTime(FileHandle, &CreationTime, &LastAccessTime, 
                     &FileInfo.LastWriteTime);
         int AddResult =  aaAddFromStream (ArchiveHandle, &FileInfo, 
                                           readFileCallback, (int) FileHandle);
         CloseHandle (FileHandle);
         return AddResult;
      } else
         return S_FALSE; // Error: could not open file
   } else {
      // Add directory
      FileInfo.UncompressedFileSize = 0;
      return aaAddFromStream (ArchiveHandle, &FileInfo, NULL, NULL);
   }
}
*/

///////////////////////////////////////////////////////////////////////////////
// Create new archive using a file system

aaHandle aaCreateArchiveFile (const TCHAR* ArchiveFileName, int ArchiveType, 
                              wchar_t* _password)
{
   return new aaArchiveCreator (ArchiveFileName, ArchiveType, _password);
}

///////////////////////////////////////////////////////////////////////////////
// Add file to new archive

DLL_EXPORT void CALL_CONV aaAddFile (aaHandle ArchiveHandle, const TCHAR* FileName)
{
    aaArchiveCreator* aac = 
        aaArchiveCreator::GetClassPointer (ArchiveHandle);
    if (aac != NULL) {
        aac->addFile (FileName);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Close archive

int CALL_CONV aaCloseArchive (aaHandle ArchiveHandle)
{

    //OutputDebugString(L"Close archive\n");

    ArchiveAccessClass* aac = 
       ArchiveAccessClass::GetClassPointer (ArchiveHandle);
    if (aac != NULL) {
        //OutputDebugString(L"delete aac\n");
        delete aac;
        return S_OK;
    } else {
        aaArchiveCreator* nac = 
            aaArchiveCreator::GetClassPointer (ArchiveHandle);
        if (nac != NULL) {
            //OutputDebugString(L"delete nac\n");
            delete nac;
            return S_OK;
        } else
            return S_FALSE;
    }
}
