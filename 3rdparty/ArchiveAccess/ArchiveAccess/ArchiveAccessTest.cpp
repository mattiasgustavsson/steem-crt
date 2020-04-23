///////////////////////////////////////////////////////////////////////////////
// ArchiveAccessTest.cpp 
// Copyright 2004 X-Ways Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This file serves as a testbed/example of how to use ArchiveAccess
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#include "../ArchiveAccess/bgstr.h"
#include "../ArchiveAccess/ArchiveAccessBase.h"

///////////////////////////////////////////////////////////////////////////////
// Settings

//#define UseStaticLinking
//#define DumpData
//#define DumpHex
#define UseStreams


#if defined(STEVEN_SEAGAL) && defined(BCC_BUILD)
#define max(a,b) ( (a)>(b)?(a):(b) )
#endif


const char DllFileName[] = "ArchiveAccess.dll";
//const char DllFileName[] = "ArchiveAccess-debug.dll";
const char UnpackDir[] = "t:\\unpacked\\";
const size_t FileNameSize = 1024;

///////////////////////////////////////////////////////////////////////////////
// Static library

#ifdef UseStaticLinking

#include "../ArchiveAccess/ArchiveAccess.h"

bool LoadArchiveAccessDll (const char*)
{
    return true;
}

#endif

///////////////////////////////////////////////////////////////////////////////
// Dynamically loaded library

#ifndef UseStaticLinking

#include "../ArchiveAccess/ArchiveAccessDynamic.h"

HINSTANCE hinstLib;

bool LoadArchiveAccessDll (const TCHAR* LibName)
{
    hinstLib = LoadLibrary(LibName); 
    if (hinstLib != NULL) {
        aaDetermineArchiveType = (paaDetermineArchiveType) GetProcAddress(hinstLib, "aaDetermineArchiveType");
//		aaInit = (paaInit) GetProcAddress(hinstLib, "aaInit");
        aaOpenArchiveFile = (paaOpenArchiveFile) GetProcAddress(hinstLib, "aaOpenArchiveFile");
        aaOpenArchive = (paaOpenArchive) GetProcAddress(hinstLib, "aaOpenArchive"); 
        aaGetFileCount = (paaGetFileCount) GetProcAddress(hinstLib, "aaGetFileCount");
        aaGetFileInfo = (paaGetFileInfo) GetProcAddress(hinstLib, "aaGetFileInfo"); 
        aaExtract = (paaExtract) GetProcAddress(hinstLib, "aaExtract");
        aaCloseArchive = (paaCloseArchive) GetProcAddress(hinstLib, "aaCloseArchive");

		bool t1 = (aaDetermineArchiveType != NULL);
		bool t2 = (aaOpenArchiveFile != NULL);
		bool t3 = (aaOpenArchive != NULL);
		bool t4 = (aaGetFileCount != NULL);
		bool t5 = (aaGetFileInfo != NULL);
		bool t6 = (aaExtract != NULL);
		bool t7 = (aaCloseArchive != NULL);

		return (t1 && t2 && t3 && t4 && t5 && t6 && t7);
    } else 
        return false;
}

void UnloadLib ()
{
    FreeLibrary(hinstLib); 
}

#endif

///////////////////////////////////////////////////////////////////////////////
// Dump buffer

