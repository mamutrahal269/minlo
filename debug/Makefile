CC := i386-elf-gcc
ASM := nasm

SRC_DIR := ../src
INC_DIR := ../include

CFLAGS := -Ofast -flto -ffunction-sections -fdata-sections -nostdlib -nostdinc++ -nostdinc \
          -ffreestanding -m32 -fno-pic -fno-pie -fno-rtti -fno-exceptions -fno-threadsafe-statics \
          -masm=intel -I$(INC_DIR) -MMD -MP -c

CLFLAGS := -m32 -nostdlib -nostdinc -ffreestanding -T ../link.ld -flto -Wl,--gc-sections

ASMFLAGS_ELF := -f elf
ASMFLAGS_BIN := -f bin

OBJS := minlib.o printf.o test.o vbe.o int386.o arith64.o

.PHONY: all clean write run

all: minlo.bin

%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $< -o $@
%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

int386.o: $(SRC_DIR)/int386.asm
	$(ASM) $(ASMFLAGS_ELF) $< -o $@

arith64.o: $(SRC_DIR)/arith64.c
	$(CC) $(CFLAGS) $< -o $@

0.bin: $(SRC_DIR)/0.asm
	$(ASM) $(ASMFLAGS_BIN) -DLOAD_SECTORS=120 $< -o $@

boot.bin: $(OBJS)
	$(CC) $(CLFLAGS) $^ -o $@

minlo.bin: 0.bin boot.bin
	cat 0.bin boot.bin > $@

clean:
	find . -type f \( -name "*.o" -o -name "*.bin" -o -name "*.d" \) -exec rm -f {} +

write: minlo.bin ../debug/c.img
	dd if=/dev/zero of=c.img bs=1 count=$(shell stat -c%s c.img) conv=notrunc
	dd if=minlo.bin of=c.img seek=0 bs=1 conv=notrunc

run: c.img
	bochs -q

-include $(OBJS:.o=.d)
