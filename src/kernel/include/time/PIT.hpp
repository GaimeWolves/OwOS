#pragma once

#include <time/Timer.hpp>
#include <interrupts/IRQHandler.hpp>
#include <arch/io.hpp>

namespace Kernel::Time
{
	class PIT final : public Timer, private Interrupts::IRQHandler
	{
	public:
		static PIT &instance()
		{
			static PIT *instance{nullptr};

			if (!instance)
				instance = new PIT();

			return *instance;
		}

		void start(uint64_t interval) override;
		void stop() override;

		// INFO: Nominal frequency of the PIT = 1193182 Hz
		//       Time quantum in nanoseconds = (1 / frequency) * 1e-9 = 1e9 / frequency
		[[nodiscard]] uint64_t get_time_quantum_in_ns() const override { return (1e9 / 1193182); };
		[[nodiscard]] uint64_t get_maximum_interval() const override { return 65536; }
		[[nodiscard]] TimerType timer_type() const override { return TimerType::Global; }

	private:
		PIT()
			: Interrupts::IRQHandler(0)
		{}

		~PIT() override = default;

		void handle_interrupt(const CPU::interrupt_frame_t &regs) override;

		uint32_t m_current_interval{0};

		IO::Port m_channel_0_data{0x40};
		IO::Port m_channel_1_data{0x41};
		IO::Port m_channel_2_data{0x42};
		IO::Port m_command_register{0x43};
	};
}