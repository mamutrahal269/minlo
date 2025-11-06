#include <minlib.hpp>
#include <cstdarg>

void int386(const byte_t intr, const regs386& iregs, regs386& oregs) {
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
byte_t* strncpy(byte_t *restrict dst, const byte_t *restrict src, word_t n) {
    byte_t* ret = dst;
    while (n) {
        if ((*dst = *src) != '\0')
            src++;
        dst++;
        n--;
    }
    return ret;
}
byte_t strncmp(const byte_t *s1, const byte_t *s2, word_t n) {
    while (n--) {
        byte_t c1 = *s1++;
        byte_t c2 = *s2++;
        if (c1 != c2)
            return c1 - c2;
        if (c1 == '\0')
            return 0;
    }
    return 0;
}
word_t esseg() {
	word_t es;
	asm (
		"mov %[seg], es\n"
		: [seg] "=r" (es)
		:
		:
	);
	return es;
}
void memcpyfar(byte_t* buffer, dword_t physAddr, word_t n) {
    for(word_t i = 0; i < n; ++i, ++physAddr) {
        asm volatile (
            "push es\n"
            "mov es, %[seg]\n"
            "mov al, byte [es:si]\n"
            "mov %[dst], al\n"
            "pop es\n"
            : [dst] "=m" (buffer[i])
            : [seg] "r" ((word_t)physAddr >> 4), "S" ((word_t)physAddr & 0xF)
            : "memory", "al"
        );
    }
}
int abs(const int n) {
	return n < 0 ? -n : n;
}
void logf(const byte_t* fmt, ...) {
	regs386 regs{};
	byte_t buffer[256];
	va_list args;
	va_start(args, fmt);
	snprintf(buffer, 256, fmt, args);
	va_end(args);
	for(byte_t i = 0; buffer[i]; ++i) {
		regs.ah = 0x0E;
		regs.al = buffer[i];
		regs.bx = 0;
		int386(0x10, regs, regs);
	}
}
#ifdef DEBUG
void outb(word_t port, byte_t val) {
	asm volatile (
		"out dx, al\n"
		: 
		: "a"(val), "d"(port)
	);
}

byte_t inb(word_t port) {
	byte_t ret;
	asm volatile (
		"in al, dx\n"
		: "=a"(ret) 
		: "d"(port)
	);
	return ret;
}
void comlogf(const byte_t* fmt, ...) {
	regs386 regs{};
	byte_t buffer[256];
	va_list args;
	va_start(args, fmt);
	snprintf(buffer, 256, fmt, args);
	va_end(args);
	for(int i = 0; buffer[i]; ++i) {
		while(!(inb(0x3F8 + 5) & 0x20));
		outb(0x3F8, buffer[i]);
	}
}
#endif
