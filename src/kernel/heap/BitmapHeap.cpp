#include "BitmapHeap.hpp"

#define BLOCKS_PER_BYTE 4

namespace Kernel::Heap
{

    inline static void setID(uint8_t *bitmap, uint32_t block, uint8_t id);
    inline static uint8_t getID(uint8_t *bitmap, uint32_t block);
    inline static uint8_t getFreeID(uint8_t idLeft, uint8_t idRight);

    // Set the 2-bit id in the specified place of the bitmap
    inline static void setID(uint8_t *bitmap, uint32_t block, uint8_t id)
    {
        uint32_t index = block / BLOCKS_PER_BYTE;
        uint32_t offset = block % BLOCKS_PER_BYTE;
        bitmap[index] = (bitmap[index] & ~(0b11 << offset * 2)) | ((id & 0b11) << offset * 2);
    }

    // Get the 2-bit id in the specified place of the bitmap
    inline static uint8_t getID(uint8_t *bitmap, uint32_t block)
    {
        uint32_t index = block / BLOCKS_PER_BYTE;
        uint32_t offset = block % BLOCKS_PER_BYTE;
        return (bitmap[index] & (0b11 << offset * 2)) >> (offset * 2);
    }

    // Get ID for a span of chunks that differs from the adjacent chunks
    inline static uint8_t getFreeID(uint8_t idLeft, uint8_t idRight)
    {
        if (idLeft != 1 && idRight != 1)
            return 1;

        if (idLeft != 2 && idRight != 2)
            return 2;

        return 3;
    }

    BitmapHeap::BitmapHeap()
    {
        m_first_block = nullptr;
        m_stats = {};
    }

    void BitmapHeap::expand(uintptr_t addr, uint32_t size, uint32_t block_size)
    {
        heap_block_t *block = (heap_block_t *)addr;
        block->mem_size = size - sizeof(heap_block_t);
        block->block_size = block_size;

        // Insert into linked list
        block->next = m_first_block;
        m_first_block = block;

        uint32_t block_count = block->mem_size / block->block_size;
        uint8_t *bitmap = (uint8_t *)(block + 1); // Bitmap lies right after header data

        // Round up to full bytes
        uint32_t bitmap_size_in_bytes = (block_count + BLOCKS_PER_BYTE - 1) / BLOCKS_PER_BYTE;

        for (uint32_t i = 0; i < bitmap_size_in_bytes; i++)
            bitmap[i] = 0;

        // Round up to full blocks
        uint32_t bitmap_size_in_blocks = (bitmap_size_in_bytes + block->block_size - 1) / block->block_size;

        for (uint32_t i = 0; i < bitmap_size_in_blocks; i++)
            setID(bitmap, i, 1);

        block->last_alloc = bitmap_size_in_blocks - 1;
        block->used_blocks = bitmap_size_in_blocks;

        // Update statistics
        uint32_t metadata_size = bitmap_size_in_blocks * block->block_size + sizeof(heap_block_t);
        m_stats.size += size;
        m_stats.free += size - metadata_size;
        m_stats.meta += metadata_size;
    }

    void *BitmapHeap::alloc(size_t size)
    {
        // Iterate through all blocks
        for (auto block = m_first_block; block; block = block->next)
        {
            if (block->mem_size - (block->used_blocks * block->block_size) < size)
                continue;

            uint32_t block_count = block->mem_size / block->block_size;
            uint32_t needed_blocks = (size + block->block_size - 1) / block->block_size;
            uint8_t *bitmap = (uint8_t *)(block + 1);

            // Iterate from last_alloc + 1 to last_alloc, wrapping around if the index exceeds the block count
            for (uint32_t start = (block->last_alloc + 1) % block_count; start != block->last_alloc; start++)
            {
                if (start >= block_count)
                    start = 0;

                // The current block is used
                if (getID(bitmap, start))
                    continue;

                uint32_t count = 0;
                for (; getID(bitmap, start + count) == 0 && count < needed_blocks && (start + count) < block_count; count++)
                    ;

                // Not enough space
                if (count < needed_blocks)
                {
                    start += count - 1;
                    continue;
                }

                uint8_t leftID = start == 0 ? 0 : getID(bitmap, start - 1);
                uint8_t rightID = start + count >= block_count ? 0 : getID(bitmap, start + count);

                uint8_t id = getFreeID(leftID, rightID);

                for (uint32_t i = 0; i < count; i++)
                    setID(bitmap, start + i, id);

                block->last_alloc = start + count - 1;
                block->used_blocks += count;

                // Update statistics
                m_stats.free -= needed_blocks * block->block_size;
                m_stats.used += needed_blocks * block->block_size;

                return (void *)((uintptr_t)bitmap + start * block->block_size);
            }
        }

        return nullptr;
    }

    void BitmapHeap::free(void *ptr)
    {
        for (auto block = m_first_block; block; block = block->next)
        {
            // Pointer is inside block
            if ((uintptr_t)ptr >= (uintptr_t)(block + 1) && (uintptr_t)ptr < (uintptr_t)block + sizeof(heap_block_t) + block->mem_size)
            {
                uint8_t *bitmap = (uint8_t *)(block + 1);
                uintptr_t offset = (uintptr_t)ptr - (uintptr_t)bitmap;
                uint32_t block_index = offset / block->block_size;
                uint8_t id = getID(bitmap, block_index);

                uint32_t block_count = block->mem_size / block->block_size;

                uint32_t i = block_index;
                for (; getID(bitmap, i) == id && i < block_count; i++)
                    setID(bitmap, i, 0);

                uint32_t size = i - block_index;
                block->used_blocks -= size;

                // Update statistics
                m_stats.free += size * block->block_size;
                m_stats.used -= size * block->block_size;

                return;
            }
        }

        // TODO: Notify error
        return;
    }

} // namespace Kernel::Heap