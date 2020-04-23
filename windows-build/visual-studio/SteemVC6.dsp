# Microsoft Developer Studio Project File - Name="Steem" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Steem - Win32 Boiler Debug_modules
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SteemVC6.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SteemVC6.mak" CFG="Steem - Win32 Boiler Debug_modules"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Steem - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Steem - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Steem - Win32 Boiler" (based on "Win32 (x86) Application")
!MESSAGE "Steem - Win32 Boiler Debug" (based on "Win32 (x86) Application")
!MESSAGE "Steem - Win32 Debug_modules" (based on "Win32 (x86) Application")
!MESSAGE "Steem - Win32 Release_modules" (based on "Win32 (x86) Application")
!MESSAGE "Steem - Win32 Boiler_modules" (based on "Win32 (x86) Application")
!MESSAGE "Steem - Win32 Boiler Debug_modules" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /Gm /GX /ZI /Od /D "_VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /I /I /I /I /GZ /c
# ADD CPP /nologo /G6 /ML /GX /Zi /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "_DEBUG" /D "NO_DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /D "SSE_DEBUG" /FR /EHa /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dinput.lib dxguid.lib winmm.lib ComCtl32.Lib /nologo /subsystem:windows /debug /machine:IX86 /out:"Debug\Steem.exe" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:IX86 /nodefaultlib:"libcpd.lib" /out:"Debug\Steem.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /w /W0 /GX /Zi /Ox /Ot /Og /Oi /Ob2 /Gy /D "_VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /I /I /I /I /GA /GF
# ADD CPP /nologo /G6 /MT /GX /Zi /Os /Oy /Ob2 /Gy /I "..\..\3rdparty\sim6xxx" /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "NDEBUG" /D "NO_DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /FR /GA /GF /c
# SUBTRACT CPP /Og
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dinput.lib dxguid.lib winmm.lib ComCtl32.Lib /nologo /subsystem:windows /machine:IX86 /out:"Release\Steem.exe" /pdbtype:sept /opt:ref /opt:icf
# ADD LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:IX86 /pdbtype:sept /libpath:"d3d/" /opt:ref /opt:icf
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy "Release\SteemVC6.exe" "d:\emu\ST\bin\steem"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Steem___Win32_Boiler0"
# PROP BASE Intermediate_Dir "Steem___Win32_Boiler0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Steem___Win32_Boiler0"
# PROP Intermediate_Dir "Steem___Win32_Boiler0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /Zp16 /MD /W3 /GX /Zi /Ob2 /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /I "..\..\3rdparty\hatari" /D "NDEBUG" /D "NO_DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /FR /GA /GF /c
# ADD CPP /nologo /G6 /Zp16 /MD /GX /Zi /Ob2 /Gy /I "..\..\3rdparty\sim6xxx" /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "NDEBUG" /D "DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /D "SSE_DEBUG" /FR /GA /GF /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:IX86 /out:"Release\SteemBeta.exe" /pdbtype:sept /opt:ref /opt:icf
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:IX86 /out:"Release\BoilerVC6.exe" /pdbtype:sept /libpath:"d3d/" /opt:ref /opt:icf
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy "Release\BoilerVC6.exe" "d:\emu\ST\bin\steem"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Steem___Win32_Boiler_Debug"
# PROP BASE Intermediate_Dir "Steem___Win32_Boiler_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Steem___Win32_Boiler_Debug"
# PROP Intermediate_Dir "Steem___Win32_Boiler_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /ML /GX /Zi /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "_DEBUG" /D "NO_DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /FR /EHa /c
# ADD CPP /nologo /G6 /ML /GX /Zi /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "_DEBUG" /D "DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /D "SSE_DEBUG" /FR /EHa /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:IX86 /out:"Debug\Steem.exe" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /map /debug /machine:IX86 /out:"Debug\Steem.exe" /pdbtype:sept /libpath:"d3d/"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Steem___Win32_Debug_modules"
# PROP BASE Intermediate_Dir "Steem___Win32_Debug_modules"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Steem___Win32_Debug_modules"
# PROP Intermediate_Dir "Steem___Win32_Debug_modules"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /ML /w /W0 /GX /Zi /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "_DEBUG" /D "NO_DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /D "SS_DEBUG" /FR /EHa /c
# ADD CPP /nologo /G6 /ML /GX /Zi /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "_DEBUG" /D "NO_DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /D "SSE_DEBUG" /D "LEGACY_BUILD" /FR /EHa /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:IX86 /out:"Debug\Steem.exe" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:IX86 /out:"Debug\Steem.exe" /pdbtype:sept /libpath:"d3d/"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Steem___Win32_Release_modules"
# PROP BASE Intermediate_Dir "Steem___Win32_Release_modules"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Steem___Win32_Release_modules"
# PROP Intermediate_Dir "Steem___Win32_Release_modules"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MT /w /W0 /GX /Zi /Os /Oy /Ob2 /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /I "..\..\3rdparty\sim6xxx" /D "NDEBUG" /D "NO_DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /FR /GA /GF /c
# SUBTRACT BASE CPP /Og
# ADD CPP /nologo /G6 /MT /w /W0 /GX /Zi /Os /Oy /Ob2 /Gy /I "..\..\3rdparty\sim6xxx" /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "NDEBUG" /D "NO_DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /D "LEGACY_BUILD" /FR /GA /GF /c
# SUBTRACT CPP /Og
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:IX86 /out:"Release\SteemBeta.exe" /pdbtype:sept /opt:ref /opt:icf
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:IX86 /out:"Release\SteemBeta.exe" /pdbtype:sept /libpath:"d3d/" /opt:ref /opt:icf
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy "Release\SteemBeta.exe" "G:\emu\ST\bin\steem"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Steem___Win32_Boiler_modules"
# PROP BASE Intermediate_Dir "Steem___Win32_Boiler_modules"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Steem___Win32_Boiler_modules"
# PROP Intermediate_Dir "Steem___Win32_Boiler_modules"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /Zp16 /MD /w /W0 /GX /Zi /Ob2 /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /I "..\..\3rdparty\sim6xxx" /D "NDEBUG" /D "DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /D "SS_DEBUG" /FR /GA /GF /c
# ADD CPP /nologo /G6 /Zp16 /MD /w /W0 /GX /Zi /Ob2 /Gy /I "..\..\3rdparty\sim6xxx" /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "NDEBUG" /D "DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /D "SSE_DEBUG" /D "LEGACY_BUILD" /FR /GA /GF /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:IX86 /out:"Release\BoilerBeta.exe" /pdbtype:sept /opt:ref /opt:icf
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:IX86 /out:"Release\BoilerBeta.exe" /pdbtype:sept /libpath:"d3d/" /opt:ref /opt:icf
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy "Release\BoilerBeta.exe" "G:\emu\ST\bin\steem"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Steem___Win32_Boiler_Debug_modules"
# PROP BASE Intermediate_Dir "Steem___Win32_Boiler_Debug_modules"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Steem___Win32_Boiler_Debug_modules"
# PROP Intermediate_Dir "Steem___Win32_Boiler_Debug_modules"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /ML /GX /Zi /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "_DEBUG" /D "DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /D "SS_DEBUG" /FR /EHa /c
# ADD CPP /nologo /G6 /ML /GX /Zi /Gy /I "..\..\include\\" /I "..\..\steem\code\\" /I "..\..\3rdparty\\" /D "_DEBUG" /D "DEBUG_BUILD" /D "VC_BUILD" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "STEVEN_SEAGAL" /D "SSE_DEBUG" /D "LEGACY_BUILD" /FR /EHa /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:IX86 /out:"Debug\Steem.exe" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 dinput.lib dxguid.lib winmm.lib ComCtl32.Lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:IX86 /out:"Debug\Steem.exe" /pdbtype:sept /libpath:"d3d/"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Steem - Win32 Debug"
# Name "Steem - Win32 Release"
# Name "Steem - Win32 Boiler"
# Name "Steem - Win32 Boiler Debug"
# Name "Steem - Win32 Debug_modules"
# Name "Steem - Win32 Release_modules"
# Name "Steem - Win32 Boiler_modules"
# Name "Steem - Win32 Boiler Debug_modules"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm"
# Begin Source File

