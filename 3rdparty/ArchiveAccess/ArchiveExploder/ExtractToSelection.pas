unit ExtractToSelection;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TExtractToSelectionForm = class(TForm)
    Label1: TLabel;
    TargetDirEdit: TEdit;
    Button1: TButton;
    Button2: TButton;
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  ExtractToSelectionForm: TExtractToSelectionForm;

implementation

{$R *.dfm}

end.
