////////////////////////////////////////////////////////////////////////////////
// BGStrings.pas
// Copyright 2004-2005 X-Ways Software Technology AG
// Written by Björn Ganster
////////////////////////////////////////////////////////////////////////////////

unit BGStrings;

interface

const ciphers = '0123456789abcdefghijklmnopqrstuvwxyz';
      Base02Prefix = '02';
      Base08Prefix = '08';
      Base10Prefix = '';
      Base16Prefix = '0x';
const
  PathDelim  = {$IFDEF MSWINDOWS} '\'; {$ELSE} '/'; {$ENDIF}
  DriveDelim = {$IFDEF MSWINDOWS} ':'; {$ELSE} '';  {$ENDIF}
  PathSep    = {$IFDEF MSWINDOWS} ';'; {$ELSE} ':'; {$ENDIF}
  newLine = char ($0d) + char ($0a);

// Convert an integer to a grouped string using a specified base
function NumToStr (num: integer; base: integer = 10; groups: integer = 0): string;

// Convert a string containing a base prefix to an integer
// todo test: this may not respect spaces in grouped strings as created by the
// function above
function StrToNum (str: string): integer;

// Get Columnth word from line, separated by ColumnSeparator
function GetWord (line: string; column: integer; ColumnSeparator: char): string;
function GetWordQuoteAware (line: string; column: integer; ColumnSeparator: char): string;

// Get the number of words in line, separated by ColumnSeparator
function GetWordCount (line: string; ColumnSeparator: char): integer;
function GetWordCountQuoteAware (line: string; ColumnSeparator: char): integer;

// Return the length of a string, but scan no more than MaxLen chars
function strLLen (str: PChar; MaxLen: integer): integer;

// Format a GetLastError result into a string
function GetWindowsErrorStr (MsgId: integer): string; overload;
function GetWindowsErrorStr: string; overload;

// Parse a file name into its path, file name and extension
procedure ParseFileName (FilePathName: string; var path, FileName, ext: string);

// Parse an URL into its protcol, site and address
function ParseURL (URL: string; var protocol, site, address: string): boolean;

// Creates a deep copy of str, meaning that the result of DeepCopy remains
// valid even if str is altered or deleted
function DeepCopy (str: PChar): PChar;

function AppendNoDups (str: string; c: char): string;

// Returns true if there are characters different from c in str
function HasOtherCharacters (str: string; c: char): boolean;

implementation

uses strings, windows, smallSysUtils;

////////////////////////////////////////////////////////////////////////////////
// Convert a string containing a base prefix such as 02, 08, 0x to an integer
// Spaces in the string are removed automatically to handle grouped strings
// such as 123 456

function StrToNum (str: string): integer;
var num, i, j, start, base: integer;
begin
   // Check for base other than 10
   if copy (str, 1, 2) = Base02Prefix then begin
      base := 2;
      start := length (Base02Prefix)+1;
   end else if copy (str, 1, 2) = Base08Prefix then begin
      base := 8;
      start := length (Base08Prefix)+1;
   end else if copy (str, 1, 2) = Base16Prefix then begin
      base := 16;
      start := length (Base16Prefix)+1;
   end else begin
      base := 10;
      start := length (Base10Prefix)+1;
   end;

   // Conversion
   num := 0;
   str := lowerCase (str);
   for i := start to length(str) do begin
      j := pos (str[i], ciphers)-1;
      if j >= 0 then
         num := j + num * base;
   end;
   StrToNum := num;
end;

////////////////////////////////////////////////////////////////////////////////
// Convert an integer to a grouped string using a specified base

function NumToStr (num: integer; base: integer; groups: integer): string;
var str: string;
    g: integer;
begin
   if num < 0 then
      NumToStr := '-' + NumToStr (-num, base)
   else if num = 0 then
      NumToStr := '0'
   else begin
      str := '';

      // Decide how to group string
{      if base = 10 then
         groups := 3
      else
         groups := 4;}

      // Convert
      g := groups;
      while num <> 0 do begin
         str := ciphers[num mod base + 1] + str;
         num := num div base;
         dec (g);
         if (g = 0) and (num <> 0) then begin
            str := ' ' + str;
            g := groups;
         end;
      end;
      NumToStr := str;
   end;
end;

////////////////////////////////////////////////////////////////////////////////
// Get Columnth word from line, separated by ColumnSeparator

function GetWord (line: string; column: integer; ColumnSeparator: char): string;
var currCol, pos, len: integer;
begin
   currCol := 0;
   pos := 1;
   len := length (line);
   result := '';

   // Skip any data before column
   while (pos <= len) and (currCol < column) do begin
      if line[pos] = ColumnSeparator then
         inc (currCol);
      inc (pos);
   end;

   // Copy any data in column
   // todo: copy and test optimzations form quote aware version 
   while (pos <= len) and (currCol = column) do begin
      if line[pos] = ColumnSeparator then
         inc (currCol)
      else
         result := result + line[pos];
      inc (pos);
   end;
end;

function GetWordQuoteAware (line: string; column: integer;
   ColumnSeparator: char): string;
var currCol, pos, len, colStart, colEnd: integer;
    QuoteMode: boolean;
const QuoteChar = '"';
begin
   currCol := 0;
   pos := 1;
   len := length (line);
   result := '';
   QuoteMode := false;

   // Skip any data before column
   while (pos <= len) and (currCol < column) do begin
      if line[pos] = QuoteChar then
         quoteMode := not QuoteMode
      else if (line[pos] = ColumnSeparator) and not QuoteMode then
         inc (currCol);
      inc (pos);
   end;

   // Copy any data in column
   ColStart := pos;
   ColEnd := pos+1;
   while (pos <= len) and (currCol = column) do begin
      if line[pos] = QuoteChar then begin
         quoteMode := not QuoteMode;
         if pos = ColStart then
            inc (ColStart);
      end else if (line[pos] = ColumnSeparator) and not QuoteMode then
         inc (currCol)
      else
         ColEnd := pos;
      inc (pos);
   end;
   result := copy (line, colStart, colEnd-colstart+1);
