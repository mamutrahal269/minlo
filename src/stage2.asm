org 0x7E00
bits 16

%ifndef DYNAMIC_CONFIG
%include "config.inc"
%endif

%define DEST_ADDR 0x100000
%define STACKPTR 0x7C00

%define MBH_PAGE_ALIGN               (1 << 0)
%define MBH_MEM_INFO                 (1 << 1)
%define MBH_VIDEO_MODE               (1 << 2)
%define MBH_ADDR_FIELDS              (1 << 16)
%define MBH_MASK                     (MBH_MEM_INFO | MBH_VIDEO_MODE)

%define MBI_MEM                      (1 << 0)
%define MBI_BOOT_DEVICE              (1 << 1)
%define MBI_CMDLINE                  (1 << 2)
%define MBI_MODS                     (1 << 3)
%define MBI_AOUT_SYMS                (1 << 4)
%define MBI_ELF_SHDR                 (1 << 5)
%define MBI_MMAP                     (1 << 6)
%define MBI_DRIVES                   (1 << 7)
%define MBI_CONFIG_TABLE             (1 << 8)
%define MBI_BOOT_LOADER              (1 << 9)
%define MBI_APM_TABLE                (1 << 10)
%define MBI_VBE                      (1 << 11)
%define MBI_FRAMEBUFFER              (1 << 12)

%define VBE_DAC_8_BIT                (1 << 0)
%define VBE_VGA_UNSUPPORTED          (1 << 1)
%define VBE_RAMDAC_BIT_BLANK         (1 << 2)
%define VBE_MODE_SUPPORTED           (1 << 0)
%define VBE_MODE_BIOS_TTY_OUT        (1 << 2)
%define VBE_MODE_COLOR_MODE          (1 << 3)
%define VBE_MODE_GRAPHIC             (1 << 4)
%define VBE_MODE_VGA_UNSUPPORTED     (1 << 5)
%define VBE_MODE_VGA_WINDOWS_MEMTYPE (1 << 6)
%define VBE_MODE_LFB                 (1 << 7)

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

%macro SETFLAG 1
    mov eax, [multiboot_info.flags]
    or eax, %1
    mov [multiboot_info.flags], eax
%endmacro
%macro TESTFLAG 1
	mov eax, [multiboot.flags]
	test eax, %1
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
    
init_mb_header:
	mov ax 16
	mov [dap.sectors], ax
	mov dl, BOOT_DRIVE
	mov ah, 42h
	mov si, dap
	int 0x13
	jc disk_error
	
	mov cx, 2048
	mov bx, BUFFER - 4
.lp:
	sub cx, 1
	jz invalid_mb
	add bx, 4
	mov eax, [bx]
	cmp eax, 0x1BADB002
	jne .lp
	add eax, [bx + 4]
	add eax, [bx + 8]
	jnz .lp
	
	mov cx, 48
	mov si, bx
	mov di, multiboot
	rep movsb

	TESTFLAG ~MBH_MASK
    jnz invalid_mb
    
    mov eax, 0xFFFF0000
	mov al, BOOT_DRIVE
    mov [multiboot_info.boot_device], eax
    SETFLAG MBI_BOOT_DEVICE
    
meminfo:
    TESTFLAG MBH_MEM_INFO
    jz do_e820
    xor eax, eax
    int 0x12
    jc memdet_failed
    mov [multiboot_info.mem_lower], eax
    xor cx, cx
    xor dx, dx
    mov ax, 0xE801
    int 15h
    jc memdet_failed
    cmp ah, 0x86
    je memdet_failed
    cmp ah, 0x80
    je memdet_failed
    jcxz .useaxbx
    mov ax, cx
    mov bx, dx
.useaxbx:
    movzx eax, ax
    movzx ebx, bx
    shl ebx, 6
    add eax, ebx
    mov [multiboot_info.mem_upper], eax
    SETFLAG MBI_MEM
    
