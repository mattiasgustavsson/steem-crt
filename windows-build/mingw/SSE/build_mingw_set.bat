@echo off
rem The following depends on systems, edit as needed

set computer=COMPUTER2

if %computer%==COMPUTER1 (
set NASMPATH=../../../../../nasm/
set MINGWROOT=..\..\..\..\..\MinGW\
set MINGWPATH=..\..\..\..\..\MinGW\bin\
set COPYPATH=..\..\..\..\emu\Steem\
rem set COPYPATH=.
set ROOT=..\..\..\)

if %computer%==COMPUTER2 (
set NASMPATH=c:/console/nasm/
set MINGWROOT=c:/console/MinGW
set MINGWPATH=c:/console/MinGW/bin/
set COPYPATH=d:/emu/st/bin/steem/
set ROOT=../../..)

rem manually create o directory if necessary
set OUT=o
set PATH=%PATH%;%MINGWPATH%


