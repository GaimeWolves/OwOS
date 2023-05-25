#pragma once

#include <bits/guards.h>
#include <sys/types.h>

__LIBC_HEADER_BEGIN

#define NAME_MAX 256

typedef struct __DIR
{

} DIR;

struct dirent
{
	ino_t d_ino;
	char d_name[NAME_MAX];
};

DIR *fdopendir(int fd);
DIR *opendir(const char *dirname);

__LIBC_HEADER_END