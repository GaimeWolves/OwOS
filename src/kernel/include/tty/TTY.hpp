#pragma once

#include "../../userland/libc/termios.h"
#include "../devices/CharacterDevice.hpp"
#include "../memory/definitions.hpp"

#include <stddef.h>

#include <libk/srmw_queue.hpp>

#include <memory/definitions.hpp>
#include <devices/CharacterDevice.hpp>
#include <devices/KeyboardDevice.hpp>
#include <devices/PS2KeyboardDevice.hpp>

namespace Kernel
{
	class TTY : public CharacterDevice
	{
	public:
		size_t read(size_t, size_t bytes, char *buffer) override;
		size_t write(size_t, size_t bytes, char *buffer) override;

		LibK::ErrorOr<void> ioctl(uint32_t request, uintptr_t arg) override;

		[[nodiscard]] size_t size() override { return 0; }

		[[nodiscard]] struct termios get_termios() const { return m_termios; }
		void set_termios(struct termios new_termios);

	protected:
		TTY(size_t major, size_t minor)
		    : CharacterDevice(major, minor)
		    , m_input_char_buffer(4096)
		{
			reset_termios();
		}

		[[nodiscard]] bool can_open_for_read() const override { return true; };
		[[nodiscard]] bool can_open_for_write() const override { return true; };

		virtual void echo(char ch) = 0;
		virtual void unecho() = 0;
		void emit(char ch);

	private:
		size_t canonical_read(size_t bytes, char *buffer);
		size_t non_canonical_read(size_t bytes, char *buffer);

		void canonical_input(char ch);
		void non_canonical_input(char ch);

		void reset_termios();
		void switch_buffers(struct termios old_termios);

		void empty_line_buffer();
		void empty_char_buffer();

		void process_echo(char ch);

		LibK::string m_name{};

		// Canonical mode processing
		LibK::string m_current_input_line{};
		LibK::string m_current_read_line{};
		LibK::SRMWQueue<LibK::string> m_input_line_buffer{};

		// Non-canonical mode processing
		LibK::CircularBuffer<char> m_input_char_buffer;

		struct termios m_termios{};
	};
}