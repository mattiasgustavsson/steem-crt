'**************************************************************************************
'
'  Wrapper class for unrar.dll
'  If anyone finds any problems, I'd appreciate an email.
'
'  marcel_madonna@ajmsoft.com
'
'  If you make changes, remember that GC tends to interfere with callbacks from
'  unmanaged code.  Test very very larges archives (1GB+), archives with many 
'  files (5000+), archives on separate volumes and password protected archives
'
'  CHANGE HISTORY
'
'  06/01/2010
'  Added logic to load 64-bit version of unrar (unrar64.dll) when running on
'  a 64-bit OS.
'
'
'  Added another parameter to the Unpacking EVENT to allow Cancel
'  Added an error code for cancel
'  Changed Error code to a public enum
'  Enabled progress tracking when unpacking by file
'
'**************************************************************************************

Imports System.Runtime.InteropServices
Imports System.Text

Public Class Decompressor

    'generate an event after each file is unpacked
    Public Event OnUnpack(ByVal r As RAREntry)
    Public Shared Event Unpacking(ByVal fTotalSize As Long, ByVal fUnpackedSize As Long, ByRef Disposition As RarDisposition)

    Public Enum RarErrors
        ERAR_END_ARCHIVE = 10
        ERAR_NO_MEMORY = 11
        ERAR_BAD_DATA = 12
        ERAR_BAD_ARCHIVE = 13
        ERAR_UNKNOWN_FORMAT = 14
        ERAR_EOPEN = 15
        ERAR_ECREATE = 16
        ERAR_ECLOSE = 17
        ERAR_EREAD = 18
        ERAR_EWRITE = 19
        ERAR_SMALL_BUF = 20
        ERAR_CANCELLED = 21
    end enum

    Const RAR_OM_LIST As Integer = 0
    Const RAR_OM_EXTRACT As Integer = 1

    Const RAR_SKIP As Integer = 0
    Const RAR_TEST As Integer = 1
    Const RAR_EXTRACT As Integer = 2

    Const UCM_CHANGEVOLUME As Integer = 0
    Const UCM_PROCESSDATA As Integer = 1
    Const UCM_NEEDPASSWORD As Integer = 2

    Const RAR_VOL_ASK As Integer = 0
    Const RAR_VOL_NOTIFY As Integer = 1

    Public Enum RarOperations
        OP_EXTRACT = 0
        OP_TEST = 1
        OP_LIST = 2
    End Enum

    Public Enum RarDisposition
        OP_CANCEL = -1
        OP_CONTINUE = +1
    End Enum

    <StructLayout(LayoutKind.Sequential, CharSet:=CharSet.Ansi)> _
    Public Structure RARHeaderData          'mdm 06/01/2010 made Public
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=260)> _
        Public ArcName As String
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=260)> _
        Public FileName As String
        Public Flags As Integer
        Public PackSize As Integer
        Public UnpSize As Integer
        Public HostOS As Integer
        Public FileCRC As Integer
        Public FileTime As Integer
        Public UnpVer As Integer
        Public Method As Integer
        Public FileAttr As Integer
        Public CmtBuf As String
        Public CmtBufSize As Integer
        Public CmtSize As Integer
        Public CmtState As Integer
    End Structure

    <StructLayout(LayoutKind.Sequential, CharSet:=CharSet.Ansi)> _
    Public Structure RARHeaderDataEx        'mdm 06/01/2010 made Public
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=1024)> _
        Public ArcName As String
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=2048)> _
        Public ArcNameW As Byte()
        <MarshalAs(UnmanagedType.ByValTStr, SizeConst:=1024)> _
        Public FileName As String
        <MarshalAs(UnmanagedType.ByValArray, SizeConst:=2048)> _
        Public FileNameW As Byte()
        Public Flags As Integer
        Public PackSize As Integer
        Public PackSizeHigh As Integer
        Public UnpSize As Integer
        Public UnpSizeHigh As Integer
        Public HostOS As Integer
        Public FileCRC As Integer
        Public FileTime As Integer
        Public UnpVer As Integer
        Public Method As Integer
        Public FileAttr As Integer
        Public CmtBuf As String
        Public CmtBufSize As Integer
        Public CmtSize As Integer
        Public CmtState As Integer
        <VBFixedArray(1024)> Public Reserved As Byte
    End Structure

    <StructLayout(LayoutKind.Sequential)> Public Structure RAROpenArchiveData
        <VBFixedString(260)> Public ArcName As String
        Public OpenMode As Integer
        Public OpenResult As Integer
        Public CmtBuf As String
        Public CmtBufSize As Integer
        Public CmtSize As Integer
        Public CmtState As Integer
    End Structure

    <StructLayout(LayoutKind.Sequential)> Public Structure RAROpenArchiveDataEx
        <VBFixedString(1024)> Public ArcName As String
        <VBFixedArray(2048)> Public ArcNameW As Byte()
        Public OpenMode As Integer
        Public OpenResult As Integer
        Public CmtBuf As String
        Public CmtBufSize As Integer
        Public CmtSize As Integer
        Public CmtState As Integer
        Public Flags As Integer
        <VBFixedArray(32)> Public Reserved As Byte
    End Structure

    'mdm 06/01/2010
    ' Add vectors for each routine in Unrar
    Private _delRARReadHeader As dRARReadHeader
    Private _delRARReadHeaderEx As dRARReadHeaderEx
    Private _delRAROpenArchive As dRAROpenArchive
    Private _delRAROpenArchiveEx As dRAROpenArchiveEx
    Private _delRARCloseArchive As dRARCloseArchive
    Private _delRARProcessFile As dRARProcessFile
    Private _delRARSetPassword As dRARSetPassword
    Private _delRARGetDllVersion As dRARGetDllVersion
    Private _delRARSetCallBack As dRARSetCallBack

    Const KERNEL As String = "kernel32.dll"         'mdm 06/01/2010

    'mdm 06/01/2010
    ' Add definitions for needed kernel routines
    <DllImport(KERNEL)> _
    Private Shared Function LoadLibraryEx(ByVal dllFilePath As String, ByVal hFile As IntPtr, ByVal dwFlags As Integer) As IntPtr
    End Function

    <DllImport(KERNEL)> _
    Private Shared Function FreeLibrary(ByVal dllPointer As IntPtr) As Boolean
    End Function

    <DllImport(KERNEL, CharSet:=CharSet.Ansi)> _
    Private Shared Function GetProcAddress(ByVal dllPointer As IntPtr, ByVal functionName As String) As IntPtr
    End Function

    ' mdm 06/01/2010 
    ' Redefined all Function declarations to use delegates to allow 
    ' dynamic load of the unrar library and avoid static calls 
    '
    'Const MYLIB As String = "E:\MyProjects\Utilities\UnRAR\debug\unrar.dll"
    '<DllImport(MYLIB, CharSet:=CharSet.Ansi, CallingConvention:=CallingConvention.StdCall)> _
    'Private Shared Function RARReadHeader(ByVal hArcData As IntPtr, ByRef HeaderData As RARHeaderData) As Integer
    'End Function
    <UnmanagedFunctionPointer(CallingConvention.StdCall)> _
    Public Delegate Function dRARReadHeader(ByVal hArcData As IntPtr, ByRef HeaderData As RARHeaderData) As Integer
    Private Function RARReadHeader(ByVal hArcData As IntPtr, ByRef HeaderData As RARHeaderData) As Integer
        Return _delRARReadHeader.Invoke(hArcData, HeaderData)
    End Function


    '<DllImport(MYLIB, CharSet:=CharSet.Ansi, CallingConvention:=CallingConvention.StdCall)> _
    'Private Shared Function RARReadHeaderEx(ByVal hArcData As IntPtr, ByRef HeaderData As RARHeaderDataEx) As Integer
    'End Function
    Public Delegate Function dRARReadHeaderEx(ByVal hArcData As IntPtr, ByRef HeaderData As RARHeaderDataEx) As Integer
    Private Function RARReadHeaderEx(ByVal hArcData As IntPtr, ByRef HeaderData As RARHeaderDataEx) As Integer
        Return _delRARReadHeaderEx.Invoke(hArcData, HeaderData)
    End Function

    '<DllImport(MYLIB, CharSet:=CharSet.Ansi, CallingConvention:=CallingConvention.StdCall)> _
    'Private Shared Function RAROpenArchive(ByRef ArchiveData As RAROpenArchiveData) As IntPtr
    'End Function
    Public Delegate Function dRAROpenArchive(ByRef ArchiveData As RAROpenArchiveData) As IntPtr
    Private Function RARRAROpenArchive(ByRef ArchiveData As RAROpenArchiveData) As Integer
        Return _delRAROpenArchive.Invoke(archiveData)
    End Function

    '<DllImport(MYLIB, CharSet:=CharSet.Ansi, CallingConvention:=CallingConvention.StdCall)> _
    'Private Shared Function RAROpenArchiveEx(ByRef ArchiveData As RAROpenArchiveDataEx) As IntPtr
    'End Function
    Private Delegate Function dRAROpenArchiveEx(ByRef ArchiveData As RAROpenArchiveDataEx) As IntPtr
    Private Function RAROpenArchiveEx(ByRef ArchiveData As RAROpenArchiveDataEx) As IntPtr
        Return _delRAROpenArchiveEx.Invoke(ArchiveData)
    End Function

    '<DllImport(MYLIB, CharSet:=CharSet.Ansi, CallingConvention:=CallingConvention.StdCall)> _
    'Private Shared Function RARCloseArchive(ByVal hArcData As IntPtr) As Integer
    'End Function
    Private Delegate Function dRARCloseArchive(ByVal hArcData As IntPtr) As Integer
    Private Function RARCloseArchive(ByVal hArcData As IntPtr) As Integer
        Return _delRARCloseArchive.Invoke(hArcData)
    End Function

    '<DllImport(MYLIB, CharSet:=CharSet.Ansi, CallingConvention:=CallingConvention.StdCall)> _
    'Private Shared Function RARProcessFile(ByVal hArcData As IntPtr, ByVal Operation As Integer, ByVal DestPath As String, ByVal DestName As String) As Integer
    'End Function
    Private Delegate Function dRARProcessFile(ByVal hArcData As IntPtr, ByVal Operation As Integer, ByVal DestPath As String, ByVal DestName As String) As Integer
    Private Function RARProcessFile(ByVal hArcData As IntPtr, ByVal Operation As Integer, ByVal DestPath As String, ByVal DestName As String) As Integer
        Return _delRARProcessFile.Invoke(hArcData, Operation, DestPath, DestName)
    End Function

    '<DllImport(MYLIB, CharSet:=CharSet.Ansi, CallingConvention:=CallingConvention.StdCall)> _
    'Private Shared Sub RARSetPassword(ByVal hArcData As IntPtr, ByVal Password As String)
    'End Sub
    Private Delegate Sub dRARSetPassword(ByVal hArcData As IntPtr, ByVal Password As String)
    Private Sub RARSetPassword(ByVal hArcData As IntPtr, ByVal Password As String)
        _delRARSetPassword.Invoke(hArcData, Password)
    End Sub

    '<DllImport(MYLIB, CharSet:=CharSet.Ansi, CallingConvention:=CallingConvention.StdCall)> _
    '  Private Shared Function RARGetDllVersion() As Integer
    'End Function
    Private Delegate Function dRARGetDllVersion() As Integer
    Private Function RARGetDllVersion() As Integer
        Return _delRARGetDllVersion.Invoke()
    End Function

    'Public Delegate Function RARCallBack(ByVal msg As Integer, ByVal userdata As Integer, ByVal P1 As IntPtr, ByVal P2 As Integer) As Integer
    '<DllImport(MYLIB, CharSet:=CharSet.Ansi, CallingConvention:=CallingConvention.StdCall)> _
    'Private Shared Sub RARSetCallback(ByVal hArcData As IntPtr, ByVal EP As RARCallBack, ByVal UserData As Long)
    'End Sub
    Public Delegate Function RARCallBack(ByVal msg As Integer, ByVal userdata As Integer, ByVal P1 As IntPtr, ByVal P2 As Integer) As Integer
    Public Delegate Sub dRARSetCallBack(ByVal hArcData As IntPtr, ByVal EP As RARCallBack, ByVal UserData As Long)
    Private Sub RARSetCallback(ByVal hArcData As IntPtr, ByVal EP As RARCallBack, ByVal UserData As Long)
        _delRARSetCallBack(hArcData, EP, UserData)
    End Sub

    Public Structure RARHead
        Public ArcName As String
        Public ArcNameW As String
        Public CmtBuf As String
        Public CmtBufSize As Integer
        Public CmtSize As Integer
        Public CmtState As Integer
    End Structure

    Public Structure RAREntry
        Public FileName As String
        Public FileNameW As String
        Public Flags As Integer
        Public PackSize As Long
        Public UnpSize As Long
        Public HostOS As Integer
        Public FileCRC As Integer
        Public FileTime As Integer
        Public UnpVer As Integer
        Public Method As Integer
        Public FileAttr As Integer
    End Structure

    ' Local Variables
    Private mDllVersion As Integer
    Private mRARFiles As Collection
    Private mheader As RARHead
    Private mRARFile As String
    Private mPassword As String
    Private mTotalArchiveSize As Long
    Private mTotalPackedSize As Long
    Private mRARHandle As IntPtr

    'List of files in the archive
    ReadOnly Property RARFiles() As Collection
        Get
            Return mRARFiles
        End Get
    End Property

    'Archive header
    ReadOnly Property RARHeader() As RARHead
        Get
            Return mheader
        End Get
    End Property

    'Total Files
    ReadOnly Property RARTotalFiles() As Integer
        Get
            Return mRARFiles.Count
        End Get
    End Property

    'Total File Size
    ReadOnly Property TotalArchiveSize() As Long
        Get
            Return mTotalArchiveSize
        End Get
    End Property


    Private uRAREx As RAROpenArchiveDataEx
    Private uHeaderEx As RARHeaderDataEx

    Public Sub New(ByVal RARFile As String)
        mRARFile = RARFile
        Init()
    End Sub

    Public Sub New(ByVal RARFile As String, ByVal Password As String)
        mRARFile = RARFile
        mPassword = Password
        Init()
    End Sub

    Public Function UnPack(ByVal fName As String, ByVal fDest As String) As Boolean
        Dim sz(0) As String
        sz(0) = fName
        Return ExtractFile(sz, fDest)
    End Function

    Public Function UnPack(ByVal fName As String(), ByVal fDest As String) As Boolean
        Return ExtractFile(fName, fDest)
    End Function

    Public Function UnPackAll(ByVal fDest As String) As Boolean
        Return ExtractAll(fDest)
    End Function

    Public Function TestArchive(ByVal fName As String) As Boolean
        Dim sz(0) As String
        sz(0) = fName
        Return TestFile(sz)
    End Function

    Public Function TestArchive(ByVal fName As String()) As Boolean
        Return TestFile(fName)
    End Function

    Public Function TestAll() As Boolean
        Return TestAllFiles()
    End Function

    Private Sub Init()

        'mdm 06/01/2010
        ' Load the appropriate version of unrar.dll the routine
        ' will throw an exception if it's not found
        Try
            LoadLibrary()
        Catch ex As Exception
            MsgBox(ex.Message)
            mDllVersion = -1    'indicate error
        End Try

        Try
            mDllVersion = RARGetDllVersion()
        Catch ex As Exception
            mDllVersion = -1    'most likely entry point not found
        End Try

        uRAREx.CmtBuf = Space(16384)
        uRAREx.CmtBufSize = 16384
        uRAREx.ArcName = mRARFile

        uHeaderEx.CmtBuf = Space(16384)

        mRARFiles = New Collection
        LoadFileList()
        mTotalArchiveSize = 0
        For Each r As RAREntry In mRARFiles
            mTotalArchiveSize += r.UnpSize
        Next

    End Sub

    Private Sub LoadFileList()

        Dim iStatus As Integer
        Dim Ret As Integer

        uRAREx.OpenMode = RAR_OM_LIST

        mRARHandle = RAROpenArchiveEx(uRAREx)
        If uRAREx.OpenResult <> 0 Then OpenError(uRAREx.OpenResult, mRARFile)
        If mPassword <> "" Then RARSetPassword(mRARHandle, mPassword)
        Dim hr As New RARCallBack(AddressOf RARCB)
        Call RARSetCallback(mRARHandle, hr, 0)
        iStatus = RARReadHeaderEx(mRARHandle, uHeaderEx)
        mheader.ArcName = uHeaderEx.ArcName
        mheader.ArcNameW = Encoding.Unicode.GetString(uHeaderEx.ArcNameW)
        mheader.CmtBuf = uRAREx.CmtBuf
        mheader.CmtBufSize = uRAREx.CmtBufSize
        mheader.CmtSize = uRAREx.CmtSize
        mheader.CmtState = uRAREx.CmtState

        Do Until iStatus <> 0
            Dim RARE As RAREntry = FormatFileEntry()
            mRARFiles.Add(RARE)
            Ret = RARProcessFile(mRARHandle, RAR_SKIP, "", "")
            If Ret <> 0 Then ProcessError(Ret)
            iStatus = RARReadHeaderEx(mRARHandle, uHeaderEx)
        Loop
        RARCloseArchive(mRARHandle)
        GC.KeepAlive(hr)

    End Sub

    Private Function FormatFileEntry() As RAREntry
        Dim RARE As RAREntry
        RARE.FileName = uHeaderEx.FileName
        RARE.FileNameW = Encoding.Unicode.GetString(uHeaderEx.FileNameW)
        RARE.Flags = uHeaderEx.Flags
        RARE.PackSize = MakeLong(uHeaderEx.PackSize, uHeaderEx.PackSizeHigh)
        RARE.UnpSize = MakeLong(uHeaderEx.UnpSize, uHeaderEx.UnpSizeHigh)
        RARE.HostOS = uHeaderEx.HostOS
        RARE.FileCRC = uHeaderEx.FileCRC
        RARE.FileTime = uHeaderEx.FileTime
        RARE.UnpVer = uHeaderEx.UnpVer
        RARE.Method = uHeaderEx.Method
        RARE.FileAttr = uHeaderEx.FileAttr
        Return RARE

    End Function

    Private Function MakeLong(ByVal iLow As Integer, ByVal iHigh As Integer) As Long
        Return IIf(iLow >= 0, iLow, iLow + 4294967296) + (CLng(iHigh) << 32)
    End Function

    Private Function ExtractFile(ByVal szFile As String(), ByVal szDest As String) As Boolean
        Return ProcessFile(szFile, szDest, RAR_EXTRACT)
    End Function

    Private Function TestFile(ByVal szFile As String()) As Boolean
        Return ProcessFile(szFile, "", RAR_TEST)
    End Function

    Private Function ProcessFile(ByVal szFile As String(), ByVal szDest As String, ByVal RARMode As Integer) As Boolean
        Dim iStatus As Integer
        Dim Ret As Integer

        uRAREx.OpenMode = RAR_OM_EXTRACT

        mRARHandle = RAROpenArchiveEx(uRAREx)
        If uRAREx.OpenResult <> 0 Then OpenError(uRAREx.OpenResult, mRARFile)
        If mPassword <> "" Then RARSetPassword(mRARHandle, mPassword)
        Dim hr As New RARCallBack(AddressOf RARCB)
        Call RARSetCallback(mRARHandle, hr, 0)
        iStatus = RARReadHeaderEx(mRARHandle, uHeaderEx)
        Dim UnpackCount As Integer = 0
        Do Until iStatus <> 0 Or UnpackCount = szFile.Length
            Do
                For i As Integer = 0 To szFile.Length - 1
                    If uHeaderEx.FileName.Trim = szFile(i) Then
                        Dim RARE As RAREntry = FormatFileEntry()
                        fTotalSize = RARE.UnpSize
                        fUnpackedSize = 0
                        Ret = RARProcessFile(mRARHandle, RARMode, szDest, "")
                        If Ret <> 0 Then ProcessError(Ret)
                        UnpackCount += 1
                        RaiseEvent OnUnpack(RARE)   'Signal file has been unpacked
                        Exit Do
                    End If
                Next
                Ret = RARProcessFile(mRARHandle, RAR_SKIP, "", "")
                If Ret <> 0 Then ProcessError(Ret)
                Exit Do
            Loop
            iStatus = RARReadHeaderEx(mRARHandle, uHeaderEx)
        Loop
        RARCloseArchive(mRARHandle)
        GC.KeepAlive(hr)
        Return True

    End Function

    Private Function ExtractAll(ByVal szDest As String) As Boolean
        Return ProcessAll(szDest, RAR_EXTRACT)
    End Function

    Private Function TestAllFiles() As Boolean
        Return ProcessAll("", RAR_TEST)
    End Function

    Private Function ProcessAll(ByVal szDest As String, ByVal RARMMode As Integer) As Boolean
        Dim iStatus As Integer
        Dim Ret As Integer

        uRAREx.OpenMode = RAR_OM_EXTRACT
        mRARHandle = RAROpenArchiveEx(uRAREx)
        If uRAREx.OpenResult <> 0 Then OpenError(uRAREx.OpenResult, mRARFile)
        If mPassword <> "" Then RARSetPassword(mRARHandle, mPassword)
        Dim hr As New RARCallBack(AddressOf RARCB)
        Call RARSetCallback(mRARHandle, hr, 0)
        iStatus = RARReadHeaderEx(mRARHandle, uHeaderEx)
        Do Until iStatus <> 0
            Dim RARE As RAREntry = FormatFileEntry()
            fTotalSize = RARE.UnpSize
            fUnpackedSize = 0
            Ret = RARProcessFile(mRARHandle, RAR_EXTRACT, szDest, "")
            If Ret <> 0 Then ProcessError(Ret)
            RaiseEvent OnUnpack(RARE)   'Signal file has been unpacked
            iStatus = RARReadHeaderEx(mRARHandle, uHeaderEx)
        Loop
        RARCloseArchive(mRARHandle)
        GC.KeepAlive(hr)
        Return True

    End Function

    Private Sub OpenArc(ByRef mH As IntPtr, ByRef h As RAROpenArchiveDataEx)
        '
        ' Open the archive and establish the callback
        mH = RAROpenArchiveEx(h)
        If uRAREx.OpenResult <> 0 Then OpenError(h.OpenResult, mRARFile)
        If mPassword <> "" Then RARSetPassword(mH, mPassword)
        Dim hr As New RARCallBack(AddressOf RARCB)
        GC.KeepAlive(hr)
        Call RARSetCallback(mH, hr, 0)
    End Sub

    Shared fUnpackedSize As Long    'Total Bytes Unpacked so Far for current File
    Shared fTotalSize As Long       'Total File Size

    Public Shared Function RARCB(ByVal msg As Integer, ByVal userdata As Integer, ByVal P1 As IntPtr, ByVal P2 As Integer) As Integer

        Select Case msg
            Case UCM_CHANGEVOLUME
                Select Case P2
                    Case RAR_VOL_ASK
                        'need a new volume - for production use, add a dialog to allow new drive
                        'name and pass it back as a null-terminated string in address pointed at 
                        'by P1 (See Password example below)
                        Dim szVol As String = Marshal.PtrToStringAnsi(P1)
                        If MsgBox("Load Volume: " & szVol, MsgBoxStyle.OkCancel, "UnRAR") <> MsgBoxResult.Ok Then Return -1
                        Return +1 ' 
                    Case RAR_VOL_NOTIFY
                        Return +1
                End Select

            Case UCM_PROCESSDATA
                'I can peek at data here or make a progress bar
                Dim Disposition As RarDisposition = RarDisposition.OP_CONTINUE
                fUnpackedSize += P2
                RaiseEvent Unpacking(fTotalSize, fUnpackedSize, Disposition)
                Select Case Disposition
                    Case RarDisposition.OP_CANCEL
                        Return -1
                    Case Else
                        Return +1
                End Select
            Case UCM_NEEDPASSWORD
                'I need a password
                Dim f As New pwDlg
                f.ShowDialog()
                Dim pwB As Byte() = Encoding.ASCII.GetBytes(f.pw & Chr(0))
                P2 = f.pw.Length
                f.Close()
                Marshal.Copy(pwB, 0, P1, pwB.Length)
                Return +1 '??
            Case Else
                MsgBox("Oops!!, I don't know what I doing here")
        End Select

    End Function

    Private Sub OpenError(ByVal ErroNum As RarErrors, ByVal ArcName As String)
        Dim szMsg As String = ""
        Select Case ErroNum
            Case RarErrors.ERAR_NO_MEMORY
                szMsg = "Not enough memory"
            Case RarErrors.ERAR_EOPEN
                szMsg = "Cannot open " & ArcName
            Case RarErrors.ERAR_BAD_ARCHIVE
                szMsg = ArcName & " is not RAR archive"
            Case RarErrors.ERAR_BAD_DATA
                szMsg = ArcName & ": archive header broken"
        End Select
        RaiseError(ErroNum, szMsg)
    End Sub

    Private Sub ProcessError(ByVal ErroNum As RarErrors)
        Dim szMsg As String
        Select Case ErroNum
            Case RarErrors.ERAR_UNKNOWN_FORMAT
                szMsg = "Unknown archive format"
            Case RarErrors.ERAR_BAD_ARCHIVE
                szMsg = "Bad volume"
            Case RarErrors.ERAR_ECREATE
                szMsg = "File create error"
            Case RarErrors.ERAR_EOPEN
                szMsg = "Volume open error"
            Case RarErrors.ERAR_ECLOSE
                szMsg = "File close error"
            Case RarErrors.ERAR_EREAD
                szMsg = "Read error"
            Case RarErrors.ERAR_EWRITE
                szMsg = "Write error"
            Case RarErrors.ERAR_BAD_DATA
                szMsg = "CRC error"     'looks like bad password comes here as well
            Case RarErrors.ERAR_CANCELLED
                szMsg = "Extract Cancelled"
            Case Else
                szMsg = "Error " & ErroNum
        End Select
        RaiseError(ErroNum, szMsg)
    End Sub

    Private Sub RaiseError(ByVal errnum As Integer, ByVal msg As String)
        Try
            'RARCloseArchive(mRARHandle)
        Catch ex As Exception
            'just in case
        End Try
        Err.Raise(errnum + vbObjectError, , msg)
    End Sub

    'mdm 06/01/2010
    ' This routine will dynamically load unrar.dll or unrar64.dll
    ' depending on the OS
    Private Sub LoadLibrary()

        Dim szLibrary As String = "unrar.dll"
        If System.Runtime.InteropServices.Marshal.SizeOf(New IntPtr) = 8 Then szLibrary = "unrar64.dll"

        Dim pUnrar As IntPtr
        pUnrar = LoadLibraryEx(szLibrary, IntPtr.Zero, 0)
        If pUnrar = IntPtr.Zero Then
            Dim errorCode As Integer = Marshal.GetLastWin32Error()
            Throw New ApplicationException(String.Format("DLL Load error: {0}, error - {1}", szLibrary, errorCode))
        End If

        ' Let's load all the vectors at one time.
        ' This routine assumes all routines are available in unrar.  If
        ' you're using a really old version or if names change in a future
        ' version, we'll add a test to insure we have a compatible version
        ' of unrar.
        Dim pAddr As IntPtr

        pAddr = GetProcAddress(pUnrar, "RARReadHeader")
        _delRARReadHeader = DirectCast(Marshal.GetDelegateForFunctionPointer(pAddr, GetType(dRARReadHeader)), dRARReadHeader)
        pAddr = GetProcAddress(pUnrar, "RARReadHeaderEx")
        _delRARReadHeaderEx = DirectCast(Marshal.GetDelegateForFunctionPointer(pAddr, GetType(dRARReadHeaderEx)), dRARReadHeaderEx)
        pAddr = GetProcAddress(pUnrar, "RAROpenArchive")
        _delRAROpenArchive = DirectCast(Marshal.GetDelegateForFunctionPointer(pAddr, GetType(dRAROpenArchive)), dRAROpenArchive)
        pAddr = GetProcAddress(pUnrar, "RAROpenArchiveEx")
        _delRAROpenArchiveEx = DirectCast(Marshal.GetDelegateForFunctionPointer(pAddr, GetType(dRAROpenArchiveEx)), dRAROpenArchiveEx)
        pAddr = GetProcAddress(pUnrar, "RARCloseArchive")
        _delRARCloseArchive = DirectCast(Marshal.GetDelegateForFunctionPointer(pAddr, GetType(dRARCloseArchive)), dRARCloseArchive)
        pAddr = GetProcAddress(pUnrar, "RARProcessFile")
        _delRARProcessFile = DirectCast(Marshal.GetDelegateForFunctionPointer(pAddr, GetType(dRARProcessFile)), dRARProcessFile)
        pAddr = GetProcAddress(pUnrar, "RARSetPassword")
        _delRARSetPassword = DirectCast(Marshal.GetDelegateForFunctionPointer(pAddr, GetType(dRARSetPassword)), dRARSetPassword)
        pAddr = GetProcAddress(pUnrar, "RARGetDllVersion")
        _delRARGetDllVersion = DirectCast(Marshal.GetDelegateForFunctionPointer(pAddr, GetType(dRARGetDllVersion)), dRARGetDllVersion)
        pAddr = GetProcAddress(pUnrar, "RARSetCallback")
        _delRARSetCallBack = DirectCast(Marshal.GetDelegateForFunctionPointer(pAddr, GetType(dRARSetCallBack)), dRARSetCallBack)

    End Sub

End Class
