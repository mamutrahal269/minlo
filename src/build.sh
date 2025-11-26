#!/bin/sh
set -e

CC=i386-elf-gcc
CC_FLAGS="-Ofast -flto -ffunction-sections -fdata-sections -nostdlib -nostdinc++ -nostdinc -ffreestanding -m32 \
-fno-pic -fno-pie -fno-rtti -fno-exceptions -fno-threadsafe-statics -masm=intel -I../include"

$CC $CC_FLAGS -c minlib.cpp
$CC $CC_FLAGS -c printf.cpp
$CC $CC_FLAGS -c internal/arith64.c
$CC $CC_FLAGS -c test.cpp
$CC $CC_FLAGS -c vbe.cpp

nasm -f elf int386.asm -o int386.o
nasm -f elf -DDEBUG internal/initcom.asm -o initcom.o
nasm -f bin -DLOAD_SECTORS=120 0.asm -o 0.bin

$CC -m32 -nostdlib -nostdinc -ffreestanding -flto -Wl,--gc-sections \
-o boot.bin arith64.o int386.o minlib.o printf.o test.o vbe.o -T ../link.ld

cat 0.bin boot.bin > minlo.bin
dd if=minlo.bin of=../debug/c.img seek=0 bs=1 conv=notrunc

cd ../debug
bochs -q
