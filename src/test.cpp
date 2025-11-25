#include <minlib.hpp>

void 
__attribute__((section(".main")))
__attribute__((noreturn))
main() {
	regs386 regs{};
	regs.ax = 0x4F0A;
	int386(0x10, regs, regs);
	printf(outt::tty, "123456789\r\n");
	printf(outt::tty, " ES: %#X \r\n DI: %#X \r\n CX: %#X",
	regs.es, regs.di, regs.cx);
	for(;;);
}
