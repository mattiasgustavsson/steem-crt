unit NewArchiveForm;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TArchiveOptionsForm = class(TForm)
    Label1: TLabel;
    EncryptFilesCheckBox: TCheckBox;
    PasswordEdit: TEdit;
    Button1: TButton;
    Button2: TButton;
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  ArchiveOptionsForm: TArchiveOptionsForm;

implementation

{$R *.dfm}

end.
