#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	puts("Hello from test executable :3");
	printf("> ");

	char buffer[256] = { 0 };
	int index = 0;
	while (1)
	{
		char ch = 0;
		read(0, &ch, 1);

		if (ch == '\n')
		{
			if (ch == '\n')
				printf("%s\n", buffer);

			if (strcmp(buffer, "exit") == 0)
			{
				puts("Goodbye :<");
				break;
			}

			memset(buffer, 0, 256);
			index = 0;

			printf("> ");

			continue;
		}

		if (index < 255)
			buffer[index++] = ch;
	}

	return 0;
}