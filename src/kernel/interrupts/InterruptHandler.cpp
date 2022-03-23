#include <interrupts/InterruptHandler.hpp>

#include <arch/interrupts.hpp>
#include <arch/Processor.hpp>

#include <panic.hpp>

namespace Kernel::Interrupts
{
	InterruptHandler::InterruptHandler(uint32_t interrupt_number)
	    : m_interrupt_number(interrupt_number)
	{
		assert(interrupt_number < CPU::MAX_INTERRUPTS);
	}

	bool InterruptHandler::is_registered() const
	{
		CPU::Processor &core = CPU::Processor::current();
		return (m_is_registered >> core.id()) & 1;
	}

	void InterruptHandler::register_handler()
	{
		CPU::Processor &core = CPU::Processor::current();

		if (m_is_registered & 1 << core.id())
			return;

		core.register_interrupt_handler(*this);
		m_is_registered |= 1 << core.id();
	}

	void InterruptHandler::unregister_handler()
	{
		CPU::Processor &core = CPU::Processor::current();

		if (!(m_is_registered & 1 << core.id()))
			return;

		core.unregister_interrupt_handler(*this);
		m_is_registered &= ~(1 << core.id());
	}
} // namespace Kernel::Interrupts
