#include <processes/GlobalScheduler.hpp>

#include <limits.h>

#include <libk/kutility.hpp>

#include <arch/Processor.hpp>

namespace Kernel
{
	void GlobalScheduler::start_kernel_only_thread(uintptr_t main)
	{
		thread_t thread = CPU::Processor::create_kernel_thread(main);
		pick_best_core(&thread).m_running_threads.push_back(LibK::move(thread));
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