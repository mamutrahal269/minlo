#include <minlib.hpp>

void int386(const u8 intr, const regs386& iregs, regs386& oregs) {
	asm volatile (
 		"pushad\n"
 		"push ds\n"
 		"push es\n"
 		"push fs\n"
 		"push gs\n"
 		"pushfd\n"

		"mov al, %[intrq]\n"
		/* IRQ number modification */
		"mov [.Lintc%= + 1], al\n"
		"jmp .Lx%=\n"
		".Lx%=:\n"
		
 		"mov ax, %[ids]\n"
 		"mov ds, ax\n"
 		"mov ax, %[ies]\n"
 		"mov es, ax\n"
 		"mov ax, %[ifs]\n"
 		"mov fs, ax\n"
 		"mov ax, %[igs]\n"
 		"mov gs, ax\n"
 		"mov eax, %[ieax]\n"
 		"mov ebx, %[iebx]\n"
 		"mov ecx, %[iecx]\n"
 		"mov edx, %[iedx]\n"
 		"mov esi, %[iesi]\n"
 		"mov edi, %[iedi]\n"
 		"mov ebp, %[iebp]\n"
 		
 		".Lintc%=:\n"
 		"int 0x00\n"

 		"mov %[oeax], eax\n"
 		"mov %[oebx], ebx\n"
 		"mov %[oecx], ecx\n"
 		"mov %[oedx], edx\n"
 		"mov %[oesi], esi\n"
 		"mov %[oedi], edi\n"
 		"mov %[oebp], ebp\n"
 		"mov ax, ds\n"
 		"mov %[ods], ax\n"
 		"mov ax, es\n"
 		"mov %[oes], ax\n"
 		"mov ax, fs\n"
 		"mov %[ofs], ax\n"
 		"mov ax, gs\n"
 		"mov %[ogs], ax\n"
 		"pushfd\n"
 		"pop eax\n"
 		"mov %[oeflags], eax\n"

 		"popfd\n"
 		"pop gs\n"
 		"pop fs\n"
 		"pop es\n"
 		"pop ds\n"
 		"popad\n"
 		: [oeax] "=m" (oregs.eax),
 		  [oebx] "=m" (oregs.ebx),
 		  [oecx] "=m" (oregs.ecx),
 		  [oedx] "=m" (oregs.edx),
 		  [oesi] "=m" (oregs.esi),
 		  [oedi] "=m" (oregs.edi),
 		  [oebp] "=m" (oregs.ebp),
 		  [ods] "=m" (oregs.ds),
 		  [oes] "=m" (oregs.es),
 		  [ofs] "=m" (oregs.fs),
 		  [ogs] "=m" (oregs.gs),
 		  [oeflags] "=m" (oregs.eflags)
 		  
 		: [ieax] "m" (iregs.eax),
 		  [iebx] "m" (iregs.ebx),
 		  [iecx] "m" (iregs.ecx),
 		  [iedx] "m" (iregs.edx),
 		  [iesi] "m" (iregs.esi),
 		  [iedi] "m" (iregs.edi),
 		  [iebp] "m" (iregs.ebp),
 		  [ids] "m" (iregs.ds),
 		  [ies] "m" (iregs.es),
 		  [ifs] "m" (iregs.fs),
 		  [igs] "m" (iregs.gs),
 		  [intrq] "m" (intr)
 		: "memory"
 	);
}
void* memcpy(void* dest, const void* src, size_t n) {
    u8* d = (u8*)dest;
    const u8* s = (const u8*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}
void* memcpy(void* dst, u32 srcptr, size_t n); {
	u8 *d = (u8*)dest;
    for(; n; --n, ++d, ++srcptr) {
        asm volatile (
			"push ds\n"
			"mov ds, ax\n"
			"movsb"
			"pop ds\n"
			:
			: "a" (srcptr >> 4), "S" (srcptr & 0xF), "D" (d)
			: "memory", "S", "D"
        );
    }
    return dest;
}
u8 memcmp(const void* ptr1, const void* ptr2, size_t n) {
	const u8* p1 = (u8*) ptr1;
	const u8* p2 = (u8*) ptr2;
	while(n--) 
		if(p1[n] != p2[n]) 
			return 1;
	return 0;
}
u16 esseg() {
	u16 es;
	asm (
		"mov %[seg], es\n"
		: [seg] "=r" (es)
		:
		:
	);
	return es;
}
void outb(u16 port, u8 val) {
	asm volatile (
		"out dx, al\n"
		: 
		: "a"(val), "d"(port)
	);
}

u8 inb(u16 port) {
	u8 ret;
	asm volatile (
		"in al, dx\n"
		: "=a"(ret) 
		: "d"(port)
	);
	return ret;
}
