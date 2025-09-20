org 0x7E00
bits 16
%include "config.inc"
%define BUFFER code_end
%define DEST_ADDR 0x100000
%define BOOT_DRIVE byte [0x7C00 + 509]
%define STACKPTR 0x7DF0

%macro SWITCH2PM 0
    cli
    in al, 0x70
    or al, 0x80
    out 0x70, al
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:%%PM
bits 32
%%PM:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, STACKPTR

%endmacro

    cld
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, STACKPTR
    sti

mmap_ent equ 0x500
do_e820:
    mov di, 0x504
	xor ebx, ebx
	xor bp, bp
	mov edx, 0x0534D4150
	mov eax, 0xE820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15
	jc e820failed
	mov edx, 0x0534D4150
	cmp eax, edx
	jne e820failed
	test ebx, ebx
	je e820failed
	jmp .jmpin
.e820lp:
	mov eax, 0xe820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15
	jc .e820ok
	mov edx, 0x0534D4150
.jmpin:
	jcxz .skipent
	cmp cl, 20
	jbe short .notext
	test byte [es:di + 20], 1
	je short .skipent
.notext:
	mov ecx, [es:di + 8]
	or ecx, [es:di + 12]
	jz .skipent
	inc bp
	add di, 24
.skipent:
	test ebx, ebx
	jne .e820lp
.e820ok:
	mov [mmap_ent], bp

%if TOTAL_SECTORS <= SECTORS_PER_LOAD
    mov ecx, TOTAL_SECTORS
    mov [dap.sectors], cx
    mov ah, 42h
    mov si, dap
    mov dl, BOOT_DRIVE
    int 13h
    jc disk_error

    SWITCH2PM

    mov esi, BUFFER
    mov edi, DEST_ADDR
    mov ecx, TOTAL_SECTORS * 512
    rep movsb

    jmp DEST_ADDR
%else
    mov cx, SECTORS_PER_LOAD
    mov [dap.sectors], cx
    mov ecx, TOTAL_SECTORS
    mov edi, DEST_ADDR
loadloop:
    mov ah, 42h
    mov si, dap
    mov dl, BOOT_DRIVE
    int 13h
    jc disk_error
    test ah, ah
    jnz disk_error

    add dword[dap.lba_low], SECTORS_PER_LOAD

    SWITCH2PM
    mov eax, ecx
    mov esi, BUFFER
    mov ecx, SECTORS_PER_LOAD * 512
    rep movsb
    mov ecx, eax

    jmp 0x18:.pm16 ; !!!
bits 16
.pm16:
    mov eax, cr0
    btr eax, 0
    mov cr0, eax
    jmp 0:.rm
.rm:
    xor eax,eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, STACKPTR
    in al, 0x70
    and al, ~0x80
    out 0x70, al
    sti

    sub ecx, SECTORS_PER_LOAD
    jz done
%if TOTAL_SECTORS % SECTORS_PER_LOAD
    cmp ecx, SECTORS_PER_LOAD
    jb last
%endif
    jmp 0:loadloop
%if TOTAL_SECTORS % SECTORS_PER_LOAD
last:
    mov cx, TOTAL_SECTORS % SECTORS_PER_LOAD
    mov [dap.sectors], cx
    mov ah, 42h
    mov si, dap
    mov dl, BOOT_DRIVE
    int 13h
    jc disk_error
    test ah, ah
    jnz disk_error

    SWITCH2PM
    mov ecx, (TOTAL_SECTORS % SECTORS_PER_LOAD) * 512
    mov esi, BUFFER
    rep movsb

    jmp 0x100000
%endif
done:
    SWITCH2PM
    jmp 0x100000
%endif
bits 16
disk_error:
    mov esi, disk_err_msg
    xor ebx, ebx
    mov ah, 0x4F
    call print
    jmp $
e820failed:
    mov esi, e820failed_msg
    xor ebx, ebx
    mov ah, 0x4F
    call print
    jmp $
print:
    mov edi, 0xB8000
    add edi, ebx
.putch:
    lodsb
    test al, al
    jz .ok
    mov [edi], ax
    inc edi
    inc edi
    jmp .putch
.ok:
    ret

gdt_beg:
    dq 0

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 11001111b
    db 0x00

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

;           code 16
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 00000000b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_beg - 1
    dd gdt_beg

bits 16
dap:
    .size           db 0x10
    .res            db 0
    .sectors        dw 0
    .buffer_offset  dw BUFFER
    .buffer_segment dw 0
    .lba_low        dd START_SECTOR
    .lba_high       dd 0

disk_err_msg db '[bootloader : stage 2] Disk read error. Please, reboot the computer', 0
e820failed_msg db '[bootloader : stage 2] Memory detection failed. Please, reboot the computer', 0
BUFFER:
times 4096 - ($-$$) nop
