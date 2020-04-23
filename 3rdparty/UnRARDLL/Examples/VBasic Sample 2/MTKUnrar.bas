Attribute VB_Name = "MTKUnrar"
Option Explicit

Const RAR_DLL_VERSION As Integer = 2
Const RAR_CONTINUE As Integer = 1
Const RAR_ABORT As Integer = -1

Public Enum ERAR
    ERAR_END_ARCHIVE = 10
    ERAR_NO_MEMORY
    ERAR_BAD_DATA
    ERAR_BAD_ARCHIVE
    ERAR_UNKNOWN_FORMAT
    ERAR_EOPEN
    ERAR_ECREATE
    ERAR_ECLOSE
    ERAR_EREAD
    ERAR_EWRITE
    ERAR_SMALL_BUF
    ' Private error definitions
    ERAR_DEST_ARR_TO_SMALL = 5000
    ERAR_UNKNOWN_DESTINATION
    ERAR_UNKNOWN_ERROR
End Enum

Public Enum RAR_OM
    RAR_OM_LIST = 0
    RAR_OM_EXTRACT
End Enum
 
Public Enum RAR
    RAR_SKIP = 0
    RAR_TEST
    RAR_EXTRACT
End Enum

Public Enum UCM
    UCM_CHANGEVOLUME = 0
    UCM_PROCESSDATA
    UCM_NEEDPASSWORD
End Enum

Public Enum RAR_VOL
    RAR_VOL_ASK = 0
    RAR_VOL_NOTIFY
End Enum

Public Enum RarOperations
    OP_EXTRACT = 0
    OP_TEST
    OP_LIST
End Enum

Public Type RARHeaderData
    ArcName As String * 260
    FileName As String * 260
    flags As Long
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

Public Type RARHeaderDataEx
    ArcName As String * 1024
    ArcNameW As String * 2048
    FileName As String * 1024
    FileNameW As String * 2048
    flags As Long
    PackSize As Long
    PackSizeHigh As Long
    UnpSize As Long
    UnpSizeHigh As Long
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
    Reserved(1024) As Integer
End Type

Public Type RAROpenArchiveData
    ArcName As String
    OpenMode As Long
    OpenResult As Long
    CmtBuf As String
    CmtBufSize As Long
    CmtSize As Long
    CmtState As Long
End Type

Public Declare Function RAROpenArchive Lib "unrar.dll" (ByRef ArchiveData As RAROpenArchiveData) As Long
Public Declare Function RARCloseArchive Lib "unrar.dll" (ByVal hArcData As Long) As Long
Public Declare Function RARReadHeader Lib "unrar.dll" (ByVal hArcData As Long, ByRef HeaderData As RARHeaderData) As Long
Public Declare Function RARReadHeaderEx Lib "unrar.dll" (ByVal hArcData As Long, ByRef HeaderData As RARHeaderDataEx) As Long
Public Declare Function RARProcessFile Lib "unrar.dll" (ByVal hArcData As Long, ByVal Operation As Long, ByVal DestPath As String, ByVal DestName As String) As Long
Public Declare Sub RARSetCallback Lib "unrar.dll" (ByVal hArcData As Long, ByVal CallbackProc As Long, ByVal UserData As Long)
Public Declare Sub RARSetPassword Lib "unrar.dll" (ByVal hArcData As Long, ByVal Password As String)
Public Declare Function RARGetDllVersion Lib "unrar.dll" () As Long

Private Declare Sub CopyMemory Lib "kernel32" Alias "RtlMoveMemory" (lpDest As Any, lpSource As Any, ByVal cBytes As Long)
Private Declare Function lstrlenW Lib "kernel32" (ByVal lpString As Long) As Long
Private Declare Function lstrlenA Lib "kernel32" (ByVal lpString As Long) As Long
'

Public Function MakeDouble(ByVal HiWord As Long, ByVal LoWord As Long) As Double
    MakeDouble = CDbl(LoWord) + CDbl(HiWord) * 4294967296#
End Function

Public Function MyCallBack(ByVal msg As Long, ByVal UserData As Long, ByVal P1 As Long, ByVal P2 As Long) As Integer
    Dim strPassword As String
    Dim strNewVolume As String

    If msg = UCM_CHANGEVOLUME Then
        If P2 = RAR_VOL_ASK Then
            strNewVolume = CallFunctionFromUserdata(UserData).FindMissingFile(PointerToStringA(P1)) & Chr(0)
            If Len(strNewVolume) > 1 Then
                CopyMemory ByVal P1, ByVal StrPtr(StrConv(strNewVolume, vbFromUnicode)), Len(strNewVolume)
                MyCallBack = RAR_CONTINUE
            Else
                MyCallBack = RAR_ABORT
            End If
        ElseIf P2 = RAR_VOL_NOTIFY Then
            If CallFunctionFromUserdata(UserData).NextVolume(PointerToStringA(P1)) Then
                MyCallBack = RAR_CONTINUE
            Else
                MyCallBack = RAR_ABORT
            End If
        End If

    ElseIf msg = UCM_PROCESSDATA Then
        Call CallFunctionFromUserdata(UserData).ProcessingLength(P2)
