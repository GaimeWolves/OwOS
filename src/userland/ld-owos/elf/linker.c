#include "linker.h"

#include <debug.h>
#include <stdio.h>
#include <stdlib.h>

void do_relocations(shared_object_t *shared_object)
{
	DEBUG_PRINTF(1, "Relocating %s\n", shared_object->so_name);

	if (shared_object->sections.rel_dyn)
	{
		for (size_t i = 0; i < shared_object->rel_dyn_count; i++)
		{
			do_relocation(shared_object, &shared_object->sections.rel_dyn[i]);
		}
	}

	if (shared_object->sections.rela_dyn)
	{
		for (size_t i = 0; i < shared_object->rela_dyn_count; i++)
		{
			do_relocation_with_addend(shared_object, &shared_object->sections.rela_dyn[i]);
		}
	}

	if (shared_object->bind_now && shared_object->sections.rel_plt)
	{
		for (size_t i = 0; i < shared_object->rel_plt_count; i++)
		{
			if (shared_object->rel_plt_type == DT_REL)
				do_relocation(shared_object, &((Elf32_Rel *)shared_object->sections.rel_plt)[i]);
			else
				do_relocation_with_addend(shared_object, &((Elf32_Rela *)shared_object->sections.rel_plt)[i]);
		}
	}
}

void do_relocation(shared_object_t *shared_object, Elf32_Rel *relocation)
{
	Elf32_Addr *ptr = (Elf32_Addr *)((uintptr_t)shared_object->base + relocation->r_offset);
	Elf32_Word symbol_index = ELF32_R_SYM(relocation->r_info);
	Elf32_Sym *symbol = &shared_object->sections.dynsym[symbol_index];
	Elf32_Word type = ELF32_ST_TYPE(symbol->st_info);
	Elf32_Word bind = ELF32_ST_BIND(symbol->st_info);
	Elf32_Word reltype = ELF32_R_TYPE(relocation->r_info);
	const char *symbol_name = get_symbol_name_by_id(symbol_index, shared_object);
	void *symbol_ptr = NULL;

	if (symbol->st_shndx != STN_UNDEF || (type == STT_FUNC && symbol->st_value != 0))
	{
		symbol_ptr = (void *)((uintptr_t)shared_object->base + symbol->st_value);
	}

	const char *name = NULL;

	switch (reltype)
	{
	case R_386_RELATIVE:
		*ptr += (Elf32_Addr)shared_object->base;
		// DEBUG_PRINTF(2, "R_386_RELATIVE: %p -> %p\n", relocation->r_offset, *ptr);
		break;
	case R_386_GLOB_DAT:
		name = "R_386_GLOB_DAT";
		__attribute__((fallthrough));
	case R_386_JMP_SLOT:
		if (!name)
			name = "R_386_JMP_SLOT";
		__attribute__((fallthrough));
	case R_386_32:
		if (!name)
			name = "R_386_32";
		__attribute__((fallthrough));
	case R_386_PC32:
		if (!name)
			name = "R_386_PC32";

		if (!symbol_ptr)
			symbol_ptr = resolve_symbol(symbol_name, shared_object);

		DEBUG_PRINTF(2, "%s: %s (%p) -> %p\n", name, symbol_name, ptr, symbol_ptr);

		if (!symbol_ptr && bind != STB_WEAK)
		{
			DEBUG_PRINTF(2, "Undefined symbol %s\n", symbol_name);
			abort();
		}

		if (reltype == R_386_PC32)
			*ptr += (Elf32_Addr)((uintptr_t)symbol_ptr - (Elf32_Addr)ptr);
		else
			*ptr = (Elf32_Addr)symbol_ptr;

		break;
	default:
		DEBUG_PRINTF(0, "TODO: rel type %d\n", ELF32_R_TYPE(relocation->r_info));
		abort();
	}
}

void do_relocation_with_addend(shared_object_t *shared_object, Elf32_Rela *relocation)
{
	(void)shared_object;
	(void)relocation;
	DEBUG_PUTS(0, "TODO: rela relocations");
	abort();
}

