/*---------------------------------------------------------------------------
PROJECT: Steem SSE
Atari ST emulator
Copyright (C) 2020 by Anthony Hayward and Russel Hayward + SSE

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see https://www.gnu.org/licenses/.

DOMAIN: File
FILE: archive.cpp
DESCRIPTION: This file contains the code for zipclass, Steem's abstraction
of the various unarchiving libraries it can use.
---------------------------------------------------------------------------*/

#include "pch.h"
#pragma hdrstop
#include <archive.h>
#include <computer.h>
#include <diskman.h>
#ifdef UNIX
#include <gui.h>
#endif
// global for conversion from unicode, reused in other parts
char ansi_string[MAX_PATH]; 

#ifdef WIN32
int (PASCAL *GetFirstInZip)(char*,TPackRec*); //find out what files are in the ZIP file (first file)
int (PASCAL *GetNextInZip)(TPackRec*);        //get next file in ZIP
void (PASCAL *CloseZipFile)(TPackRec*);       //free buffers and close ZIP after GetFirstInZip()
BYTE (PASCAL *isZip)(char*);                    //determine if a file is a ZIP file
int (PASCAL *UnzipFile)(char*,char*,WORD,long,void*,long);        //unzipping
#endif

zipclass zippy; 
#ifdef UNIX
bool enable_zip=true;
#else
bool enable_zip=false;
#endif
bool FloppyArchiveIsReadWrite=0;


#ifdef WIN32

#if !defined(SSE_NO_UNZIPD32) // x64: no unzipd64.dll available

HINSTANCE hUnzip=NULL;


void LoadUnzipDLL() {
  hUnzip=SteemLoadLibrary(UNZIP_DLL); // unzipd32.dll
  if(hUnzip) 
  {
    GetFirstInZip=(int(PASCAL*)(char*,TPackRec*))GetProcAddress(hUnzip,"GetFirstInZip");
    GetNextInZip=(int(PASCAL*)(TPackRec*))GetProcAddress(hUnzip,"GetNextInZip");
    CloseZipFile=(void(PASCAL*)(TPackRec*))GetProcAddress(hUnzip,"CloseZipFile");
    isZip=(BYTE(PASCAL*)(char*))GetProcAddress(hUnzip,"isZip");
    UnzipFile=(int(PASCAL*)(char*,char*,WORD,long,void*,long))GetProcAddress(hUnzip,"unzipfile");
    if(!(GetFirstInZip && GetNextInZip && CloseZipFile && isZip && UnzipFile))
    {
      FreeLibrary(hUnzip);
      hUnzip=NULL;
    }
    else
      enable_zip=SSEConfig.unzipd32Dll=true;
  }//hunzip
#ifdef SSE_DEBUG
  if(hUnzip)
    TRACE_INIT("%s loaded\n",UNZIP_DLL);
  else
    TRACE_INIT("%s not available",UNZIP_DLL);
#endif
}

#endif

#if defined(SSE_UNRAR_SUPPORT_WIN)

#define LOGSECTION LOGSECTION_INIT

HINSTANCE hUnrar=NULL;
HANDLE (PASCAL *rarOpenArchive)(struct RAROpenArchiveData *ArchiveData)=NULL;
int (PASCAL *rarCloseArchive)(HANDLE hArcData)=NULL;
int (PASCAL *rarReadHeader)(HANDLE hArcData,struct RARHeaderData *HeaderData)=NULL;
int (PASCAL *rarProcessFile)(HANDLE hArcData,int Operation,char *DestPath,char *DestName)=NULL;

