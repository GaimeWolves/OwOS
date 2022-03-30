#pragma once

#include <stdint.h>

#include <interrupts/InterruptHandler.hpp>
#include <interrupts/InterruptController.hpp>
#include <interrupts/InterruptManager.hpp>

namespace Kernel::Interrupts
{
	class IRQHandler : public InterruptHandler
	{
	public:
		explicit IRQHandler(uint32_t interrupt_number)
			: InterruptHandler(InterruptManager::instance().get_mapped_interrupt_number(interrupt_number))
		    , m_original_irq_number(interrupt_number)
		{
			m_interrupt_controller = InterruptManager::instance().get_responsible_controller(*this);
			assert(m_interrupt_controller);
		}

		void handle_interrupt(const CPU::interrupt_frame_t &regs) override = 0;

		void eoi() override
		{
			m_interrupt_controller->eoi(*this);
		}

		virtual void enable_irq()
		{
			// TODO: Handle shared IRQ enabling and disabling better

			if (!is_registered())
				register_handler();

			if (!m_enabled)
				m_enabled = m_interrupt_controller->enable_interrupt(*this);

			assert(m_enabled);
		}

		virtual void disable_irq()
		{
			if (is_registered())
				unregister_handler();

			if (m_enabled)
				m_enabled = !m_interrupt_controller->disable_interrupt(*this);

			assert(!m_enabled);
		}

		[[nodiscard]] InterruptType type() const override { return InterruptType::IRQHandler; }
		[[nodiscard]] bool is_enabled() const { return m_enabled; }
		[[nodiscard]] uint32_t original_interrupt_number() const { return m_original_irq_number; }

	protected:
		InterruptController *m_interrupt_controller{nullptr};
		uint32_t m_original_irq_number{0};
		bool m_enabled{false};
	};
}