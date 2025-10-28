bits 16

%define VBE_DAC_8_BIT                (1 << 0)
%define VBE_VGA_UNSUPPORTED          (1 << 1)
%define VBE_RAMDAC_BIT_BLANK         (1 << 2)
%define VBE_MODE_SUPPORTED           (1 << 0)
%define VBE_MODE_BIOS_TTY_OUT        (1 << 2)
%define VBE_MODE_COLOR_MODE          (1 << 3)
%define VBE_MODE_GRAPHICS            (1 << 4)
%define VBE_MODE_VGA_UNSUPPORTED     (1 << 5)
%define VBE_MODE_VGA_WINDOWS_MEMTYPE (1 << 6)
%define VBE_MODE_LFB                 (1 << 7)

%define FIELD_MODE 0
%define FIELD_WIDTH 4
%define FIELD_HEIGHT 8
%define FIELD_DEPTH 12

%define FIELD_VBE_CONTROL_INFO    0
%define FIELD_VBE_MODE_INFO       4
%define FIELD_VBE_MODE            8
%define FIELD_VBE_INTERFACE_SEG   10
%define FIELD_VBE_INTERFACE_OFF   12
%define FIELD_VBE_INTERFACE_LEN   14
%define FIELD_FRAMEBUFFER_ADDR    16
%define FIELD_FRAMEBUFFER_PITCH   24
%define FIELD_FRAMEBUFFER_WIDTH   28
%define FIELD_FRAMEBUFFER_HEIGHT  32
%define FIELD_FRAMEBUFFER_BPP     36
%define FIELD_FRAMEBUFFER_TYPE    37
%define FIELD_COLOR_INFO          38
global init_video

section .text
; input:
; 	ax - 16 bit address of the structure with input
; 	parameters (the last 4 fields of the multiboot header)
; 	bx - 16 bit address of the structure with output
; 	parameters (the last 10 fields of the multiboot info)
; output:
; 	CF = 1 - error
; 	CF = 0 - ok
init_video:
	push eax
	push ebx
	push ecx
	push edx
	push si
	push di
	push bp
	push fs
	push es
	pushf
; bp - address of the input parameters structure
	mov [out_addr], bx
	mov bp, ax

	mov ax, 0x4F00
	mov di, VbeInfoBlock
	int 0x10
	cmp ax, 0x004F
	jne end.failure
	mov eax, [VbeInfoBlock.VbeSignature]
	cmp eax, 'ASEV'
	jne end.failure
	mov ax, [VbeInfoBlock.VbeVersion]
	cmp ax, 0x0300
	jb end.failure
	mov eax, [VbeInfoBlock.VideoModePtr]
	mov si, ax
	shr eax, 16
	mov fs, ax
	mov ax, [bp + FIELD_MODE]
	test ax, ax
	jz graphics_mode
text_mode:
	mov ax, 0x4F03
	int 10h
	cmp ax, 0x004F
	jne .00
	mov [best_mode], bx
	jmp .lp
.00:
	mov word [best_mode], 0x3
.lp:
	mov cx, [fs:si]
	add si, 2
	jnc .0
	mov ax, fs
	add ax, 0x1000
	mov fs, ax
.0:
	cmp cx, 0xFFFF
	je .set_bm
	mov eax, [best_diff]
	test eax, eax
	jz .set_bm
	mov ax, 0x4F01
	mov di, ModeInfoBlock
	int 10h
	cmp ax, 0x004F
	jne .lp
	mov ax, [ModeInfoBlock.ModeAttributes]
	test ax, VBE_MODE_SUPPORTED
	jz .lp
	test ax, VBE_MODE_VGA_UNSUPPORTED | VBE_MODE_GRAPHICS
	jnz .lp
	mov al, [ModeInfoBlock.MemoryModel]
	; 00h - text model
	test al, al
	jnz .lp
	mov eax, [bp + FIELD_WIDTH]
	test eax, eax
	jnz .1
	movzx eax, word [ModeInfoBlock.XResolution]
.1:
	mov edx, [bp + FIELD_HEIGHT]
	test edx, edx
	jnz .2
	movzx edx, word [ModeInfoBlock.YResolution]
.2:
	mul edx
	movzx edx, word [ModeInfoBlock.XResolution]
	movzx ebx, word [ModeInfoBlock.YResolution]
	push eax
	mov eax, edx
	mul ebx
	mov edx, eax
	pop eax
	sub eax, edx
	jns .3
	neg eax
