#pragma once

#include <libk/kstring.hpp>
#include <libk/kcstdarg.hpp>
#include <libk/printf.hpp>

namespace Kernel
{
	void start_logger_thread();

	void critical_empty_logger();
	void critical_putc(const char ch);
	void critical_puts(const char *str);
	void critical_printf(const char *fmt, ...);
	void critical_vprintf(const char *fmt, va_list ap);

	void log(LibK::string &&message);
	void log(const char *fmt, ...);
	void log(const char *tag, const char *fmt, ...);
}
