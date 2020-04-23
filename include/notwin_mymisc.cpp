#ifdef UNIX
#include <../pch.h>
#endif


#include <conditions.h>
#include <stdio.h>
#include "easystr.h"

void ZeroMemory(void *Mem,DWORD Len) {
  memset(Mem,0,Len);
}


bool DeleteFile(char *File) {
  return unlink(File)==0;
}


UINT GetTempFileName(char *PathName,char *Prefix,UINT Unique,
                      char *TempFileName) {
  EasyStr Ret;
  WORD Num=WORD(Unique?WORD(Unique):WORD(rand()&0xffff));
  for(;;)
  {
    Ret=PathName;
    Ret+=SLASH;
    Ret+=Prefix;
    Ret.SetLength(MAX_PATH+4);
    char *StartOfNum=Ret.Right()+1;
    ultoa(Num,StartOfNum,16);
    strupr(StartOfNum);
    Ret+=".TMP";
    if(Ret.Length()<MAX_PATH)
    {
      if(Unique==0)
      {
        if(access(Ret,0)==0)
          //File exists
          Num++;
        else
        {
          strcpy(TempFileName,Ret);
          fclose(fopen(TempFileName,"wb"));
          return Num;
        }
      }
      else
      {
        strcpy(TempFileName,Ret);
        return Num;
      }
    }
    else
      return 0;
  }
}


BOOL CopyFile(LPCSTR lpExistingFileName,LPCSTR lpNewFileName,BOOL bFailIfExists) {
  if(!Exists(lpExistingFileName)||Exists(lpNewFileName))
    return FALSE;
  FILE* source = fopen(lpExistingFileName, "rb");
  FILE* dest = fopen(lpNewFileName, "wb");

  // clean and more secure
  // feof(FILE* stream) returns non-zero if the end of file indicator for stream is set
  size_t size;
  BYTE buf;
  while (size = fread(&buf, 1, 1, source)) {
      fwrite(&buf, 1, size, dest);
  }

  fclose(source);
  fclose(dest);
  return TRUE;
}