@echo off

call build_bcc_set.bat

echo Building assembly objects using NASM
"%NASMPATH%nasm" -o obj\asm_draw.obj -fobj -dWIN32 -w+macro-params -w+macro-selfref -w+orphan-labels -i%ROOT%\steem\asm\ %ROOT%\steem\asm\asm_draw.asm  
"%NASMPATH%nasm" -o obj\asm_osd_draw.obj -fobj -dWIN32 -w+macro-params -w+macro-selfref -w+orphan-labels -i%ROOT%\steem\asm\ %ROOT%\steem\asm\asm_osd_draw.asm 
"%NASMPATH%nasm" -o obj\asm_portio.obj -fobj -dWIN32 -i%ROOT%\include\asm\ %ROOT%\include\asm\asm_portio.asm 
"%NASMPATH%nasm" -o obj\asm_int.obj -fobj -dWIN32 -i%ROOT%\steem\asm\ %ROOT%\steem\asm\asm_int_16_2.asm

rem pause