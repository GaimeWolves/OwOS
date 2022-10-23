#include <assert.h>

#include <stdio.h>

__attribute__((noreturn)) void __assertion_failed(const char *condition, const char *, unsigned, const char *)
{
	puts("ASSERTION FAILED");
	puts(condition);

	for(;;)
		;
}