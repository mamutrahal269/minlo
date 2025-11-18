#pragma once
#include <minlib.hpp>
struct [[gnu::packed]] chs {
	u16 cylinder;
	u8 head;
	u8 sector;
};
struct [[gnu::packed]] disk_descriptor {
	u32 size;
	u8 number;
	u8 mode;
	chs max;
	u16 port[16];
	u16 endport;
	disk_descriptor (const u8 disk_n);
};

u8 readdisk(u32 lba, u32 sectors, const chs& max, const u8 drive_n, void* buffer);
