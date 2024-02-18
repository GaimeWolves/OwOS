#include <__printf.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE: This is older code I copied from an old project
// TODO: Review and refactor this

#define ARG(type) (type) va_arg(conversion->ap, type) // Shorthand for va_arg

// Flags used in printf
#define PRINTF_FLAG_LADJUST   0x01 // Left adjust
#define PRINTF_FLAG_SIGN      0x02 // Always print sign
#define PRINTF_FLAG_SPACE     0x04 // Print space on positive value
#define PRINTF_FLAG_ALT       0x08 // Alternative representation
#define PRINTF_FLAG_ZERO      0x10 // Adjust with zeroes
#define PRINTF_FLAG_PRECISION 0x40 // Precision explicitly specified
#define PRINTF_FLAG_PARSING   0x80 // Used in conversion

#define CONVERSION_LENGTH_NONE 0x00 // int
#define CONVERSION_LENGTH_HH   0x01 // char
#define CONVERSION_LENGTH_H    0x02 // short
#define CONVERSION_LENGTH_L    0x04 // long
#define CONVERSION_LENGTH_LL   0x08 // long long
#define CONVERSION_LENGTH_J    0x10 // intmax_t
#define CONVERSION_LENGTH_Z    0x20 // size_t
#define CONVERSION_LENGTH_T    0x40 // ptrdiff_t

static char *uitoa(size_t num, char *buf, size_t base, bool prepend_zeros);
static char *ulltoa(unsigned long long num, char *buf, size_t base, bool prepend_zeros);
static char *lltoa(long long num, char *buf, size_t base, bool prepend_zeros);

static void str_putc(const char ch, printf_conv_t *conv);
static void str_puts(const char *str, size_t size, printf_conv_t *conv);

static void file_putc(const char ch, printf_conv_t *conv);
static void file_puts(const char *str, size_t size, printf_conv_t *conv);

static void parse_length(const char **format, uint8_t *length);

static void write_padding(printf_conv_t *conversion, bool ladjust, char fill);
static void write_string(printf_conv_t *conversion, const char *s, size_t size);
static void write_char(printf_conv_t *conversion, const char c);
static void write_char_seq(printf_conv_t *conversion, const char c, size_t count);

static void printf_parse_number(printf_conv_t *conversion, char *buffer, void *var);
static void printf_parse_varg(printf_conv_t *conversion, char *buffer);
static void printf_parse_flags(printf_conv_t *conversion);
static void printf_parse_width(printf_conv_t *conversion);
static char printf_parse_precision(printf_conv_t *conversion);

static char* ulltoa(unsigned long long num, char* buf, size_t base, bool prepend_zeros)
{
	if (base < 2)
		return 0;

	size_t i = 0;
	do {
		size_t r = num % base;
		buf[i++] = (r > 9) ? (r - 10) + 'A' : r + '0';
		num /= base;
	} while (num > 0 || (prepend_zeros && ((base == 16 && i < 16) || (base == 2 && i < 64))));

	size_t end = i - 1;
	for (size_t k = 0; k <= end / 2; k++) {
		char c = buf[k];
		buf[k] = buf[end - k];
		buf[end - k] = c;
	}

	buf[end + 1] = 0;
	return buf;
}

static char* lltoa(long long num, char* buf, size_t base, bool prepend_zeros)
{
	if (base < 2)
		return 0;

	if (num < 0) {
		*buf = '-';
		num *= -1;
		ulltoa(num, buf + 1, base, prepend_zeros);
	}
	else
		ulltoa(num, buf, base, prepend_zeros);

	return buf;
}

static char* uitoa(size_t num, char* buf, size_t base, bool prepend_zeros)
{
	if (base < 2)
		return 0;

	size_t i = 0;
	do {
		size_t r = num % base;
		buf[i++] = (r > 9) ? (r - 10) + 'A' : r + '0';
		num /= base;
	} while (num > 0 || (prepend_zeros && ((base == 16 && i < 8) || (base == 2 && i < 32))));

	size_t end = i - 1;
	for (size_t k = 0; k <= end / 2; k++) {
		char c = buf[k];
		buf[k] = buf[end - k];
		buf[end - k] = c;
	}

	buf[end + 1] = 0;
	return buf;
}

