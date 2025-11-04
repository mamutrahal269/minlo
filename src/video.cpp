#include <minlib.cpp>
#include <video.hpp>
asm(".code16gcc\n");

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
	} ModeInfoBlock[2];
}
mode_descriptor init_video(const byte_t mode_type, 
						   const dword_t width,
						   const dword_t height,
						   const dword_t depth) {
	word_t best_mode;
	word_t best_diff = 0xFFFF;
	mode_descriptor desc;
	regs386 iregs{}, oregs{};
	iregs.es = esseg();
	VbeInfoBlock.VbeSignature[0] = 'V';
	VbeInfoBlock.VbeSignature[1] = 'B';
	VbeInfoBlock.VbeSignature[2] = 'E';
	VbeInfoBlock.VbeSignature[3] = '2';
	iregs.ax = 0x4F00;
	iregs.di = reinterpret_cast<word_t>(&VbeInfoBlock);
	int386(0x10, iregs, oregs);
	if(oregs.ax != 0x004F) return {};
	if(!(VbeInfoBlock.VbeSignature[0] == 'V' &&
		 VbeInfoBlock.VbeSignature[1] == 'E' &&
		 VbeInfoBlock.VbeSignature[2] == 'S' &&
		 VbeInfoBlock.VbeSignature[3] == 'A')) return {};
	if(VbeInfoBlock.VbeVersion < 0x0300) return {};
	dword_t videoModePhys = (VbeInfoBlock.VideoModePtr >> 16) * 16 +
							(VbeInfoBlock.VideoModePtr & 0xFF);
	memcpyfar(static_cast<byte_t*>(&iregs.cx),
			  videoModePhys,
			  sizeof(iregs.cx));
	if(iregs.cx == 0xFFFF) return {};
	regs.di = reinterpret_cast<word_t>(&ModeInfoBlock[0]);
	iregs.ax = 0x4F01;
	int386(0x10, iregs, oregs);
	if(oregs.ax != 0x004F) return {};
	memcpyfar(static_cast<byte_t*>(&iregs.cx),
			  videoModePhys,
			  sizeof(iregs.cx));
	
	while(regs.cx != 0xFFFF) {
		
	}
}
