#pragma once

#include <stdint.h>

#include <arch/definitions.hpp>
#include <processes/definitions.hpp>
#include <memory/VirtualMemoryManager.hpp>

namespace Kernel
{
	class GlobalScheduler
	{
	public:
		static thread_t start_kernel_only_thread(uintptr_t main);
		static thread_t start_userspace_thread(uintptr_t main, Memory::memory_space_t &memorySpace);

	private:
		static CPU::Processor &pick_best_core(thread_t *thread);
	};
}