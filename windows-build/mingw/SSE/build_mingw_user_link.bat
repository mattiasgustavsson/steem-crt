@echo off

echo Build script for Steem SSE - MinGW build with boiler

call build_mingw_set.bat

set PROGRAMNAME=SteemBetaMinGW.exe

echo -----------------------------------------------
echo linking Steem SSE MinGW build
echo -----------------------------------------------
mingw32-make.exe -fmakefile_MinGW.txt USER=1 link MAKEFILE_PATH=makefile_MinGW.txt

if exist "%OUT%\Steem.exe" (ren "%OUT%\Steem.exe" "%PROGRAMNAME%"
copy "%OUT%\%PROGRAMNAME%" "%COPYPATH%")

pause

