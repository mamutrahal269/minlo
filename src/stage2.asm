org 0x7E00
bits 16

%ifndef DYNAMIC_CONFIG
%include "config.inc"
%endif

%define DEST_ADDR 0x100000
%define BOOT_DRIVE byte [0x7C00 + 445]
%define STACKPTR 0x7C00
%define MAGIC_NUM 0x5D0

%macro SWITCH2PM 0
%ifdef DEBUG
	mov si, dbg_msg_pm
	call dbgprint
%endif
    cli
    in al, 0x70
    or al, 0x80
    out 0x70, al
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
    mov esp, STACKPTR

%endmacro

%ifdef DEBUG
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
%endif
   
%ifdef DEBUG
	mov si, dbg_msg_rm
	call dbgprint
    mov si, dbg_msg_stage2
    call dbgprint
%endif

    cld
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, STACKPTR
    sti
    mov al, BOOT_DRIVE
    mov [params.boot_drive], al

do_e820:
    mov di, 0x500
    mov [params.mmap_addr], di
	xor ebx, ebx
	xor bp, bp
	mov edx, 0x0534D4150
	mov eax, 0xE820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15
	jc e820failed
	mov edx, 0x0534D4150
	cmp eax, edx
	jne e820failed
	test ebx, ebx
	je e820failed
	jmp .jmpin
.e820lp:
	mov eax, 0xe820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15
	jc .e820ok
	mov edx, 0x0534D4150
.jmpin:
	jcxz .skipent
	cmp cl, 20
	jbe short .notext
	test byte [es:di + 20], 1
	je short .skipent
.notext:
	mov ecx, [es:di + 8]
	or ecx, [es:di + 12]
	jz .skipent
	inc bp
	add di, 24
.skipent:
	test ebx, ebx
	jne .e820lp
.e820ok:
	mov [params.mmap_ents], bp
%ifdef DEBUG
	mov si, dbg_msg_memdec
	call dbgprint
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

    mov esi, BUFFER
    mov edi, DEST_ADDR
    mov ecx, TOTAL_SECTORS * 512
    rep movsb

    mov eax, MAGIC_NUM
    mov ebx, params

    jmp DEST_ADDR
%else
    mov cx, SECTORS_PER_LOAD
    mov [dap.sectors], cx
    mov ecx, TOTAL_SECTORS
    mov edi, DEST_ADDR
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
    mov esi, BUFFER
    mov ecx, SECTORS_PER_LOAD * 512
    rep movsb
    mov ecx, eax

;	I noticed that simply clearing the PE bit isn't enough; the processor 
;	still uses 32-bit instructions. 
;	Therefore, I have to load the descriptor 
;	selector with the D bit cleared.
;	This might be a bug in my emulator.
    jmp 0x18:.pm16

bits 16
.pm16:
    mov eax, cr0
    btr eax, 0
    mov cr0, eax
    jmp 0:.rm
.rm:
    xor eax,eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, STACKPTR
    in al, 0x70
    and al, ~0x80
    out 0x70, al
    sti
%ifdef DEBUG
	mov si, dbg_msg_rm
	call dbgprint
%endif
    sub ecx, SECTORS_PER_LOAD
    jz done
    cmp ecx, SECTORS_PER_LOAD
    jb last
    jmp 0:loadloop
last:
    mov ecx, TOTAL_SECTORS % SECTORS_PER_LOAD
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
    mov esi, BUFFER
    rep movsb

    mov eax, MAGIC_NUM
    mov ebx, params

    jmp 0x100000
done:
    SWITCH2PM

    mov eax, MAGIC_NUM
    mov ebx, params

    jmp 0x100000
%endif
bits 16
%ifdef DEBUG
dbgprint:
.loop:
    lodsb
    test al, al
    jz .done
    call dbgputc
    jmp .loop
.done:
    cli
    
dbgputc:
    push dx
    push ax
    mov dx, 0x3FD
.wait:
    in al, dx
    test al, 0x20
    jz .wait
    pop ax
    mov dx, 0x3F8
    out dx, al
    pop dx
    ret
%endif
    
disk_error:
    mov esi, disk_err_msg
    call print
    jmp $
e820failed:
    mov esi, e820failed_msg
    call print
    jmp $
print:
    mov ah, 0x0E
    lodsb
    test al, al
    jz .done
    mov bl, 0x00
    int 10h
    jmp print
.done:
    ret

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

dap:
    .size           db 0x10
    .res            db 0
    .sectors        dw 0
    .buffer_offset  dw BUFFER
    .buffer_segment dw 0
    .lba_low        dd START_SECTOR
    .lba_high       dd 0

params:
    .mmap_addr dw 0
    .mmap_ents dw 0
    .boot_drive db 0

disk_err_msg db 'Disk read error. Please, reboot the computer', 0
e820failed_msg db 'Memory detection failed. Please, reboot the computer', 0

%ifdef DEBUG
dbg_msg_stage2 db 'Stage 2 loaded', 0x0D, 0x0A, 0
dbg_msg_memdec db 'Memory detection successful', 0x0D, 0x0A, 0
dbg_msg_pm db 'Entering protected mode...', 0x0D, 0x0A, 0
dbg_msg_rm db 'CPU in real mode', 0x0D, 0x0A, 0
%endif
BUFFER:
times 4096 - ($-$$) db 0
