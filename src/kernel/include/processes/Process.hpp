#pragma once

#include <sys/types.h>

#include <libk/kvector.hpp>
#include <libk/AVLTree.hpp>
#include <libk/ErrorOr.hpp>
#include <libk/klist.hpp>
#include <libk/karray.hpp>

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
		[[nodiscard]] int8_t get_exit_signal() const { return m_exit_signal; }
		[[nodiscard]] ProcessState get_state() const { return m_state; }
		void exit(int8_t exit_code, int8_t exit_signal);

		LibK::ErrorOr<pid_t> waitpid(pid_t pid, int *stat_loc, int options);

		LibK::ErrorOr<uintptr_t> set_signal_handler(int8_t signal, uintptr_t function);
		LibK::ErrorOr<void> send_signal(int8_t signal);
		[[nodiscard]] bool has_pending_signal() const { return !m_pending_signals.empty(); }
		void prepare_next_signal(thread_t *thread);

		void exec(File *file, const char **argv, const char **envp);
		Process *fork();
		void adopt(Process *process);

		[[nodiscard]] const LibK::string &get_cwd() const { return m_cwd; }
		void set_cwd(const char *path) { m_cwd = path; }

	private:
		explicit Process(Process *other);

		// TODO: A vector is very bad for opened files
		pid_t m_pid;
		LibK::vector<FileContext> m_opened_files{};
		LibK::vector<thread_t *> m_threads;
		Memory::memory_space_t m_memory_space;
		int8_t m_exit_code{0};
		int8_t m_exit_signal{};
		Process *m_parent{nullptr};
		LibK::vector<Process *> m_children{};
		ProcessState m_state{ProcessState::Running};
		LibK::string m_cwd{"/"};

		// TODO: definitely wrong handling for signals
		LibK::list<int8_t> m_pending_signals{};
		LibK::array<uintptr_t, 32> m_signal_handlers{};
		Memory::memory_region_t m_signal_trampoline{};
	};
}
