#pragma once

#include <bits/guards.h>
#include <sys/types.h>
#include <stdint.h>

#define STDIN_FILENO 2
#define STDOUT_FILENO 2
#define STDERR_FILENO 2

#define PATH_MAX 1024

__LIBC_HEADER_BEGIN

	extern char *optarg;
	extern int opterr, optind, optopt;

    void _exit(int status);

	int close(int);

    int execv(const char *path, char *const argv[]);
    int execve(const char *path, char *const argv[], char *const envp[]);
	int execvp(const char *, char *const []);

    pid_t fork(void);

    int access(const char *path, int amode);
    int chdir(const char *path);
    int unlink(const char *path);

    __attribute__((deprecated)) char *getwd(char buf[PATH_MAX]);

    ssize_t read(int, void *, size_t);
    ssize_t write(int, const void *, size_t);

    int dup(int fildes);

    int getopt(int argc, char * const argv[], const char *optstring);

    int isatty(int fildes);

    unsigned sleep(unsigned seconds);

__LIBC_HEADER_END