void LoadUnrarDLL() {
  UNRAR_OK=false;
  hUnrar=SteemLoadLibrary(UNRAR_DLL);
  if(hUnrar)
  {
    // get dll functions
    rarOpenArchive=(HANDLE(PASCAL*)(struct RAROpenArchiveData *ArchiveData))
      GetProcAddress(hUnrar,"RAROpenArchive");
    rarCloseArchive=(int(PASCAL*)(HANDLE hArcData))
      GetProcAddress(hUnrar,"RARCloseArchive");
    rarReadHeader=(int(PASCAL*)(HANDLE hArcData,struct RARHeaderData *HeaderData))
      GetProcAddress(hUnrar,"RARReadHeader");
    rarProcessFile=(int(PASCAL*)(HANDLE hArcData,int Operation,char *DestPath,char *DestName))
      GetProcAddress(hUnrar,"RARProcessFile");
    if(rarOpenArchive&&rarCloseArchive&&rarReadHeader&&rarProcessFile)
    {
      // prefill structures for all archives
      ZeroMemory(&zippy.ArchiveData,sizeof(zippy.ArchiveData));
      ZeroMemory(&zippy.HeaderData,sizeof(zippy.HeaderData));
      zippy.ArchiveData.OpenMode=RAR_OM_EXTRACT;
      enable_zip=true;
      UNRAR_OK=true;
    }
    else
      FreeLibrary(hUnrar);
  }
}

#undef LOGSECTION
#endif
#endif//WIN32

#define LOGSECTION LOGSECTION_IMAGE_INFO


zipclass::zipclass() {
  type[0]=0;
  is_open=false;
  current_file_n=0;
  err=0;
#if defined(SSE_UNRAR_SUPPORT_WIN)
  hArcData=0;
#endif
}


#if defined(SSE_ARCHIVEACCESS_SUPPORT)

bool zipclass::for_archiveaccess() { // little helper function, v401
  return (ARCHIVEACCESS_OK && (
      MatchesAnyString_I(type,"7Z","BZ2","GZ","TAR","ARJ",NULL)
      || !SSEConfig.unzipd32Dll && (strcmp(type,"ZIP")==0||strcmp(type,"STZ")==0)
      || !SSEConfig.UnrarDll && strcmp(type,"RAR")==0 ));
}

#endif

#if defined(SSE_7Z_SUPPORT_UNIX)

bool zipclass::for_7za() {
  return (MatchesAnyString_I(type,"7Z","BZ2","GZ","TAR","ARJ",NULL));
}

#endif


