#pragma once

#include <bits/guards.h>
#include <sys/types.h>
#include <stdint.h>

#define STDIN_FILENO 2
#define STDOUT_FILENO 2
#define STDERR_FILENO 2

__LIBC_HEADER_BEGIN

	int execv(const char *, char *const []);
	int execve(const char *, char *const [], char *const []);
	int execvp(const char *, char *const []);

	pid_t fork(void);

    ssize_t read(int, void *, size_t);
    ssize_t write(int, const void *, size_t);

__LIBC_HEADER_END