#pragma once

#include <stddef.h>
#include <stdint.h>

extern "C"
{
	size_t strlen(const char *str);
	char *strrev(char *str);
	int strcmp(const char *lhs, const char *rhs);

	void *memset(void *dest, int ch, size_t count);
	void *memmove(void *dest, const void *src, size_t count);
	void *memcpy(void *dest, const void *src, size_t count);
}
