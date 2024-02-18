#include <processes/Process.hpp>

#include <libk/katomic.hpp>

#include <processes/GlobalScheduler.hpp>
#include <memory/VirtualMemoryManager.hpp>
#include <arch/Processor.hpp>
#include <elf/elf.hpp>
#include <logging/logger.hpp>
#include <tty/VirtualConsole.hpp>

#include "../../userland/libc/signal.h"

namespace Kernel
{
	static LibK::atomic<pid_t> s_pid_counter{0};

	Process::Process()
	{
		m_pid = ++s_pid_counter;
		m_memory_space = Memory::VirtualMemoryManager::create_memory_space();

		for (size_t i = 0; i < m_signal_handlers.size(); i++)
			m_signal_handlers[i] = SIG_DFL;

		uintptr_t signal_trampoline_address;
		uintptr_t signal_trampoline_size;

		CPU::Processor::get_signal_trampoline(&signal_trampoline_address, &signal_trampoline_size);
		auto config = Memory::mapping_config_t { .userspace = true };
		m_signal_trampoline = Memory::VirtualMemoryManager::instance().allocate_region_at(0x1000, signal_trampoline_size, config);
		memcpy(m_signal_trampoline.virt_region().pointer(), (void *)signal_trampoline_address, signal_trampoline_size);
	}

	void Process::start_thread(size_t index)
	{
		GlobalScheduler::start_thread(m_threads[index]);
	}

