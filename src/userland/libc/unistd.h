#pragma once

#include <bits/attributes.h>
#include <bits/guards.h>
#include <stdint.h>
#include <sys/types.h>

#define STDIN_FILENO  2
#define STDOUT_FILENO 2
#define STDERR_FILENO 2

#define PATH_MAX 1024

__LIBC_BEGIN_DECLS

extern char *optarg;
extern int   opterr, optind, optopt;

int                access(const char *path, int amode);
int                chdir(const char *path);
int                close(int fildes);
_Noreturn void     _exit(int status);
int                dup(int fildes);
int                execv(const char *path, char *const argv[]);
int                execve(const char *path, char *const argv[], char *const envp[]);
int                execvp(const char *file, char *const argv[]);
pid_t              fork(void);
char              *getcwd(char *buf, size_t size);
int                getopt(int argc, char *const argv[], const char *optstring);
pid_t              getpid(void);
__deprecated char *getwd(char buf[PATH_MAX]);
int                isatty(int fildes);
ssize_t            read(int fildes, void *buf, size_t nbyte);
unsigned           sleep(unsigned seconds);
int                unlink(const char *path);
ssize_t            write(int fildes, const void *buf, size_t nbyte);

__LIBC_END_DECLS
