#include <libk/kcmalloc.hpp>

#include <stddef.h>
#include <stdint.h>

#include <common_attributes.h>

#include <libk/kcassert.hpp>
#include <libk/kcstdio.hpp>
#include <libk/kcstring.hpp>

#include "BitmapHeap.hpp"

#define HEAP_SIZE 2 * 1024 * 1024 // 2MiB initial heap space

__section(".heap") static uint8_t kcmalloc_heap[HEAP_SIZE];

static Kernel::Heap::BitmapHeap heap;
static bool isInitialized = false;

namespace Kernel::Heap
{
	void init()
	{
		if (!isInitialized)
		{
			heap.expand((uintptr_t)kcmalloc_heap, HEAP_SIZE, 16);
			isInitialized = true;
		}
	}

	const heap_statistics_t &getStatistics()
	{
		return heap.getStatistics();
	}
} // namespace Kernel::Heap

extern "C"
{
	void *kmalloc(size_t size, size_t align)
	{
		// TODO: Do error detection and prevention
		void *ptr = heap.alloc(size, align);
		assert(ptr);
		return ptr;
	}

	void *krealloc(void *ptr, size_t size, size_t align)
	{
		size_t current_size = heap.size(ptr);

		if (current_size >= size)
		{
			// TODO: Implement shrinking of heap allocation
			return ptr;
		}

		void *n_ptr = kmalloc(size, align);
		memmove(n_ptr, ptr, current_size);
		kfree(ptr);

		assert(n_ptr);
		return n_ptr;
	}

	void *kcalloc(size_t size, size_t align)
	{
		void *ptr = kmalloc(size, align);
		assert(ptr);
		memset(ptr, 0, size);
		return ptr;
	}

	void kfree(void *ptr)
	{
		// TODO: Do error detection
		heap.free(ptr);
	}
}

// C++ new and delete operators
void *operator new(size_t size)
{
	return kmalloc(size);
}

void *operator new[](size_t size)
{
	return kmalloc(size);
}

void *operator new(size_t size, std::align_val_t al)
{
	return kmalloc(size, static_cast<size_t>(al));
}

void *operator new[](size_t size, std::align_val_t al)
{
	return kmalloc(size, static_cast<size_t>(al));
}

void operator delete(void *p)
{
	kfree(p);
}

void operator delete[](void *p)
{
	kfree(p);
}

void operator delete(void *p, __unused size_t size)
{
	kfree(p);
}

void operator delete[](void *p, __unused size_t size)
{
	kfree(p);
}

void operator delete(void* ptr, __unused size_t, __unused std::align_val_t) noexcept
{
	kfree(ptr);
}

void operator delete[](void* ptr, __unused size_t, __unused std::align_val_t) noexcept
{
	kfree(ptr);
}
