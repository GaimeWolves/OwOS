#pragma once

#include <stddef.h>
#include <stdint.h>

#include <memory/definitions.hpp>

namespace Kernel::Memory::Arch
{
	// Forward declaration to be overwritten
	struct paging_space_t;
} // namespace Kernel::Memory::Arch

#ifdef ARCH_i686
#	include <arch/i686/memory.hpp>
#endif

namespace Kernel::Memory::Arch
{
	paging_space_t create_kernel_space();
	paging_space_t create_memory_space();

	void map(paging_space_t &memory_space, uintptr_t phys_addr, uintptr_t virt_addr, size_t size, mapping_config_t config);
	void unmap(paging_space_t &memory_space, uintptr_t virt_addr, size_t size);
	uintptr_t as_physical(uintptr_t virt_addr);

	memory_region_t get_kernel_region();
	memory_region_t get_mapping_region();

	void load(paging_space_t &memory_space);
	void invalidate(uintptr_t virtual_address);
} // namespace Kernel::Memory::Arch