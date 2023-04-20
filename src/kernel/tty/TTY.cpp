#include <tty/TTY.hpp>

#include <arch/Processor.hpp>
#include <tty/FramebufferConsole.hpp>

namespace Kernel
{
	TTY TTY::s_tty = TTY(0, 0);

	size_t TTY::read(size_t, size_t bytes, Memory::memory_region_t region)
	{
		char *data = reinterpret_cast<char *>(region.virt_region().pointer());

		size_t read = bytes;

		while (bytes)
		{
			while(m_input_buffer.is_empty())
				CPU::Processor::pause();

			m_input_lock.lock();
			size_t size = LibK::min(bytes, m_input_buffer.size());
			bytes -= size;
			while (size--)
				*data++ = m_input_buffer.pop();
			m_input_lock.unlock();
		}

		return read;
	}

	// TODO: temporary - abstract to sub class
	void TTY::advance()
	{
		m_column++;

		if (m_column == FramebufferConsole::instance().get_width())
		{
			m_column = 0;
			m_row++;

			if (m_row == FramebufferConsole::instance().get_height()) {
				m_row--;
				FramebufferConsole::instance().scroll_up();
				FramebufferConsole::instance().clear(m_row, 0, m_row, FramebufferConsole::instance().get_width());
			}
		}
	}

	size_t TTY::write(size_t, size_t bytes, Memory::memory_region_t region)
	{
		size_t written = bytes;

		char *data = reinterpret_cast<char *>(region.virt_region().pointer());

		while(bytes--)
		{
			if (*data == '\n') {
				m_column = 0;
				m_row++;

				if (m_row == FramebufferConsole::instance().get_height()) {
					m_row--;
					FramebufferConsole::instance().scroll_up();
					FramebufferConsole::instance().clear(m_row, 0, m_row, FramebufferConsole::instance().get_width());
				}
			}
			else
			{
				FramebufferConsole::instance().put_char_at(m_row, m_column, *data++);
				advance();
			}
		}

		return written;
	}

	void TTY::on_key_event(key_event_t event)
	{
		if (!(event.modifiers & KeyModifiers::Pressed))
			return;

		if (event.code_point)
		{
			m_input_lock.lock();
			m_input_buffer.push(event.code_point);
			m_input_lock.unlock();

			if (event.code_point == '\n') {
				m_column = 0;
				m_row++;

				if (m_row == FramebufferConsole::instance().get_height()) {
					m_row--;
					FramebufferConsole::instance().scroll_up();
					FramebufferConsole::instance().clear(m_row, 0, m_row, FramebufferConsole::instance().get_width());
				}
			}
			else
			{
				FramebufferConsole::instance().put_char_at(m_row, m_column, event.code_point);
				advance();
			}
		}
	}
}