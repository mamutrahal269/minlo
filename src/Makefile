CC := i386-elf-gcc
ASM := nasm
CFLAGS := -Ofast -flto -ffunction-sections -fdata-sections -nostdlib -nostdinc++ -nostdinc -ffreestanding -m32 \
          -fno-pic -fno-pie -fno-rtti -fno-exceptions -fno-threadsafe-statics -masm=intel -I../include -MMD -MP -c
CLFLAGS := -m32 -nostdlib -nostdinc -ffreestanding -T ../link.ld -flto -Wl,--gc-sections
ASMFLAGS_ELF := -f elf
ASMFLAGS_BIN := -f bin
OBJS := minlib.o printf.o test.o vbe.o internal/arith64.o int386.o initcom.o
.PHONY: all clean write run

all: minlo.bin
%.o : %.cpp
	$(CC) $(CFLAGS) $< -o $@
%.o : %.c
	$(CC) $(CFLAGS) $< -o $@
int386.o: int386.asm
	$(ASM) $(ASMFLAGS_ELF) $< -o $@
initcom.o: internal/initcom.asm
	$(ASM) $(ASMFLAGS_ELF) -DDEBUG $< -o $@
0.bin: 0.asm
	$(ASM) $(ASMFLAGS_BIN) -DLOAD_SECTORS=120 $< -o $@
boot.bin: $(OBJS)
	$(CC) $(CLFLAGS) $^ -o $@
minlo.bin: 0.bin boot.bin
	cat 0.bin boot.bin > $@
clean:
	find . -type f \( -name "*.o" -o -name "*.bin" -o -name "*.d" \) -exec rm -f {} +
write: minlo.bin ../debug/c.img
	dd if=/dev/zero of=../debug/c.img bs=1 count=$(shell stat -c%s ../debug/c.img) conv=notrunc
	dd if=minlo.bin of=../debug/c.img seek=0 bs=1 conv=notrunc
run: ../debug/c.img
	cd ../debug && bochs -q
-include $(OBJS:.o=.d)
