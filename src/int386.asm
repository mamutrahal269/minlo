%define EAX_OFF 0
%define EBX_OFF 4
%define ECX_OFF 8
%define EDX_OFF 12
%define ESI_OFF 16
%define EDI_OFF 20
%define EBP_OFF 24
%define EFLAGS_OFF 28
%define DS_OFF 32
%define ES_OFF 34
%define FS_OFF 36
%define GS_OFF 38
bits 32
global int386
section .text.int386
jmp skip
; eax - irq number
; edx - input buffer
; ecx - output buffer
int386: ;0x7E05
	sgdt [gdt_descriptor]
	mov [out_buf], ecx
	
 	pushad
	pushfd
 	push es
 	push fs
 	push gs
 	push ds
	
	push dword [edx + EAX_OFF]
	push dword [edx + EBX_OFF]
	push dword [edx + ECX_OFF]
	push dword [edx + EDX_OFF]
	push dword [edx + ESI_OFF]
	push dword [edx + EDI_OFF]
	push dword [edx + EBP_OFF]
	push word [edx + DS_OFF]
	push word [edx + ES_OFF]
	push word [edx + FS_OFF]
	push word [edx + GS_OFF]
	
	jmp 0x18:.pm16
bits 16
.pm16:
    mov ebx, cr0
    btr ebx, 0
    mov cr0, ebx
    jmp 0:.rm16
.rm16:
	mov [.intc + 1], al
	xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    in al, 0x70
    and al, ~0x80
    out 0x70, al
    sti
	
	pop gs
	pop fs
	pop es
	pop ds
	pop ebp
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	
.intc:
	int 0x00
	
	pushfd
	push eax
	push ebx
	push ecx
	push edx
	push esi
	push edi
	push ebp
	push ds
	push es
	push fs
	push gs
	
    cli
    in al, 0x70
    or al, 0x80
    out 0x70, al
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:.pm32
bits 32
.pm32:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    mov edi, [out_buf]
    pop word [edi + GS_OFF]
    pop word [edi + FS_OFF]
    pop word [edi + ES_OFF]
    pop word [edi + DS_OFF]
    pop dword [edi + EBP_OFF]
    pop dword [edi + EDI_OFF]
    pop dword [edi + ESI_OFF]
    pop dword [edi + EDX_OFF]
    pop dword [edi + ECX_OFF]
    pop dword [edi + EBX_OFF]
    pop dword [edi + EAX_OFF]
    pop dword [edi + EFLAGS_OFF]
    
    pop ds
    pop gs
    pop fs
    pop es
    popfd
    popad
    ret
out_buf dd 0
gdt_descriptor:
	dw 0
	dd 0
align 16, db 0x90
skip:
