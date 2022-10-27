#include <sys/internals.h>

#include <unistd.h>

void __libc_init()
{
	__stdio_init();
	__malloc_init();
}