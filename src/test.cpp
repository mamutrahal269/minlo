#include <minlib.hpp>
#include <vbe.hpp>

u8 state[4096];
void __attribute__((section(".main"))) __attribute__((noreturn)) main() {
	printf(outt::tty, "RET CODE: %hhu", !VBEmode_setup(mode_type::text, ~0, ~0, 0).vbe_mode_info);
	for(;;);
}
