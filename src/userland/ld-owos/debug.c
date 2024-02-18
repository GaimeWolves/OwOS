#include "debug.h"

#include <stdio.h>

void debug_puts(int depth, const char *str)
{
	if (depth == 0)
		puts(str);
	else
		printf("%*c%s\n", depth, ' ', str);
}

void debug_printf(int depth, const char *fmt, ...)
{
	if (depth != 0)
		printf("%*c", depth, ' ');

	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}