#ifdef UseStreams
void DumpBuf (char* buf, int count)
{
    const char HexChars[] = "0123456789abcdef";
    const int ValuesPerLine = 16;
    const int AddressChars = 4;
    int i, j;

    for (i = 0; i * ValuesPerLine < count; i++) {

        for (j = AddressChars-2; j >= 0; j--)
            cout << HexChars[(i >> (j*4)) & 15];
        cout << "0: ";

        //cout << HexChars[(i>>4) & 15] << HexChars[i & 15] << "0: ";
        for (j = 0; j < ValuesPerLine; j++) {
            if (i * ValuesPerLine + j < count) {
                unsigned char c = buf[i * ValuesPerLine + j];
                cout << HexChars[(c>>4) & 15] << HexChars[c & 15] << " ";
            } else
                cout << "   ";
        }

        for (j = 0; j < ValuesPerLine; j++) {
            if (i * ValuesPerLine + j < count) {
                unsigned char c = max (buf[i * ValuesPerLine + j], 32);
                cout << c;
            } else
                cout << " ";
        }
        cout << endl;
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Callback function for reading data

#ifdef UseStreams
HRESULT __stdcall readCallback (int StreamID, INT64 offset, UINT32 count, 
                                void* buf, UINT32* processedSize)
{
   #ifdef DumpData
      cout << "Reading " << count << " bytes from offset "
           << (unsigned long) offset << endl;
   #endif

   unsigned long ReadBytes;
   HANDLE handle = reinterpret_cast <HANDLE> (StreamID);
   long offsHi = (long) (offset >> 32);
   long  offsLo = (long) (offset & ((1 << 32) -1));

   SetFilePointer (handle, offsLo, &offsHi, FILE_BEGIN);
   int result = ReadFile(handle, buf, count, &ReadBytes, NULL);

   if (processedSize != NULL)
       (*processedSize) = ReadBytes;

   if (count != ReadBytes)
	   printf ("Read %i bytes from offset %i, but %i bytes were requested!\n",
	      ReadBytes, (int) offset, count);
       //cout << "Read " << ReadBytes << " bytes from offset " << (int) offset 
       //     << ", but " << count << " bytes were requested!" << endl;

   #ifdef DumpHex
      DumpBuf ((char*) (buf), min (count, 32));
   #endif

    if (result != 0)
        return S_OK;
    else
        return S_FALSE;
}

#endif

///////////////////////////////////////////////////////////////////////////////
// Callback function for writing data

HRESULT __stdcall writeCallback (int StreamID, INT64 offset, UINT32 count, 
                                 const void* buf, UINT32* processedSize)
{
    #ifdef DumpData
        cout << "Writing " << count << " bytes" << endl;
    #endif

    int result = 1;
    if (count != 0) {
        unsigned long procSize;
        HANDLE outFileHandle = reinterpret_cast <HANDLE> (StreamID);

        long offsHi = (long) (offset >> 32);
        long offsLo = (long) (offset & ((1 << 32) -1));

        SetFilePointer (outFileHandle, offsLo, &offsHi, FILE_BEGIN);
        result = WriteFile (outFileHandle, buf, count, &procSize, NULL);
    
        if (processedSize != NULL)
            (*processedSize) = procSize;
    
       if (count != procSize)
           cout << "Wrote " << procSize << " bytes, but " << count 
                << " bytes were requested!" << endl;
    }
    
    if (result != 0)
        return S_OK;
    else
        return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Find last occurance of c
// Returns last occurance of c or the length of str

int FindLast (char c, char* str)
{
    int i = 0;
    int res = 0;
    while (str[i] != 0) {
        if (str[i] == c) {
            res = i;
        }
        i++;
    }

    return res;
}

///////////////////////////////////////////////////////////////////////////////
// Create any folders necessary to allow creation of file given in newPath

void CreateNewPathElements (const TCHAR* newPath)
{
    TCHAR newPathCopy[FileNameSize];
    bgstrlcpy (newPathCopy, newPath, FileNameSize);
    int i = 0;

    while (newPathCopy[i] != 0) 
    {
        if (newPathCopy[i] == '\\' || newPathCopy[i] == '/') 
        {
            // Set newPath to end at index i
            TCHAR backup = newPathCopy[i];
            newPathCopy[i] = 0;

            // Check if path exists
            WIN32_FIND_DATA FindFileData;
            HANDLE hFind = FindFirstFile(newPathCopy, &FindFileData);
            bool FileExists = (hFind != INVALID_HANDLE_VALUE);
            FindClose(hFind);

            if (!FileExists) {
                // Create path
                CreateDirectory (newPathCopy, NULL);
            }

            // Restore newPath
            newPathCopy[i] = backup;
        }
        i++;
    }
}

///////////////////////////////////////////////////////////////////////////////
// main function

#ifdef _WIN32
#ifdef _UNICODE
int wmain( int argc, wchar_t *argv[ ], wchar_t *envp[ ] )
#else
int main(int argc, char* argv[])
#endif
#else
int main(int argc, char* argv[])
#endif

{
    if (argc <= 1) {
        printf("Usage: archiveaccess <filename>\n"
               "This will extract <filename> to the current directory\n"
               "Licensed under LGPL");
        exit (0);
    }

	// Figure out path to DLL
	const size_t dllPathLen = 300;
	TCHAR dllPath[dllPathLen];
	const TCHAR* prgPath = argv[0];
	size_t prgPathLen = bgstrlen (prgPath);
	bool found = false;
	if (prgPathLen > 3)
		if (prgPath[1] == ':' && prgPath[2] == OSPathSep) {
			// Absolute path given
            size_t lastSepPos = bgReverseStrScan (prgPath, prgPathLen-1, (TCHAR) OSPathSep);
			bgstrlcpy (dllPath, prgPath, min (dllPathLen, lastSepPos+2));
			bgstrlcat (&dllPath[lastSepPos+1], DllFileName, bgstrlen(DllFileName)+1);
			found = true;
		}
	if (!found)
	   bgstrlcpy(dllPath, prgPath, prgPathLen);
	
	if (!LoadArchiveAccessDll (dllPath)) {
       printf ("Error loading %s\n", dllPath);
	   exit (0);
	}

//	aaInit (L"D:\Programme\7-Zip\Codecs");

	wchar_t password[] = L"xxx";
    aaHandle ArchiveHandle = 0;
    int OpenArchiveError = 0;
    TCHAR* ArchiveFileName = argv[1];
    #ifdef UseStreams
       // Open file using CreateFile
       HANDLE FileHandle = CreateFile (ArchiveFileName, FILE_READ_DATA, 
                                       FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL, NULL);

       if (FileHandle == (void*) -1) {
           OpenArchiveError = 1;
       } else {
          // Open archive
          int iStreamID = reinterpret_cast <int> (FileHandle);
          unsigned long FileSizeHi;
          UINT32 FileSizeLo = GetFileSize (FileHandle, &FileSizeHi);
          INT64 FileSize = (FileSizeHi << 32) + FileSizeLo;
          int flags;
          int ArchiveType = 
             aaDetermineArchiveType (readCallback, iStreamID, FileSize, &flags);
          if (flags & ArchiveFlagEncrypted)
             printf("Archive contains encrypted files\n");
          if (flags & ArchiveFlagSolid)
             printf("Archive is solid\n");
          ArchiveHandle = 
             aaOpenArchive (readCallback, iStreamID, FileSize, 
                            ArchiveType, &OpenArchiveError, password);
       }
    #else
       // Let ArchiveAccess open the file
       ArchiveHandle = 
          openArchiveFile (ArchiveFileName, &OpenArchiveError, password);
    #endif

    if (OpenArchiveError == 0) {

        int FileCount = aaGetFileCount (ArchiveHandle);
        int errors = 0;
		wprintf (L"%s contains %i files.\n", ArchiveFileName, FileCount);
        //cout << " contains " << FileCount << " files."´<< endl;
        for (int i = 0; i < FileCount; i++) {
            FileInArchiveInfo FileInfo;
            aaGetFileInfo (ArchiveHandle, i, &FileInfo);

            // Get file name of file to extract
            //TCHAR newPath[FileNameSize];
            TCHAR* newPath = new TCHAR[bgstrlen(UnpackDir) + bgstrlen(FileInfo.path) + 1];

			//assert (bgstrlen(UnpackDir) + bgstrlen(FileInfo.path) < FileNameSize);
            bgstrlcpy (newPath, UnpackDir, FileNameSize);
            if (bgstrlen (FileInfo.path) == 0 /* || FileCount == 1*/ ) {
                bgstrlcat (newPath, "\\", FileNameSize);
                int ArchiveFileWOPath = bgReverseStrScan (ArchiveFileName, 
					                       bgstrlen(ArchiveFileName), (TCHAR) OSPathSep);
                bgstrlcat (newPath, ArchiveFileName + ArchiveFileWOPath + 1, 
                              FileNameSize);
                bgstrlcat (newPath, ".unpacked", FileNameSize);
				if (FileInfo.UncompressedFileSize > 0 && FileInfo.UncompressedFileSize < 0xffFFffFF) {
					wprintf(L"Extracting %I64d bytes to ", FileInfo.UncompressedFileSize);
					wprintf(L"%s\n", newPath);
				} else
					wprintf(L"Extracting to %s, size unknown\n", newPath);
            } else {
                bgstrlcat (newPath, "\\", FileNameSize);
                bgstrlcat (newPath, FileInfo.path, FileNameSize);
				wprintf (L"Extracting %s, size: %I64d, to ", FileInfo.path,
                         FileInfo.UncompressedFileSize);
				wprintf(L"%s\n", newPath);

				/*SYSTEMTIME sysTime;
				FileTimeToSystemTime (&FileInfo.CreationTime, &sysTime);
				printf ("ctime: %i.%i.%i, %i:%i:%i\n", sysTime.wDay, sysTime.wMonth, sysTime.wYear,
					     sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

				FileTimeToSystemTime (&FileInfo.LastWriteTime, &sysTime);
				printf ("mtime: %i.%i.%i, %i:%i:%i\n", sysTime.wDay, sysTime.wMonth, sysTime.wYear,
					     sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

				FileTimeToSystemTime (&FileInfo.LastAccessTime, &sysTime);
				printf ("atime: %i.%i.%i, %i:%i:%i\n", sysTime.wDay, sysTime.wMonth, sysTime.wYear,
					     sysTime.wHour, sysTime.wMinute, sysTime.wSecond);*/
			}

            CreateNewPathElements (newPath);

            // Extract or create directory
            /*if (FileInfo.IsEncrypted) 
            {
                cout << "Cannot open encrypted file" << endl;
                errors ++;
            } else */ if (FileInfo.IsDir) {
				wprintf(L"Creating directory %s\n", newPath);
                CreateDirectory (newPath, NULL);
            } else {
                HANDLE outFileHandle = 
                    CreateFile (newPath, FILE_WRITE_DATA, FILE_SHARE_READ, 
                                NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 
                                NULL);
                UINT64 ExtractedFileSize;
                int oStreamID = reinterpret_cast <int> (outFileHandle);
                aaExtract (ArchiveHandle, i, oStreamID, writeCallback, 
                           &ExtractedFileSize);
                CloseHandle (outFileHandle);

                // Test for right file size
                if (FileInfo.UncompressedFileSize != ExtractedFileSize) 
                {
                    cout << "Extracted " << ExtractedFileSize
                         << " bytes instead of " 
                         << FileInfo.UncompressedFileSize 
                         << endl << endl;
                    errors ++;
                } 
            }

        }
		printf("%i errors\n", errors);
        aaCloseArchive (ArchiveHandle);
    } else {
        wprintf(L"Error %i opening archive %s\n", 
			   OpenArchiveError, ArchiveFileName);
    }
    #ifdef UseStreams
    // Close file and archive, then wait for user to press key to quit
    CloseHandle (FileHandle);
    #endif

    printf("\nPress return to exit\n");
    scanf ("c");
    return 0;
}

