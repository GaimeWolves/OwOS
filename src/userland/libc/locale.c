#include <locale.h>

#include <__debug.h>

#include <stdlib.h>
#include <stdio.h>

char *setlocale(int category, const char *locale)
{
	TRACE("setlocale(%d, %s)\n", category, (locale ? locale : "(nil)"));

	(void)category;
	(void)locale;
	puts("setlocale() not fully implemented");

	if (!locale)
		return "C";

	abort();
}