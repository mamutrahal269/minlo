bits 32
org 0x100000

mov esi, msg
mov edi, 0xB8000
mov ah, 0x4F
putch:
    lodsb
    test al, al
    jz stop
    stosw
    jmp putch
stop:
    jmp $
msg db 'Hello, World!', 0
times 256*512 nop
