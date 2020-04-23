@echo off

echo Build script for Steem SSE - MinGW build 

call build_mingw_set.bat

echo -----------------------------------------------
echo Building 3rd party code using MinGW
echo -----------------------------------------------
mingw32-make.exe -fmakefile_MinGW.txt USER=1 3rdparty MAKEFILE_PATH=makefile_MinGW.txt

pause



