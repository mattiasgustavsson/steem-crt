@echo off

echo Build script for Steem SSE - BCC build with Debugger

call build_bcc_set.bat

set PROGRAMNAME=BoilerBeta BCC.exe

del "%OUT%\6301.obj"
del "%OUT%\AviFile.obj"
del "%OUT%\dsp.obj"
del "%OUT%\*.exe"
del "%OUT%\*.exe"
del "%OUT%\*.dll"

call build_bcc_asm.bat

echo -----------------------------------------------
echo Building 3rd party code using Borland C/C++ 5.5
echo -----------------------------------------------
"%BCCPATH%make.exe" -fmakefil3.txt -DFORCE_DEBUG_BUILD -DSTEVEN_SEAGAL -DSSE_DD 3rdparty
rem pause
echo -----------------------------------------------------
echo Building Steem SSE (Debugger) using Borland C/C++ 5.5
echo -----------------------------------------------------
"%BCCPATH%make.exe" -fmakefil3.txt -DFORCE_DEBUG_BUILD -DBCC_BUILD -DSTEVEN_SEAGAL -DSSE_DD

rem "%BCCPATH%make.exe" -fmakefil3.txt -DFORCE_DEBUG_BUILD -DBCC_BUILD link  



if exist "%OUT%\Steem.exe" (ren "%OUT%\Steem.exe" "%PROGRAMNAME%"
copy "%OUT%\%PROGRAMNAME%" "%COPYPATH%")

pause

