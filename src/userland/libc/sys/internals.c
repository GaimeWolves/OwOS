#include <sys/internals.h>

void __libc_init()
{
	__stdio_init();
}