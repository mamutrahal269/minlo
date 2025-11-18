#include <minlib.cpp>
#include <vbe.hpp>

#define VBE_DAC_8_BIT                              (1 << 0)
#define VBE_VGA_INCOMPATIBLE                       (1 << 1)
#define VBE_RAMDAC_BIT_BLANK                       (1 << 2)
#define VBE_MODE_SUPPORTED                         (1 << 0)
#define VBE_MODE_BIOS_TTY_OUT                      (1 << 2) 
#define VBE_MODE_COLOR                             (1 << 3)
#define VBE_MODE_GRAPHICS                          (1 << 4)
#define VBE_MODE_VGA_INCOMPATIBLE                  (1 << 5)
#define VBE_MODE_VGA_INCOMPATIBLE_WINDOWED_MEMTYPE (1 << 6)
#define VBE_MODE_LFB                               (1 << 7)

#define VBE_SUCCESS 0x004F
#define TEXT_MODE 1
#define GRAPHICS_MODE 0

namespace {
	struct [[gnu::packed]] {
		byte_t  VbeSignature[4];
		word_t  VbeVersion;
		dword_t OemStringPtr;
		dword_t  Capabilities;
		dword_t VideoModePtr;
		word_t  TotalMemory;
		word_t  OemSoftwareRev;
		dword_t OemVendorNamePtr;
		dword_t OemProductNamePtr;
		dword_t OemProductRevPtr;
		byte_t  Reserved[222];
		byte_t  OemData[256];
	} VbeInfoBlock;

