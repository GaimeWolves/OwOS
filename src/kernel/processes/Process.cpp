#include <processes/Process.hpp>

#include <processes/GlobalScheduler.hpp>

namespace Kernel
{
	Process::Process(uintptr_t entry, Memory::memory_space_t &memory_space)
	{
		m_memory_space = memory_space;
		auto main_thread = GlobalScheduler::start_userspace_thread(entry, m_memory_space);
		main_thread.parent_process = this;
		m_threads.push_back(main_thread);
	}
}