#include <libk/kcassert.hpp>

#include <arch/Processor.hpp>
#include <arch/stack_tracing.hpp>
#include <common_attributes.h>
#include <libk/kcstdio.hpp>

__noreturn void __assertion_failed(const char *condition, const char *file, unsigned line, const char *function)
{
	Kernel::CPU::Processor::current().disable_interrupts();
	kprintf("(#%d) %s:%d %s: Assertion '%s' failed.\n", Kernel::CPU::Processor::current().id(), file, line, function, condition);
	Kernel::panic();
}