	struct [[gnu::packed]] {
	    word_t  ModeAttributes;
	    byte_t  WinAAttributes;
    	byte_t  WinBAttributes;
    	word_t  WinGranularity;
    	word_t  WinSize;
    	word_t  WinASegment;
    	word_t  WinBSegment;
    	dword_t WinFuncPtr;
    	word_t  BytesPerScanLine;
	    word_t  XResolution;
    	word_t  YResolution;
    	byte_t  XCharSize;
    	byte_t  YCharSize;
    	byte_t  NumberOfPlanes;
    	byte_t  BitsPerPixel;
    	byte_t  NumberOfBanks;
	    byte_t  MemoryModel;
	    byte_t  BankSize;
    	byte_t  NumberOfImagePages;
    	byte_t  Reserved1;
    	byte_t  RedMaskSize;
    	byte_t  RedFieldPosition;
    	byte_t  GreenMaskSize;
	    byte_t  GreenFieldPosition;
	    byte_t  BlueMaskSize;
	    byte_t  BlueFieldPosition;
	    byte_t  RsvdMaskSize;
	    byte_t  RsvdFieldPosition;
	    byte_t  DirectColorModeInfo;
	    dword_t PhysBasePtr;
	    dword_t Reserved2;
	    word_t  Reserved3;
	    word_t  LinBytesPerScanLine;
	    byte_t  BnkNumberOfImagePages;
	    byte_t  LinNumberOfImagePages;
	    byte_t  LinRedMaskSize;
	    byte_t  LinRedFieldPosition;
	    byte_t  LinGreenMaskSize;
	    byte_t  LinGreenFieldPosition;
	    byte_t  LinBlueMaskSize;
    	byte_t  LinBlueFieldPosition;
    	byte_t  LinRsvdMaskSize;
    	byte_t  LinRsvdFieldPosition;
    	dword_t MaxPixelClock;
	    byte_t  Reserved4[189];
	} ModeInfoBlock;
}
void* VBEcontrol() {
	static byte_t initz = 0;
	if(initz) return &VbeInfoBlock;
	memcpy(VbeInfoBlock.VbeSignature, "VBE2", 4);
	regs386 regs;
	regs.es = esseg();
	regs.di = reinterpret_cast<word_t>(&VbeInfoBlock);
	regs.ax = 0x4F00;
	int386(0x10, regs, regs);
	if(regs.ax != VBE_SUCCESS || memcmp(VbeInfoBlock.VbeSignature, "VESA", 4)) return nullptr;
	initz = 1;
	return &VbeInfoBlock;
}
byte_t VBEmode_setup(const word_t mode) {
	regs386 regs{};
	regs.ax = 0x4F02;
	regs.bx = mode;
	int386(0x10, regs, regs);
	if(regs.ax != VBE_SUCCESS) return 1;
	return 0;
}
gfx_mode gfxsetup(const byte_t mode_type, 
				  const dword_t width,
				  const dword_t height,
				  const dword_t depth) {
	if(mode_type > GRAPHICS_MODE) return {};
	regs386 iregs{}, oregs{};
	iregs.es = esseg();
	if(!gfxcontrol()) return {};
	if(VbeInfoBlock.VbeVersion < 0x0300) return {};
	dword_t mode_phys = (VbeInfoBlock.VideoModePtr >> 16) * 16 + (VbeInfoBlock.VideoModePtr & 0xFFFF);
	memcpyfar(&iregs.cx, mode_phys, sizeof(iregs.cx));
	if(iregs.cx == 0xFFFF) return {};
	word_t best_mode = ~0;
	dword_t best_diff = ~0;
	for(; iregs.cx != 0xFFFF && best_diff; mode_phys += 2) {
		memcpyfar(&iregs.cx, mode_phys, sizeof(iregs.cx));
		iregs.ax = 0x4F01;
		iregs.di = reinterpret_cast<word_t>(&ModeInfoBlock);
		int386(0x10, iregs, oregs);
		if(oregs.ax != VBE_SUCCESS) continue;
		if(!(ModeInfoBlock.ModeAttributes & VBE_MODE_SUPPORTED)) continue;
		if(mode_type == TEXT_MODE) {
			if(ModeInfoBlock.MemoryModel != 0) continue;
			if(ModeInfoBlock.ModeAttributes & (VBE_MODE_VGA_INCOMPATIBLE | VBE_MODE_GRAPHICS | VBE_MODE_VGA_INCOMPATIBLE_WINDOWED_MEMTYPE)) continue;
			
		}
		else {
			constexpr word_t mask = VBE_MODE_COLOR_MODE | VBE_MODE_GRAPHICS | VBE_MODE_LFB;
			if(ModeInfoBlock.ModeAttributes & VBE_MODE_UNSUPPORTED) continue;
			if((ModeInfoBlock.ModeAttributes & mask) != mask) continue;
			if(ModeInfoBlock.MemoryModel > 7) continue;
		}
		dword_t cur_diff =
			(width  == 0 ? 0 :
				(width  > ModeInfoBlock.XResolution ?
					width  - ModeInfoBlock.XResolution :
					ModeInfoBlock.XResolution - width)) +
			(height == 0 ? 0 :
				(height > ModeInfoBlock.YResolution ?
					height - ModeInfoBlock.YResolution :
					ModeInfoBlock.YResolution - height)) +
			(mode_type == TEXT_MODE ? 0 :
				(depth == 0 ? 0 :
					(depth > ModeInfoBlock.BitsPerPixel ?
						depth - ModeInfoBlock.BitsPerPixel :
						ModeInfoBlock.BitsPerPixel - depth)));

		if( cur_diff < best_diff) {
			best_diff = cur_diff;
			best_mode = iregs.cx;
		}
	}
	iregs.cx = best_mode;
	iregs.di = reinterpret_cast<word_t>(&ModeInfoBlock);
	iregs.ax = 0x4F01;
	int386(0x10, iregs, oregs);
	if(oregs.ax != VBE_SUCCESS) return {};
	if(best_mode == ~0 || gfxsetup(mode_type == TEXT_MODE ? best_mode : (best_mode | (1 << 14))) return {};
	
	gfx_mode vmode{};
	vmode.vbe_mode_info = &ModeInfoBlock;
	vmode.vbe_mode = best_mode;
	iregs.ax = 0x4F0A;
	iregs.bl = 0;
	int386(0x10, iregs, oregs);
	if(oregs.ax == VBE_SUCCESS) {
		vmode.vbe_interface_seg = oregs.es;
		vmode.vbe_interface_off = oregs.di;
		vmode.vbe_interface_len = oregs.cx;
	}
	else vmode.vbe_interface_seg = vmode.vbe_interface_off = vmode.vbe_interface_len = 0;
	vmode.framebuffer_addr = mode_type == TEXT_MODE ? ModeInfoBlock.WinASegment * 16 : ModeInfoBlock.PhysBasePtr;
	vmode.framebuffer_pitch = mode_type == TEXT_MODE ? ModeInfoBlock.BytesPerScanLine : ModeInfoBlock.LinBytesPerScanLine;
	vmode.framebuffer_width = ModeInfoBlock.XResolution;
	vmode.framebuffer_height = ModeInfoBlock.YResolution;
	vmode.framebuffer_bpp = mode_type == TEXT_MODE ? 16 : ModeInfoBlock.BitsPerPixel;
	if(mode_type == TEXT_MODE) vmode.framebuffer_type = 2;
	else if(mode_type == GRAPHICS_MODE) {
		if(ModeInfoBlock.MemoryModel == 0x6 || ModeInfoBlock.MemoryModel == 0x7) {
			vmode.framebuffer_type = 1;
			vmode.framebuffer_red_field_position = ModeInfoBlock.RedFieldPosition;
			vmode.framebuffer_red_mask_size = ModeInfoBlock.RedMaskSize;
			vmode.framebuffer_green_field_position = ModeInfoBlock.GreenFieldPosition;
			vmode.framebuffer_green_mask_size = ModeInfoBlock.GreenMaskSize;
			vmode.framebuffer_blue_field_position = ModeInfoBlock.BlueFieldPosition;
			vmode.framebuffer_blue_mask_size = ModeInfoBlock.BlueMaskSize;
		}
		else {
			vmode.framebuffer_type = 0;
			constexpr auto colors = 64u * 4u;
			static byte_t palette[colors];
			iregs.ax = 0x4F09;
			iregs.cx = colors;
			iregs.bl = 1;
			iregs.dx = 0;
			iregs.di = reinterpret_cast<word_t>(palette);
			int386(0x10, iregs, oregs);
			if(oregs.ax == VBE_SUCCESS) {
				for (word_t i = 0; i < colors; ++i) {
					word_t src = i * 4;
					word_t dst = i * 3;

					byte_t blue  = palette[src + 0];
					byte_t green = palette[src + 1];
					byte_t red   = palette[src + 2];

					palette[dst + 0] = red;
					palette[dst + 1] = green;
					palette[dst + 2] = blue;
				}
				vmode.framebuffer_palette_addr = reintrpret_cast<dword_t>(palette);
				vmode.framebuffer_palette_num_colors = colors;
			}
		}
	}
	return vmode;
}
