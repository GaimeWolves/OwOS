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

	void InterruptHandler::register_handler()
	{
		if (m_is_registered)
			return;

		CPU::Processor::current().register_interrupt_handler(*this);
		m_is_registered = true;
	}

	void InterruptHandler::unregister_handler()
	{
		if (!m_is_registered)
			return;

		CPU::Processor::current().unregister_interrupt_handler(*this);
		m_is_registered = false;
	}
} // namespace Kernel::Interrupts
