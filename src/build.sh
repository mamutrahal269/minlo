#!/bin/sh
set -e
i386-elf-gcc -nostdlib -nostdinc++ -nostdinc -ffreestanding -m32 -fno-pic -fno-pie -fno-rtti -fno-exceptions -fno-threadsafe-statics -masm=intel -c -I../include minlib.cpp
i386-elf-gcc -nostdlib -nostdinc++ -nostdinc -ffreestanding -m32 -fno-pic -fno-pie -fno-rtti -fno-exceptions -fno-threadsafe-statics -masm=intel -c -I../include printf.cpp
i386-elf-gcc -nostdlib -nostdinc++ -nostdinc -ffreestanding -m32 -fno-pic -fno-pie -fno-rtti -fno-exceptions -fno-threadsafe-statics -masm=intel -c -I../include internal/arith64.c
i386-elf-gcc -nostdlib -nostdinc++ -nostdinc -ffreestanding -m32 -fno-pic -fno-pie -fno-rtti -fno-exceptions -fno-threadsafe-statics -masm=intel -c -I../include test.cpp
nasm -f elf int386.asm -o int386.o
nasm -f elf -DDEBUG internal/initcom.asm -o initcom.o
nasm -f bin -DLOAD_SECTORS=120 0.asm -o 0.bin
ld -m elf_i386 -T ../link.ld -o boot.bin arith64.o int386.o minlib.o printf.o test.o
cat 0.bin boot.bin > minlo.bin
dd if=minlo.bin of=../debug/c.img seek=0 bs=1 conv=notrunc
cd ../debug
bochs -q

