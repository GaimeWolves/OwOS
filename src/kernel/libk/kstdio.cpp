#include <libk/kstdio.hpp>

#include <libk/kstdarg.hpp>
#include <vga/textmode.hpp>

#include "printf.hpp"

using namespace Kernel::VGA;
using namespace Kernel::LibK;

extern "C"
{
	void kputc(const char ch)
	{
		Textmode::putc(ch);
	}

	void kputs(const char *str)
	{
		Textmode::puts(str);
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
	}

	void printf_debug_msg(const char *msg, ...)
	{
		kputc('[');

		Textmode::set_color(Textmode::Color::MAGENTA, Textmode::Color::BLACK);
		kputs(" DEBUG");
		Textmode::set_color(Textmode::Color::WHITE, Textmode::Color::BLACK);

		kputc(']');
		kputc(' ');

		va_list ap;
		va_start(ap, msg);

		kvprintf(msg, ap);

		va_end(ap);

		kputc('\n');
	}

	void printf_test_msg(const char *msg, ...)
	{
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
	}
} // namespace Kernel::LibK