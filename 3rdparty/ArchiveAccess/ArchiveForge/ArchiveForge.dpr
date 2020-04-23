program ArchiveForge;

uses
  Forms,
  MainUnit in 'MainUnit.pas' {ArchiveForgeForm},
  NewArchiveForm in 'NewArchiveForm.pas' {ArchiveOptionsForm},
  ArchiveAccess in '..\ArchiveExploder\ArchiveAccess.pas',
  Dialogs;

{$R *.res}

begin
   // Load ArchiveAccess dll
   if LoadArchiveAccessDll (ArchiveAccessDllName) then begin
      Application.Initialize;
      Application.CreateForm(TArchiveForgeForm, ArchiveForgeForm);
  Application.CreateForm(TArchiveOptionsForm, ArchiveOptionsForm);
  Application.Run;
   end else
      MessageDlg ('Unable to load ' + ArchiveAccessDllName + ', giving up',
                  mtWarning, [mbOK], 0);
end.
