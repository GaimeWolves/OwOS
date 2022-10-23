#pragma once

#include <bits/guards.h>
#include <bits/FILE.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

__LIBC_HEADER_BEGIN

struct printf_conv_t;

typedef void (*printf_putc_callback)(const char ch, struct printf_conv_t *conv);
typedef void (*printf_puts_callback)(const char *str, size_t size, struct printf_conv_t *conv);

typedef struct printf_conv_t
{
	// State of current conversion
	uint8_t flags;
	int minimal_width;
	int precision;
	uint8_t length;
	char specifier;

	// State of current printf call
	const char *format;
	char *buffer;
	FILE *file;
	size_t bufsz;
	int written;
	va_list ap;

	printf_putc_callback putc;
	printf_puts_callback puts;
} printf_conv_t;

void __printf_impl(printf_conv_t *conversion);

__LIBC_HEADER_END