do_e820:
    mov edi, 0x500
    mov [multiboot_info.mmap_addr], edi
	xor ebx, ebx
	xor bp, bp
	mov edx, 0x0534D4150
	mov eax, 0xE820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15
	jc memdet_failed
	mov edx, 0x0534D4150
	cmp eax, edx
	jne memdet_failed
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
	imul eax, bp, 24
	mov [multiboot_info.mmap_length], eax
	SETFLAG MBI_MMAP
	
	TESTFLAG MBH_VIDEO_MODE
	jnz video_init_end
	mov eax, [multiboot.mode_type]
	cmp eax, 1
	ja video_failed
	mov ax, 0x4F00
	mov di, svga_info
	int 0x10
	cmp ax, 0x004F
	jne video_failed
	mov eax, [VbeInfoBlock.VbeSignature]
	cmp eax, 0x41534556
	mov eax, [VbeInfoBlock.VbeVersion]
	cmp eax, 0x0300
	jb video_failed
	mov ax, 0x4F03
	int 10h
	cmp ax, 0x004F
	jne video_failed
	mov [default_mode], bx
	mov eax, [VbeInfoBlock.VideoModePtr]
	mov si, ax
	shr eax, 16
	mov fs, ax
	mov ax, [multiboot.mode_type]
	test ax, ax
	jnz graphic_mode

text_mode:
	mov ax, [default_mode]
	mov [best_mode], ax
.lp:
	mov cx, fs:[si]
	add si, 2
	jnc @f

	mov ax, fs
	add ax, 0x1000
	mov fs, ax
@@:
	cmp cx, 0xFFFF
	je .set_bm
	mov eax, [best_diff]
	test eax, eax
	jz .set_bm
	mov ax, 0x4F01
	mov di, ModelInfoBlock
	int 10h
	cmp ax, 0x004F
	jne .lp
	mov ax, [ModelInfoBlock.ModeAttributes]
	test ax, VBE_MODE_SUPPORTED
	jz .lp
	mov ax, [ModelInfoBlock.ModeAttributes]
	test ax, VBE_MODE_VGA_UNSUPPORTED | VBE_MODE_GRAPHIC
	jnz .lp
	mov al, [ModelInfoBlock.MemoryModel]
	; 00h - text model
	test al, al
	jnz .lp
	mov eax, [multiboot.width]
	test eax, eax
	jnz @f
	mov eax, [ModelInfoBlock.XResolution]
@@:
	mov edx, [multiboot.height]
	test edx, edx
	jnz @f
	mov edx, [ModelInfoBlock.YResolution]
@@:
	imul eax, edx
	mov edx, [ModelInfoBlock.XResolution]
	imul edx, [ModelInfoBlock.YResolution]
	sub eax, edx
	jns @f
	neg eax
@@:
	cmp eax, [best_diff]
	jae .lp
	mov [best_diff], eax
	mov [best_mode], cx
	jmp .lp
.set_bm:
	mov cx, [best_mode]
	mov ax, 0x4F01
	mov di, ModelInfoBlock
	int 10h
	cmp ax, 0x004F
	jne video_failed
	mov ax, 0x4F02
	mov bx, cx
	int 10h
	cmp ax, 0x004F
	jne video_failed
	mov eax, VbeInfoBlock
	mov [multiboot_info.vbe_control_info], eax
	mov eax, ModelInfoBlock
	mov [multiboot_info.vbe_mode_info], eax
	movzx ebx, bx
	mov [multiboot_info.vbe_mode], ebx
	; TODO: add VBE3 PMIBlock support
	mov ax, 0x4F0A
	xor bl, bl
	int 0x10
	cmp ax, 0x004F
	jne video_failed
	mov ax, es
	mov dx, di
	mov [multiboot_info.vbe_interface_seg], ax
	mov [multiboot_info.vbe_interface_off], dx
	mov [multiboot_info.vbe_interface_length], cx
	mov [multiboot_info.framebuffer_type], 2
	mov eax, [ModelInfoBlock.WinASegment]
	shl eax, 4
	mov [multiboot_info.framebuffer_addr], eax
	mov eax, [ModelInfoBlock.XResolution]
	mov [multiboot_info.framebuffer_width], eax
	mov eax, [ModelInfoBlock.YResolution]
	mov [multiboot_info.framebuffer_height], eax
	mov [multiboot_info.framebuffer_bpp], 16
	mov eax, [ModelInfoBlock.BytesPerScanLine]
	mov [multiboot_info.framebuffer_pitch], eax
	SETFLAG (MBI_VBE | MBI_FRAMEBUFFER)
	jmp video_init_end
