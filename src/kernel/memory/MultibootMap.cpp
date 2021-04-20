#include <memory/MultibootMap.hpp>

#include <libk/kcassert.hpp>
#include <multiboot.h>

namespace Kernel::Memory
{
	void MultibootMap::load_map(multiboot_info_t *multiboot_info)
	{
		assert(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP);

		multiboot_mmap_entry_t *entry = (multiboot_mmap_entry_t *)multiboot_info->mmap_addr;

		m_mem_size = 0;
		m_usable_mem_size = 0;

		while ((uintptr_t)entry < (uintptr_t)(multiboot_info->mmap_addr + multiboot_info->mmap_length))
		{
			map_region_t region{
			    .base_address = entry->addr,
			    .length = entry->len,
			    .type = (MultibootRegionType)entry->type,
			};

			uint64_t region_top = region.base_address + region.length;
			if (region_top > m_mem_size)
				m_mem_size = region_top;

			if (region.type == MultibootRegionType::Available)
			{
				if (region_top > m_usable_mem_size)
					m_usable_mem_size = region_top;
			}

			m_entries.push_back(region);
			entry = (multiboot_mmap_entry_t *)(((uintptr_t)entry) + entry->size + sizeof(entry->size));
		}
	}
}; // namespace Kernel::Memory