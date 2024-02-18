#include <arch/stack_tracing.hpp>

#include <libk/kcstring.hpp>

#include <arch/i686/interrupts.hpp>
#include <common_attributes.h>
#include <logging/logger.hpp>

extern "C"
{
	extern uintptr_t _virtual_addr;
}

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

	void print_stacktrace(uintptr_t starting_frame)
	{
		stackframe_t *stackframe = reinterpret_cast<stackframe_t *>(starting_frame);

		if (!starting_frame)
		{
			asm volatile("mov %%ebp, %%eax"
			             : "=a"(stackframe));
		}

		bool got_timer_interrupt = false;

		int current_index = 0;
		while (stackframe)
		{
			uintptr_t address = stackframe->eip - 5;

			if (address < (uintptr_t)&_virtual_addr)
			{
				critical_printf("#%d %p (unknown)\n", current_index, address);
				break;
			}

			symbol_t symbol = get_symbol_by_address(address); // subtract size of call instruction

			if (symbol.address == 0)
			{
				critical_printf("#%d %p (unknown)\n", current_index,address);
				break;
			}
			else if (symbol.file == nullptr)
				critical_printf("#%d %p in %s\n", current_index, address, symbol.name);
			else
				critical_printf("#%d %p in %s at %s\n", current_index, address, symbol.name, symbol.file);

			current_index++;

			if (strcmp(symbol.name, "common_interrupt_handler_entry") == 0)
			{
				// TODO: Investigate, why faults have a different stack layout (I thought I already mitigated this)
				interrupt_frame_t *regs = (uintptr_t) stackframe->regs_fault > 0xc0000000 ? stackframe->regs_fault : stackframe->regs;

				if (got_timer_interrupt && regs->isr_number == 0xfc)
					break ;

				critical_printf("    <---- Interrupt 0x%.2x ---->\n", regs->isr_number);

				if (regs->isr_number == 0xfc)
					got_timer_interrupt = true;

				if (regs->eip < (uintptr_t)&_virtual_addr)
				{
					critical_printf("#%d %p (unknown)\n", current_index, regs->eip);
					break;
				}

				symbol_t interrupted_symbol = get_symbol_by_address(regs->eip);
				critical_printf("#%d %p in %s at %s\n", current_index++, regs->eip, interrupted_symbol.name, interrupted_symbol.file);

				stackframe = (stackframe_t *)regs->ebp;
				continue;
			}

			stackframe = stackframe->next;
		}
	}
}
