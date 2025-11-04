#include <minlib.hpp>
#include <cstdarg>

static dword_t write_char(byte_t *buf, dword_t size, dword_t pos, byte_t c) {
		if (pos < size - 1) {
			buf[pos] = c;
	}
	return 1;
}

static dword_t num_to_str(byte_t *str, unsigned long long num, dword_t base, dword_t uppercase) {
	const byte_t *digits = uppercase ? (const byte_t*)"0123456789ABCDEF" : (const byte_t*)"0123456789abcdef";
	byte_t temp[64];
	dword_t i = 0;
	
	if (num == 0) {
			str[0] = '0';
		str[1] = '\0';
		return 1;
	}
	
	while (num > 0) {
			temp[i++] = digits[num % base];
		num /= base;
	}
	
	for (dword_t j = 0; j < i; j++) {
			str[j] = temp[i - 1 - j];
	}
	str[i] = '\0';
	return i;
}

dword_t snprintf(byte_t *buf, dword_t size, const byte_t *fmt, ...) {
		va_list args;
	va_start(args, fmt);
	
	dword_t pos = 0;
	const byte_t *p = fmt;
	
	if (size == 0) {
			va_end(args);
		return 0;
	}
	
	while (*p) {
			if (*p != '%') {
				pos += write_char(buf, size, pos, *p);
			p++;
			continue;
		}
		
		p++;
		
		dword_t plus_flag = 0;
		dword_t space_flag = 0;
		dword_t hash_flag = 0;
		
		while (*p == '+' || *p == ' ' || *p == '#') {
				if (*p == '+') plus_flag = 1;
			if (*p == ' ') space_flag = 1;
			if (*p == '#') hash_flag = 1;
			p++;
		}
		
		if (*p == 'd' || *p == 'i') {
				dword_t val = va_arg(args, dword_t);
			byte_t num_str[64];
			dword_t is_negative = 0;
			
			if ((int)val < 0) {
					is_negative = 1;
				val = -(int)val;
			}
			
			num_to_str(num_str, val, 10, 0);
			
			if (is_negative) {
					pos += write_char(buf, size, pos, '-');
			} else if (plus_flag) {
					pos += write_char(buf, size, pos, '+');
			} else if (space_flag) {
					pos += write_char(buf, size, pos, ' ');
			}
			
			for (byte_t *s = num_str; *s; s++) {
					pos += write_char(buf, size, pos, *s);
			}
			
		} else if (*p == 'u') {
				dword_t val = va_arg(args, dword_t);
			byte_t num_str[64];
			num_to_str(num_str, val, 10, 0);
			
			for (byte_t *s = num_str; *s; s++) {
					pos += write_char(buf, size, pos, *s);
			}
			
		} else if (*p == 'x' || *p == 'X') {
				dword_t val = va_arg(args, dword_t);
			byte_t num_str[64];
			dword_t uppercase = (*p == 'X');
			
			if (hash_flag && val != 0) {
					pos += write_char(buf, size, pos, '0');
				pos += write_char(buf, size, pos, uppercase ? 'X' : 'x');
			}
			
			num_to_str(num_str, val, 16, uppercase);
			
			for (byte_t *s = num_str; *s; s++) {
					pos += write_char(buf, size, pos, *s);
			}
			
		} else if (*p == 'o') {
				dword_t val = va_arg(args, dword_t);
			byte_t num_str[64];
			num_to_str(num_str, val, 8, 0);
			
			for (byte_t *s = num_str; *s; s++) {
					pos += write_char(buf, size, pos, *s);
			}
			
		} else if (*p == 'p') {
				void *ptr = va_arg(args, void*);
			unsigned long long val = (unsigned long long)ptr;
			byte_t num_str[64];
			
			pos += write_char(buf, size, pos, '0');
			pos += write_char(buf, size, pos, 'x');
			
			num_to_str(num_str, val, 16, 0);
			
			for (byte_t *s = num_str; *s; s++) {
					pos += write_char(buf, size, pos, *s);
			}
			
		} else if (*p == 's') {
				byte_t *str = va_arg(args, byte_t*);
			if (!str) str = (byte_t*)"(null)";
			
			while (*str) {
					pos += write_char(buf, size, pos, *str);
				str++;
			}
			
		} else if (*p == 'c') {
				byte_t c = (byte_t)va_arg(args, dword_t);
			pos += write_char(buf, size, pos, c);
			
		} else if (*p == '%') {
				pos += write_char(buf, size, pos, '%');
			
		} else {
				pos += write_char(buf, size, pos, '%');
			pos += write_char(buf, size, pos, *p);
		}
		
		p++;
	}
	
	if (pos < size) {
			buf[pos] = '\0';
	} else if (size > 0) {
			buf[size - 1] = '\0';
	}
	
	va_end(args);
	return pos;
}
