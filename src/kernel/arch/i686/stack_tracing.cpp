#include <arch/stack_tracing.hpp>

#include <libk/kcstdio.hpp>
#include <libk/kcstring.hpp>

#include <arch/i686/interrupts.hpp>
#include <common_attributes.h>

namespace Kernel::CPU
{
	// NOTE: Stack tracing past interrupts:
	//       To trace past interrupt boundaries check for the function "common_interrupt_handler"
	//       it holds an argument "interrupt_frame_t *regs" which holds the eip and ebp of the function that
	//       caused the interrupt. Continue tracing using these values.

	typedef struct stackframe_t
	{
		struct stackframe_t *next;
		uintptr_t eip;
		interrupt_frame_t *regs; // NOTE: Only used in interrupts
		interrupt_frame_t *regs_fault; // NOTE: Only used in interrupts
	} __packed stackframe_t;

	void print_stacktrace()
	{
		stackframe_t *stackframe = nullptr;
		asm volatile("mov %%ebp, %%eax"
		             : "=a"(stackframe));

		int current_index = 0;
		while (stackframe)
		{
			symbol_t symbol = get_symbol_by_address(stackframe->eip - 5); // subtract size of call instruction

			if (symbol.address == 0)
			{
				kprintf("#%d %p (unknown)\n", current_index, stackframe->eip - 5);
				return;
			}
			else if (symbol.file == nullptr)
				kprintf("#%d %p in %s\n", current_index, stackframe->eip - 5, symbol.name);
			else
				kprintf("#%d %p in %s at %s\n", current_index, stackframe->eip - 5, symbol.name, symbol.file);

			current_index++;

			if (strcmp(symbol.name, "common_interrupt_handler") == 0)
			{
				// TODO: Investigate, why faults have a different stack layout (I thought I already mitigated this)
				interrupt_frame_t *regs = (uintptr_t) stackframe->regs_fault > 0xc0000000 ? stackframe->regs_fault : stackframe->regs;

				kprintf("    <---- Interrupt 0x%.2x ---->\n", regs->isr_number);

				symbol_t interrupted_symbol = get_symbol_by_address(regs->eip);
				kprintf("#%d %p in %s at %s\n", current_index++, regs->eip, interrupted_symbol.name, interrupted_symbol.file);

				stackframe = (stackframe_t *)regs->ebp;
				continue;
			}

			stackframe = stackframe->next;
		}
	}
}