SOURCE=..\..\steem\emu.cpp
DEP_CPP_EMU_C=\
	"..\..\3rdparty\6301\6301.h"\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\d3d9.h"\
	"..\..\3rdparty\d3d\d3d9caps.h"\
	"..\..\3rdparty\d3d\D3d9types.h"\
	"..\..\3rdparty\d3d\d3dx9.h"\
	"..\..\3rdparty\d3d\d3dx9anim.h"\
	"..\..\3rdparty\d3d\d3dx9core.h"\
	"..\..\3rdparty\d3d\d3dx9effect.h"\
	"..\..\3rdparty\d3d\D3dx9math.h"\
	"..\..\3rdparty\d3d\d3dx9math.inl"\
	"..\..\3rdparty\d3d\d3dx9mesh.h"\
	"..\..\3rdparty\d3d\d3dx9shader.h"\
	"..\..\3rdparty\d3d\d3dx9shape.h"\
	"..\..\3rdparty\d3d\d3dx9tex.h"\
	"..\..\3rdparty\d3d\d3dx9xof.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\dsp\dsp.h"\
	"..\..\3rdparty\FreeImage\FreeImage.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\3rdparty\SDL-WIN\include\begin_code.h"\
	"..\..\3rdparty\SDL-WIN\include\close_code.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_active.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_audio.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_cdrom.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_dreamcast.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_macos.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_macosx.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_minimal.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_os2.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_symbian.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_win32.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_cpuinfo.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_endian.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_error.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_events.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_joystick.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_keyboard.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_keysym.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_loadso.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_main.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_mouse.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_mutex.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_platform.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_quit.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_rwops.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_stdinc.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_thread.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_timer.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_version.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_video.h"\
	"..\..\3rdparty\various\neochrome.h"\
	"..\..\3rdparty\various\ym2149_fixed_vol.h"\
	"..\..\include\binary.h"\
	"..\..\include\circularbuffer.h"\
	"..\..\include\clarity.h"\
	"..\..\include\configstorefile.h"\
	"..\..\include\directory_tree.h"\
	"..\..\include\dirsearch.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easycompress.h"\
	"..\..\include\easystr.h"\
	"..\..\include\easystringlist.h"\
	"..\..\include\internal_speaker.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\portio.h"\
	"..\..\include\x\hxc.h"\
	"..\..\include\x\hxc_alert.h"\
	"..\..\include\x\hxc_dir_lv.h"\
	"..\..\include\x\hxc_fileselect.h"\
	"..\..\include\x\hxc_popup.h"\
	"..\..\include\x\hxc_popuphints.h"\
	"..\..\include\x\icongroup.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\acc.h"\
	"..\..\steem\code\acia.decla.h"\
	"..\..\steem\code\acia.h"\
	"..\..\steem\code\blitter.cpp"\
	"..\..\steem\code\blitter.decla.h"\
	"..\..\steem\code\blitter.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.cpp"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\cpu.h"\
	"..\..\steem\code\cpuinit.cpp"\
	"..\..\steem\code\d2.decla.h"\
	"..\..\steem\code\debug_emu.cpp"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\debug_emu.h"\
	"..\..\steem\code\diskman.decla.h"\
	"..\..\steem\code\display.decla.h"\
	"..\..\steem\code\display.h"\
	"..\..\steem\code\draw.cpp"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\draw.h"\
	"..\..\steem\code\emulator.cpp"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\emulator.h"\
	"..\..\steem\code\fdc.cpp"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\fdc.h"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\floppy_drive.h"\
	"..\..\steem\code\gui.decla.h"\
	"..\..\steem\code\gui.h"\
	"..\..\steem\code\harddiskman.decla.h"\
	"..\..\steem\code\hdimg.decla.h"\
	"..\..\steem\code\hdimg.h"\
	"..\..\steem\code\ikbd.cpp"\
	"..\..\steem\code\ikbd.decla.h"\
	"..\..\steem\code\ikbd.h"\
	"..\..\steem\code\init_sound.decla.h"\
	"..\..\steem\code\init_sound.h"\
	"..\..\steem\code\iolist.decla.h"\
	"..\..\steem\code\iolist.h"\
	"..\..\steem\code\ior.cpp"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\iorw.h"\
	"..\..\steem\code\iow.cpp"\
	"..\..\steem\code\loadsave.decla.h"\
	"..\..\steem\code\loadsave.h"\
	"..\..\steem\code\loadsave_emu.cpp"\
	"..\..\steem\code\macros.decla.h"\
	"..\..\steem\code\macros.h"\
	"..\..\steem\code\mfp.cpp"\
	"..\..\steem\code\mfp.decla.h"\
	"..\..\steem\code\mfp.h"\
	"..\..\steem\code\midi.cpp"\
	"..\..\steem\code\midi.decla.h"\
	"..\..\steem\code\midi.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\onegame.decla.h"\
	"..\..\steem\code\onegame.h"\
	"..\..\steem\code\osd.decla.h"\
	"..\..\steem\code\osd.h"\
	"..\..\steem\code\palette.decla.h"\
	"..\..\steem\code\palette.h"\
	"..\..\steem\code\psg.cpp"\
	"..\..\steem\code\psg.decla.h"\
	"..\..\steem\code\psg.h"\
	"..\..\steem\code\reset.cpp"\
	"..\..\steem\code\reset.h"\
	"..\..\steem\code\resnum.decla.h"\
	"..\..\steem\code\rs232.cpp"\
	"..\..\steem\code\rs232.decla.h"\
	"..\..\steem\code\rs232.h"\
	"..\..\steem\code\run.cpp"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\run.h"\
	"..\..\steem\code\shortcutbox.decla.h"\
	"..\..\steem\code\shortcutbox.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSE6301.cpp"\
	"..\..\steem\code\SSE\SSE6301.h"\
	"..\..\Steem\code\SSE\SSEAcsi.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\steem\code\SSE\SSECpu.cpp"\
	"..\..\steem\code\SSE\SSECpu.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.cpp"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEFrameReport.cpp"\
	"..\..\steem\code\SSE\SSEFrameReport.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEGlue.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEInline.h"\
	"..\..\steem\code\SSE\SSEInterrupt.cpp"\
	"..\..\steem\code\SSE\SSEInterrupt.h"\
	"..\..\steem\code\SSE\SSEMMU.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSESDL.cpp"\
	"..\..\steem\code\SSE\SSESDL.h"\
	"..\..\steem\code\SSE\SSEShifter.cpp"\
	"..\..\steem\code\SSE\SSEShifter.h"\
	"..\..\steem\code\SSE\SSESTF.cpp"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEVideo.cpp"\
	"..\..\steem\code\SSE\SSEVideo.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\code\SteemFreeImage.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\code\steemh.h"\
	"..\..\steem\code\stemdialogs.decla.h"\
	"..\..\steem\code\stemdialogs.h"\
	"..\..\steem\code\stemdos.cpp"\
	"..\..\steem\code\stemdos.decla.h"\
	"..\..\steem\code\stemdos.h"\
	"..\..\steem\code\stjoy.decla.h"\
	"..\..\steem\code\stjoy.h"\
	"..\..\steem\code\stports.cpp"\
	"..\..\steem\code\stports.decla.h"\
	"..\..\steem\code\stports.h"\
	"..\..\steem\code\translate.decla.h"\
	"..\..\steem\code\translate.h"\
	"..\..\steem\code\x\x_midi.cpp"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_EMU_C=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\include\beos\be_mymisc.h"\
	"..\..\steem\code\d3d9.h"\
	"..\..\steem\code\D3d9types.h"\
	"..\..\steem\code\d3dx9core.h"\
	"..\..\steem\code\D3dx9math.h"\
	"..\..\steem\code\io68\ym_fixed_vol.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\helper.cpp
