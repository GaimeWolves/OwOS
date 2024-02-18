#pragma once

#include "shared_object.h"

void do_relocations(shared_object_t *);
void do_relocation(shared_object_t *, Elf32_Rel *);
void do_relocation_with_addend(shared_object_t *, Elf32_Rela *);
