#pragma once

#include <stdint.h>

#include <interrupts/forward.hpp>

namespace Kernel::Interrupts
{
	class InterruptController
	{
	public:
		virtual void enable() = 0;
		virtual void disable() = 0;

		virtual bool eoi(IRQHandler &handler) = 0;

		virtual bool enable_interrupt(IRQHandler &handler) = 0;
		virtual bool disable_interrupt(IRQHandler &handler) = 0;

		[[nodiscard]] virtual uint32_t get_gsi_base() const = 0;
		[[nodiscard]] virtual uint32_t get_gsi_count() const = 0;
	};
} // namespace Kernel::Interrupts
