#include <minlib.hpp>
#include <memory.hpp>

byte_t memlu(dword_t& meml, dword_t& memu) {
	regs386 regs{};
	int386(0x12, regs, regs);
	if(regs.eflags & EFLAGS_CF) return 1;
	meml = regs.ax;

	regs.cx = regs.dx = 0;
	regs.ax = 0xE801;
	int386(0x15, regs, regs);
	if(regs.eflags & EFLAGS_CF) return 1;
	if(regs.ah == 0x86 || regs.ah == 0x80) return 1;

	dword_t base = regs.cx ? regs.cx : regs.ax;
	dword_t mul  = regs.cx ? regs.dx : regs.bx;
	dword_t add;

	if (__builtin_mul_overflow(mul, 64u, &add) ||
		__builtin_add_overflow(base, add, &memu)) memu = ~0;
	else memu = base + add;
	return 0;
}
word_t e820map(e820_ent* ents, word_t max_ents = 0xFFFF) {
	regs386 regs{};
	word_t i = 0;
	do {
		regs.eax = 0xE820;
		regs.edx = 0x534D4150;
		regs.ecx = 24;
		regs.es = esseg();
		regs.di = reinterpret_cast<word_t>(&ents[i]);
		int386(0x15, regs, regs);
		if (regs.eax != 0x534D4150 ||
			regs.eflags & EFLAGS_CF) return i;
		if((regs.cl > 20 && ents[i].ACPI & 1) || 
			!ents[i].length) continue;
		++i;
	} while(i < max_ents && regs.ebx);
	return i;
}
