#pragma once

#include <multiboot.h>

#include <libk/kvector.hpp>

namespace Kernel::Memory
{
	enum class MultibootRegionType : uint8_t
	{
		Available = MULTIBOOT_MEMORY_AVAILABLE,
		Reserved = MULTIBOOT_MEMORY_RESERVED,
		ACPIReclaimable = MULTIBOOT_MEMORY_ACPI_RECLAIMABLE,
		NVS = MULTIBOOT_MEMORY_NVS,
		BadRam = MULTIBOOT_MEMORY_BADRAM,
	};

	typedef struct map_region_t
	{
		uint64_t base_address;
		uint64_t length;
		MultibootRegionType type;
	} map_region_t;

	class MultibootMap
	{
	public:
		MultibootMap() = default;
		MultibootMap(multiboot_info_t *multiboot_info) { load_map(multiboot_info); }
		~MultibootMap() = default;

		void load_map(multiboot_info_t *multiboot_info);

		const LibK::vector<map_region_t> &get_entries() const { return m_entries; }
		uint64_t get_memory_size() const { return m_mem_size; }
		uint64_t get_usable_mem_size() const { return m_usable_mem_size; }

	private:
		LibK::vector<map_region_t> m_entries;
		uint64_t m_mem_size;
		uint64_t m_usable_mem_size;
	};
}; // namespace Kernel::Memory
