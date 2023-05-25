#include "loader.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <debug.h>

shared_object_t *parse_elf_headers(void *base, const char *soname)
{
	debug_printf(1, "Parsing shared object '%s'\n", soname);

	Elf32_Ehdr *header = (Elf32_Ehdr *)base;

	Elf32_Dyn *dynamic_vector = NULL;
	size_t byte_size = 0;

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

	size_t dependency_count = 0;
	for (Elf32_Word i = 0; i < dyn_count; i++)
	{
		Elf32_Dyn vec = dynamic_vector[i];

		if (vec.d_tag == DT_NEEDED)
			dependency_count++;
		else if (vec.d_tag == DT_NULL)
			break;
	}

	size_t object_size = sizeof(shared_object_t) + sizeof(void *) * dependency_count;
	shared_object_t *object = (shared_object_t *)malloc(object_size);

	object->base = base;
	object->so_name = soname;
	object->sections.dynamic = dynamic_vector;
	object->dependency_count = dependency_count;
	object->preinit_count = 0;
	object->init_count = 0;
	object->fini_count = 0;

	Elf32_Word rel_dyn_size = 0;
	Elf32_Word rel_ent_size = 0;
	Elf32_Word rela_dyn_size = 0;
	Elf32_Word rela_ent_size = 0;
	Elf32_Word rel_plt_size = 0;
	Elf32_Word rel_plt_type = 0;

	bool do_break = false;
	for (Elf32_Word i = 0; i < dyn_count; i++)
	{
		Elf32_Dyn vec = dynamic_vector[i];

		switch (vec.d_tag)
		{
		case DT_HASH:
		{
			Elf32_Word *hash = (Elf32_Word *)((uintptr_t)base + vec.d_un.d_ptr);
			object->symbol_count = hash[1];
			break;
		}
		case DT_REL:
			object->sections.rel_dyn = (Elf32_Rel *)((uintptr_t)base + vec.d_un.d_val);
			break;
		case DT_RELSZ:
			rel_dyn_size = vec.d_un.d_val;
			break;
		case DT_RELENT:
			rel_ent_size = vec.d_un.d_val;
			break;
		case DT_RELA:
			object->sections.rela_dyn = (Elf32_Rela *)((uintptr_t)base + vec.d_un.d_val);
			break;
		case DT_RELASZ:
			rela_dyn_size = vec.d_un.d_val;
			break;
		case DT_RELAENT:
			rela_ent_size = vec.d_un.d_val;
			break;
		case DT_STRTAB:
			object->sections.dynstr = (char *)((uintptr_t)base + vec.d_un.d_val);
			break;
		case DT_SYMTAB:
			object->sections.dynsym = (Elf32_Sym *)((uintptr_t)base + vec.d_un.d_val);
			break;
		case DT_PLTGOT:
			object->sections.got = (void *)((uintptr_t)base + vec.d_un.d_ptr);
			break;
		case DT_PLTREL:
			rel_plt_type = vec.d_un.d_val;
			break;
		case DT_PLTRELSZ:
			rel_plt_size = vec.d_un.d_val;
			break;
		case DT_JMPREL:
			object->sections.rel_plt = (void *)((uintptr_t)base + vec.d_un.d_ptr);
			break;
		case DT_BIND_NOW:
			object->bind_now = true;
			break ;
		case DT_INIT:
			object->sections.init = (Elf32_Addr *)((uintptr_t)base + vec.d_un.d_ptr);
			break;
		case DT_FINI:
			object->sections.fini = (Elf32_Addr *)((uintptr_t)base + vec.d_un.d_ptr);
			break;
		case DT_INIT_ARRAY:
			object->sections.init_array = (Elf32_Addr *)((uintptr_t)base + vec.d_un.d_ptr);
			break;
		case DT_INIT_ARRAYSZ:
			object->init_count = vec.d_un.d_val / sizeof(Elf32_Addr);
			break;
		case DT_FINI_ARRAY:
			object->sections.fini_array = (Elf32_Addr *)((uintptr_t)base + vec.d_un.d_ptr);
			break;
		case DT_FINI_ARRAYSZ:
			object->fini_count = vec.d_un.d_val / sizeof(Elf32_Addr);
			break;
		case DT_PREINIT_ARRAY:
			object->sections.preinit_array = (Elf32_Addr *)((uintptr_t)base + vec.d_un.d_ptr);
			break;
		case DT_PREINIT_ARRAYSZ:
			object->preinit_count = vec.d_un.d_val / sizeof(Elf32_Addr);
			break;
		case DT_NULL:
			do_break = true;
			break;
		}

		if (do_break)
			break;
	}

	// TODO: implement lazy linking
	object->bind_now = true;

	Elf32_Word rel_dyn_count = 0;
	if (object->sections.rel_dyn)
		rel_dyn_count = rel_dyn_size / rel_ent_size;

	Elf32_Word rela_dyn_count = 0;
	if (object->sections.rela_dyn)
		rela_dyn_count = rela_dyn_size / rela_ent_size;

	Elf32_Word rel_plt_count = 0;
	if (object->sections.rel_plt)
		rel_plt_count = rel_plt_size / (rel_plt_type == DT_REL ? rel_ent_size : rela_ent_size);

	object->rel_plt_type = rel_plt_type;
	object->rel_plt_count = rel_plt_count;
	object->rel_dyn_count = rel_dyn_count;
	object->rela_dyn_count = rela_dyn_count;

	debug_puts(2, "Sections:");
	debug_printf(3, ".dynamic: %p\n", object->sections.dynamic);
	debug_printf(3, ".dynsym: %p\n", object->sections.dynsym);
	debug_printf(3, ".dynstr: %p\n", object->sections.dynstr);
	debug_printf(3, ".got: %p\n", object->sections.got);
	debug_printf(3, ".rel_dyn: %p\n", object->sections.rel_dyn);
	debug_printf(3, ".rela_dyn: %p\n", object->sections.rela_dyn);
	debug_printf(3, ".rel(a)_plt: %p\n", object->sections.rel_plt);
	debug_printf(3, ".init: %p\n", object->sections.init);
	debug_printf(3, ".init_array: %p\n", object->sections.init_array);
	debug_printf(3, ".fini: %p\n", object->sections.fini);
	debug_printf(3, ".fini_array: %p\n", object->sections.fini_array);
	debug_printf(3, ".preinit_array: %p\n", object->sections.preinit_array);

	debug_puts(2, "Relocation counts:");
	debug_printf(3, ".rel_dyn: %d\n", object->rel_dyn_count);
	debug_printf(3, ".rela_dyn: %d\n", object->rela_dyn_count);
	debug_printf(3, ".rel(a)_plt: %d\n", object->rel_plt_count);

	debug_puts(2, "Function counts:");
	debug_printf(3, ".init_array: %d\n", object->init_count);
	debug_printf(3, ".fini_array: %d\n", object->fini_count);
	debug_printf(3, ".preinit_array: %d\n", object->preinit_count);

	add_shared_object(object);

	debug_puts(2, "Dependencies:");

	for (Elf32_Word i = 0; i < dyn_count; i++)
	{
		Elf32_Dyn vec = dynamic_vector[i];
		if (vec.d_tag == DT_NEEDED)
		{
			debug_puts(3, &object->sections.dynstr[vec.d_un.d_val]);
		}
	}

	if (dependency_count == 0)
		debug_puts(3, "(none)");

	int index = 0;
	for (Elf32_Word i = 0; i < dyn_count; i++)
	{
		Elf32_Dyn vec = dynamic_vector[i];

		if (vec.d_tag == DT_NEEDED)
		{
			shared_object_t *dependency = load_dependency_by_name(&object->sections.dynstr[vec.d_un.d_val]);
			object->dependencies[index++] = dependency;
		}

		if (vec.d_tag == DT_NULL)
			break;
	}

	return object;
}

