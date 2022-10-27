#include <syscall/syscalls.hpp>

#include <memory/VirtualMemoryManager.hpp>

namespace Kernel
{
	uintptr_t syscall$munmap(void *addr, size_t len __unused)
	{
		Memory::VirtualMemoryManager::instance().free(addr);
		return 0;
	}
}