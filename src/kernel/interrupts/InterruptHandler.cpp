#include <interrupts/InterruptHandler.hpp>

#include <arch/interrupts.hpp>
#include <arch/processor.hpp>

#include <panic.hpp>

namespace Kernel::Interrupts
{
	InterruptHandler::InterruptHandler(uint32_t interrupt_number)
	    : m_interrupt_number(interrupt_number)
	{
		assert(interrupt_number < Processor::MAX_INTERRUPTS);
	}

	void InterruptHandler::register_handler()
	{
		if (m_is_registered)
			return;

		// TODO: Save and restore interrupt enable state
		//Processor::clear_interrupts();

		m_is_registered = Processor::register_interrupt(this);
		assert(m_is_registered);

		//Processor::enable_interrupts();
	}

	void InterruptHandler::unregister_handler()
	{
		if (!m_is_registered)
			return;

		//Processor::clear_interrupts();

		m_is_registered = !Processor::unregister_interrupt(this);
		assert(!m_is_registered);

		//Processor::enable_interrupts();
	}
} // namespace Kernel::Interrupts
