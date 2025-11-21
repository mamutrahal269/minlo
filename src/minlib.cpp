#include <minlib.hpp>
void* memcpy(void* dest, const void* src, size_t n) {
    u8* d = (u8*)dest;
    const u8* s = (const u8*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}
void* memcpy(void* dst, u32 srcptr, size_t n) {
	u8 *d = (u8*)dst;
    for(; n; --n, ++d, ++srcptr) {
        asm volatile (
			"pushw ds\n"
			"mov ds, ax\n"
			"movsb\n"
			"popw ds\n"
			:
			: "a" (srcptr >> 4), "S" (srcptr & 0xF), "D" (d)
			: "memory"
        );
    }
    return dst;
}
u8 memcmp(const void* ptr1, const void* ptr2, size_t n) {
	const u8* p1 = (u8*) ptr1;
	const u8* p2 = (u8*) ptr2;
	while(n--) 
		if(p1[n] != p2[n]) 
			return 1;
	return 0;
}
u16 esseg() {
	u16 es;
	asm (
		"mov %[seg], es\n"
		: [seg] "=r" (es)
		:
		:
	);
	return es;
}
void outb(u16 port, u8 val) {
	asm volatile (
		"out dx, al\n"
		: 
		: "a"(val), "d"(port)
	);
}

u8 inb(u16 port) {
	u8 ret;
	asm volatile (
		"in al, dx\n"
		: "=a"(ret) 
		: "d"(port)
	);
	return ret;
}
