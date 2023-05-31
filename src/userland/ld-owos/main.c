#include <sys/arch/i386/auxv.h>
#include <sys/internals.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

#include <debug.h>
#include <elf/elf.h>
#include <elf/loader.h>
#include <elf/linker.h>

// TODO: Just a rudimentary implementation for early relocations of types R_386_RELATIVE
static void do_early_relocations(int auxvc, auxv_t *auxvp)
{
	void *base = NULL;

	for (int i = 0; i < auxvc; i++)
	{
		if (auxvp[i].a_type == AT_BASE)
		{
			base = auxvp->a_un.a_ptr;
			break;
		}
	}

	assert(base);

	Elf32_Dyn *dynamic_vector = NULL;
	size_t byte_size = 0;

	Elf32_Ehdr *header = (Elf32_Ehdr *)base;
	for (int i = 0; i < header->e_phnum; i++)
	{
		Elf32_Phdr *pheader = (Elf32_Phdr *)((uintptr_t)base + header->e_phoff + header->e_phentsize * i);
		if (pheader->p_type == PT_DYNAMIC)
		{
			dynamic_vector = (Elf32_Dyn *)((uintptr_t)base + pheader->p_vaddr);
			byte_size = pheader->p_memsz;
		}
	}

	assert(dynamic_vector);
	Elf32_Word dyn_count = byte_size / sizeof(Elf32_Dyn);

	Elf32_Rel *relocations = NULL;
	Elf32_Word entry_size = 0;
	Elf32_Word section_size = 0;

	for (Elf32_Word i = 0; i < dyn_count; i++)
	{
		Elf32_Dyn vec = dynamic_vector[i];
		if (vec.d_tag == DT_REL)
		{
			relocations = (Elf32_Rel *)((uintptr_t)base + vec.d_un.d_val);
		}
		else if (vec.d_tag == DT_RELENT)
		{
			entry_size = vec.d_un.d_val;
		}
		else if (vec.d_tag == DT_RELSZ)
		{
			section_size = vec.d_un.d_val;
		}
		else if (vec.d_tag == DT_NULL)
		{
			break;
		}
	}

	assert(relocations);

	Elf32_Word rel_count = section_size / entry_size;

	for (Elf32_Word i = 0; i < rel_count; i++)
	{
		Elf32_Rel relocation = relocations[i];

		Elf32_Addr *ptr = (Elf32_Addr *)((uintptr_t)base + relocation.r_offset);

		switch (ELF32_R_TYPE(relocation.r_info))
		{
		case R_386_RELATIVE:
			*ptr = (Elf32_Addr)((uintptr_t)base + relocation.r_offset);
			break;
		default:
			assert(false);
		}
	}
}

static void debug_print_arguments(int argc, char *argv[], int envc, char *envp[], int auxvc, auxv_t *auxvp)
{
	(void)argv;
	(void)envp;

	DEBUG_PUTS(1, "Arguments:");
	for (int i = 0; i < argc; i++)
	{
		DEBUG_PUTS(2, argv[i]);
	}

	DEBUG_PUTS(1, "Environment:");
	for (int i = 0; i < envc; i++)
	{
		DEBUG_PUTS(2, envp[i]);
	}

	DEBUG_PUTS(1, "Auxiliary vectors:");
	for (int i = 0; i < auxvc; i++)
	{
		auxv_t auxv = auxvp[i];
		switch (auxv.a_type)
		{
		case AT_NULL:
			break;
		case AT_BASE:
			DEBUG_PRINTF(2, "AT_BASE:     %p\n", auxv.a_un.a_ptr);
			break;
		case AT_EXECFD:
			DEBUG_PRINTF(2, "AT_EXECFD:   %ld\n", auxv.a_un.a_val);
			break;
		case AT_PAGESZ:
			DEBUG_PRINTF(2, "AT_PAGESZ:   %ld\n", auxv.a_un.a_val);
			break;
		case AT_ENTRY:
			DEBUG_PRINTF(2, "AT_ENTRY:    %p\n", auxv.a_un.a_ptr);
			break;
		case AT_EXECFN:
			DEBUG_PRINTF(2, "AT_EXECFN:   %s\n", (char *)auxv.a_un.a_ptr);
			break;
		case AT_EXECBASE:
			DEBUG_PRINTF(2, "AT_EXECBASE: %p\n", auxv.a_un.a_ptr);
			break;
		default:
			DEBUG_PRINTF(2, "UNKNOWN:     %d - %ld\n", auxv.a_type, auxv.a_un.a_val);
			break;
		}
	}
}

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
	DEBUG_PUTS(0, "Hello from the dynamic linker");
	DEBUG_PUTS(0, "Stage 0: Initialization");
	DEBUG_PUTS(0, " Parsing stack");

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

	DEBUG_PUTS(0, " Relocating ourselves");
	do_early_relocations(auxvc, auxvp);

	DEBUG_PUTS(0, " Initializing static libc");
	__libc_init(argc, argv, envp);

	debug_print_arguments(argc, argv, envc, envp, auxvc, auxvp);

	initialize_shared_object_list();

	DEBUG_PUTS(0, "Stage 1: Loading dependencies");

	void *execbase = NULL;
	const char *soname = NULL;
	for (int i = 0; i < auxvc; i++)
	{
		auxv_t aux = auxvp[i];

		if (aux.a_type == AT_EXECBASE)
		{
			execbase = aux.a_un.a_ptr;
		}
		else if (aux.a_type == AT_EXECFN)
		{
			soname = (const char *)aux.a_un.a_ptr;
		}
	}

	shared_object_t *executable = parse_elf_headers(execbase, soname);

	DEBUG_PUTS(0, "Stage 2: Perform relocations");
	for (shared_object_t **so = get_shared_object_list(); *so; so++)
		do_relocations(*so);

	DEBUG_PUTS(0, "Stage 3: Init functions");
	call_init_functions(executable);

	DEBUG_PUTS(0, "Stage 4: Calling main");

	typedef void(*main_func_t)(int, char**, char**);

	main_func_t main = NULL;
	for (int i = 0; i < auxvc; i++)
	{
		if (auxvp[i].a_type == AT_ENTRY)
		{
			main = (main_func_t)(auxvp[i].a_un.a_ptr);
			break;
		}
	}

	DEBUG_PRINTF(1, "main found at %p\n", main);
	DEBUG_PRINTF(1, "main(%d, %p, %p)\n", argc, argv, envp);
	main(argc, argv, envp);

	abort();
}