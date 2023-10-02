#include <sys/stat.h>

#include <__debug.h>

#include <sys/arch/i386/syscall.h>

#include <stdlib.h>
#include <stdio.h>

int stat(const char *restrict path, struct stat *restrict buf)
{
	TRACE("stat(%s, %p)\r\n", path, buf);

	return syscall(__SC_stat, __SC_stat_TYPE_STAT, NULL, path, 0, buf, 0);
}

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/fstat.html
int fstat(int filedes, struct stat *buf)
{
	TRACE("fstat(%d, %p)\r\n", filedes, buf);

	return syscall(__SC_stat, __SC_stat_TYPE_FSTAT, NULL, NULL, filedes, buf, 0);
}

int mkdir(const char *path, mode_t mode)
{
	(void)path;
	(void)mode;
	puts("mkdir() not implemented");
	abort();
}