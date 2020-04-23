@echo off

echo Build script for Steem SSE - MinGW build with boiler

call build_mingw_set.bat

echo -----------------------------------------------
echo Building Steem SSE (Boiler) using MinGW
echo -----------------------------------------------
rem STEVEN_SEAGAL is defined in the makefile
mingw32-make.exe -fmakefile_MinGW.txt DEBUG=1 MAKEFILE_PATH=makefile_MinGW.txt

rem call build_mingw_debug_link.bat

pause


