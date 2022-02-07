#pragma once

#include <arch/interrupts.hpp>

namespace Kernel::Interrupts
{
	class InterruptHandler
	{
	public:
		enum class Type
		{
			GenericInterrupt,
			IRQHandler,
			UnhandledInterrupt,
			SpuriousInterrupt,
		};

		virtual ~InterruptHandler() = default;

		void register_handler();
		void unregister_handler();

		virtual void handle_interrupt(const Processor::registers_t &regs) = 0;
		virtual void eoi() = 0;

		[[nodiscard]] uint32_t interrupt_number() const { return m_interrupt_number; }

		[[nodiscard]] virtual Type type() const = 0;

	protected:
		explicit InterruptHandler(uint32_t interrupt_number);

		[[nodiscard]] bool is_registered() const { return m_is_registered; }

	private:
		uint32_t m_interrupt_number{0};
		bool m_is_registered{false};
	};
} // namespace Kernel::Interrupts
