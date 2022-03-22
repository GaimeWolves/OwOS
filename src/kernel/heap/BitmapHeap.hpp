#pragma once

#include <stdint.h>

#include <libk/kcmalloc.hpp>
#include <locking/Mutex.hpp>

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

		heap_block_t *m_first_block{nullptr};
		heap_statistics_t m_stats{0, 0, 0, 0};

	public:
		void expand(uintptr_t addr, uint32_t size, uint32_t block_size);

		void *alloc(size_t size, size_t align = 1);
		void free(void *ptr);

		size_t size(void *ptr);

		const heap_statistics_t &getStatistics() { return m_stats; }

		Locking::Mutex m_heap_mutex{};
	};
} // namespace Kernel::Heap
