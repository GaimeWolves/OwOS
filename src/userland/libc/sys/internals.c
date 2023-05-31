#include <sys/internals.h>

char **environ;

void __libc_init(int argc, char **argv, char **envp)
{
	(void)argc;
	(void)argv;

	environ = envp;

	__stdio_init();
	__malloc_init();
}