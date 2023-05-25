#include <signal.h>

#include <stdlib.h>
#include <stdio.h>

void (*signal(int sig, void (*func)(int)))(int)
{
	(void)sig;
	(void)func;
	puts("signal() not implemented");

	return 0;
}

int sigaction(int sig, const struct sigaction *restrict act, struct sigaction *restrict oact)
{
	(void)sig;
	(void)act;
	(void)oact;
	puts("sigaction() not implemented");

	return 0;
}

int sigemptyset(sigset_t *set)
{
	puts("sigemptyset() not fully implemented");

	if (!set)
		return -1;

	*set = 0;
	return 0;
}