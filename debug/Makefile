CC := i386-elf-gcc
ASM := nasm

SRC_DIR := ../src
INC_DIR := ../include

CFLAGS := -Ofast -march=i386 -mtune=i386 -flto -ffunction-sections -fdata-sections -nostdlib -nostdinc++ -nostdinc \
          -ffreestanding -m32 -fno-pic -fno-pie -fno-rtti -fno-exceptions -fno-threadsafe-statics \
          -masm=intel -I$(INC_DIR) -MMD -MP -c

CLFLAGS := -m32 -nostdlib -nostdinc -ffreestanding -T ../link.ld -flto -Wl,--gc-sections

ASMFLAGS_ELF := -f elf
ASMFLAGS_BIN := -f bin

OBJS := arith64.o disk.o int386.o memory.o minlib.o printf.o vbe.o test.o

.PHONY: all clean write run

all: minlo.bin

%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $< -o $@
%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

test.o: test.cpp
	$(CC) $(CFLAGS) $< -o $@
int386.o: $(SRC_DIR)/int386.asm
	$(ASM) $(ASMFLAGS_ELF) $< -o $@
0.bin: $(SRC_DIR)/0.asm
	$(ASM) $(ASMFLAGS_BIN) -DLOAD_SECTORS=120 $< -o $@
boot.bin: $(OBJS)
	$(CC) $(CLFLAGS) $^ -o $@

minlo.bin: 0.bin boot.bin
	cat 0.bin boot.bin > $@

clean:
	find . -type f \( -name "*.o" -o -name "*.bin" -o -name "*.d" \) -exec rm -f {} +

c.img:
	bximage -func=create -hd=16M -imgmode=flat -sectsize=512 -q c.img
write: minlo.bin c.img
	dd if=minlo.bin of=c.img seek=0 bs=1 conv=notrunc

run: c.img
	bochs -q

-include $(OBJS:.o=.d)
