///////////////////////////////////////////////////////////////////////////////
// ArchiveAccess
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// This file declares access functions to file archives
///////////////////////////////////////////////////////////////////////////////

unit ArchiveAccess;

interface

// Use SysUtils if you don't have Strings
//uses SysUtils, windows;
uses Strings, windows;

///////////////////////////////////////////////////////////////////////////////
// The following define controls whether ArchiveAccess is loaded on program
// start or at a later time
{ $Define LoadDllAtPrgStart}
{$define debug}

const
   //ArchiveAccessDllName = 'ArchiveAccess-debug.dll';
   ArchiveAccessDllName = '../ArchiveAccess-debug.dll';
   //ArchiveAccessDllName = 'ArchiveAccess.dll';
   //ArchiveAccessDllName = 'zip.dll';
   //ArchiveAccessDllName: array[0..7] of Char = 'zip.dll';

///////////////////////////////////////////////////////////////////////////////
// Archive Format Id's: interpret extension string as 32-bit integer ...
const ArchiveFormatUnknown = 0;
const ArchiveFormat7Zip    = $377a6970;
const ArchiveFormatZip     = $7a697020;
const ArchiveFormatRAR     = $72617220;
const ArchiveFormatRPM     = $72706d20;
const ArchiveFormatDeb     = $64656220;
const ArchiveFormatTar     = $74617220;
const ArchiveFormatGZ      = $677a2020;
const ArchiveFormatBZip2   = $627a3220; // 'bz2 '
const ArchiveFormatCab     = $63616220;
const ArchiveFormatArj     = $61726a20;
const ArchiveFormatCPIO    = $6370696f;
const ArchiveFormatJar     = $6a617220; //equivalent to .zip format
const ArchiveFormatTGZ     = $74677A20;

// Archive Flags returned when calling determineArchiveType
const ArchiveFlagOrdinary = 0;
const ArchiveFlagEncrypted = 1;
const ArchiveFlagSolid = 2;

// Constants for StreamRequestFuncs
const OpenStreamRequest = 1;
const CloseStreamRequest = 2;

///////////////////////////////////////////////////////////////////////////////
// This struct contains file information from an archive. The caller may store
// this information for accessing this file after calls to findFirst, findNext

const FileInArchiveInfoStringSize = 1024;
type
   TFileInArchiveInfo = {packed} record
      ArchiveHandle: integer; // handle for Archive/class pointer
      dummy: integer; // needed for 8-byte alignment!
      CompressedFileSize: int64;
      UncompressedFileSize: int64;
      attributes: integer;
      IsDir, IsEncrypted: boolean;
	   LastWriteTime: FileTime;
      path: array[0..FileInArchiveInfoStringSize] of char; // Pointer to Unicode characters, wchar_t?
   end;
   pFileInArchiveInfo = ^TFileInArchiveInfo;

////////////////////////////////////////////////////////////////////////////////
// Callback functions declaration

type
   aaHandle = PChar;
   ReadCallbackFunc =
			function (StreamID: integer; offset: int64; count: Dword; buf: PChar;
								processedSize: pDWord): integer; stdcall;

   WriteCallbackFunc =
			function (StreamID: integer; offset: int64; count: DWord; buf: PChar;
                processedSize: pDWord): integer; stdcall;

   StreamRequestFunc =
			function (ArchiveHandle: aaHandle; index, RequestedAction: integer): integer;
      stdcall;

////////////////////////////////////////////////////////////////////////////////
// Determine archive type from file without knowing its extension
// Returns one of the archive types in ArchiveAccessDefs.h

{$ifdef LoadDllAtPrgStart}
function determineArchiveType (rwCallBack: ReadCallbackFunc; StreamID: integer;
                               ArchiveSize: comp; var flags: integer): integer;
stdcall; external ArchiveAccessDllName;
{$else}
type TDetermineArchiveType =
function (rwCallBack: ReadCallbackFunc; StreamID: integer;
          ArchiveSize: comp; var flags: integer):
   integer; stdcall;
var DetermineArchiveType: TDetermineArchiveType;
{$endif}