.3:
	cmp eax, [best_diff]
	jae .lp
	mov [best_diff], eax
	mov [best_mode], cx
	jmp .lp
.set_bm:
	mov cx, [best_mode]
	mov ax, 0x4F01
	mov di, ModeInfoBlock
	int 10h
	cmp ax, 0x004F
	jne end.failure

; now bp - address of the output parameters structure

	mov dx, [out_addr]
	mov bp, dx
	
	mov ax, 0x4F02
	mov bx, cx
	int 10h
	cmp ax, 0x004F
	jne end.failure
	mov eax, VbeInfoBlock
	mov [bp + FIELD_VBE_CONTROL_INFO], eax
	mov eax, ModeInfoBlock
	mov [bp + FIELD_VBE_MODE_INFO], eax
	movzx ebx, bx
	mov [bp + FIELD_VBE_MODE], ebx
	; TODO: add VBE3 PMIBlock support
	mov ax, 0x4F0A
	xor bl, bl
	int 0x10
	cmp ax, 0x004F
	jne end.failure
	mov ax, es
	mov dx, di
	mov [bp + FIELD_VBE_INTERFACE_SEG], ax
	mov [bp + FIELD_VBE_INTERFACE_OFF], dx
	mov [bp + FIELD_VBE_INTERFACE_LEN], cx
	mov byte [bp + FIELD_FRAMEBUFFER_TYPE], 2
	movzx eax, word [ModeInfoBlock.WinASegment]
	shl eax, 4
	mov [bp + FIELD_FRAMEBUFFER_ADDR], eax
	movzx eax, word [ModeInfoBlock.XResolution]
	mov [bp + FIELD_FRAMEBUFFER_WIDTH], eax
	movzx eax, word [ModeInfoBlock.YResolution]
	mov [bp + FIELD_FRAMEBUFFER_HEIGHT], eax
	mov byte [bp + FIELD_FRAMEBUFFER_BPP], 16
	movzx eax, word [ModeInfoBlock.BytesPerScanLine]
	mov [bp + FIELD_FRAMEBUFFER_PITCH], eax
	jmp end

graphics_mode:
.lp:
	mov cx, fs:[si]
	add si, 2
	jnc .1

	mov ax, fs
	add ax, 0x1000
	mov fs, ax
.1:
	cmp cx, 0xFFFF
	je .set_bm
	mov eax, [best_diff]
	test eax, eax
	jz .set_bm
	mov ax, 0x4F01
	mov di, ModeInfoBlock
	int 10h
	cmp ax, 0x004F
	jne .lp
	mov ax, [ModeInfoBlock.ModeAttributes]
	mov bx, VBE_MODE_SUPPORTED  | \
			VBE_MODE_COLOR_MODE | \
			VBE_MODE_GRAPHICS   | \
			VBE_MODE_LFB
	and ax, bx
	cmp ax, bx
	jne .lp
	mov al, [ModeInfoBlock.MemoryModel]
	cmp al, 0x7
	ja .lp
	mov eax, [bp + FIELD_WIDTH]
	test eax, eax
	jnz .2
	movzx eax, word [ModeInfoBlock.XResolution]
.2:
	mov edx, [bp + FIELD_HEIGHT]
	test edx, edx
	jnz .3
	movzx edx, word [ModeInfoBlock.YResolution]
.3:
	mov ebx, [bp + FIELD_DEPTH]
	test ebx, ebx
	jnz .5
	movzx ebx, byte [ModeInfoBlock.BitsPerPixel]
.5:
	mul edx
	mul ebx
	movzx edx, word [ModeInfoBlock.YResolution]
	movzx ebx, word [ModeInfoBlock.XResolution]
	push eax
	mov eax, edx
	mul ebx
	movzx ebx, byte [ModeInfoBlock.BitsPerPixel]
	mul ebx
	mov edx, eax
	pop eax
	sub eax, edx
	jns .6
	neg eax
.6:
	cmp eax, [best_diff]
	jae .lp
	mov [best_diff], eax
	mov [best_mode], cx
	jmp .lp
.set_bm:
	mov cx, [best_mode]
	test cx, cx
	jz end.failure
	mov ax, 0x4F01
	mov di, ModeInfoBlock
	int 10h
	cmp ax, 0x004F
	jne end.failure
	
