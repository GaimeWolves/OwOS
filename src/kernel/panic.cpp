#include <panic.hpp>

#include <common_attributes.h>

#include <libk/kcstdarg.hpp>
#include <libk/katomic.hpp>
#include <libk/kshared_ptr.hpp>

#include <arch/Processor.hpp>
#include <arch/stack_tracing.hpp>
#include <logging/logger.hpp>

namespace Kernel
{
	class PanicMessage final : public CPU::ProcessorMessage
	{
	public:
		void handle() override
		{
			CPU::Processor::current().enter_critical();
			for (;;)
				CPU::Processor::halt();
		}
	};

	static LibK::atomic_bool s_is_panicking = false;
	static PanicMessage s_panic_message = PanicMessage();

	__noreturn void panic()
	{
		panic(nullptr);
	}

	__noreturn void panic(const char *fmt, ...)
	{
		CPU::Processor::current().enter_critical();
		if (s_is_panicking)
		{
			for (;;)
				CPU::Processor::halt();
		}

		s_is_panicking = true;
		CPU::Processor::current().smp_broadcast(LibK::shared_ptr<CPU::ProcessorMessage>(&s_panic_message), true);

		critical_empty_logger();
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