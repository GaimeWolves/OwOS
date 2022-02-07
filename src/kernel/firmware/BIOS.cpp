#include <firmware/BIOS.hpp>

#include <memory/VirtualMemoryManager.hpp>

namespace Kernel
{
	namespace BIOS
	{
		Memory::memory_region_t map_bda_rom()
		{
			return Memory::memory_region_t{
			    .region = {
			        .address = (uintptr_t)Memory::VirtualMemoryManager::instance().map_physical(0x400, 0x100),
			        .size = 0x100,
			    },
			    .phys_address = 0x400,
			    .mapped = true,
			    .present = true,
			    .kernel = true,
			    .is_mmio = false,
			};
		}

		Memory::memory_region_t map_ebda_rom()
		{
			auto bda_region = map_bda_rom();
			auto bda_ptr = bda_region.region.pointer();

			uintptr_t ebda_base_addr = *(uint16_t *)(bda_region.region.address + 0x0E);
			ebda_base_addr <<= 4;

			uint16_t ebda_size = *(uint16_t *)(bda_region.region.address + 0x13);

			Memory::VirtualMemoryManager::instance().free(bda_ptr);

			return Memory::memory_region_t{
			    .region = {
			        .address = (uintptr_t)Memory::VirtualMemoryManager::instance().map_physical(ebda_base_addr, ebda_size),
			        .size = ebda_size,
			    },
			    .phys_address = ebda_base_addr,
			    .mapped = true,
			    .present = true,
			    .kernel = true,
			    .is_mmio = false,
			};
		}

		Memory::memory_region_t map_bios_rom()
		{
			return Memory::memory_region_t{
			    .region = {
			        .address = (uintptr_t)Memory::VirtualMemoryManager::instance().map_physical(0xE0000, 0x20000),
			        .size = 0x20000,
			    },
			    .phys_address = 0xE0000,
			    .mapped = true,
			    .present = true,
			    .kernel = true,
			    .is_mmio = false,
			};
		}

	} // namespace BIOS
} // namespace Kernel