; now bp - address of the output parameters structure

	mov dx, [out_addr]
	mov bp, dx
	
	mov ax, 0x4F02
	mov bx, cx
	or bx, (1 << 14)
	int 10h
	cmp ax, 0x004F
	jne end.failure
	mov eax, VbeInfoBlock
	mov [bp + FIELD_VBE_CONTROL_INFO], eax
	mov eax, ModeInfoBlock
	mov [bp + FIELD_VBE_MODE_INFO], eax
	movzx ebx, bx
	mov [bp + FIELD_VBE_MODE], ebx
	; TODO: add VBE3 PMIBlock support
	mov ax, 0x4F0A
	xor bl, bl
	int 0x10
	cmp ax, 0x004F
	jne end.failure
	mov ax, es
	mov dx, di
	mov [bp + FIELD_VBE_INTERFACE_SEG], ax
	mov [bp + FIELD_VBE_INTERFACE_OFF], dx
	mov [bp + FIELD_VBE_INTERFACE_LEN], cx
	mov eax, [ModeInfoBlock.PhysBasePtr]
	mov [bp + FIELD_FRAMEBUFFER_ADDR], eax
	movzx eax, word [ModeInfoBlock.LinBytesPerScanLine]
	mov [bp + FIELD_FRAMEBUFFER_PITCH], eax
	movzx eax, word [ModeInfoBlock.XResolution]
	mov [bp + FIELD_FRAMEBUFFER_WIDTH], eax
	movzx eax, word [ModeInfoBlock.YResolution]
	mov [bp + FIELD_FRAMEBUFFER_HEIGHT], eax
	mov al, [ModeInfoBlock.BitsPerPixel]
	mov [bp + FIELD_FRAMEBUFFER_BPP], al
	
	mov al, [ModeInfoBlock.MemoryModel]
	cmp al, 0x6
	jb .nondirect
	mov al, [ModeInfoBlock.LinRedFieldPosition]
	mov [bp + FIELD_COLOR_INFO], al
	mov al, [ModeInfoBlock.LinRedMaskSize]
	mov [bp + FIELD_COLOR_INFO + 1], al
	mov al, [ModeInfoBlock.LinGreenFieldPosition]
	mov [bp + FIELD_COLOR_INFO + 2], al
	mov al, [ModeInfoBlock.LinGreenMaskSize]
	mov [bp + FIELD_COLOR_INFO + 3], al
	mov al, [ModeInfoBlock.LinBlueFieldPosition]
	mov [bp + FIELD_COLOR_INFO + 4], al
	mov al, [ModeInfoBlock.LinBlueMaskSize]
	mov [bp + FIELD_COLOR_INFO + 5], al
	mov byte [bp + FIELD_FRAMEBUFFER_TYPE], 1
	jmp end
.nondirect:
	mov ax, 1
	mov cl, [ModeInfoBlock.BitsPerPixel]
	shl ax, cl
	mov cx, ax
	mov dx, 0
	mov ax, 0x4F09
	mov bl, 0x1
	mov di, framebuffer_palette
	int 10h
	cmp ax, 0x004F
	jne end.failure

	mov si, framebuffer_palette
	mov di, framebuffer_palette
	push cx

;	********	example		********
; before: blue | green | red  | align | blue  | green | red    | align
; after:  red  | green | blue | red   | green | blue  | unused | unused
.sanitize_palette:
	lodsb
	mov bl, al
	lodsb
	mov bh, al
	lodsb
	inc si
	stosb
	mov al, bh
	stosb
	mov al, bl
	stosb
	loop .sanitize_palette
	pop cx
	mov dword [bp + FIELD_COLOR_INFO], framebuffer_palette
	push eax
	mov eax, ecx
	mov ebx, 3
	mul ebx
	mov [bp + FIELD_COLOR_INFO + 4], ax
	pop eax
	mov byte [bp + FIELD_FRAMEBUFFER_TYPE], 0

end:
	popf
	pop es
	pop fs
	pop bp
	pop di
	pop si
	pop edx
	pop ecx
	pop ebx
	pop eax
	clc
	ret
.failure:
	popf
	pop es
	pop fs
	pop bp
	pop di
	pop si
	pop edx
	pop ecx
	pop ebx
	pop eax
	stc
	ret
section .data
best_mode dw 0
best_diff dd 0xFFFFFFFF
out_addr  dw 0

framebuffer_palette:
	resd 256
PaletteEntry:
	resd 1
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