DEP_CPP_HELPE=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\include\binary.h"\
	"..\..\include\choosefolder.cpp"\
	"..\..\include\circularbuffer.cpp"\
	"..\..\include\circularbuffer.h"\
	"..\..\include\clarity.h"\
	"..\..\include\configstorefile.cpp"\
	"..\..\include\configstorefile.h"\
	"..\..\include\configstorefile_bad.cpp"\
	"..\..\include\directory_tree.cpp"\
	"..\..\include\directory_tree.h"\
	"..\..\include\dirsearch.cpp"\
	"..\..\include\dirsearch.h"\
	"..\..\include\dynamicarray.cpp"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easycompress.cpp"\
	"..\..\include\easystr.cpp"\
	"..\..\include\easystr.h"\
	"..\..\include\easystringlist.cpp"\
	"..\..\include\easystringlist.h"\
	"..\..\include\input_prompt.cpp"\
	"..\..\include\input_prompt.h"\
	"..\..\include\mymisc.cpp"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.cpp"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\portio.cpp"\
	"..\..\include\portio.h"\
	"..\..\include\scrollingcontrolswin.cpp"\
	"..\..\include\scrollingcontrolswin.h"\
	"..\..\include\x\hxc.cpp"\
	"..\..\include\x\hxc.h"\
	"..\..\include\x\hxc_alert.cpp"\
	"..\..\include\x\hxc_alert.h"\
	"..\..\include\x\hxc_button.cpp"\
	"..\..\include\x\hxc_dir_lv.cpp"\
	"..\..\include\x\hxc_dir_lv.h"\
	"..\..\include\x\hxc_dropdown.cpp"\
	"..\..\include\x\hxc_edit.cpp"\
	"..\..\include\x\hxc_fileselect.cpp"\
	"..\..\include\x\hxc_fileselect.h"\
	"..\..\include\x\hxc_listview.cpp"\
	"..\..\include\x\hxc_popup.cpp"\
	"..\..\include\x\hxc_popup.h"\
	"..\..\include\x\hxc_popuphints.cpp"\
	"..\..\include\x\hxc_popuphints.h"\
	"..\..\include\x\hxc_prompt.cpp"\
	"..\..\include\x\hxc_prompt.h"\
	"..\..\include\x\hxc_scrollarea.cpp"\
	"..\..\include\x\hxc_scrollbar.cpp"\
	"..\..\include\x\hxc_textdisplay.cpp"\
	"..\..\include\x\icongroup.cpp"\
	"..\..\include\x\icongroup.h"\
	"..\..\include\x\x_mymisc.cpp"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\include\x\x_portio.cpp"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\translate.decla.h"\
	"..\..\steem\code\translate.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_HELPE=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\include\beos\be_mymisc.cpp"\
	"..\..\include\beos\be_mymisc.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\main.cpp
