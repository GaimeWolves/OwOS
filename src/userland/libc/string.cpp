#include <string.h>

void *memcpy(void *__restrict, const void *__restrict, size_t)
{
	return nullptr;
}

void *memset(void *, int, size_t)
{
	return nullptr;
}


char *strcat(char *__restrict, const char *__restrict)
{
	return nullptr;
}

char *strchr(const char *, int)
{
	return nullptr;
}

char *strcpy(char *__restrict, const char *__restrict)
{
	return nullptr;
}


size_t strlen(const char *)
{
	return 0;
}


