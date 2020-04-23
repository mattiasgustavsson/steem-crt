:: JLG This batch file can be used to create both object files from ASM input
:: You may need to ajust the path to the location where nasm has been installed

@set path=C:\Program Files (x86)\nasm;%path%
nasm -fwin32 -dWIN32 -o..\obj\asm_draw.obj -w+macro-params -w+macro-selfref -w+orphan-labels asm_draw.asm
nasm -fwin32 -dWIN32 -o..\obj\asm_osd.obj  -w+macro-params -w+macro-selfref -w+orphan-labels asm_osd_draw.asm
@pause

