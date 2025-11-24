bits 32
section .text
%ifdef DEBUG
	push ax
	push dx
    mov dx, 0x3FB
    mov al, 0x80
    out dx, al

    mov dx, 0x3F8
    mov al, 0x0C
    out dx, al

    mov dx, 0x3F9
    mov al, 0x00
    out dx, al

    mov dx, 0x3FB
    mov al, 0x03
    out dx, al

    mov dx, 0x3FC
    mov al, 0x03
    out dx, al
    pop dx
    pop ax
%endif
