#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	FILE *file = fopen("test.txt", "w+");

	size_t size = 4096 * 32;
	static char buffer[4096 * 32];

	for (size_t i = 0; i < size; i++)
		buffer[i] = (char)('0' + i % 10);

	for (size_t i = 0; i < 1100 / 32; i++)
	{
		write(fileno(file), buffer, size);
	}

	fflush(file);
	fclose(file);

	return 0;
}