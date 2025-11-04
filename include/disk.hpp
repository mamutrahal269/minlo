#include <minlib.hpp>

struct disk_descriptor {
	dword_t size;
	byte_t number;
	byte_t mode;
	word_t cylinders;
	byte_t heads;
	byte_t sectors;
	word_t port[16];
	word_t endport;
};
struct chs {
	word_t cylinder;
	byte_t head;
	byte_t sector;
};

byte_t readdisk(dword_t lba,
                dword_t sectors,
                const chs& max,
                const byte_t drive_n,
                void* buffer);
disk_descriptor diskdesc(const byte_t disk_n);
