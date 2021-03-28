#include <heap/kmalloc.hpp>

#include <stdint.h>
#include <stddef.h>

#include "BitmapHeap.hpp"

#define HEAP_SIZE 2 * 1024 * 1024 // 2MiB initial heap space

__attribute__((section(".heap"))) static uint8_t kmalloc_heap[HEAP_SIZE];

static Kernel::Heap::BitmapHeap heap;

namespace Kernel::Heap
{
    void init()
    {
        heap.expand((uintptr_t)kmalloc_heap, HEAP_SIZE, 16);
    }

    const heap_statistics_t &getStatistics()
    {
        return heap.getStatistics();
    }
} // namespace Kernel::Heap

void *kmalloc(size_t size)
{
    // TODO: Do error detection and prevention
    return heap.alloc(size);
}

void kfree(void *ptr)
{
    // TODO: Do error detection
    heap.free(ptr);
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

void operator delete(void *p)
{
    kfree(p);
}

void operator delete[](void *p)
{
    kfree(p);
}

void operator delete(void *p, size_t size __attribute__((unused)))
{
    kfree(p);
}

void operator delete[](void *p, size_t size __attribute__((unused)))
{
    kfree(p);
}