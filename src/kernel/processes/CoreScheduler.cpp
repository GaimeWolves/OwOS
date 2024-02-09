#include <processes/CoreScheduler.hpp>

#include <libk/kcstdio.hpp>

#include <logging/logger.hpp>
#include <arch/Processor.hpp>
#include <time/EventManager.hpp>

namespace Kernel
{
	void CoreScheduler::initialize()
	{
		CPU::Processor &core = CPU::Processor::current();
		core.m_idle_thread = Kernel::CPU::Processor::create_kernel_thread((uintptr_t)idle);
		core.m_scheduler_initialized = true;

		// Kickstart local APIC timer if not already running
		Time::EventManager::instance().schedule_event([](){}, SMALLEST_INTERVAL, true);
	}

	void CoreScheduler::tick()
	{
		CPU::Processor &core = CPU::Processor::current();

		thread_t *current_thread = core.m_current_thread;
		thread_t *next_thread = pick_next();

		//if (next_thread == &core.m_idle_thread && (current_thread != &core.m_idle_thread || !current_thread->has_started))
		//	LibK::printf_debug_msg("[CoreScheduler] CPU idling");

		if (current_thread && current_thread->state == ThreadState::Running)
			current_thread->state = ThreadState::Ready;
		next_thread->state = ThreadState::Running;
		core.m_current_thread = next_thread;
		core.m_memory_space = next_thread->parent_process ? &next_thread->parent_process->get_memory_space() : Memory::VirtualMemoryManager::instance().get_kernel_memory_space();

		if (current_thread != next_thread || !next_thread->has_started)
		{
			if (current_thread && current_thread->has_started)
				core.update_thread_context(*current_thread);

			bool has_started = next_thread->has_started;
			next_thread->has_started = true;
			core.enter_critical();
			core.get_exit_function_stack().top() = [next_thread, &core, has_started](){
				if (has_started)
					core.enter_thread_context(*next_thread);
				else
					core.initial_enter_thread_context(*next_thread);
			};
			core.leave_critical();
		}

		if (core.get_irq_counter() <= 1)
			core.process_deferred_queue();
	}

	thread_t *CoreScheduler::pick_next()
	{
		CPU::Processor &core = CPU::Processor::current();

		if (core.m_running_threads.empty())
			return core.m_idle_thread;

		size_t start = core.m_current_thread_index;
		do
		{
			core.m_current_thread_index = (core.m_current_thread_index + 1) % core.m_running_threads.size();
			thread_t *next = core.m_running_threads[core.m_current_thread_index];

			switch (next->state)
			{
			case ThreadState::Ready:
				return next;
			case ThreadState::Blocked:
				if (next->lock->try_lock())
				{
					next->lock = nullptr;
					return next;
				}
				break;
			case ThreadState::Terminated:
				if (next == core.m_current_thread)
					break;

				kfree(reinterpret_cast<void *>(next->kernel_stack));
				core.m_running_threads.erase(core.m_running_threads.begin() + core.m_current_thread_index);

				if (core.m_running_threads.empty())
					return core.m_idle_thread;

				if (core.m_current_thread_index-- == start)
					start = (start + 1) % core.m_running_threads.size();

				break;
			case ThreadState::Running:
				if (core.m_running_threads.size() == 1)
					return next;
			default:
				break;
			}
		} while (core.m_current_thread_index != start && core.m_running_threads.size() > 1);

		return core.m_idle_thread;
	}

	void CoreScheduler::suspend(thread_t *thread)
	{
		thread->state = ThreadState::Suspended;
		CPU::Processor::sleep();
	}

	void CoreScheduler::resume(thread_t *thread)
	{
		thread->state = ThreadState::Ready;
	}

	void CoreScheduler::block(Locking::Mutex *lock)
	{
		CPU::Processor &core = CPU::Processor::current();

		core.m_current_thread->lock = lock;
		core.m_current_thread->state = ThreadState::Blocked;

		CPU::Processor::sleep();
	}

	void CoreScheduler::terminate(thread_t *thread)
	{
		// TODO: Think about dangling locked resources, heap allocations and allocated/mapped pages
		thread->state = ThreadState::Terminated;
	}

	void CoreScheduler::terminate_current()
	{
		terminate(CPU::Processor::current().get_current_thread());
		CPU::Processor::sleep();
	}

	__noreturn void CoreScheduler::idle()
	{
		for (;;)
			CPU::Processor::sleep();
	}
}