#include <tests.hpp>

#include "definitions.hpp"

#include <memory/VirtualMemoryManager.hpp>

#include <libk/kcstdarg.hpp>
#include <logging/logger.hpp>
#include <libk/kcstring.hpp>
#include <libk/kutility.hpp>

namespace Kernel::Tests
{
	bool test_vmm()
	{
		log("TEST", "Virtual Memory Manager (+ Physical Memory Manager)");

		void *allocs[100];
		allocs[0] = (void *)Memory::VirtualMemoryManager::instance().allocate_region(0x500).virt_address;
		allocs[1] = (void *)Memory::VirtualMemoryManager::instance().allocate_region(0x500).virt_address;
		allocs[2] = (void *)Memory::VirtualMemoryManager::instance().allocate_region(0x500).virt_address;
		allocs[3] = (void *)Memory::VirtualMemoryManager::instance().allocate_region(0x500).virt_address;

		for (size_t i = 0; i < LibK::size(allocs) - 4; i += 4)
		{
			allocs[i + 4] = (void *)Memory::VirtualMemoryManager::instance().allocate_region(0x500 * (i + 1)).virt_address;
			allocs[i + 5] = (void *)Memory::VirtualMemoryManager::instance().allocate_region(0x500 * (i + 2)).virt_address;
			allocs[i + 6] = (void *)Memory::VirtualMemoryManager::instance().allocate_region(0x500 * (i + 3)).virt_address;
			allocs[i + 7] = (void *)Memory::VirtualMemoryManager::instance().allocate_region(0x500 * (i + 4)).virt_address;
			Memory::VirtualMemoryManager::instance().free(allocs[i]);
			Memory::VirtualMemoryManager::instance().free(allocs[i + 1]);
			Memory::VirtualMemoryManager::instance().free(allocs[i + 2]);
			Memory::VirtualMemoryManager::instance().free(allocs[i + 3]);
		}

		Memory::VirtualMemoryManager::instance().free(allocs[LibK::size(allocs) - 4]);
		Memory::VirtualMemoryManager::instance().free(allocs[LibK::size(allocs) - 3]);
		Memory::VirtualMemoryManager::instance().free(allocs[LibK::size(allocs) - 2]);
		Memory::VirtualMemoryManager::instance().free(allocs[LibK::size(allocs) - 1]);

		log(get_tag(true), "Pseudo-random allocations");

		char *buffer = (char *)Memory::VirtualMemoryManager::instance().allocate_region(0x6000).virt_address;

		// Should both work (i.e. no page fault)
		buffer[0] = 'A';
		buffer[0x5FFF] = 'A';

		Memory::VirtualMemoryManager::instance().free(buffer);

		log(get_tag(true), "Write to buffer");

		return true;
	}
} // namespace Kernel::Tests
