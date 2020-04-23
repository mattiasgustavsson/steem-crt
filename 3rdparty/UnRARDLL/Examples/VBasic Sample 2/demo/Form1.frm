VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   4110
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   11025
   LinkTopic       =   "Form1"
   ScaleHeight     =   4110
   ScaleWidth      =   11025
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Command1 
      Caption         =   "Command1"
      Height          =   615
      Left            =   2640
      TabIndex        =   1
      Top             =   1200
      Width           =   3135
   End
   Begin MSComctlLib.ProgressBar ProgressBar1 
      Height          =   495
      Left            =   840
      TabIndex        =   0
      Top             =   2640
      Width           =   9735
      _ExtentX        =   17171
      _ExtentY        =   873
      _Version        =   393216
      Appearance      =   1
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim WithEvents objTKUnrar As TKUnrar.CTKUnrar
Attribute objTKUnrar.VB_VarHelpID = -1
'

Private Sub Command1_Click()
    Debug.Print objTKUnrar.TestRarFiles("m:\test.rar")
End Sub

Private Sub Form_Load()
    Dim arrstrFilenames(100) As String

    Set objTKUnrar = New TKUnrar.CTKUnrar
End Sub

Private Sub objTKUnrar_CommentError(lngErrorNo As Long, strErrorMsg As String, strFilename As String)
    Debug.Print "CommentError: " & strFilename & " - " & strErrorMsg
End Sub

Private Sub objTKUnrar_MissingFile(strFilename As String, strNewFilename As String)
    Debug.Print "MissingFile: " & strFilename
    strNewFilename = ""
End Sub

Private Sub objTKUnrar_MissingPassword(strNewPassword As String)
    Debug.Print "MissingPassword"
    strNewPassword = ""
End Sub

Private Sub objTKUnrar_NextVolumne(strFilename As String, blnContinue As Boolean)
    Debug.Print "NextVolumne: " & strFilename
    blnContinue = True
End Sub

Private Sub objTKUnrar_OpenError(lngErrorNo As Long, strErrorMsg As String, strFilename As String)
    Debug.Print "OpenError: " & strFilename & " - " & strErrorMsg
End Sub

Private Sub objTKUnrar_ProcessError(lngErrorNo As Long, strErrorMsg As String, strFilename As String)
    Debug.Print "ProcessError: " & strFilename & " - " & strErrorMsg
End Sub

Private Sub objTKUnrar_CommentFound(strFilename As String, strComment As String)
    Debug.Print "CommentFound: " & strFilename & "-" & strComment
End Sub

Private Sub objTKUnrar_ProcessingFile(strFilename As String, dblFileSize As Double, datFileTime As Date, intMajorVer As Integer, intMinorVer As Integer, lngPackingMethod As Long, lngFileAttr As Long)
    Debug.Print "ProcessingFile: " & strFilename & " " & dblFileSize & " bytes"
    ProgressBar1.Value = 0
End Sub

Private Sub objTKUnrar_Progress(dblFileSize As Double, dblExtracted As Double)
    Debug.Print dblFileSize, dblExtracted, dblExtracted / dblFileSize * 100
    ProgressBar1.Value = dblExtracted / dblFileSize * 100
End Sub
