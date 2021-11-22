#include <arch/interrupts.hpp>

#include "interrupt_stubs.hpp"

#include <arch/i686/interrupts.hpp>
#include <arch/i686/pic.hpp>
#include <arch/processor.hpp>
#include <common_attributes.h>

#include <libk/kcstdio.hpp>
#include <libk/kstring.hpp>

#define MAX_INTERRUPTS 256

#define INIT_INTERRUPT(i, n)                                                        \
	{                                                                               \
		idt[(i)] = create_idt_entry(isr_##i##_entry, IDTEntryType::INTERRUPT_GATE); \
		irqs[(i)].name = LibK::string((n));                                         \
	}

#define INIT_TRAP(i, n)                                                        \
	{                                                                          \
		idt[(i)] = create_idt_entry(isr_##i##_entry, IDTEntryType::TRAP_GATE); \
		irqs[(i)].name = LibK::string((n));                                    \
	}

#define INIT_PIC_INTERRUPT(i, n)                                                    \
	{                                                                               \
		idt[(i)] = create_idt_entry(isr_##i##_entry, IDTEntryType::INTERRUPT_GATE); \
		irqs[(i)].name = LibK::string((n));                                         \
		irqs[(i)].acknowledge = pic_eoi;                                            \
	}

namespace Kernel::Processor
{
	static idt_entry_t idt[MAX_INTERRUPTS];
	static idt_descriptor_t idtr;
	static irq_descriptor_t irqs[MAX_INTERRUPTS];

	static idt_entry_t create_idt_entry(void (*entry)(), IDTEntryType type);

	extern "C" __naked void common_interrupt_handler_entry();
	extern "C" void common_interrupt_handler(registers_t *regs);

	static idt_entry_t create_idt_entry(void (*entry)(), IDTEntryType type)
	{
		static_assert(sizeof(size_t) == sizeof(uint32_t));
		size_t address = (size_t)entry;

		// We don't implement TSS currently
		assert(type != IDTEntryType::TASK_GATE);

		return idt_entry_t{
		    .offset_low = (uint16_t)(address & 0xFFFF),
		    .segment = 0x08,
		    .type = (uint8_t)type,
		    .privilege = 0,
		    .present = 1,
		    .offset_high = (uint16_t)(address >> 16),
		};
	}

	extern "C" __naked void common_interrupt_handler_entry()
	{
		asm(
		    "pusha\n"
		    "pushl %ds\n"
		    "pushl %es\n"
		    "pushl %fs\n"
		    "pushl %gs\n"
		    "pushl %ss\n"
		    "mov $0x10, %ax\n"
		    "mov %ax, %ds\n"
		    "mov %ax, %es\n"
		    "pushl %esp\n"
		    "cld\n"
		    "call common_interrupt_handler\n"
		    "popl %esp\n"
		    "popl %ss\n"
		    "popl %gs\n"
		    "popl %fs\n"
		    "popl %es\n"
		    "popl %ds\n"
		    "popa\n"
		    "addl $0x8, %esp\n"
		    "iret\n");
	}

	extern "C" void common_interrupt_handler(registers_t *regs)
	{
		uint32_t n = regs->isr_number;
		if (irqs[n].acknowledge)
			irqs[n].acknowledge(n);

		for (auto &action : irqs[n].actions)
			action(n, regs);

		if (irqs[n].actions.size() == 0)
			unhandled_interrupt(n, regs);
	}

	void init_interrupts()
	{
		// Initialize first 32 reserved interrupts
		INIT_INTERRUPT(0x00, "Divide by 0");
		INIT_INTERRUPT(0x01, "Single step (Debugger)");
		INIT_INTERRUPT(0x02, "Non Maskable Interrupt (NMI)");
		INIT_INTERRUPT(0x03, "Breakpoint (Debugger)");
		INIT_TRAP(0x04, "Overflow");
		INIT_INTERRUPT(0x05, "Bounds check");
		INIT_INTERRUPT(0x06, "Invalid opcode");
		INIT_INTERRUPT(0x07, "Device not available");
		INIT_INTERRUPT(0x08, "Double Fault");
		INIT_INTERRUPT(0x09, "Coprocessor segment overrun");
		INIT_INTERRUPT(0x0A, "Invalid Task State Segment (TSS)");
		INIT_INTERRUPT(0x0B, "Segment not present");
		INIT_INTERRUPT(0x0C, "Stack fault exception");
		INIT_INTERRUPT(0x0D, "General Protection Fault (GPF)");
		INIT_INTERRUPT(0x0E, "Page Fault");
		INIT_INTERRUPT(0x0F, "Unassigned");
		INIT_INTERRUPT(0x10, "x87 FPU Error");
		INIT_INTERRUPT(0x11, "Aligment Check");
		INIT_INTERRUPT(0x12, "Machine Check");
		INIT_INTERRUPT(0x13, "SIMD FPU Exception");
		INIT_INTERRUPT(0x14, "Intel reserved");
		INIT_INTERRUPT(0x15, "Intel reserved");
		INIT_INTERRUPT(0x16, "Intel reserved");
		INIT_INTERRUPT(0x17, "Intel reserved");
		INIT_INTERRUPT(0x18, "Intel reserved");
		INIT_INTERRUPT(0x19, "Intel reserved");
		INIT_INTERRUPT(0x1A, "Intel reserved");
		INIT_INTERRUPT(0x1B, "Intel reserved");
		INIT_INTERRUPT(0x1C, "Intel reserved");
		INIT_INTERRUPT(0x1D, "Intel reserved");
		INIT_INTERRUPT(0x1E, "Intel reserved");
		INIT_INTERRUPT(0x1F, "Intel reserved");

		// Remap PIC and initialize corresponding interrupts
		init_pic();

		INIT_PIC_INTERRUPT(0x20, "(PIC) Timer");
		INIT_PIC_INTERRUPT(0x21, "(PIC) Keyboard");
		INIT_PIC_INTERRUPT(0x22, "(PIC) Cascade");
		INIT_PIC_INTERRUPT(0x23, "(PIC) Serial Port 2");
		INIT_PIC_INTERRUPT(0x24, "(PIC) Serial Port 1");
		INIT_PIC_INTERRUPT(0x25, "(PIC) Parallel Port 2");
		INIT_PIC_INTERRUPT(0x26, "(PIC) Diskette drive");
		INIT_PIC_INTERRUPT(0x27, "(PIC) Parallel Port 1");

		INIT_PIC_INTERRUPT(0x28, "(PIC) CMOS RTC");
		INIT_PIC_INTERRUPT(0x29, "(PIC) CGA vertical retrace");
		INIT_PIC_INTERRUPT(0x2A, "(PIC) Reserved");
		INIT_PIC_INTERRUPT(0x2B, "(PIC) Reserved");
		INIT_PIC_INTERRUPT(0x2C, "(PIC) Auxiliary device");
		INIT_PIC_INTERRUPT(0x2D, "(PIC) FPU");
		INIT_PIC_INTERRUPT(0x2E, "(PIC) Hard disk controller");
		INIT_PIC_INTERRUPT(0x2F, "(PIC) Reserved");

		INIT_INTERRUPT(0x30, "Unused");
		INIT_INTERRUPT(0x31, "Unused");
		INIT_INTERRUPT(0x32, "Unused");
		INIT_INTERRUPT(0x33, "Unused");
		INIT_INTERRUPT(0x34, "Unused");
		INIT_INTERRUPT(0x35, "Unused");
		INIT_INTERRUPT(0x36, "Unused");
		INIT_INTERRUPT(0x37, "Unused");
		INIT_INTERRUPT(0x38, "Unused");
		INIT_INTERRUPT(0x39, "Unused");
		INIT_INTERRUPT(0x3A, "Unused");
		INIT_INTERRUPT(0x3B, "Unused");
		INIT_INTERRUPT(0x3C, "Unused");
		INIT_INTERRUPT(0x3D, "Unused");
		INIT_INTERRUPT(0x3E, "Unused");
		INIT_INTERRUPT(0x3F, "Unused");

		INIT_INTERRUPT(0x40, "Unused");
		INIT_INTERRUPT(0x41, "Unused");
		INIT_INTERRUPT(0x42, "Unused");
		INIT_INTERRUPT(0x43, "Unused");
		INIT_INTERRUPT(0x44, "Unused");
		INIT_INTERRUPT(0x45, "Unused");
		INIT_INTERRUPT(0x46, "Unused");
		INIT_INTERRUPT(0x47, "Unused");
		INIT_INTERRUPT(0x48, "Unused");
		INIT_INTERRUPT(0x49, "Unused");
		INIT_INTERRUPT(0x4A, "Unused");
		INIT_INTERRUPT(0x4B, "Unused");
		INIT_INTERRUPT(0x4C, "Unused");
		INIT_INTERRUPT(0x4D, "Unused");
		INIT_INTERRUPT(0x4E, "Unused");
		INIT_INTERRUPT(0x4F, "Unused");

		INIT_INTERRUPT(0x50, "Unused");
		INIT_INTERRUPT(0x51, "Unused");
		INIT_INTERRUPT(0x52, "Unused");
		INIT_INTERRUPT(0x53, "Unused");
		INIT_INTERRUPT(0x54, "Unused");
		INIT_INTERRUPT(0x55, "Unused");
		INIT_INTERRUPT(0x56, "Unused");
		INIT_INTERRUPT(0x57, "Unused");
		INIT_INTERRUPT(0x58, "Unused");
		INIT_INTERRUPT(0x59, "Unused");
		INIT_INTERRUPT(0x5A, "Unused");
		INIT_INTERRUPT(0x5B, "Unused");
		INIT_INTERRUPT(0x5C, "Unused");
		INIT_INTERRUPT(0x5D, "Unused");
		INIT_INTERRUPT(0x5E, "Unused");
		INIT_INTERRUPT(0x5F, "Unused");

		INIT_INTERRUPT(0x60, "Unused");
		INIT_INTERRUPT(0x61, "Unused");
		INIT_INTERRUPT(0x62, "Unused");
		INIT_INTERRUPT(0x63, "Unused");
		INIT_INTERRUPT(0x64, "Unused");
		INIT_INTERRUPT(0x65, "Unused");
		INIT_INTERRUPT(0x66, "Unused");
		INIT_INTERRUPT(0x67, "Unused");
		INIT_INTERRUPT(0x68, "Unused");
		INIT_INTERRUPT(0x69, "Unused");
		INIT_INTERRUPT(0x6A, "Unused");
		INIT_INTERRUPT(0x6B, "Unused");
		INIT_INTERRUPT(0x6C, "Unused");
		INIT_INTERRUPT(0x6D, "Unused");
		INIT_INTERRUPT(0x6E, "Unused");
		INIT_INTERRUPT(0x6F, "Unused");

		INIT_INTERRUPT(0x70, "Unused");
		INIT_INTERRUPT(0x71, "Unused");
		INIT_INTERRUPT(0x72, "Unused");
		INIT_INTERRUPT(0x73, "Unused");
		INIT_INTERRUPT(0x74, "Unused");
		INIT_INTERRUPT(0x75, "Unused");
		INIT_INTERRUPT(0x76, "Unused");
		INIT_INTERRUPT(0x77, "Unused");
		INIT_INTERRUPT(0x78, "Unused");
		INIT_INTERRUPT(0x79, "Unused");
		INIT_INTERRUPT(0x7A, "Unused");
		INIT_INTERRUPT(0x7B, "Unused");
		INIT_INTERRUPT(0x7C, "Unused");
		INIT_INTERRUPT(0x7D, "Unused");
		INIT_INTERRUPT(0x7E, "Unused");
		INIT_INTERRUPT(0x7F, "Unused");

		INIT_INTERRUPT(0x80, "Syscall");

		// Remaining interrupts
		INIT_INTERRUPT(0x81, "Unused");
		INIT_INTERRUPT(0x82, "Unused");
		INIT_INTERRUPT(0x83, "Unused");
		INIT_INTERRUPT(0x84, "Unused");
		INIT_INTERRUPT(0x85, "Unused");
		INIT_INTERRUPT(0x86, "Unused");
		INIT_INTERRUPT(0x87, "Unused");
		INIT_INTERRUPT(0x88, "Unused");
		INIT_INTERRUPT(0x89, "Unused");
		INIT_INTERRUPT(0x8A, "Unused");
		INIT_INTERRUPT(0x8B, "Unused");
		INIT_INTERRUPT(0x8C, "Unused");
		INIT_INTERRUPT(0x8D, "Unused");
		INIT_INTERRUPT(0x8E, "Unused");
		INIT_INTERRUPT(0x8F, "Unused");

		INIT_INTERRUPT(0x90, "Unused");
		INIT_INTERRUPT(0x91, "Unused");
		INIT_INTERRUPT(0x92, "Unused");
		INIT_INTERRUPT(0x93, "Unused");
		INIT_INTERRUPT(0x94, "Unused");
		INIT_INTERRUPT(0x95, "Unused");
		INIT_INTERRUPT(0x96, "Unused");
		INIT_INTERRUPT(0x97, "Unused");
		INIT_INTERRUPT(0x98, "Unused");
		INIT_INTERRUPT(0x99, "Unused");
		INIT_INTERRUPT(0x9A, "Unused");
		INIT_INTERRUPT(0x9B, "Unused");
		INIT_INTERRUPT(0x9C, "Unused");
		INIT_INTERRUPT(0x9D, "Unused");
		INIT_INTERRUPT(0x9E, "Unused");
		INIT_INTERRUPT(0x9F, "Unused");

		INIT_INTERRUPT(0xA0, "Unused");
		INIT_INTERRUPT(0xA1, "Unused");
		INIT_INTERRUPT(0xA2, "Unused");
		INIT_INTERRUPT(0xA3, "Unused");
		INIT_INTERRUPT(0xA4, "Unused");
		INIT_INTERRUPT(0xA5, "Unused");
		INIT_INTERRUPT(0xA6, "Unused");
		INIT_INTERRUPT(0xA7, "Unused");
		INIT_INTERRUPT(0xA8, "Unused");
		INIT_INTERRUPT(0xA9, "Unused");
		INIT_INTERRUPT(0xAA, "Unused");
		INIT_INTERRUPT(0xAB, "Unused");
		INIT_INTERRUPT(0xAC, "Unused");
		INIT_INTERRUPT(0xAD, "Unused");
		INIT_INTERRUPT(0xAE, "Unused");
		INIT_INTERRUPT(0xAF, "Unused");

		INIT_INTERRUPT(0xB0, "Unused");
		INIT_INTERRUPT(0xB1, "Unused");
		INIT_INTERRUPT(0xB2, "Unused");
		INIT_INTERRUPT(0xB3, "Unused");
		INIT_INTERRUPT(0xB4, "Unused");
		INIT_INTERRUPT(0xB5, "Unused");
		INIT_INTERRUPT(0xB6, "Unused");
		INIT_INTERRUPT(0xB7, "Unused");
		INIT_INTERRUPT(0xB8, "Unused");
		INIT_INTERRUPT(0xB9, "Unused");
		INIT_INTERRUPT(0xBA, "Unused");
		INIT_INTERRUPT(0xBB, "Unused");
		INIT_INTERRUPT(0xBC, "Unused");
		INIT_INTERRUPT(0xBD, "Unused");
		INIT_INTERRUPT(0xBE, "Unused");
		INIT_INTERRUPT(0xBF, "Unused");

		INIT_INTERRUPT(0xC0, "Unused");
		INIT_INTERRUPT(0xC1, "Unused");
		INIT_INTERRUPT(0xC2, "Unused");
		INIT_INTERRUPT(0xC3, "Unused");
		INIT_INTERRUPT(0xC4, "Unused");
		INIT_INTERRUPT(0xC5, "Unused");
		INIT_INTERRUPT(0xC6, "Unused");
		INIT_INTERRUPT(0xC7, "Unused");
		INIT_INTERRUPT(0xC8, "Unused");
		INIT_INTERRUPT(0xC9, "Unused");
		INIT_INTERRUPT(0xCA, "Unused");
		INIT_INTERRUPT(0xCB, "Unused");
		INIT_INTERRUPT(0xCC, "Unused");
		INIT_INTERRUPT(0xCD, "Unused");
		INIT_INTERRUPT(0xCE, "Unused");
		INIT_INTERRUPT(0xCF, "Unused");

		INIT_INTERRUPT(0xD0, "Unused");
		INIT_INTERRUPT(0xD1, "Unused");
		INIT_INTERRUPT(0xD2, "Unused");
		INIT_INTERRUPT(0xD3, "Unused");
		INIT_INTERRUPT(0xD4, "Unused");
		INIT_INTERRUPT(0xD5, "Unused");
		INIT_INTERRUPT(0xD6, "Unused");
		INIT_INTERRUPT(0xD7, "Unused");
		INIT_INTERRUPT(0xD8, "Unused");
		INIT_INTERRUPT(0xD9, "Unused");
		INIT_INTERRUPT(0xDA, "Unused");
		INIT_INTERRUPT(0xDB, "Unused");
		INIT_INTERRUPT(0xDC, "Unused");
		INIT_INTERRUPT(0xDD, "Unused");
		INIT_INTERRUPT(0xDE, "Unused");
		INIT_INTERRUPT(0xDF, "Unused");

		INIT_INTERRUPT(0xE0, "Unused");
		INIT_INTERRUPT(0xE1, "Unused");
		INIT_INTERRUPT(0xE2, "Unused");
		INIT_INTERRUPT(0xE3, "Unused");
		INIT_INTERRUPT(0xE4, "Unused");
		INIT_INTERRUPT(0xE5, "Unused");
		INIT_INTERRUPT(0xE6, "Unused");
		INIT_INTERRUPT(0xE7, "Unused");
		INIT_INTERRUPT(0xE8, "Unused");
		INIT_INTERRUPT(0xE9, "Unused");
		INIT_INTERRUPT(0xEA, "Unused");
		INIT_INTERRUPT(0xEB, "Unused");
		INIT_INTERRUPT(0xEC, "Unused");
		INIT_INTERRUPT(0xED, "Unused");
		INIT_INTERRUPT(0xEE, "Unused");
		INIT_INTERRUPT(0xEF, "Unused");

		INIT_INTERRUPT(0xF0, "Unused");
		INIT_INTERRUPT(0xF1, "Unused");
		INIT_INTERRUPT(0xF2, "Unused");
		INIT_INTERRUPT(0xF3, "Unused");
		INIT_INTERRUPT(0xF4, "Unused");
		INIT_INTERRUPT(0xF5, "Unused");
		INIT_INTERRUPT(0xF6, "Unused");
		INIT_INTERRUPT(0xF7, "Unused");
		INIT_INTERRUPT(0xF8, "Unused");
		INIT_INTERRUPT(0xF9, "Unused");
		INIT_INTERRUPT(0xFA, "Unused");
		INIT_INTERRUPT(0xFB, "Unused");
		INIT_INTERRUPT(0xFC, "Unused");
		INIT_INTERRUPT(0xFD, "Unused");
		INIT_INTERRUPT(0xFE, "Unused");
		INIT_INTERRUPT(0xFF, "Unused");

		idtr.base = (uint32_t)idt;
		idtr.limit = sizeof(idt);

		asm volatile("lidt %0" ::"m"(idtr)
		             : "memory");
	}

	irq_descriptor_t &get_irq(int id)
	{
		assert(id > 0 && id < MAX_INTERRUPTS);
		return irqs[id];
	}

	bool register_irq(int id, irqaction_t action)
	{
		if (id < 0 || id >= MAX_INTERRUPTS)
			return false;

		irqs[id].actions.push_back(action);
		return true;
	}
} // namespace Kernel::Processor