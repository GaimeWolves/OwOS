#include <sys/arch/i386/auxv.h>

#include <stdio.h>

void _start() __attribute__((used));
void _entry(int, char *) __attribute__((used));

__attribute__((naked)) void _start()
{
	asm(
	    "push $0\n"
	    "jmp _entry@plt\n");
}

// Stack layout is in accordance with http://www.sco.com/developers/devspecs/abi386-4.pdf
__attribute__((noreturn)) void _entry(int argc, char *arg0)
{
	char **argv = &arg0;
	char **envp = &argv[argc + 1];

	int envc = 0;
	char **env = envp;

	while (*(env++))
		envc++;

	auxv_t *auxvp = (auxv_t *)env;

	int auxvc = 0;
	auxv_t *auxv = auxvp;

	while ((auxv++)->a_type != AT_NULL)
		auxvc++;

	puts("Hello from the dynamic linker");
	puts("Listing arguments:");

	for (int i = 0; i < argc; i++)
		puts(argv[i]);

	puts("\nListing environment:");

	for (int i = 0; i < envc; i++)
		puts(envp[i]);

	puts("\nListing auxiliary vector types;");

	char buf[3] = {0};
	for (int i = 0; i < auxvc; i++)
	{
		buf[0] = '0' + auxvp[i].a_type / 10;
		buf[1] = '0' + auxvp[i].a_type % 10;
		puts(buf);
	}

	for (;;)
		;
}