DEP_CPP_MAIN_=\
	"..\..\3rdparty\6301\6301.h"\
	"..\..\3rdparty\ArchiveAccess\ArchiveAccess\ArchiveAccessBase.h"\
	"..\..\3rdparty\ArchiveAccess\ArchiveAccess\ArchiveAccessSSE.h"\
	"..\..\3rdparty\ArchiveAccess\ArchiveAccess\bgstr.h"\
	"..\..\3rdparty\ArchiveAccess\CPP\Common\myCom.h"\
	"..\..\3rdparty\ArchiveAccess\CPP\Common\MyGuidDef.h"\
	"..\..\3rdparty\ArchiveAccess\CPP\Common\MyWindows.h"\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\d3d9.h"\
	"..\..\3rdparty\d3d\d3d9caps.h"\
	"..\..\3rdparty\d3d\D3d9types.h"\
	"..\..\3rdparty\d3d\d3dx9.h"\
	"..\..\3rdparty\d3d\d3dx9anim.h"\
	"..\..\3rdparty\d3d\d3dx9core.h"\
	"..\..\3rdparty\d3d\d3dx9effect.h"\
	"..\..\3rdparty\d3d\D3dx9math.h"\
	"..\..\3rdparty\d3d\d3dx9math.inl"\
	"..\..\3rdparty\d3d\d3dx9mesh.h"\
	"..\..\3rdparty\d3d\d3dx9shader.h"\
	"..\..\3rdparty\d3d\d3dx9shape.h"\
	"..\..\3rdparty\d3d\d3dx9tex.h"\
	"..\..\3rdparty\d3d\d3dx9xof.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\dsp\dsp.h"\
	"..\..\3rdparty\FreeImage\FreeImage.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\3rdparty\SDL-WIN\include\begin_code.h"\
	"..\..\3rdparty\SDL-WIN\include\close_code.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_active.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_audio.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_cdrom.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_dreamcast.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_macos.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_macosx.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_minimal.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_os2.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_symbian.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_win32.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_cpuinfo.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_endian.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_error.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_events.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_joystick.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_keyboard.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_keysym.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_loadso.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_main.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_mouse.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_mutex.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_platform.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_quit.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_rwops.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_stdinc.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_thread.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_timer.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_version.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_video.h"\
	"..\..\3rdparty\UnRARDLL\unrar.h"\
	"..\..\3rdparty\unrarlib\unrarlib\unrarlib.h"\
	"..\..\3rdparty\various\neochrome.h"\
	"..\..\include\binary.h"\
	"..\..\include\choosefolder.h"\
	"..\..\include\circularbuffer.h"\
	"..\..\include\clarity.h"\
	"..\..\include\configstorefile.h"\
	"..\..\include\dderr_meaning.h"\
	"..\..\include\di_get_contents.cpp"\
	"..\..\include\di_get_contents.h"\
	"..\..\include\directory_tree.h"\
	"..\..\include\dirsearch.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easycompress.h"\
	"..\..\include\easystr.h"\
	"..\..\include\easystringlist.h"\
	"..\..\include\input_prompt.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\portio.h"\
	"..\..\include\scrollingcontrolswin.h"\
	"..\..\include\wordwrapper.cpp"\
	"..\..\include\wordwrapper.h"\
	"..\..\include\x\hxc.h"\
	"..\..\include\x\hxc_alert.h"\
	"..\..\include\x\hxc_dir_lv.h"\
	"..\..\include\x\hxc_fileselect.h"\
	"..\..\include\x\hxc_popup.h"\
	"..\..\include\x\hxc_popuphints.h"\
	"..\..\include\x\hxc_prompt.h"\
	"..\..\include\x\icongroup.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\acc.cpp"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\acc.h"\
	"..\..\steem\code\acia.decla.h"\
	"..\..\steem\code\acia.h"\
	"..\..\steem\code\archive.cpp"\
	"..\..\steem\code\archive.decla.h"\
	"..\..\steem\code\archive.h"\
	"..\..\steem\code\associate.cpp"\
	"..\..\steem\code\blitter.decla.h"\
	"..\..\steem\code\blitter.h"\
	"..\..\steem\code\boiler.cpp"\
	"..\..\steem\code\boiler.decla.h"\
	"..\..\steem\code\boiler.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\controls.cpp"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\cpu.h"\
	"..\..\steem\code\d2.cpp"\
	"..\..\steem\code\d2.decla.h"\
	"..\..\steem\code\d2.h"\
	"..\..\steem\code\dataloadsave.cpp"\
	"..\..\steem\code\dataloadsave.decla.h"\
	"..\..\steem\code\dataloadsave.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\debug_emu.h"\
	"..\..\steem\code\dir_id.cpp"\
	"..\..\steem\code\dir_id.decla.h"\
	"..\..\steem\code\dir_id.h"\
	"..\..\steem\code\diskman.cpp"\
	"..\..\steem\code\diskman.decla.h"\
	"..\..\steem\code\diskman.h"\
	"..\..\steem\code\diskman_diags.cpp"\
	"..\..\steem\code\diskman_drag.cpp"\
	"..\..\steem\code\display.cpp"\
	"..\..\steem\code\display.decla.h"\
	"..\..\steem\code\display.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\draw.h"\
	"..\..\steem\code\dwin_edit.cpp"\
	"..\..\steem\code\dwin_edit.decla.h"\
	"..\..\steem\code\dwin_edit.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\emulator.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\fdc.h"\
	"..\..\steem\code\floppy_drive.cpp"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\floppy_drive.h"\
	"..\..\steem\code\gui.cpp"\
	"..\..\steem\code\gui.decla.h"\
	"..\..\steem\code\gui.h"\
	"..\..\steem\code\harddiskman.cpp"\
	"..\..\steem\code\harddiskman.decla.h"\
	"..\..\steem\code\harddiskman.h"\
	"..\..\steem\code\hdimg.cpp"\
	"..\..\steem\code\hdimg.decla.h"\
	"..\..\steem\code\hdimg.h"\
	"..\..\steem\code\historylist.cpp"\
	"..\..\steem\code\historylist.h"\
	"..\..\steem\code\ikbd.decla.h"\
	"..\..\steem\code\ikbd.h"\
	"..\..\steem\code\include.h"\
	"..\..\steem\code\infobox.cpp"\
	"..\..\steem\code\infobox.decla.h"\
	"..\..\steem\code\infobox.h"\
	"..\..\steem\code\init_sound.cpp"\
	"..\..\steem\code\init_sound.decla.h"\
	"..\..\steem\code\init_sound.h"\
	"..\..\steem\code\iolist.cpp"\
	"..\..\steem\code\iolist.decla.h"\
	"..\..\steem\code\iolist.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\iorw.h"\
	"..\..\steem\code\key_table.cpp"\
	"..\..\steem\code\key_table.decla.h"\
	"..\..\steem\code\key_table.h"\
	"..\..\steem\code\loadsave.cpp"\
	"..\..\steem\code\loadsave.decla.h"\
	"..\..\steem\code\loadsave.h"\
	"..\..\steem\code\macros.cpp"\
	"..\..\steem\code\macros.decla.h"\
	"..\..\steem\code\macros.h"\
	"..\..\steem\code\mem_browser.cpp"\
	"..\..\steem\code\mem_browser.decla.h"\
	"..\..\steem\code\mem_browser.h"\
	"..\..\steem\code\mfp.decla.h"\
	"..\..\steem\code\mfp.h"\
	"..\..\steem\code\midi.decla.h"\
	"..\..\steem\code\midi.h"\
	"..\..\steem\code\mr_static.cpp"\
	"..\..\steem\code\mr_static.decla.h"\
	"..\..\steem\code\mr_static.h"\
	"..\..\steem\code\notifyinit.cpp"\
	"..\..\steem\code\notifyinit.decla.h"\
	"..\..\steem\code\notifyinit.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\onegame.cpp"\
	"..\..\steem\code\onegame.decla.h"\
	"..\..\steem\code\onegame.h"\
	"..\..\steem\code\options.cpp"\
	"..\..\steem\code\options.decla.h"\
	"..\..\steem\code\options.h"\
	"..\..\steem\code\options_create.cpp"\
	"..\..\steem\code\osd.cpp"\
	"..\..\steem\code\osd.decla.h"\
	"..\..\steem\code\osd.h"\
	"..\..\steem\code\palette.cpp"\
	"..\..\steem\code\palette.decla.h"\
	"..\..\steem\code\palette.h"\
	"..\..\steem\code\patchesbox.cpp"\
	"..\..\steem\code\patchesbox.h"\
	"..\..\steem\code\psg.decla.h"\
	"..\..\steem\code\psg.h"\
	"..\..\steem\code\reset.h"\
	"..\..\steem\code\resnum.decla.h"\
	"..\..\steem\code\resnum.h"\
	"..\..\steem\code\rs232.decla.h"\
	"..\..\steem\code\rs232.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\run.h"\
	"..\..\steem\code\screen_saver.cpp"\
	"..\..\steem\code\screen_saver.h"\
	"..\..\steem\code\shortcutbox.cpp"\
	"..\..\steem\code\shortcutbox.decla.h"\
	"..\..\steem\code\shortcutbox.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSE6301.h"\
	"..\..\Steem\code\SSE\SSEAcsi.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\steem\code\SSE\SSECpu.h"\
	"..\..\Steem\code\SSE\SSEDebug.cpp"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEFrameReport.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEGlue.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEInterrupt.h"\
	"..\..\steem\code\SSE\SSEMMU.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSESDL.h"\
	"..\..\steem\code\SSE\SSEShifter.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\code\SteemFreeImage.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\code\steemh.h"\
	"..\..\steem\code\steemintro.cpp"\
	"..\..\steem\code\stemdialogs.cpp"\
	"..\..\steem\code\stemdialogs.decla.h"\
	"..\..\steem\code\stemdialogs.h"\
	"..\..\steem\code\stemdos.decla.h"\
	"..\..\steem\code\stemdos.h"\
	"..\..\steem\code\stemwin.cpp"\
	"..\..\steem\code\stjoy.cpp"\
	"..\..\steem\code\stjoy.decla.h"\
	"..\..\steem\code\stjoy.h"\
	"..\..\steem\code\stjoy_directinput.cpp"\
	"..\..\steem\code\stports.decla.h"\
	"..\..\steem\code\stports.h"\
	"..\..\steem\code\trace.cpp"\
	"..\..\steem\code\translate.decla.h"\
	"..\..\steem\code\translate.h"\
	"..\..\steem\code\unzip_win32.h"\
	"..\..\steem\code\x\x_controls.cpp"\
	"..\..\steem\code\x\x_controls.h"\
	"..\..\steem\code\x\x_diskman.cpp"\
	"..\..\steem\code\x\x_display.cpp"\
	"..\..\steem\code\x\x_gui.cpp"\
	"..\..\steem\code\x\x_harddiskman.cpp"\
	"..\..\steem\code\x\x_infobox.cpp"\
	"..\..\steem\code\x\x_joy.cpp"\
	"..\..\steem\code\x\x_notifyinit.cpp"\
	"..\..\steem\code\x\x_options.cpp"\
	"..\..\steem\code\x\x_options_create.cpp"\
	"..\..\steem\code\x\x_patchesbox.cpp"\
	"..\..\steem\code\x\x_screen_saver.cpp"\
	"..\..\steem\code\x\x_shortcutbox.cpp"\
	"..\..\steem\code\x\x_sound.cpp"\
	"..\..\steem\code\x\x_sound_portaudio.cpp"\
	"..\..\steem\code\x\x_sound_rtaudio.cpp"\
	"..\..\steem\code\x\x_stemdialogs.cpp"\
	"..\..\steem\code\x\x_stemwin.cpp"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_MAIN_=\
	"..\..\..\..\..\usr\include\linux\kd.h"\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\include\beos\be_mymisc.h"\
	"..\..\steem\code\d3d9.h"\
	"..\..\steem\code\D3d9types.h"\
	"..\..\steem\code\d3dx9core.h"\
	"..\..\steem\code\D3dx9math.h"\
	"..\..\steem\code\io68\ym_fixed_vol.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\Steem.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc"
