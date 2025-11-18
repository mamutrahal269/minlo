#include <minlib.hpp>
#include <disk.hpp>

u8 readdisk(u32 lba, u32 sectors, const chs& max, const u8 drive_n, void* buffer) {
	u32 chs_limit;
	if (__builtin_mul_overflow(max.cylinder, max.head, &chs_limit))
		return 0xFF;

	if (__builtin_mul_overflow(chs_limit, max.sector, &chs_limit))
		return 0xFF;

	u32 end_lba;
	if (__builtin_add_overflow(lba, sectors, &end_lba) ||
		end_lba >= chs_limit) return 0xFF;

	regs386 regs{};
	u16 bufptr = reinterpret_cast<u32>(buffer);

	while (sectors) {
		u16 cyl = lba / (max.head * max.sector);

		regs.ah = 2;
		regs.al = sectors < 127 ? sectors : 127;
		regs.es = esseg();
		regs.bx = bufptr;
		regs.ch = cyl;
		regs.cl = ((lba % max.sector) + 1) | ((cyl >> 2) & 0xC0);
		regs.dh = (lba / max.sector) % max.head;
		regs.dl = drive_n;
		int386(0x13, regs, regs);
		if (regs.eflags & eflags::CF)
			return regs.ah;

		sectors -= regs.al;

		if (__builtin_add_overflow(bufptr, regs.al * 512, &bufptr))
			return 0xFF;

		if (__builtin_add_overflow(lba, regs.al, &lba))
			return 0xFF;
	}
	return 0;
}
disk_descriptor::disk_descriptor(const u8 disk_n) {
	regs386 regs{};
	regs.ah = 8;
	regs.dl = disk_n;
	int386(0x13, regs, regs);
	if(regs.eflags & eflags::CF) {
		size = number = mode  = max.cylinder = max.head = max.sector = port[0] = 0;
		return;
	}
	size = sizeof(disk_descriptor);
	number = disk_n;
	max.cylinder = (((regs.cl & 0xC0) << 2) | regs.ch) + 1;
	max.head = regs.dh + 1;
	max.sector = regs.cl & ~0xC0;
	regs.ah = 0x41;
	regs.dl = disk_n;
	regs.bx = 0x55AA;
	int386(0x13, regs, regs);
	mode = regs.bx == 0xAA55;
}
