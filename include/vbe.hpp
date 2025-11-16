#include <minlib.hpp>
struct [[gnu::packed]] gfx_mode {
	void* vbe_mode_info;
	word_t vbe_mode;
	word_t vbe_interface_seg, vbe_interface_off, vbe_interface_len;
	qword_t framebuffer_addr;
	dword_t framebuffer_pitch, framebuffer_width, framebuffer_height;
	byte__t framebuffer_bpp, framebuffer_type;
	union {
		struct {
			dword_t framebuffer_palette_addr;
			word_t framebuffer_palette_num_colors;
		};
		struct {
			byte_t framebuffer_red_field_position, framebuffer_red_mask_size;
			byte_t framebuffer_green_field_position, framebuffer_green_mask_size;
			byte_t framebuffer_blue_field_position, framebuffer_blue_mask_size;
		};
	};
};
void* gfxcontrol();
byte_t gfxsetup(const word_t mode);
gfx_mode gfxsetup(const byte_t mode_type, 
				  const dword_t width,
				  const dword_t height,
				  const dword_t depth);
