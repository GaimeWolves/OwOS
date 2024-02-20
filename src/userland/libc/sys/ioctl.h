#pragma once

#include <bits/guards.h>


#define __ENUM_IOCTL_REQUESTS(E) \
	E(TCGETS)                    \
	E(TCSETS)

__LIBC_BEGIN_DECLS

enum __IOCtlNumber
{
#define __ENUM_FN(constant) constant,
	__ENUM_IOCTL_REQUESTS(__ENUM_FN)
#undef __ENUM_FN
};

#ifndef __LIBC_KEEP_DEFS
#	undef __ENUM_IOCTL_REQUESTS
#endif

int ioctl(int fd, unsigned long request, ...);

__LIBC_END_DECLS
