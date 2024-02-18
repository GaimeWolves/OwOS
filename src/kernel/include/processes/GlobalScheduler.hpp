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
		static thread_t *create_kernel_only_thread(Process *parent, uintptr_t main);
		static thread_t *create_userspace_thread(Process *parent, Memory::memory_space_t &memorySpace);
		static void start_thread(thread_t *thread);

	private:
		static CPU::Processor &pick_best_core(thread_t *thread);
	};
}