////////////////////////////////////////////////////////////////////////////////
// Access file using a file system
// Returns an integer handle to the file (casted class pointer)
// 0 indicates failure

{$ifdef LoadDllAtPrgStart}
function openArchive (rwCallBack: ReadCallbackFunc; StreamID: integer;
							 ArchiveSize: comp; ArchiveType: integer;
							 var result: integer; Password: PChar {WideString}): aaHandle;
stdcall; external ArchiveAccessDllName;
{$else}
type TOpenArchive =
	 function (rwCallBack: ReadCallbackFunc; StreamID: integer;
              ArchiveSize: comp; ArchiveType: integer;
							var result: integer; Password: PChar {WideString}): aaHandle;
    stdcall;
var OpenArchive: TOpenArchive;
{$endif}


////////////////////////////////////////////////////////////////////////////////
// Set/get owner for an archive handle
// Useful for determining the owner object of an archive from its handle
// in a callback function

{$ifdef LoadDllAtPrgStart}
function setOwner (ArchiveHandle, newOwner: aaHandle): boolean;
stdcall; external ArchiveAccessDllName;
{$else}
type TSetOwner =
	 function (ArchiveHandle, newOwner: aaHandle): boolean; stdcall;
var setOwner: TSetOwner;
{$endif}

{$ifdef LoadDllAtPrgStart}
function getOwner (ArchiveHandle: aaHandle): boolean;
stdcall; external ArchiveAccessDllName;
{$else}
type TGetOwner =
	 function (ArchiveHandle: aaHandle): aaHandle; stdcall;
var getOwner: TGetOwner;
{$endif}


////////////////////////////////////////////////////////////////////////////////
// Get number of files in archive

{$ifdef LoadDllAtPrgStart}
function getFileCount (ArchiveHandle: aaHandle): integer;
stdcall; external ArchiveAccessDllName;
{$else}
type TGetFileCount =
	 function (ArchiveHandle: aaHandle): integer; stdcall;
var GetFileCount: TGetFileCount;
{$endif}

////////////////////////////////////////////////////////////////////////////////
// Get information on a file in the archive
// FileInfo is a pointer to a record that is filled by the procedure, but
// created by the caller

{$ifdef LoadDllAtPrgStart}
function getFileInfo (ArchiveHandle: aaHandle; FileNum: integer;
                      FileInfo: pFileInArchiveInfo): integer;
stdcall; external ArchiveAccessDllName;
{$else}
type TGetFileInfo =
	 function (ArchiveHandle: aaHandle; FileNum: integer;
              FileInfo: pFileInArchiveInfo): integer; stdcall;
var GetFileInfo: TGetFileInfo;
{$endif}

////////////////////////////////////////////////////////////////////////////////
// Extract bytes from an archive to memory

{$ifdef LoadDllAtPrgStart}

function extract (ArchiveHandle: aaHandle; FileNum, StreamID: integer;
   WriteCallback: WriteCallbackFunc; var WrittenSize: comp): integer;
stdcall; external ArchiveAccessDllName;
{$else}
type TExtract =
	 function (ArchiveHandle: aaHandle; FileNum, StreamID: integer;
              WriteCallback: WriteCallbackFunc; var WrittenSize: comp): integer;
	 stdcall;
var Extract: TExtract;
{$endif}


////////////////////////////////////////////////////////////////////////////////
// Extract multiple files from an archive

{$ifdef LoadDllAtPrgStart}
procedure extractMultiple (ArchiveHandle: aaHandle; FileCount: integer;
	FileNums: pWord; WriteFunc: WriteCallbackFunc; RequestStream: StreamRequestFunc);
stdcall; external ArchiveAccessDllName;
{$else}
type TExtractMultiple =
   procedure (ArchiveHandle: aaHandle; FileCount: integer;
						 FileNums: pWord; WriteFunc: WriteCallbackFunc;
              RequestStream: StreamRequestFunc);
stdcall;
var ExtractMultiple: TExtractMultiple;
{$endif}

////////////////////////////////////////////////////////////////////////////////
// Extract all files from an archive

