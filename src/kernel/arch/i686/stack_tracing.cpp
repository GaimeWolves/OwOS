#include <arch/stack_tracing.hpp>

#include <libk/kcstdio.hpp>
#include <libk/kcstring.hpp>

#include <arch/i686/interrupts.hpp>
#include <common_attributes.h>

namespace Kernel::CPU
{
	// NOTE: Stack tracing past interrupts:
	//       To trace past interrupt boundaries check for the function "common_interrupt_handler"
	//       it holds an argument "registers_t *regs" which holds the eip and ebp of the function that
	//       caused the interrupt. Continue tracing using these values.

	typedef struct stackframe_t
	{
		struct stackframe_t *next;
		uintptr_t eip;
		registers_t *regs; // NOTE: Only used in interrupts
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
				kprintf("#%d %p (unknown)\n", current_index, stackframe->eip - 5);
			else if (symbol.file == nullptr)
				kprintf("#%d %p in %s\n", current_index, stackframe->eip - 5, symbol.name);
			else
				kprintf("#%d %p in %s at %s\n", current_index, stackframe->eip - 5, symbol.name, symbol.file);

			current_index++;

			if (strcmp(symbol.name, "common_interrupt_handler") == 0)
			{
				kprintf("    <---- Interrupt 0x%.2x ---->\n", stackframe->regs->isr_number);
				stackframe = (stackframe_t *)stackframe->regs->ebp;
				continue;
			}

			stackframe = stackframe->next;
		}
	}
}