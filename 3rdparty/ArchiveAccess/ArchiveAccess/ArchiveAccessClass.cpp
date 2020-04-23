///////////////////////////////////////////////////////////////////////////////
// ArchiveAccessClass.cpp
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// Licensed under the LGPL
// This file implements a class for accessing archives
///////////////////////////////////////////////////////////////////////////////

#include "windows.h"

#include "Common/Defs.h"
#include "ArchiveAccess.h"
#include "ArchiveAccessClass.h"
#include "aaCallbackOutStream.h"
#include "bgstr.h"
#include "../CPP/Windows/PropVariantConversions.h"

#ifdef ZipSupport
#include "7Zip/Archive/Zip/ZipHandler.h"
#endif

#ifdef RarSupport
#include "7Zip/Archive/Rar/RarHandler.h"
#endif

#ifdef RpmSupport
#include "7Zip/Archive/Rpm/RpmHandler.h"
#endif

#ifdef DebSupport
#include "7Zip/Archive/Deb/DebHandler.h"
#endif

#ifdef TarSupport
#include "7Zip/Archive/Tar/TarHandler.h"
#endif

#ifdef _7ZipSupport
#include "7Zip/Archive/7Z/7ZHandler.h"
#endif

#ifdef GZSupport
//#include "7Zip/Archive/GzHandler.h"
#endif

#ifdef BZip2Support
//#include "7Zip/Archive/bzip2/bZip2Handler.h"
#endif

#ifdef CabSupport
#include "7Zip/Archive/cab/CabHandler.h"
#endif

#ifdef ArjSupport
//#include "7Zip/Archive/arj/ArjHandler.h"
#endif

#ifdef CPIOSupport
#include "7Zip/Archive/cpio/CPIOHandler.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// ArchiveAccessClass constructor: Access file using a file system
// Returns an integer handle to the file (casted class pointer)
// 0 indicates failure

/*ArchiveAccessClass::ArchiveAccessClass (char* fileName)
{
    error = 0;
}*/

///////////////////////////////////////////////////////////////////////////////
// Class definition copied from CPP\7zip\Archive\ArjHandler.cpp

#ifdef ArjSupport
namespace NArchive {
namespace NArj {

const int kBlockSizeMax = 2600;

namespace NFileHeader
{
  namespace NCompressionMethod
  {
    enum
    {
      kStored = 0,
      kCompressed1a = 1,
      kCompressed1b = 2,
      kCompressed1c = 3,
      kCompressed2 = 4,
      kNoDataNoCRC = 8,
      kNoData = 9
    };
  }

  namespace NFileType
  {
    enum
    {
      kBinary = 0,
      k7BitText = 1,
      kArchiveHeader = 2,
      kDirectory = 3,
      kVolumeLablel = 4,
      kChapterLabel = 5
    };
  }
  
  namespace NFlags
  {
    const Byte kGarbled = 1;
    const Byte kVolume = 4;
    const Byte kExtFile = 8;
    const Byte kPathSym = 0x10;
    const Byte kBackup = 0x20;
  }

  namespace NHostOS
  {
    enum EEnum
    {
      kMSDOS = 0,  // filesystem used by MS-DOS, OS/2, Win32
          // pkarj 2.50 (FAT / VFAT / FAT32 file systems)
      kPRIMOS,
      kUnix,
      kAMIGA,
      kMac,
      kOS_2,
      kAPPLE_GS,
      kAtari_ST,
      kNext,
      kVAX_VMS,
      kWIN95
    };
  }
}

struct CItem
{
  AString Name;
  AString Comment;

  UInt32 MTime;
  UInt32 PackSize;
  UInt32 Size;
  UInt32 FileCRC;
  UInt32 SplitPos;

  Byte Version;
  Byte ExtractVersion;
  Byte HostOS;
  Byte Flags;
  Byte Method;
  Byte FileType;

