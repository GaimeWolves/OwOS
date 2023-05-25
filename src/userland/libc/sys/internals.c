#include <sys/internals.h>

#include <stdio.h>

char **environ;

void __libc_init(int argc, char **argv, char **envp)
{
	(void)argc;
	(void)argv;

	environ = envp;

	__stdio_init();
	__malloc_init();

	puts("__libc_init");
}