#include <panic.hpp>

#include <common_attributes.h>

#include <libk/kcstdarg.hpp>

#include <arch/Processor.hpp>
#include <arch/stack_tracing.hpp>
#include <logging/logger.hpp>

namespace Kernel
{
	__noreturn void panic()
	{
		panic(nullptr);
	}

	__noreturn void panic(const char *fmt, ...)
	{
		critical_puts("KERNEL PANIC: ");

		if (fmt)
		{
			va_list ap;
			va_start(ap, fmt);
			critical_vprintf(fmt, ap);
			va_end(ap);
		}

		critical_putc('\n');

		CPU::print_stacktrace();

		for (;;)
			CPU::Processor::halt();
	}
} // namespace Kernel