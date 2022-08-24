#include "logging/logger.hpp"

#include "message_queue.hpp"

#include <processes/GlobalScheduler.hpp>
#include <processes/CoreScheduler.hpp>
#include <devices/SerialDevice.hpp>
#include <vga/textmode.hpp>
#include <arch/Processor.hpp>

namespace Kernel
{
	static MessageQueue message_queue;
	static thread_t logging_thread;
	static Locking::Spinlock logger_lock;

	static void putc(const char ch)
	{
		if (VGA::Textmode::is_initialized())
			VGA::Textmode::putc(ch);

		if (!Kernel::SerialDevice::get(Kernel::SerialDevice::COM1).is_faulty())
		{
			Kernel::Memory::memory_region_t tmp;
			tmp.virt_address = (uintptr_t)&ch;
			Kernel::SerialDevice::get(Kernel::SerialDevice::COM1).write(0, 1, tmp);
		}
	}

	static void puts(const char *str)
	{
		if (VGA::Textmode::is_initialized())
			VGA::Textmode::puts(str);

		if (!Kernel::SerialDevice::get(Kernel::SerialDevice::COM1).is_faulty())
		{
			Kernel::Memory::memory_region_t tmp;
			tmp.virt_address = (uintptr_t)str;
			Kernel::SerialDevice::get(Kernel::SerialDevice::COM1).write(0, strlen(str), tmp);
		}

		putc('\n');
	}

	[[noreturn]] static void print_log_messages()
	{
		while (true)
		{
			if (message_queue.empty())
			{
				CoreScheduler::suspend(&logging_thread);
				continue;
			}

			auto message = message_queue.get();
			puts(message.c_str());
		}
	}

	static void kill_logger()
	{
		if (logging_thread.state != ThreadState::Terminated)
		{
			CoreScheduler::terminate(&logging_thread);
			putc('\n');
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
		logging_thread = GlobalScheduler::start_kernel_only_thread((uintptr_t)print_log_messages);
	}

	void critical_putc(const char ch)
	{
		kill_logger();
		putc(ch);
	}

	void critical_puts(const char *str)
	{
		kill_logger();
		puts(str);
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
		message_queue.put(LibK::move(message));
	}

	void log(const char *fmt, ...)
	{
		LibK::string message;

		va_list ap;
		va_start(ap, fmt);
		printf_into_string(message, fmt, ap);
		va_end(ap);

		log(LibK::move(message));
	}

	void log(const char *tag, const char *fmt, ...)
	{
		LibK::string message;

		printf_into_string(message, "(#%d) [%s] ", CPU::Processor::current().id(), tag);

		va_list ap;
		va_start(ap, fmt);
		printf_into_string(message, fmt, ap);
		va_end(ap);

		log(LibK::move(message));
	}
}