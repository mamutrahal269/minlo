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

    call a20_test
    jnc a20_ok
    
    call    a20wait  
    mov     al,0xAD  
    out     0x64,al  
    call    a20wait  
    mov     al,0xD0  
    out     0x64,al  
    call    a20wait2  
    in      al,0x60  
    push    eax  
    call    a20wait  
    mov     al,0xD1  
    out     0x64,al  
    call    a20wait  
    pop     eax  
    or      al,2  
    out     0x60,al  
    call    a20wait  
    mov     al,0xAE  
    out     0x64,al
    
    call a20_test
    jnc a20_ok
    
    in al, 0x92
    test al, 2
    jnz fast_a20
    call a20_test
    jc ax2401h
fast_a20:
    or al, 2
    and al, 0xFE
    out 0x92, al
    
    call a20_test
    jnc a20_ok
    
ax2401h:
    mov ax, 2401h
    int 15h
    jc eeh_port
    test ah, ah
    jz a20_ok
    
    call a20_test
    jnc a20_ok
eeh_port:
    in al, 0xEE
    
    call a20_test
    jnc a20_failed
a20_ok:
    jmp 0x7E00
;----------------------------------------------------
bits 16
a20_test:
    push es
    push ds
    mov ax, 0x0000
    mov es, ax
    mov ax, word[es:0x7DFE]
    mov dx, 0xFFFF
    mov ds, dx
    mov dx, word[ds:0x7E0E]
    cmp ax, dx
    jne .a20_enabled
    rol ax, 1
    mov word[es:0x7DFE], ax
    mov dx, word[ds:0x7E0E]
    cmp dx, ax
    pop ds
    pop es
    jne .a20_enabled
    
    stc
    ret
.a20_enabled:
    pop ds
    pop es
    clc
    ret

a20wait:
    in al, 0x64
    test al, 2
    jnz a20wait
    ret
a20wait2:
    in al, 0x64
    test al, 1
    jz a20wait
    ret

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
a20_failed:
    mov si, a20_msg
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
a20_msg db 'A20 line activation error.', 0x0D, 0x0A, 0
newline db 0x0D, 0x0A, 0

%if LODSECTRS > 127
    %error "too many sectors"
%endif

times 509 - ($-$$) db 0
boot_drive db 0
dw 0xAA55