static void str_putc(const char ch, printf_conv_t *conv)
{
	char **buffer = &conv->buffer;
	*(*buffer)++ = ch;
}

static void str_puts(const char *str, size_t size, printf_conv_t *conv)
{
	char **buffer = &conv->buffer;
	strncpy(*buffer, str, size);
	*buffer += size;
}

static void file_putc(const char ch, printf_conv_t *conv)
{
	FILE *file = conv->file;
	fputc(ch, file);
}

static void file_puts(const char *str, size_t size, printf_conv_t *conv)
{
	FILE *file = conv->file;

	while (size-- > 0)
		fputc(*str++, file);
}

static void write_padding(printf_conv_t *conversion, bool ladjust, char fill)
{
	if (conversion->minimal_width <= 0)
		return;

	if (ladjust != (conversion->flags & PRINTF_FLAG_LADJUST))
		return;

	write_char_seq(conversion, fill, conversion->minimal_width);
}

static void write_string(printf_conv_t *conversion, const char *s, size_t size)
{
	conversion->written += size;
	size = size > conversion->bufsz ? conversion->bufsz : size;

	conversion->puts(s, size, conversion);
	conversion->bufsz -= size;
}

static void write_char(printf_conv_t *conversion, const char c)
{
	conversion->written++;

	if (conversion->bufsz > 1)
	{
		conversion->putc(c, conversion);
		conversion->bufsz--;
	}
}

static void write_char_seq(printf_conv_t *conversion, const char c, size_t count)
{
	conversion->written += count;

	count = count > conversion->bufsz - 1 ? conversion->bufsz - 1 : count;

	while (count--)
	{
		conversion->putc(c, conversion);
		conversion->bufsz--;
	}
}

static void printf_parse_number(printf_conv_t *conversion, char *buffer, void *var)
{
	switch (conversion->specifier)
	{
	case 'd':
	case 'i':
	{
		lltoa(*(long long *)var, buffer, 10, false);
		break;
	}
	case 'u':
	{
		ulltoa(*(unsigned long long *)var, buffer, 10, false);
		break;
	}
	case 'x':
	case 'X':
	{
		ulltoa(*(unsigned long long *)var, buffer, 16, false);
		break;
	}
	case 'o':
	{
		ulltoa(*(unsigned long long *)var, buffer, 8, false);
		break;
	}
	default:
		break;
	}
}

static void printf_parse_varg(printf_conv_t *conversion, char *buffer)
{
	uint64_t var;

	switch (conversion->length)
	{
	case CONVERSION_LENGTH_HH:
	{
		// First convert to int because of integer promotion in var args
		var = (char)ARG(int);
		break;
	}
	case CONVERSION_LENGTH_H:
	{
		// First convert to int because of integer promotion in var args
		var = (short)ARG(int);
		break;
	}
	case CONVERSION_LENGTH_NONE:
	{
		var = ARG(int);
		break;
	}
	case CONVERSION_LENGTH_L:
	{
		var = ARG(long);
		break;
	}
	case CONVERSION_LENGTH_LL:
	{
		var = ARG(unsigned long long);
		break;
	}
	case CONVERSION_LENGTH_J:
	{
		var = ARG(uintmax_t);
		break;
	}
	case CONVERSION_LENGTH_Z:
	{
		var = ARG(size_t);
		break;
	}
	case CONVERSION_LENGTH_T:
	{
		var = ARG(ptrdiff_t);
		break;
	}
	default:
		break;
	}

	printf_parse_number(conversion, buffer, &var);
}

