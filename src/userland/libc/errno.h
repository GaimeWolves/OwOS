#pragma once

#include <bits/guards.h>

__LIBC_HEADER_BEGIN

#define __ENUM_ERRNO_CODES(E)     \
	E(ESUCCESS, "No error")       \
	E(EACCES, "")                 \
	E(EAGAIN, "")                 \
	E(EBADF, "")                  \
	E(EINVAL, "Invalid argument") \
	E(EMFILE, "")                 \
	E(ENODEV, "")                 \
	E(ENOMEM, "")                 \
	E(ENOTSUP, "")                \
	E(ENXIO, "")                  \
	E(EOVERFLOW, "")              \
	E(ENOTTY, "")                 \
	E(ENOENT, "")                 \
	E(EINTR, "")                 \
	E(ECHILD, "")

enum __ErrnoCode
{
#define __ENUM_FN(constant, string) constant,
	__ENUM_ERRNO_CODES(__ENUM_FN)
#undef __ENUM_FN
};

extern int errno;

#ifndef __LIBC_KEEP_DEFS
#	undef __ENUM_ERRNO_CODES
#endif

__LIBC_HEADER_END