#include <panic.hpp>

#include <common_attributes.h>

#include <libk/kcstdarg.hpp>
#include <libk/kcstdio.hpp>

#include <arch/processor.hpp>

namespace Kernel
{
	__noreturn void panic()
	{
		panic(nullptr);
	}

	__noreturn void panic(const char *fmt, ...)
	{
		kputs("KERNEL PANIC: ");

		if (fmt)
		{
			va_list ap;
			va_start(ap, fmt);
			kvprintf(fmt, ap);
			va_end(ap);
		}

		kputc('\n');

		// TODO: print backtrace

		for (;;)
			Processor::halt();
	}
} // namespace Kernel