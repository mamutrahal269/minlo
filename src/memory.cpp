#include <minlib.hpp>
#include <memory.hpp>

u8 memlu(u32& meml, u32& memu) {
	regs386 regs{};
	int386(0x12, regs, regs);
	if(regs.eflags & eflags::CF) return 1;
	meml = regs.ax;

	regs.cx = regs.dx = 0;
	regs.ax = 0xE801;
	int386(0x15, regs, regs);
	if(regs.eflags & eflags::CF) return 1;
	if(regs.ah == 0x86 || regs.ah == 0x80) return 1;

	u32 base = regs.cx ? regs.cx : regs.ax;
	u32 mul  = regs.cx ? regs.dx : regs.bx;
	u32 add;

	if (__builtin_mul_overflow(mul, 64u, &add) ||
		__builtin_add_overflow(base, add, &memu)) memu = ~0;
	else memu = base + add;
	return 0;
}
u8 e820call(e820_ent& ent, u32& n) {
    regs386 regs{};
    regs.ebx = n;
    regs.eax = 0xE820;
    regs.edx = 0x534D4150;
    regs.ecx = 24;
    regs.es = esseg();
    regs.di = reinterpret_cast<u32>(&ent);
    int386(0x15, regs, regs);
    if (regs.eax != 0x534D4150 || regs.eflags & eflags::CF) 
        return 2;
    n = regs.ebx;
    if ((regs.ecx > 20 && ent.ACPI & 1) || !ent.length) 
        return 1;
    return 0;
}
