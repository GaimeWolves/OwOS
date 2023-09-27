#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	char buf[256] = {0};
	memset(buf, 0, sizeof(buf));

	while (true)
	{
		putchar('>');
		putchar(' ');
		read(STDIN_FILENO, buf, sizeof(buf));
		buf[sizeof(buf) - 1] = 0;

		for (size_t i = 0; i < sizeof(buf); i++)
		{
			if (buf[i] == '\r' || buf[i] == '\n')
				buf[i] = 0;
		}

		if (strncmp(buf, "exit", sizeof(buf) - 1) == 0)
			break;

		pid_t pid = fork();

		if (pid == -1)
		{
			perror("fork() failed");
			continue;
		}

		if (pid == 0)
		{
			// TODO: args
			char * const exec_argv[] = { buf, NULL };
			execv(exec_argv[0], exec_argv);

			perror("exec() failed");
			exit(1);
		}

		pid_t ret;
		do
		{
			ret = wait(NULL);
		} while (ret == -1 && errno == EINTR);
	}

	return 0;
}