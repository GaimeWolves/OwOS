#include <sys/internals.h>

char **environ;

void __libc_init(int argc, char **argv)
{
	environ = &argv[argc + 1];

	__stdio_init();
	__malloc_init();
}