bool zipclass::first(char *name) {
  if(enable_zip==0) 
    return ZIPPY_FAIL;
  if(is_open) 
    close();
  type[0]=0;
  char *dot=strrchr(name,'.');
  if(dot) 
  {
    if(strlen(dot+1)<11) 
      strcpy(type,dot+1);
  }
  if(type[0]==0) 
    strcpy(type,"ZIP");
  strupr(type);
#if defined(SSE_UNRAR_SUPPORT_UNIX)
  if(UNRAR_OK && strcmp(type,"RAR")) 
  {
    // first() provokes extraction of archive
    char command_string[1024]; // unsafe
    sprintf(command_string,"unrar e -o+ \"%s\" %s/unzip/",name,WriteDir.Text);
    int ec=system(command_string);
    TRACE("%d=%s\n",ec,command_string);
    if(!ec)
    {
      is_open=true;
      current_file_n=0;
      current_file_offset=0;
      crc=0; //?
      return ZIPPY_SUCCEED;
    }
    else
      return ZIPPY_FAIL;//tmp
  }
  else
#endif
#if defined(SSE_7Z_SUPPORT_UNIX)
  if(for_7za())
  {
    // first() provokes extraction of archive
    char command_string[1024]; // unsafe
    // could be 7z for extra?
    sprintf(command_string,"7za x -o%s/unzip/ \"%s\" ",WriteDir.Text,name);
    int ec=system(command_string);
    TRACE("%d=%s\n",ec,command_string);
    if(!ec)
    {
      is_open=true;
      current_file_n=0;
      current_file_offset=0;
      crc=0; //?
      return ZIPPY_SUCCEED;
    }
    else
      return ZIPPY_FAIL;//tmp
  } 
  else
#endif
#if defined(SSE_ARCHIVEACCESS_SUPPORT)
  // ArchiveAccess handles zip and rar if other plugins missing
  if(for_archiveaccess())
  {
    if(ArchiveAccess_Open(name))
    {
      //TRACE_LOG("sizeof(FileInfo)=%d\n",sizeof(FileInfo)); // surprise! was 2100 in bcc (2104 in vc)
      is_open=true;
      current_file_n=0;
      current_file_offset=0;
      m_attrib=WORD(FileInfo.attributes);
      crc=0; //?
      return ZIPPY_SUCCEED;
    }
    else
      return ZIPPY_FAIL;
  }
  else
#endif
#if !defined(SSE_NO_UNZIPD32)
  if(strcmp(type,"ZIP")==0||strcmp(type,"STZ")==0)
  {
#ifdef UNIX
    uf=unzOpen(name);
    if (uf==NULL){
      last_error="Couldn't open ";
      last_error+=name;
      return ZIPPY_FAIL;
    }else{
      is_open=true;
      err=unzGetGlobalInfo(uf,&gi);
      current_file_n=0;
      current_file_offset=0;
      err=unzGetCurrentFileInfo(uf,&fi,filename_inzip,
            sizeof(filename_inzip),NULL,0,NULL,0);
      if (err){
        close();
        return ZIPPY_FAIL;
      }
      crc=fi.crc;
    }
#endif
#ifdef WIN32
    ZeroMemory(&PackInfo,sizeof(PackInfo));
    if(GetFirstInZip(name,&PackInfo)!=UNZIP_Ok)
      return ZIPPY_FAIL;
    is_open=true;
    current_file_n=0;
    current_file_offset=PackInfo.HeaderOffset;
    m_attrib=PackInfo.Attr;
    crc=PackInfo.Crc;
#endif
    return ZIPPY_SUCCEED;
  } else
#endif
#ifdef RARLIB_SUPPORT
  if (strcmp(type,"RAR")==0){
    if (urarlib_list(name,(ArchiveList_struct*)&rar_list)<=0) return ZIPPY_FAIL;

    // There are some files in this RAR
    while (rar_list){
      if ((rar_list->item.FileAttr & 0x10)==0) break; // Not directory
      rar_list=rar_list->next;
    }
    if (rar_list==NULL) return ZIPPY_FAIL;

    is_open=true;
    rar_current=rar_list;
    current_file_n=0;
    current_file_offset=0;
    m_attrib=WORD(rar_current->item.FileAttr);
    crc=rar_current->item.FileCRC;

    return ZIPPY_SUCCEED;
  } else
#endif
#if defined(SSE_UNRAR_SUPPORT_WIN)
  if(UNRAR_OK && strcmp(type,"RAR")==0) 
  {
    ArchiveData.ArcName=name;
    hArcData=rarOpenArchive(&ArchiveData);
    ASSERT(hArcData);
    ASSERT(!ArchiveData.OpenResult);
    if(!hArcData || ArchiveData.OpenResult)
      return ZIPPY_FAIL;
//    TRACE_LOG("UnRAR: %s now open\n",name);
    int ec=rarReadHeader(hArcData,&HeaderData);
    if(!ec)
    {
//      ASSERT( !(HeaderData.FileAttr&0x10) ); // directory!
//      TRACE_LOG("UnRAR 1st file %s v%d\n",HeaderData.FileName,HeaderData.UnpVer);
      is_open=true;
      current_file_n=0;
      current_file_offset=0;
      m_attrib=WORD(HeaderData.FileAttr);
      crc=HeaderData.FileCRC;
      return ZIPPY_SUCCEED;
    }
  }
#else
  ;
#endif
  return ZIPPY_FAIL;
}


