TARGETS = mbr.bin stage2.bin test.bin
.PHONY: all clean
all: $(TARGETS) minlo.bin c.img
mbr.bin: ../src/mbr.asm
	nasm -f bin -I.. $< -o $@
stage2.bin: ../src/stage2.asm ../config.inc
	nasm -f bin -I.. $< -o $@
test.bin: test.asm
	nasm -f bin -I.. $< -o $@
minlo.bin: $(TARGETS)
	cat $(TARGETS) > $@
c.img: minlo.bin
	dd if=$< of=$@ seek=0 bs=1 conv=notrunc
clean:
	rm -f $(TARGETS) minlo.bin
