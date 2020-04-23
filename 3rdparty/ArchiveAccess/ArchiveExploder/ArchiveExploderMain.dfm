object ArchiveExploderForm: TArchiveExploderForm
  Left = 343
  Top = 267
  Width = 870
  Height = 640
  Caption = 'Archive Exploder'
  Color = clBtnFace
  Constraints.MinHeight = 200
  Constraints.MinWidth = 400
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 22
    Width = 46
    Height = 13
    Caption = 'Password'
  end
  object SelectButton: TButton
    Left = 200
    Top = 16
    Width = 75
    Height = 25
    Caption = 'Open'
    TabOrder = 0
    OnClick = SelectButtonClick
  end
  object ArchiveGrid: TStringGrid
    Left = 8
    Top = 56
    Width = 841
    Height = 449
    FixedCols = 0
    RowCount = 2
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goColSizing, goRowSelect]
    TabOrder = 1
    OnDblClick = ArchiveGridDblClick
    ColWidths = (
      190
      64
      114
      64
      64)
  end
  object ExtractAllButton: TButton
    Left = 288
    Top = 16
    Width = 75
    Height = 25
    Caption = 'Extract All'
    TabOrder = 2
    OnClick = ExtractAllButtonClick
  end
  object logMemo: TMemo
    Left = 8
    Top = 512
    Width = 841
    Height = 89
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 3
  end
  object PasswordEdit: TEdit
    Left = 64
    Top = 18
    Width = 121
    Height = 21
    PasswordChar = '*'
    TabOrder = 4
  end
  object OpenDialog: TOpenDialog
    Left = 768
    Top = 8
  end
  object SaveDialog1: TSaveDialog
    Left = 808
    Top = 8
  end
end
