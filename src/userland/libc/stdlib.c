#include <stdlib.h>

#include <ctype.h>

#include <stdio.h>

void abort(void)
{
	puts("abort");

	for (;;)
		;
}

void exit(int status)
{
	printf("exit(%d)\n", status);

	for (;;)
		;
}

long strtol(const char *str, char **str_end, int base)
{
	return (long)strtoll(str, str_end, base);
}

long long strtoll(const char *str, char **str_end, int base)
{
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
	return (unsigned long)strtoull(str, str_end, base);
}

unsigned long long strtoull(const char *str, char **str_end, int base)
{
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