graphics_mode:
	mov cx, fs:[si]
	add si, 2
	jnc @f

	mov ax, fs
	add ax, 0x1000
	mov fs, ax
@@:
	cmp cx, 0xFFFF
	je .set_bm
	mov eax, [best_diff]
	test eax, eax
	jz set_bm
	mov ax, 0x4F01
	mov di, ModelInfoBlock
	int 10h
	cmp ax, 0x004F
	jne video_failed
	cmp ax, 0x004F
	jne .lp
	mov ax, [ModelInfoBlock.ModeAttributes]
	mov bx, VBE_MODE_SUPPORTED  | \
			VBE_MODE_COLOR_MODE | \
			VBE_MODE_GRAPHIC    | \
			VBE_MODE_LFB
	and ax, bx
	cmp ax, bx
	jne .lp
	mov eax, [multiboot.width]
	test eax, eax
	jnz @f
	mov eax, [ModelInfoBlock.XResolution]
@@:
	mov edx, [multiboot.height]
	test edx, edx
	jnz @f
	mov edx, [ModelInfoBlock.YResolution]
@@:
	mov ebx, [multiboot.depth]
	test ebx, ebx
	jnz @f
	mov ebx, [ModelInfoBlock.BitsPerPixel]
@@:
	imul eax, edx
	imul eax, ebx
	mov edx, [ModelInfoBlock.YResolution]
	imul edx, [ModelInfoBlock.XResolution]
	imul edx, [ModelInfoBlock.BitsPerPixel]
	sub eax, edx
	js @f
	neg eax
@@:
	cmp [best_diff], eax
	jae .lp
	mov [best_diff], eax
	mov [best_mode], cx
	jmp .lp
.set_bm:
    mov cx, [best_mode]
	test cx, cx
	jz video_failed
	mov ax, 0x4F01
    mov di, ModelInfoBlock
    int 10h
    cmp ax, 0x004F
    jne video_failed
    mov ax, 0x4F02
    mov bx, cx
	or bx, (1 << 14)
    int 10h
    cmp ax, 0x004F
    jne video_failed
    mov eax, VbeInfoBlock
    mov [multiboot_info.vbe_control_info], eax
    mov eax, ModelInfoBlock
    mov [multiboot_info.vbe_mode_info], eax
    movzx ebx, bx
    mov [multiboot_info.vbe_mode], ebx
    ; TODO: add VBE3 PMIBlock support
    mov ax, 0x4F0A
    xor bl, bl
    int 0x10
    cmp ax, 0x004F
    jne video_failed
    mov ax, es
    mov dx, di
    mov [multiboot_info.vbe_interface_seg], ax
    mov [multiboot_info.vbe_interface_off], dx
    mov [multiboot_info.vbe_interface_length], cx
    
    mov eax, [ModelInfoBlock.PhysBasePtr]
    mov [multiboot_info.framebuffer_addr], eax
    mov eax, [ModelInfoBlock.XResolution]
    mov [multiboot_info.framebuffer_width], eax
    mov eax, [ModelInfoBlock.YResolution]
    mov [multiboot_info.framebuffer_height], eax
    mov eax, [ModeInfoBlock.BitsPerPixel]
    mov [multiboot_info.framebuffer_bpp], eax 
    mov eax, [ModelInfoBlock.BytesPerScanLine]
    mov [multiboot_info.framebuffer_pitch], eax
    
    SETFLAG (MBI_VBE | MBI_FRAMEBUFFER)
video_init_end:

load_kernel:
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
kload:
    mov cx, SECTORS_PER_LOAD
    mov [dap.sectors], cx
    mov ecx, TOTAL_SECTORS
    mov edi, DEST_ADDR
.lp:
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

    sub ecx, SECTORS_PER_LOAD
    jz done

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
    cmp ecx, SECTORS_PER_LOAD
    jb last
    jmp .lp
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
    
kload_done:

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
memdet_failed:
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
multiboot:
	magic         dd 0
	flags         dd 0
	checksum      dd 0
	header_addr   dd 0
	load_addr     dd 0
	load_end_addr dd 0
	bss_end_addr  dd 0
	entry_addr    dd 0
	mode_type     dd 0
	width         dd 0
	height        dd 0
	depth         dd 0
