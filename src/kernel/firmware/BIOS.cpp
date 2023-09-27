#include <firmware/BIOS.hpp>

#include <memory/VirtualMemoryManager.hpp>

namespace Kernel
{
	namespace BIOS
	{
		Memory::memory_region_t map_bda_rom()
		{
			return Memory::VirtualMemoryManager::instance().map_region(0x400, 0x100);
		}

		Memory::memory_region_t map_ebda_rom()
		{
			auto bda_region = map_bda_rom();

			uintptr_t ebda_base_addr = *(uint16_t *)(bda_region.virt_address + 0x0E);
			ebda_base_addr <<= 4;

			uint8_t ebda_size_low = *(uint8_t *)(bda_region.virt_address + 0x13);
			uint8_t ebda_size_high = *(uint8_t *)(bda_region.virt_address + 0x14);
			uint16_t ebda_size = ((uint16_t)(ebda_size_high) << 8) | (uint16_t)(ebda_size_low);

			Memory::VirtualMemoryManager::instance().free(bda_region);

			return Memory::VirtualMemoryManager::instance().map_region(ebda_base_addr, ebda_size);
		}

		Memory::memory_region_t map_bios_rom()
		{
			return Memory::VirtualMemoryManager::instance().map_region(0xE0000, 0x20000);
		}

	} // namespace BIOS
} // namespace Kernel
