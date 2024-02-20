#pragma once

#include <bits/guards.h>

#define O_CREAT 2
#define O_EXCL  4

#define O_RDONLY 1
#define O_RDWR   8

__LIBC_BEGIN_DECLS

int open(const char *path, int oflag, ...);

__LIBC_END_DECLS
