#pragma once

#include <stddef.h>
#include <stdint.h>

#include <common_attributes.h>

namespace Kernel::CPU
{
	inline constexpr uint32_t GDT_ENTRY_COUNT = 8;

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
	} __packed gdt_entry_t;

	typedef struct gdt_descriptor_t
	{
		uint16_t size;
		uint32_t offset;
	} __packed gdt_descriptor_t;

	typedef struct tss_entry_t
	{
		uint32_t link : 16, : 16;
		uint32_t esp0;
		uint32_t ss0 : 16, : 16;
		uint32_t esp1;
		uint32_t ss1 : 16, : 16;
		uint32_t esp2;
		uint32_t ss2 : 16, : 16;
		uint32_t cr3;
		uint32_t eip;
		uint32_t eflags;
		uint32_t eax;
		uint32_t ecx;
		uint32_t edx;
		uint32_t ebx;
		uint32_t esp;
		uint32_t ebp;
		uint32_t esi;
		uint32_t edi;
		uint32_t es : 16, : 16;
		uint32_t cs : 16, : 16;
		uint32_t ss : 16, : 16;
		uint32_t ds : 16, : 16;
		uint32_t fs : 16, : 16;
		uint32_t gs : 16, : 16;
		uint32_t ldtr : 16, : 32;
		uint16_t iopb;
	} __packed tss_entry_t;
} // namespace Kernel::Processor