dap:
    .size           db 0x10
    .res            db 0
    .sectors        dw 0
    .buffer_offset  dw BUFFER
    .buffer_segment dw 0
    .lba_low        dd START_SECTOR
    .lba_high       dd 0
multiboot_info:
    .flags               resd 1
    .mem_lower           resd 1
    .mem_upper           resd 1
    .boot_device         resd 1
    .cmdline             resd 1
    .mods_count          resd 1
    .mods_addr           resd 1
    .syms                resd 3
    .mmap_length         resd 1
    .mmap_addr           resd 1
    .drives_length       resd 1
    .drives_addr         resd 1
    .config_table        resd 1
    .boot_loader_name    resd 1
    .apm_table           resd 1
    .vbe_control_info    resd 1
    .vbe_mode_info       resd 1
    .vbe_mode            resw 1
    .vbe_interface_seg   resw 1
    .vbe_interface_off   resw 1
    .vbe_interface_len   resw 1
    .framebuffer_addr    resq 1
    .framebuffer_pitch   resd 1
    .framebuffer_width   resd 1
    .framebuffer_height  resd 1
    .framebuffer_bpp     resb 1
    .framebuffer_type    resb 1
    .color_info          resb 6
VbeInfoBlock:
    .VbeSignature      db 'VBE2'
    .VbeVersion        resw 1
    .OemStringPtr      resd 1
    .Capabilities      resb 4
    .VideoModePtr      resd 1
    .TotalMemory       resw 1
    .OemSoftwareRev    resw 1
    .OemVendorNamePtr  resd 1
    .OemProductNamePtr resd 1
    .OemProductRevPtr  resd 1
    .Reserved          resb 222
    .OemData           resb 256
    
ModeInfoBlock:
    .ModeAttributes        resw 1
    .WinAAttributes        resb 1
    .WinBAttributes        resb 1
    .WinGranularity        resw 1
    .WinSize               resw 1
    .WinASegment           resw 1
    .WinBSegment           resw 1
    .WinFuncPtr            resd 1
    .BytesPerScanLine      resw 1
    .XResolution           resw 1
    .YResolution           resw 1
    .XCharSize             resb 1
    .YCharSize             resb 1
    .NumberOfPlanes        resb 1
    .BitsPerPixel          resb 1
    .NumberOfBanks         resb 1
    .MemoryModel           resb 1
    .BankSize              resb 1
    .NumberOfImagePages    resb 1
                           resb 1
    .RedMaskSize           resb 1
    .RedFieldPosition      resb 1
    .GreenMaskSize         resb 1
    .GreenFieldPosition    resb 1
    .BlueMaskSize          resb 1
    .BlueFieldPosition     resb 1
    .RsvdMaskSize          resb 1
    .RsvdFieldPosition     resb 1
    .DirectColorModeInfo   resb 1
    .PhysBasePtr           resd 1
                           resd 1
                           resw 1
    .LinBytesPerScanLine   resw 1
    .BnkNumberOfImagePages resb 1
    .LinNumberOfImagePages resb 1
    .LinRedMaskSize        resb 1
    .LinRedFieldPosition   resb 1
    .LinGreenMaskSize      resb 1
    .LinGreenFieldPosition resb 1
    .LinBlueMaskSize       resb 1
    .LinBlueFieldPosition  resb 1
    .LinRsvdMaskSize       resb 1
    .LinRsvdFieldPosition  resb 1
    .MaxPixelClock         resd 1
                           resb 189
	
disk_err_msg   db 'Disk read error. Please, reboot the computer', 0
e820failed_msg db 'Memory detection failed. Please, reboot the computer', 0
default_mode   dw 0
best_mode      dw 0
best_diff      dd 0xFFFFFFFF
%ifdef DEBUG
dbg_msg_stage2 db 'Stage 2 loaded', 0x0D, 0x0A, 0
dbg_msg_memdec db 'Memory detection successful', 0x0D, 0x0A, 0
dbg_msg_pm     db 'Entering protected mode...', 0x0D, 0x0A, 0
dbg_msg_rm     db 'CPU in real mode', 0x0D, 0x0A, 0
%endif
BUFFER:
times 4096 - ($-$$) db 0
