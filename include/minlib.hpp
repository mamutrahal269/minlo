using byte_t = unsigned char;
using word_t = unsigned short;
using dword_t = unsigned int;
#define EFLAGS_CF 0x0001  
#define EFLAGS_PF 0x0004  
#define EFLAGS_AF 0x0010  
#define EFLAGS_ZF 0x0040  
#define EFLAGS_SF 0x0080  
#define EFLAGS_TF 0x0100  
#define EFLAGS_IF 0x0200  
#define EFLAGS_DF 0x0400  
#define EFLAGS_OF 0x0800  
#define EFLAGS_IOPL 0x3000  
#define EFLAGS_NT 0x4000  
#define EFLAGS_RF 0x10000  
#define EFLAGS_VM 0x20000  
#define EFLAGS_AC 0x40000  
#define EFLAGS_VIF 0x80000  
#define EFLAGS_VIP 0x100000  
#define EFLAGS_ID 0x200000
struct regs386 {
	union {
		dword_t eax;
		word_t ax;
		struct { byte_t al, ah; };
	};
	union {
		dword_t ebx;
		word_t bx;
		struct { byte_t bl, bh; };
	};
	union {
		dword_t ecx;
		word_t cx;
		struct { byte_t cl, ch; };
	};
	union {
		dword_t edx;
		word_t dx;
		struct { byte_t dl, dh; };
	};
	union {
		dword_t esi;
		word_t si;
	};
	union {
		dword_t edi;
		word_t di;
	};
	union {
		dword_t ebp;
		word_t bp;
	};
	dword_t eflags;
	word_t ds, es, fs, gs;
};

void int386(const byte_t intr, const regs386& iregs, regs386& oregs);
dword_t snprintf(byte_t *buf, dword_t size, const byte_t *fmt, ...);
void logf(const byte_t* fmt, ...);
#ifdef DEBUG
void outb(word_t port, byte_t val);
byte_t inb(word_t port);
void comlogf(const byte_t* fmt, ...);
#endif
