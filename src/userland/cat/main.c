#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <sys/stat.h>

#define SIZE 256

int main(int argc, char **argv)
{
	char buffer[SIZE];

	for (int i = 1; i < argc; i++)
	{
		FILE *file = fopen(argv[i], "r");

		if (!file)
		{
			perror("fopen");
			exit(1);
		}

		struct stat file_stat;
		fstat(fileno(file), &file_stat);

		while (file_stat.st_size)
		{
			size_t read = fread(buffer, sizeof(char), SIZE, file);
			fwrite(buffer, sizeof(char), read, stdout);

			file_stat.st_size -= read;

			if (read < SIZE)
				break;
		}

		fclose(file);
	}

	return 0;
}