bool zipclass::next() {
  if(enable_zip==0)
    return ZIPPY_FAIL;
    
#if defined(SSE_UNRAR_SUPPORT_UNIX) || defined(SSE_7Z_SUPPORT_UNIX)
  if(0
#if defined(SSE_UNRAR_SUPPORT_UNIX)  
    ||(strcmp(type,"RAR")==0)
#endif
#if defined(SSE_7Z_SUPPORT_UNIX)
    ||(for_7za())
#endif
    )
  {
    bool ok=false;
    DirSearch ds; 
    EasyStr Fol=RunDir+SLASH+"unzip"+SLASH;
    if(ds.Find(Fol+"*.*"))
    {
      while(ds.Name=="." || ds.Name=="..")
        ds.Next();      
      for(int i=0;i<=current_file_n;i++)
        ok=ds.Next();
    }
    ds.Close();
    if(ok)
    {
      current_file_n++;
      current_file_offset=current_file_n;
      return ZIPPY_SUCCEED;      
    }
    return ZIPPY_FAIL;
  }
  else
#endif        
#if defined(SSE_ARCHIVEACCESS_SUPPORT)
  if(for_archiveaccess()) 
  {
    if(is_open==0||!ArchiveHandle)
      return ZIPPY_FAIL;
    //ASSERT(current_archived_file==current_file_n); // not if folders TODO?
    if(ArchiveAccess_Select(current_file_n+1))
    {
      current_file_n++;
      current_archived_file++;
      
      m_attrib=WORD(FileInfo.attributes);
      current_file_offset=current_file_n;
      return ZIPPY_SUCCEED;
    }
  } else
#endif
#if !defined(SSE_NO_UNZIPD32)
  if(strcmp(type,"ZIP")==0||strcmp(type,"STZ")==0)
  {
#ifdef UNIX
    if (is_open==0) return ZIPPY_FAIL;
  	if (current_file_n>=(int)(gi.number_entry-1)) return ZIPPY_FAIL;
    err=unzGoToNextFile(uf);
    if (err) return ZIPPY_FAIL;
    err=unzGetCurrentFileInfo(uf,&fi,filename_inzip,
          sizeof(filename_inzip),NULL,0,NULL,0);
    if (err) return ZIPPY_FAIL;
    current_file_n++;
    current_file_offset=current_file_n;
    crc=fi.crc;
#endif
#ifdef WIN32
    err=GetNextInZip(&PackInfo);
    if (err!=UNZIP_Ok) return ZIPPY_FAIL;
    m_attrib=PackInfo.Attr;
    crc=PackInfo.Crc;
    current_file_n++;
    current_file_offset=PackInfo.HeaderOffset;
#endif
    return ZIPPY_SUCCEED;
  }else
#endif
#ifdef RARLIB_SUPPORT
  if (strcmp(type,"RAR")==0){
    if (is_open==0 || rar_current==NULL) return ZIPPY_FAIL;
    do{
      rar_current=rar_current->next;
      current_file_n++;
      if (rar_current==NULL) return ZIPPY_FAIL;
    }while (rar_current->item.FileAttr & 0x10); // Skip if directory
    current_file_offset=current_file_n;
    m_attrib=WORD(rar_current->item.FileAttr);
    crc=rar_current->item.FileCRC;
    return ZIPPY_SUCCEED;
  }else
#endif
#if defined(SSE_UNRAR_SUPPORT_WIN)
  if(strcmp(type,"RAR")==0) 
  {
    if(is_open==0||!UNRAR_OK||!hArcData)
      return ZIPPY_FAIL;
    int ec;
    // we must 'process' the current one or reading will fail
    ec=rarProcessFile(hArcData,RAR_TEST,NULL,NULL);
    ASSERT(!ec);
    ec=rarReadHeader(hArcData,&HeaderData);
    current_file_n++;
    if(!ec)
    {
//      ASSERT( !(HeaderData.FileAttr&0x10) ); // directory!
//      TRACE_LOG("UnRAR next file %s\n",HeaderData.FileName);
      current_file_offset=current_file_n;
      m_attrib=WORD(HeaderData.FileAttr);
      crc=HeaderData.FileCRC;
      return ZIPPY_SUCCEED;
    }
//    else
//      TRACE_LOG("UnRAR next %d ec %d\n",current_file_n,ec);
  }
#else
  ;
#endif
  return ZIPPY_FAIL;
}


