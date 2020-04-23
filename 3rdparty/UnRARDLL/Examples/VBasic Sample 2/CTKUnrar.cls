VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CTKUnrar"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit

'// ADDED
'// Converts strings from Unicode to OEM encoding to make sure
'// certain characters in paths are handled properly by RARProcessFile
Private Declare Sub CharToOem Lib "user32" Alias "CharToOemA" _
 (ByVal StrFrom As String, ByVal StrTo As String)

'// ADDED
'// For use by LongToUnsigned
Private Const OFFSET_4 = 4294967296#

Event OpenError(lngErrorNo As Long, strErrorMsg As String, strFilename As String)
Event ProcessError(lngErrorNo As Long, strErrorMsg As String, strFilename As String)
Event CommentError(lngErrorNo As Long, strErrorMsg As String, strFilename As String)
Event CommentFound(strFilename As String, strComment As String)
Event ProcessingFile(strFilename As String, dblFileSize As Double, datFileTime As Date, intMajorVer As Integer, intMinorVer As Integer, lngPackingMethod As Long, lngFileAttr As Long)
Event NextVolumne(strFilename As String, ByRef blnContinue As Boolean)
Event MissingFile(strFilename As String, ByRef strNewFilename As String)
Event MissingPassword(ByRef strNewPassword As String)
Event Progress(dblFileSize As Double, dblExtracted As Double)

' Private variables
Private m_dblLastFileSize As Double
Private m_dblLastFileUnpacked As Double
Private m_intLastPercentStep As Integer
Private m_blnPasswordProtected As Boolean
' Properties
Private m_intProgressSteps As Integer
Private m_strLastErrorMsg As String

Public Function LongToUnsigned(Value As Long) As Double
    '// ADDED
    '// This functions makes sure the unsigned integers
    '// that unrar.dll returns are handled properly as
    '// VB does not support unsigned integers natively
    '// See KB article 189323 for more info
    '// http://support.microsoft.com/?kbid=189323
    '
    'The function takes an unsigned Long from an API and
    'converts it to a Double for display or arithmetic purposes
    '
    If Value < 0 Then
        LongToUnsigned = Value + OFFSET_4
    Else
        LongToUnsigned = Value
    End If
    '
End Function
Private Sub Class_Initialize()
    m_intProgressSteps = 4
    m_strLastErrorMsg = ""
    m_blnPasswordProtected = False
End Sub

Private Sub Class_Terminate()
'
End Sub

Public Function ListRarFiles(strRarFile As String, arrstrFilenames() As String, Optional strPassword As String = "") As Boolean
    ListRarFiles = HandleRarFiles(strRarFile, OP_LIST, arrstrFilenames, , strPassword)
End Function

Public Function TestRarFiles(strRarFile As String, Optional strPassword As String = "") As Boolean
    TestRarFiles = HandleRarFiles(strRarFile, OP_TEST, , , strPassword)
End Function

Public Function ExtractRarFiles(strRarFile As String, strDestPath As String, Optional strPassword As String = "") As Boolean
    ExtractRarFiles = HandleRarFiles(strRarFile, OP_EXTRACT, , strDestPath, strPassword)
End Function