  // UInt16 FilespecPositionInFilename;
  UInt16 FileAccessMode;
  // Byte FirstChapter;
  // Byte LastChapter;
  
  UInt64 DataPosition;
  
  bool IsEncrypted() const { return (Flags & NFileHeader::NFlags::kGarbled) != 0; }
  bool IsDir() const { return (FileType == NFileHeader::NFileType::kDirectory); }
  bool IsSplitAfter() const { return (Flags & NFileHeader::NFlags::kVolume) != 0; }
  bool IsSplitBefore() const { return (Flags & NFileHeader::NFlags::kExtFile) != 0; }
  UInt32 GetWinAttributes() const
  {
    UInt32 winAtrributes;
    switch(HostOS)
    {
      case NFileHeader::NHostOS::kMSDOS:
      case NFileHeader::NHostOS::kWIN95:
        winAtrributes = FileAccessMode;
        break;
      default:
        winAtrributes = 0;
    }
    if (IsDir())
      winAtrributes |= FILE_ATTRIBUTE_DIRECTORY;
    return winAtrributes;
  }

  HRESULT Parse(const Byte *p, unsigned size);
};


struct CArchiveHeader
{
  // Byte ArchiverVersion;
  // Byte ExtractVersion;
  Byte HostOS;
  // Byte Flags;
  // Byte SecuryVersion;
  // Byte FileType;
  // Byte Reserved;
  UInt32 CTime;
  UInt32 MTime;
  UInt32 ArchiveSize;
  // UInt32 SecurityEnvelopeFilePosition;
  // UInt16 FilespecPositionInFilename;
  // UInt16 LengthOfSecurityEnvelopeSata;
  // Byte EncryptionVersion;
  // Byte LastChapter;
  AString Name;
  AString Comment;
  
  HRESULT Parse(const Byte *p, unsigned size);
};

class CInArchive
{
  UInt32 _blockSize;
  Byte _block[kBlockSizeMax + 4];
  
  HRESULT ReadBlock(bool &filled);
  HRESULT ReadSignatureAndBlock(bool &filled);
  HRESULT SkipExtendedHeaders();

  HRESULT SafeReadBytes(void *data, UInt32 size);
    
public:
  CArchiveHeader Header;

  IInStream *Stream;
  IArchiveOpenCallback *Callback;
  UInt64 NumFiles;
  UInt64 NumBytes;

  HRESULT Open(const UInt64 *searchHeaderSizeLimit);
  HRESULT GetNextItem(bool &filled, CItem &item);
};

class CHandler:
  public IInArchive,
  public CMyUnknownImp
{
public:
  MY_UNKNOWN_IMP1(IInArchive)

  INTERFACE_IInArchive(;)

  HRESULT Open2(IInStream *inStream, const UInt64 *maxCheckStartPosition,
      IArchiveOpenCallback *callback);
private:
  CInArchive _archive;
  CObjectVector<CItem> _items;
  CMyComPtr<IInStream> _stream;
};

}
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Class definition copied from CPP\7zip\Archive\GzHandler.cpp

#ifdef GZSupport

#include "..\CPP\7zip\Compress\DeflateDecoder.h"
//#include "../Compress/DeflateDecoder.h"
#include "..\CPP\7zip\Archive\DeflateProps.h"

namespace NArchive {
namespace NGz {

namespace NHeader
{
  namespace NFlags
  {
    const Byte kIsText = 1 << 0;
    const Byte kCrc = 1 << 1;
    const Byte kExtra = 1 << 2;
    const Byte kName = 1 << 3;
    const Byte kComment = 1 << 4;
  }
  
  namespace NExtraFlags
  {
    const Byte kMaximum = 2;
    const Byte kFastest = 4;
  }
  
  namespace NCompressionMethod
  {
    const Byte kDeflate = 8;
  }

