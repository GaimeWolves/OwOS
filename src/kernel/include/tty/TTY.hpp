#pragma once

#include "../../../userland/libc/sys/types.h"
#include "../devices/CharacterDevice.hpp"
#include "../memory/definitions.hpp"

#include <stddef.h>

#include <libk/CircularBuffer.hpp>

#include <memory/definitions.hpp>
#include <devices/CharacterDevice.hpp>
#include <devices/KeyboardDevice.hpp>
#include <devices/PS2KeyboardDevice.hpp>

namespace Kernel
{
	class TTY : public CharacterDevice
	{
	public:
		static TTY *get_tty() { return &s_tty; }
		static void initialize()
		{
			s_tty.m_keyboard = new PS2KeyboardDevice(0, 0);
			s_tty.m_keyboard->set_listener(&s_tty);
			reinterpret_cast<PS2KeyboardDevice *>(s_tty.m_keyboard)->enable();
		}

		size_t read(size_t, size_t bytes, Memory::memory_region_t region) override;
		size_t write(size_t, size_t bytes, Memory::memory_region_t region) override;

		LibK::StringView name() override { return LibK::StringView(m_name); };
		[[nodiscard]] size_t size() override { return 0; }

		void on_key_event(key_event_t event);

	protected:
		[[nodiscard]] bool can_open_for_read() const override { return true; };
		[[nodiscard]] bool can_open_for_write() const override { return true; };

	private:
		TTY(size_t major, size_t minor)
		    : CharacterDevice(major, minor)
			, m_input_buffer(256)
		{}

		void advance();

		LibK::string m_name{"tty0"};
		KeyboardDevice *m_keyboard{nullptr};
		LibK::CircularBuffer<char> m_input_buffer;
		Locking::Spinlock m_input_lock{};

		// TODO: abstract echoing into subclass
		size_t m_row = 0;
		size_t m_column = 0;

		// TODO: Only one tty for now
		static TTY s_tty;
	};
}