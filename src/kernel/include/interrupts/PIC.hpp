#pragma once

#include <stddef.h>
#include <stdint.h>

#include <arch/io.hpp>
#include <common_attributes.h>
#include <interrupts/InterruptController.hpp>
#include <interrupts/IRQHandler.hpp>

namespace Kernel::Interrupts
{
	class PIC final : public InterruptController
	{
	public:
		PIC();

		void enable() override;
		void disable() override;

		bool eoi(IRQHandler &handler) override;

		bool enable_interrupt(IRQHandler &handler) override;
		bool disable_interrupt(IRQHandler &handler) override;

		[[nodiscard]] uint32_t get_gsi_base() const override { return 0; }
		[[nodiscard]] uint32_t get_gsi_count() const override { return 16; }

	private:
		enum Controller
		{
			Master,
			Slave,
		};

		always_inline void send_command(uint8_t cmd, Controller controller)
		{
			if (controller == Master)
				m_master_command.out(cmd);
			else
				m_slave_command.out(cmd);
		}

		always_inline void send_data(uint8_t data, Controller controller)
		{
			if (controller == Master)
				m_master_data.out(data);
			else
				m_slave_data.out(data);
		}

		always_inline uint8_t read_data(Controller controller)
		{
			if (controller == Master)
				return m_master_data.in<uint8_t>();
			else
				return m_slave_data.in<uint8_t>();
		}

		IO::Port m_master_command{0x20};
		IO::Port m_master_data{0x21};

		IO::Port m_slave_command{0xA0};
		IO::Port m_slave_data{0xA1};

		uint16_t m_interrupt_mask{0xFFFF};
	};
} // namespace Kernel::Interrupts
