#pragma once

#include <libk/kvector.hpp>

#include <interrupts/IRQHandler.hpp>

namespace Kernel::Interrupts
{
	class SharedIRQHandler final : public IRQHandler
	{
	public:
		explicit SharedIRQHandler(uint32_t interrupt_number)
		    : IRQHandler(interrupt_number)
		{
		}

		void handle_interrupt(const CPU::registers_t &regs) override
		{
			for (auto handler : m_handlers)
			{
				if (handler->is_enabled())
					handler->handle_interrupt(regs);
			}
		}

		void eoi() override
		{
			m_interrupt_controller->eoi(*this);
		}

		void add_handler(IRQHandler *handler)
		{
			// Handlers should never be added twice
			for (auto other : m_handlers)
				assert(other != handler);

			m_handlers.push_back(handler);
		}

		void remove_handler(IRQHandler *handler)
		{
			for (auto it = m_handlers.begin(); it != m_handlers.end(); ++it)
			{
				if (*it == handler)
				{
					m_handlers.erase(it);
					return;
				}
			}

			// Handlers should never be mistakenly deleted
			assert(false);
		}

		[[nodiscard]] bool empty() const { return m_handlers.empty(); }

	private:
		LibK::vector<IRQHandler *> m_handlers;
	};
}