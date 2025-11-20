#include <minlib.hpp>
#include <vbe.hpp>
#define VBE_MODE_SUPPORTED (1 << 0)
#define VBE_MODE_BIOS_TTY_OUT (1 << 2)
#define VBE_MODE_COLOR (1 << 3)
#define VBE_MODE_GRAPHICS (1 << 4)
#define VBE_MODE_VGA_INCOMPATIBLE (1 << 5)
#define VBE_MODE_VGA_INCOMPATIBLE_WINDOWED_MEMORY (1 << 6)
#define VBE_MODE_LFB (1 << 7)
#define VBE_SUCCESS 0x004F

namespace {
	struct [[gnu::packed]] {
		u8  VbeSignature[4];
		u16  VbeVersion;
		u32 OemStringPtr;
		u32  Capabilities;
		u32 VideoModePtr;
		u16  TotalMemory;
		u16  OemSoftwareRev;
		u32 OemVendorNamePtr;
		u32 OemProductNamePtr;
		u32 OemProductRevPtr;
		u8  Reserved[222];
		u8  OemData[256];
	} VbeInfoBlock;

	struct [[gnu::packed]] {
	    u16  ModeAttributes;
	    u8  WinAAttributes;
    	u8  WinBAttributes;
    	u16  WinGranularity;
    	u16  WinSize;
    	u16  WinASegment;
    	u16  WinBSegment;
    	u32 WinFuncPtr;
    	u16  BytesPerScanLine;
	    u16  XResolution;
    	u16  YResolution;
    	u8  XCharSize;
    	u8  YCharSize;
    	u8  NumberOfPlanes;
    	u8  BitsPerPixel;
    	u8  NumberOfBanks;
	    u8  MemoryModel;
	    u8  BankSize;
    	u8  NumberOfImagePages;
    	u8  Reserved1;
    	u8  RedMaskSize;
    	u8  RedFieldPosition;
    	u8  GreenMaskSize;
	    u8  GreenFieldPosition;
	    u8  BlueMaskSize;
	    u8  BlueFieldPosition;
	    u8  RsvdMaskSize;
	    u8  RsvdFieldPosition;
	    u8  DirectColorModeInfo;
	    u32 PhysBasePtr;
	    u32 Reserved2;
	    u16  Reserved3;
	    u16  LinBytesPerScanLine;
	    u8  BnkNumberOfImagePages;
	    u8  LinNumberOfImagePages;
	    u8  LinRedMaskSize;
	    u8  LinRedFieldPosition;
	    u8  LinGreenMaskSize;
	    u8  LinGreenFieldPosition;
	    u8  LinBlueMaskSize;
    	u8  LinBlueFieldPosition;
    	u8  LinRsvdMaskSize;
    	u8  LinRsvdFieldPosition;
    	u32 MaxPixelClock;
	    u8  Reserved4[189];
	} ModeInfoBlock;
}
void* VBEcontroller() {
	static u8 initf = 0;
	if(initf) return &VbeInfoBlock;
	memcpy(VbeInfoBlock.VbeSignature, "VBE2" /* input signature */ , 4);
	regs386 regs;
	regs.es = esseg();
	regs.di = reinterpret_cast<u32>(&VbeInfoBlock);
	regs.ax = 0x4F00; /* Function 00h - Return VBE Controller Information */
	int386(0x10, regs, regs);
	if(regs.ax != VBE_SUCCESS || memcmp(VbeInfoBlock.VbeSignature, "VESA" /* output signature */ , 4)) return nullptr;
	initf = 1;
	return &VbeInfoBlock;
}
u16 VBEstate(const u8 op, u8* state_buff, const size_t bufsiz) {
	regs386 regs{};
	regs.ax = 0x4F04; /* Function 04h - Save/Restore State */
	/* dl = 0 Return Save/Restore State buffer size */
	int386(0x10, regs, regs);
	if((regs.bx * 64u) > bufsiz) return 1;
	regs.ax = 0x4F04;
	regs.dl = op == RESTORE_STATE ? 2 : 1;
	regs.cx = 0b1111; /* D0=	Save/Restore controller hardware state,
						 D1=	Save/Restore BIOS data state,
						 D2=	Save/Restore DAC state,
						 D3=	Save/Restore Register state */
	regs.es = esseg();
	regs.bx = reinterpret_cast<u32>(state_buff);
	int386(0x10, regs, regs);
	if(regs.ax != VBE_SUCCESS) return 1;
	return 0;
}
u8 VBEmode_setup(const u16 mode) {
	regs386 regs{};
	regs.ax = 0x4F02; /* Function 02h - Set VBE Mode */
	regs.bx = mode;
	int386(0x10, regs, regs);
	if(regs.ax != VBE_SUCCESS) return 1;
	return 0;
}
video_mode VBEmode_setup(const mode_type mode, const u32 width,  const u32 height, const u32 depth) {
	if(!VBEcontroller()) return {};
	if(VbeInfoBlock.VbeVersion < 0x0300) return {};
	
	regs386 iregs{}, oregs{};
	iregs.es = esseg();
	u32 mode_phys = (VbeInfoBlock.VideoModePtr >> 16) * 16 + (VbeInfoBlock.VideoModePtr & 0xFFFF);
	memcpy(&iregs.cx, mode_phys, sizeof(iregs.cx));
	u16 best_mode = ~0;
	u32 best_diff = ~0;
	
	iregs.ax = 0x4F01;
	iregs.di = reinterpret_cast<u32>(&ModeInfoBlock);
	for(; iregs.cx != 0xFFFF && best_diff; mode_phys += 2) {
		memcpy(&iregs.cx, mode_phys, sizeof(iregs.cx));
		int386(0x10, iregs, oregs);
		if(oregs.ax != VBE_SUCCESS) continue;
		if(!(ModeInfoBlock.ModeAttributes & VBE_MODE_SUPPORTED)) continue;
		if(mode == mode_type::text) {
			if(ModeInfoBlock.MemoryModel != 0) continue;
			if(ModeInfoBlock.ModeAttributes & (VBE_MODE_VGA_INCOMPATIBLE | VBE_MODE_GRAPHICS | VBE_MODE_VGA_INCOMPATIBLE_WINDOWED_MEMORY)) continue;
			
		}
		else {
			constexpr u16 mask = VBE_MODE_COLOR | VBE_MODE_GRAPHICS | VBE_MODE_LFB;
			if(ModeInfoBlock.ModeAttributes & VBE_MODE_SUPPORTED) continue;
			if((ModeInfoBlock.ModeAttributes & mask) != mask) continue;
			if(ModeInfoBlock.MemoryModel > 7) continue;
		}
		u32 cur_diff =
			(width  == 0 ? 0 :
				(width  > ModeInfoBlock.XResolution ?
					(width  - ModeInfoBlock.XResolution) :
					(ModeInfoBlock.XResolution - width))) +
			(height == 0 ? 0 :
				(height > ModeInfoBlock.YResolution ?
					(height - ModeInfoBlock.YResolution) :
					(ModeInfoBlock.YResolution - height))) +
			(mode == mode_type::text ? 0 :
				(depth == 0 ? 0 :
					(depth > ModeInfoBlock.BitsPerPixel ?
						(depth - ModeInfoBlock.BitsPerPixel) :
						(ModeInfoBlock.BitsPerPixel - depth))));

		if( cur_diff < best_diff) {
			best_diff = cur_diff;
			best_mode = iregs.cx;
		}
	}
	iregs.cx = best_mode;
	int386(0x10, iregs, oregs);
	if(oregs.ax != VBE_SUCCESS) return {};
	if(best_mode == ~0 || VBEmode_setup(mode == mode_type::text ? best_mode : (best_mode | (1 << 14 /* use LFB flags */ )))) return {};
	
	video_mode vmode{};
	vmode.vbe_mode_info = &ModeInfoBlock;
	vmode.vbe_mode = best_mode;
	iregs.ax = 0x4F0A;
	iregs.bl = 0; /* Return protected mode table */
	int386(0x10, iregs, oregs);
	if(oregs.ax == VBE_SUCCESS) {
		vmode.vbe_interface_seg = oregs.es;
		vmode.vbe_interface_off = oregs.di;
		vmode.vbe_interface_len = oregs.cx;
	}
	else vmode.vbe_interface_seg = vmode.vbe_interface_off = vmode.vbe_interface_len = 0;
	vmode.framebuffer_addr = mode == mode_type::text ? (ModeInfoBlock.WinASegment * 16) : ModeInfoBlock.PhysBasePtr;
	vmode.framebuffer_pitch = mode == mode_type::text ? ModeInfoBlock.BytesPerScanLine : ModeInfoBlock.LinBytesPerScanLine;
	vmode.framebuffer_width = ModeInfoBlock.XResolution;
	vmode.framebuffer_height = ModeInfoBlock.YResolution;
	vmode.framebuffer_bpp = mode == mode_type::text ? 16 : ModeInfoBlock.BitsPerPixel;
	if(mode == mode_type::text) vmode.framebuffer_type = 2;
	else if(mode == mode_type::graphics) {
		if (ModeInfoBlock.MemoryModel == 0x6 || ModeInfoBlock.MemoryModel == 0x7) {
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
			static u8 palette[colors];
			iregs.ax = 0x4F09;
			iregs.cx = colors;
			iregs.bl = 1;
			iregs.dx = 0;
			iregs.di = reinterpret_cast<u32>(palette);
			int386(0x10, iregs, oregs);
			if(oregs.ax == VBE_SUCCESS) {
				for (size_t i = 0; i < colors; ++i) {
					u16 src = i * 4;
					u16 dst = i * 3;

					u8 blue  = palette[src + 0];
					u8 green = palette[src + 1];
					u8 red   = palette[src + 2];

					palette[dst + 0] = red;
					palette[dst + 1] = green;
					palette[dst + 2] = blue;
				}
				vmode.framebuffer_palette_addr = reinterpret_cast<u32>(palette);
				vmode.framebuffer_palette_num_colors = colors;
			}
		}
	}
	return vmode;
}
