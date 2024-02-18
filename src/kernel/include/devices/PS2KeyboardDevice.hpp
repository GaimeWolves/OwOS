#pragma once

#include <interrupts/IRQHandler.hpp>
#include <devices/KeyboardDevice.hpp>
#include <devices/KeyCode.hpp>

#include <arch/io.hpp>

namespace Kernel
{
	class PS2KeyboardDevice : public KeyboardDevice, private Interrupts::IRQHandler
	{
	public:
		PS2KeyboardDevice(size_t major, size_t minor)
		    : KeyboardDevice(major, minor)
		    , Interrupts::IRQHandler(1)
		{}

		void handle_interrupt(const CPU::interrupt_frame_t &regs) override;

		void enable();

	private:
		void parse_modifiers(bool released, KeyCode key);

		IO::Port m_input_buffer_port{0x60};
		bool m_e0_prefix{false};
		uint8_t m_modifiers{0};
	};
}