{$ifdef LoadDllAtPrgStart}
procedure extractAll (ArchiveHandle: aaHandle;
	WriteFunc: WriteCallbackFunc; RequestStream: StreamRequestFunc);
stdcall; external ArchiveAccessDllName;
{$else}
type TExtractAll =
   procedure (ArchiveHandle: aaHandle; WriteFunc: WriteCallbackFunc;
              RequestStream: StreamRequestFunc);
stdcall;
var ExtractAll: TExtractAll;
{$endif}

////////////////////////////////////////////////////////////////////////////////
// Close archive
// All FileInArchiveInfo's are invalid after calling this function

{$ifdef LoadDllAtPrgStart}
function closeArchive (ArchiveHandle: integer): integer;
stdcall; external ArchiveAccessDllName;
{$else}
type TCloseArchive = function  (ArchiveHandle: aaHandle): integer; stdcall;
var CloseArchive: TCloseArchive;
{$endif}

////////////////////////////////////////////////////////////////////////////////
// Create new archive using a file system

{$ifdef LoadDllAtPrgStart}
function CreateArchiveFile (ArchiveFileName: PChar; ArchiveType: integer;
														Password: PChar {WideString}): aaHandle;
stdcall; external ArchiveAccessDllName;
{$else}
type TCreateArchiveFile = function (ArchiveFileName: PChar;
	 ArchiveType: integer; Password: PChar {WideString}): aaHandle; stdcall;
var CreateArchiveFile: TCreateArchiveFile;
{$endif}

////////////////////////////////////////////////////////////////////////////////
// Add file to new archive

{$ifdef LoadDllAtPrgStart}
function AddFromStream (FileInfo: pFileInArchiveInfo;
	 ReadFunc: ReadCallbackFunc; iStreamID: integer): integer;
stdcall; external ArchiveAccessDllName;
{$else}
type TAddFromStream = function  (FileInfo: pFileInArchiveInfo;
	 ReadFunc: ReadCallbackFunc; iStreamID: integer): integer; stdcall;
var AddFromStream: TAddFromStream;
{$endif}

////////////////////////////////////////////////////////////////////////////////
// Add file to new archive, store path names relative to current directory

{$ifdef LoadDllAtPrgStart}
function AddFile (ArchiveHandle aaHandle; FileName: PChar): integer;
stdcall; external ArchiveAccessDllName;
{$else}
type TAddFile = function  (ArchiveHandle: aaHandle; FileName: PChar): integer;
stdcall;
var AddFile: TAddFile;
{$endif}

////////////////////////////////////////////////////////////////////////////////
// Load or unload ArchiveAccess dll

function LoadArchiveAccessDll (FileName: string): boolean;
procedure UnloadArchiveAccessDll;
function StrAsInt (str: PChar): integer;
function IsExtensionSupportedByArchiveAccess(Ext: PChar): Boolean;
function GetArchiveExtension (ArchiveType: integer): string;

implementation

var
	ArchiveAccessDLL: THandle;

////////////////////////////////////////////////////////////////////////////////
// Get archive type string

function GetArchiveExtension (ArchiveType: integer): string;
var i: integer;
    c: byte;
begin
   for i := 1 to 4 do begin
      c := (ArchiveType shr (i*8)) and 255;
      if (c>32) and (c < 128) then
         result := char (c) + result;
   end;
end;

////////////////////////////////////////////////////////////////////////////////
// Load dll from FileName

function LoadArchiveAccessDll (FileName: string): boolean;
var ErrorStr: string;

   function LoadOrAddError (FuncName: PChar): pointer;
   var p: pointer;
   begin
      p := GetProcAddress (ArchiveAccessDLL, FuncName);
      if p = nil then
         ErrorStr := ErrorStr + FuncName + ', ';
      LoadOrAddError := p;
   end;

