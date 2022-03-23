#include <libk/kcstdio.hpp>

#include <devices/SerialPort.hpp>
#include <libk/kcstdarg.hpp>
#include <vga/textmode.hpp>
#include <locking/Mutex.hpp>

#include "printf.hpp"

using namespace Kernel::VGA;
using namespace Kernel::Devices;
using namespace Kernel::LibK;

static Kernel::Locking::Mutex kernel_print_lock;

extern "C"
{
	void kputc(const char ch)
	{
		if (Textmode::is_initialized())
			Textmode::putc(ch);

		if (SerialPort::is_initialized())
			SerialPort::get(SerialPort::COM1).write_one(ch);
	}

	void kputs(const char *str)
	{
		if (Textmode::is_initialized())
			Textmode::puts(str);

		if (SerialPort::is_initialized())
			SerialPort::get(SerialPort::COM1).write(str);
	}

	void kvprintf(const char *fmt, va_list ap)
	{
		printf_internal(
		    [](char *&, char ch) {
			    kputc(ch);
		    },
		    nullptr, fmt, ap);
	}

	void kvsprintf(char *buffer, const char *fmt, va_list ap)
	{
		printf_internal(
		    [](char *&buf, char ch) {
			    *buf++ = ch;
		    },
		    buffer, fmt, ap);
	}

	void kprintf(const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);

		kvprintf(fmt, ap);

		va_end(ap);
	}

	void ksprintf(char *buffer, const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);

		kvsprintf(buffer, fmt, ap);

		va_end(ap);
	}
}

namespace Kernel::LibK
{
	void printf_check_msg(bool ok, const char *msg, ...)
	{
		kernel_print_lock.lock();
		kputc('[');

		if (ok)
		{
			Textmode::set_color(Textmode::Color::GREEN, Textmode::Color::BLACK);
			kputs("  OK  ");
		}
		else
		{
			Textmode::set_color(Textmode::Color::RED, Textmode::Color::BLACK);
			kputs("FAILED");
		}

		Textmode::set_color(Textmode::Color::WHITE, Textmode::Color::BLACK);
		kputc(']');
		kputc(' ');

		va_list ap;
		va_start(ap, msg);

		kvprintf(msg, ap);

		va_end(ap);

		kputc('\n');
		kernel_print_lock.unlock();
	}

	void printf_debug_msg(const char *msg, ...)
	{
		kernel_print_lock.lock();
		kprintf("(#%d) [", CPU::Processor::current().id());

		Textmode::set_color(Textmode::Color::MAGENTA, Textmode::Color::BLACK);
		kputs("DEBUG");
		Textmode::set_color(Textmode::Color::WHITE, Textmode::Color::BLACK);

		kputc(']');
		kputc(' ');

		va_list ap;
		va_start(ap, msg);

		kvprintf(msg, ap);

		va_end(ap);

		kputc('\n');
		kernel_print_lock.unlock();
	}

	void printf_test_msg(const char *msg, ...)
	{
		kernel_print_lock.lock();
		kputc('[');

		Textmode::set_color(Textmode::Color::LIGHT_GREEN, Textmode::Color::BLACK);
		kputs(" TEST ");
		Textmode::set_color(Textmode::Color::WHITE, Textmode::Color::BLACK);

		kputc(']');
		kputc(' ');

		va_list ap;
		va_start(ap, msg);

		kvprintf(msg, ap);

		va_end(ap);

		kputc('\n');
		kernel_print_lock.unlock();
	}
} // namespace Kernel::LibK