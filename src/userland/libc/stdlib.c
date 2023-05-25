#include <stdlib.h>

#include <__debug.h>

#include <bits/environ.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

void abort(void)
{
	TRACE("abort()\n", "");
	puts("abort");

	for (;;)
		;
}

int atexit(void (*func)(void))
{
	TRACE("atexit(%p)\n", func);
	(void)func;
	puts("atexit() not implemented");
	abort();
}

int atoi(const char *str)
{
	TRACE("atoi(%p)\n", str);
	(void)str;
	puts("atoi() not implemented");
	abort();
}

void *calloc(size_t nelem, size_t elsize)
{
	TRACE("calloc(%lu, %lu)\n", nelem, elsize);

	void *ptr = malloc(nelem * elsize);

	if (!ptr)
		return NULL;

	memset(ptr, 0, nelem * elsize);

	return ptr;
}

void exit(int status)
{
	TRACE("exit(%d)\n", status);

	(void)status;

	printf("exit(%d)\n", status);

	for (;;)
		;
}

int system(const char *command)
{
	TRACE("system(%s)\n", command);

	(void)command;
	puts("system() not implemented");
	abort();
}

long strtol(const char *str, char **str_end, int base)
{
	TRACE("strtol(%s, %p, %d)\n", str, str_end, base);
	return (long)strtoll(str, str_end, base);
}

long long strtoll(const char *str, char **str_end, int base)
{
	TRACE("strtoll(%s, %p, %d)\n", str, str_end, base);

	while(isspace(str[0]))
		str++;

	long long sign = 1;
	if (str[0] == '-')
	{
		str++;
		sign = -1;
	}

	return sign * (long long)strtoull(str, str_end, base);
}

unsigned long strtoul(const char *str, char **str_end, int base)
{
	TRACE("strtoul(%s, %p, %d)\n", str, str_end, base);
	return (unsigned long)strtoull(str, str_end, base);
}

unsigned long long strtoull(const char *str, char **str_end, int base)
{
	TRACE("strtoull(%s, %p, %d)\n", str, str_end, base);

	if (base < 0 || base == 1 || base > 36)
		return 0;

	if (!str)
		return 0;

	unsigned long long num = 0;

	while(isspace(str[0]))
		str++;

	if (base == 0)
	{
		if (str[0] == '0')
		{
			str++;

			if (tolower(str[0]) == 'x')
			{
				str++;
				base = 16;
			}
			else
			{
				base = 8;
			}
		}
		else
		{
			base = 10;
		}
	}

	while(str)
	{
		if (!isalnum(str[0]))
			break;

		int value;

		if (isdigit(str[0]))
			value = (int)(str[0] - '0');
		else
			value = (int)(toupper(str[0]) - 'A') + 10;

		if (value >= base)
			break;

		num *= (unsigned long long)base;
		num += (unsigned long long)value;

		str++;
	}

	if (str_end)
		*str_end = (char*)str;

	return num;
}

char *getenv(const char *name)
{
	TRACE("getenv(%s)\n", name);

	if (!name)
		return NULL;

	char **envp = environ;
	size_t len = strlen(name);

	while (*envp)
	{
		if (strncmp(name, *envp, len) == 0 && (*envp)[len] == '=')
			return *envp + len + 1;

		envp++;
	}

	return NULL;
}

int setenv(const char *envname, const char *envval, int overwrite)
{
	TRACE("setenv(%s, %s, %d)\n", envname, envval, overwrite);

	(void)envname;
	(void)envval;
	(void)overwrite;
	puts("putenv() not implemented :(");
	abort();
}

int putenv(char *string)
{
	TRACE("putenv(%s)\n", string);

	(void)string;
	puts("setenv() not implemented");
	abort();
}

void qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *))
{
	TRACE("qsort(%p, %lu, %lu, %p)\n", base, nel, width, compar);

	(void)base;
	(void)nel;
	(void)width;
	(void)compar;
	puts("qsort() not implemented");
	abort();
}
