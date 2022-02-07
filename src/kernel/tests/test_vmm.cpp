#include <tests.hpp>

#include <memory/VirtualMemoryManager.hpp>

#include <libk/kcstdarg.hpp>
#include <libk/kcstdio.hpp>
#include <libk/kcstring.hpp>
#include <libk/kutility.hpp>

namespace Kernel::Tests
{
	bool test_vmm()
	{
		LibK::printf_test_msg("Virtual Memory Manager (+ Physical Memory Manager)");

		void *allocs[100];
		allocs[0] = Memory::VirtualMemoryManager::instance().alloc_kernel_buffer(0x500);
		allocs[1] = Memory::VirtualMemoryManager::instance().alloc_kernel_buffer(0x500);
		allocs[2] = Memory::VirtualMemoryManager::instance().alloc_kernel_buffer(0x500);
		allocs[3] = Memory::VirtualMemoryManager::instance().alloc_kernel_buffer(0x500);

		for (size_t i = 0; i < LibK::size(allocs) - 4; i += 4)
		{
			allocs[i + 4] = Memory::VirtualMemoryManager::instance().alloc_kernel_buffer(0x500 * (i + 1));
			allocs[i + 5] = Memory::VirtualMemoryManager::instance().alloc_kernel_buffer(0x500 * (i + 2));
			allocs[i + 6] = Memory::VirtualMemoryManager::instance().alloc_kernel_buffer(0x500 * (i + 3));
			allocs[i + 7] = Memory::VirtualMemoryManager::instance().alloc_kernel_buffer(0x500 * (i + 4));
			Memory::VirtualMemoryManager::instance().free(allocs[i]);
			Memory::VirtualMemoryManager::instance().free(allocs[i + 1]);
			Memory::VirtualMemoryManager::instance().free(allocs[i + 2]);
			Memory::VirtualMemoryManager::instance().free(allocs[i + 3]);
		}

		Memory::VirtualMemoryManager::instance().free(allocs[LibK::size(allocs) - 4]);
		Memory::VirtualMemoryManager::instance().free(allocs[LibK::size(allocs) - 3]);
		Memory::VirtualMemoryManager::instance().free(allocs[LibK::size(allocs) - 2]);
		Memory::VirtualMemoryManager::instance().free(allocs[LibK::size(allocs) - 1]);

		LibK::printf_check_msg(true, "Pseudo-random allocations");

		char *buffer = (char *)Memory::VirtualMemoryManager::instance().alloc_kernel_buffer(0x6000);

		// Should both work (i.e. no page fault)
		buffer[0] = 'A';
		buffer[0x5FFF] = 'A';

		Memory::VirtualMemoryManager::instance().free(buffer);

		LibK::printf_check_msg(true, "Write to buffer");

		return true;
	}
} // namespace Kernel::Tests