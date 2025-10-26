bits 16

global extrd
extern boot_drive

section .text
; eax - buffer physical addr
; ebx - lba low
; ecx - lba high
; dx - sectors

extrd:
	push eax
	and eax, 0xF
	mov [dap.buffer_offset], ax
	pop eax
	shr eax, 4
	mov [dap.buffer_segment], ax
	mov [dap.lba_low], ebx
	mov [dap.lba_high], ecx
	mov [dap.sectors], dx
	
	mov ah, 0x42
	mov si, dap
	mov dl, [boot_drive]
	int 0x13
	ret

section .data
dap:
    .size           db 0x10
    .res            db 0
    .sectors        dw 0
    .buffer_offset  dw 0
    .buffer_segment dw 0
    .lba_low        dd 0
    .lba_high       dd 0