static void printf_parse_flags(printf_conv_t *conversion)
{
	while (conversion->format[0] && conversion->flags & PRINTF_FLAG_PARSING)
	{
		switch (conversion->format[0])
		{
		case '-':
		{
			conversion->flags |= PRINTF_FLAG_LADJUST;
			break;
		}
		case '+':
		{
			conversion->flags |= PRINTF_FLAG_SIGN;
			break;
		}
		case ' ':
		{
			conversion->flags |= PRINTF_FLAG_SPACE;
			break;
		}
		case '#':
		{
			conversion->flags |= PRINTF_FLAG_ALT;
			break;
		}
		case '0':
		{
			conversion->flags |= PRINTF_FLAG_ZERO;
			break;
		}
		default:
		{
			// Clear parsing flag
			conversion->flags ^= PRINTF_FLAG_PARSING;
			conversion->format--; // So we don't skip anything
			break;
		}
		}
		conversion->format++;
	}
}

static void printf_parse_width(printf_conv_t *conversion)
{
	if (conversion->format[0] == '*')
	{
		conversion->minimal_width = ARG(int);
		conversion->format++;
	}
	else if (isdigit(conversion->format[0]))
	{
		char *str_end;
		conversion->minimal_width = (int)strtol(conversion->format, &str_end, 10);
		conversion->format = str_end;
	}

	if (conversion->minimal_width < 0)
	{
		conversion->flags |= PRINTF_FLAG_LADJUST;
		conversion->minimal_width *= -1;
	}
}

// Helper function to parse the specified precision
// Returns the character used for padding a converted integer
static char printf_parse_precision(printf_conv_t *conversion)
{
	char fill = ' ';

	if (conversion->format[0] == '.')
	{
		conversion->format++;
		conversion->flags |= PRINTF_FLAG_PRECISION;

		if (conversion->format[0] == '*')
		{
			conversion->precision = ARG(int);
			conversion->format++;
		}
		else if (isdigit(conversion->format[0]))
		{
			char *str_end;
			conversion->precision = (int)strtol(conversion->format, &str_end, 10);
			conversion->format = str_end;
		}

		if (conversion->precision < 0)
		{
			conversion->precision = 0;
		}
	}
	else if (conversion->flags & PRINTF_FLAG_ZERO)
	{
		fill = '0';
	}

	return fill;
}

// Helper function to parse the length modifier
static void parse_length(const char **format, uint8_t *length)
{
	switch ((*format)[0])
	{
	case 'h':
	{
		if ((*format)[1] == 'h')
		{
			*length = CONVERSION_LENGTH_HH;
			(*format)++;
		}
		else
			*length = CONVERSION_LENGTH_H;
		break;
	}
	case 'l':
	{
		if ((*format)[1] == 'l')
		{
			*length = CONVERSION_LENGTH_LL;
			(*format)++;
		}
		else
			*length = CONVERSION_LENGTH_L;
		break;
	}
	case 'j':
	{
		*length = CONVERSION_LENGTH_J;
		break;
	}
	case 'z':
	{
		*length = CONVERSION_LENGTH_Z;
		break;
	}
	case 't':
	{
		*length = CONVERSION_LENGTH_T;
		break;
	}
	default:
	{
		(*format)--; // So we don't skip anything
		break;
	}
	}
	(*format)++;
}

