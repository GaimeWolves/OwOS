#include <dirent.h>

#include <stdlib.h>
#include <stdio.h>

DIR *fdopendir(int fd)
{
	(void)fd;
	puts("fdopendir() not implemented");
	abort();
}

DIR *opendir(const char *dirname)
{
	(void)dirname;
	puts("opendir() not implemented");
	abort();
}

struct dirent *readdir(DIR *dirp)
{
	(void)dirp;
	puts("readdir() not implemented");
	abort();
}

int closedir(DIR *dirp)
{
	(void)dirp;
	puts("closedir() not implemented");
	abort();
}