asm (".code16gcc\n");
extern "C" unsigned int [[regparm(3)]] lba2chs(const unsigned int lba, const void* p1, void* p2) {
	struct [[gnu::packed]] chs_t {
		unsigned short cyl;
		unsigned char  head;
		unsigned char  sect;
	};
	const chs_t* disk_geometry = reinterpret_cast<const chs_t*>(p1);
	chs_t* chs = reinterpret_cast<chs_t*>(p2);
	if(lba > (disk_geometry->cyl * disk_geometry->head * disk_geometry->sect))
		return 1;
	chs->cyl = lba / (disk_geometry->head * disk_geometry->sect);
	chs->head = (lba / disk_geometry->sect) % disk_geometry->head;
	chs->sect = (lba % disk_geometry->sect) + 1;
	return 0;
}
