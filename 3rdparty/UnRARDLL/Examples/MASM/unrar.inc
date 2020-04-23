; =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
;
;  include file for unrar.dll (version 30.Aug.02)
;  recreated from unrar.h by F.Muecke
;
; =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=

; =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
;  constants
;  ~~~~~~~~~
ERAR_END_ARCHIVE     equ    10
ERAR_NO_MEMORY       equ    11
ERAR_BAD_DATA        equ    12
ERAR_BAD_ARCHIVE     equ    13
ERAR_UNKNOWN_FORMAT  equ    14
ERAR_EOPEN           equ    15
ERAR_ECREATE         equ    16
ERAR_ECLOSE          equ    17
ERAR_EREAD           equ    18
ERAR_EWRITE          equ    19
ERAR_SMALL_BUF       equ    20
ERAR_UNKNOWN         equ    21

RAR_OM_LIST          equ    0
RAR_OM_EXTRACT       equ    1

RAR_SKIP             equ    0
RAR_TEST             equ    1
RAR_EXTRACT          equ    2

RAR_VOL_ASK          equ    0
RAR_VOL_NOTIFY       equ    1

RAR_DLL_VERSION      equ    3

; =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
;  structures
;  ~~~~~~~~~~
RARHeaderData STRUC
  ArcName       db 260 dup (0)
  FileName      db 260 dup (0)
  Flags         dd 0
  PackSize      dd 0
  UnpSize       dd 0
  HostOS        dd 0
  FileCRC       dd 0
  FileTime      dd 0
  UnpVer        dd 0
  Method        dd 0
  FileAttr      dd 0
  lpCmtBuf      dd 0
  CmtBufSize    dd 0
  CmtSize       dd 0
  CmtState      dd 0
RARHeaderData ENDS

RARHeaderDataEx STRUC
  ArcName       db 1024 dup (0)
  ArcNameW      db 1024 dup (0)
  FileName      db 1024 dup (0)
  FileNameW     db 1024 dup (0)
  Flags         dd 0
  PackSize      dd 0
  PackSizeHigh  dd 0
  UnpSize       dd 0
  UnpSizeHigh   dd 0
  HostOS        dd 0
  FileCRC       dd 0
  FileTime      dd 0
  UnpVer        dd 0
  Method        dd 0
  FileAttr      dd 0
  lpCmtBuf      dd 0
  CmtBufSize    dd 0
  CmtSize       dd 0
  CmtState      dd 0
  Reserved      dd 1024 dup (0)
RARHeaderDataEx ENDS

RAROpenArchiveData STRUC
  lpArcName     dd 0
  OpenMode      dd 0
  OpenResult    dd 0
  lpCmtBuf      dd 0
  CmtBufSize    dd 0
  CmtSize       dd 0
  CmtState      dd 0
RAROpenArchiveData ENDS

RAROpenArchiveDataEx STRUC
  lpArcName     dd 0
  lpArcNameW    dd 0
  OpenMode      dd 0
  OpenResult    dd 0
  lpCmtBuf      dd 0
  CmtBufSize    dd 0
  CmtSize       dd 0
  CmtState      dd 0
  Flags         dd 0
  Reserved      dd 32 dup (0)
RAROpenArchiveDataEx ENDS

; =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
;  UNRARCALLBACK_MESSAGES enumeration
UCM_CHANGEVOLUME    equ 0
UCM_PROCESSDATA     equ 1
UCM_NEEDPASSWORD    equ 2

; =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
;  Processes
;  ~~~~~~~~~
UnRARCallback           PROTO :DWORD,:DWORD,:DWORD,:DWORD
ChangeVolProc           PROTO :DWORD,:DWORD ;C calling convention ??
ProcessDataProc         PROTO :DWORD,:DWORD ;C calling convention ??
RAROpenArchive          PROTO :DWORD
RAROpenArchiveEx        PROTO :DWORD
RARCloseArchive         PROTO :DWORD
RARReadHeader           PROTO :DWORD,:DWORD
RARReadHeaderEx         PROTO :DWORD,:DWORD
RARProcessFile          PROTO :DWORD,:DWORD,:DWORD,:DWORD
RARSetCallback          PROTO :DWORD,:DWORD,:DWORD
RARSetChangeVolProc     PROTO :DWORD,:DWORD
RARSetProcessDataProc   PROTO :DWORD,:DWORD
RARSetPassword          PROTO :DWORD,:DWORD
RARGetDllVersion        PROTO

; =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
;  that's it