  namespace NHostOS
  {
    enum EEnum
    {
      kFAT = 0,
      kAMIGA,
      kVMS,
      kUnix,
      kVM_CMS,
      kAtari,
      kHPFS,
      kMac,
      kZ_System,
      kCPM,
      kTOPS20,
      kNTFS,
      kQDOS,
      kAcorn,
      kVFAT,
      kMVS,
      kBeOS,
      kTandem,
      
      kUnknown = 255
    };
  }
}

static const char *kHostOSes[] =
{
  "FAT",
  "AMIGA",
  "VMS",
  "Unix",
  "VM/CMS",
  "Atari",
  "HPFS",
  "Macintosh",
  "Z-System",
  "CP/M",
  "TOPS-20",
  "NTFS",
  "SMS/QDOS",
  "Acorn",
  "VFAT",
  "MVS",
  "BeOS",
  "Tandem",
  "OS/400",
  "OS/X"
};

static const char *kUnknownOS = "Unknown";

class CItem
{
  bool TestFlag(Byte flag) const { return (Flags & flag) != 0; }
public:
  Byte Method;
  Byte Flags;
  Byte ExtraFlags;
  Byte HostOS;
  UInt32 Time;
  UInt32 Crc;
  UInt32 Size32;

  AString Name;
  AString Comment;
  // CByteBuffer Extra;

  // bool IsText() const { return TestFlag(NHeader::NFlags::kIsText); }
  bool HeaderCrcIsPresent() const { return TestFlag(NHeader::NFlags::kCrc); }
  bool ExtraFieldIsPresent() const { return TestFlag(NHeader::NFlags::kExtra); }
  bool NameIsPresent() const { return TestFlag(NHeader::NFlags::kName); }
  bool CommentIsPresent() const { return TestFlag(NHeader::NFlags::kComment); }

  void Clear()
  {
    Name.Empty();
    Comment.Empty();
    // Extra.SetCapacity(0);
  }

  HRESULT ReadHeader(NCompress::NDeflate::NDecoder::CCOMCoder *stream);
  HRESULT ReadFooter1(NCompress::NDeflate::NDecoder::CCOMCoder *stream);
  HRESULT ReadFooter2(ISequentialInStream *stream);

