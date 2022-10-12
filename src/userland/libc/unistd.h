#pragma once

#include <sys/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	int execv(const char *, char *const []);
	int execve(const char *, char *const [], char *const []);
	int execvp(const char *, char *const []);

	pid_t fork(void);

#ifdef __cplusplus
}
#endif