# Begin Source File

SOURCE=..\..\steem\code\acc.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\acc.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\acia.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\acia.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\archive.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\archive.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\blitter.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\blitter.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\boiler.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\boiler.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\conditions.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\cpu.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\cpu.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\d2.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\d2.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\dataloadsave.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\dataloadsave.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\debug_emu.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\debug_emu.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\dir_id.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\dir_id.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\diskman.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\diskman.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\display.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\display.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\draw.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\draw.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\dwin_edit.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\dwin_edit.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\emulator.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\emulator.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\fdc.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\fdc.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\floppy_drive.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\floppy_drive.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\gui.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\gui.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\harddiskman.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\harddiskman.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\hdimg.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\hdimg.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\historylist.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\ikbd.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\ikbd.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\include.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\infobox.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\infobox.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\init_sound.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\init_sound.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\iolist.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\iolist.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\iorw.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\iorw.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\key_table.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\key_table.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\loadsave.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\loadsave.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\macros.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\macros.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\mem_browser.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\mem_browser.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\mfp.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\mfp.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\midi.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\midi.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\mr_static.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\mr_static.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\notifyinit.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\notifyinit.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\notwindows.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\onegame.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\onegame.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\options.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\options.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\osd.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\osd.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\palette.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\palette.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\patchesbox.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\pch.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\psg.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\psg.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\reset.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\resnum.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\resnum.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\rs232.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\rs232.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\run.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\run.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\screen_saver.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\shortcutbox.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\shortcutbox.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SteemFreeImage.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\steemh.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\steemh.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stemdialogs.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stemdialogs.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stemdos.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stemdos.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stjoy.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stjoy.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stports.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stports.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\trace.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\trace.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\translate.decla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\translate.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\unzip_win32.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\steem\rc\debug_close.ico
# End Source File
# Begin Source File

SOURCE=..\..\Steem\rc\debug_icons.bmp
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\debug_trash.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\disk_manager.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_accurate_fdc.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_back.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk_history_menu.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk_link.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk_link_broken.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk_readonly.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk_zip_readonly.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_disk_zip_readwrite.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_drive_a.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_drive_b.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_drive_b_disabled.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_folder.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_folder_link.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_folder_link_broken.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_folder_parent.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_forward.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_hard_drives.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_hard_drives_acsi.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_hard_drives_francais.ico
# End Source File
# Begin Source File

SOURCE=..\..\Steem\rc\dm_hard_drives_gemdos.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_home.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_menu.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_set_home.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\dm_tools_menu.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\fast_forward.ico
# End Source File
# Begin Source File

SOURCE=..\..\Steem\rc\flags.bmp
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\fullscreen_quit.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\hard_drive.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info_cart_howto.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info_disk_howto.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info_drawspeed.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info_faq.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info_links.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\info_text.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\joy.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\macro_play.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\memory_snapshot.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\memory_snapshot_file_icon.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_associations.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_brightcon.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_C1.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_C2.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_display.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_fullscreen.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_general.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_icons.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_machine.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_macros.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_midi.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_osd.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_ports.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_profiles.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_sound.ico
# End Source File
# Begin Source File

SOURCE=..\..\Steem\rc\options_sse.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_startup.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\options_tos.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\paste.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\patch.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\patch_new.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\pc_folder.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\point.cur
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\program_file_icon.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\record.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\reset.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\reset_need.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\resource.rc
# ADD BASE RSC /l 0x809 /i "\data\Prg\ST\steem\rc" /i "\Downloads\steemsse-code\steemsse\steem\rc"
# ADD RSC /l 0x409 /i "\data\Prg\ST\steem\rc" /i "\Downloads\steemsse-code\steemsse\steem\rc"
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\run.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\shortcut.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\shortcut_off.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\shortcut_on.ico
# End Source File
# Begin Source File

SOURCE=..\..\Steem\rc\st_chars_mono.bmp
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\steem.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\steem_256_file_icon.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\take_screenshot.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\update.ico
# End Source File
# Begin Source File

SOURCE=..\..\steem\rc\windowed_mode.ico
# End Source File
# End Group
# Begin Group "Third Party"

# PROP Default_Filter ""
# Begin Group "ijor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdparty\pasti\div68kCycleAccurate.c
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\pasti\pasti.h
# End Source File
# End Group
# Begin Group "dsp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdparty\dsp\dsp.cpp
DEP_CPP_DSP_C=\
	"..\..\3rdparty\dsp\dsp.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\dsp\dsp.h
# End Source File
# End Group
# Begin Group "6301"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdparty\6301\6301.c
DEP_CPP_6301_=\
	"..\..\3rdparty\6301\6301.h"\
	"..\..\3rdparty\6301\alu.c"\
	"..\..\3rdparty\6301\alu.h"\
	"..\..\3rdparty\6301\callstac.c"\
	"..\..\3rdparty\6301\callstac.h"\
	"..\..\3rdparty\6301\chip.h"\
	"..\..\3rdparty\6301\cpu.c"\
	"..\..\3rdparty\6301\cpu.h"\
	"..\..\3rdparty\6301\defs.h"\
	"..\..\3rdparty\6301\fprinthe.c"\
	"..\..\3rdparty\6301\instr.c"\
	"..\..\3rdparty\6301\instr.h"\
	"..\..\3rdparty\6301\ireg.c"\
	"..\..\3rdparty\6301\ireg.h"\
	"..\..\3rdparty\6301\memory.c"\
	"..\..\3rdparty\6301\memsetl.c"\
	"..\..\3rdparty\6301\opfunc.c"\
	"..\..\3rdparty\6301\opfunc.h"\
	"..\..\3rdparty\6301\optab.c"\
	"..\..\3rdparty\6301\optab.h"\
	"..\..\3rdparty\6301\reg.c"\
	"..\..\3rdparty\6301\reg.h"\
	"..\..\3rdparty\6301\sci.c"\
	"..\..\3rdparty\6301\sci.h"\
	"..\..\3rdparty\6301\symtab.c"\
	"..\..\3rdparty\6301\symtab.h"\
	"..\..\3rdparty\6301\timer.c"\
	"..\..\3rdparty\6301\timer.h"\
	"..\..\3rdparty\6301\tty.c"\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\acia.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSE6301.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_6301_=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\6301.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\alu.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\alu.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\callstac.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\callstac.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\chip.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\cpu.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\cpu.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\defs.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\fprinthe.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\instr.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\instr.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\ireg.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\ireg.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\main.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\memory.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\memory.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\memsetl.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\opfunc.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\opfunc.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\optab.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\optab.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\reg.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\reg.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\sci.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\sci.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\symtab.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\symtab.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\timer.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\timer.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\6301\tty.c

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "caps"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdparty\caps\CapsAPI.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\caps\CapsFDC.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\caps\CommonTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\caps\CAPSImg.lib
# End Source File
# End Group
# Begin Group "AVI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdparty\avi\AviFile.cpp
DEP_CPP_AVIFI=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_AVIFI=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\avi\AviFile.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\avi\Resource.h
# End Source File
# End Group
# Begin Group "SDL"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\begin_code.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\close_code.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_active.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_audio.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_byteorder.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_cdrom.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_config.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_config_dreamcast.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_config_macos.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_config_macosx.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_config_minimal.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_config_nds.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_config_os2.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_config_symbian.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_config_win32.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_copying.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_cpuinfo.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_endian.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_error.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_events.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_getenv.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_joystick.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_keyboard.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_keysym.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_loadso.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_main.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_mouse.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_mutex.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_name.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_opengl.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_platform.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_quit.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_rwops.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_stdinc.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_syswm.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_thread.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_timer.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_types.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_version.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\include\SDL_video.h"
# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\lib\x86\SDL.lib"

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1
# PROP Ignore_Default_Tool 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP BASE Ignore_Default_Tool 1
# PROP Exclude_From_Build 1
# PROP Ignore_Default_Tool 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\3rdparty\SDL-WIN\lib\x86\SDLmain.lib"

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1
# PROP Ignore_Default_Tool 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP BASE Ignore_Default_Tool 1
# PROP Exclude_From_Build 1
# PROP Ignore_Default_Tool 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

