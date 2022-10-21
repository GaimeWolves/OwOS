#include <processes/Process.hpp>

#include <processes/GlobalScheduler.hpp>

namespace Kernel
{
	Process::Process(uintptr_t entry)
	{
		m_memory_space = Memory::VirtualMemoryManager::create_memory_space();
		m_entrypoint = entry;
		auto main_thread = GlobalScheduler::create_userspace_thread(this, m_entrypoint, m_memory_space);
		m_threads.push_back(main_thread);
	}

	void Process::start()
	{
		GlobalScheduler::start_thread(m_threads[0]);
	}

	int Process::add_file(FileContext &&file)
	{
		for (size_t fd = 0; fd < m_opened_files.size(); fd++)
		{
			if (m_opened_files[fd].is_null())
			{
				m_opened_files[fd] = file;
				return fd;
			}
		}

		int fd = m_opened_files.size();
		m_opened_files.push_back(file);
		return fd;
	}

	void Process::remove_file(int fd)
	{
		assert(fd < (int)m_opened_files.size() && !m_opened_files[fd].is_null());
		m_opened_files[fd].close();
		m_opened_files[fd] = FileContext();
	}
}