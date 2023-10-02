#include <unistd.h>

#include <__debug.h>

#include <bits/environ.h>
#include <sys/arch/i386/syscall.h>

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

char *optarg;
int opterr, optind, optopt;

int close(int filedes)
{
	TRACE("close(%d)\r\n", filedes);
	return syscall(__SC_close, filedes);
}

void _exit(int status)
{
	(void)status;
	TRACE("_exit(%d)\r\n", status);

	for(;;)
		;
}

int execv(const char *path, char *const argv[])
{
	return execve(path, argv, environ);
}

int execve(const char *path, char *const argv[], char *const envp[])
{
	return syscall(__SC_exec, path, argv, envp);
}

int execvp(const char *path, char *const argv[])
{
	// TODO: PATH variable search

	return execv(path, argv);
}

pid_t fork(void)
{
	return syscall(__SC_fork);
}

ssize_t read(int fd, void *buf, size_t count)
{
	ssize_t ret = syscall(__SC_read, fd, buf, count);

	TRACE("read(%d, %p, %zu) -> %zd\r\n", fd, buf, count, ret);

	return ret;
}

ssize_t write(int fd, const void *buf, size_t count)
{
	return syscall(__SC_write, fd, buf, count);
}

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/access.html
int access(const char *path, int amode)
{
	TRACE("access(%s, %d)\r\n", path, amode);

	(void)amode;
	puts("access() not working properly! R_OK, W_OK, X_OK always allowed!");

	struct stat buf;
	if (stat(path, &buf) != 0) {
		errno = ENOENT;
		return -1;
	}

	return 0;
}

int chdir(const char *path)
{
	TRACE("chdir(%s)\r\n", path);

	return syscall(__SC_chdir, path);
}

int unlink(const char *path)
{
	TRACE("unlink(%s)\r\n", path);
	(void)path;
	puts("unlink() not implemented");
	abort();
}

char *getcwd(char *buf, size_t size)
{
	TRACE("getcwd(%.*s)\r\n", PATH_MAX, buf);

	uintptr_t ret = syscall(__SC_getcwd, buf, size);

	return ret == -1UL ? NULL : (char *)ret;
}

char *getwd(char buf[PATH_MAX])
{
	TRACE("getwd(%.*s)\r\n", PATH_MAX, buf);

	return getcwd(buf, PATH_MAX);
}

int dup(int fildes)
{
	TRACE("dup(%d)\r\n", fildes);
	(void)fildes;
	puts("dup() not implemented");
	abort();
}

int getopt(int argc, char * const argv[], const char *optstring)
{
	TRACE("getopt(%d, %p, %s)\r\n", argc, argv, optstring);
	(void)argc;
	(void)argv;
	(void)optstring;
	puts("getopt() not implemented");
	abort();
}

int isatty(int fildes)
{
	TRACE("isatty(%d)\r\n", fildes);
	puts("isatty() not fully implemented!");

	return fildes >= 0 && fildes <= STDERR_FILENO;
}

unsigned sleep(unsigned seconds)
{
	TRACE("sleep(%u)\r\n", seconds);
	(void)seconds;
	puts("sleep() not implemented");
	abort();
}