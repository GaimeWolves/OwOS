#pragma once

#include <bits/guards.h>
#include <stdint.h>
#include <stddef.h>

__LIBC_BEGIN_DECLS

typedef struct heap_block_t
{
	struct heap_block_t *next;
	uint32_t mem_size;
	uint32_t used_blocks;
	uint32_t block_size;
	uint32_t last_alloc;
} heap_block_t;

void __heap_expand(uintptr_t addr, uint32_t size, uint32_t block_size);
void *__heap_alloc(size_t size, size_t align);
void __heap_free(void *ptr);
size_t __heap_size(void *ptr);

__LIBC_END_DECLS
