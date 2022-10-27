#include <unistd.h>

#include <sys/arch/i386/syscall.h>

int close(int filedes)
{
	return syscall(__SC_close, filedes);
}

ssize_t read(int fd, void *buf, size_t count)
{
	return syscall(__SC_read, fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count)
{
	return syscall(__SC_write, fd, buf, count);
}