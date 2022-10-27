#include <sys/stat.h>

#include <sys/arch/i386/syscall.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/fstat.html
int fstat(int filedes, struct stat *buf)
{
	return syscall(__SC_stat, filedes, buf);
}