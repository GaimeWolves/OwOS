#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/assert.html
__attribute__((noreturn)) void __assertion_failed(const char *condition, const char *file, unsigned line, const char *function)
{
	// The information written about the call that failed shall include the text of the argument,
	// the name of the source file, the source file line number, and the name of the enclosing function;
	// the latter are, respectively, the values of the preprocessing macros __FILE__ and __LINE__ and of the identifier __func__.
	//
	// assert() shall write information about the particular call that failed on stderr
	puts("ASSERTION FAILED");
	puts(condition);
	fprintf(stderr, "%s:%d: %s: Assertion '%s' failed.\n", file, line, function, condition);

	// and shall call abort().
	abort();
}