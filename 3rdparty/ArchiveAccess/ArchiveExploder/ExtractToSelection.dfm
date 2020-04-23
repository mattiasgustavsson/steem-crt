object ExtractToSelectionForm: TExtractToSelectionForm
  Left = 277
  Top = 272
  Width = 310
  Height = 115
  Caption = 'Extract all files'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 16
    Top = 20
    Width = 48
    Height = 13
    Caption = 'Extract to:'
  end
  object TargetDirEdit: TEdit
    Left = 72
    Top = 16
    Width = 217
    Height = 21
    TabOrder = 0
    Text = 'd:\temp\unpacked'
  end
  object Button1: TButton
    Left = 128
    Top = 48
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 1
  end
  object Button2: TButton
    Left = 216
    Top = 48
    Width = 75
    Height = 25
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
