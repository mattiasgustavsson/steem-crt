// Virtual Pascal interface unit for UnRAR.dll
//
// Ported to Virtual Pascal by Alexey Fayans, fidonet: 2:5030/1997, e-mail: shad@shad.pp.ru

{$IFDEF Open32}
  {&cdecl+}
{$ELSE}
  {&StdCall+}
{$ENDIF Open32}
{&AlignRec+,SmartLink+,OrgName+,X+,Z+,Use32-}

unit UnRAR;

interface

uses
  Windows;

const
  ERAR_END_ARCHIVE    = 10;
  ERAR_NO_MEMORY      = 11;
  ERAR_BAD_DATA       = 12;
  ERAR_BAD_ARCHIVE    = 13;
  ERAR_UNKNOWN_FORMAT = 14;
  ERAR_EOPEN          = 15;
  ERAR_ECREATE        = 16;
  ERAR_ECLOSE         = 17;
  ERAR_EREAD          = 18;
  ERAR_EWRITE         = 19;
  ERAR_SMALL_BUF      = 20;
  ERAR_UNKNOWN        = 21;

  RAR_OM_LIST         =  0;
  RAR_OM_EXTRACT      =  1;

  RAR_SKIP            =  0;
  RAR_TEST            =  1;
  RAR_EXTRACT         =  2;

  RAR_VOL_ASK         =  0;
  RAR_VOL_NOTIFY      =  1;

  RAR_DLL_VERSION     =  3;

  UCM_CHANGEVOLUME    =  0;
  UCM_PROCESSDATA     =  1;
  UCM_NEEDPASSWORD    =  2;

type
  PRARHeaderData = ^TRARHeaderData;
  TRARHeaderData = packed record
    ArcName: packed array[0..259] of Char;
    FileName: packed array[0..259] of Char;
    Flags: UInt;
    PackSize: UInt;
    UnpSize: UInt;
    HostOS: UInt;
    FileCRC: UInt;
    FileTime: UInt;
    UnpVer: UInt;
    Method: UInt;
    FileAttr: UInt;
    CmtBuf: PChar;
    CmtBufSize: UInt;
    CmtSize: UInt;
    CmtState: UInt;
  end;

  PRARHeaderDataEx = ^TRARHeaderDataEx;
  TRARHeaderDataEx = packed record
    ArcName: packed array [0..1023] of Char;
    ArcNameW: packed array [0..2047] of Char;
    FileName: packed array [0..1023] of Char;
    FileNameW: packed array [0..2047] of Char;
    Flags: UInt;
    PackSize: UInt;
    PackSizeHigh: UInt;
    UnpSize: UInt;
    UnpSizeHigh: UInt;
    HostOS: UInt;
    FileCRC: UInt;
    FileTime: UInt;
    UnpVer: UInt;
    Method: UInt;
    FileAttr: UInt;
    CmtBuf: PChar;
    CmtBufSize: UInt;
    CmtSize: UInt;
    CmtState: UInt;
    Reserved: packed array [0..1023] of UInt;
  end;

  PRAROpenArchiveData = ^TRAROpenArchiveData;
  TRAROpenArchiveData = packed record
    ArcName: PChar;
    OpenMode: UInt;
    OpenResult: UInt;
    CmtBuf: PChar;
    CmtBufSize: UInt;
    CmtSize: UInt;
    CmtState: UInt;
  end;

  PRAROpenArchiveDataEx = ^TRAROpenArchiveDataEx;
  TRAROpenArchiveDataEx = packed record
    ArcName: PChar;
    ArcNameW: PWideChar;
    OpenMode: UInt;
    OpenResult: UInt;
    CmtBuf: PChar;
    CmtBufSize: UInt;
    CmtSize: UInt;
    CmtState: UInt;
    Flags: UInt;
    Reserved: packed array [0..31] of UInt;
  end;

  TUnrarCallback = function(Msg: UInt; UserData, P1, P2: Longint): Int;

function RAROpenArchive(ArchiveData: PRAROpenArchiveData): THandle;
function RAROpenArchiveEx(ArchiveData: PRAROpenArchiveDataEx): THandle;
function RARCloseArchive(hArcData: THandle): Int;
function RARReadHeader(hArcData: THandle; HeaderData: PRARHeaderData): Int;
function RARReadHeaderEx(hArcData: THandle; HeaderData: PRARHeaderDataEx): Int;
function RARProcessFile(hArcData: THandle; Operation: Int; DestPath, DestName: PChar): Int;
function RARProcessFileW(hArcData: THandle; Operation: Int; DestPath, DestName: PWideChar): Int;
procedure RARSetCallback(hArcData: THandle; UnrarCallback: TUnrarCallback; UserData: Longint);
procedure RARSetPassword(hArcData: THandle; Password: PChar);

// obsolete functions
type
  TChangeVolProc = function(ArcName: PChar; Mode: Int): Int;
  TProcessDataProc = function(Addr: PUChar; Size: Int): Int;

procedure RARSetChangeVolProc(hArcData: THandle; ChangeVolProc: TChangeVolProc);
procedure RARSetProcessDataProc(hArcData: THandle; ProcessDataProc: TProcessDataProc);

implementation

{$L UnRAR.LIB}

function RAROpenArchive;           external;
function RAROpenArchiveEx;         external;
function RARCloseArchive;          external;
function RARReadHeader;            external;
function RARReadHeaderEx;          external;
function RARProcessFile;           external;
function RARProcessFileW;          external;
procedure RARSetCallback;          external;
procedure RARSetPassword;          external;
procedure RARSetChangeVolProc;     external;
procedure RARSetProcessDataProc;   external;

end.
