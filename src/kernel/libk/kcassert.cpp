#include <libk/kcassert.hpp>

#include <arch/Processor.hpp>
#include <arch/stack_tracing.hpp>
#include <common_attributes.h>
#include <libk/kcstdio.hpp>

__noreturn void __assertion_failed(const char *condition, const char *file, unsigned line, const char *function)
{
	Kernel::CPU::Processor::current().disable_interrupts();
	kprintf("%s:%d %s: Assertion '%s' failed.\n", file, line, function, condition);

	Kernel::CPU::print_stacktrace();

	Kernel::CPU::Processor::halt();
}