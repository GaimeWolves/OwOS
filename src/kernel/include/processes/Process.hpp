#pragma once

#include <libk/kvector.hpp>
#include <libk/AVLTree.hpp>

#include <processes/definitions.hpp>
#include <memory/VirtualMemoryManager.hpp>
#include <filesystem/FileContext.hpp>

namespace Kernel
{
	class Process
	{
	public:
		explicit Process(uintptr_t entry);

		void start();

		[[nodiscard]] bool is_running() const
		{
			return m_threads.any_of([](const thread_t *thread) {
				return thread->state != ThreadState::Terminated;
			});
		}

		Memory::memory_space_t &get_memory_space() { return m_memory_space; }

		thread_t *get_thread_by_index(size_t index) { return m_threads[index]; }

		int add_file(FileContext &&file);
		FileContext &get_file_by_index(int index) { return m_opened_files[index]; }
		void remove_file(int fd);

	private:
		LibK::vector<FileContext> m_opened_files{};
		LibK::vector<thread_t *> m_threads;
		Memory::memory_space_t m_memory_space;
		uintptr_t m_entrypoint{};
	};
}