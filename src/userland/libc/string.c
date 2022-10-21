#include <string.h>

#define __LIBC_KEEP_DEFS
#include <errno.h>

const char * const error_string[] = {
#define __ENUM_FN(constant, string) string,
    __ENUM_ERRNO_CODES(__ENUM_FN)
#undef __ENUM_FN
};

void *memcpy(void *__restrict, const void *__restrict, size_t)
{
	return 0;
}

void *memset(void *, int, size_t)
{
	return 0;
}


char *strcat(char *__restrict, const char *__restrict)
{
	return 0;
}

char *strchr(const char *, int)
{
	return 0;
}

char *strcpy(char *__restrict, const char *__restrict)
{
	return 0;
}


size_t strlen(const char *s)
{
	if (!s)
		return 0;

	size_t len = 0;

	while(*(s++))
		len++;

	return len;
}

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/strerror.html
char *strerror(int errnum)
{
	// NOTE: const cast allowed because of specification:
	// The application shall not modify the string returned.
	return (char *)error_string[errnum];
}
