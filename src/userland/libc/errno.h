#pragma once

#include <bits/guards.h>

#define __ENUM_ERRNO_CODES(E)              \
	E(ESUCCESS, "No error")                \
	E(EACCES, "Permission denied")         \
	E(EAGAIN, "")                          \
	E(EBADF, "")                           \
	E(ECHILD, "")                          \
	E(EINTR, "")                           \
	E(EINVAL, "Invalid argument")          \
	E(EMFILE, "")                          \
	E(ENODEV, "")                          \
	E(ENOENT, "No such file or directory") \
	E(ENOEXEC, "Exec format error")        \
	E(ENOMEM, "")                          \
	E(ENOTDIR, "Not a directory")          \
	E(ENOTSUP, "")                         \
	E(ENOTTY, "")                          \
	E(ENXIO, "")                           \
	E(EOVERFLOW, "")                       \
	E(ERANGE, "Numerical result out of range")

__LIBC_BEGIN_DECLS

enum __ErrnoCode
{
#define __ENUM_FN(constant, string) constant,
	__ENUM_ERRNO_CODES(__ENUM_FN)
#undef __ENUM_FN
};

#ifndef __LIBC_KEEP_DEFS
#	undef __ENUM_ERRNO_CODES
#endif

extern int errno;

// TODO: define error codes as macros (an enum is not sufficient)

__LIBC_END_DECLS
