#pragma once

#include <stdint.h>

#include <arch/definitions.hpp>
#include <processes/definitions.hpp>

namespace Kernel
{
	class GlobalScheduler
	{
	public:
		static void start_kernel_only_thread(uintptr_t main);

	private:
		static CPU::Processor &pick_best_core(thread_t *thread);
	};
}