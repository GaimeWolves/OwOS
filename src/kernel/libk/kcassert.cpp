#include <libk/kcassert.hpp>

#include <arch/processor.hpp>
#include <common_attributes.h>
#include <libk/kcstdio.hpp>

__noreturn void __assertion_failed(const char *condition, const char *file, unsigned line, const char *function)
{
	Kernel::Processor::clear_interrupts();
	kprintf("%s:%d %s: Assertion '%s' failed.\n", file, line, function, condition);
	// TODO: Print backtrace when we have it available
	Kernel::Processor::halt();
}