begin
   {$ifndef LoadDllAtPrgStart}
	 if ArchiveAccessDLL = 0 then begin
      //if FileExists (FileName) then
			ArchiveAccessDLL := LoadLibrary (PChar (FileName));
      if (ArchiveAccessDLL <> 0) then begin

         // Function names are case-sensitive!
         @DetermineArchiveType := LoadOrAddError ('aaDetermineArchiveType');
         @OpenArchive := LoadOrAddError ('aaOpenArchive');
				 @SetOwner := LoadOrAddError ('aaSetOwner');
				 @GetOwner := LoadOrAddError ('aaGetOwner');
				 @GetFileCount := LoadOrAddError ('aaGetFileCount');
         @getFileInfo := LoadOrAddError ('aaGetFileInfo');
         @Extract := LoadOrAddError ('aaExtract');
         @ExtractAll := LoadOrAddError ('aaExtractAll');
         @ExtractMultiple := LoadOrAddError ('aaExtractMultiple');
				 @CreateArchiveFile := LoadOrAddError ('aaCreateArchiveFile');
         @AddFile := LoadOrAddError ('aaAddFile');
         @AddFromStream := LoadOrAddError ('aaAddFromStream');
         @CloseArchive := LoadOrAddError ('aaCloseArchive');

         if ErrorStr <> '' then begin
            {$ifdef DEBUG}
            MessageBox (0, PChar ('Missing Archive Access Dll functions: ' +
                        ErrorStr),
                        PChar ('Error Loading Dll'), 0);
            {$endif}
            LoadArchiveAccessDll := false;
         end else
            LoadArchiveAccessDll := true;
      end else
				LoadArchiveAccessDll := false;
   end else
      LoadArchiveAccessDll := true;
   {$else}
   LoadArchiveAccessDll := (FileName = ArchiveAccessDllName);
   {$endif}
end;

////////////////////////////////////////////////////////////////////////////////
// Set the dll to unloaded status; users of ArchiveAccess usually do not have
// to call this function. This function does nothing if dll is loaded at
// program start

procedure SetUnloaded;
begin
   {$ifndef LoadDllAtPrgStart}
   ArchiveAccessDLL := 0;
   OpenArchive := nil;
   GetFileCount := nil;
   getFileInfo := nil;
   Extract := nil;
   ExtractAll := nil;
   ExtractMultiple := nil;
   AddFile := nil;
   CreateArchiveFile := nil;
   AddFile := nil;
   AddFromStream := nil;
   {$endif}
end;

////////////////////////////////////////////////////////////////////////////////
// Remove ArchiveAccess from memory
// This function does nothing if dll is loaded at program start

procedure UnloadArchiveAccessDll;
begin
	 {$ifndef LoadDllAtPrgStart}
   FreeLibrary (ArchiveAccessDll);
   SetUnloaded;
   {$endif}
end;

function StrAsInt (str: PChar): integer;
var
	_4DigitFNExt: array[0..3] of Char;
	i, len: Integer;
begin
	for i := 0 to 3 do
		 _4DigitFNExt[i]:=' ';
   len := strlen(str);
   if len > 4 then
      len := 4;
	CopyMemory(@_4DigitFNExt, str, len);

	CharLowerBuff(_4DigitFNExt, 4);

	 StrAsInt := (ord(_4DigitFNExt[0]) shl 24) + (ord(_4DigitFNExt[1]) shl 16) +
							 (ord(_4DigitFNExt[2]) shl 8) + (ord(_4DigitFNExt[3]) shl 0);
end;

function IsExtensionSupportedByArchiveAccess(Ext: PChar): Boolean;
var
	ExtensionInt: Integer;
begin
	ExtensionInt:=StrAsInt(Ext);

	Result:=
		(ExtensionInt=ArchiveFormatZip) or
		(ExtensionInt=ArchiveFormatJar) or
		(ExtensionInt=ArchiveFormatRAR) or
		(ExtensionInt=ArchiveFormatTar) or
		(ExtensionInt=ArchiveFormatGZ) or
		(ExtensionInt=ArchiveFormatTGZ) or
		(ExtensionInt=ArchiveFormatBZip2) or
		(ExtensionInt=ArchiveFormat7Zip) or
		(ExtensionInt=ArchiveFormatArj);
end;

////////////////////////////////////////////////////////////////////////////////
// Initialize Archive access

begin
   SetUnloaded;
end.