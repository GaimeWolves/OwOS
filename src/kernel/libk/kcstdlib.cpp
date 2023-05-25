#include <libk/kcstdlib.hpp>

#include <libk/kcctype.hpp>

extern "C"
{
	long int strtol(const char* str, char** endptr, int base)
	{
		if (base == 1 || base > 36 || base < 0)
			return 0;

		if (!str)
			return 0;

		long int num = 0;

		while (true)
		{
			if (*str < '0' || *str >= '0' + base)
				break;

			num *= base;
			num += (uint8_t)(*str - '0');
			str++;
		}

		if (endptr)
			*endptr = (char *)str;

		return num;
	}
}