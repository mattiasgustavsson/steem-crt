rem create a set for your computer

set computer=COMPUTER2

if %computer%==COMPUTER1 (
set NASMPATH=..\..\..\..\nasm\
rem manually create obj directory if necessary
set OUT=obj
set ROOT=..\..\
set BCCROOT=..\..\..\..\bcc\
set BCCPATH=..\..\..\..\bcc\Bin\
set COPYPATH=..\..\..\emu\Steem\)

if %computer%==COMPUTER2 (
set NASMPATH=c:\console\nasm\
rem manually create obj directory if necessary
set OUT=obj
set ROOT=..\..\
set BCCROOT=c:\console\bcc\
set BCCPATH=c:\console\bcc\Bin\
set COPYPATH=d:\emu\st\bin\steem\)

rem to check repo source
if %computer%==COMPUTER2B (
set NASMPATH=c:\console\nasm\
rem manually create obj directory if necessary
set OUT=obj
rem set ROOT=k:\download\steemsse\
set ROOT=D:\Temp\steemsse-code\steemsse\steemsse
set BCCROOT=c:\console\bcc\
set BCCPATH=c:\console\bcc\Bin\
set COPYPATH=d:\emu\st\bin\steem\)

rem to check repo source
if %computer%==COMPUTER2C (
set NASMPATH=c:\console\nasm\
rem manually create obj directory if necessary
set OUT=obj
rem set ROOT=k:\download\steemsse\
set ROOT=D:\Emu\ST\steem\SteemIO\working_copy\steemsse-code\steemsse\steemsse
rem set ROOT=K:\project\SteemIO\working_copy\steemsse-code\steemsse\steemsse
set BCCROOT=c:\console\bcc\
set BCCPATH=c:\console\bcc\Bin\
set COPYPATH=d:\emu\st\bin\steem\)