#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		puts("Usage: ls [file]");
		exit(1);
	}

	DIR *dir;

	if (argc == 1)
		dir = opendir(".");
	else
		dir = opendir(argv[1]);

	struct dirent *entry = readdir(dir);

	while (entry != NULL)
	{
		puts(entry->d_name);
		entry = readdir(dir);
	}

	closedir(dir);

	return 0;
}
