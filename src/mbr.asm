%define LODSECTRS 8
bits 16
org 0x7C00
main16:
    mov [boot_drive], dl
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7BFF
    sti

    mov ah,0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 13h
    jc unsup_lba
    cmp bx, 0xAA55
    jne unsup_lba
    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 13h
    jc disk_error

    mov ah, 0x06
    mov al, 0
    mov bh, 0x07
    mov cx, 0
    mov dx, 184Fh
    int 0x10

    mov ah, 1
    mov ch, 20h
    xor cl, cl
    int 10h

    push es
    push ds
    mov ax, 0x0000
    mov es, ax
    mov ax, word[es:0x7DFE]
    mov dx, 0xFFFF
    mov ds, dx
    mov dx, word[ds:0x7E0E]
    cmp ax, dx
    jne .a20_ok
    shl ax, 8
    mov word[es:0x7DFE], ax
    mov dx, word[ds:0x7E0E]
    cmp dx, ax
    pop ds
    pop es
    jne .a20_ok

    in al, 0x92
    test al, 2
    jnz .a20_ok
    or al, 2
    and al, 0xFE
    out 0x92, al
.a20_ok:

    jmp 0x7E00
;----------------------------------------------------
bits 16
disk_error:
    mov si, disk_msg
    push ax
    call print_msg
    pop ax
    mov al, ah
    call bout
    mov si, newline
    call print_msg
    jmp stop
unsup_lba:
    mov si, lba_err
    call print_msg
    jmp stop
bout:
    push ax
    shr al, 4
    call nout
    pop ax
nout:
    and al, 0x0F
    daa
    add al, 0xF0
    adc al, 0x40
cout:
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    ret

print_msg:
    mov ah, 0x0E
    lodsb
    test al, al
    jz .done
    mov bl, 0x00
    int 10h
    jmp print_msg
.done:
    ret

stop:
    mov cx, 15
    mov ah, 0x86
    int 0x15
    int 0x18

;----------------------------------------------------
dap:
                    db 0x10
                    db 0
    .sectors        dw LODSECTRS
    .buffer_offset  dw 0x7E00
    .buffer_segment dw 0
    .lba_low        dd 0x1
    .lba_high       dd 0
;----------------------------------------------------

disk_msg db 'Disk read error. Code: 0x', 0
lba_err db 'Your storage device does not support LBA.', 0x0D, 0x0A, 0
newline db 0x0D, 0x0A, 0

%if LODSECTRS > 127
    %error "too many sectors"
%endif

times 509 - ($-$$) db 0
boot_drive db 0 ; save for second stage
dw 0xAA55

