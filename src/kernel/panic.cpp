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
	static Locking::Spinlock s_trace_lock;

	class PanicMessage final : public CPU::ProcessorMessage
	{
	public:
		void handle() override
		{
			CPU::Processor::current().enter_critical();

			s_trace_lock.lock();

			critical_printf("\n\nStacktrace (Core #%d):\n", CPU::Processor::current().id());
			CPU::print_stacktrace();

			s_trace_lock.unlock();

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

		s_trace_lock.lock();

		critical_empty_logger();
		critical_printf("KERNEL PANIC (Core #%d): ", CPU::Processor::current().id());

		if (fmt)
		{
			va_list ap;
			va_start(ap, fmt);
			critical_vprintf(fmt, ap);
			va_end(ap);
		}

		critical_printf("\n\nRegisters:\n", CPU::Processor::current().id());
		CPU::print_registers();

		critical_printf("\nStacktrace (Core #%d):\n", CPU::Processor::current().id());
		CPU::print_stacktrace();

		s_trace_lock.unlock();

		for (;;)
			CPU::Processor::halt();
	}
} // namespace Kernel