Private Function HandleRarFiles(strRarFile As String, opMode As RarOperations, Optional arrstrFilenames As Variant, Optional strDestPath As String = "", Optional strPassword As String = "") As Boolean
    Dim uRAR As RAROpenArchiveData
    Dim uHeaderEx As RARHeaderDataEx
    Dim lngRarHandle As Long
    Dim intStatus As Integer
    Dim intReturn As Integer
    Dim strCurrentFilename As String
    Dim strLastFilename As String
    Dim lngpFilenames As Long
    Dim strTmp As String

    HandleRarFiles = False

    '// ADDED
    '// See API declaration
    CharToOem strDestPath, strDestPath

    ' Fill the rar header structure
    uRAR.ArcName = strRarFile
    uRAR.CmtBuf = Space(16384)
    uRAR.CmtBufSize = 16384
    If opMode = OP_LIST Then
        uRAR.OpenMode = RAR_OM_LIST
    ElseIf opMode = OP_TEST Or opMode = OP_EXTRACT Then
        uRAR.OpenMode = RAR_OM_EXTRACT
    Else
        RaiseEvent ProcessError(ERAR_UNKNOWN_ERROR, "Unknown extracion mode", strRarFile)
        m_strLastErrorMsg = "Unknown extracion mode"
        Exit Function
    End If

    If opMode = OP_LIST Then
        lngpFilenames = 0
        If UBound(arrstrFilenames) = 0 Then
            RaiseEvent ProcessError(ERAR_DEST_ARR_TO_SMALL, "Supplied array to small to fit all files", strCurrentFilename)
            m_strLastErrorMsg = "Supplied array to small to fit all files"
            Exit Function
        End If
    End If

    ' Open the archive
    lngRarHandle = RAROpenArchive(uRAR)
    If uRAR.OpenResult <> 0 Then
        ' Something went wrong
        Call OpenError(uRAR.OpenResult, strRarFile)
    Else
        ' Set the callback
        Call RARSetCallback(lngRarHandle, AddressOf MyCallBack, ObjPtr(Me))
        ' If the user supplied a password, set it
        If Len(strPassword) > 0 Then
            Call RARSetPassword(lngRarHandle, strPassword)
            m_blnPasswordProtected = True
        End If
        If uRAR.CmtState = 0 Then
            ' No comment
        ElseIf uRAR.CmtState = 1 Then
            ' Comment
            RaiseEvent CommentFound(strRarFile, uRAR.CmtBuf)
        Else
            Call CommentError(uRAR.CmtState, strRarFile)
        End If

        strLastFilename = ""
        m_dblLastFileSize = 0
        m_dblLastFileUnpacked = 0
        m_intLastPercentStep = 0
        ' Start processing the files
        intStatus = RARReadHeaderEx(lngRarHandle, uHeaderEx)
        strTmp = Left(uHeaderEx.ArcName, InStr(1, uHeaderEx.ArcName, vbNullChar) - 1)
        Call NextVolume(strTmp)
        Do Until intStatus <> 0
            ' Get the current filename
            strCurrentFilename = Left(uHeaderEx.FileName, InStr(1, uHeaderEx.FileName, vbNullChar) - 1)
            
            '// ADDED
            '// Makes sure unsigned ints are handled properly
            '// see function LongToUnsigned for more info
            UnpSizeHighEx = LongToUnsigned(uHeaderEx.UnpSizeHigh)
            UnpSizeEx = LongToUnsigned(uHeaderEx.UnpSize)
            
            If strLastFilename <> strCurrentFilename Then
                m_dblLastFileSize = MakeDouble(uHeaderEx.UnpSizeHigh, uHeaderEx.UnpSize)
                m_dblLastFileUnpacked = 0
                m_intLastPercentStep = 0
                RaiseEvent ProcessingFile(strCurrentFilename, m_dblLastFileSize, Now(), uHeaderEx.UnpVer / 10, uHeaderEx.UnpVer Mod 10, uHeaderEx.Method, uHeaderEx.FileAttr)
                strLastFilename = strCurrentFilename
                If opMode = OP_LIST Then
                    If lngpFilenames > UBound(arrstrFilenames) Then
                        RaiseEvent ProcessError(ERAR_DEST_ARR_TO_SMALL, "Supplied array to small to fit all files", strCurrentFilename)
                        m_strLastErrorMsg = "Supplied array to small to fit all files"
                        HandleRarFiles = False
                        Exit Do
                    End If
                    arrstrFilenames(lngpFilenames) = strCurrentFilename
                    lngpFilenames = lngpFilenames + 1
                End If
            End If
            intReturn = ERAR_UNKNOWN_ERROR
            If opMode = OP_LIST Then
                intReturn = RARProcessFile(lngRarHandle, RAR_SKIP, "", "")
            ElseIf opMode = OP_TEST Then
                intReturn = RARProcessFile(lngRarHandle, RAR_TEST, "", "")
            ElseIf opMode = OP_EXTRACT Then
                intReturn = RARProcessFile(lngRarHandle, RAR_EXTRACT, strDestPath, "")
            End If
            If intReturn = 0 Then
                HandleRarFiles = True
            Else
                ' Handle error and quit
                Call ProcessError(CLng(intReturn), strCurrentFilename)
                HandleRarFiles = False
                Exit Do
            End If
            ' Handle next file
            intStatus = RARReadHeaderEx(lngRarHandle, uHeaderEx)
        Loop
'        RaiseEvent Progress(m_dblLastFileUnpacked, m_dblLastFileUnpacked)
        Call RARCloseArchive(lngRarHandle)
    End If
End Function

