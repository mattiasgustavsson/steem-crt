// UnRDLL - simple example of UnRAR.dll usage
// Translated from UnRDLL.c
// Use Delphi 2.0 or higher to compile this module
//
// Ported to Delphi by Eugene Kotlyarov, fidonet: 2:5058/26.9 ek@oris.ru
// Fixed version by Alexey Torgashin <alextp@mail.ru>, 2:5020/604.24@fidonet

program UnRDLL;

{$APPTYPE CONSOLE}
{$H+}

uses Windows, UnRAR, SysUtils;

const
  EXTRACT = 0;
  TEST    = 1;
  PRINT   = 2;

procedure ShowComment(CmtBuf: PChar); forward;
procedure OutHelp; forward;
procedure OutOpenArchiveError(Error: Integer; ArcName: PChar); forward;
procedure OutProcessFileError(Error: Integer); forward;
procedure ShowArcInfo(Flags: UINT; ArcName: PChar); forward;

const
  CR = #13#10;

function SFmt(const S: String; Width: Byte): String;
begin
  if Length(S) < Width then
    Result := S + StringOfChar(' ', Width - Length(S));
end;

function CallbackProc(msg: UINT; UserData, P1, P2: integer) :integer; stdcall;
var
  Ch: Char;
  I: Integer;
  C: PChar;
  S: String;
begin
  Result := 0;
  case msg of
    UCM_CHANGEVOLUME:
      if (P2 = RAR_VOL_ASK) then begin
        Write(CR, 'Insert disk with ', PChar(P1), ' and press ''Enter'' or enter ''Q'' to exit ');
        Readln(Ch);
        if (UpCase (Ch) = 'Q') then
          Result := -1;
      end;
    UCM_NEEDPASSWORD:
      begin
        Write(CR, 'Please enter the password for this archive: ');
        Readln(S);
        C := PChar(S);
        Move(pointer(C)^, pointer(p1)^, StrLen(C) + 1);
          //+1 to copy the zero
      end;
    UCM_PROCESSDATA: begin
      if (UserData <> 0) and (PINT (UserData)^ = PRINT) then begin
        Flush (Output);
        // Windows.WriteFile fails on big data
        for I := 0 to P2 - 1 do
          Write(PChar(P1 + I)^);
        Flush (Output);
      end;
    end;
  end;
end;

procedure ExtractArchive(ArcName: PChar; Mode: Integer);
var
  hArcData: THandle;
  RHCode, PFCode: Integer;
  CmtBuf: array[0..Pred(16384)] of Char;
  HeaderData: RARHeaderData;
  OpenArchiveData: RAROpenArchiveDataEx;
  Operation: Integer;
begin
  OpenArchiveData.ArcName := ArcName;
  OpenArchiveData.CmtBuf := @CmtBuf;
  OpenArchiveData.CmtBufSize := SizeOf(CmtBuf);
  OpenArchiveData.OpenMode := RAR_OM_EXTRACT;
  hArcData := RAROpenArchiveEx(OpenArchiveData);

  if (OpenArchiveData.OpenResult <> 0) then
  begin
    OutOpenArchiveError(OpenArchiveData.OpenResult, ArcName);
    Exit;
  end;

  ShowArcInfo(OpenArchiveData.Flags, ArcName);

  if (OpenArchiveData.CmtState = 1) then
    ShowComment(CmtBuf);

  RARSetCallback (hArcData, CallbackProc, Integer (@Mode));

  HeaderData.CmtBuf := nil;

  repeat
    RHCode := RARReadHeader(hArcData, HeaderData);
    if RHCode <> 0 then 
      Break;

    case Mode of
      EXTRACT: Write(CR, 'Extracting ', SFmt(HeaderData.FileName, 45));
      TEST:    Write(CR, 'Testing ', SFmt(HeaderData.FileName, 45));
      PRINT:   Write(CR, 'Printing ', SFmt(HeaderData.FileName, 45), CR);
    end;

    if Mode = EXTRACT then
      Operation := RAR_EXTRACT
    else
      Operation := RAR_TEST;
    PFCode := RARProcessFile(hArcData, Operation, nil, nil);

    if (PFCode = 0) then
      Write(' Ok')
    else begin
      OutProcessFileError(PFCode);
      Break;
    end;
  until False;

  if (RHCode = ERAR_BAD_DATA) then
    Write(CR, 'File header broken');

  RARCloseArchive(hArcData);
end;

procedure ListArchive(ArcName: PChar);
var
  hArcData: THandle;
  RHCode, PFCode: Integer;
  CmtBuf: array[0..Pred(16384)] of Char;
  HeaderData: RARHeaderDataEx;
  OpenArchiveData: RAROpenArchiveDataEx;
