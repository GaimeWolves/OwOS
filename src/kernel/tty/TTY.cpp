#include <tty/TTY.hpp>

#include <../../userland/libc/sys/ioctl.h>

#include <arch/Processor.hpp>

namespace Kernel
{
	size_t TTY::read(size_t, size_t bytes, Memory::memory_region_t region)
	{
		if (m_termios.c_lflag & ICANON)
			return canonical_read(bytes, region);
		else
			return non_canonical_read(bytes, region);
	}

	size_t TTY::canonical_read(size_t bytes, Memory::memory_region_t region)
	{
		if (m_current_read_line.empty())
		{
			while (m_input_line_buffer.empty())
				CPU::Processor::pause();

			m_current_read_line = m_input_line_buffer.get();
		}

		char *data = reinterpret_cast<char *>(region.virt_region().pointer());

		size_t read = 0;

		while (bytes--)
		{
			if (m_current_read_line.empty())
				break;

			*data++ = m_current_read_line.front();
			m_current_read_line.erase(m_current_read_line.begin());
			read++;
		}

		return read;
	}

	size_t TTY::non_canonical_read(size_t bytes, Memory::memory_region_t region)
	{
		cc_t min = m_termios.c_cc[VMIN];

		char *data = reinterpret_cast<char *>(region.virt_region().pointer());

		if (min > 0)
		{
			// TODO: implement TIME > 0. This just assumes TIME = 0
			while (m_input_char_buffer.size() < min)
				CPU::Processor::sleep();
		}

		size_t read = LibK::min(m_input_char_buffer.size(), bytes);
		size_t counter = read;

		while (counter--)
			*data++ = m_input_char_buffer.pop();

		return read;
	}

	size_t TTY::write(size_t, size_t bytes, Memory::memory_region_t region)
	{
		size_t written = bytes;

		char *data = reinterpret_cast<char *>(region.virt_region().pointer());

		while(bytes--)
			process_echo(*data++);

		return written;
	}

	// Terminal Output Modes: https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap11.html#tag_11_02_03
	void TTY::process_echo(char ch)
	{
		// If OPOST is set, output data shall be post-processed as described below, {...}; otherwise, characters shall be transmitted without change
		if (!(m_termios.c_oflag & OPOST))
		{
			echo(ch);
			return;
		}

		// If ONLCR is set, the NL character shall be transmitted as the CR-NL character pair.
		if (ch == '\n' && m_termios.c_oflag & ONLCR)
		{
			echo('\r');
		}

		echo(ch);
	}

	void TTY::emit(char ch)
	{
		if (m_termios.c_lflag & ICANON)
			canonical_input(ch);
		else
			non_canonical_input(ch);
	}

	void TTY::canonical_input(char ch)
	{
		if (m_termios.c_lflag & ICANON && (ch == '\n' || ch == m_termios.c_cc[VEOL] || ch == m_termios.c_cc[VEOF]))
		{
			if (ch != m_termios.c_cc[VEOF])
				m_current_input_line.push_back(ch);

			m_input_line_buffer.put(LibK::string(m_current_input_line.c_str()));
			m_current_input_line = LibK::string();

			if (m_termios.c_lflag & ECHO || m_termios.c_lflag & ECHONL)
				process_echo('\n');

			return;
		}

		if (ch == m_termios.c_cc[VERASE] && m_termios.c_lflag & ECHOE && m_termios.c_lflag & ICANON)
		{
			if (m_current_input_line.empty())
				return;

			m_current_input_line.pop_back();

			if (m_termios.c_lflag & ECHO)
				unecho();

			return;
		}

		if (ch == m_termios.c_cc[VKILL] && m_termios.c_lflag & ECHOK && m_termios.c_lflag & ICANON)
		{
			while (!m_current_input_line.empty())
			{
				m_current_input_line.pop_back();

				if (m_termios.c_lflag & ECHO)
					unecho();
			}

			return;
		}

		m_current_input_line.push_back(ch);

		if (m_termios.c_lflag & ECHO)
			process_echo(ch);
	}

	void TTY::non_canonical_input(char ch)
	{
		m_input_char_buffer.push(ch);

		if (m_termios.c_lflag & ECHO)
			process_echo(ch);
	}

	// Linux TTY ioctl(): https://linux.die.net/man/4/tty_ioctl
	LibK::ErrorOr<void> TTY::ioctl(uint32_t request, uintptr_t arg)
	{
		if (request == TCGETS)
		{
			// Equivalent to tcgetattr(fd, argp).
			auto dest_termios = (struct termios *)arg;
			*dest_termios = get_termios();
			return ESUCCESS;
		}

		if (request == TCSETS)
		{
			// Equivalent to tcsetattr(fd, TCSANOW, argp).
			auto src_termios = (struct termios *)arg;
			set_termios(*src_termios);
			return ESUCCESS;
		}

		return EINVAL;
	}

	void TTY::set_termios(struct termios new_termios)
	{
		struct termios old_termios = m_termios;
		m_termios = new_termios;
		switch_buffers(old_termios);
	}

	void TTY::reset_termios()
	{
		// Partly just copied from running 'stty -a' in the linux console
		m_termios.c_iflag = ICRNL;
		m_termios.c_oflag = OPOST | ONLCR;
		m_termios.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL;
		m_termios.c_cflag = CS8;
		m_termios.c_ispeed = B38400;
		m_termios.c_ospeed = B38400;

		m_termios.c_cc[VINTR] = '\003';
		m_termios.c_cc[VQUIT] = '\034';
		m_termios.c_cc[VERASE] = '\177';
		m_termios.c_cc[VKILL] = '\025';
		m_termios.c_cc[VEOF] = '\004';
		m_termios.c_cc[VEOL] = 0;
		m_termios.c_cc[VSTART] = '\021';
		m_termios.c_cc[VSTOP] = '\023';
		m_termios.c_cc[VSUSP] = '\032';
		m_termios.c_cc[VMIN] = 1;
		m_termios.c_cc[VTIME] = 0;
	}

	void TTY::switch_buffers(struct termios old_termios)
	{
		if ((old_termios.c_lflag & ICANON) == (m_termios.c_lflag & ICANON))
			return;

		if (m_termios.c_lflag & ICANON)
		{
			// From char buffered to line buffered
			empty_line_buffer();

			while (!m_input_char_buffer.is_empty())
			{
				m_current_input_line.push_back(m_input_char_buffer.pop());

				if (m_current_input_line.back() == '\n')
				{
					m_input_line_buffer.put(LibK::string(m_current_input_line.c_str()));
					m_current_input_line = LibK::string();
				}
			}
		}
		else
		{
			// From line buffered to char buffered
			LibK::stack<LibK::string> lines;

			while (!m_input_line_buffer.empty())
				lines.push(LibK::string(m_input_line_buffer.get().c_str()));

			if (!m_current_input_line.empty())
			{
				lines.push(LibK::string(m_current_input_line.c_str()));
				m_current_input_line = LibK::string();
			}

			empty_char_buffer();

			while (!lines.empty())
			{
				for (char &ch : lines.top())
				{
					m_input_char_buffer.push(ch);
				}

				lines.pop();
			}
		}
	}

	void TTY::empty_line_buffer()
	{
		m_current_input_line = LibK::string();

		while (!m_input_line_buffer.empty())
			m_input_line_buffer.get();
	}

	void TTY::empty_char_buffer()
	{
		while (!m_input_char_buffer.is_empty())
			m_input_char_buffer.pop();
	}
}