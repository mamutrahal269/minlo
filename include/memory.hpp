#pragma once
#include <minlib.hpp>
struct [[gnu::packed]] e820_ent {
	u64 base;
	u64 length;
	u32 type;
	u32 ACPI;
};
u8 memlu(u32& meml, u32& memu);
u8 e820call(e820_ent& ent, u32& n);
