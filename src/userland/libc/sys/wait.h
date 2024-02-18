#pragma once

#include <bits/guards.h>
#include <sys/types.h>

#define WEXITSTATUS(status) ((status) & 0xFF)
#define WTERMSIG(status)    (((status) >> 8) & 0xFF)
#define WSTOPSIG(status)    WTERMSIG(status)
#define WIFCONTINUED        (!WEXITSTATUS(status) && !WSTOPSIG(status))
#define WIFSTOPPED(status)  (!WEXITSTATUS(status) && WSTOPSIG(status))
#define WIFEXITED(status)   (WEXITSTATUS(status) && !WSTOPSIG(status))
#define WIFSIGNALED(status) (WEXITSTATUS(status) && WSTOPSIG(status))

#define WEXITED    0x01
#define WSTOPPED   0x02
#define WCONTINUED 0x04

__LIBC_HEADER_BEGIN

pid_t wait(int *stat_loc);
pid_t waitpid(pid_t pid, int *stat_loc, int options);

__LIBC_HEADER_END
