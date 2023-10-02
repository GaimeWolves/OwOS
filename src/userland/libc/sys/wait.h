#pragma once

#include <bits/guards.h>
#include <sys/types.h>

#define WEXITSTATUS(wstatus) (wstatus & 0xFF)

__LIBC_HEADER_BEGIN

pid_t wait(int *stat_loc);
pid_t waitpid(pid_t pid, int *stat_loc, int options);

__LIBC_HEADER_END