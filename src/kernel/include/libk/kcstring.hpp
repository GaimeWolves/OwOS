#pragma once

#include <stddef.h>
#include <stdint.h>

extern "C"
{
	size_t strlen(const char *str);
	char *strrev(char *str);
	int strcmp(const char *lhs, const char *rhs);
	int strncmp(const char *lhs, const char *rhs, size_t count);

	unsigned long strtoul(const char *str, char **str_end, int base);

	void *memset(void *dest, int ch, size_t count);
	void *memmove(void *dest, const void *src, size_t count);
	void *memcpy(void *dest, const void *src, size_t count);
}
