#pragma once

#include <bits/guards.h>

__LIBC_HEADER_BEGIN

#define __ENUM_IOCTL_REQUESTS(E) \
	E(TCGETS)                    \
	E(TCSETS)

enum __IOCtlNum
{
#define __ENUM_FN(constant) constant,
	__ENUM_IOCTL_REQUESTS(__ENUM_FN)
#undef __ENUM_FN
};

int ioctl(int fd, unsigned long request, ...);

#ifndef __LIBC_KEEP_DEFS
#	undef __ENUM_IOCTL_REQUESTS
#endif

__LIBC_HEADER_END
