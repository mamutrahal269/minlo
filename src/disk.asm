bits 16
global read_disk
extern lba2chs

; input:
;	eax   - lba
;	dl    - disk num
;	ecx   - sectors
;	si    - disk geometry
;	es:bx - buffer
; output:
;	flags.CF = 0 - ok
;	flags.CF = 1 - error
section .text
read_disk:
.lp:
	push eax
	push edx
	push ecx
	movzx edx, si
	mov ecx, cyl
	call lba2chs
	test eax, eax
	jnz .failure
	
	mov ax, 0x0201
	mov cx, [cyl]
	xchg cl, ch
	shl cl, 6
	or cl, [sec]
	mov dh, [head]
	int 0x13
	jc .failure
	
	pop ecx
	pop edx
	pop eax
	add bx, 512
	jnc .0
	push ax
	mov ax, es
	add ax, 0x1000
	mov es, ax
	pop ax
.0:
	inc eax
	dec ecx
	jnz .lp

	clc
	ret
.failure:
	pop ecx
	pop edx
	pop eax
	stc
	ret
section .data
	cyl  dw 0
	head db 0
	sec  db 0
