#pragma once

#include <sys/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

	__attribute__((noreturn)) void abort(void);
	int abs(int);
	int atexit(void (*func)(void));
	int atoi(const char *str);
	__attribute__((malloc)) __attribute__((alloc_size(1, 2))) void *calloc(size_t nelem, size_t elsize);
	__attribute__((noreturn)) void exit(int);
	void free(void *);
	char *getenv(const char *name);
	int setenv(const char *envname, const char *envval, int overwrite);
	int putenv(char *string);
	__attribute__((malloc)) __attribute__((alloc_size(1))) void *malloc(size_t);

	int system(const char *command);

	void *realloc(void *, size_t);

	long strtol(const char *__restrict, char **__restrict, int);
	long long strtoll(const char *__restrict, char **__restrict, int);
	unsigned long strtoul(const char *__restrict, char **__restrict, int);
	unsigned long long strtoull(const char *__restrict, char **__restrict, int);

	void qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));

#ifdef __cplusplus
}
#endif