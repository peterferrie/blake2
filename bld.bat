@echo off
yasm -DBIN -fbin b2sx.asm -ob2sx.bin
yasm -fwin32 b2sx.asm
cl /O2 /Os /GS- /DUSE_ASM b2s_test.c b2sx.obj
del *.obj