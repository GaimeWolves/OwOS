#include <sys/internals.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define HEAP_REGION_SIZE (2 << 16)
#define HEAP_BLOCK_SIZE 0x16

#include <stdio.h>

#include "__heap.h"

void __malloc_init()
{
	void *initial_heap = mmap((void *)0x100000, HEAP_REGION_SIZE, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	assert((int)initial_heap != MAP_FAILED);
	__heap_expand((uintptr_t)initial_heap, HEAP_REGION_SIZE, HEAP_BLOCK_SIZE);
}

void *malloc(size_t size)
{
	void *ptr = __heap_alloc(size, sizeof(int));

	if (!ptr)
		puts("alloc failed");

	return ptr;
}

void free(void *ptr)
{
	__heap_free(ptr);
}

void *realloc(void *ptr, size_t size)
{
	size_t current_size = __heap_size(ptr);

	if (current_size >= size)
		return ptr;

	void *n_ptr = malloc(size);
	memmove(n_ptr, ptr, current_size);
	free(ptr);

	return n_ptr;
}
