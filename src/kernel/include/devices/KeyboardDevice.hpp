#pragma once

#include <stdint.h>

#include <libk/CircularBuffer.hpp>

#include <devices/CharacterDevice.hpp>
#include <arch/spinlock.hpp>
#include <tty/definitions.hpp>

namespace Kernel
{
	enum KeyModifiers : uint8_t
	{
		Shift = 0x01,
		Control = 0x02,
		Alt = 0x04,
		AltGr = 0x08,
		Super = 0x10,
		CapsLock = 0x20,
		NumLock = 0x40,
		Pressed = 0x80,
	};

	typedef struct key_event_t
	{
		uint32_t scancode;
		uint8_t modifiers;
		char code_point;
	} key_event_t;

	class KeyboardDevice : public CharacterDevice
	{
	public:
		KeyboardDevice(size_t major, size_t minor)
		    : CharacterDevice(major, minor)
		    , m_buffer(16)
		{}

		size_t read(size_t offset, size_t bytes, Memory::memory_region_t region) override;
		size_t write(size_t, size_t, Memory::memory_region_t) override { return 0; };

		LibK::StringView name() override { return LibK::StringView(m_name); };

		[[nodiscard]] size_t size() override{ return m_buffer.size() * sizeof(key_event_t); };

		void set_listener(TTY *listener) { m_listener = listener; }

	protected:
		[[nodiscard]] bool can_open_for_read() const override { return true; }
		[[nodiscard]] bool can_open_for_write() const override { return false; }

		void push(key_event_t event);

	private:
		LibK::CircularBuffer<key_event_t> m_buffer;
		Locking::Spinlock m_lock;
		LibK::string m_name{"kbd"};
		TTY *m_listener;
	};
}