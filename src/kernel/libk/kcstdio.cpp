#include <libk/kcstdio.hpp>

#include <devices/SerialDevice.hpp>
#include <libk/kcstdarg.hpp>
#include <vga/textmode.hpp>
#include <locking/Mutex.hpp>
#include <logging/logger.hpp>

using namespace Kernel::VGA;
using namespace Kernel::LibK;

static Kernel::Locking::Mutex kernel_print_lock;

extern "C"
{
	void kputc(const char ch)
	{
		if (Textmode::is_initialized())
			Textmode::putc(ch);

		if (!Kernel::SerialDevice::get(Kernel::SerialDevice::COM1).is_faulty())
		{
			Kernel::Memory::memory_region_t tmp;
			tmp.virt_address = (uintptr_t)&ch;
			Kernel::SerialDevice::get(Kernel::SerialDevice::COM1).write(0, 1, tmp);
		}
	}

	void kputs(const char *str)
	{
		if (Textmode::is_initialized())
			Textmode::puts(str);

		if (!Kernel::SerialDevice::get(Kernel::SerialDevice::COM1).is_faulty())
		{
			Kernel::Memory::memory_region_t tmp;
			tmp.virt_address = (uintptr_t)str;
			Kernel::SerialDevice::get(Kernel::SerialDevice::COM1).write(0, strlen(str), tmp);
		}
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