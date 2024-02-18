#pragma once

#include <bits/guards.h>

#define O_CREAT 2
#define O_EXCL 4

#define O_RDONLY 1
#define O_RDWR 8

__LIBC_HEADER_BEGIN

int open(const char *, int, ...);

__LIBC_HEADER_END
