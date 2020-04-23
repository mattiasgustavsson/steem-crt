///////////////////////////////////////////////////////////////////////////////
// ArchiveExploder
// Copyright 2004 X-Ways Software Technology AG
// Author: Björn Ganster
// This unit serves as an example for the use of ArchiveAccess
///////////////////////////////////////////////////////////////////////////////

unit ArchiveExploderMain;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, Grids, StdCtrls, ArchiveAccess, ComCtrls;

type
  TArchiveExploderForm = class(TForm)
    SelectButton: TButton;
    ArchiveGrid: TStringGrid;
    OpenDialog: TOpenDialog;
    SaveDialog1: TSaveDialog;
    ExtractAllButton: TButton;
    logMemo: TMemo;
    Label1: TLabel;
    PasswordEdit: TEdit;
    procedure FormResize(Sender: TObject);
    procedure SelectButtonClick(Sender: TObject);
    procedure ArchiveGridDblClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure AddObject(FileInfo: pFileInArchiveInfo; row: integer);
    procedure ExtractAllButtonClick(Sender: TObject);
  private
    { Private declarations }
    FileHandle: integer;
    ArchiveHandle: aaHandle;
  public
    { Public declarations }
    procedure ShowFiles (FileName: string);
  end;

var
  ArchiveExploderForm: TArchiveExploderForm;

const
  WindowTitle = 'ArchiveExploder';

implementation

uses ExtractToSelection, BGStrings;

const DirSepChar = '\';

{$R *.dfm}

var ExtractAllTargetDir: string;

// ============================================================================
// Things to do on window creation

procedure TArchiveExploderForm.FormCreate(Sender: TObject);
begin
   ArchiveGrid.cells[0,0] := 'File Name';
   ArchiveGrid.cells[1,0] := 'Attributes';
   ArchiveGrid.cells[2,0] := 'Compressed';
   ArchiveGrid.Cells[3,0] := 'Uncompressed';
   ArchiveGrid.cells[4,0] := 'Created';

   ArchiveHandle := nil;
   ExtractAllButton.Enabled := false;

   if paramCount >= 1 then
      ShowFiles (paramStr (1));
end;

// ============================================================================
// Resize window

procedure TArchiveExploderForm.FormResize(Sender: TObject);
const margin = 10;
begin
   // Place components
   ArchiveGrid.left := margin;
   ArchiveGrid.Top := SelectButton.Height + 2 * margin;
   ArchiveGrid.Width := ClientWidth - 2*margin;
   //ArchiveGrid.Height := Height - ArchiveGrid.top - 5*margin;
   ArchiveGrid.Height := ClientHeight - ArchiveGrid.top - 2*margin - logmemo.height;

   logMemo.left := margin;
   logMemo.Top := ClientHeight - margin - logMemo.Height;
   logMemo.Width := ArchiveGrid.width;

end;

// ============================================================================
// Create directories need for creation of given file

procedure CreateDirs (path: string);
var FullPath, dir, newDir: string;
    pos: integer;
begin
   FullPath := ExpandFileName (path);
   dir := ExtractFileDir (FullPath) + DirSepChar;

   pos := 5;
   while pos <= length (dir) do begin
      if dir[pos] = DirSepChar then begin
         NewDir := copy (dir, 1, pos -1);
         CreateDir (newDir);
      end;
      inc (pos);
   end;
end;

// ============================================================================
// Request a write stream

function RequestStream (ArchiveHandle: aaHandle; index, RequestedAction: integer):
   integer; stdcall;
var pOutFileName: PChar;
    FileInfo: TFileInArchiveInfo;
begin
   result := 0;
   if RequestedAction = OpenStreamRequest then begin
      GetFileInfo (ArchiveHandle, index, @FileInfo);
      pOutFileName := PChar (ExtractAllTargetDir + DirSepChar +
                             string (FileInfo.path));
      CreateDirs (string (pOutFileName));
      if not FileInfo.IsDir then
         result := CreateFile (pOutFileName, Generic_write,
                               FILE_SHARE_READ, nil, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, 0);
   end else begin
      CloseHandle (index);
   end;
end;

// ============================================================================
// Callback function called from ArchiveAccess to write to a file

function WriteCallBack (StreamID: integer; offset: int64; count: DWord;
                        buf: PChar; processedSize: pDWord): integer; stdcall;
