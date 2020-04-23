@echo off

echo Build script for Steem SSE - MinGW build 

call build_mingw_set.bat

call build_asm_mingw.bat

echo -----------------------------------------------
echo Building 3rd party code using MinGW
echo -----------------------------------------------
mingw32-make.exe -fmakefile_MinGW.txt USER=1 3rdparty MAKEFILE_PATH=makefile_MinGW.txt

echo -----------------------------------------------
echo Building Steem SSE using MinGW
echo -----------------------------------------------
rem STEVEN_SEAGAL is defined in the makefile
mingw32-make.exe -fmakefile_MinGW.txt USER=1 MAKEFILE_PATH=makefile_MinGW.txt

call build_mingw_user_link.bat