char* zipclass::filename_in_zip() {
  if(enable_zip==0) 
    return "";

#if defined(SSE_UNRAR_SUPPORT_UNIX) || defined(SSE_7Z_SUPPORT_UNIX)
  if(0
#if defined(SSE_UNRAR_SUPPORT_UNIX)  
    ||(strcmp(type,"RAR")==0)
#endif
#if defined(SSE_7Z_SUPPORT_UNIX)
    ||(for_7za())
#endif
    )    
  {
    DirSearch ds; 
    EasyStr Fol=RunDir+SLASH+"unzip"+SLASH;
    if(ds.Find(Fol+"*.*"))
    {
     // do{
      //TRACE("file %s\n",ds.Name.Text); // . and ..
      //}while(ds.Next());
      while(ds.Name=="." || ds.Name=="..")
        ds.Next();
      for(int i=0;i<current_file_n;i++)
        ds.Next();
      strcpy(ansi_string,ds.Name.Text);
    }
    ds.Close();
    return ansi_string;
  }
  else
#endif      
 
#if defined(SSE_ARCHIVEACCESS_SUPPORT)
  if(for_archiveaccess())
  {
    if(ArchiveHandle) // need to convert name from unicode
    {
      WideCharToMultiByte ( CP_ACP,                // ANSI code page
        WC_COMPOSITECHECK,     // Check for accented characters
        (LPCWSTR)FileInfo.path,         // Source Unicode string
        -1,                    // -1 means string is zero-terminated
        (char*)ansi_string, //SS a global
        MAX_PATH,  // Size of buffer
        NULL,                  // No default character
        NULL );                // Don't care about this flag
      TRACE_LOG("%S -> %s\n",FileInfo.path,ansi_string);
      return (char*)ansi_string;
    }
  } else
#endif
#if !defined(SSE_NO_UNZIPD32)
  if(strcmp(type,"ZIP")==0||strcmp(type,"STZ")==0) 
  {
#ifdef UNIX
    return filename_inzip;
#endif
#ifdef WIN32
    return PackInfo.FileName;
#endif
  } else
#endif//aa4
#ifdef RARLIB_SUPPORT
  if(strcmp(type,"RAR")==0) 
  {
    if (rar_current) return rar_current->item.Name;
  } else
#endif
#if defined(SSE_UNRAR_SUPPORT_WIN)
  if(strcmp(type,"RAR")==0) 
  {
    if(UNRAR_OK&&hArcData) 
      return HeaderData.FileName;
  }
#else
  ;
#endif
  return "";
}


bool zipclass::close() {
  if(enable_zip==0) 
    return ZIPPY_FAIL;
  if(is_open) 
  {
#if defined(SSE_UNRAR_SUPPORT_UNIX) || defined(SSE_7Z_SUPPORT_UNIX)
    if(0
#if defined(SSE_UNRAR_SUPPORT_UNIX)  
      ||(strcmp(type,"RAR")==0)
#endif
#if defined(SSE_7Z_SUPPORT_UNIX)
      ||(for_7za())
#endif
      )    
    {
      char command_string[1024]; // unsafe
      sprintf(command_string,"rm -r %s/unzip/*.*",WriteDir.Text);
      int ec=system(command_string);
      TRACE("%d=%s\n",ec,command_string);    
      is_open=false;
      return ZIPPY_SUCCEED;    
    }
    else
#endif    
#if defined(SSE_ARCHIVEACCESS_SUPPORT)
    if(for_archiveaccess()) 
    {
      ArchiveAccess_Close();
      is_open=false;
      return ZIPPY_SUCCEED;
    }
    else
#endif
#if !defined(SSE_NO_UNZIPD32)
    if(strcmp(type,"ZIP")==0||strcmp(type,"STZ")==0)
    {
      UNIX_ONLY( unzClose(uf); )
      WIN_ONLY( CloseZipFile(&PackInfo); )
      is_open=false;
      return ZIPPY_SUCCEED;
    } else
#endif
#ifdef RARLIB_SUPPORT
    if(strcmp(type,"RAR")==0)
    {
      is_open=false;
      return ZIPPY_SUCCEED;
    } else
#endif
#if defined(SSE_UNRAR_SUPPORT_WIN)
    if(UNRAR_OK && strcmp(type,"RAR")==0)
    {
//      TRACE_LOG("closing RAR archive\n");
      VERIFY( !rarCloseArchive(hArcData) );
      is_open=false;
      return ZIPPY_SUCCEED;
    }
#else
    ;
#endif
  }
  return ZIPPY_FAIL;
}


