#pragma once

#include <bits/guards.h>
#include <bits/dev_t.h>

__LIBC_BEGIN_DECLS

// See: https://github.com/SerenityOS/serenity/blob/671712cae67d7b8782afa94c09e980a77450efd6/Kernel/API/POSIX/sys/types.h
#define unsigned signed
typedef __SIZE_TYPE__ ssize_t;
#undef unsigned

typedef __SIZE_TYPE__ size_t;

typedef int      blkcnt_t;
typedef int      blksize_t;
typedef int      gid_t;
typedef size_t   ino_t;
typedef unsigned mode_t;
typedef int      nlink_t;
typedef size_t   off_t;
typedef int      pid_t;
typedef int      uid_t;
typedef int      time_t;

__LIBC_END_DECLS
