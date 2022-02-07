#pragma once

#include <memory/definitions.hpp>

namespace Kernel
{
	namespace BIOS
	{
		Memory::memory_region_t map_bda_rom();
		Memory::memory_region_t map_ebda_rom();
		Memory::memory_region_t map_bios_rom();
	} // namespace BIOS
} // namespace Kernel
