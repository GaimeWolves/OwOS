#pragma once

#include <bits/guards.h>
#include <sys/types.h>

#define NAME_MAX 256

__LIBC_BEGIN_DECLS

struct dirent
{
	ino_t d_ino;
	char  d_name[NAME_MAX + 1];
};

typedef struct __DIR
{
	// TODO: implement some form of caching here?
	int           fd;
	struct dirent dirent_buffer;
} DIR;

int            closedir(DIR *dirp);
DIR           *fdopendir(int fd);
DIR           *opendir(const char *dirname);
struct dirent *readdir(DIR *dirp);

__LIBC_END_DECLS
