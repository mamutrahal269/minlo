org 0x7E00
bits 16
%include "config.inc"
%define DATA_LOAD_ADDRESS code_end
%define DESTINATION_ADDRESS 0x100000
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

%if 0
    mov al, 0x10             ; не знаю почему,но без этого dap ломается. можно поробовать убрать
    mov [dap.size], al
    mov eax, 0
    mov [dap.res], al
    mov [dap.sectors], ax
    mov [dap.buffer_segment], ax
    mov dword[dap.lba_high], eax
    mov ax, DATA_LOAD_ADDRESS
    mov [dap.buffer_offset], ax
    mov eax, 2
    mov [dap.lba_low], eax
%endif


%if TOTAL_SECTORS <= SECTORS_PER_LOAD
    mov ecx, TOTAL_SECTORS
    mov [dap.sectors], cx
    mov ah, 42h
    mov si, dap
    mov dl, BOOT_DRIVE
    int 13h
    jc disk_error

    SWITCH2PM

    mov esi, DATA_LOAD_ADDRESS
    mov edi, DESTINATION_ADDRESS
    mov ecx, TOTAL_SECTORS * 512
    rep movsb

    jmp DESTINATION_ADDRESS
%else
    mov cx, SECTORS_PER_LOAD
    mov [dap.sectors], cx
    mov ecx, TOTAL_SECTORS
    mov edi, DESTINATION_ADDRESS
loadloop:
    mov ah, 42h
    mov si, dap
    mov dl, BOOT_DRIVE
    int 13h
    jc disk_error
    test ah, ah
    jnz disk_error

    add dword[dap.lba_low], SECTORS_PER_LOAD
    adc dword[dap.lba_high], 0

    SWITCH2PM
    mov eax, ecx
    mov esi, DATA_LOAD_ADDRESS
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

    sub ecx, SECTORS_PER_LOAD
    jz done
    cmp ecx, SECTORS_PER_LOAD
    jb last

    jmp loadloop
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
    mov esi, DATA_LOAD_ADDRESS
    rep movsb

    jmp 0x100000
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
