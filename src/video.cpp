#include <minlib.cpp>
#include <video.hpp>
#define VBE_SUCCESS 0x004F
#define TEXT_MODE 1
#define GRAPHICS_MODE 0

namespace {
	struct [[gnu::packed]] {
		byte_t  VbeSignature[4];
		word_t  VbeVersion;
		dword_t OemStringPtr;
		byte_t  Capabilities[4];
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
mode_descriptor init_video(const byte_t mode_type, 
						   const dword_t width,
						   const dword_t height,
						   const dword_t depth) {
	if(mode_type > GRAPHICS_MODE) return {};
	strncpy(VbeInfoBlock.VbeSignature, "VBE2", 4);
	regs386 iregs{}, oregs{};
	iregs.ax = 0x4F00;
	iregs.es = esseg();
	iregs.di = reinterpret_cast<word_t>(&VbeInfoBlock);
	int386(0x10, iregs, oregs);
	if(oregs.ax != VBE_SUCCESS) return {};
	if(strncmp(VbeInfoBlock.VbeSignature, "VESA", 4)) return {};
	if(VbeInfoBlock.VbeVersion < 0x0300) return {};
	dword_t mode_phys = (VbeInfoBlock.VideoModePtr >> 16) * 16 + (VbeInfoBlock.VideoModePtr & 0xFFFF);
	memcpyfar(static_cast<byte_t*>(&iregs.cx), mode_phys, sizeof(iregs.cx));
	if(iregs.cx == 0xFFFF) return {};
	word_t best_mode = -1;
	dword_t best_diff = -1;
	for(; iregs.cx != 0xFFFF; mode_phys += 2) {
		if(best_diff == 0) break;
		memcpyfar(static_cast<byte_t*>(&iregs.cx), mode_phys, sizeof(iregs.cx));
		iregs.ax = 0x4F01;
		iregs.di = reinterpret_cast<word_t>(&ModeInfoBlock);
		int386(0x10, iregs, oregs);
		if(oregs.ax != VBE_SUCCESS) continue;
		if(!(ModeInfoBlock.ModeAttributes & VBE_MODE_SUPPORTED)) continue;
		if(mode_type == TEXT_MODE) {
			if(ModeInfoBlock.MemoryModel != 0) continue;
			if(ModeInfoBlock.ModeAttributes & (VBE_MODE_VGA_UNSUPPORTED | VBE_MODE_GRAPHICS)) continue;
			
		}
		else {
			const word_t mask = VBE_MODE_COLOR_MODE | VBE_MODE_GRAPHICS | VBE_MODE_LFB;
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
	if(best_mode == -1 || set_video(mode_type == TEXT_MODE ? best_mode : best_mode | (1 << 14)) return {};
	
	mode_descriptor desc{};
	desc.vbe_control_info = &VbeInfoBlock;
	desc.vbe_mode_info = &ModeInfoBlock;
	desc.vbe_mode = best_mode;
	iregs.ax = 0x4F0A;
	iregs.bl = 0;
	int386(0x10, iregs, oregs);
	if(oregs.ax == VBE_SUCCESS) {
		desc.vbe_interface_seg = oregs.es;
		desc.vbe_interface_off = oregs.di;
		desc.vbe_interface_len = oregs.cx;
	}
	else desc.vbe_interface_seg = desc.vbe_interface_off = desc.vbe_interface_len = 0;
	desc.framebuffer_addr = mode_type == TEXT_MODE ? ModeInfoBlock.WinASegment * 16 : ModeInfoBlock.PhysBasePtr;
	desc.framebuffer_pitch = mode_type == TEXT_MODE ? ModeInfoBlock.BytesPerScanLine : ModeInfoBlock.LinBytesPerScanLine;
	desc.framebuffer_width = ModeInfoBlock.XResolution;
	desc.framebuffer_height = ModeInfoBlock.YResolution;
	desc.framebuffer_bpp = mode_type == TEXT_MODE ? 16 : ModeInfoBlock.BitsPerPixel;
	if(mode_type == GRAPHICS_MODE) {
		if(ModeInfoBlock.MemoryModel == 0x6 || ModeInfoBlock.MemoryModel == 0x7) {
			desc.framebuffer_type = 1;
			desc.framebuffer_red_field_position = ModeInfoBlock.RedFieldPosition;
			desc.framebuffer_red_mask_size = ModeInfoBlock.RedMaskSize;
			desc.framebuffer_green_field_position = ModeInfoBlock.GreenFieldPosition;
			desc.framebuffer_green_mask_size = ModeInfoBlock.GreenMaskSize;
			desc.framebuffer_blue_field_position = ModeInfoBlock.BlueFieldPosition;
			desc.framebuffer_blue_mask_size = ModeInfoBlock.BlueMaskSize;
			
	}
	else {
		desc.framebuffer_type = 0;
		const auto colors = 2 << ModeInfoBlock.BitsPerPixel;
		byte_t palette[colors * 4];
		iregs.ax = 0x4F09;
		iregs.cx = colors;
		iregs.bl = 1;
		iregs.dx = 0;
		iregs.di reinterpret_cast<word_t>(palette);
		int386(0x10, iregs, oregs);
		if(oregs.ax == VBE_SUCCESS) {
			
		}
	}
}
