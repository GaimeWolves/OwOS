#pragma once

#include <sys/types.h>

#include <libk/kvector.hpp>
#include <libk/AVLTree.hpp>
#include <libk/ErrorOr.hpp>

#include <filesystem/FileContext.hpp>
#include <memory/VirtualMemoryManager.hpp>
#include <processes/definitions.hpp>

namespace Kernel
{
	enum class ProcessState
	{
		Running,
		Zombie,
	};

	class Process
	{
	public:
		Process();

		void start_thread(size_t index);
		void add_thread(thread_t *thread);

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

		[[nodiscard]] pid_t get_pid() const { return m_pid; }

		[[nodiscard]] int8_t get_exit_code() const { return m_exit_code; }
		[[nodiscard]] ProcessState get_state() const { return m_state; }
		void exit(int8_t exit_code);

		LibK::ErrorOr<pid_t> waitpid(pid_t pid, int *stat_loc, int options);

		void exec(File *file, const char **argv, const char **envp);
		Process *fork();
		void adopt(Process *process);

	private:
		explicit Process(Process *other);

		// TODO: A vector is very bad for opened files
		pid_t m_pid;
		LibK::vector<FileContext> m_opened_files{};
		LibK::vector<thread_t *> m_threads;
		Memory::memory_space_t m_memory_space;
		int8_t m_exit_code{0};
		Process *m_parent{nullptr};
		LibK::vector<Process *> m_children{};
		ProcessState m_state{ProcessState::Running};
	};
}