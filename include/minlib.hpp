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
byte_t* strncpy(byte_t *restrict dst, const byte_t *restrict src, word_t n);
byte_t strncmp(const byte_t *s1, const byte_t *s2, word_t n);
template <typename T>
inline T abs(const T num) {
	return num < 0 ? -num : num;
}
word_t esseg();
void memcpyfar(byte_t* buffer, dword_t physAddr, word_t n);
dword_t snprintf(byte_t *buf, dword_t size, const byte_t *fmt, ...);
void logf(const byte_t* fmt, ...);
#ifdef DEBUG
void outb(word_t port, byte_t val);
byte_t inb(word_t port);
void comlogf(const byte_t* fmt, ...);
#endif