void zipclass::list_contents(char *name,EasyStringList *eslp,bool st_disks_only) {
  if(enable_zip==0) 
    return;
  eslp->DeleteAll();
  if(first(name)==0) 
  {
    do{
      EasyStr a=filename_in_zip();
      bool addflag=true;
      if(st_disks_only) 
      {
        if(FileIsDisk(a)==DISK_UNCOMPRESSED
#if defined(SSE_TOS_PRG_AUTORUN)
          // no interference with context menu, MSA Converter
          && !MatchesAnyString_I(strrchr(a.Text,'.')+1,
          DISK_EXT_PRG,DISK_EXT_TOS,NULL)
#endif
          || FileIsDisk(a)==DISK_PASTI)
          addflag=true;
        else
          addflag=0;
      }
      if(addflag) 
        eslp->Add(a,current_file_offset,m_attrib,crc);
    }while (next()==0);
  }
  close();
}


bool zipclass::extract_file(char *fn,int offset,char *dest_dir,bool hide,
                            DWORD attrib) {
//  TRACE_LOG("zippy extract %s (#%d) to %s\n",fn,offset,dest_dir);
  if(enable_zip==0) 
    return ZIPPY_FAIL;
#if defined(SSE_UNRAR_SUPPORT_UNIX) || defined(SSE_7Z_SUPPORT_UNIX)
  if(0
#if defined(SSE_UNRAR_SUPPORT_UNIX)  
    ||(strcmp(type,"RAR")==0)
#endif
#if defined(SSE_7Z_SUPPORT_UNIX)
    ||(for_7za())
#endif
    )    
  {
    bool ok=false;
    if(is_open)
      close();    
      
    if(first(fn)==0)
    {
      DirSearch ds; 
      EasyStr Fol=RunDir+SLASH+"unzip"+SLASH;
      
      if(ds.Find(Fol+"*.*"))
      {
        ok=true;
        while(ds.Name=="." || ds.Name=="..")
          ds.Next();        
        for(int i=0;i<offset;i++)
          ok=ds.Next();
        if(ok)
        {
          char command_string[1024]; // unsafe
          sprintf(command_string,"cp \"%s/unzip/%s\" \"%s\"",WriteDir.Text,ds.Name.Text,dest_dir);
          int ec=system(command_string);
          TRACE("%d=%s\n",ec,command_string);
        }
      }
      ds.Close();
    }
    close();
    return (ok) ? ZIPPY_SUCCEED : ZIPPY_FAIL;
  }
  else
#endif      
#if defined(SSE_ARCHIVEACCESS_SUPPORT)
  if(for_archiveaccess()) 
  {
    if(is_open)
      close();
    if(first(fn)==0) 
    {
      while(offset>0) {
        if(next()) { // Failed
          close();
          return ZIPPY_FAIL;
        }
        offset--;
      }
    }
    ArchiveAccess_Extract(dest_dir);
    close();
#ifdef WIN32
    if(hide)
      SetFileAttributes(dest_dir,FILE_ATTRIBUTE_HIDDEN);
    else
      SetFileAttributes(dest_dir,attrib);
#endif
    return ZIPPY_SUCCEED;
  } else
#endif
#if !defined(SSE_NO_UNZIPD32)
  if(strcmp(type,"ZIP")==0||strcmp(type,"STZ")==0) 
  {
    if(is_open) 
      close();
#ifdef UNIX
    uf=unzOpen(fn);
    if (uf==NULL){
      last_error=Str("Couldn't open ")+fn;
      return ZIPPY_FAIL;
    }
    is_open=true;
    err=unzGetGlobalInfo(uf,&gi);
    if (err){
      close();
      return ZIPPY_FAIL;
    }
    unz_global_info gi;
    int err=unzGetGlobalInfo(uf,&gi);
    if (err!=UNZ_OK){
      close();
      last_error="couldn't get global info";
      return ZIPPY_FAIL;
    }
    if (offset>=(int)gi.number_entry){
      close();
      last_error="too few files in zip";
      return ZIPPY_FAIL;
    }
  //  unzGoToFirstFile(uf);
    for (int i=0;i<offset;i++) unzGoToNextFile(uf);
#define UNZIP_BUF_SIZE 8192
    BYTE buf[UNZIP_BUF_SIZE];
    err=unzGetCurrentFileInfo(uf,&fi,filename_inzip,
          sizeof(filename_inzip),NULL,0,NULL,0);
    if (err) return ZIPPY_FAIL;
    EasyStr dest_file=dest_dir;
    if (dest_dir[0]==0 || dest_dir[strlen(dest_dir)-1]=='/'){
      char *t=strrchr(filename_inzip,'/');
      if (t){
        t++;
      }else{
        t=strrchr(filename_inzip,'\\');
        if(t){
          t++;
        }else{
          t=filename_inzip;
        }
      }
      dest_file+=t;
    }
    err=unzOpenCurrentFile(uf);
    if (err){
      close();
      last_error="error opening file in ZIP";
      printf("%s\n",last_error.Text);
      return ZIPPY_FAIL;
    }
    FILE *fout=fopen(dest_file,"wb");
    if (fout==NULL){
      close();unzCloseCurrentFile(uf);
      last_error="error opening file ";last_error+=dest_file.Text;
      printf("%s\n",last_error.Text);
      return ZIPPY_FAIL;
    }
    do{
      err=unzReadCurrentFile(uf,buf,UNZIP_BUF_SIZE);
      if (err<0){
        last_error=EasyStr("error #")+err+" with zipfile in unzReadCurrentFile";
        printf("%s\n",last_error.Text);
        break;
      }else if(err>0){
        fwrite(buf,err,1,fout);
      }
    }while (err>0);
    fclose(fout);
    err=unzCloseCurrentFile(uf);
    close();
    if (err) return ZIPPY_FAIL;
#endif
#ifdef WIN32
    EasyStr dest_file=dest_dir;
    err=UnzipFile(fn,dest_file.Text,(WORD)(hide ? 2:attrib),offset,NULL,0);
    if(err!=UNZIP_Ok) 
      return ZIPPY_FAIL;
#endif
    return ZIPPY_SUCCEED;
  } else
#endif
#ifdef RARLIB_SUPPORT
  if (strcmp(type,"RAR")==0){
    if (is_open) close();
    if (first(fn)==0){
      while (offset > 0){
        if (next()){ // Failed
          close();
          return ZIPPY_FAIL;
        }
        offset--;
      }
    }
    char *data_ptr;
    DWORD data_size;
    if (urarlib_get(&data_ptr,&data_size,rar_current->item.Name,fn,"")==0) return ZIPPY_FAIL;
    close();
    EasyStr dest_file=dest_dir;
    FILE *f=fopen(dest_file,"wb");
    if (f==NULL) return ZIPPY_FAIL;
    fwrite(data_ptr,1,data_size,f);
    fclose(f);
#ifdef WIN32
    if (hide){
      SetFileAttributes(dest_file,FILE_ATTRIBUTE_HIDDEN);
    }else{
      SetFileAttributes(dest_file,m_attrib);
    }
#endif
    return ZIPPY_SUCCEED;
  } else 
#endif
#if defined(SSE_UNRAR_SUPPORT_WIN)
  if(UNRAR_OK && strcmp(type,"RAR")==0)
  {
    if(is_open)
      close();
    if(first(fn)==0) 
    {
      while(offset>0) {
        if(next()) { // Failed
          close();
          return ZIPPY_FAIL;
        }
        offset--;
      }
    }
    //ASSERT(hArcData);
    int ec=rarProcessFile(hArcData,RAR_EXTRACT,NULL,dest_dir); 
//    TRACE_LOG("%s -> %s : %d\n",HeaderData.FileName,dest_dir,ec);
    //ASSERT(!ec);
    if(ec)
      return ZIPPY_FAIL;    
    close();
#ifdef WIN32
    if(hide)
      SetFileAttributes(dest_dir,FILE_ATTRIBUTE_HIDDEN);
    else
      SetFileAttributes(dest_dir,attrib);
#endif
    return ZIPPY_SUCCEED;
  }
#endif
  return ZIPPY_FAIL;
}
