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
bits 16
global int386
; eax - irq number
; edx - input buffer addr
; ecx - output buffer addr
int386:
 	pushad
	pushfd
 	push es
 	push fs
 	push gs
 	push ds
 	mov [.ecx_val], ecx
 	push ax
 	mov ax, ds
 	mov [.ds_val], ax
 	pop ax
	
	mov [.intc + 1], al
	jmp 0:.reset_cache
.reset_cache:
	
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
	
	push ds
	push eax
	push ecx
	mov ax, ds
	mov ax, [cs:.ds_val]
	mov ds, ax
	mov ecx, [cs:.ecx_val]
	pop dword [ecx + ECX_OFF]
	pop dword [ecx + EAX_OFF]
	pop word [ecx + DS_OFF]
	mov [ecx + EBX_OFF], ebx
	mov [ecx + EDX_OFF], edx
	mov [ecx + ESI_OFF], esi
	mov [ecx + EDI_OFF], edi
	mov [ecx + EBP_OFF], ebp
	pushfd
	pop dword [ecx + EFLAGS_OFF]
	mov ax, es
	mov [ecx + ES_OFF], ax
	mov ax, fs
	mov [ecx + FS_OFF], ax
	mov ax, gs
	mov [ecx + GS_OFF], ax
	pop ds
	pop gs
	pop fs
	pop es
	popfd
	popad
	ret
.ecx_val dd 0
.ds_val dw 0
