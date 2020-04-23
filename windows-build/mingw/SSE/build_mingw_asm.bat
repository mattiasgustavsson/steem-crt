@echo off

rem object files are different for mingw
rem this should be called only when the assembly code has been modified
rem (or you deleted your object files by accident)

call build_mingw_set.bat

echo Building assembly objects using NASM
"%NASMPATH%nasm" -fcoff -dMINGW -dWIN32 -o %OUT%\asm_draw.o -w+macro-params -w+macro-selfref -w+orphan-labels -i%ROOT%\steem\asm\ %ROOT%\steem\asm\asm_draw.asm  
"%NASMPATH%nasm" -fcoff -dMINGW -dWIN32 -o %OUT%\asm_osd_draw.o -w+macro-params -w+macro-selfref -w+orphan-labels -i%ROOT%\steem\asm\ %ROOT%\steem\asm\asm_osd_draw.asm 
"%NASMPATH%nasm" -fcoff -dMINGW -dWIN32 -o %OUT%\asm_portio.o -i%ROOT%\include\asm\ %ROOT%\include\asm\asm_portio.asm 
"%NASMPATH%nasm" -fcoff -dMINGW -dWIN32 -o %OUT%\asm_int.o -i%ROOT%\steem\asm\ %ROOT%\steem\asm\asm_int_16_2.asm

pause