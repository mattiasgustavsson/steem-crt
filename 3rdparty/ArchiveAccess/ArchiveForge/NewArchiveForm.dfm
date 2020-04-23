object ArchiveOptionsForm: TArchiveOptionsForm
  Left = 578
  Top = 772
  Width = 226
  Height = 149
  Caption = 'New Archive Options'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 16
    Top = 56
    Width = 49
    Height = 13
    Caption = 'Password:'
  end
  object EncryptFilesCheckBox: TCheckBox
    Left = 16
    Top = 16
    Width = 97
    Height = 17
    Caption = 'Encrypt Files'
    TabOrder = 0
  end
  object PasswordEdit: TEdit
    Left = 80
    Top = 48
    Width = 121
    Height = 21
    TabOrder = 1
  end
  object Button1: TButton
    Left = 16
    Top = 88
    Width = 75
    Height = 25
    Caption = 'OK'
    ModalResult = 1
    TabOrder = 2
  end
  object Button2: TButton
    Left = 128
    Top = 88
    Width = 75
    Height = 25
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 3
  end
end