	void Process::add_thread(thread_t *thread)
	{
		m_threads.push_back(thread);
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

	void Process::exec(File *file, const char **argv, const char **envp)
	{
		thread_t *main_thread = CPU::Processor::current().get_current_thread();

		for (auto thread : m_threads)
		{
			if (thread != main_thread)
				thread->state = ThreadState::Terminated;
		}

		m_threads.clear();
		m_threads.push_back(main_thread);

		// TODO: FD_CLOEXEC

		// TODO: This breaks multi-threaded applications in an SMP environment,
		//       as threads may still be running on different cores.
		Memory::VirtualMemoryManager::free_current_userspace();

		uintptr_t signal_trampoline_address;
		uintptr_t signal_trampoline_size;

		CPU::Processor::get_signal_trampoline(&signal_trampoline_address, &signal_trampoline_size);
		auto config = Memory::mapping_config_t { .userspace = true };
		m_signal_trampoline = Memory::VirtualMemoryManager::instance().allocate_region_at(0x1000, signal_trampoline_size, config);
		memcpy(m_signal_trampoline.virt_region().pointer(), (void *)signal_trampoline_address, signal_trampoline_size);

		assert(ELF::load(this, file, argv, envp, true));

		CPU::interrupt_frame_t *frame = CPU::Processor::current().get_interrupt_frame_stack().top();
		thread_registers_t state = CPU::Processor::create_state_for_exec(frame->eip, frame->old_esp, m_memory_space);
		CPU::Processor::current().get_exit_function_stack().top() = [state, main_thread]() {
			CPU::Processor::current().enter_thread_after_exec(main_thread, state);
		};
	}

	void Process::exit(int8_t exit_code, int8_t exit_signal)
	{
		if (m_pid == 1)
		{
			log("init exited but we don't have shutdown!");

			for (auto thread : m_threads)
				thread->state = ThreadState::Terminated;

			for (;;)
				CPU::Processor::sleep();
		}

		m_exit_code = exit_signal ? exit_signal : exit_code;
		m_exit_signal = exit_signal;

		// TODO: this is a hack
		VirtualConsole::get_current().set_controlling_process(m_parent);

		// TODO: Only close all files after all threads are stopped
		for (auto &file : m_opened_files)
		{
			if (!file.is_null())
			{
				file.close();
				file = FileContext();
			}
		}

		m_opened_files.clear();

		Process *reaper = this;

		// TODO: implement subreapers
		while (reaper->m_parent)
			reaper = reaper->m_parent;

		for (auto child : m_children)
			reaper->adopt(child);

		m_children.clear();

		m_state = ProcessState::Zombie;

		// NOTE: The corresponding core scheduler handles final termination of threads
		for (auto thread : m_threads)
			thread->state = ThreadState::Terminated;
	}

	LibK::ErrorOr<pid_t> Process::waitpid(pid_t pid, int *stat_loc, int options)
	{
		(void)options;

		Process *zombie = nullptr;

		if (pid != -1)
		{
			bool found = false;
			for (auto child : m_children)
			{
				if (child->m_pid == pid)
				{
					found = true;
					break;
				}
			}

			if (!found)
				return ECHILD;
		}

		while (!zombie)
		{
			if (m_children.empty())
				return ECHILD;

			for (auto it = m_children.begin(); it != m_children.end(); ++it)
			{
				Process *child = *it;
				if (child->m_state == ProcessState::Zombie)
				{
					if (pid != -1 && child->m_pid != pid)
						continue;

					m_children.erase(it);
					zombie = child;
					break;
				}
			}

			if (!zombie)
				CPU::Processor::sleep();
		}

		if (stat_loc)
		{
			*stat_loc = 0;
			*stat_loc |= zombie->get_exit_code();
			*stat_loc |= ((int16_t)zombie->get_exit_signal()) << 8;
		}

		return zombie->get_pid();
	}

	LibK::ErrorOr<uintptr_t> Process::set_signal_handler(int8_t signal, uintptr_t function)
	{
		if (signal < 0 || signal >= (int8_t)m_signal_handlers.size())
			return EINVAL;

		uintptr_t old_func = m_signal_handlers[signal];
		m_signal_handlers[signal] = function;

		return old_func;
	}

	LibK::ErrorOr<void> Process::send_signal(int8_t signal)
	{
		if (signal < 0 || signal >= (int8_t)m_signal_handlers.size())
			return EINVAL;

		if (m_signal_handlers[signal] == SIG_IGN)
			return ESUCCESS;

		if (m_signal_handlers[signal] == SIG_DFL)
		{
			// TODO: default action is always termination
			exit(signal, signal);
			return ESUCCESS;
		}

		// TODO: no handling of signal masks, etc. (i don't know how the signal API works)
		m_pending_signals.push_back(signal);

		return ESUCCESS;
	}

	void Process::prepare_next_signal(thread_t *thread)
	{
		uintptr_t signal_handler = 0;
		int8_t signal;
		while (!m_pending_signals.empty())
		{
			signal = m_pending_signals.pop_front();
			uintptr_t handler = m_signal_handlers[signal];

			if (handler != SIG_DFL && handler != SIG_IGN)
			{
				signal_handler = handler;
				break;
			}
		}

		if (!signal_handler)
			return;

		CPU::Processor::do_sigenter(thread, thread->registers, m_signal_trampoline.virt_address, signal_handler, signal, 0, 0);
	}

	void Process::adopt(Process *process)
	{
		process->m_parent = this;
		m_children.push_back(process);

		// TODO: this seems like a hack
		if (process == VirtualConsole::get_current().get_controlling_process())
			VirtualConsole::get_current().set_controlling_process(this);
	}

	Process::Process(Process *other)
	{
		auto *thread_copy = new thread_t;

		for (auto thread : other->m_threads)
		{
			if (thread->state == ThreadState::Terminated || thread != CPU::Processor::current().get_current_thread())
				continue;

			*thread_copy = *thread;
			break;
		}

		CPU::Processor::current().update_thread_context(*thread_copy);

#ifdef ARCH_i686
		thread_copy->registers.frame->set_return_value(0);
#endif

		m_pid = ++s_pid_counter;
		m_cwd = LibK::string(other->m_cwd.c_str());
		m_memory_space = Memory::VirtualMemoryManager::copy_current_memory_space();
		m_parent = other;
		m_signal_handlers = other->m_signal_handlers;
		m_signal_trampoline = other->m_signal_trampoline;

		thread_copy->parent_process = this;

		// TODO: move to Processor
#ifdef ARCH_i686
		thread_copy->registers.cr3 = m_memory_space.paging_space.physical_pd_address;
#endif

		m_threads.push_back(thread_copy);

		for (auto &file : other->m_opened_files)
			m_opened_files.emplace_back(file);

		other->m_children.push_back(this);
	}

	Process *Process::fork()
	{
		auto *new_process = new Process(this);
		GlobalScheduler::start_thread(new_process->m_threads[0]);

		// TODO: this seems like a hack
		if (this == VirtualConsole::get_current().get_controlling_process())
			VirtualConsole::get_current().set_controlling_process(new_process);

		return new_process;
	}
}
