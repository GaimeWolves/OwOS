#pragma once

#include <stddef.h>
#include <stdint.h>

extern "C"
{
	size_t strlen(const char *str);
	size_t strnlen_s(const char *str, size_t count);
	char *strrev(char *str);
	int strcmp(const char *lhs, const char *rhs);
	int strncmp(const char *lhs, const char *rhs, size_t count);
	char *strchr(char *str, int character);
	char *strcpy(char *__restrict, const char *__restrict);
	char *strncpy(char *__restrict, const char *__restrict, size_t);

	unsigned long strtoul(const char *str, char **str_end, int base);

	void *memset(void *dest, int ch, size_t count);
	void *memmove(void *dest, const void *src, size_t count);
	void *memcpy(void *dest, const void *src, size_t count);
	int memcmp(const void *ptr1, const void *ptr2, size_t num);

	void *memset32(void *dest, uint32_t word, size_t count);
}

const char *strchr(const char *str, int character);
