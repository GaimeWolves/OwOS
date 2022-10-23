#include <stdlib.h>

#include <ctype.h>

void abort(void)
{
	for (;;)
		;
}

void exit(int)
{
	for (;;)
		;
}

void free(void *)
{

}

void *malloc(size_t)
{
	return 0;
}

long strtol(const char *str, char **str_end, int base)
{
	return (long)strtoll(str, str_end, base);
}

long long strtoll(const char *str, char **str_end, int base)
{
	// Remove preceeding whitespace
	while(isspace(str[0]))
		str++;

	// Remove minus
	long long sign = 1;
	if (str[0] == '-')
	{
		str++;
		sign = -1;
	}

	// Multiply parsed number by sign
	return sign * (long long)strtoull(str, str_end, base);
}

unsigned long strtoul(const char *str, char **str_end, int base)
{
	return (unsigned long)strtoull(str, str_end, base);
}

unsigned long long strtoull(const char *str, char **str_end, int base)
{
	// Base should be 0 or between 2 and 36
	if (base < 0 || base == 1 || base > 36)
		return 0;

	if (!str)
		return 0;

	unsigned long long num = 0;

	// Remove preceeding whitespace
	while(isspace(str[0]))
		str++;

	// Recognize base automatically if set to 0
	if (base == 0)
	{
		// If the number starts with zero
		if (str[0] == '0')
		{
			str++;

			// Is it 0x or 0X? -> Hex number
			if (tolower(str[0]) == 'x')
			{
				str++;
				base = 16;
			}
			else // Otherwise octal number
				base = 8;
		}
		else // Otherwise decimal number
			base = 10;
	}

	// Parse the remaining string
	while(str)
	{
		// Break on invalid character
		if (!isalnum(str[0]))
			break;

		int value;

		// Calculate the digit
		if (isdigit(str[0]))
			value = (int)(str[0] - '0');
		else
			value = (int)(toupper(str[0]) - 'A') + 10;

		// Break if digit not in range of base
		if (value >= base)
			break;

		// Shift number one digit to the left and add new digit
		num *= (unsigned long long)base;
		num += (unsigned long long)value;

		str++;
	}

	// Store end of the number in the string
	if (str_end)
		*str_end = (char*)str;

	return num;
}