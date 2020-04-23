#pragma once
#ifndef NOTWIN_MYMISCH_H
#define NOTWIN_MYMISCH_H

#if defined(UNIX) || defined(BEOS)
#ifndef SLASH
#define SLASH "/"
#define SLASHCHAR '/'
#endif

#ifndef MAX_PATH
#define MAX_PATH 5000
#endif

#ifndef DWORD
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned int UINT;
#endif

#else

#ifndef SLASH
#define SLASH "\\"
#define SLASHCHAR '\\'
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#endif

void ZeroMemory(void *Mem,DWORD Len);
bool DeleteFile(char *File);
UINT GetTempFileName(char *PathName,char *Prefix,UINT Unique,char *TempFileName);
BOOL CopyFile(
  LPCSTR lpExistingFileName,
  LPCSTR lpNewFileName,
  BOOL    bFailIfExists
);

#endif//#ifndef NOTWIN_MYMISCH_H
