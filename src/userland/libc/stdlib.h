#pragma once

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

	__attribute__((noreturn)) void abort(void);
	int abs(int);
	int atexit(void (*)(void));
	int atoi(const char *);
	__attribute__((malloc)) __attribute__((alloc_size(1, 2))) void *calloc(size_t, size_t);
	__attribute__((noreturn)) void exit(int);
	void free(void *);
	char *getenv(const char *);
	__attribute__((malloc)) __attribute__((alloc_size(1))) void *malloc(size_t);

	void *realloc(void *, size_t);

	long strtol(const char *__restrict, char **__restrict, int);
	long long strtoll(const char *__restrict, char **__restrict, int);
	unsigned long strtoul(const char *__restrict, char **__restrict, int);
	unsigned long long strtoull(const char *__restrict, char **__restrict, int);

#ifdef __cplusplus
}
#endif