shared_object_t *load_dependency_by_name(const char *soname)
{
	debug_printf(1, "Loading shared object '%s'\n", soname);
	shared_object_t *shared_object = get_shared_object_by_soname(soname);

	if (shared_object)
		return shared_object;

	// TODO: correct library search
	char *path;
	if (strcmp(soname, "libc.so.1") == 0)
	{
		size_t path_length = strlen("/lib/") + strlen(soname) + 1;
		path = malloc(path_length);
		memset(path, 0, path_length);
		strcat(path, "/lib/");
		strcat(path, soname);
	}
	else
	{
		size_t path_length = strlen("/usr/lib/") + strlen(soname) + 1;
		path = malloc(path_length);
		memset(path, 0, path_length);
		strcat(path, "/usr/lib/");
		strcat(path, soname);
	}

	FILE *file = fopen(path, "r");
	free(path);
	int fd = fileno(file);

	struct stat file_info;
	fstat(fd, &file_info);

	off_t size = file_info.st_size;
	Elf32_Ehdr *header = mmap(0, size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	read(fd, header, (size_t)size);

	size_t image_size = 0;

	for (int i = 0; i < header->e_phnum; i++)
	{
		Elf32_Phdr *pheader = (Elf32_Phdr *)((uintptr_t)header + header->e_phoff + header->e_phentsize * i);
		if (pheader->p_type == PT_LOAD)
		{
			size_t high_address = pheader->p_vaddr + pheader->p_memsz;
			if (high_address > image_size)
				image_size = high_address;
		}
	}

	void *image_base = mmap(0, image_size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	debug_printf(2, "Loading at %p. Image size: %d\n", image_base, image_size);

	for (int i = 0; i < header->e_phnum; i++)
	{
		Elf32_Phdr *pheader = (Elf32_Phdr *)((uintptr_t)header + header->e_phoff + header->e_phentsize * i);
		if (pheader->p_type == PT_LOAD)
		{
			memcpy((void *)((uintptr_t)image_base + pheader->p_vaddr), (void *)((uintptr_t)header + pheader->p_offset), pheader->p_filesz);
			memset((void *)((uintptr_t)image_base + pheader->p_vaddr + pheader->p_filesz), 0, pheader->p_memsz - pheader->p_filesz);
		}
	}

	munmap(header, size);
	fclose(file);

	shared_object = parse_elf_headers(image_base, soname);

	return shared_object;
}

typedef void (*init_func_t)(void);

static void call_function(Elf32_Addr func)
{
	if (func != 0 && (int)func != -1)
	{
		debug_printf(2, "Calling function %p\n", func);
		((init_func_t)func)();
	}
}

static void recursive_call_init_functions(shared_object_t *so)
{
	debug_printf(2, "Calling init functions of %s\n", so->so_name);
	for (size_t i = 0; i < so->dependency_count; i++)
	{
		if (so->dependencies[i]->handled)
			continue;

		recursive_call_init_functions(so->dependencies[i]);
	}

	if (so->sections.init)
		call_function((Elf32_Addr)so->sections.init);

	for (size_t i = 0; i < so->init_count; i++)
		call_function(so->sections.init_array[i]);

	so->handled = true;
}

void call_init_functions(shared_object_t *so)
{
	debug_puts(1, "Calling preinit functions");

	for (size_t i = 0; i < so->preinit_count; i++)
		call_function(so->sections.preinit_array[i]);

	debug_puts(1, "Calling init functions recursively");

 	for (size_t i = 0; i < so->dependency_count; i++)
	{
		if (so->dependencies[i]->handled)
			continue;

		recursive_call_init_functions(so->dependencies[i]);
	}

	shared_object_t **iterator = get_shared_object_list();
	while (*iterator)
		(*iterator++)->handled = false;
}