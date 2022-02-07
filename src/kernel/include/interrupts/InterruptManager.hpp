#pragma once

#include <libk/kvector.hpp>

#include <interrupts/InterruptController.hpp>
#include <interrupts/forward.hpp>

namespace Kernel::Interrupts
{
	class InterruptManager
	{
	public:
		static InterruptManager &instance()
		{
			static InterruptManager *instance{nullptr};

			if (!instance)
				instance = new InterruptManager();

			return *instance;
		}

		void initialize();

		InterruptController *get_responsible_controller(IRQHandler &handler) const;
		uint32_t get_mapped_interrupt_number(uint32_t interrupt_number);

	private:
		InterruptManager() = default;
		~InterruptManager() = default;

		void parse_madt();

		LibK::vector<InterruptController *> m_controllers;
		LibK::vector<InterruptController *> m_active_controllers;
	};
} // namespace Kernel::Interrupts
