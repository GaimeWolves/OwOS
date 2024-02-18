#pragma once

#include "shared_object.h"

shared_object_t *parse_elf_headers(void *base, const char *soname);
shared_object_t *load_dependency_by_name(const char *soname);

void call_init_functions(shared_object_t *so);