var written: cardinal;
    lo, hi: DWord;
begin
   lo := offset;
   hi := offset shr 32;
   SetFilePointer (StreamID, lo, @hi, FILE_BEGIN);

   written := _lWrite (StreamID, buf, count);

   if written = HFILE_ERROR then
      WriteCallBack := GetLastError
   else
      WriteCallBack := s_OK;

   if processedSize <> nil then
      processedSize^ := written;
end;

// ============================================================================
// Callback function called from ArchiveAccess that provides read access to
// the archive

function ReadArchiveCallback (StreamID: integer; offset: int64; count: Dword;
                              buf: PChar; processedSize: pDWord): integer;
stdcall;
var ReadBytes: DWord;
    lo, hi: DWord;
begin
   lo := offset;
   hi := offset shr 32;
   SetFilePointer (StreamID, lo, @hi, FILE_BEGIN);
   ReadBytes := _lRead (StreamID, buf, count);

   if Readbytes = HFILE_ERROR then
      ReadArchiveCallBack := GetLastError
   else
      ReadArchiveCallBack := s_OK;

   if processedSize <> nil then
      processedSize^ := ReadBytes;
end;

// ============================================================================
// On Click to select button, let user choose an archive

procedure TArchiveExploderForm.SelectButtonClick(Sender: TObject);
begin
   if OpenDialog.execute then begin
      ShowFiles (OpenDialog.FileName);
   end;
end;

procedure TArchiveExploderForm.ShowFiles (FileName: string);
var lo, hi: DWord;
    FullSize: comp;
    OpenResult, i, RowCount, ArchiveFormat, flags: integer;
    FileInfo: TFileInArchiveInfo;
    start, stop: cardinal;
    ArchivePropertyStr: string;
begin
      // Set grid properties
      start := getTickCount;

      ArchiveGrid.RowCount := 2;
      ArchiveGrid.FixedRows := 1;
      ExtractAllButton.Enabled := false;

      // Close previously used handles
      if FileHandle <> 0 then
         CloseHandle (FileHandle);

      // Open new archive
      FileHandle := CreateFile (PChar (FileName), GENERIC_READ, 0,
                                nil, OPEN_EXISTING, File_Attribute_NORMAL, 0);
      lo := GetFileSize (FileHandle, @hi);
      FullSize := lo + (hi shr 32);

      // Determine archive format
      flags := 0;
      ArchiveFormat := DetermineArchiveType (ReadArchiveCallback, FileHandle,
                                             FullSize, flags);
      //ArchivePropertyStr := NumToStr (flags, 2) + ' ';
      ArchivePropertyStr := '';
      if flags and ArchiveFlagEncrypted <> 0 then
         ArchivePropertyStr := 'encrypted ';
      if flags and ArchiveFlagSolid <> 0 then
         ArchivePropertyStr := ArchivePropertyStr + 'solid ';

      // Open archive in dll
      ArchiveHandle := OpenArchive (ReadArchiveCallback, FileHandle, FullSize,
                                    ArchiveFormat, OpenResult,
                                    PChar (PasswordEdit.text));

      if OpenResult <> 0 then begin
         LogMemo.lines.Add('Error opening ' + FileName + ', code ' +
                           IntToStr (OpenResult));
      end;

      // List archive contents in browser
      RowCount := GetFileCount (ArchiveHandle);
      ArchiveGrid.RowCount := RowCount + 1;
      for i := 0 to RowCount - 1 do begin
         GetFileInfo (ArchiveHandle, i, @FileInfo);
         AddObject (@FileInfo, i+1);
      end;
      ExtractAllButton.Enabled := true;
      stop := getTickCount;
      LogMemo.lines.add ('Opened ' + ArchivePropertyStr +
                         GetArchiveExtension (ArchiveFormat) +' archive ' +
                         FileName + ', ' +
                         FloatToStrF (stop - start, ffGeneral, 5, 0) + 'ms');
      caption := WindowTitle + ' - ' + FileName;
end;

// ============================================================================

function IntToStrNice (num: integer; base: integer; GroupSepChar: char = ' '):
   string;
var str: string;
    g, groups: integer;
