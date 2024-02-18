#pragma once

#include <stdint.h>

#include <common_attributes.h>
#include <arch/interrupts.hpp>

namespace Kernel
{
	typedef struct thread_registers_t
	{
		uint32_t cs;
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
		uint32_t eflags;
		uint32_t cr3;
		uint32_t eip;
		CPU::interrupt_frame_t *frame;
	} __packed thread_registers_t;
}
