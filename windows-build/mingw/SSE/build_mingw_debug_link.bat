@echo off

call build_mingw_set.bat

set PROGRAMNAME=BoilerBetaMinGW.exe

echo -----------------------------------------------
echo linking Steem SSE Boiler MinGW build
echo -----------------------------------------------
mingw32-make.exe -fmakefile_MinGW.txt DEBUG=1 link MAKEFILE_PATH=makefile_MinGW.txt


if exist "%OUT%\Steem.exe" (ren "%OUT%\Steem.exe" "%PROGRAMNAME%"
copy "%OUT%\%PROGRAMNAME%" "%COPYPATH%")

pause

