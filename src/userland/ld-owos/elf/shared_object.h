#pragma once

#include <stddef.h>
#include <stdbool.h>

#include "elf.h"

typedef struct shared_object_t
{
	const char *so_name;
	void *base;

	bool bind_now;

	Elf32_Word rel_dyn_count;
	Elf32_Word rela_dyn_count;
	Elf32_Word rel_plt_count;
	Elf32_Word rel_plt_type;

	Elf32_Word symbol_count;

	Elf32_Word preinit_count;
	Elf32_Word init_count;
	Elf32_Word fini_count;

	bool handled;

	struct
	{
		Elf32_Dyn *dynamic;
		Elf32_Rel *rel_dyn;
		Elf32_Rela *rela_dyn;
		void *rel_plt;
		Elf32_Sym *dynsym;
		const char *dynstr;
		void **got;
		Elf32_Addr *preinit_array;
		Elf32_Addr *init;
		Elf32_Addr *init_array;
		Elf32_Addr *fini_array;
		Elf32_Addr *fini;
	} sections;

	size_t dependency_count;
	struct shared_object_t *dependencies[];
} shared_object_t;

void initialize_shared_object_list();
shared_object_t *get_shared_object_by_soname(const char *soname);
shared_object_t **get_shared_object_list();
void add_shared_object(shared_object_t *shared_object);

const char *get_symbol_name_by_id(Elf32_Word id, shared_object_t *shared_object);
void *find_symbol_by_name(const char *name, shared_object_t *shared_object);

void *resolve_symbol(const char *name, shared_object_t *shared_object);
