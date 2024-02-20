#include "__heap.h"

#include <assert.h>
#include <stdbool.h>
#include <sys/mman.h>

// NOTE: For now this is just adapted from the kernels basic bitmap heap

#define BLOCKS_PER_BYTE 4

static heap_block_t *s_heap;

inline static void    setID(uint8_t *bitmap, uint32_t block, uint8_t id);
inline static uint8_t getID(uint8_t *bitmap, uint32_t block);
inline static uint8_t getFreeID(uint8_t idLeft, uint8_t idRight);

inline static bool      can_align(uint8_t *bitmap, uint32_t block_size, uint32_t block, size_t align);
inline static size_t    get_alignment_offset(uintptr_t address, size_t align);
inline static uintptr_t align_address(uintptr_t address, size_t align);

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

inline static bool can_align(uint8_t *bitmap, uint32_t block_size, uint32_t block, size_t align)
{
	if (align <= block_size)
		return true;

	uintptr_t beginAddr = (uintptr_t)bitmap + block * block_size;

	return beginAddr % align + block_size > align;
}

inline static size_t get_alignment_offset(uintptr_t address, size_t align)
{
	if (align == 1)
		return 0;

	return align - address % align;
}

inline static uintptr_t align_address(uintptr_t address, size_t align)
{
	return address + get_alignment_offset(address, align);
}

void __heap_expand(uintptr_t addr, uint32_t size, uint32_t block_size)
{
	heap_block_t *block = (heap_block_t *)addr;
	block->mem_size = size - sizeof(heap_block_t);
	block->block_size = block_size;

	// Insert into linked list
	block->next = s_heap;
	s_heap = block;

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
}

void *__heap_alloc(size_t size, size_t align)
{
	for (heap_block_t *block = s_heap; block; block = block->next)
	{
		if (block->mem_size - (block->used_blocks * block->block_size) < size)
			continue;

		uint32_t block_count = block->mem_size / block->block_size;
		uint8_t *bitmap = (uint8_t *)(block + 1);

		// Iterate from last_alloc + 1 to last_alloc, wrapping around if the index exceeds the block count
		for (uint32_t start = (block->last_alloc + 1) % block_count; start != block->last_alloc; start++)
		{
			if (start >= block_count)
				start = 0;

			if (!can_align(bitmap, block->block_size, start, align))
				continue;

			uintptr_t address = (uintptr_t)bitmap + start * block->block_size;
			size_t    offset = get_alignment_offset(address, align);
			uint32_t  needed_blocks = (size + offset + block->block_size - 1) / block->block_size;

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

			return (void *)(address + offset);
		}
	}

	return NULL;
}

void __heap_free(void *ptr)
{
	if (!ptr)
		return;

	for (heap_block_t *block = s_heap; block; block = block->next)
	{
		// Pointer is inside block
		if ((uintptr_t)ptr >= (uintptr_t)(block + 1) && (uintptr_t)ptr < (uintptr_t)block + sizeof(heap_block_t) + block->mem_size)
		{
			uint8_t  *bitmap = (uint8_t *)(block + 1);
			uintptr_t offset = (uintptr_t)ptr - (uintptr_t)bitmap;
			uint32_t  block_index = offset / block->block_size;
			uint8_t   id = getID(bitmap, block_index);

			if (id == 0)
				return;

			while (getID(bitmap, block_index) == id && block_index != 0)
				block_index--;

			if (getID(bitmap, block_index) != id)
				block_index++;

			uint32_t block_count = block->mem_size / block->block_size;

			uint32_t i = block_index;
			for (; getID(bitmap, i) == id && i < block_count; i++)
				setID(bitmap, i, 0);

			uint32_t size = i - block_index;
			block->used_blocks -= size;

			break;
		}
	}
}

size_t __heap_size(void *ptr)
{
	for (heap_block_t *block = s_heap; block; block = block->next)
	{
		// Pointer is inside block
		if ((uintptr_t)ptr >= (uintptr_t)(block + 1) && (uintptr_t)ptr < (uintptr_t)block + sizeof(heap_block_t) + block->mem_size)
		{
			uint8_t  *bitmap = (uint8_t *)(block + 1);
			uintptr_t offset = (uintptr_t)ptr - (uintptr_t)bitmap;
			size_t    alignment = offset % block->block_size;
			uint32_t  block_index = offset / block->block_size;
			uint8_t   id = getID(bitmap, block_index);

			uint32_t block_count = block->mem_size / block->block_size;

			uint32_t count = 0;
			for (; getID(bitmap, block_index + count) == id && (block_index + count) < block_count; count++)
				;

			return count * block->block_size - alignment;
		}
	}

	return 0;
}
