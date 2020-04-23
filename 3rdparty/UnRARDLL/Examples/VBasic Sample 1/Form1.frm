VERSION 5.00
Begin VB.Form Form1 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "UnRAR.dll Visual Basic Example"
   ClientHeight    =   5325
   ClientLeft      =   45
   ClientTop       =   345
   ClientWidth     =   6990
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5325
   ScaleWidth      =   6990
   StartUpPosition =   2  'CenterScreen
   Begin VB.CommandButton Command5 
      Cancel          =   -1  'True
      Caption         =   "&Exit"
      Default         =   -1  'True
      Enabled         =   0   'False
      Height          =   375
      Left            =   120
      TabIndex        =   1
      Top             =   4920
      Width           =   6735
   End
   Begin VB.ListBox List1 
      Height          =   4740
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   6735
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'******************************************************************
'
'   Ported to Visual Basic by Pedro Lamas
'
'E-mail:  sniper@hotpop.com
'HomePage (dedicated to VB):  www.terravista.pt/portosanto/3723/
'
'******************************************************************

Const ERAR_END_ARCHIVE = 10
Const ERAR_NO_MEMORY = 11
Const ERAR_BAD_DATA = 12
Const ERAR_BAD_ARCHIVE = 13
Const ERAR_UNKNOWN_FORMAT = 14
Const ERAR_EOPEN = 15
Const ERAR_ECREATE = 16
Const ERAR_ECLOSE = 17
Const ERAR_EREAD = 18
Const ERAR_EWRITE = 19
Const ERAR_SMALL_BUF = 20
 
Const RAR_OM_LIST = 0
Const RAR_OM_EXTRACT = 1
 
Const RAR_SKIP = 0
Const RAR_TEST = 1
Const RAR_EXTRACT = 2
 
Const RAR_VOL_ASK = 0
Const RAR_VOL_NOTIFY = 1

Enum RarOperations
    OP_EXTRACT = 0
    OP_TEST = 1
    OP_LIST = 2
End Enum
 
Private Type RARHeaderData
    ArcName As String * 260
    FileName As String * 260
    Flags As Long
    PackSize As Long
    UnpSize As Long
    HostOS As Long
    FileCRC As Long
    FileTime As Long
    UnpVer As Long
    Method As Long
    FileAttr As Long
    CmtBuf As String
    CmtBufSize As Long
    CmtSize As Long
    CmtState As Long
End Type
 
Private Type RAROpenArchiveData
    ArcName As String
    OpenMode As Long
    OpenResult As Long
    CmtBuf As String
    CmtBufSize As Long
    CmtSize As Long
    CmtState As Long
End Type
 
Private Declare Function RAROpenArchive Lib "unrar.dll" (ByRef ArchiveData As RAROpenArchiveData) As Long
Private Declare Function RARCloseArchive Lib "unrar.dll" (ByVal hArcData As Long) As Long
Private Declare Function RARReadHeader Lib "unrar.dll" (ByVal hArcData As Long, ByRef HeaderData As RARHeaderData) As Long
Private Declare Function RARProcessFile Lib "unrar.dll" (ByVal hArcData As Long, ByVal Operation As Long, ByVal DestPath As String, ByVal DestName As String) As Long
Private Declare Sub RARSetChangeVolProc Lib "unrar.dll" (ByVal hArcData As Long, ByVal Mode As Long)
Private Declare Sub RARSetPassword Lib "unrar.dll" (ByVal hArcData As Long, ByVal Password As String)

Private Sub Command5_Click()
    End
End Sub

