#include <minlib.hpp>
using minlib::regs386;

void minlib::int386(const byte_t intr, const regs386& iregs, regs386& oregs) {
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
void* minlib::memcpy(void* dest, const void* src, word_t n) {
    byte_t* d = (byte_t*)dest;
    const byte_t* s = (const byte_t*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}
void* minlib::memcpy(void* dest, dword_t srcptr, word_t n) {
	byte_t *d = (byte_t*)dest;
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
byte_t minlib::memcmp(const void* ptr1, const void* ptr2, word_t n) {
	const byte_t* p1 = (byte_t*) ptr1;
	const byte_t* p2 = (byte_t*) ptr2;
	while(n--) 
		if(p1[n] != p2[n]) 
			return 1;
	return 0;
}
word_t minlib::esseg() {
	word_t es;
	asm (
		"mov %[seg], es\n"
		: [seg] "=r" (es)
		:
		:
	);
	return es;
}
dword_t printf(const output_type o, const char* fmt, ...) 
void minlib::logf(const byte_t* fmt, ...) {
	regs386 regs{};
	byte_t buffer[256];
	va_list args;
	va_start(args, fmt);
	minlib::snprintf(buffer, 256, fmt, args);
	va_end(args);
	for(byte_t i = 0; buffer[i]; ++i) {
		regs.ah = 0x0E;
		regs.al = buffer[i];
		regs.bx = 0;
		int386(0x10, regs, regs);
	}
}
#ifdef DEBUG
void minlib::outb(word_t port, byte_t val) {
	asm volatile (
		"out dx, al\n"
		: 
		: "a"(val), "d"(port)
	);
}

byte_t minlib::inb(word_t port) {
	byte_t ret;
	asm volatile (
		"in al, dx\n"
		: "=a"(ret) 
		: "d"(port)
	);
	return ret;
}
void minlib::comlogf(const byte_t* fmt, ...) {
	regs386 regs{};
	byte_t buffer[256];
	va_list args;
	va_start(args, fmt);
	minlib::snprintf(buffer, 256, fmt, args);
	va_end(args);
	for(int i = 0; buffer[i]; ++i) {
		while(!(inb(0x3F8 + 5) & 0x20));
		outb(0x3F8, buffer[i]);
	}
}
#endif
