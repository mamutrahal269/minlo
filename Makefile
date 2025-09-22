TARGETS = mbr.bin stage2.bin
DYNAMIC_CONFIG ?= 0
ifneq ($(DYNAMIC_CONFIG),0)
ASM_FLAGS = -DDYNAMIC_CONFIG -DTOTAL_SECTORS=$(TOTAL_SECTORS) -DSECTORS_PER_LOAD=$(SECTORS_PER_LOAD) -DSTART_SECTOR=$(START_SECTOR) -f bin
else
ASM_FLAGS = -f bin
endif
.PHONY: all clean
all: $(TARGETS) minlo.bin
mbr.bin: src/mbr.asm
	nasm $(ASM_FLAGS) $< -o $@
stage2.bin: src/stage2.asm config.inc
	nasm $(ASM_FLAGS) $< -o $@
minlo.bin: $(TARGETS)
	cat $(TARGETS) > $@
clean:
	rm -f $(TARGETS) minlo.bin
