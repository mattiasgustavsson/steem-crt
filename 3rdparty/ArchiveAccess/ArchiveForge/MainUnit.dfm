object ArchiveForgeForm: TArchiveForgeForm
  Left = 343
  Top = 218
  Width = 721
  Height = 525
  Caption = 'ArchiveForge'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefault
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Grid: TStringGrid
    Left = 0
    Top = 41
    Width = 713
    Height = 457
    Align = alClient
    FixedCols = 0
    RowCount = 2
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goColSizing]
    TabOrder = 0
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 713
    Height = 41
    Align = alTop
    TabOrder = 1
    object AddFilesButton: TButton
      Left = 96
      Top = 8
      Width = 75
      Height = 25
      Caption = 'Add Files'
      TabOrder = 0
      OnClick = AddFilesButtonClick
    end
    object NewArchiveButtonButton: TButton
      Left = 8
      Top = 8
      Width = 75
      Height = 25
      Caption = 'New'
      TabOrder = 1
      OnClick = NewArchiveButtonButtonClick
    end
    object RemoveFilesButton: TButton
      Left = 184
      Top = 8
      Width = 75
      Height = 25
      Caption = 'Remove Files'
      TabOrder = 2
      OnClick = RemoveFilesButtonClick
    end
    object SaveButton: TButton
      Left = 272
      Top = 8
      Width = 75
      Height = 25
      Caption = 'Save'
      TabOrder = 3
      OnClick = SaveButtonClick
    end
  end
  object OpenDialog1: TOpenDialog
    Options = [ofHideReadOnly, ofAllowMultiSelect, ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Left = 408
    Top = 8
  end
  object SaveDialog1: TSaveDialog
    Filter = 'Zip Files|*.zip|Tar Files|*.tar|All Files|*.*'
    Left = 440
    Top = 8
  end
end