!ENDIF 

# End Source File
# End Group
# Begin Group "various"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdparty\various\neochrome.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\various\sound.h
# End Source File
# End Group
# Begin Group "FreeImage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdparty\FreeImage\FreeImage.h
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\FreeImage\FreeImage.lib
# End Source File
# End Group
# Begin Group "ArchiveAccess"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\3rdparty\ArchiveAccess\ArchiveAccess\ArchiveAccessSSE.cpp
DEP_CPP_ARCHI=\
	"..\..\3rdparty\ArchiveAccess\ArchiveAccess\ArchiveAccessBase.h"\
	"..\..\3rdparty\ArchiveAccess\ArchiveAccess\ArchiveAccessDynamic.h"\
	"..\..\3rdparty\ArchiveAccess\ArchiveAccess\bgstr.h"\
	"..\..\3rdparty\ArchiveAccess\CPP\Common\Defs.h"\
	"..\..\3rdparty\ArchiveAccess\CPP\Common\myCom.h"\
	"..\..\3rdparty\ArchiveAccess\CPP\Common\MyGuidDef.h"\
	"..\..\3rdparty\ArchiveAccess\CPP\Common\MyWindows.h"\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_ARCHI=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\3rdparty\ArchiveAccess\ArchiveAccess\ArchiveAccessSSE.h
# End Source File
# End Group
# End Group
# Begin Group "Object Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\steem\asm\asm_draw_VC.obj
# End Source File
# Begin Source File

SOURCE=..\..\steem\asm\asm_osd_VC.obj
# End Source File
# End Group
# Begin Group "code"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\steem\code\acc.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\archive.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\associate.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\blitter.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\boiler.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\controls.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\cpu.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\cpuinit.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\d2.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\dataloadsave.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\debug.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\debug_emu.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\dir_id.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\diskman.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\diskman_diags.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\diskman_drag.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\display.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\draw.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\dwin_edit.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\emulator.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\fdc.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\floppy_drive.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\gui.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\harddiskman.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\hdimg.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\historylist.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\ikbd.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\infobox.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\init_sound.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\iolist.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\ior.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\iow.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\key_table.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\loadsave.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\loadsave_emu.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\macros.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\mem_browser.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\mfp.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\midi.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\mr_static.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\notifyinit.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\onegame.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\options.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\options_create.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\osd.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\palette.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\patchesbox.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\psg.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\reset.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\rs232.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\run.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\screen_saver.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\shortcutbox.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\steemintro.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stemdialogs.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stemdos.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stemwin.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stjoy.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stjoy_directinput.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\stports.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\trace.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\binary.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\choosefolder.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\choosefolder.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\circularbuffer.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\circularbuffer.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\clarity.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\colcombo.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\colcombo.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\configstorefile.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\configstorefile.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\configstorefile_bad.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\configstorefile_list.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\dderr_meaning.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\dderr_strings.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\ddutil.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\di_get_contents.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\di_get_contents.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\directory_tree.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\directory_tree.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\dirsearch.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\dirsearch.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\dynamicarray.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\dynamicarray.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\easycompress.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\easycompress.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\easystr.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\easystr.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\easystringlist.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\easystringlist.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\fixed.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\input_prompt.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\input_prompt.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\internal_speaker.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\lbprintf.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\myddutil.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\mymisc.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\mymisc.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\notwin_mymisc.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\notwin_mymisc.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\portio.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\portio.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\scrollingcontrolswin.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\scrollingcontrolswin.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\sprites.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\tbrowseforfolder.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\wordwrapper.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\wordwrapper.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "SSE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSE.cpp
DEP_CPP_SSE_C=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSE_C=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSE.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSE6301.cpp
DEP_CPP_SSE63=\
	"..\..\3rdparty\6301\6301.h"\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easystr.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\acia.decla.h"\
	"..\..\steem\code\acia.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\ikbd.decla.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSE6301.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEFrameReport.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEShifter.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSE63=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\include\beos\be_mymisc.h"\
	

!IF  "$(CFG)" == "Steem - Win32 Debug"

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSE6301.h
# End Source File
# Begin Source File

SOURCE=..\..\Steem\code\SSE\SSEAcsi.cpp
DEP_CPP_SSEAC=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easystr.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\mfp.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\Steem\code\SSE\SSEAcsi.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEAC=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\include\beos\be_mymisc.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Steem\code\SSE\SSEAcsi.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSECapsImg.cpp
DEP_CPP_SSECA=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\dsp\dsp.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\include\binary.h"\
	"..\..\include\circularbuffer.h"\
	"..\..\include\clarity.h"\
	"..\..\include\configstorefile.h"\
	"..\..\include\directory_tree.h"\
	"..\..\include\dirsearch.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easycompress.h"\
	"..\..\include\easystr.h"\
	"..\..\include\easystringlist.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\portio.h"\
	"..\..\include\x\hxc.h"\
	"..\..\include\x\hxc_alert.h"\
	"..\..\include\x\hxc_dir_lv.h"\
	"..\..\include\x\hxc_fileselect.h"\
	"..\..\include\x\hxc_popup.h"\
	"..\..\include\x\hxc_popuphints.h"\
	"..\..\include\x\icongroup.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\blitter.decla.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\d2.decla.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\diskman.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\gui.decla.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\mfp.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\psg.decla.h"\
	"..\..\steem\code\resnum.decla.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\steem\code\SSE\SSECpu.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEFrameReport.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEInterrupt.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSEShifter.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\code\stemdialogs.decla.h"\
	"..\..\steem\code\stemdialogs.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSECA=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\include\beos\be_mymisc.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSECapsImg.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSECpu.cpp
