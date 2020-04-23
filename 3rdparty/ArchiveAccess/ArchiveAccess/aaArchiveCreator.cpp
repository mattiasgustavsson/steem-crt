//////////////////////////////////////////////////////////////////////////////
// aaArchiveCreator.h
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This is the implementation of a class that creates new archives
///////////////////////////////////////////////////////////////////////////////

#include "aaArchiveCreator.h"
#include <windows.h>
#include "7zip/UI/Common/ArchiveCommandLine.h"
#include "7zip/UI/Console/OpenCallbackConsole.h"
#include "7zip/UI/Console/UpdateCallbackConsole.h"
#include "Common/StdOutStream.h"
#include "windows/FileFind.h"

//#ifdef FORMAT_ZIP
#include "7zip/Archive/Zip/ZipHandler.h"
//#endif

using namespace NWindows;
using namespace NFile;

///////////////////////////////////////////////////////////////////////////////
// Constructor

aaArchiveCreator::aaArchiveCreator (const TCHAR* ArchiveFileName, int ArchiveType, 
                                    wchar_t* _password)
{
   outArchive = new NArchive::NZip::CHandler();

   // todo: create file for archive
}

///////////////////////////////////////////////////////////////////////////////
// Add file to archive, given its name

void aaArchiveCreator::addFile (const TCHAR* FileName)
{
   CDirItem newItem;

   // todo: Fill out newItem
   HANDLE handle = CreateFile (FileName, 
                            FILE_READ_ATTRIBUTES, // access rights
                            FILE_SHARE_DELETE | FILE_SHARE_READ | 
                            FILE_SHARE_WRITE, // Shared access flags
                            NULL, // Security descriptor
                            OPEN_EXISTING, // Creation disposition
                            0, // File attributes
                            NULL); // Template file
   newItem.Attrib = GetFileAttributes (FileName);
   //getAttributesByHandle (handle, &newItem.Attributes, 0);
   GetFileTime (handle, &newItem.CTime, &newItem.ATime, &newItem.MTime);
   DWORD fsLow, fsHigh;
   fsLow = GetFileSize (handle, &fsHigh);
   newItem.Size = fsLow + (fsHigh << 32);
//   UString Name = FileName; // todo
//   UString FullPath = FileName; // todo

   DirItems.Add (newItem);
}

///////////////////////////////////////////////////////////////////////////////
// Destructor

aaArchiveCreator::~aaArchiveCreator ()
{
   // Commit changes to the archive
   // todo: parameters
   //UpdateWithItemLists();
   UpdateArchive();

   // Clean up
   delete outArchive;
}

///////////////////////////////////////////////////////////////////////////////
// Copy count bytes from input file into output stream

/*bool aaArchiveCreator::CopyFile (ReadCallback ReadFunc, int iStreamID, 
                                 UINT64 ReadStartOffset, UINT64 count)
{
   char buf[512];
   UINT64 readOffset = ReadStartOffset;
   UINT32 blockRead, blockWritten;
   UINT64 done = 0;
   UINT64 amount;
   do {
      amount = count - done;
      if (amount > sizeof (buf))
         amount = sizeof (buf);
      ReadFunc (iStreamID, readOffset, (UINT32) amount, buf, &blockRead);
      outStream.Write (&buf, blockRead, &blockWritten);
      done += blockWritten;
      readOffset += blockWritten;
   } while (blockWritten == amount
     &&     done < count);

   return (done == count);
}*/

///////////////////////////////////////////////////////////////////////////////
// Add files as requested by addFile

