#include <memory/PhysicalMemoryManager.hpp>

#include <arch/memory.hpp>
#include <panic.hpp>

#include <libk/kcassert.hpp>
#include <libk/kcmalloc.hpp>
#include <libk/kcstdio.hpp>
#include <libk/kcstring.hpp>
#include <libk/kmath.hpp>
#include <libk/kutility.hpp>

extern "C"
{
	extern uintptr_t _virtual_addr;
	extern uintptr_t _physical_addr;
	extern uintptr_t _kernel_start;
	extern uintptr_t _kernel_end;
}

namespace Kernel::Memory
{
	void PhysicalMemoryManager::init(multiboot_info_t *multiboot_info)
	{
		m_memory_map.load_map(multiboot_info);

		m_available_memory = m_memory_map.get_usable_mem_size();
		m_used_memory = m_available_memory;
		m_explicit_used_memory = 0;

		size_t page_count = (m_memory_map.get_usable_mem_size() + PAGE_SIZE - 1) / PAGE_SIZE;
		size_t page_size = PAGE_SIZE;

		for (size_t i = 0; i < LibK::size(m_buddies); i++)
		{
			// Round up to full uint32_t
			size_t bitmap_size = (page_count + 32 - 1) / 32 * 4;
			uint32_t *bitmap = (uint32_t *)kmalloc(bitmap_size);
			memset(bitmap, 0xFF, bitmap_size);

			assert(bitmap);

			m_buddies[i] = {
			    .page_size = page_size,
			    .page_count = page_count,
			    .last_alloc = 0,
			    .bitmap = bitmap,
			};

			page_count /= 2;
			page_size *= 2;
		}

		for (auto &region : m_memory_map.get_entries())
		{
			if (region.type == MultibootRegionType::Available)
			{
				uint32_t start = region.base_address / PAGE_SIZE;
				uint32_t end = (region.base_address + region.length + PAGE_SIZE - 1) / PAGE_SIZE - 1;
				mark_free(start, end);

				m_used_memory -= (end - start + 1) * PAGE_SIZE;
			}
		}

		{
			size_t kernel_size = (uintptr_t)&_kernel_end - (uintptr_t)&_kernel_start;
			uintptr_t phys_addr = (uintptr_t)&_physical_addr;

			uint32_t start = phys_addr / PAGE_SIZE;
			uint32_t end = (phys_addr + kernel_size + PAGE_SIZE - 1) / PAGE_SIZE - 1;

			mark_used(start, end);

			m_used_memory += (end - start + 1) * PAGE_SIZE;
		}
	}

	void PhysicalMemoryManager::set_bit(size_t bit, uint32_t *bitmap)
	{
		size_t idx = bit / 32;
		size_t off = bit % 32;
		bitmap[idx] |= 1 << off;
	}

	bool PhysicalMemoryManager::get_bit(size_t bit, uint32_t *bitmap)
	{
		size_t idx = bit / 32;
		size_t off = bit % 32;
		return (bitmap[idx] & (1 << off)) != 0;
	}

	void PhysicalMemoryManager::clear_bit(size_t bit, uint32_t *bitmap)
	{
		size_t idx = bit / 32;
		size_t off = bit % 32;
		bitmap[idx] &= ~(1 << off);
	}

	void PhysicalMemoryManager::set_dword(size_t bit_start, uint32_t *bitmap)
	{
		size_t idx = bit_start / 32;
		bitmap[idx] = 0xFFFFFFFF;
	}

	uint32_t PhysicalMemoryManager::get_dword(size_t bit_start, uint32_t *bitmap)
	{
		size_t idx = bit_start / 32;
		return bitmap[idx];
	}

	void PhysicalMemoryManager::clear_dword(size_t bit_start, uint32_t *bitmap)
	{
		size_t idx = bit_start / 32;
		bitmap[idx] = 0;
	}

	void PhysicalMemoryManager::mark_used(size_t span_begin, size_t span_end)
	{
		for (auto &buddy : m_buddies)
		{
			for (size_t i = span_begin; i <= span_end; i++)
			{
				if (i % 32 == 0 && span_end - i >= 32)
				{
					set_dword(i, buddy.bitmap);
					i += 31;
					continue;
				}

				set_bit(i, buddy.bitmap);
			}

			span_begin /= 2;
			span_end /= 2;
		}
	}