DEP_CPP_SSECP=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\include\binary.h"\
	"..\..\include\circularbuffer.h"\
	"..\..\include\clarity.h"\
	"..\..\include\configstorefile.h"\
	"..\..\include\dirsearch.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easycompress.h"\
	"..\..\include\easystr.h"\
	"..\..\include\easystringlist.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\portio.h"\
	"..\..\include\x\hxc.h"\
	"..\..\include\x\hxc_alert.h"\
	"..\..\include\x\hxc_fileselect.h"\
	"..\..\include\x\hxc_popup.h"\
	"..\..\include\x\hxc_popuphints.h"\
	"..\..\include\x\icongroup.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\blitter.decla.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\d2.decla.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\gui.decla.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\mfp.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\resnum.decla.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSECpu.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEFrameReport.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEShifter.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSECP=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\include\beos\be_mymisc.h"\
	

!IF  "$(CFG)" == "Steem - Win32 Debug"

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSECpu.h
# End Source File
# Begin Source File

SOURCE=..\..\Steem\code\SSE\SSEDebug.cpp
DEP_CPP_SSEDE=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\d3d9.h"\
	"..\..\3rdparty\d3d\d3d9caps.h"\
	"..\..\3rdparty\d3d\D3d9types.h"\
	"..\..\3rdparty\d3d\d3dx9.h"\
	"..\..\3rdparty\d3d\d3dx9anim.h"\
	"..\..\3rdparty\d3d\d3dx9core.h"\
	"..\..\3rdparty\d3d\d3dx9effect.h"\
	"..\..\3rdparty\d3d\D3dx9math.h"\
	"..\..\3rdparty\d3d\d3dx9math.inl"\
	"..\..\3rdparty\d3d\d3dx9mesh.h"\
	"..\..\3rdparty\d3d\d3dx9shader.h"\
	"..\..\3rdparty\d3d\d3dx9shape.h"\
	"..\..\3rdparty\d3d\d3dx9tex.h"\
	"..\..\3rdparty\d3d\d3dx9xof.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\FreeImage\FreeImage.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\3rdparty\various\neochrome.h"\
	"..\..\include\binary.h"\
	"..\..\include\circularbuffer.h"\
	"..\..\include\clarity.h"\
	"..\..\include\configstorefile.h"\
	"..\..\include\directory_tree.h"\
	"..\..\include\dirsearch.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easycompress.h"\
	"..\..\include\easystr.h"\
	"..\..\include\easystringlist.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\portio.h"\
	"..\..\include\x\hxc.h"\
	"..\..\include\x\hxc_alert.h"\
	"..\..\include\x\hxc_fileselect.h"\
	"..\..\include\x\hxc_popup.h"\
	"..\..\include\x\hxc_popuphints.h"\
	"..\..\include\x\icongroup.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\display.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\gui.decla.h"\
	"..\..\steem\code\harddiskman.decla.h"\
	"..\..\steem\code\mfp.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\resnum.decla.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEMMU.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\code\stemdialogs.decla.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEDE=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\include\beos\be_mymisc.h"\
	"..\..\steem\code\d3d9.h"\
	"..\..\steem\code\D3d9types.h"\
	"..\..\steem\code\d3dx9core.h"\
	"..\..\steem\code\D3dx9math.h"\
	

!IF  "$(CFG)" == "Steem - Win32 Debug"

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Steem\code\SSE\SSEDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEDecla.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEDisk.cpp
DEP_CPP_SSEDI=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEDI=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEDisk.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEDma.cpp
DEP_CPP_SSEDM=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\configstorefile.h"\
	"..\..\include\directory_tree.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easystr.h"\
	"..\..\include\easystringlist.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\x\hxc.h"\
	"..\..\include\x\icongroup.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\blitter.decla.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\d2.decla.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\harddiskman.decla.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\mfp.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\Steem\code\SSE\SSEAcsi.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\steem\code\SSE\SSECpu.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEFrameReport.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSEShifter.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\code\stemdialogs.decla.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEDM=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\include\beos\be_mymisc.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEDma.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEDrive.cpp
DEP_CPP_SSEDR=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\dsp\dsp.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\3rdparty\various\sound.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\blitter.decla.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\d2.decla.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\mfp.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\psg.decla.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\steem\code\SSE\SSECpu.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEFrameReport.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEInterrupt.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSEShifter.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEDR=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEDrive.h
# End Source File
# Begin Source File

SOURCE=..\..\Steem\code\SSE\SSEFloppy.cpp
DEP_CPP_SSEFL=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\dsp\dsp.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\configstorefile.h"\
	"..\..\include\directory_tree.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easystr.h"\
	"..\..\include\easystringlist.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\x\hxc.h"\
	"..\..\include\x\hxc_dir_lv.h"\
	"..\..\include\x\hxc_popup.h"\
	"..\..\include\x\icongroup.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\blitter.decla.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\d2.decla.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\diskman.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\mfp.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\psg.decla.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\steem\code\SSE\SSECpu.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEFrameReport.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEInterrupt.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSEShifter.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\code\stemdialogs.decla.h"\
	"..\..\steem\code\stemdialogs.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEFL=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\include\beos\be_mymisc.h"\
	

!IF  "$(CFG)" == "Steem - Win32 Debug"

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Steem\code\SSE\SSEFloppy.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEFrameReport.cpp
DEP_CPP_SSEFR=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEFrameReport.h"\
	"..\..\steem\code\SSE\SSEMMU.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEShifter.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEFR=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	

!IF  "$(CFG)" == "Steem - Win32 Debug"

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEFrameReport.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEGhostDisk.cpp
DEP_CPP_SSEGH=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEGH=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEGhostDisk.h
# End Source File
# Begin Source File

SOURCE=..\..\Steem\code\SSE\SSEGlue.cpp
DEP_CPP_SSEGL=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\Steem\code\SSE\SSEGlue.h"\
	"..\..\steem\code\SSE\SSEMMU.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEGL=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Steem\code\SSE\SSEGlue.h
# End Source File
# Begin Source File

SOURCE=..\..\Steem\code\SSE\SSEHfe.cpp
DEP_CPP_SSEHF=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\easystr.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEHF=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\include\beos\be_mymisc.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Steem\code\SSE\SSEHfe.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEInline.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEInterrupt.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEInterrupt.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEMMU.cpp
DEP_CPP_SSEMM=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEMMU.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEMM=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEMMU.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEOption.cpp
DEP_CPP_SSEOP=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEOP=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEOption.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEParameters.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEScp.cpp
DEP_CPP_SSESC=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\dsp\dsp.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\blitter.decla.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\d2.decla.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\mfp.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\psg.decla.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\steem\code\SSE\SSECpu.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEFrameReport.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEInterrupt.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSEShifter.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSESC=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEScp.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSESDL.cpp
DEP_CPP_SSESD=\
	"..\..\3rdparty\SDL-WIN\include\begin_code.h"\
	"..\..\3rdparty\SDL-WIN\include\close_code.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_active.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_audio.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_cdrom.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_dreamcast.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_macos.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_macosx.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_minimal.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_os2.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_symbian.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_config_win32.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_cpuinfo.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_endian.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_error.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_events.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_joystick.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_keyboard.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_keysym.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_loadso.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_main.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_mouse.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_mutex.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_platform.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_quit.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_rwops.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_stdinc.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_thread.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_timer.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_version.h"\
	"..\..\3rdparty\SDL-WIN\include\SDL_video.h"\
	"..\..\steem\code\SSE\SSESDL.h"\
	

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSESDL.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEShifter.cpp
DEP_CPP_SSESH=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\d3d9.h"\
	"..\..\3rdparty\d3d\d3d9caps.h"\
	"..\..\3rdparty\d3d\D3d9types.h"\
	"..\..\3rdparty\d3d\d3dx9.h"\
	"..\..\3rdparty\d3d\d3dx9anim.h"\
	"..\..\3rdparty\d3d\d3dx9core.h"\
	"..\..\3rdparty\d3d\d3dx9effect.h"\
	"..\..\3rdparty\d3d\D3dx9math.h"\
	"..\..\3rdparty\d3d\d3dx9math.inl"\
	"..\..\3rdparty\d3d\d3dx9mesh.h"\
	"..\..\3rdparty\d3d\d3dx9shader.h"\
	"..\..\3rdparty\d3d\d3dx9shape.h"\
	"..\..\3rdparty\d3d\d3dx9tex.h"\
	"..\..\3rdparty\d3d\d3dx9xof.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\FreeImage\FreeImage.h"\
	"..\..\3rdparty\various\neochrome.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\display.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\mfp.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\palette.decla.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEFrameReport.cpp"\
	"..\..\steem\code\SSE\SSEFrameReport.h"\
	"..\..\Steem\code\SSE\SSEGlue.h"\
	"..\..\steem\code\SSE\SSEMMU.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEShifter.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSESH=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\steem\code\d3d9.h"\
	"..\..\steem\code\D3d9types.h"\
	"..\..\steem\code\d3dx9core.h"\
	"..\..\steem\code\D3dx9math.h"\
	

