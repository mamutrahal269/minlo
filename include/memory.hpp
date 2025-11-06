#include <minlib.hpp>
struct [[gnu::packed]] e820_ent {
	qword_t base;
	qword_t length;
	dword_t type;
	dword_t ACPI;
}
byte_t memlu(dword_t& meml, dword_t& memu);
byte_t e820call(e820_ent& ent, dword_t& n);
