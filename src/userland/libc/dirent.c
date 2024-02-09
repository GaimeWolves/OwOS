#include <dirent.h>

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/arch/i386/syscall.h>

#include "__debug.h"

DIR *fdopendir(int fd)
{
	TRACE("fdopendir(%d)\r\n", fd);
	DIR *dirp = malloc(sizeof(DIR));

	if (!dirp)
		return dirp;

	dirp->fd = fd;
	return dirp;
}

DIR *opendir(const char *dirname)
{
	TRACE("opendir(%s)\r\n", dirname);
	int fd = open(dirname, 0, 0);
	return fdopendir(fd);
}

struct dirent *readdir(DIR *dirp)
{
	TRACE("readdir(%p)\r\n", dirp);
	int written = syscall(__SC_getdents, dirp->fd, &dirp->dirent_buffer, 1);

	if (written > 0)
		return &dirp->dirent_buffer;

	return NULL;
}

int closedir(DIR *dirp)
{
	TRACE("closedir(%p)\r\n", dirp);
	close(dirp->fd);
	free(dirp);

	return 0;
}