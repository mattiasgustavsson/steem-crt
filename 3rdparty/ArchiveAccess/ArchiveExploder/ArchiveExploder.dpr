program ArchiveExploder;

uses
  Forms,
  ArchiveExploderMain in 'ArchiveExploderMain.pas' {ArchiveExploderForm},
  ArchiveAccess in 'ArchiveAccess.pas',
  ExtractToSelection in 'ExtractToSelection.pas' {ExtractToSelectionForm},
  Dialogs,
  BGStrings;

{$R *.res}

begin
   // Load ArchiveAccess dll
   if LoadArchiveAccessDll (ArchiveAccessDllName) then begin
      Application.Initialize;
      Application.CreateForm(TArchiveExploderForm, ArchiveExploderForm);
      Application.CreateForm(TExtractToSelectionForm, ExtractToSelectionForm);
      Application.Run;
   end else
      MessageDlg ('Unable to load ' + ArchiveAccessDllName + ', giving up',
                  mtWarning, [mbOK], 0);
end.

