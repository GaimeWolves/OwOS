#include <fcntl.h>

#include <__debug.h>

#include <sys/arch/i386/syscall.h>
#include <sys/types.h>
#include <stdarg.h>

int open(const char *path, int oflag, ...)
{
	TRACE("open(%s, %lu)\r\n", path, oflag);

	va_list ap;
	va_start(ap, oflag);
	mode_t mode = (mode_t)va_arg(ap, mode_t);
	va_end(ap);

	return syscall(__SC_open, path, oflag, mode);
}