Friend Sub ProcessingLength(lngBufferSize As Long)
    ' Keep control of how much data we have extracted
    m_dblLastFileUnpacked = m_dblLastFileUnpacked + lngBufferSize
    If m_dblLastFileSize <> 0 Then
        ' Send an event to the listener whenever we reach the next progress step (0%, x%, 2x%, 3x% etc)
        If CInt(m_dblLastFileUnpacked / m_dblLastFileSize * 100) > m_intLastPercentStep + m_intProgressSteps Then
            RaiseEvent Progress(m_dblLastFileSize, m_dblLastFileUnpacked)
            ' Keep track of last percentage step
            m_intLastPercentStep = m_intLastPercentStep + m_intProgressSteps
        End If
    End If
End Sub

Friend Function NextVolume(strFilename As String) As Boolean
    Dim blnContinue As Boolean

    RaiseEvent NextVolumne(strFilename, blnContinue)
    NextVolume = blnContinue
End Function

Friend Function FindMissingFile(strFilename As String) As String
    Dim strNewFile As String

    RaiseEvent MissingFile(strFilename, strNewFile)
    FindMissingFile = strNewFile
End Function

Friend Function MissingPassword() As String
    Dim strNewPassword As String

    m_blnPasswordProtected = True
    RaiseEvent MissingPassword(strNewPassword)
    MissingPassword = strNewPassword
End Function

Private Sub CommentError(lngErrorNum As Long, strRarName As String)
    Dim strErrorMsg As String

    Select Case lngErrorNum
        Case ERAR_NO_MEMORY
            strErrorMsg = "Not enough memory"
        Case ERAR_BAD_DATA:
            strErrorMsg = "Broken comment"
        Case ERAR_UNKNOWN_FORMAT:
            strErrorMsg = "Unknown comment format"
        Case ERAR_SMALL_BUF:
            strErrorMsg = "Buffer too small, comments not completely read"
        Case Else
            strErrorMsg = "Unknown error"
    End Select
    m_strLastErrorMsg = strErrorMsg
    RaiseEvent CommentError(lngErrorNum, strErrorMsg, strRarName)
End Sub

Private Sub OpenError(lngErrorNum As Long, strRarName As String)
    Dim strErrorMsg As String

    Select Case lngErrorNum
        Case ERAR_NO_MEMORY
            strErrorMsg = "Not enough memory"
        Case ERAR_EOPEN:
            strErrorMsg = "Cannot open file"
        Case ERAR_BAD_ARCHIVE:
            strErrorMsg = "File is not RAR archive"
        Case ERAR_BAD_DATA:
            strErrorMsg = "Archive header broken"
        Case Else
            strErrorMsg = "Unknown error"
    End Select
    m_strLastErrorMsg = strErrorMsg
    RaiseEvent OpenError(lngErrorNum, strErrorMsg, strRarName)
End Sub

Private Sub ProcessError(lngErrorNum As Long, strRarName As String)
    Dim strErrorMsg As String

    Select Case lngErrorNum
        Case ERAR_UNKNOWN_FORMAT
            strErrorMsg = "Unknown archive format"
        Case ERAR_BAD_ARCHIVE:
            strErrorMsg = "Bad volume"
        Case ERAR_ECREATE:
            strErrorMsg = "File create error"
        Case ERAR_EOPEN:
            strErrorMsg = "Volume open error"
        Case ERAR_ECLOSE:
            strErrorMsg = "File close error"
        Case ERAR_EREAD:
            strErrorMsg = "Read error"
        Case ERAR_EWRITE:
            strErrorMsg = "Write error"
        Case ERAR_BAD_DATA:
            If m_blnPasswordProtected Then
                strErrorMsg = "CRC error - Wrong password?"
            Else
                strErrorMsg = "CRC error"
            End If
        Case Else
            strErrorMsg = "Unknown error"
    End Select
    m_strLastErrorMsg = strErrorMsg
    RaiseEvent ProcessError(lngErrorNum, strErrorMsg, strRarName)
End Sub

' The intProgressSteps property
Property Get intProgressSteps() As Integer
    intProgressSteps = m_intProgressSteps
End Property
Property Let intProgressSteps(ByVal newValue As Integer)
    m_intProgressSteps = newValue
End Property

' The strLastErrorMsg property
Property Get strLastErrorMsg() As String
    strLastErrorMsg = m_strLastErrorMsg
End Property
