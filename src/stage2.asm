org 0x7E00
bits 16
%include "config.inc"
%define BUFFER code_end
%define DEST_ADDR 0x100000
%define BOOT_DRIVE byte [0x7C00 + 509]

%macro SWITCH2PM 0
    cli
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
    mov esp, 0x7BFF

%endmacro

code_begin:
    cld
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7BFF
    sti


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
%error "too many sectors"
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

    mov eax, cr0
    btr eax, 0
    mov cr0, eax
    jmp 0:rm
bits 16
rm:
    xor eax,eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x7BFF
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
    mov ah, 0x40
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
    db 10011010b
    db 11001111b
    db 0x00

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
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
    .buffer_offset  dw DATA_LOAD_ADDRESS
    .buffer_segment dw 0
    .lba_low        dd START_SECTOR
    .lba_high       dd 0

disk_err_msg db '[bootloader : stage 2] Disk read error. Please, reboot the computer', 0
align 512, db 0
code_end:
