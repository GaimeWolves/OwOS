#pragma once

#include <stdint.h>

#include <libk/kcstddef.hpp>

namespace Kernel::Heap
{
	typedef struct heap_statistics_t
	{
		size_t size; // Total memory used by a heap
		size_t free; // Free memory usable by the user
		size_t used; // Used memory (only user)
		size_t meta; // Used memory (only metadata)
	} heap_statistics_t;

	void init();
	const heap_statistics_t &getStatistics();
} // namespace Kernel::Heap

extern "C"
{
	__attribute__((malloc)) void *kmalloc(size_t size, size_t align = 1);
	void *krealloc(void *ptr, size_t size, size_t align = 1);
	void *kcalloc(size_t size, size_t align = 1);

	void kfree(void *ptr);
}
