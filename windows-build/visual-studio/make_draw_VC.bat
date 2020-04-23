set NASMPATH=D:\Console\nasm\



"%NASMPATH%nasm" -fwin32 -dWIN32 -d_VC_BUILD -oasm_draw_VC.obj -w+macro-params -w+macro-selfref -w+orphan-labels ../../steem/asm/asm_draw.asm
pause