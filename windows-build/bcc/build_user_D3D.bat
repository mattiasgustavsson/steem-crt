@echo off

echo Build script for Steem SSE - BCC build

call build_bcc_set.bat

set PROGRAMNAME=Beta BCC.exe

del "%OUT%\6301.obj"
del "%OUT%\AviFile.obj"
del "%OUT%\dsp.obj"
del "%OUT%\*.exe"
del "%OUT%\*.dll"

rem call build_bcc_asm.bat

echo -----------------------------------------------
echo Building 3rd party code using Borland C/C++ 5.5
echo -----------------------------------------------
"%BCCPATH%make.exe" -fmakefil3.txt -DDONT_ALLOW_DEBUG 3rdparty
echo ------------------------------------------
echo Building Steem SSE using Borland C/C++ 5.5
echo ------------------------------------------
"%BCCPATH%make.exe" -fmakefil3.txt -DDONT_ALLOW_DEBUG -DBCC_BUILD


if exist "%OUT%\Steem.exe" (ren "%OUT%\Steem.exe" "%PROGRAMNAME%"
copy "%OUT%\%PROGRAMNAME%" "%COPYPATH%")


pause