begin
  OpenArchiveData.ArcName := ArcName;
  OpenArchiveData.CmtBuf := @CmtBuf;
  OpenArchiveData.CmtBufSize := SizeOf(CmtBuf);
  OpenArchiveData.OpenMode := RAR_OM_LIST;
  hArcData := RAROpenArchiveEx(OpenArchiveData);

  if (OpenArchiveData.OpenResult <> 0) then
  begin
    OutOpenArchiveError(OpenArchiveData.OpenResult, ArcName);
    Exit;
  end;

  ShowArcInfo(OpenArchiveData.Flags, ArcName);

  if (OpenArchiveData.CmtState = 1) then
    ShowComment(CmtBuf);

  RARSetCallback (hArcData, CallbackProc, 0);

  HeaderData.CmtBuf := @CmtBuf;
  HeaderData.CmtBufSize := SizeOf(CmtBuf);

  Write(CR, 'File', StringOfChar(' ',42),'Size');
  Write(CR, StringOfChar('-', 50));
  repeat
    RHCode := RARReadHeaderEx(hArcData, HeaderData);
    if RHCode <> 0 then 
      Break;
    Write(CR, SFmt(HeaderData.FileName, 39), ' ',
      (HeaderData.UnpSize + HeaderData.UnpSizeHigh * 4294967296.0):10:0);
    if (HeaderData.CmtState = 1) then
      ShowComment(CmtBuf);
    PFCode:= RARProcessFile(hArcData, RAR_SKIP, nil, nil);
    if (PFCode <> 0) then
    begin
      OutProcessFileError(PFCode);
      Break;
    end;
  until False;

  if (RHCode = ERAR_BAD_DATA) then
    Write(CR, 'File header broken');

  RARCloseArchive(hArcData);
end;

procedure ShowComment(CmtBuf: PChar);
begin
  Write(CR, 'Comment:', CR, CmtBuf, CR);
end;

procedure ShowArcInfo(Flags: UINT; ArcName: PChar);
  function CheckFlag(S: string; FlagBit: UINT): string;
  begin
    if (Flags and FlagBit) > 0 then result := 'yes' else result := 'no';
    Write(CR, Format('%-16s %s',[S, Result]));
  end;
begin
  Writeln(CR, 'Archive ', ArcName);
  CheckFlag('Volume:', 1);
  CheckFlag('Comment:', 2);
  CheckFlag('Locked:', 4);
  CheckFlag('Solid:', 8);
  CheckFlag('New naming:', 16);
  CheckFlag('Authenticity:', 32);
  CheckFlag('Recovery:', 64);
  CheckFlag('Encr.headers:', 128);
  CheckFlag('First volume:', 256);
  Writeln(CR, '---------------------------');
end;

procedure OutHelp;
begin
  Write(CR, 'UNRDLL.   This is a simple example of UNRAR.DLL usage', CR);
  Write(CR, 'Syntax:', CR);
  Write(CR, 'UNRDLL X <Archive>     extract archive contents');
  Write(CR, 'UNRDLL T <Archive>     test archive contents');
  Write(CR, 'UNRDLL P <Archive>     print archive contents to stdout');
  Write(CR, 'UNRDLL L <Archive>     view archive contents', CR);
end;

procedure OutOpenArchiveError(Error: Integer; ArcName: PChar);
begin
  case Error of
    ERAR_NO_MEMORY:   Write(CR, 'Not enough memory');
    ERAR_EOPEN:       Write(CR, 'Cannot open ', ArcName);
    ERAR_BAD_ARCHIVE: Write(CR, ArcName, ' is not RAR archive');
    ERAR_BAD_DATA:    Write(CR, ArcName, ': archive header broken');
    ERAR_UNKNOWN:     Write(CR, 'Unknown error');
  end;
end;

procedure OutProcessFileError(Error: Integer);
begin
  case Error of
    ERAR_UNKNOWN_FORMAT: Write('Unknown archive format');
    ERAR_BAD_ARCHIVE:    Write('Bad volume');
    ERAR_ECREATE:        Write('File create error');
    ERAR_EOPEN:          Write('Volume open error');
    ERAR_ECLOSE:         Write('File close error');
    ERAR_EREAD:          Write('Read error');
    ERAR_EWRITE:         Write('Write error');
    ERAR_BAD_DATA:       Write('CRC error');
    ERAR_UNKNOWN:        Write('Unknown error');
  end;
end;

begin
  if ParamCount <> 2 then
  begin
    OutHelp;
    Halt(0);
  end;

  case UpCase(ParamStr(1)[1]) of
    'X': ExtractArchive(PChar(ParamStr(2)), EXTRACT);
    'T': ExtractArchive(PChar(ParamStr(2)), TEST);
    'P': ExtractArchive(PChar(ParamStr(2)), PRINT);
    'L': ListArchive(PChar(ParamStr(2)));
  else
    OutHelp;
  end;

  Halt(0);
end.