  HRESULT WriteHeader(ISequentialOutStream *stream);
  HRESULT WriteFooter(ISequentialOutStream *stream);
};

class CHandler:
  public IInArchive,
  public IArchiveOpenSeq,
  public IOutArchive,
  public ISetProperties,
  public CMyUnknownImp
{
  CItem _item;
  UInt64 _startPosition;
  UInt64 _headerSize;
  UInt64 _packSize;
  bool _packSizeDefined;
  CMyComPtr<IInStream> _stream;
  CMyComPtr<ICompressCoder> _decoder;
  NCompress::NDeflate::NDecoder::CCOMCoder *_decoderSpec;

  CDeflateProps _method;

public:
  MY_UNKNOWN_IMP4(IInArchive, IArchiveOpenSeq, IOutArchive, ISetProperties)
  INTERFACE_IInArchive(;)
  INTERFACE_IOutArchive(;)
  STDMETHOD(OpenSeq)(ISequentialInStream *stream);
  STDMETHOD(SetProperties)(const wchar_t **names, const PROPVARIANT *values, Int32 numProps);

  CHandler()
  {
    _decoderSpec = new NCompress::NDeflate::NDecoder::CCOMCoder;
    _decoder = _decoderSpec;
  }
};

}
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Class definition copied from CPP\7zip\Archive\Bz2Handler.cpp

#ifdef BZip2Support
namespace NArchive {
namespace NBz2 {

class CHandler:
  public IInArchive,
  public IArchiveOpenSeq,
  public IOutArchive,
  public ISetProperties,
  public CMyUnknownImp
{
  CMyComPtr<IInStream> _stream;
  CMyComPtr<ISequentialInStream> _seqStream;
  UInt64 _packSize;
  UInt64 _startPosition;
  bool _packSizeDefined;

  UInt32 _level;
  UInt32 _dicSize;
  UInt32 _numPasses;
  #ifndef _7ZIP_ST
  UInt32 _numThreads;
  #endif

  void InitMethodProperties()
  {
    _level = 5;
    _dicSize =
    _numPasses = 0xFFFFFFFF;
    #ifndef _7ZIP_ST
    _numThreads = NWindows::NSystem::GetNumberOfProcessors();;
    #endif
  }

public:
  MY_UNKNOWN_IMP4(IInArchive, IArchiveOpenSeq, IOutArchive, ISetProperties)

  INTERFACE_IInArchive(;)
  INTERFACE_IOutArchive(;)
  STDMETHOD(OpenSeq)(ISequentialInStream *stream);
  STDMETHOD(SetProperties)(const wchar_t **names, const PROPVARIANT *values, Int32 numProps);

  CHandler() { InitMethodProperties(); }
};

}
}
#endif

///////////////////////////////////////////////////////////////////////////////
// ArchiveAccessClass constructor: Access file using blockwise transfer, for 
// reading from stdin or writing to stdout
// Returns an integer handle to the file (casted class pointer)
// 0 indicates failure
// function is a callback to a read function
// StreamID is used to identify this data stream to function

// incomplete: variable create not used
// create should be replaced by an enum that can take the following values:
// ReadOnly = 0, ReadWrite = 1, ReadWriteCreateIfInexistant = 2, 
// ReadWriteCreate = 3

ArchiveAccessClass::ArchiveAccessClass (ReadCallback ReadFunction, aaHandle StreamHandle, 
                                        INT64 _FileSize, int ArchiveType, 
                                        int* result,
                                        bool CloseStreamAfterDestruction,
                                        wchar_t* _password)
   : inStream (ReadFunction, StreamHandle, _FileSize, CloseStreamAfterDestruction)
{
   switch (ArchiveType) {
      #ifdef _7ZipSupport
         case ArchiveFormat7Zip:
             archive = new NArchive::N7z::CHandler ();
             break;
      #endif;
      #ifdef ZipSupport
         case ArchiveFormatZip: 
             archive = new NArchive::NZip::CHandler ();
             break;
      #endif
      #ifdef RarSupport
         case ArchiveFormatRAR:
            archive = new NArchive::NRar::CHandler ();
            break;
      #endif
      #ifdef RpmSupport
         case ArchiveFormatRPM:
            archive = new NArchive::NRpm::CHandler ();
            break;
      #endif
      #ifdef DebSupport
         case ArchiveFormatDeb:
            archive = new NArchive::NDeb::CHandler ();
            break;
      #endif
      #ifdef TarSupport
         case ArchiveFormatTar:
             archive = new NArchive::NTar::CHandler ();
            break;
      #endif
      #ifdef GZSupport
         case ArchiveFormatGZ:
             archive = new NArchive::NGz::CHandler ();
             break;
      #endif
      #ifdef BZip2Support
         case ArchiveFormatBZip2:
             archive = new NArchive::NBz2::CHandler ();
             break;
      #endif
      #ifdef CabSupport
         case ArchiveFormatCab:
             archive = new NArchive::NCab::CHandler ();
             break;
      #endif
      #ifdef ArjSupport
         case ArchiveFormatArj:
             archive = new NArchive::NArj::CHandler ();
             break;
      #endif
      #ifdef CPIOSupport
         case ArchiveFormatCPIO:
             archive = new NArchive::NCpio::CHandler ();
             break;
      #endif
         default:
             archive = NULL;
             break;   
   }

   // Open archive using inStream
   // The callback functions in the Open, Extract methods of the archive classes 
   // exist merely to signal progress information. These callback functions may 
   // be set to NULL. 
   int OpenResult = -1;
   if (archive != NULL) {
      archive->AddRef();
      OpenResult = archive->Open (reinterpret_cast <IInStream*> (&inStream), 
                                  NULL, NULL);
   }
   if (result != NULL) {
      (* result) = OpenResult;
   }

   // Make deep copy of _password (may be deleted by caller)
   if (_password != NULL) {
      int len = bgstrlen (_password, 1024) + 1;
      password = new wchar_t [len+1];
      bgstrlcpy (password, _password, len);
   } else
      password = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor

ArchiveAccessClass::~ArchiveAccessClass ()
{
	if (archive != NULL) {
       archive->Close ();
	   archive->Release();
       //delete archive; // ???
	}
   if (password != NULL)
      delete password;
}

///////////////////////////////////////////////////////////////////////////////
// Create new archive using a file system
// Returns an integer handle to the file (casted class pointer)

/*int ArchiveAccessClass::createArchive (char* archiveName, int ArchiveType)
{
    return 0;
}*/

///////////////////////////////////////////////////////////////////////////////
// Create new archive using a file system
// Returns an integer handle to the file (casted class pointer)
// function is a callback to a write function
// StreamID is used to identify this data stream to function

/*int ArchiveAccessClass::createArchive (ReadWriteCallBack* function, int StreamID)
{
    return 0;
}*/

///////////////////////////////////////////////////////////////////////////////
// Get number of files in archive

int ArchiveAccessClass::getFileCount ()
{
   if (archive != NULL)
   {
       unsigned int result;
       archive->GetNumberOfItems(&result);
       return result;
   } else
       return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Get information on a file in the archive
// Unfortunately, not all archive formats store full information on the files 
// contained in them. If any data is missing, the function result will be set
// to S_FALSE to indicate this. In particular,
// - the file name may be missing. In this case, a valid file name may often
//   be built by truncating one extension (.bz2, .gz) from the archive file 
//   name or by appending .unpacked
// - the file size may be reported as 0 (bzip). In this case, the file size
//   can be obtained by unpacking the file to disk and reading the file size
//   from the unpacked file

int ArchiveAccessClass::getFileInfo (unsigned int FileNum, FileInArchiveInfo* FileInfo)
{
    tagPROPVARIANT prop;
    prop.iVal = 0;
    prop.vt = VT_EMPTY;
    
    if (archive != NULL)
    {
        int result = S_OK;
        HRESULT GetPropRes;

        // Get path and file name
   ///ss     FileInfo->path [0] = 0; //fixes BZ2
        GetPropRes = archive->GetProperty(FileNum, kpidPath, &prop);
        if (GetPropRes == S_OK && prop.vt == VT_BSTR) {
            //bgstrlcpy (FileInfo->path, prop.bstrVal, FileInArchiveInfoStringSize);
			UString ustr = ConvertPropVariantToString (prop);
			const TCHAR* FNchars = ustr;
            //bgstrlcpy (FileInfo->path, FNchars, FileInArchiveInfoStringSize);
			size_t len = ustr.Length();
			if (2*len+2 >= sizeof(FileInfo->path)) {
				len = (sizeof(FileInfo->path) >> 1)-2;
			}
            //memset (FileInfo->path, 0, sizeof(FileInfo->path));
			memcpy (FileInfo->path, FNchars, 2*len);
			FileInfo->path[len] = 0;
			//memset (&(FileInfo->path[2*len]), 0, sizeof(FileInfo->path)-2*len-2);
        } else {
            result = S_FALSE;
        }

        // Get archive handle
        FileInfo->ArchiveHandle = (int) (this);

        // Get file attributes
        GetPropRes = archive->GetProperty(FileNum, kpidAttrib, &prop);
        if (GetPropRes == S_OK && prop.vt != VT_EMPTY) {
            FileInfo->attributes = prop.ulVal;
        } else {
            FileInfo->attributes = 0;
            result = S_FALSE;
        }

        // Get compressed file size
        GetPropRes = archive->GetProperty(FileNum, kpidPackSize, &prop);
        if (GetPropRes == S_OK && prop.vt != VT_EMPTY) {
           FileInfo->CompressedFileSize = ConvertPropVariantToUInt64(prop);
        } else {
            FileInfo->CompressedFileSize = -1;
            result = S_FALSE;
        }

        // Get uncompressed file size
        GetPropRes = archive->GetProperty(FileNum, kpidSize, &prop);
        if (GetPropRes == S_OK && prop.vt != VT_EMPTY) {
            FileInfo->UncompressedFileSize = ConvertPropVariantToUInt64(prop);
        } else {
            FileInfo->UncompressedFileSize = -1;
            result = S_FALSE;
        }

        // Check if entry is file or directory
        GetPropRes = archive->GetProperty(FileNum, kpidIsDir, &prop);
        if (GetPropRes == S_OK && prop.vt != VT_EMPTY) {
            FileInfo->IsDir = (prop.boolVal != 0);
        } else {
            FileInfo->IsDir = false;
            result = S_FALSE;
        }

        // Check if entry is encrypted
        GetPropRes = archive->GetProperty(FileNum, kpidEncrypted, &prop);
        if (GetPropRes == S_OK && prop.vt != VT_EMPTY) {
            FileInfo->IsEncrypted = (prop.boolVal != 0);
        } else {
            FileInfo->IsEncrypted = false;
            //result = S_FALSE;
        }

        // Get last write/modification time
        GetPropRes = archive->GetProperty(FileNum, kpidMTime, &prop);
        if (GetPropRes == S_OK && prop.vt != VT_EMPTY) {
            FileInfo->LastWriteTime = prop.filetime;
        } else {
            FileInfo->LastWriteTime.dwLowDateTime = 0;
            FileInfo->LastWriteTime.dwHighDateTime = 0;
            result = S_FALSE;
        }

		// Get last access time
        GetPropRes = archive->GetProperty(FileNum, kpidATime, &prop);
        if (GetPropRes == S_OK && prop.vt != VT_EMPTY) {
            FileInfo->LastAccessTime = prop.filetime;
        } else {
            FileInfo->LastAccessTime.dwLowDateTime = 0;
            FileInfo->LastAccessTime.dwHighDateTime = 0;
            result = S_FALSE;
        }

		// Get creation time
        GetPropRes = archive->GetProperty(FileNum, kpidCTime, &prop);
        if (GetPropRes == S_OK && prop.vt != VT_EMPTY) {
            FileInfo->CreationTime = prop.filetime;
        } else {
            FileInfo->CreationTime.dwLowDateTime = 0;
            FileInfo->CreationTime.dwHighDateTime = 0;
            result = S_FALSE;
        }
        return result;
    } else
        return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Extract a file from an archive to memory

int ArchiveAccessClass::extractFile (
    unsigned int FileNum, aaHandle oStreamHandle, WriteCallback WriteFunc, 
    UINT64* WrittenSize)
{
    aaCallbackOutStream ExtractionHelper (WriteFunc, oStreamHandle);
    int OpResult = archive->Extract (&FileNum, 1, 0, &ExtractionHelper);
    if (WrittenSize != NULL)
        *WrittenSize = ExtractionHelper.GetExtractedByteCount();
    return OpResult;
}

///////////////////////////////////////////////////////////////////////////////
// Extract multiple files from an archive

void ArchiveAccessClass::extractFiles (int FileCount, unsigned int FileNums [], 
    WriteCallback WriteFunc, StreamRequest RequestStream)
{
    if (FileCount > 0) {
        aaCallbackOutStream ExtractionHelper ((aaHandle) (this), WriteFunc, RequestStream);
        int OpResult = archive->Extract (FileNums, FileCount, 0, &ExtractionHelper);
    }
    //return OpResult;
    //return 0;
}
