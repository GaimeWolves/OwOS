#pragma once

#include <memory/MultibootMap.hpp>
#include <arch/spinlock.hpp>

#include <limits.h>
#include <multiboot.h>
#include <stddef.h>
#include <stdint.h>

namespace Kernel::Memory
{
	class PhysicalMemoryManager
	{
	private:
		typedef struct buddy_t
		{
			size_t page_size;
			size_t page_count;
			size_t last_alloc;
			uint32_t *bitmap;
		} buddy_t;

	public:
		static PhysicalMemoryManager &instance()
		{
			static PhysicalMemoryManager *instance{nullptr};

			if (!instance)
				instance = new PhysicalMemoryManager();

			return *instance;
		}

		PhysicalMemoryManager(PhysicalMemoryManager &) = delete;
		void operator=(const PhysicalMemoryManager &) = delete;

		void init(multiboot_info_t *multiboot_info);

		void reserve(uintptr_t address, size_t size);

		void *alloc(size_t size, uint32_t min_address = 0, uint32_t max_address = UINT32_MAX, uint32_t boundary = 0);
		void free(void *page, size_t size);

	private:
		PhysicalMemoryManager() = default;
		~PhysicalMemoryManager() = default;

		void set_bit(size_t bit, uint32_t *bitmap);
		bool get_bit(size_t bit, uint32_t *bitmap);
		void clear_bit(size_t bit, uint32_t *bitmap);

		void set_dword(size_t bit_start, uint32_t *bitmap);
		uint32_t get_dword(size_t bit_start, uint32_t *bitmap);
		void clear_dword(size_t bit_start, uint32_t *bitmap);

		void mark_used(size_t span_begin, size_t span_end);
		void mark_free(size_t span_begin, size_t span_end);

		size_t get_buddy(size_t size);

		buddy_t m_buddies[8];
		MultibootMap m_memory_map;

		size_t m_available_memory{0};     // Available memory in the system
		size_t m_used_memory{0};          // Memory that is marked as used
		size_t m_explicit_used_memory{0}; // Memory that is explicitly used according to allocations

		// TODO: look into lockless designs
		Locking::Spinlock m_lock{};
	};
} // namespace Kernel::Memory