Private Sub RARExecute(Mode As RarOperations, RarFile As String, Optional Password As String)
    ' Description:-
    ' Extract file(s) from RAR archive.
    ' Parameters:-
    ' Mode = Operation to perform on RAR Archive
    ' RARFile = RAR Archive filename
    ' sPassword = Password (Optional)
    Dim lHandle As Long
    Dim iStatus As Integer
    Dim uRAR As RAROpenArchiveData
    Dim uHeader As RARHeaderData
    Dim sStat As String, Ret As Long
     
    uRAR.ArcName = RarFile
    uRAR.CmtBuf = Space(16384)
    uRAR.CmtBufSize = 16384
    
    If Mode = OP_LIST Then
        uRAR.OpenMode = RAR_OM_LIST
    Else
        uRAR.OpenMode = RAR_OM_EXTRACT
    End If
    
    lHandle = RAROpenArchive(uRAR)
    If uRAR.OpenResult <> 0 Then OpenError uRAR.OpenResult, RarFile
 
    If Password <> "" Then RARSetPassword lHandle, Password
    
    If (uRAR.CmtState = 1) Then MsgBox uRAR.CmtBuf, vbApplicationModal + vbInformation, "Comment"
    
    iStatus = RARReadHeader(lHandle, uHeader)
    Show
    Do Until iStatus <> 0
        sStat = Left(uHeader.FileName, InStr(1, uHeader.FileName, vbNullChar) - 1)
        Select Case Mode
        Case RarOperations.OP_EXTRACT
            List1.AddItem "Extracting " & sStat
            Ret = RARProcessFile(lHandle, RAR_EXTRACT, "", uHeader.FileName)
        Case RarOperations.OP_TEST
            List1.AddItem "Testing " & sStat
            Ret = RARProcessFile(lHandle, RAR_TEST, "", uHeader.FileName)
        Case RarOperations.OP_LIST
            List1.AddItem "File: " & sStat & vbTab & vbTab & vbTab & "Size: " & uHeader.UnpSize
            Ret = RARProcessFile(lHandle, RAR_SKIP, "", "")
        End Select
        
        If Ret = 0 Then
            List1.List(List1.ListCount - 1) = List1.List(List1.ListCount - 1) & vbTab & vbTab & "OK"
        Else
            ProcessError Ret
        End If
        
        iStatus = RARReadHeader(lHandle, uHeader)
        Refresh
    Loop
    
    If iStatus = ERAR_BAD_DATA Then Erro ("File header broken")
    
    RARCloseArchive lHandle
End Sub

Private Sub Form_Load()
    Dim Vals() As String, Msg As String
    If Command = "" Then
        Msg = "VBUNRAR.EXE.     This is a simple example of UNRAR.DLL usage" & vbCrLf & vbCrLf
        Msg = Msg & "Syntax:" & vbCrLf
        Msg = Msg & "VBUNRAR  X  <Archive>  <Password>     extract archive contents" & vbCrLf
        Msg = Msg & "VBUNRAR  T  <Archive>  <Password>     test archive contents" & vbCrLf
        Msg = Msg & "VBUNRAR  L  <Archive>  <Password>     view archive contents"
        MsgBox Msg, vbApplicationModal + vbInformation, "VBUnRAR"
        End
    End If
    Vals = Split(Command, " ")
    
    ReDim Preserve Vals(2)
    If Vals(0) = "" Or Vals(1) = "" Then Erro ("Missing arguments!")
    Select Case UCase(Vals(0))
    Case "X"
        RARExecute OP_EXTRACT, Vals(1), Vals(2)
    Case "T"
        RARExecute OP_TEST, Vals(1), Vals(2)
    Case "L"
        RARExecute OP_LIST, Vals(1), Vals(2)
    Case Else
        Erro "Invalid Arguments!"
    End Select
    Command5.Enabled = True
End Sub

Private Sub OpenError(ErroNum As Long, ArcName As String)
    Select Case ErroNum
    Case ERAR_NO_MEMORY
        Erro "Not enough memory"
    Case ERAR_EOPEN:
        Erro "Cannot open " & ArcName
    Case ERAR_BAD_ARCHIVE:
        Erro ArcName & " is not RAR archive"
    Case ERAR_BAD_DATA:
        Erro ArcName & ": archive header broken"
    End Select
End Sub

Private Sub ProcessError(ErroNum As Long)
    Select Case ErroNum
    Case ERAR_UNKNOWN_FORMAT
        Erro "Unknown archive format"
    Case ERAR_BAD_ARCHIVE:
        Erro "Bad volume"
    Case ERAR_ECREATE:
        Erro "File create error"
    Case ERAR_EOPEN:
        Erro "Volume open error"
    Case ERAR_ECLOSE:
        Erro "File close error"
    Case ERAR_EREAD:
        Erro "Read error"
    Case ERAR_EWRITE:
        Erro "Write error"
    Case ERAR_BAD_DATA:
        Erro "CRC error"
    End Select
End Sub

Private Sub Erro(Msg As String)
    MsgBox Msg, vbApplicationModal + vbExclamation, "Error"
    End
End Sub
