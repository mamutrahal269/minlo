#include <minlib.hpp>
enum : u8 {
	SAVE_STATE, RESTORE_STATE
};
enum class mode_type {
	graphics = 0, text = 1
};
struct [[gnu::packed]] video_mode {
	const void* vbe_mode_info;
	u16 vbe_mode;
	u16 vbe_interface_seg, vbe_interface_off, vbe_interface_len;
	u64 framebuffer_addr;
	u32 framebuffer_pitch, framebuffer_width, framebuffer_height;
	u8 framebuffer_bpp, framebuffer_type;
	union {
		struct {
			u32 framebuffer_palette_addr;
			u32 framebuffer_palette_num_colors;
		};
		struct {
			u8 framebuffer_red_field_position, framebuffer_red_mask_size;
			u8 framebuffer_green_field_position, framebuffer_green_mask_size;
			u8 framebuffer_blue_field_position, framebuffer_blue_mask_size;
		};
	};
};
const void* VBEcontroller();
u16 VBEstate(const u8 op, u8* state_buff, const size_t bufsiz);
u8 VBEmode_setup(const u16 mode);
video_mode VBEmode_setup(const mode_type mode, const u32 width,  const u32 height, const u32 depth);
