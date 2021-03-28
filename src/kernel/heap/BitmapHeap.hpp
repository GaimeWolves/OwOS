#ifndef BITMAP_HEAP_H
#define BITMAP_HEAP_H 1

#include <stdint.h>

#include <heap/kmalloc.hpp>

namespace Kernel::Heap
{

    class BitmapHeap
    {
    private:
        typedef struct heap_block_t
        {
            struct heap_block_t *next;
            uint32_t mem_size;
            uint32_t used_blocks;
            uint32_t block_size;
            uint32_t last_alloc;
        } heap_block_t;

        heap_block_t *m_first_block;
        heap_statistics_t m_stats;

    public:
        BitmapHeap();
        ~BitmapHeap() = default;

        void expand(uintptr_t addr, uint32_t size, uint32_t block_size);

        void *alloc(size_t size);
        void free(void *ptr);

        const heap_statistics_t &getStatistics() { return m_stats; }
    };

} // namespace Kernel::Heap

#endif // BITMAP_HEAP_H