end;

////////////////////////////////////////////////////////////////////////////////
// Get the number of words in line, separated by ColumnSeparator

function GetWordCount (line: string; ColumnSeparator: char): integer;
var i: integer;
begin
   result := 0;
   for i := 1 to length (line) do
      if line [i] = ColumnSeparator then
         inc (result);
end;

function GetWordCountQuoteAware (line: string; ColumnSeparator: char): integer;
var pos: integer;
    QuoteMode: boolean;
const QuoteChar = '"';
begin
   result := 0;
   QuoteMode := false;

   for pos := 1 to length (line) do
      if line[pos] = QuoteChar then
         quoteMode := not QuoteMode
      else if (line[pos] = ColumnSeparator) and not QuoteMode then
         inc (result);
end;

////////////////////////////////////////////////////////////////////////////////
// Format a GetLastError result into a string

function GetWindowsErrorStr (MsgId: integer): string;
var msgBuf: PChar;
begin
   FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER or FORMAT_MESSAGE_FROM_SYSTEM,
      nil,
      MsgID,
      0,
      @MsgBuf,
      0,
      nil);
   if MsgBuf <> nil then
      result := MsgBuf
   else
      result := '<NULL>';
   LocalFree (cardinal (MsgBuf));
end;

function GetWindowsErrorStr: string;
begin
   result := GetWindowsErrorStr (GetLastError);
end;
////////////////////////////////////////////////////////////////////////////////
// Return the length of a string, but scan no more than MaxLen chars

function strLLen (str: PChar; MaxLen: integer): integer;
var i: integer;
begin
   i := 0;
   if str <> nil then
      while (i < MaxLen) and (str [i] <> char (0)) do
         inc (i);
   result := i;
end;

////////////////////////////////////////////////////////////////////////////////
// Parse a file name into its path, file name and extension

procedure ParseFileName (FilePathName: string; var path, FileName, ext: string);
var DotPos, DelimPos: integer;
begin
   // Find position of last dot and path delimiter
   DotPos := length (FilePathName);
   while (DotPos > 0) and (FilePathName[DotPos] <> '.')
   do dec (DotPos);
   DelimPos := length (FilePathName);
   while (DelimPos > 0) and (FilePathName[DelimPos] <> PathDelim)
   do dec (DelimPos);

   // Fill path, FileName, ext
   if DotPos > 0 then begin
      ext := copy (FilePathName, DotPos+1, length (FilePathName));
   end else begin
      ext := '';
      DotPos := length (FilePathName)+1;
   end;

   if DelimPos > 0 then begin
      FileName := copy (FilePathName, DelimPos+1, DotPos - DelimPos-1);
      path := copy (FilePathName, 1, DelimPos-1);
   end else begin
      FileName := copy (FilePathName, 1, DotPos-1);
      path := '';
   end;
end;

////////////////////////////////////////////////////////////////////////////////
// Parse an URL into its protcol, site and address

function ParseURL (URL: string; var protocol, site, address: string): boolean;
var i, urlLen, protEnd, addrStart: integer;
begin
   result := false;
   protEnd := 0;
   addrStart := 0;
   site := url;
   protocol := '';
   address := '';

   // Test for :, / as separators
   i := 1;
   urlLen := length (URL);
   while (i < length (URL)) and (addrStart = 0) do begin
      if (URL[i] = ':') and (protEnd = 0) and (i + 3 < urlLen) then
      begin
         if  (url[i+0] = ':')
         and (url[i+1] = '/')
         and (url[i+2] = '/')
         then begin
            protEnd := i;
            i := i+2;
            addrStart := 0;
         end;
      end else if (URL[i] = '/') and (addrStart = 0) then begin
         addrStart := i;
      end;
      inc (i);
   end;

   // Set results
   if (protEnd > 0) and (addrStart > 0) //and (protEnd+3 < URLLen)
   then begin
      // full url, including local address
      protocol := copy (url, 1, protEnd-1);
      site := copy (url, protEnd+3, addrStart-(protEnd+3));
      address := copy (url, addrStart+1, length (url));
      result := true;
   end else if (protEnd > 0) and (addrStart = 0)
   then begin
      // empty local address
      protocol := copy (url, 1, protEnd-1);
      site := copy (url, protEnd+3, urlLen-(protEnd+3));
      address := '';
      result := true;
   end;
end;

////////////////////////////////////////////////////////////////////////////////
// Creates a deep copy of str, meaning that the result of DeepCopy remains
// valid even if str is altered or deleted

function DeepCopy (str: PChar): PChar;
var len: integer;
begin
   if str <> nil then begin
      len := strlen (str);
      getmem (result, len+1);
      strcopy (result, str);
   end else
      result := nil;
end;

////////////////////////////////////////////////////////////////////////////////

function AppendNoDups (str: string; c: char): string;
begin
   if str[length(str)] <> c then
      result := str + c
   else
      result := str;
end;

////////////////////////////////////////////////////////////////////////////////
// Returns true if there are characters different from c in str

function HasOtherCharacters (str: string; c: char): boolean;
var i: integer;
begin
   result := false;
   for i := 1 to length (str) do
      if str[i] <> c then
         result := true;
end;

end.
