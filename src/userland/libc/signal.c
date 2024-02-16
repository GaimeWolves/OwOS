#include <signal.h>

#include "__debug.h"
#include <stdio.h>
#include <stdlib.h>

#include <sys/arch/i386/syscall.h>

void (*signal(int sig, void (*func)(int)))(int)
{
	(void)sig;
	(void)func;
	puts("signal() not implemented");

	return 0;
}

int sigaction(int sig, const struct sigaction *restrict act, struct sigaction *restrict oact)
{
	TRACE("sigaction(%d, %p, %p)\r\n", signal, act, oact);

	return syscall(__SC_sigaction, sig, act, oact);
}

int sigemptyset(sigset_t *set)
{
	puts("sigemptyset() not fully implemented");

	if (!set)
		return -1;

	*set = 0;
	return 0;
}

int sigaddset(sigset_t *set, int signo)
{
	TRACE("sigaddset(%p, %d)\r\n", set, signo);

	puts("sigaddset() not fully implemented");

	if (!set)
		return -1;

	*set |= (1 << signo);
	return 0;
}

int sigismember(const sigset_t *set, int signo)
{
	TRACE("sigismember(%p, %d)\r\n", set, signo);

	puts("sigismember() not fully implemented");

	if (!set)
		return -1;

	return (*set & (1 << signo)) != 0;
}

int kill(pid_t pid, int sig)
{
	TRACE("kill(%d, %d)\r\n", pid, signo);

	puts("kill() not implemented");

	(void)pid;
	(void)sig;

	return 0;
}

int sigprocmask(int how, const sigset_t *__restrict set, sigset_t *__restrict oset)
{
	TRACE("sigprocmask(%d, %p, %p)\r\n", how, set, oset);

	puts("sigprocmask() not implemented");

	(void)how;
	(void)set;
	(void)oset;

	return 0;
}