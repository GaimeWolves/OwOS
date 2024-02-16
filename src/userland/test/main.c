#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <signal.h>

static volatile int run = 10;

void signal_handler(int signal, siginfo_t *siginfo, void *context)
{
	printf("signal_handler(%d, %p, %p) - run = %d\n", signal, siginfo, context, --run);
}

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	printf("signal_handler at %p\n", signal_handler);

	struct sigaction act;

	act.sa_sigaction = signal_handler;
	sigaction(SIGINT, &act, NULL);

	while (run)
		;

	printf("exiting!\n");

	return 0;
}