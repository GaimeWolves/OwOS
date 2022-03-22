#pragma once

#include <arch/interrupts.hpp>

namespace Kernel::Interrupts
{
	enum class InterruptType
	{
		GenericInterrupt,
		IRQHandler,
		UnhandledInterrupt,
		SpuriousInterrupt,
	};

	class InterruptHandler
	{
	public:
		virtual ~InterruptHandler() = default;

		void register_handler();
		void unregister_handler();

		virtual void handle_interrupt(const CPU::registers_t &regs) = 0;
		virtual void eoi() = 0;

		[[nodiscard]] uint32_t interrupt_number() const { return m_interrupt_number; }

		[[nodiscard]] virtual InterruptType type() const = 0;

	protected:
		explicit InterruptHandler(uint32_t interrupt_number);

		[[nodiscard]] bool is_registered() const { return m_is_registered; }

	private:
		uint32_t m_interrupt_number{0};
		bool m_is_registered{false};
	};
} // namespace Kernel::Interrupts
