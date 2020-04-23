///////////////////////////////////////////////////////////////////////////////
// ArchiveAccessDynamic.h
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This file declares some access functions to file archives
///////////////////////////////////////////////////////////////////////////////

/*

7-Zip is a library that contains read/write access functions for dlls.
It is implemented in C++, so there is no easy way to use these functions
in Delphi. The purpose of ArchiveAccess is to provide access to 7-zip's
classes using plain C. These functions can be used easily in Delphi.

*/

///////////////////////////////////////////////////////////////////////////////
// Includes

#ifndef ArchiveAccess__H
#define ArchiveAccess__H

#include "../CPP/Common/Defs.h"
#include "windows.h"

///////////////////////////////////////////////////////////////////////////////
// Types

typedef void* aaHandle;
class aaBase 
{
public:
   virtual ~aaBase () = 0; // needs at least one virtual function for rtti
};

///////////////////////////////////////////////////////////////////////////////
// Defines & Constants

// Archive Format Id's
// Simply use the first four characters in extension name as a 32 bit integer,
// padded with spaces
const int ArchiveFormatUnknown =  0;
const int ArchiveFormatZip     = 'zip '; // works (even some encrypted files)
const int ArchiveFormat7Zip    = '7z  '; // works
const int ArchiveFormatRAR     = 'rar '; // works (even some encrypted files)
const int ArchiveFormatTar     = 'tar '; // works
const int ArchiveFormatGZ      = 'gz  '; // works
const int ArchiveFormatBZip2   = 'bz2 '; // works
const int ArchiveFormatArj     = 'arj '; // works
//const int ArchiveFormatCab     = 'cab '; // untested
//const int ArchiveFormatRPM     = 'rpm '; // untested
//const int ArchiveFormatDeb     = 'deb '; // untested
//const int ArchiveFormatCPIO    = 'cpio'; // untested

// Archive Flags returned when calling determineArchiveType
const int ArchiveFlagOrdinary = 0;
const int ArchiveFlagEncrypted = 1;
const int ArchiveFlagSolid = 2;

// This struct contains file information from an archive. The caller may store 
// this information for accessing this file after calls to findFirst, findNext
const int FileInArchiveInfoStringSize = 1024;
struct FileInArchiveInfo {
	int ArchiveHandle; // handle for Archive/class pointer
    //int FileIndex;
	UINT64 CompressedFileSize;
	UINT64 UncompressedFileSize;
	UINT32 attributes;
	bool IsDir, IsEncrypted;
	_FILETIME LastWriteTime, CreationTime, LastAccessTime;
	unsigned short path[FileInArchiveInfoStringSize];
};

// Possible stream requests
enum RequestedActionEnum {
	OpenStream = 1,
	CloseStream = 2
};

///////////////////////////////////////////////////////////////////////////////
// Callbacks

// Read/Write Callbacks
// This is the callback function declaration for read access to
// the file containing the archive
// StreamID: used to identify the stream to the callback function
// offset: the offset in the file where reading starts
// count: number of bytes to read
// buf: pointer to buffer containg the data read after success
// processedSize: number of bytes actually read (out)
// HRESULT: returns the error state of the operation
typedef HRESULT (__stdcall *ReadCallback) 
(int StreamID, INT64 offset, UINT32 count, void* buf, UINT32* processedSize);

typedef HRESULT (__stdcall *WriteCallback) 
(int StreamID, INT64 offset, UINT32 count, const void* buf, UINT32* processedSize);

// Request a write stream 
// This callback is given the index of the file to decompress and
// should return a handle for a WriteCallback
// RequestedAction: see RequestedActionEnum
// index: for OpenStream, the index of the file in the archive
//        for CloseStream, the StreamID returned by an OpenStream operation

typedef int (__stdcall *StreamRequest) (aaHandle ArchiveHandle, int index, 
										int RequestedAction);

///////////////////////////////////////////////////////////////////////////////
// Exported functions

extern "C" {

#define CALL_CONV __stdcall

// Try to determine archive type by looking at the magic number stored in the
// file
typedef int (CALL_CONV *paaDetermineArchiveType) 
(ReadCallback ReadFunction, int StreamID, INT64 _FileSize, int* flags);
paaDetermineArchiveType aaDetermineArchiveType;

// Initialize codecs
//typedef int (CALL_CONV *paaInit) (const TCHAR* codecsPath);
//paaInit aaInit;

// Access file using a file system
// Returns an integer handle to the file (casted class pointer)
// 0 indicates failure
typedef aaHandle (CALL_CONV *paaOpenArchiveFile) (char* ArchiveFileName, 
   int* result);
paaOpenArchiveFile aaOpenArchiveFile;

// Access file using a file system
// Returns an integer handle to the file (casted class pointer)
// 0 indicates failure
//int openArchive (char* fileName);

// Access file using blockwise transfer, for reading from stdin or writing to 
// stdout
// Returns an integer handle to the file (casted class pointer), 
// 0 indicates failure
// function is a callback to a read/write function
// StreamID is used to identify this data stream to function
typedef aaHandle (CALL_CONV *paaOpenArchive) (ReadCallback function, 
   int StreamID, INT64 _FileSize, int ArchiveType, int* result, 
   wchar_t* password);
paaOpenArchive aaOpenArchive;

// Get number of files in archive
typedef int (CALL_CONV *paaGetFileCount) (aaHandle ArchiveHandle);
paaGetFileCount aaGetFileCount;

// Get information on a file in the archive
// FileInfo is a pointer to a record that is filled by the procedure, but 
// created by the caller
typedef int (CALL_CONV *paaGetFileInfo) (aaHandle ArchiveHandle, int FileNum,
                                         FileInArchiveInfo* FileInfo);
paaGetFileInfo aaGetFileInfo;

// Extract bytes from an archive to memory
typedef int (CALL_CONV *paaExtract) (aaHandle ArchiveHandle, int FileNum, 
                                     int StreamID, WriteCallback WriteFunc,
                                     UINT64* WrittenSize);
paaExtract aaExtract;

// Extract multiple files from an archive
// todo: how to report errors? Should the return value simply return the number
// of files uncompressed without errors? The cleanest way would be an error
// callback
typedef int (CALL_CONV *paaExtractMultiple) (aaHandle ArchiveHandle, int FileCount, 
    unsigned int FileNums [], WriteCallback WriteFunc,
    StreamRequest RequestStream);
paaExtractMultiple aaExtractMultiple;

// Extract all files from an archive
typedef int (CALL_CONV *paaExtractAll) (aaHandle ArchiveHandle, 
    WriteCallback WriteFunc, StreamRequest RequestStream);
paaExtractAll aaExtractAll;

// Extract bytes from an archive to memory
typedef int (CALL_CONV *paaCloseArchive) (aaHandle ArchiveHandle);
paaCloseArchive aaCloseArchive;

}

#endif
