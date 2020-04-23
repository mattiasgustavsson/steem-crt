@echo off

echo Build script for Steem SSE - MinGW build 

call build_mingw_set.bat

echo -----------------------------------------------
echo Building Steem SSE using MinGW
echo -----------------------------------------------
rem STEVEN_SEAGAL is defined in the makefile
mingw32-make.exe -fmakefile_MinGW.txt USER=1 MAKEFILE_PATH=makefile_MinGW.txt

rem call build_mingw_user_link.bat

pause



