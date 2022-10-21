#include <unistd.h>

#include <sys/arch/i386/syscall.h>

int execv(const char *, char *const [])
{
	return 0;
}

int execve(const char *, char *const [], char *const [])
{
	return 0;
}

int execvp(const char *, char *const [])
{
	return 0;
}

pid_t fork(void)
{
	return 0;
}

ssize_t read(int fd, void *buf, size_t count)
{
	return syscall(__SC_read, fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count)
{
	return syscall(__SC_write, fd, buf, count);
}