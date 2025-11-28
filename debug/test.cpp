#include <minlib.hpp>
#include <vbe.hpp>
[[gnu::section(".main"), noreturn, gnu::used]] void main() {
	const auto vmode = VBEmode_setup(mode_type::graphics, 1, 1, 1);
	printf(outt::e9, "vbe_mode_info: %p\r\n", vmode.vbe_mode_info);
	printf(outt::e9, "vbe_mode: %u\r\n", vmode.vbe_mode);

	printf(outt::e9, "vbe_interface_seg: %u\r\n", vmode.vbe_interface_seg);
	printf(outt::e9, "vbe_interface_off: %u\r\n", vmode.vbe_interface_off);
	printf(outt::e9, "vbe_interface_len: %u\r\n", vmode.vbe_interface_len);

	printf(outt::e9, "framebuffer_addr: 0x%llx\r\n", (unsigned long long)vmode.framebuffer_addr);
	printf(outt::e9, "framebuffer_pitch: %u\r\n", vmode.framebuffer_pitch);
	printf(outt::e9, "framebuffer_width: %u\r\n", vmode.framebuffer_width);
	printf(outt::e9, "framebuffer_height: %u\r\n", vmode.framebuffer_height);

	printf(outt::e9, "framebuffer_bpp: %u\r\n", vmode.framebuffer_bpp);
	printf(outt::e9, "framebuffer_type: %u\r\n", vmode.framebuffer_type);

	printf(outt::e9, "framebuffer_palette_addr: %u\r\n", vmode.framebuffer_palette_addr);
	printf(outt::e9, "framebuffer_palette_num_colors: %u\r\n", vmode.framebuffer_palette_num_colors);

	printf(outt::e9, "framebuffer_red_field_position: %u\r\n", vmode.framebuffer_red_field_position);
	printf(outt::e9, "framebuffer_red_mask_size: %u\r\n", vmode.framebuffer_red_mask_size);

	printf(outt::e9, "framebuffer_green_field_position: %u\r\n", vmode.framebuffer_green_field_position);
	printf(outt::e9, "framebuffer_green_mask_size: %u\r\n", vmode.framebuffer_green_mask_size);

	printf(outt::e9, "framebuffer_blue_field_position: %u\r\n", vmode.framebuffer_blue_field_position);
	printf(outt::e9, "framebuffer_blue_mask_size: %u\r\n", vmode.framebuffer_blue_mask_size);

    for(;;);
}
