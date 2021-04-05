#ifndef ARCH_i686_INTERRUPTS_H
#define ARCH_i686_INTERRUPTS_H 1

#include <stddef.h>
#include <stdint.h>

namespace Kernel::Processor
{
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
	} __attribute__((packed)) idt_entry_t;

	typedef struct idt_descriptor_t
	{
		uint16_t limit;
		uint32_t base;
	} __attribute__((packed)) idt_descriptor_t;

	typedef struct registers_t
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
	} registers_t;
} // namespace Kernel::Processor

#endif // ARCH_i686_INTERRUPTS_H