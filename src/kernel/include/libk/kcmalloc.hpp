#pragma once

#include <stddef.h>
#include <stdint.h>

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

// These must lie in the std namespace
namespace std
{
	enum class align_val_t : size_t {};
};

extern "C"
{
	__attribute__((malloc)) void *kmalloc(size_t size, size_t align = 1);
	void *krealloc(void *ptr, size_t size, size_t align = 1);
	void *kcalloc(size_t size, size_t align = 1);

	void kfree(void *ptr);
}

// C++ placement new and delete operators
inline void *operator new(size_t, void *p) { return p; }
inline void *operator new[](size_t, void *p) { return p; }

inline void operator delete(void *, void *){}
inline void operator delete[](void *, void *){}
