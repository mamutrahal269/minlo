#include <minlib.hpp>
#define OOB(x) ((unsigned)(x)-'A' > 'z'-'A')
#define S(x) [(x)-'A']
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define RETINVAL return -2
#define RETOVERFLOW return -1

namespace {
	enum : int {
		BARE, LPRE, LLPRE, HPRE, HHPRE, BIGLPRE,
		ZTPRE, JPRE,
		STOP,
		PTR, INT, UINT, ULLONG,
		LONG, ULONG,
		SHORT, USHORT, CHAR, UCHAR,
		LLONG, SIZET, IMAX, UMAX, PDIFF, UIPTR,
		NOARG,
		MAXSTATE
	};
	constexpr auto NL_ARGMAX = 9U;
	constexpr auto INT_MAX   = 2147483647;
	constexpr auto ULONG_MAX = 4294967295;
	constexpr auto ALT_FORM  = (1U<<('#'-' '));
	constexpr auto ZERO_PAD  = (1U<<('0'-' '));
	constexpr auto LEFT_ADJ  = (1U<<('-'-' '));
	constexpr auto PAD_POS   = (1U<<(' '-' '));
	constexpr auto MARK_POS  = (1U<<('+'-' '));
	constexpr auto GROUPED   = (1U<<('\''-' '));
	constexpr auto FLAGMASK  = (ALT_FORM|ZERO_PAD|LEFT_ADJ|PAD_POS|MARK_POS|GROUPED);
	const unsigned char states[8][58] = {
	  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, UINT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, INT, INT, 0, 0, 0, HPRE, INT, JPRE, 0, LPRE, 0, PTR, UINT, UIPTR, 0, 0, PTR, ZTPRE, UINT, 0, 0, UINT, 0, ZTPRE,  },
	  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ULONG, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, UINT, LONG, 0, 0, 0, 0, LONG, 0, 0, LLPRE, 0, PTR, ULONG, 0, 0, 0, PTR, 0, ULONG, 0, 0, ULONG, 0, 0,  },
	  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ULLONG, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LLONG, 0, 0, 0, 0, LLONG, 0, 0, 0, 0, PTR, ULLONG, 0, 0, 0, 0, 0, ULLONG, 0, 0, ULLONG, 0, 0,  },
	  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, USHORT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, SHORT, 0, 0, 0, HHPRE, SHORT, 0, 0, 0, 0, PTR, USHORT, 0, 0, 0, 0, 0, USHORT, 0, 0, USHORT, 0, 0,  },
	  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, UCHAR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CHAR, 0, 0, 0, 0, CHAR, 0, 0, 0, 0, PTR, UCHAR, 0, 0, 0, 0, 0, UCHAR, 0, 0, UCHAR, 0, 0,  },
	  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, PTR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  },
	  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, SIZET, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, PDIFF, 0, 0, 0, 0, PDIFF, 0, 0, 0, 0, PTR, SIZET, 0, 0, 0, 0, 0, SIZET, 0, 0, SIZET, 0, 0,  },
	  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, UMAX, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, IMAX, 0, 0, 0, 0, IMAX, 0, 0, 0, 0, PTR, UMAX, 0, 0, 0, 0, 0, UMAX, 0, 0, UMAX, 0, 0,  },
	};
	union arg {
		u64 i;
		void *p;
	};
	void out(const outt ot, const char *s, size_t l) {
		switch(ot) {
			case outt::tty: {
				regs386 regs{};
				for(size_t i = 0; s[i] && i < l; ++i) {
					regs.ah = 0x0E;
					regs.al = s[i];
					int386(0x10, regs, regs);
				}
				return;
			}
			case outt::com: {
				for(size_t i = 0; s[i] && i < l; ++i) {
					while(!(inb(0x3F8 + 5) & 0x20));
					outb(0x3F8, s[i]);
				}
			}
		}
		return;
	}
	size_t strnlen(const char *s, size_t n) {
		size_t i = 0;
		while (i < n && s[i] != '\0') {
			i++;
		}
		return i;
	}
	bool isdigit(const char c) { return (c >= '0' && c <= '9'); }
	int getint(char* &s) {
		int i;
		for (i=0; isdigit(*s); ++s) {
			if (i > INT_MAX/10U || *s-'0' > INT_MAX-10*i) i = -1;
			else i = 10*i + (*s-'0');
		}
		return i;
	}
	void pad(const outt f, char c, int w, int l, int fl) {
		if (fl & (LEFT_ADJ | ZERO_PAD) || l >= w) return;
		char padstr[256];
		l = w - l;
		int n = (l > sizeof(padstr)) ? sizeof(padstr) : l;
		for (int i = 0; i < n; ++i) padstr[i] = c;
		for (; l >= sizeof(padstr); l -= sizeof(padstr))
			out(f, padstr, sizeof(padstr));
		out(f, padstr, l);
	}
	void pop_arg(union arg &arg, int type, va_list &ap) {
		switch (type) {
			   case PTR:	arg.p = va_arg(ap, void *);
		break; case INT:	arg.i = va_arg(ap, int);
		break; case UINT:	arg.i = va_arg(ap, unsigned int);
		break; case LONG:	arg.i = va_arg(ap, long);
		break; case ULONG:	arg.i = va_arg(ap, unsigned long);
		break; case ULLONG:	arg.i = va_arg(ap, unsigned long long);
		break; case SHORT:	arg.i = (short)va_arg(ap, int);
		break; case USHORT:	arg.i = (unsigned short)va_arg(ap, int);
		break; case CHAR:	arg.i = (signed char)va_arg(ap, int);
		break; case UCHAR:	arg.i = (unsigned char)va_arg(ap, int);
		break; case LLONG:	arg.i = va_arg(ap, long long);
		break; case SIZET:	arg.i = va_arg(ap, size_t);
		break; case IMAX:	arg.i = va_arg(ap, i64);
		break; case UMAX:	arg.i = va_arg(ap, u64);
		break; case PDIFF:	arg.i = va_arg(ap, i32);
		break; case UIPTR:	arg.i = reinterpret_cast<u32>(va_arg(ap, void *));
		}
	}
	char *fmt_x(u64 x, char *s, int lower) {
		static const char xdigits[] = "0123456789ABCDEF";
		for (; x; x>>=4) *--s = xdigits[(x&15)]|lower;
		return s;
	}
	char *fmt_o(u64 x, char *s) {
		for (; x; x>>=3) *--s = '0' + (x&7);
		return s;
	}

	char *fmt_u(u64 x, char *s) {
		unsigned long y;
		for (   ; x>ULONG_MAX; x/=10) *--s = '0' + x%10;
		for (y=x;           y; y/=10) *--s = '0' + y%10;
		return s;
	}
	int printf_core(const outt f, const char *fmt, va_list &ap, union arg *nl_arg, int *nl_type) {
		char *a, *z, *s=const_cast<char*>(fmt);
		unsigned l10n=0, fl;
		int w, p, xp;
		union arg arg;
		int argpos;
		unsigned st, ps;
		int cnt=0, l=0;
		size_t i;
		char buf[sizeof(u64)*3];
		const char *prefix;
		int t, pl;

		for (;;) {
			/* This error is only specified for snprintf, but since it's
			 * unspecified for other forms, do the same. Stop immediately
			 * on overflow; otherwise %n could produce wrong results. */
			if (l > INT_MAX - cnt) RETOVERFLOW;

			/* Update output count, end loop when fmt is exhausted */
			cnt += l;
			if (!*s) break;

			/* Handle literal text and %% format specifiers */
			for (a=s; *s && *s!='%'; s++);
			for (z=s; s[0]=='%' && s[1]=='%'; z++, s+=2);
			if (z-a > INT_MAX-cnt) RETOVERFLOW;
			l = z-a;
			if (f) out(f, a, l);
			if (l) continue;

			if (isdigit(s[1]) && s[2]=='$') {
				l10n=1;
				argpos = s[1]-'0';
				s+=3;
			} else {
				argpos = -1;
				s++;
			}

			/* Read modifier flags */
			for (fl=0; (unsigned)*s-' '<32 && (FLAGMASK&(1U<<*s-' ')); s++)
				fl |= 1U<<*s-' ';

			/* Read field width */
			if (*s=='*') {
				if (isdigit(s[1]) && s[2]=='$') {
					l10n=1;
					if (!f) nl_type[s[1]-'0'] = INT, w = 0;
					else w = nl_arg[s[1]-'0'].i;
					s+=3;
				} else if (!l10n) {
					w = f ? va_arg(ap, int) : 0;
					s++;
				} else RETINVAL;
				if (w<0) fl|=LEFT_ADJ, w=-w;
			} else if ((w=getint(s))<0) RETOVERFLOW;

			/* Read precision */
			if (*s=='.' && s[1]=='*') {
				if (isdigit(s[2]) && s[3]=='$') {
					if (!f) nl_type[s[2]-'0'] = INT, p = 0;
					else p = nl_arg[s[2]-'0'].i;
					s+=4;
				} else if (!l10n) {
					p = f ? va_arg(ap, int) : 0;
					s+=2;
				} else RETINVAL;
				xp = (p>=0);
			} else if (*s=='.') {
				s++;
				p = getint(s);
				xp = 1;
			} else {
				p = -1;
				xp = 0;
			}

			/* Format specifier state machine */
			st=0;
			do {
				if (OOB(*s)) RETINVAL;
				ps=st;
				st=states[st]S(*s++);
			} while (st-1<STOP);
			if (!st) RETINVAL;
			
			if (argpos>=0) {
				if (!f) nl_type[argpos]=st;
				else arg=nl_arg[argpos];
			} else if (f) pop_arg(arg, st, ap);
			else return 0;

			if (!f) continue;

			z = buf + sizeof(buf);
			prefix = "-+   0X0x";
			pl = 0;
			t = s[-1];

			/* - and 0 flags are mutually exclusive */
			if (fl & LEFT_ADJ) fl &= ~ZERO_PAD;

			switch(t) {
				case 'n':
					switch(ps) {
					case BARE: *(int *)arg.p = cnt; break;
					case LPRE: *(long *)arg.p = cnt; break;
					case LLPRE: *(long long *)arg.p = cnt; break;
					case HPRE: *(unsigned short *)arg.p = cnt; break;
					case HHPRE: *(unsigned char *)arg.p = cnt; break;
					case ZTPRE: *(size_t *)arg.p = cnt; break;
					case JPRE: *(u64 *)arg.p = cnt; break;
					}
					continue;
				case 'p':
					p = MAX(p, 2*sizeof(void*));
					t = 'x';
					fl |= ALT_FORM;
				case 'x': case 'X':
					a = fmt_x(arg.i, z, t&32);
					if (arg.i && (fl & ALT_FORM)) prefix+=(t>>4), pl=2;
					if (0) {
				case 'o':
					a = fmt_o(arg.i, z);
					if ((fl&ALT_FORM) && p<z-a+1) p=z-a+1;
					} if (0) {
				case 'd': case 'i':
					pl=1;
					if (arg.i>INT_MAX) {
						arg.i=-arg.i;
					} else if (fl & MARK_POS) {
						prefix++;
					} else if (fl & PAD_POS) {
						prefix+=2;
					} else pl=0;
				case 'u':
					a = fmt_u(arg.i, z);
					}
					if (xp && p<0) RETOVERFLOW;
					if (xp) fl &= ~ZERO_PAD;
					if (!arg.i && !p) {
						a=z;
						break;
					}
					p = MAX(p, z-a + !arg.i);
					break;
				case 'c':
					*(a=z-(p=1))=arg.i;
					fl &= ~ZERO_PAD;
					break;
				case 's':
					a = arg.p ? reinterpret_cast<decltype(a)>(arg.p) : const_cast<char*>("(null)");
					z = a + strnlen(a, p<0 ? INT_MAX : p);
					if (p<0 && *z) RETOVERFLOW;
					p = z-a;
					fl &= ~ZERO_PAD;
					break;
			}

			if (p < z-a) p = z-a;
			if (p > INT_MAX-pl) RETOVERFLOW;
			if (w < pl+p) w = pl+p;
			if (w > INT_MAX-cnt) RETOVERFLOW;

			pad(f, ' ', w, pl+p, fl);
			out(f, prefix, pl);
			pad(f, '0', w, pl+p, fl^ZERO_PAD);
			pad(f, '0', p, z-a, 0);
			out(f, a, z-a);
			pad(f, ' ', w, pl+p, fl^LEFT_ADJ);

			l = w;
		}

		if (f) return cnt;
		if (!l10n) return 0;

		for (i=1; i<=NL_ARGMAX && nl_type[i]; i++) 
			pop_arg(nl_arg[i], nl_type[i], ap);
		for (; i<=NL_ARGMAX && !nl_type[i]; i++);
		if (i<=NL_ARGMAX) RETINVAL;
		return 1;
	}
}
int printf(const outt f, const char *fmt, ...) {
	va_list ap, ap2;
	va_start(ap, fmt);
	va_copy(ap2, ap);
	int nl_type[NL_ARGMAX+1] = {0};
	union arg nl_arg[NL_ARGMAX+1];
	int ret;
	if ((ret = printf_core(none, fmt, ap2, nl_arg, nl_type)) < 0) {
		va_end(ap);
		va_end(ap2);
		return ret;
	}
	ret = printf_core(f, fmt, ap, nl_arg, nl_type);
	va_end(ap);
	va_end(ap2);
	return ret;
}
