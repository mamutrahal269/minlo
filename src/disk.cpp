#include <minlib.hpp>
#include <disk.hpp>
using namespace minlib;

byte_t readdisk(dword_t lba,
				dword_t sectors,
				const chs& max,
				const byte_t drive_n,
				void* buffer) {
	dword_t chs_limit;
	if (__builtin_mul_overflow(max.cylinder, max.head, &chs_limit))
		return 0xFF;

	if (__builtin_mul_overflow(chs_limit, max.sector, &chs_limit))
		return 0xFF;

	dword_t end_lba;
	if (__builtin_add_overflow(lba, sectors, &end_lba) ||
		end_lba >= chs_limit) return 0xFF;

	regs386 regs{};
	word_t bufptr = reinterpret_cast<dword_t>(buffer);

	while (sectors) {
		word_t cyl = lba / (max.head * max.sector);

		regs.ah = 2;
		regs.al = sectors < 127 ? sectors : 127;
		regs.es = esseg();
		regs.bx = bufptr;
		regs.ch = cyl;
		regs.cl = ((lba % max.sector) + 1) | ((cyl >> 2) & 0xC0);
		regs.dh = (lba / max.sector) % max.head;
		regs.dl = drive_n;
		int386(0x13, regs, regs);
		if (regs.eflags & eflags.CF)
			return regs.ah;

		sectors -= regs.al;

		if (__builtin_add_overflow(bufptr, regs.al * 512, &bufptr))
			return 0xFF;

		if (__builtin_add_overflow(lba, regs.al, &lba))
			return 0xFF;
	}
	return 0;
}
disk_descriptor diskdesc(const byte_t disk_n) {
	disk_descriptor disk{};
	regs386 regs{};
	regs.ah = 8;
	regs.dl = disk_n;
	int386(0x13, regs, regs);
	if(regs.eflags & eflags.CF) return {};

	disk.size = sizeof(disk_descriptor);
	disk.number = disk_n;
	disk.cylinders = (((regs.cl & 0xC0) << 2) | regs.ch) + 1;
	disk.heads = regs.dh + 1;
	disk.sectors = regs.cl & ~0xC0;

	regs.ah = 0x41;
	regs.dl = disk_n;
	regs.bx = 0x55AA;
	int386(0x13, regs, regs);
	disk.mode = (regs.bx == 0xAA55) ? 1 : 0;
	return disk;
}
