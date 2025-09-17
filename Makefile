TARGETS = mbr.bin stage2.bin
.PHONY: all clean
all: $(TARGETS) minlo.bin
mbr.bin: src/mbr.asm
	nasm -f bin $< -o $@
stage2.bin: src/stage2.asm config.inc
	nasm -f bin $< -o $@
minlo.bin: $(TARGETS)
	cat $(TARGETS) > $@
clean:
	rm -f $(TARGETS) minlo.bin
