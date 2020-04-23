/*---------------------------------------------------------------------------
PROJECT: Steem SSE
DOMAIN: File
FILE: unzip_win32.h
DESCRIPTION: Declarations for the interface with unzip32d.dll
struct TPackRec
---------------------------------------------------------------------------*/


/*  origin of unzipd32.dll finally found
https://www.sac.sk/files.php?d=7&p=62
https://www.sac.sk/download/pack/unzdl206.zip contains the dll in question

from Pascal source

type  TDirtype=array[0..259] of char;
 
      TPackRec=packed record
        internal:array[0..11] of byte;  {Used internally by the dll}
        Time,                     {file time}
        Size,                     {file size}
        CompressSize,             {size in zipfile}
        headeroffset,             {file offset in zip: needed in unzipfile}
        CRC: Longint;             {CRC, sort of checksum}
        FileName: tdirtype;       {file name}
        PackMethod,               {pack method, see below}
        Attr,                     {file attribute}
        Flags:word;               {lo byte: arj_flags; hi byte: file_type}
      end;
in original Steem thanks: "Christian Ghisler for his unzip library"
in unizip.pas: C code by info-zip group, translated to pascal by Christian Ghisler
{based on unz51g.zip}
{Special thanks go to Mark Adler,
 who wrote the main inflate and explode code,
 and did NOT copyright it!!!}
*/

#pragma once
#ifndef UNZIP_WIN32_H
#define UNZIP_WIN32_H

#pragma pack(push, 8) //?

struct TPackRec {
  char InternalUse[12];    // Used internally by the dll
  int Time;                // File time
  int Size;                // File size
  int CompressSize;        // Size in zipfile
  int HeaderOffset;        // File offset in zip
  long Crc;                // CRC, sort of checksum
  char FileName[260];      // File name
  WORD PackMethod;         /* 0=Stored, 1=Shrunk, 2=Reduced 1, 3=Reduced 2, 4=Reduced 3, 5=Reduced 4,
                              6=Imploded,7=Tokenized (format does not exist), 8=Deflated,
                              More than 8=Unknown method.
                              For this DLL this number can only be 0, 8, or more than 8
                              */
  WORD Attr;               // File attributes
  WORD Flags;              // Only used by ARJ unpacker (LOBYTE: arj_flags, HIBYTE: file_type)
};

#pragma pack(pop)

extern int (PASCAL *GetFirstInZip)(char*,TPackRec*); //find out what files are in the ZIP file (first file)
extern int (PASCAL *GetNextInZip)(TPackRec*);        //get next file in ZIP
extern void (PASCAL *CloseZipFile)(TPackRec*);       //free buffers and close ZIP after GetFirstInZip()
extern BYTE (PASCAL *isZip)(char*);                    //determine if a file is a ZIP file
extern int (PASCAL *UnzipFile)(char*,char*,WORD,long,void*,long);        //unzipping

#define UNZIP_Ok           0               // Unpacked ok
#define UNZIP_CRCErr       1               // CRC error
#define UNZIP_WriteErr     2               // Error writing out file: maybe disk full
#define UNZIP_ReadErr      3               // Error reading zip file
#define UNZIP_ZipFileErr   4               // Error in zip structure
#define UNZIP_UserAbort    5               // Aborted by user
#define UNZIP_NotSupported 6               // ZIP Method not supported!
#define UNZIP_Encrypted    7               // Zipfile encrypted
#define UNZIP_InUse        -1              // DLL in use by other program!
#define UNZIP_DLLNotFound  -2              // DLL not loaded!


#endif//#ifndef UNZIP_WIN32_H
