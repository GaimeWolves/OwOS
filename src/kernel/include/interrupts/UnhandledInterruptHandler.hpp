#pragma once

#include <common_attributes.h>
#include <interrupts/InterruptHandler.hpp>
#include <panic.hpp>

namespace Kernel::Interrupts
{
	class UnhandledInterruptHandler final : public InterruptHandler
	{
	public:
		explicit UnhandledInterruptHandler(uint32_t interrupt_number)
		    : InterruptHandler(interrupt_number)
		{
		}

		~UnhandledInterruptHandler() override = default;

		void handle_interrupt(const Processor::registers_t &reg __unused) override
		{
			panic("Unhandled interrupt handler called [%d]", interrupt_number());
		}

		void eoi() override
		{
			panic("Unhandled interrupt eoi called [%d]", interrupt_number());
		}

		[[nodiscard]] Type type() const override { return Type::UnhandledInterrupt; }
	};
} // namespace Kernel::Interrupts
