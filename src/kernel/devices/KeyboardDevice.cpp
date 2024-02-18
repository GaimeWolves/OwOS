#include <devices/KeyboardDevice.hpp>

#include <tty/TTY.hpp>
#include <arch/Processor.hpp>

namespace Kernel
{
	size_t KeyboardDevice::read(size_t, size_t bytes, char *buffer)
	{
		size_t read = 0;
		while (read < bytes)
		{
			if (m_buffer.is_empty())
				break;

			if (bytes - read < sizeof(key_event_t))
				break;

			m_lock.lock();
			key_event_t event = m_buffer.pop();
			m_lock.unlock();
			memcpy(buffer, &event, sizeof(key_event_t));
			read += sizeof(key_event_t);
		}

		return read;
	}

	void KeyboardDevice::push(Kernel::key_event_t event)
	{
		m_lock.lock();
		m_buffer.push(event);
		m_lock.unlock();

		if (!m_listeners.empty())
		{
			CPU::Processor::current().defer_call([this, event]() {
				for (auto *it : m_listeners)
					it->handle_key_event(event);
			});
		}
	}
}
