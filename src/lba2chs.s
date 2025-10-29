	.intel_syntax noprefix
	.file	"lba2chs.cpp"
                                        # Start of file scope inline assembly

                                        # End of file scope inline assembly
	.text
	.globl	lba2chs                         # -- Begin function lba2chs
	.p2align	4
	.type	lba2chs,@function
lba2chs:                                # @lba2chs
	.cfi_startproc
# %bb.0:
	push	ebp
	.cfi_def_cfa_offset 8
	.cfi_offset ebp, -8
	mov	ebp, esp
	.cfi_def_cfa_register ebp
	sub	esp, 12
	mov	eax, dword ptr [ebp + 16]
	mov	eax, dword ptr [ebp + 12]
	mov	eax, dword ptr [ebp + 8]
	mov	eax, dword ptr [ebp + 12]
	mov	dword ptr [ebp - 8], eax
	mov	eax, dword ptr [ebp + 16]
	mov	dword ptr [ebp - 12], eax
	mov	eax, dword ptr [ebp + 8]
	mov	ecx, dword ptr [ebp - 8]
	movzx	ecx, word ptr [ecx]
	mov	edx, dword ptr [ebp - 8]
	movzx	edx, byte ptr [edx + 2]
	imul	ecx, edx
	mov	edx, dword ptr [ebp - 8]
	movzx	edx, byte ptr [edx + 3]
	imul	ecx, edx
	cmp	eax, ecx
	jbe	.LBB0_2
# %bb.1:
	mov	dword ptr [ebp - 4], 1
	jmp	.LBB0_3
.LBB0_2:
	mov	eax, dword ptr [ebp + 8]
	mov	ecx, dword ptr [ebp - 8]
	movzx	ecx, byte ptr [ecx + 2]
	mov	edx, dword ptr [ebp - 8]
	movzx	edx, byte ptr [edx + 3]
	imul	ecx, edx
	xor	edx, edx
	div	ecx
	mov	cx, ax
	mov	eax, dword ptr [ebp - 12]
	mov	word ptr [eax], cx
	mov	eax, dword ptr [ebp + 8]
	mov	ecx, dword ptr [ebp - 8]
	movzx	ecx, byte ptr [ecx + 3]
	xor	edx, edx
	div	ecx
	mov	ecx, dword ptr [ebp - 8]
	movzx	ecx, byte ptr [ecx + 2]
	xor	edx, edx
	div	ecx
	mov	cl, dl
	mov	eax, dword ptr [ebp - 12]
	mov	byte ptr [eax + 2], cl
	mov	eax, dword ptr [ebp + 8]
	mov	ecx, dword ptr [ebp - 8]
	movzx	ecx, byte ptr [ecx + 3]
	xor	edx, edx
	div	ecx
	add	edx, 1
	mov	cl, dl
	mov	eax, dword ptr [ebp - 12]
	mov	byte ptr [eax + 3], cl
	mov	dword ptr [ebp - 4], 0
.LBB0_3:
	mov	eax, dword ptr [ebp - 4]
	add	esp, 12
	pop	ebp
	.cfi_def_cfa esp, 4
	ret
.Lfunc_end0:
	.size	lba2chs, .Lfunc_end0-lba2chs
	.cfi_endproc
                                        # -- End function
	.ident	"clang version 20.1.8"
	.section	".note.GNU-stack","",@progbits
	.addrsig
