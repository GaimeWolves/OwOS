#pragma once

#include <bits/guards.h>
#include <sys/types.h>

__LIBC_HEADER_BEGIN

#define NAME_MAX 256

struct dirent
{
	ino_t d_ino;
	char d_name[NAME_MAX + 1];
};

typedef struct __DIR
{
	// TODO: implement some form of caching here?
	int fd;
	struct dirent dirent_buffer;
} DIR;

DIR *fdopendir(int fd);
DIR *opendir(const char *dirname);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

__LIBC_HEADER_END