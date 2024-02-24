#include "logging/logger.hpp"

#include <libk/srmw_queue.hpp>
#include <libk/kstring.hpp>

#include <processes/GlobalScheduler.hpp>
#include <processes/CoreScheduler.hpp>
#include <devices/SerialDevice.hpp>
#include <tty/VirtualConsole.hpp>
#include <arch/Processor.hpp>

namespace Kernel
{
	static LibK::SRMWQueue<LibK::string> s_message_queue{};
	static thread_t *s_logging_thread;
	static bool s_logging_thread_started{false};

	static void putc(const char ch, bool critical)
	{
		if (critical)
			VirtualConsole::get_current().write(0, 1, const_cast<char *>(&ch));

		if (!Kernel::SerialDevice::get(Kernel::SerialDevice::COM1).is_faulty())
			Kernel::SerialDevice::get(Kernel::SerialDevice::COM1).write(0, 1, const_cast<char *>(&ch));
	}

	static void puts(const char *str, bool critical)
	{
		if (critical)
			VirtualConsole::get_current().write(0, strlen(str), const_cast<char *>(str));

		if (!Kernel::SerialDevice::get(Kernel::SerialDevice::COM1).is_faulty())
			Kernel::SerialDevice::get(Kernel::SerialDevice::COM1).write(0, strlen(str), const_cast<char *>(str));

		putc('\n', critical);
	}

	[[noreturn]] static void print_log_messages()
	{
		while (true)
		{
			if (s_message_queue.empty())
			{
				CoreScheduler::suspend(s_logging_thread);
				continue;
			}

			auto message = s_message_queue.get();
			puts(message.c_str(), false);
		}
	}

	static void kill_logger()
	{
		if (s_logging_thread && s_logging_thread->state != ThreadState::Terminated)
		{
			CoreScheduler::terminate(s_logging_thread);
			putc('\n', true);
		}
	}

	void critical_empty_logger()
	{
		while (!s_message_queue.empty())
		{
			auto message = s_message_queue.get();
			puts(message.c_str(), false);
		}
	}

	static void printf_into_string(LibK::string &string, const char *fmt, va_list ap)
	{
		LibK::printf_internal(
		    [&string](char *&, char ch) {
			string.push_back(ch);
		    },
		    nullptr, fmt, ap);
	}

	static void printf_into_string(LibK::string &string, const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		printf_into_string(string, fmt, ap);
		va_end(ap);
	}

	void start_logger_thread()
	{
		s_logging_thread = GlobalScheduler::create_kernel_only_thread(nullptr, (uintptr_t)print_log_messages);
		GlobalScheduler::start_thread(s_logging_thread);
		s_logging_thread_started = true;
	}

	void critical_putc(const char ch)
	{
		kill_logger();
		putc(ch, true);
	}

	void critical_puts(const char *str)
	{
		kill_logger();
		puts(str, true);
	}

	void critical_printf(const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		critical_vprintf(fmt, ap);
		va_end(ap);
	}

	void critical_vprintf(const char *fmt, va_list ap)
	{
		kill_logger();
		LibK::printf_internal(
		    [](char *&, char ch) {
				critical_putc(ch);
		    },
		    nullptr, fmt, ap);
	}

	void log(LibK::string &&message)
	{
		s_message_queue.put(std::move(message));
		if (s_logging_thread_started)
			CoreScheduler::resume(s_logging_thread);
	}

	void log(const char *fmt, ...)
	{
		LibK::string message;

		va_list ap;
		va_start(ap, fmt);
		printf_into_string(message, fmt, ap);
		va_end(ap);

		log(std::move(message));
	}

	void log(const char *tag, const char *fmt, ...)
	{
		LibK::string message;

		printf_into_string(message, "(#%d) [%s] ", CPU::Processor::current().id(), tag);

		if (fmt)
		{
			va_list ap;
			va_start(ap, fmt);
			printf_into_string(message, fmt, ap);
			va_end(ap);
		}
		else
		{
			printf_into_string(message, "<empty string>");
		}

		log(std::move(message));
	}
}
