#pragma once

#include <bits/guards.h>
#include <sys/types.h>

__LIBC_HEADER_BEGIN

	void *memcpy(void *__restrict, const void *__restrict, size_t);
    void *memmove(void *, const void *, size_t);
	void *memset(void *, int, size_t);

	char *strcat(char *__restrict, const char *__restrict);
	char *strchr(const char *, int);
	char *strcpy(char *__restrict, const char *__restrict);

	size_t strlen(const char *);
    char *strncpy(char *__restrict, const char *__restrict, size_t);

    char *strerror(int);

__LIBC_HEADER_END