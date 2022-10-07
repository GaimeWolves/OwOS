#pragma once

#include <libk/kvector.hpp>
#include <libk/AVLTree.hpp>

#include <processes/definitions.hpp>
#include <memory/VirtualMemoryManager.hpp>

namespace Kernel
{
	class Process
	{
	public:
		explicit Process(uintptr_t entry, Memory::memory_space_t &memory_space);

		[[nodiscard]] bool is_running() const
		{
			return m_threads.any_of([](const thread_t &thread) {
				return thread.state != ThreadState::Terminated;
			});
		}

		Memory::memory_space_t &get_memory_space() { return m_memory_space; }

	private:
		LibK::vector<thread_t> m_threads;
		Memory::memory_space_t m_memory_space;
	};
}