void __printf_impl(printf_conv_t *conversion)
{
	if ((!conversion->buffer && !conversion->file) || !conversion->format)
	{
		conversion->written = -1;
		return;
	}

	if (conversion->buffer)
	{
		conversion->putc = (printf_putc_callback)str_putc;
		conversion->puts = (printf_puts_callback)str_puts;
	}
	else
	{
		conversion->putc = (printf_putc_callback)file_putc;
		conversion->puts = (printf_puts_callback)file_puts;
	}

	const char **format = &conversion->format;
	size_t *bufsz = &conversion->bufsz;

	if (*bufsz == 0)
		return;

	if (conversion->putc == (printf_putc_callback)str_putc)
		(*bufsz)--;

	while (**format && *bufsz > 1)
	{
		if (**format == '%')
		{
			conversion->specifier = 0;
			conversion->flags = 0;
			conversion->length = 0;
			conversion->precision = 0;
			conversion->minimal_width = 0;

			(*format)++;

			printf_parse_flags(conversion);
			printf_parse_width(conversion);
			char fill = printf_parse_precision(conversion);
			parse_length(format, &conversion->length);

			conversion->specifier = **format;
			switch (conversion->specifier)
			{
			case '%':
			{
				// Full specifier must be %%
				if (conversion->flags | conversion->length | conversion->minimal_width | conversion->precision)
					break;

				write_char(conversion, '%');

				break;
			}
			case 'c':
			{
				conversion->written++;

				// Specification says to first convert to unsigned char
				unsigned char c = (unsigned char)ARG(int);

				conversion->minimal_width--;
				write_padding(conversion, false, ' ');

				write_char(conversion, c);

				write_padding(conversion, true, ' ');

				break;
			}
			case 's':
			{
				char *s = ARG(char *);
				size_t size = strlen(s);

				// Precision declares maximum characters to be printed
				if (conversion->flags & PRINTF_FLAG_PRECISION)
					size = size > (size_t)conversion->precision ? (size_t)conversion->precision : size;

				conversion->minimal_width -= size;
				write_padding(conversion, false, ' ');

				write_string(conversion, s, size);

				write_padding(conversion, true, ' ');

				break;
			}
			case 'd':
			case 'i':
			case 'u':
			{
				// Decimal representation takes up at most 21 characters + sign character
				char repr[22] = {0};
				printf_parse_varg(conversion, repr);

				size_t size = strlen(repr);

				// Sign char calculated based on sign flag and sign of converted number
				char sign = 0;
				if (conversion->specifier != 'u')
				{
					if (repr[0] == '-')
					{
						memmove((void *)repr, (void *)(repr + 1), size + 1);
						sign = '-';
						size--;
					}
					else if (conversion->flags & PRINTF_FLAG_SIGN)
						sign = '+';
					else if (conversion->flags & PRINTF_FLAG_SPACE)
						sign = ' ';
				}

				// Padding calculated depending on precision and zero flag
				size_t padding = 0;
				if (~conversion->flags & PRINTF_FLAG_PRECISION && conversion->flags & PRINTF_FLAG_ZERO)
					padding = size > (size_t)conversion->minimal_width ? 0 : conversion->minimal_width - size;
				else
					padding = size > (size_t)conversion->precision ? 0 : conversion->precision - size;

				// If precision and value both equal 0 nothing is printed
				if (!(conversion->flags & PRINTF_FLAG_PRECISION) || conversion->precision > 0 || repr[0] != '0')
					conversion->minimal_width -= size + padding;

				write_padding(conversion, false, fill);

				// Only print if precision and representation not zero
				if (!(conversion->flags & PRINTF_FLAG_PRECISION) || conversion->precision > 0 || repr[0] != '0')
				{
					if (sign)
						write_char(conversion, sign);

					write_char_seq(conversion, '0', padding);
					write_string(conversion, repr, size);
				}

				write_padding(conversion, true, fill);

				break;
			}
			case 'o':
			{
				// Octal representation takes up at most 22 characters
				char repr[22] = {0};
				printf_parse_varg(conversion, repr);

				size_t size = strlen(repr);

				// Padding calculated depending on precision and zero flag
				size_t padding = 0;
				if (~conversion->flags & PRINTF_FLAG_PRECISION && conversion->flags & PRINTF_FLAG_ZERO)
					padding = size > (size_t)conversion->minimal_width ? 0 : conversion->minimal_width - size;
				else
					padding = size > (size_t)conversion->precision ? 0 : conversion->precision - size;

				// If precision and value both equal 0 nothing is printed
				if (!(conversion->flags & PRINTF_FLAG_PRECISION) || conversion->precision > 0 || repr[0] != '0')
					conversion->minimal_width -= size + padding;

				// Alternative representation requires at least one leading zero
				if (conversion->flags & PRINTF_FLAG_ALT)
					conversion->minimal_width--;

				write_padding(conversion, false, fill);

				// Alternative representation prints a preceding zero
				if (conversion->flags & PRINTF_FLAG_ALT)
					write_char(conversion, '0');

				// Only print if precision and representation not zero
				if (!(conversion->flags & PRINTF_FLAG_PRECISION) || conversion->precision > 0 || repr[0] != '0')
				{
					write_char_seq(conversion, '0', padding);
					write_string(conversion, repr, size);
				}

				write_padding(conversion, true, fill);

				break;
			}
			case 'x':
			case 'X':
			{
				// Hexadecimal representation takes up at most 17 characters
				char repr[17] = {0};
				printf_parse_varg(conversion, repr);

				size_t size = strlen(repr);

				// Upper or lower case depending on specifier;
				for (size_t i = 0; i < size; i++)
					repr[i] = islower(conversion->specifier) ? tolower(repr[i]) : repr[i];

				// Padding calculated depending on precision and zero flag
				size_t padding = 0;
				if (~conversion->flags & PRINTF_FLAG_PRECISION && conversion->flags & PRINTF_FLAG_ZERO)
					padding = size > (size_t)conversion->minimal_width ? 0 : conversion->minimal_width - size;
				else
					padding = size > (size_t)conversion->precision ? 0 : conversion->precision - size;

				if (!(conversion->flags & PRINTF_FLAG_PRECISION) || conversion->precision > 0 || repr[0] != '0')
					conversion->minimal_width -= size + padding;

				if (conversion->flags & PRINTF_FLAG_ALT && repr[0] != '0')
					conversion->minimal_width -= 2;

				write_padding(conversion, false, fill);

				// Alternative representation prints 0x before the value
				if (conversion->flags & PRINTF_FLAG_ALT && repr[0] != '0')
				{
					write_char(conversion, '0');
					write_char(conversion, conversion->specifier); // x or X depending on specifier
				}

				// If precision and value both equal 0 nothing is printed
				if (!(conversion->flags & PRINTF_FLAG_PRECISION) || conversion->precision > 0 || repr[0] != '0')
				{
					write_char_seq(conversion, '0', padding);
					write_string(conversion, repr, size);
				}

				write_padding(conversion, true, fill);

				break;
			}
			case 'n':
			{
				switch (conversion->length)
				{
				case CONVERSION_LENGTH_HH:
				{
					*ARG(char *) = (char)conversion->written;
					break;
				}
				case CONVERSION_LENGTH_H:
				{
					*ARG(short *) = (short)conversion->written;
					break;
				}
				case CONVERSION_LENGTH_NONE:
				{
					*ARG(int *) = (int)conversion->written;
					break;
				}
				case CONVERSION_LENGTH_L:
				{
					*ARG(long *) = (long)conversion->written;
					break;
				}
				case CONVERSION_LENGTH_LL:
				{
					*ARG(long long *) = (long long)conversion->written;
					break;
				}
				case CONVERSION_LENGTH_J:
				{
					*ARG(intmax_t *) = (intmax_t)conversion->written;
					break;
				}
				case CONVERSION_LENGTH_Z:
				{
					*ARG(size_t *) = (size_t)conversion->written;
					break;
				}
				case CONVERSION_LENGTH_T:
				{
					*ARG(ptrdiff_t *) = (ptrdiff_t)conversion->written;
					break;
				}
				default:
					break;
				}

				break;
			}
			case 'p':
			{
				// Implementation defined as 0xDEADBEEF or (nil)
				unsigned long ptr = ARG(unsigned long);
				char repr[11] = {'0', 'x', 0, 0, 0, 0, 0, 0, 0, 0, 0};
				uitoa((size_t)ptr, repr + 2, 16, true);

				conversion->minimal_width -= ptr == 0 ? 5 : 10;

				write_padding(conversion, false, ' ');

				write_string(conversion, ptr == 0 ? "(nil)" : repr, ptr == 0 ? 5 : 10);

				write_padding(conversion, true, ' ');

				break;
			}
			default:
				(*format)--; // So we don't skip anything
				break;
			}
		}
		else
		{
			conversion->written++;
			if (*bufsz > 1)
			{
				conversion->putc(**format, conversion);
				(*bufsz)--;
			}
		}

		(*format)++;
	}

	// Terminate with null byte if sprintf was used
	if (conversion->putc == (printf_putc_callback)str_putc)
		conversion->putc('\0', conversion);
}
