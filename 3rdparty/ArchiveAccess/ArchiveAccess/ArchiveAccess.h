///////////////////////////////////////////////////////////////////////////////
// ArchiveAccess.h
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This file declares some access functions to file archives
///////////////////////////////////////////////////////////////////////////////

/*

7-Zip is a library that contains functions for accessing archives. It is 
implemented in C++, so there is no easy way to use these functions in Delphi. 
The purpose of ArchiveAccess is to provide a well-documented plain C layer to 
7-zip's classes. It should also be usable with other programming languages, 
such as Delphi (tested), Java or Visual Basic.

*/

///////////////////////////////////////////////////////////////////////////////
// Includes

#ifndef ArchiveAccess__H
#define ArchiveAccess__H

#include "Common/Defs.h"
#include "../CPP/7zip/UI/common/LoadCodecs.h"
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
const int ArchiveFormat7Zip    =(int) '7z  '; // works
const int ArchiveFormatRAR     = 'rar '; // works (even some encrypted files)
const int ArchiveFormatTar     = 'tar '; // works
const int ArchiveFormatGZ      = 'gz  '; // works
const int ArchiveFormatBZip2   = 'bz2 '; // works
const int ArchiveFormatArj     = 'arj '; // works
//const int ArchiveFormatCab     = 'cab '; // untested
//const int ArchiveFormatRPM     = 'rpm '; // untested
//const int ArchiveFormatDeb     = 'deb '; // untested
//const int ArchiveFormatCPIO    = 'cpio'; // untested
const int ArchiveFormatJAR     = 'jar ';

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
(aaHandle StreamHandle, INT64 offset, UINT32 count, void* buf, UINT32* processedSize);

typedef HRESULT (__stdcall *WriteCallback) 
(aaHandle StreamHandle, INT64 offset, UINT32 count, const void* buf, UINT32* processedSize);

// Request a write stream 
// This callback is given the index of the file to decompress and
// should return a handle for a WriteCallback
// RequestedAction: see RequestedActionEnum
// index: for OpenStream, the index of the file in the archive
//        for CloseStream, the StreamID returned by an OpenStream operation

// todo: split into two functions, one opening files, one for closing the files

typedef aaHandle (__stdcall *StreamRequest) (aaHandle ArchiveHandle, 
	aaHandle index, int RequestedAction);

///////////////////////////////////////////////////////////////////////////////
// Exported functions

extern "C" {

#define DLL_EXPORT __declspec(dllexport)
//#define DLL_EXPORT 
#define CALL_CONV __stdcall

// Determine archive type from file without knowing its extension
// Returns one of the archive types in ArchiveAccessDefs.h
DLL_EXPORT int CALL_CONV 
aaDetermineArchiveType (ReadCallback ReadFunction, aaHandle StreamHandle, 
                        INT64 _FileSize, int* flags = NULL);

// Initialize codecs
DLL_EXPORT CCodecs * CALL_CONV
aaGetCodecs ();
DLL_EXPORT void CALL_CONV
aaInit (const TCHAR* codecsPath = NULL);

// Access file using a file system
// Returns a handle to the file (casted class pointer)
// 0 indicates failure
DLL_EXPORT aaHandle CALL_CONV aaOpenArchiveFile (const char* ArchiveFileName, int* result,
                                                 wchar_t* password = NULL);

// Access file using blockwise transfer, for reading from stdin or writing to 
// stdout
// Returns an integer handle to the file (casted class pointer), 
// 0 indicates failure
// function is a callback to a read/write function
// StreamID is used to identify this data stream to function
DLL_EXPORT aaHandle CALL_CONV aaOpenArchive (ReadCallback ReadFunction, aaHandle StreamHandle, 
                                      INT64 _FileSize, int ArchiveType, 
                                      int* result, wchar_t* password = NULL);

// Set/get the owner of an ArchiveHandle
// Useful for determining the owner of an archive handle in callbacks
DLL_EXPORT bool CALL_CONV aaSetOwner (aaHandle ArchiveHandle, int newOwner);
DLL_EXPORT int  CALL_CONV aaGetOwner (aaHandle ArchiveHandle);

// Get number of files in archive
DLL_EXPORT int CALL_CONV aaGetFileCount (aaHandle ArchiveHandle);

// Get information on a file in the archive
// FileInfo is a pointer to a record that is filled by the procedure, but 
// created by the caller
DLL_EXPORT int CALL_CONV aaGetFileInfo (aaHandle ArchiveHandle, int FileNum, 
									  FileInArchiveInfo* FileInfo);

// Extract bytes from an archive to memory, returns an HRESULT
DLL_EXPORT int CALL_CONV aaExtract (aaHandle ArchiveHandle, unsigned int FileNum, 
								  aaHandle WriteStreamHandle, WriteCallback WriteFunc,
								  UINT64* WrittenSize = NULL);

// Extract multiple files from an archive
// todo: how to report errors? Should the return value simply return the number
// of files uncompressed without errors? The cleanest way would be an error
// callback or multiline-error report, null-terminated
DLL_EXPORT void CALL_CONV aaExtractMultiple (aaHandle ArchiveHandle, int FileCount, 
	unsigned int FileNums [], WriteCallback WriteFunc,
	StreamRequest RequestStream);

// Extract all files from an archive
// todo: how to report errors?
DLL_EXPORT void CALL_CONV aaExtractAll (aaHandle ArchiveHandle, 
   WriteCallback WriteFunc, StreamRequest RequestStream);

// Create new archive using streams
/*DLL_EXPORT aaHandle aaCreateArchive (WriteCallback WriteFunction, 
   int oStreamID, INT64 _FileSize, int ArchiveType, int* result, 
   bool CloseStreamAfterDestruction = false, wchar_t* _password = NULL);*/


// Create new archive using a file system
DLL_EXPORT aaHandle aaCreateArchiveFile (
   const TCHAR* ArchiveFileName, int ArchiveType, wchar_t* _password = NULL);

// Add file to new archive
/*DLL_EXPORT int CALL_CONV aaAddFromStream (FileInArchiveInfo* file, 
   ReadCallback ReadFunc, int iStreamID);*/

// Add file to new archive, store path names relative to current directory
DLL_EXPORT void CALL_CONV aaAddFile (aaHandle ArchiveHandle, const TCHAR* FileName);

// Close archive
// All FileInArchiveInfo's are invalid after calling this function
DLL_EXPORT int CALL_CONV aaCloseArchive (aaHandle ArchiveHandle);

}

#endif