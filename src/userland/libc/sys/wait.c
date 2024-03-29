#include <sys/wait.h>

#include <sys/syscall.h>

pid_t wait(int *stat_loc)
{
	return waitpid(-1, stat_loc, 0);
}

pid_t waitpid(pid_t pid, int *stat_loc, int options)
{
	return syscall(__SC_waitpid, pid, stat_loc, options);
}
