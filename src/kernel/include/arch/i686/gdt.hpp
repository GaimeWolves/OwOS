#ifndef ARCH_i686_GDT_H
#define ARCH_i686_GDT_H 1

#include <stddef.h>
#include <stdint.h>

namespace Kernel::Processor
{
	typedef struct gdt_entry_t
	{
		uint32_t limit_low : 16;
		uint32_t base_low : 24;
		uint8_t accessed : 1;
		uint8_t read_write : 1;
		uint8_t direction_conforming : 1;
		uint8_t executable : 1;
		uint8_t type : 1;
		uint8_t privilege : 2;
		uint8_t present : 1;
		uint32_t limit_high : 4, : 2;
		uint8_t size : 1;
		uint8_t granularity : 1;
		uint32_t base_high : 8;

		inline uint32_t base() { return base_low | (base_high << 24); }
		inline uint32_t limit() { return limit_low | (limit_high << 16); }
	} __attribute__((packed)) gdt_entry_t;

	typedef struct gdt_descriptor_t
	{
		uint16_t size;
		uint32_t offset;
	} __attribute__((packed)) gdt_descriptor_t;

	inline gdt_entry_t create_gdt_selector(bool is_userspace, bool is_code)
	{
		return gdt_entry_t{
		    .limit_low = 0xFFFF,
		    .base_low = 0,
		    .accessed = 0,
		    .read_write = 1,
		    .direction_conforming = 0,
		    .executable = is_code,
		    .type = 1,
		    .privilege = (uint8_t)(is_userspace ? 3 : 0),
		    .present = 1,
		    .limit_high = 0x0F,
		    .size = 1,
		    .granularity = 1,
		    .base_high = 0,
		};
	}
} // namespace Kernel::Processor

#endif // ARCH_i686_GDT_H