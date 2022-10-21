#include <processes/GlobalScheduler.hpp>

#include <limits.h>

#include <libk/kutility.hpp>

#include <arch/Processor.hpp>

namespace Kernel
{
	thread_t *GlobalScheduler::create_kernel_only_thread(Process *parent, uintptr_t main)
	{
		thread_t *thread = CPU::Processor::create_kernel_thread(main);
		thread->parent_process = parent;
		return thread;
	}

	thread_t *GlobalScheduler::create_userspace_thread(Process *parent, uintptr_t main, Memory::memory_space_t &memorySpace)
	{
		thread_t *thread = CPU::Processor::create_userspace_thread(main, memorySpace);
		thread->parent_process = parent;
		return thread;
	}


	void GlobalScheduler::start_thread(thread_t *thread)
	{
		pick_best_core(thread).m_running_threads.push_back(thread);
	}

	CPU::Processor &GlobalScheduler::pick_best_core(thread_t *thread __unused)
	{
		size_t best_count = SIZE_MAX;
		CPU::Processor *best_core;

		CPU::Processor::enumerate([&best_count, &best_core](CPU::Processor &core) {
			if (core.m_running_threads.size() < best_count)
			{
				best_count = core.m_running_threads.size();
				best_core = &core;
			}

			return true;
		});

		return *best_core;
	}
}