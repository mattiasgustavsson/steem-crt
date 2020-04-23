// Free Pascal interface unit for UnRAR.dll
//
// Ported to Free Pascal by Alexey Fayans, fidonet: 2:5030/1997, e-mail: shad@shad.pp.ru

{$IFNDEF NO_SMART_LINK}
{$SMARTLINK ON}
{$ENDIF}
{$MODE objfpc}
{$CALLING stdcall}
{$PACKRECORDS C}

unit UnRAR;

interface

uses Windows;

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
  TRARHeaderData = record
    ArcName: array[0..259] of Char;
    FileName: array[0..259] of Char;
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
  TRARHeaderDataEx = record
    ArcName: array [0..1023] of Char;
    ArcNameW: array [0..1023] of WideChar;
    FileName: array [0..1023] of Char;
    FileNameW: array [0..1023] of WideChar;
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
    Reserved: array [0..1023] of UInt;
  end;

  PRAROpenArchiveData = ^TRAROpenArchiveData;
  TRAROpenArchiveData = record
    ArcName: PChar;
    OpenMode: UInt;
    OpenResult: UInt;
    CmtBuf: PChar;
    CmtBufSize: UInt;
    CmtSize: UInt;
    CmtState: UInt;
  end;

  PRAROpenArchiveDataEx = ^TRAROpenArchiveDataEx;
  TRAROpenArchiveDataEx = record
    ArcName: PChar;
    ArcNameW: PWideChar;
    OpenMode: UInt;
    OpenResult: UInt;
    CmtBuf: PChar;
    CmtBufSize: UInt;
    CmtSize: UInt;
    CmtState: UInt;
    Flags: UInt;
    Reserved: array [0..31] of UInt;
  end;

  TUnrarCallback = function(Msg: UInt; UserData, P1, P2: Longint): Integer;

function RAROpenArchive(ArchiveData: PRAROpenArchiveData): THandle;
function RAROpenArchiveEx(ArchiveData: PRAROpenArchiveDataEx): THandle;
function RARCloseArchive(hArcData: THandle): Integer;
function RARReadHeader(hArcData: THandle; HeaderData: PRARHeaderData): Integer;
function RARReadHeaderEx(hArcData: THandle; HeaderData: PRARHeaderDataEx): Integer;
function RARProcessFile(hArcData: THandle; Operation: Integer; DestPath, DestName: PChar): Integer;
function RARProcessFileW(hArcData: THandle; Operation: Integer; DestPath, DestName: PWideChar): Integer;
procedure RARSetCallback(hArcData: THandle; UnrarCallback: TUnrarCallback; UserData: Longint);
procedure RARSetPassword(hArcData: THandle; Password: PChar);

// obsolete functions
type
  TChangeVolProc = function(ArcName: PChar; Mode: Integer): Integer;
  TProcessDataProc = function(Addr: PUChar; Size: Integer): Integer;

procedure RARSetChangeVolProc(hArcData: THandle; ChangeVolProc: TChangeVolProc);
procedure RARSetProcessDataProc(hArcData: THandle; ProcessDataProc: TProcessDataProc);

implementation

function RAROpenArchive(ArchiveData: PRAROpenArchiveData): THandle;
  external 'unrar.dll' name 'RAROpenArchive';
function RAROpenArchiveEx(ArchiveData: PRAROpenArchiveDataEx): THandle;
  external 'unrar.dll' name 'RAROpenArchiveEx';
function RARCloseArchive(hArcData: THandle): Integer;
  external 'unrar.dll' name 'RARCloseArchive';
function RARReadHeader(hArcData: THandle; HeaderData: PRARHeaderData): Integer;
  external 'unrar.dll' name 'RARReadHeader';
function RARReadHeaderEx(hArcData: THandle; HeaderData: PRARHeaderDataEx): Integer;
  external 'unrar.dll' name 'RARReadHeaderEx';
function RARProcessFile(hArcData: THandle; Operation: Integer; DestPath, DestName: PChar): Integer;
  external 'unrar.dll' name 'RARProcessFile';
function RARProcessFileW(hArcData: THandle; Operation: Integer; DestPath, DestName: PWideChar): Integer;
  external 'unrar.dll' name 'RARProcessFileW';
procedure RARSetCallback(hArcData: THandle; UnrarCallback: TUnrarCallback; UserData: Longint);
  external 'unrar.dll' name 'RARSetCallback';
procedure RARSetPassword(hArcData: THandle; Password: PChar);
  external 'unrar.dll' name 'RARSetPassword';
procedure RARSetChangeVolProc(hArcData: THandle; ChangeVolProc: TChangeVolProc);
  external 'unrar.dll' name 'RARSetChangeVolProc';
procedure RARSetProcessDataProc(hArcData: THandle; ProcessDataProc: TProcessDataProc);
  external 'unrar.dll' name 'RARSetProcessDataProc';

end.
