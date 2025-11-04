#include <minlib.cpp>
struct [[gnu::packed]] e820_ent {
	qword_t base;
	qword_t length;
	dword_t type;
	dword_t ACPI;
}
byte_t memlu(dword_t& meml, dword_t& memu);
word_t e820map(e820_ent* ents, word_t max_ents = 0xFFFF);
