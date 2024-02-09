#include <libk/kcstring.hpp>

#include <libk/kcctype.hpp>

extern "C"
{
	size_t strlen(const char *str)
	{
		size_t len = 0;

		while (*(str++))
			len++;

		return len;
	}

	char *strrev(char *str)
	{
		size_t len = strlen(str);

		for (size_t i = 0; i < len / 2; i++)
		{
			char ch = str[i];
			str[i] = str[len - i - 1];
			str[len - i - 1] = ch;
		}

		return str;
	}

	int strcmp(const char *lhs, const char *rhs)
	{
		char c1, c2;

		do
		{
			c1 = *lhs++;
			c2 = *rhs++;
		} while (c1 == c2 && c1);

		return c1 - c2;
	}

	int strncmp(const char *lhs, const char *rhs, size_t count)
	{
		char c1 = *lhs;
		char c2 = *rhs;

		while (c1 == c2 && c1 && count)
		{
			c1 = *lhs++;
			c2 = *rhs++;
			count--;
		}

		return c1 - c2;
	}

	unsigned long strtoul(const char *str, char **str_end, int base)
	{
		if (!str)
			return 0;

		if (base > 36 || base == 1 || base < 0)
			return 0;

		while(isspace(*str))
			str++;

		if (base == 0)
		{
			base = 10;

			if (*str == '0')
			{
				str++;
				base = 8;

				if (tolower(*str) == 'x')
				{
					str++;
					base = 16;
				}
			}
		}

		unsigned long number = 0;

		while(*str)
		{
			if (!isalnum(*str))
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
			number *= (long)base;
			number += (long)value;

			str++;
		}

		if (str_end)
			*str_end = (char*)str;

		return number;
	}

	void *memset(void *dest, int ch, size_t count)
	{
#ifdef ARCH_i686
		asm volatile("cld \n rep stosb" : : "a"(ch), "c"(count), "D"(dest) : "cc");
#else
		char *p = (char *)dest;
		while (count--)
			*p++ = (char)ch;
#endif
		return dest;
	}

	void *memset32(void *dest, uint32_t word, size_t count)
	{
#ifdef ARCH_i686
		asm volatile("cld \n rep stosl" : : "a"(word), "c"(count), "D"(dest) : "cc");
#else
		uint32_t *data = (uint32_t *)dest;

		while(count--)
		{
			*data++ = word;
		}
#endif

		return dest;
	}

	void *memmove(void *dest, const void *src, size_t count)
	{
		const char *sp = (const char *)src;
		char *dp = (char *)dest;

		if (src < dest)
		{
			for (size_t i = count; i > 0; i--)
			{
				dp[i - 1] = sp[i - 1];
			}
		}
		else if (src > dest)
		{
			memcpy(dest, src, count);
		}

		return dest;
	}

	void *memcpy(void *dest, const void *src, size_t n)
	{
#ifdef ARCH_i686
		int d0, d1, d2;
		asm volatile("rep ; movsl\n"
		             "movl %4, %%ecx\n"
		             "andl $3, %%ecx\n"
		             "jz 1f\n"
		             "rep ; movsb\n"
		             "1:"
		             : "=&c"(d0), "=&D"(d1), "=&S"(d2)
		             : "0"(n / 4), "g"(n), "1"((uintptr_t)dest), "2"((uintptr_t)src)
		             : "memory");
#else
		const char *sp = (const char *)src;
		char *dp = (char *)dest;

		for (; n > 0; n--)
			*dp++ = *sp++;
#endif

		return dest;
	}

	int memcmp(const void *ptr1, const void *ptr2, size_t num)
	{
		const char *p1 = (const char *)ptr1;
		const char *p2 = (const char *)ptr2;

		while (*p1 == *p2 && --num)
		{
			p1++;
			p2++;
		}

		return *p1 - *p2;
	}

	char *strchr(char *str, int character)
	{
		return const_cast<char *>(strchr(const_cast<const char *>(str), character));
	}
}

const char *strchr(const char *str, int character)
{
	char ch = (char)character;

	while(*str != ch)
	{
		if (*str++ == 0)
			return nullptr;
	}

	return str;
}

char *strcpy(char *__restrict dest, const char *__restrict src)
{
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

char *strncpy(char *__restrict dest, const char *__restrict src, size_t count)
{
	char* buf = dest;
	size_t i;

	for (i = 0; i < count && src[i]; i++)
		dest[i] = src[i];

	for (; i < count; i++)
		dest[i] = '\0';

	return buf;
}