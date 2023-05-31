#pragma once

#include <stddef.h>
#include <stdint.h>

#include <common_attributes.h>

namespace Kernel::CPU
{
	inline constexpr uint32_t MAX_INTERRUPTS = 256;
	inline constexpr uint32_t FIRST_USABLE_INTERRUPT = 32; // 0x00 - 0x19 reserved for exceptions, etc.

	enum class IDTEntryType : uint8_t
	{
		TASK_GATE = 0x5,
		INTERRUPT_GATE = 0xE,
		TRAP_GATE = 0xF,
	};

	typedef struct idt_entry_t
	{
		uint16_t offset_low : 16;
		uint16_t segment : 16;
		uint8_t : 8;
		uint8_t type : 4;
		uint8_t : 1;
		uint8_t privilege : 2;
		uint8_t present : 1;
		uint16_t offset_high : 16;

		inline uint32_t offset() { return (uint32_t)offset_low | ((uint32_t)offset_high << 16); }
	} __packed idt_entry_t;

	typedef struct idt_descriptor_t
	{
		uint16_t limit;
		uint32_t base;
	} __packed idt_descriptor_t;

	typedef struct interrupt_frame_t
	{
		uint32_t ss;
		uint32_t gs;
		uint32_t fs;
		uint32_t es;
		uint32_t ds;
		uint32_t edi;
		uint32_t esi;
		uint32_t ebp;
		uint32_t esp;
		uint32_t ebx;
		uint32_t edx;
		uint32_t ecx;
		uint32_t eax;
		uint32_t isr_number;
		uint32_t error_code;
		uint32_t eip;
		uint32_t cs;
		uint32_t eflags;
		uint32_t old_esp;
		uint32_t old_ss;

		void set_return_value(uintptr_t value) { eax = value; }
	} interrupt_frame_t;
} // namespace Kernel::Processor
