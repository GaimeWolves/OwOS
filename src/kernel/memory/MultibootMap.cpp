#include <memory/MultibootMap.hpp>

#include <multiboot.h>
#include <libk/kassert.hpp>

namespace Kernel::Memory
{
	void MultibootMap::load_map(multiboot_info_t *multiboot_info)
	{
		assert(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP);

		multiboot_mmap_entry_t *entry = (multiboot_mmap_entry_t *)multiboot_info->mmap_addr;

		while ((uintptr_t)entry < (uintptr_t)(multiboot_info->mmap_addr + multiboot_info->mmap_length))
		{
			map_region_t region{
				.base_address = (uintptr_t)entry->addr,
				.length = (size_t)entry->len,
				.type = (RegionType)entry->type,
			};

			m_entries.push_back(region);
			entry = (multiboot_mmap_entry_t *)(((uintptr_t)entry) + entry->size + sizeof(entry->size));
		}
	}

}; // namespace Kernel::Memory