#pragma once

#include <bits/guards.h>
#include <sys/types.h>

__LIBC_HEADER_BEGIN

	void *memcpy(void *__restrict, const void *__restrict, size_t);
    void *memmove(void *, const void *, size_t);
	void *memset(void *, int, size_t);
    int memcmp(const void *s1, const void *s2, size_t n);
    void *memchr(const void *s, int c, size_t n);

	char *strcat(char *__restrict, const char *__restrict);
    char *strncat(char *restrict s1, const char *restrict s2, size_t n);
    char *strchr(const char *s, int c);
    int strcmp(const char *, const char *);
    int strncmp(const char *s1, const char *s2, size_t n);
	char *strcpy(char *__restrict, const char *__restrict);
    char *strncpy(char *__restrict, const char *__restrict, size_t);
    char *strstr(const char *s1, const char *s2);
    char *strtok(char *__restrict s, const char *__restrict sep);
    char *strpbrk(const char *s1, const char *s2);
    char *strdup(const char *s);

	size_t strlen(const char *);
    size_t strspn(const char *s1, const char *s2);
    char *strrchr(const char *s, int c);

    char *strerror(int);

__LIBC_HEADER_END