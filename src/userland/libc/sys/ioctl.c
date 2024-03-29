#include <sys/mman.h>

#include <__debug.h>

#include <stdarg.h>
#include <sys/syscall.h>

int ioctl(int fd, unsigned long request, ...)
{
	TRACE("ioctl(%d, %lu)\r\n", fd, request);

	va_list ap;
	va_start(ap, request);

	char *argp = va_arg(ap, char *);

	va_end(ap);

	return syscall(__SC_ioctl, fd, request, argp);
}
