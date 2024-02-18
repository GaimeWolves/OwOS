#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	pid_t pid = fork();

	if (pid == -1)
	{
		perror("fork() failed");
		exit(1);
	}

	if (pid == 0)
	{
		char * const exec_argv[] = { "/bin/sh", NULL };
		execv(exec_argv[0], exec_argv);

		perror("exec() failed");
		exit(1);
	}

	pid_t ret;
	do
	{
		ret = wait(NULL);
	} while (ret != -1 || errno == EINTR);

	puts("init exiting");

	return 0;
}