const ciphers = '0123456789abcdefghijklmnopqrstuvwxyz';
begin
   if num < 0 then
      IntToStrNice := '-' + IntToStrNice (-num, base)
   else if num = 0 then
      IntToStrNice := '0'
   else begin
      str := '';

      if base = 10 then
         groups := 3
      else
         groups := 4;

      g := groups;
      while num <> 0 do begin
         str := ciphers[num mod base + 1] + str;
         num := num div base;
         dec (g);
         if (g = 0) and (num <> 0) then begin
            str := GroupSepChar + str;
            g := groups;
         end;
      end;
      IntToStrNice := str;
   end;
end;

// ============================================================================
// Create table entry for file in archive

procedure TArchiveExploderForm.AddObject(FileInfo: pFileInArchiveInfo; row: integer);
var AttrStr: string;
    sysTime: _SystemTime;
begin
      if (FileInfo^.attributes and faDirectory) <> 0 then
         AttrStr := 'D'
      else
         AttrStr := '-';

      if (FileInfo^.attributes and faReadOnly) <> 0 then
         AttrStr := AttrStr + 'R'
      else
         AttrStr := AttrStr + '-';

      if (FileInfo^.attributes and faHidden) <> 0 then
         AttrStr := AttrStr + 'H'
      else
         AttrStr := AttrStr + '-';

      if (FileInfo^.attributes and faSysFile) <> 0 then
         AttrStr := AttrStr + 'S'
      else
         AttrStr := AttrStr + '-';

      if (FileInfo^.attributes and faArchive) <> 0 then
         AttrStr := AttrStr + 'A'
      else
         AttrStr := AttrStr + '-';

      FileTimeToSystemTime (FileInfo^.LastWriteTime, sysTime);

      ArchiveGrid.cells[0, row] := FileInfo^.path;
      ArchiveGrid.cells[1, row] := AttrStr;
      ArchiveGrid.cells[2, row] := IntToStrNice (FileInfo.CompressedFileSize, 10);
      ArchiveGrid.cells[3, row] := IntToStrNice (FileInfo.UncompressedFileSize, 10);
      ArchiveGrid.cells[4, row] := IntToStr (sysTime.wYear) + '-' +
                                   IntToStr (sysTime.wMonth) + '-' +
                                   IntToStr (sysTime.wDay) + ', ' +
                                   IntToStr (sysTime.wHour) + ':' +
                                   IntToStr (sysTime.wMinute);
end;

// ============================================================================
// On double click to a line in the grid, extract the archived file

procedure TArchiveExploderForm.ArchiveGridDblClick(Sender: TObject);
var OutFileHandle: THandle;
    OutFileName: string;
    pOutFileName: PChar;
    WrittenSize: comp;
    start, stop: cardinal;
begin
   SaveDialog1.FileName :=
      ExtractFileName (ArchiveGrid.Cells[0, ArchiveGrid.Selection.Top]);
   if SaveDialog1.Execute then begin
      OutFileName := SaveDialog1.Files.Strings[0];
      pOutFileName := PChar (OutFileName);
      OutFileHandle := CreateFile (pOutFileName, Generic_write,
                                   FILE_SHARE_READ, nil, CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL, 0);
      start := getTickCount;
      extract (ArchiveHandle, ArchiveGrid.Selection.Top - 1, OutFileHandle,
               WriteCallBack, WrittenSize);
      stop := getTickCount;
      LogMemo.lines.add ('Extracted ' +
                       ArchiveGrid.Cells[0, ArchiveGrid.Selection.Top] +
                       ' to ' + string (outFileName) + ', ' +
                       FloatToStrF ((stop - start) / 1000, ffGeneral, 5, 0) + 's');
      CloseHandle (OutFileHandle);
   end;
end;

// ============================================================================
// Extract all files

procedure TArchiveExploderForm.ExtractAllButtonClick(Sender: TObject);
var start, stop: cardinal;
begin
   If ExtractToSelectionForm.showModal = mrOK then begin
      ExtractAllTargetDir := ExtractToSelectionForm.TargetDirEdit.text;
      start := getTickCount;
      extractAll (ArchiveHandle, WriteCallback, requestStream);
      stop := getTickCount;
      LogMemo.lines.add ('All files extracted to ' + ExtractAllTargetDir +
                         ', ' + FloatToStrF ((stop - start) / 1000, ffGeneral,
                         5, 0) + 's');
   end;
end;

end.
