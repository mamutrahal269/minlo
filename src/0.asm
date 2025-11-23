bits 16
org 0x7C00

_start:
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov sp, 0x7BFF
	sti
	
	mov bp, 4 ; attemps
	movzx dx, dl
	push dx
readlp:
	mov di, sp
	mov dx, [di]
	test bp, bp
	dec bp
	jz disk_error
	xor ah, ah
	int 0x13
	jc disk_error
	
	mov di, sp
	mov dx, [di]
	mov ah, 0x02
	mov al, [load_sectors]
	xor ch, ch
	mov cl, 2
	mov bx, 0x7E0
	mov es, bx
	xor bx, bx
	int 0x13
	jc readlp
	
	or al, 2
	and al, 0xFE
	out 0x92, al
	
	cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:.pm
bits 32
.pm:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
	pop dx
	movzx ax, [load_sectors]
	jmp 0x7E00

bits 16
disk_error:
	mov si, disk_msg
	call print_msg
	jmp die
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
die:
	mov cx, 15
	mov ah, 0x86
	int 0x15
	int 0x18
disk_msg db 'Disk read error. Boot stopped...', 0x0D, 0x0A, 0 
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
    
times 445 - ($-$$) db 0
%ifdef LOAD_SECTORS
load_sectors db LOAD_SECTORS
%else
load_sectors db 0
%endif
times 64 db 0
dw 0xAA55
