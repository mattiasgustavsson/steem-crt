@echo off

echo Build script for Steem SSE - MinGW build with boiler

call build_mingw_set.bat

del "%OUT%\6301.o"
del "%OUT%\AviFile.o"
del "%OUT%\dsp.o"
del "%OUT%\*.exe"

rem call build_asm_mingw.bat

echo -----------------------------------------------
echo Building 3rd party code using MinGW
echo -----------------------------------------------
mingw32-make.exe -fmakefile_MinGW.txt DEBUG=1 3rdparty MAKEFILE_PATH=makefile_MinGW.txt

pause


