#include <devices/KeyboardDevice.hpp>

#include <tty/TTY.hpp>

namespace Kernel
{
	size_t KeyboardDevice::read(size_t, size_t bytes, Memory::memory_region_t region)
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
			memcpy(region.virt_region().pointer(), &event, sizeof(key_event_t));
			read += sizeof(key_event_t);
		}

		return read;
	}

	void KeyboardDevice::push(Kernel::key_event_t event)
	{
		m_lock.lock();
		m_buffer.push(event);
		m_lock.unlock();

		if (m_listener)
			m_listener->on_key_event(event);
	}
}