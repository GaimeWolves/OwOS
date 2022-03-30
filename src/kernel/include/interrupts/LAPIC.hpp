#pragma once

#include <stdint.h>
#include <limits.h>

#include <time/Timer.hpp>
#include <interrupts/IRQHandler.hpp>

#define APIC_TIMER_IRQ (CPU::MAX_INTERRUPTS - 4)

namespace Kernel::Interrupts
{
	class APICErrorInterruptHandler;
	class APICSpuriousInterruptHandler;

	class APICTimer : public Time::Timer, private InterruptHandler
	{
	public:
		static APICTimer &instance()
		{
			static APICTimer *instance{nullptr};

			if (!instance)
				instance = new APICTimer();

			return *instance;
		}

		void initialize();

		void start(uint64_t interval) override;
		void stop() override;

		[[nodiscard]] uint64_t get_time_quantum_in_ns() const override { return m_time_quantum; };
		[[nodiscard]] uint64_t get_maximum_interval() const override { return UINT32_MAX; };
		[[nodiscard]] Time::TimerType timer_type() const override { return Time::TimerType::CPU; };

		void eoi() override;

		[[nodiscard]] InterruptType type() const override { return InterruptType::GenericInterrupt; }

	private:
		APICTimer()
		    : InterruptHandler(APIC_TIMER_IRQ)
		{}

		~APICTimer() override = default;

		void handle_interrupt(const CPU::interrupt_frame_t &regs __unused) override;

		uint64_t m_time_quantum{};
	};

	class LAPIC final
	{
		friend APICTimer;
	public:
		static LAPIC &instance()
		{
			static LAPIC *instance{nullptr};

			if (!instance)
				instance = new LAPIC();

			return *instance;
		}

		void initialize(uintptr_t physical_addr);
		void initialize_ap();
		void enable();

		void eoi();

		void send_ipi(uint8_t vector, uint8_t core);
		void broadcast_ipi(uint8_t vector, bool excluding_self);

		void start_smp_boot();
		void set_available_ap_count(uint32_t count) { m_available_aps = count; }

		[[nodiscard]] uint32_t get_ap_id();
		void set_ap_id(uint32_t id);

	private:
		LAPIC() = default;
		~LAPIC() = default;

		void write_register(uintptr_t offset, uint32_t value);
		uint32_t read_register(uintptr_t offset);

		void write_icr(uint8_t vector, uint8_t delivery_mode, uint8_t destination_mode, uint8_t shorthand, uint8_t destination);

		uintptr_t m_physical_addr{0};
		uintptr_t m_virtual_addr{0};

		APICErrorInterruptHandler *m_error_interrupt_handler{nullptr};
		APICSpuriousInterruptHandler *m_spurious_interrupt_handler{nullptr};

		uint32_t m_available_aps{0};
	};
} // namespace Kernel::Interrupts
