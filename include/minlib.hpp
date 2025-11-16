#pragma once
using i8 = signed char;
using u8 = unsigned char;
using i16 = signed short;
using u16 = unsigned short;
using i32 = signed int;
using u32 = unsigned int;
using i64 = signed long long;
using u64 = unsigned long long;
using size_t = u32;

using va_list = __builtin_va_list;
#define va_start(ap, last)   __builtin_va_start(ap, last)
#define va_arg(ap, type)     __builtin_va_arg(ap, type)
#define va_end(ap)           __builtin_va_end(ap)

enum outt : i8 {
	none = 0, tty, com
};
enum eflags : u32 {
	CF = 0x0001,  
	PF = 0x0004, 
	AF = 0x0010,  
	ZF = 0x0040,  
	SF = 0x0080,  
	TF = 0x0100,  
	IF = 0x0200, 
	DF = 0x0400,  
	OF = 0x0800
};
struct regs386 {
	union {
		u32 eax;
		u16 ax;
		struct { u8 al, ah; };
	};
	union {
		u32 ebx;
		u16 bx;
		struct { u8 bl, bh; };
	};
	union {
		u32 ecx;
		u16 cx;
		struct { u8 cl, ch; };
	};
	union {
		u32 edx;
		u16 dx;
		struct { u8 dl, dh; };
	};
	union {
		u32 esi;
		u16 si;
	};
	union {
		u32 edi;
		u16 di;
	};
	union {
		u32 ebp;
		u16 bp;
	};
	u32 eflags;
	u16 ds, es, fs, gs;
};
void int386(const u8 intr, const regs386& iregs, regs386& oregs);
void* memcpy(void* dest, const void* src, size_t n);
void* memcpyfar(void* dst, u32 srcptr, size_t n);
u8 memcmp(const void* ptr1, const void* ptr2, size_t n);
u16 esseg();
int printf(const outt o, const char* fmt, ...);
void outb(u16 port, u8 val);
u8 inb(u16 port);