!IF  "$(CFG)" == "Steem - Win32 Debug"

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEShifter.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEShifterEvents.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEShifterEvents.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSESTF.cpp
DEP_CPP_SSEST=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\include\binary.h"\
	"..\..\include\circularbuffer.h"\
	"..\..\include\clarity.h"\
	"..\..\include\configstorefile.h"\
	"..\..\include\dirsearch.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easycompress.h"\
	"..\..\include\easystr.h"\
	"..\..\include\easystringlist.h"\
	"..\..\include\mymisc.h"\
	"..\..\include\notwin_mymisc.h"\
	"..\..\include\portio.h"\
	"..\..\include\x\hxc.h"\
	"..\..\include\x\hxc_alert.h"\
	"..\..\include\x\hxc_fileselect.h"\
	"..\..\include\x\hxc_popup.h"\
	"..\..\include\x\hxc_popuphints.h"\
	"..\..\include\x\icongroup.h"\
	"..\..\include\x\x_mymisc.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\gui.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\resnum.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\Steem\code\SSE\SSEGlue.h"\
	"..\..\steem\code\SSE\SSEInterrupt.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEST=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	"..\..\include\beos\be_mymisc.h"\
	

!IF  "$(CFG)" == "Steem - Win32 Debug"

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSESTF.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSESTW.cpp
DEP_CPP_SSESTW=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSESTW=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSESTW.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEVideo.cpp
DEP_CPP_SSEVI=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEVideo.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEVI=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	

!IF  "$(CFG)" == "Steem - Win32 Debug"

# ADD CPP /Zp1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEVideo.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEWD1772.cpp
DEP_CPP_SSEWD=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\caps\CapsAPI.h"\
	"..\..\3rdparty\caps\CapsFDC.h"\
	"..\..\3rdparty\caps\CapsLib.h"\
	"..\..\3rdparty\caps\comlib.h"\
	"..\..\3rdparty\caps\CommonTypes.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\dsp\dsp.h"\
	"..\..\3rdparty\hfe\hfe_format.h"\
	"..\..\3rdparty\hfe\libhxcfe.h"\
	"..\..\3rdparty\hfe\plugins_id.h"\
	"..\..\3rdparty\pasti\pasti.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\fdc.decla.h"\
	"..\..\steem\code\floppy_drive.decla.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\mfp.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\psg.decla.h"\
	"..\..\steem\code\run.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\steem\code\SSE\SSECapsImg.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEDisk.h"\
	"..\..\steem\code\SSE\SSEDma.h"\
	"..\..\steem\code\SSE\SSEDrive.h"\
	"..\..\Steem\code\SSE\SSEFloppy.h"\
	"..\..\steem\code\SSE\SSEGhostDisk.h"\
	"..\..\Steem\code\SSE\SSEHfe.h"\
	"..\..\steem\code\SSE\SSEInterrupt.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEScp.h"\
	"..\..\steem\code\SSE\SSESTF.h"\
	"..\..\steem\code\SSE\SSESTW.h"\
	"..\..\steem\code\SSE\SSEWD1772.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEWD=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEWD1772.h
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEYM2149.cpp
DEP_CPP_SSEYM=\
	"..\..\3rdparty\avi\AviFile.h"\
	"..\..\3rdparty\d3d\ddraw.h"\
	"..\..\3rdparty\d3d\dinput.h"\
	"..\..\3rdparty\d3d\dsound.h"\
	"..\..\3rdparty\d3d\sal.h"\
	"..\..\3rdparty\dsp\dsp.h"\
	"..\..\include\binary.h"\
	"..\..\include\clarity.h"\
	"..\..\include\dynamicarray.h"\
	"..\..\include\easystr.h"\
	"..\..\steem\code\acc.decla.h"\
	"..\..\steem\code\conditions.h"\
	"..\..\steem\code\cpu.decla.h"\
	"..\..\steem\code\debug_emu.decla.h"\
	"..\..\steem\code\draw.decla.h"\
	"..\..\steem\code\emulator.decla.h"\
	"..\..\steem\code\iorw.decla.h"\
	"..\..\steem\code\notwindows.h"\
	"..\..\steem\code\psg.decla.h"\
	"..\..\steem\code\SSE\SSE.h"\
	"..\..\Steem\code\SSE\SSEDebug.h"\
	"..\..\steem\code\SSE\SSEDecla.h"\
	"..\..\steem\code\SSE\SSEOption.h"\
	"..\..\steem\code\SSE\SSEParameters.h"\
	"..\..\steem\code\SSE\SSEYM2149.h"\
	"..\..\steem\code\steemh.decla.h"\
	"..\..\steem\pch.h"\
	
NODEP_CPP_SSEYM=\
	"..\..\3rdparty\d3d\codeanalysis\sourceannotations.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\steem\code\SSE\SSEYM2149.h
# End Source File
# End Group
# Begin Group "Unix"

# PROP Default_Filter ""
# Begin Group "include_x"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\x\hxc.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_alert.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_alert.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_button.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_dir_lv.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_dir_lv.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_dropdown.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_edit.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_fileselect.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_fileselect.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_listview.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_popup.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_popup.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_popuphints.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_popuphints.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_prompt.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_prompt.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_scrollarea.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_scrollbar.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\hxc_textdisplay.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\icongroup.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\icongroup.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "code_x"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\steem\code\x\x_controls.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_controls.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_diskman.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_display.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_gui.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_harddiskman.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_infobox.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_joy.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_midi.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\x_mymisc.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\x_mymisc.h

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_notifyinit.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_options.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_options_create.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_patchesbox.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\include\x\x_portio.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_screen_saver.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_shortcutbox.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_sound.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_sound_portaudio.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_sound_rtaudio.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_stemdialogs.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\steem\code\x\x_stemwin.cpp

!IF  "$(CFG)" == "Steem - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Release_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Steem - Win32 Boiler Debug_modules"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=..\..\Steem\rc\charset.blk
# End Source File
# End Target
# End Project