	void PhysicalMemoryManager::mark_free(size_t span_begin, size_t span_end)
	{
		for (size_t i = span_begin; i <= span_end; i++)
		{
			if (i % 32 == 0 && span_end - i >= 32)
			{
				clear_dword(i, m_buddies[0].bitmap);
				i += 31;
				continue;
			}

			clear_bit(i, m_buddies[0].bitmap);
		}

		for (size_t i = 1; i < LibK::size(m_buddies); i++)
		{
			span_begin /= 2;
			span_end /= 2;

			for (size_t j = span_begin; j <= span_end; j++)
			{
				if (j % 32 == 0 && span_end - j >= 32)
				{
					if (get_dword(j * 2, m_buddies[i - 1].bitmap) == 0 && get_dword(j * 2 + 32, m_buddies[i - 1].bitmap) == 0)
					{
						clear_dword(j, m_buddies[i].bitmap);
						j += 31;
						continue;
					}
				}

				if (!get_bit(j * 2, m_buddies[i - 1].bitmap) && !get_bit(j * 2 + 1, m_buddies[i - 1].bitmap))
					clear_bit(j, m_buddies[i].bitmap);
			}
		}
	}

	size_t PhysicalMemoryManager::get_buddy(size_t size)
	{
		for (size_t i = 0; i < LibK::size(m_buddies); i++)
		{
			if (m_buddies[i].page_size >= size)
				return i;
		}

		return LibK::size(m_buddies) - 1;
	}

	void *PhysicalMemoryManager::alloc(size_t size, uint32_t min_address, uint32_t max_address, uint32_t boundary)
	{
		size_t idx = get_buddy(size);
		auto &buddy = m_buddies[idx];

		size_t needed_page_count = (size + buddy.page_size - 1) / buddy.page_size;

		size_t start = (min_address + buddy.page_size - 1) / buddy.page_size;
		uint32_t real_max_address = LibK::min(max_address, m_memory_map.get_usable_mem_size());
		size_t end = real_max_address / buddy.page_size;

		size_t idx_start = LibK::max(buddy.last_alloc, start);

		for (size_t i = idx_start + 1; i != idx_start; i++)
		{
			if (i >= end)
			{
				i = start;

				if (i == idx_start)
					break;
			}

			uintptr_t phys_addr = i * buddy.page_size;

			if (boundary && phys_addr / boundary != (phys_addr + size) / boundary)
				continue;

			size_t count = 0;
			for (; i + count < buddy.page_count && count < needed_page_count && !get_bit(i, buddy.bitmap); count++)
				;

			if (count < needed_page_count)
			{
				i += count;
				continue;
			}

			size_t needed_small_page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
			size_t small_page_idx = i << idx; // Identical to i * (2 ^^ idx)
			mark_used(small_page_idx, small_page_idx + needed_small_page_count - 1);

			m_used_memory += needed_small_page_count * PAGE_SIZE;
			m_explicit_used_memory += size;

			for (auto &other_buddy : m_buddies)
			{
				other_buddy.last_alloc = small_page_idx + needed_small_page_count - 1;
				small_page_idx /= 2;
				needed_small_page_count /= 2;
			}

			return (void *)phys_addr;
		}

		// TODO: Alert VMM to free up some memory (e.g. free cached objects, swap memory)
		panic("Out Of Memory (OOM) while allocating physical buffer of size %d", size);
	}

	void PhysicalMemoryManager::free(void *page, size_t size)
	{
		size_t page_idx = (uintptr_t)(page) / PAGE_SIZE;
		auto &buddy = m_buddies[get_buddy(size)];
		size_t page_count = (size + buddy.page_size - 1) / PAGE_SIZE;

		mark_free(page_idx, page_idx + page_count - 1);

		m_used_memory -= page_count * PAGE_SIZE;
		m_explicit_used_memory -= size;
	}
} // namespace Kernel::Memory