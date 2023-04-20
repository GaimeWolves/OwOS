#include <sys/internals.h>

#include <unistd.h>
#include <stdio.h>

void __libc_init()
{
	__stdio_init();
	__malloc_init();

	puts("__libc_init");
}