'        Debug.Print "Size of data = " & P2
'        Debug.Print HexDump(P1, P2)
        MyCallBack = RAR_CONTINUE

    ElseIf msg = UCM_NEEDPASSWORD Then
        strPassword = CallFunctionFromUserdata(UserData).MissingPassword() & Chr(0)
        If Len(strPassword) = 1 Then
            ' If the user supplies an empty password, we have to generate a fake one, else we
            ' won't receieve a CRC error message from RARProcessFile. Bug?
            strPassword = "CTKUnrar" & Now() & Chr(0)
        End If
        CopyMemory ByVal P1, ByVal StrPtr(StrConv(strPassword, vbFromUnicode)), IIf(Len(strPassword) > P2, P2, Len(strPassword))
    Else
        Debug.Print "Unknown msg"
    End If
End Function

' resolve the passed object pointer into an object reference.
' DO NOT PRESS THE "STOP" BUTTON WHILE IN THIS PROCEDURE!
Private Function CallFunctionFromUserdata(ByVal UserData As Long) As CTKUnrar
    Dim CwdEx As CTKUnrar

    CopyMemory CwdEx, UserData, 4&
    Set CallFunctionFromUserdata = CwdEx
    CopyMemory CwdEx, 0&, 4&
End Function

Public Function PointerToStringW(ByVal lpString As Long) As String
   Dim sText As String
   Dim lLength As Long

   If lpString Then
      lLength = lstrlenW(lpString)
      If lLength Then
         sText = Space$(lLength)
         CopyMemory ByVal StrPtr(sText), ByVal lpString, lLength * 2
      End If
   End If
   PointerToStringW = sText
End Function

Public Function IsUnicode(s As String) As Boolean
    IsUnicode = Not (Len(s) = LenB(s))
End Function

Public Function PointerToStringA(lpStringA As Long) As String
   Dim Buffer() As Byte
   Dim nLen As Long

   If lpStringA Then
      nLen = lstrlenA(ByVal lpStringA)
      If nLen Then
         ReDim Buffer(0 To (nLen - 1)) As Byte
         CopyMemory Buffer(0), ByVal lpStringA, nLen
         PointerToStringA = StrConv(Buffer, vbUnicode)
      End If
   End If
End Function

Public Function PointerToDWord(ByVal lpDWord As Long) As Long
   Dim nRet As Long
   If lpDWord Then
      CopyMemory nRet, ByVal lpDWord, 4
      PointerToDWord = nRet
   End If
End Function

Private Function HexDump(ByVal lpBuffer As Long, ByVal nBytes As Long) As String
   Dim i As Long, j As Long
   Dim ba() As Byte
   Dim sRet As String
   Dim dBytes As Long
   
   ' Size recieving buffer as requested,
   ' then sling memory block to buffer.
   ReDim ba(0 To nBytes - 1) As Byte
   Call CopyMemory(ba(0), ByVal lpBuffer, nBytes)
   sRet = String(81, "=") & vbCrLf & _
          "lpBuffer = &h" & Hex$(lpBuffer) & _
          "   nBytes = " & nBytes
   
   ' Buffer may well not be even multiple of 16.
   ' If not, we need to round up.
   If nBytes Mod 16 = 0 Then
      dBytes = nBytes
   Else
      dBytes = ((nBytes \ 16) + 1) * 16
   End If
   
   ' Loop through buffer, displaying 16 bytes per
   ' row. Preface with offset, trail with ASCII.
   For i = 0 To (dBytes - 1)
      ' Add address and offset from beginning
      ' if at the start of new row.
      If (i Mod 16) = 0 Then
         sRet = sRet & vbCrLf & Right$("00000000" _
                & Hex$(lpBuffer + i), 8) & "  " & _
                Right$("0000" & Hex$(i), 4) & "  "
      End If
      
      ' Append this byte.
      If i < nBytes Then
         sRet = sRet & Right$("0" & Hex(ba(i)), 2)
      Else
         sRet = sRet & "  "
      End If
      
      ' Special handling...
      If (i Mod 16) = 15 Then
         ' Display last 16 characters in
         ' ASCII if at end of row.
         sRet = sRet & "  "
         For j = (i - 15) To i
            If j < nBytes Then
               If ba(j) >= 32 And ba(j) <= 126 Then
                  sRet = sRet & Chr$(ba(j))
               Else
                  sRet = sRet & "."
               End If
            End If
         Next j
      ElseIf (i Mod 8) = 7 Then
         ' Insert hyphen between 8th and
         ' 9th bytes of hex display.
         sRet = sRet & "-"
      Else
         ' Insert space between other bytes.
         sRet = sRet & " "
      End If
   Next i
   HexDump = sRet & vbCrLf & String(81, "=") & vbCrLf
End Function
