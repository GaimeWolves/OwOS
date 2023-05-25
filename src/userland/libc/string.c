#include <string.h>

#include <__debug.h>

#include <stdbool.h>

#define __LIBC_KEEP_DEFS
#include <errno.h>

const char * const error_string[] = {
#define __ENUM_FN(constant, string) string,
    __ENUM_ERRNO_CODES(__ENUM_FN)
#undef __ENUM_FN
};

#include <stdlib.h>
#include <stdio.h>

void *memcpy(void *__restrict s1, const void *__restrict s2, size_t n)
{
	TRACE("memcpy(%p, %p, %lu)\n", s1, s2, n);
	char* dp = s1;
	const char* sp = s2;

	while(n--)
		*dp++ = *sp++;

	return s1;
}

void *memmove(void *dest, const void *src, size_t count)
{
	TRACE("memmove(%p, %p, %lu)\n", dest, src, count);
	const char* sp = src;
	char* dp = dest;

	if (src < dest) {
		for (size_t i = count; i > 0; i--) {
			dp[i - 1] = sp[i - 1];
		}
	}
	else if (src > dest) {
		for (size_t i = 0; i < count; i++) {
			dp[i] = sp[i];
		}
	}

	return dest;
}

void *memset(void *dest, int ch, size_t count)
{
	TRACE("memset(%p, %c, %lu)\n", dest, ch, count);
	char *p = (char *)dest;
	while (count--)
		*p++ = (char)ch;

	return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	TRACE("memcmp(%p, %p, %lu)\n", s1, s2, n);
	(void)s1;
	(void)s2;
	(void)n;
	puts("memcmp() not implemented");
	abort();
}

void *memchr(const void *s, int c, size_t n)
{
	TRACE("memchr(%p, %c, %lu)\n", s, c, n);
	char *ptr = (char *)s;

	while (n-- > 0)
	{
		if (*ptr == c)
			return ptr;

		ptr++;
	}

	return NULL;
}

char *strcat(char *__restrict s1, const char *__restrict s2)
{
	TRACE("strcat(%s, %s)\n", s1, s2);
	size_t offset = strlen(s1);
	memcpy(&s1[offset], s2, strlen(s2) + 1);
	return s1;
}

char *strncat(char *restrict s1, const char *restrict s2, size_t n)
{
	TRACE("strncat(%s, %s, %c)\n", s1, s2, n);
	(void)s1;
	(void)s2;
	(void)n;
	puts("strncat() not implemented");
	abort();
}

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/strchr.html
char *strchr(const char *s, int c)
{
	TRACE("strchr(%s, %c)\n", s, c);
	if (!s)
		return NULL;

	char *str = (char *)s;

	if (c == 0)
		return str + strlen(s);

	while (*str) {
		if (*str == c) {
			return str;
		}

		str++;
	}

	return NULL;
}

int strcmp(const char *s1, const char *s2)
{
	TRACE("strcmp(%s, %s)\n", s1, s2);
	unsigned char c1, c2;

	do
	{
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;

		if (!c1)
			return c1 - c2;
	} while (c1 == c2);

	return c1 - c2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	TRACE("strncmp(%s, %s, %lu)\n", s1, s2, n);
	unsigned char c1, c2;

	while (n-- > 0)
	{
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;

		if (!c1 || c1 != c2)
			return c1 - c2;
	}

	return 0;
}

char *strcpy(char *__restrict dest, const char *__restrict src)
{
	TRACE("strcpy(%s, %s)\n", dest, src);
	char* buf = dest;

	if (!src)
	{
		*dest = '\0';
		return buf;
	}

	while (*src)
		*dest++ = *src++;

	*dest = '\0';

	return buf;
}


size_t strlen(const char *s)
{
	if (!s)
		return 0;

	size_t len = 0;

	while(*s++)
		len++;

	return len;
}

char *strncpy(char *__restrict dest, const char *__restrict src, size_t count)
{
	TRACE("strncmp(%s, %s, %lu)\n", dest, src, count);
	char* buf = dest;
	size_t i;

	for (i = 0; i < count && src[i]; i++)
		dest[i] = src[i];

	for (; i < count; i++)
		dest[i] = '\0';

	return buf;
}

char *strstr(const char *s1, const char *s2)
{
	TRACE("strstr(%s, %s)\n", s1, s2);

	char *str = (char *)s1;

	if (!s2 || s2[0] == '\0')
		return str;

	if (!s1)
		return NULL;

	size_t len = strlen(s2);

	// TODO: There are faster algorithms for this
	while (*str)
	{
		bool found = true;

		for (size_t i = 0; i < len; i++)
		{
			if (str[i] != s2[i])
			{
				found = false;
				break;
			}
		}

		if (found)
			return str;

		str++;
	}

	return NULL;
}

char *strtok(char *restrict s, const char *restrict sep)
{
	TRACE("strtok(%s, %s)\n", s, sep);
	(void)s;
	(void)sep;
	puts("strtok() not implemented");
	abort();
}

char *strpbrk(const char *s1, const char *s2)
{
	TRACE("strpbrk(%s, %s)\n", s1, s2);
	(void)s1;
	(void)s2;
	puts("strpbrk() not implemented");
	abort();
}

size_t strspn(const char *s1, const char *s2)
{
	TRACE("strspn(%s, %s)\n", s1, s2);

	(void)s1;
	(void)s2;
	puts("strspn() not implemented");
	abort();
}

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/strrchr.html
char *strrchr(const char *s, int c)
{
	TRACE("strrchr(%s, %c)\n", s, c);

	if (!s)
		return NULL;

	char *str = (char *)s;

	if (c == 0)
		return str + strlen(s);

	char *last = NULL;

	while (*str) {
		if (*str == c) {
			last = str;
		}

		str++;
	}

	return last;
}

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/strerror.html
char *strerror(int errnum)
{
	TRACE("strerror(%d)\n", errnum);
	// NOTE: const cast allowed because of specification:
	// The application shall not modify the string returned.
	return (char *)error_string[errnum];
}
