#include <fcntl.h>

#include <sys/arch/i386/syscall.h>
#include <sys/types.h>
#include <stdarg.h>

int open(const char *path, int oflag, ...)
{
	va_list ap;
	va_start(ap, oflag);
	mode_t mode = (mode_t)va_arg(ap, mode_t);
	va_end(ap);

	return syscall(__SC_open, path, oflag, mode);
}