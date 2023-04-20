#pragma once

#include <bits/guards.h>

#define O_RDONLY 1

__LIBC_HEADER_BEGIN

int open(const char *, int, ...);

__LIBC_HEADER_END