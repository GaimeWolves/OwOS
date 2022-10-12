#pragma once

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

	void *memcpy(void *__restrict, const void *__restrict, size_t);
	void *memset(void *, int, size_t);

	char *strcat(char *__restrict, const char *__restrict);
	char *strchr(const char *, int);
	char *strcpy(char *__restrict, const char *__restrict);
	size_t strlen(const char *);

#ifdef __cplusplus
}
#endif