bool aaArchiveCreator::UpdateArchive()
{
  // Taken from 7zip/UI/Console/Main.cpp
/*  #ifdef _WIN32  
  SetFileApisToOEM();
  #endif

  CArchiveCommandLineOptions options;

  // todo: Implement own c-compatible, redirectable stream
  CStdOutStream &stdStream = options.StdOutMode ? g_StdErr : g_StdOut;

//  if (options.UpdateOptions.SfxMode 
//  &&  options.UpdateOptions.SfxModule.IsEmpty())
//    options.UpdateOptions.SfxModule = kDefaultSfxModule;

  bool passwordIsDefined = 
    options.PasswordEnabled && !options.Password.IsEmpty();

  COpenCallbackConsole openCallback;
  openCallback.OutStream = &stdStream;
  openCallback.PasswordIsDefined = passwordIsDefined;
  openCallback.Password = options.Password;

  CUpdateCallbackConsole callback;
  callback.EnablePercents = options.EnablePercents;
  callback.PasswordIsDefined = passwordIsDefined;
  callback.AskPassword = options.PasswordEnabled && options.Password.IsEmpty();
  callback.Password = options.Password;
  callback.StdOutMode = options.UpdateOptions.StdOutMode;
  callback.Init(&stdStream);

  CUpdateErrorInfo errorInfo;

  // Taken from UI/Common/Update.cpp
  const UString archiveName = options.ArchivePath.GetFinalPath();

  UString defaultItemName;
  NFind::CFileInfoW archiveFileInfo;

  UString tempDirPrefix;
  bool usesTempDir = false;
  
  CTempFiles tempFiles;

  bool createTempFile = false;
  if(!options.StdOutMode && options.UpdateOptions.UpdateArchiveItself)
  {
    CArchivePath &ap = options.UpdateOptions.Commands[0].ArchivePath;
    ap = options.ArchivePath;
    // if ((archive != 0 && !usesTempDir) || !options.WorkingDir.IsEmpty())
    if ((   archive != 0 
        ||  !options.UpdateOptions.WorkingDir.IsEmpty()) 
    && !usesTempDir 
    && options.UpdateOptions.VolumesSizes.Size() == 0)
    {
      createTempFile = true;
      ap.Temp = true;
      if (!options.UpdateOptions.WorkingDir.IsEmpty())
      {
        ap.TempPrefix = options.UpdateOptions.WorkingDir;
        NormalizeDirPathPrefix(ap.TempPrefix);
      }
    }
  }

  for(int i = 0; i < options.UpdateOptions.Commands.Size(); i++)
  {
    CArchivePath &ap = options.UpdateOptions.Commands[i].ArchivePath;
    if (usesTempDir)
    {
      // Check it
      ap.Prefix = tempDirPrefix;
      // ap.Temp = true;
      // ap.TempPrefix = tempDirPrefix;
    }
    if (i > 0 || !createTempFile)
    {
      const UString &path = ap.GetFinalPath();
      if (NFind::DoesFileExist(path))
      {
        errorInfo.SystemError = 0;
        errorInfo.Message = L"File already exists";
        errorInfo.FileName = path;
        return E_FAIL;
      }
    }
  }

  CObjectVector<CArchiveItem> archiveItems;
  if (archive != NULL)
  {
    RINOK(EnumerateInArchiveItems(censor, 
        archive, defaultItemName, archiveFileInfo, archiveItems));
  }

  RINOK(UpdateWithItemLists(options, archive, archiveItems, dirItems, 
      tempFiles, errorInfo, callback));

  if (archive != NULL)
  {
    RINOK(archiveLink.Close());
    archiveLink.Release();
  }

  tempFiles.Paths.Clear();
  if(createTempFile)
  {
    try
    {
      CArchivePath &ap = options.Commands[0].ArchivePath;
      const UString &tempPath = ap.GetTempPath();
      if (archive != NULL)
         if (!NDirectory::DeleteFileAlways(archiveName))
        {
          errorInfo.SystemError = ::GetLastError();
          errorInfo.Message = L"delete file error";
          errorInfo.FileName = archiveName;
          return E_FAIL;
        }
      if (!NDirectory::MyMoveFile(tempPath, archiveName))
      {
        errorInfo.SystemError = ::GetLastError();
        errorInfo.Message = L"move file error";
        errorInfo.FileName = tempPath;
        errorInfo.FileName2 = archiveName;
        return E_FAIL;
      }
    }
    catch(...)
    {
      throw;
    }
  }

  #ifdef _WIN32
  if (options.EMailMode)
  {
    NDLL::CLibrary mapiLib;
    if (!mapiLib.Load(TEXT("Mapi32.dll")))
    {
      errorInfo.SystemError = ::GetLastError();
      errorInfo.Message = L"can not load Mapi32.dll";
      return E_FAIL;
    }
    LPMAPISENDDOCUMENTS fnSend = (LPMAPISENDDOCUMENTS)
        mapiLib.GetProcAddress("MAPISendDocuments");
    if (fnSend == 0)
    {
      errorInfo.SystemError = ::GetLastError();
      errorInfo.Message = L"can not find MAPISendDocuments function";
      return E_FAIL;
    }
    UStringVector fullPaths;
    int i;
    for(i = 0; i < options.Commands.Size(); i++)
    {
      CArchivePath &ap = options.Commands[i].ArchivePath;
      UString arcPath;
      if(!NDirectory::MyGetFullPathName(ap.GetFinalPath(), arcPath))
      {
        errorInfo.SystemError = ::GetLastError();
        return E_FAIL;
      }
      fullPaths.Add(arcPath);
    }
    CCurrentDirRestorer curDirRestorer;
    for(i = 0; i < fullPaths.Size(); i++)
    {
      UString arcPath = fullPaths[i];
      UString fileName = ExtractFileNameFromPath(arcPath);
      AString path = GetAnsiString(arcPath);
      AString name = GetAnsiString(fileName);
      // Warning!!! MAPISendDocuments function changes Current directory
      fnSend(0, ";", (LPSTR)(LPCSTR)path, (LPSTR)(LPCSTR)name, 0); 
    }
  }
  #endif

  // Taken from 7zip/UI/Console/Main.cpp
/*  if (result != S_OK)
  {
    stdStream << "\nError:\n";
    if (!errorInfo.Message.IsEmpty())
      stdStream << errorInfo.Message << endl;
    if (!errorInfo.FileName.IsEmpty())
      stdStream << errorInfo.FileName << endl;
    if (!errorInfo.FileName2.IsEmpty())
      stdStream << errorInfo.FileName2 << endl;
    if (errorInfo.SystemError != 0)
      stdStream << NError::MyFormatMessageW(errorInfo.SystemError) << endl;
    throw CSystemException(result);
  }
  int exitCode = NExitCode::kSuccess;
  int numErrors = callback.FailedFiles.Size();
  if (numErrors == 0)
    stdStream << kEverythingIsOk << endl;
  else
  {
    stdStream << endl;
    stdStream << "WARNINGS for files:" << endl << endl;
    for (int i = 0; i < numErrors; i++)
    {
      stdStream << callback.FailedFiles[i] << " : ";
      stdStream << NError::MyFormatMessageW(callback.FailedCodes[i]) << endl;
    }
    stdStream << "----------------" << endl;
    stdStream << "WARNING: Cannot open " << numErrors << " file";
    if (numErrors > 1)
      stdStream << "s";
    stdStream << endl;
    exitCode = NExitCode::kWarning;
  }*/
return false;
}