unit MainUnit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ExtCtrls, Grids;

type
  TArchiveForgeForm = class(TForm)
    Grid: TStringGrid;
    OpenDialog1: TOpenDialog;
    Panel1: TPanel;
    AddFilesButton: TButton;
    NewArchiveButtonButton: TButton;
    RemoveFilesButton: TButton;
    SaveDialog1: TSaveDialog;
    SaveButton: TButton;
    procedure AddFilesButtonClick(Sender: TObject);
    procedure NewArchiveButtonButtonClick(Sender: TObject);
    procedure RemoveFilesButtonClick(Sender: TObject);
    procedure SaveButtonClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    { Private declarations }
    ArchiveName: string;
    ArchiveType: integer;
  public
    { Public declarations }
  end;

var
  ArchiveForgeForm: TArchiveForgeForm;

implementation

uses ArchiveAccess, NewArchiveForm, BGStrings;

{$R *.dfm}

////////////////////////////////////////////////////////////////////////////////
// OnCreate handler, loads ArchiveAccess.dll or terminates with message

procedure TArchiveForgeForm.FormCreate(Sender: TObject);
begin
   grid.cells[0,0] := 'File Name';
   grid.cells[1,0] := 'Size';
   grid.ColCount := 2;
   grid.ColWidths[0] := 300;
   grid.ColWidths[1] := 100;
end;

////////////////////////////////////////////////////////////////////////////////

procedure TArchiveForgeForm.NewArchiveButtonButtonClick(Sender: TObject);
var i: integer;
    ext: string;
begin
   if ArchiveOptionsForm.showModal = mrOK then begin
      if SaveDialog1.execute then begin
         ArchiveName := saveDialog1.FileName;

         // Select file format based on current filter selection and append an
         // extension if no extension has been selected
         i := 2 * SaveDialog1.FilterIndex - 1;
         ext := GetWord (SaveDialog1.Filter, i, '|');
         if ext <> '*.*' then begin
            ext := copy (ExtractFileExt (ext), 2, 3);
         end else begin
            ext := 'zip';
         end;
         ArchiveType := StrAsInt (PChar (ext));
         if (ExtractFileExt (ArchiveName) = '')
         then ArchiveName := ArchiveName + '.' + ext;

         grid.RowCount := 2;
         for i := 0 to grid.colCount do
            grid.cells[i, 1] := '';
         caption := 'ArchiveForge - ' + ArchiveName;
      end;
   end;
end;

////////////////////////////////////////////////////////////////////////////////

procedure TArchiveForgeForm.AddFilesButtonClick(Sender: TObject);
var i, start: integer;
    f: file;
begin
   if OpenDialog1.Execute then begin
      start := grid.RowCount;
      while (start > 1) and (grid.cells[0,start-1] = '') do
         dec (start);
      if start + OpenDialog1.Files.Count > grid.rowCount then
         grid.RowCount := start + OpenDialog1.Files.Count;
      for i := 0 to OpenDialog1.Files.count-1 do begin
         grid.cells[0, start+i] := OpenDialog1.Files[i];
         grid.cells[0, start+i] := OpenDialog1.Files[i];
         try
            assignFile (f, OpenDialog1.Files[i]);
            reset(f, 1);
            grid.cells[1, start+i] := NumToStr (FileSize (f), 10);
         finally
            CloseFile (f);
         end;
      end;
   end;
end;

////////////////////////////////////////////////////////////////////////////////

procedure TArchiveForgeForm.RemoveFilesButtonClick(Sender: TObject);
var i, first, last, count: integer;
begin
   first := grid.Selection.Top;
   last := grid.Selection.Bottom+1;
   count := last-first;
   for i := first to grid.RowCount do begin
      grid.Rows [i] := grid.Rows [i + count];
   end;
   grid.RowCount := grid.RowCount - count;
end;

////////////////////////////////////////////////////////////////////////////////

procedure TArchiveForgeForm.SaveButtonClick(Sender: TObject);
var ArchiveHandle: aaHandle;
    row: integer;
    password, FileName: string;
begin
   // Create an archive containing the selected files ...
   if ArchiveOptionsForm.EncryptFilesCheckBox.checked then
      password := ''
   else
      password := ArchiveOptionsForm.PasswordEdit.text;
   ArchiveHandle :=
      CreateArchiveFile (PChar (ArchiveName), ArchiveType, password);
   if ArchiveHandle <> nil then begin
      for row := 1 to grid.RowCount-1 do begin
         FileName := PChar (grid.cells[0, row]);
         if  (Filename <> '')
         and (Filename <> '.')
         and (Filename <> '..')
         then AddFile (ArchiveHandle, PChar (FileName));
      end;
   end else
      MessageBox (handle, 'Archive creation failed', 'ArchiveForge', 0);
   CloseArchive (ArchiveHandle);
end;

end.

