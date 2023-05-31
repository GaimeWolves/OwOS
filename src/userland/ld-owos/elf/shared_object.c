#include "shared_object.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <debug.h>

static shared_object_t **s_shared_object_list;
static size_t s_shared_object_count;

void initialize_shared_object_list()
{
	s_shared_object_list = malloc(sizeof(void *));
	*s_shared_object_list = NULL;
	s_shared_object_count = 0;
}

shared_object_t *get_shared_object_by_soname(const char *soname)
{
	for (size_t i = 0; i < s_shared_object_count; i++)
	{
		shared_object_t *so = s_shared_object_list[i];
		if (strcmp(so->so_name, soname) == 0)
			return so;
	}

	return NULL;
}

shared_object_t **get_shared_object_list()
{
	return s_shared_object_list;
}

void add_shared_object(shared_object_t *shared_object)
{
	s_shared_object_list = realloc(s_shared_object_list, sizeof(void *) * (s_shared_object_count + 1));
	s_shared_object_list[s_shared_object_count++] = shared_object;
	s_shared_object_list[s_shared_object_count] = NULL;
}

const char *get_symbol_name_by_id(Elf32_Word id, shared_object_t *shared_object)
{
	Elf32_Sym *sym = &shared_object->sections.dynsym[id];

	if (id == 0)
		return NULL;

	return &shared_object->sections.dynstr[sym->st_name];
}

void *find_symbol_by_name(const char *name, shared_object_t *shared_object)
{
	for (size_t i = 0; i < shared_object->symbol_count; i++)
	{
		Elf32_Sym *sym = &shared_object->sections.dynsym[i];

		// Elf32_Word bind = ELF32_ST_BIND(sym->st_info);
		Elf32_Word type = ELF32_ST_TYPE(sym->st_info);
		const char *symbol_name = &shared_object->sections.dynstr[sym->st_name];

		const char *other = NULL;
		if (sym->st_shndx != STN_UNDEF || (type == STT_FUNC && sym->st_value != 0))
			other = symbol_name;

		if (other && strcmp(name, other) == 0)
			return (void *)((uintptr_t)shared_object->base + sym->st_value);
	}

	return NULL;
}

void *resolve_symbol(const char *name, shared_object_t *shared_object)
{
	static shared_object_t **s_queue = NULL;

	if (!s_queue)
		s_queue = malloc(sizeof(void *) * s_shared_object_count);

	shared_object_t **tail = s_queue;
	shared_object_t **head = s_queue;

	*head++ = shared_object;
	void *symbol = NULL;

	while (head != tail)
	{
		shared_object_t *so = *tail++;
		// DEBUG_PRINTF(3, "Checking %s for %s\n", so->so_name, name);
		symbol = find_symbol_by_name(name, so);

		if (symbol)
			break;

		for (size_t i = 0; i < so->dependency_count; i++)
		{
			shared_object_t *dep = so->dependencies[i];

			if (dep->handled)
				continue;

			dep->handled = true;
			*head++ = so->dependencies[i];
		}
	}

	shared_object_t **iterator = s_queue;
	while (iterator != head)
		(*iterator++)->handled = false;

	return symbol;
}