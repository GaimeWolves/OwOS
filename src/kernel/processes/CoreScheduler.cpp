#include <processes/CoreScheduler.hpp>

#include <libk/kcstdio.hpp>

#include <time/EventManager.hpp>
#include <arch/Processor.hpp>

namespace Kernel
{
	void CoreScheduler::initialize()
	{
		CPU::Processor &core = CPU::Processor::current();
		core.m_idle_thread = core.create_kernel_thread((uintptr_t)idle);
		Time::EventManager::instance().schedule_event(tick, 10 * 1000 * 1000, true);

		core.m_scheduler_initialized = true;
	}

	void CoreScheduler::tick()
	{
		CPU::Processor &core = CPU::Processor::current();

		thread_t *current_thread = core.m_current_thread;

		if (current_thread && current_thread->has_started)
			core.update_thread_context(*current_thread);

		thread_t *next_thread = pick_next();

		//if (next_thread == &core.m_idle_thread && (current_thread != &core.m_idle_thread || !current_thread->has_started))
		//	LibK::printf_debug_msg("[CoreScheduler] CPU idling");

		if (current_thread && current_thread->state == ThreadState::Running)
			current_thread->state = ThreadState::Waiting;
		next_thread->state = ThreadState::Running;
		core.m_current_thread = next_thread;

		if (current_thread != next_thread || !next_thread->has_started)
		{
			bool has_started = next_thread->has_started;
			next_thread->has_started = true;
			//if (core.m_current_thread_index < core.m_running_threads.size())
			//	LibK::printf_debug_msg("Entering thread #%d context", core.m_current_thread_index);
			core.set_exit_function([next_thread, &core, has_started](){
				Time::EventManager::instance().schedule_event(tick, SMALLEST_INTERVAL, true);

				if (has_started)
					core.enter_thread_context(*next_thread);
				else
					core.initial_enter_thread_context(*next_thread);
			});

			return;
		}

		Time::EventManager::instance().schedule_event(tick, SMALLEST_INTERVAL, true);
	}

	thread_t *CoreScheduler::pick_next()
	{
		CPU::Processor &core = CPU::Processor::current();

		if (core.m_running_threads.empty())
			return &core.m_idle_thread;

		size_t start = core.m_current_thread_index;
		do
		{
			core.m_current_thread_index = (core.m_current_thread_index + 1) % core.m_running_threads.size();
			thread_t *next = &core.m_running_threads[core.m_current_thread_index];

			switch (next->state)
			{
			case ThreadState::Waiting:
				return next;
			case ThreadState::Blocked:
				if (!next->lock->is_locked() && next->lock->try_lock())
				{
					next->lock = nullptr;
					return next;
				}
				break;
			case ThreadState::Terminated:
				kfree(reinterpret_cast<void *>(next->kernel_stack));
				core.m_running_threads.erase(core.m_running_threads.begin() + core.m_current_thread_index);

				if (core.m_running_threads.empty())
					return &core.m_idle_thread;

				if (core.m_current_thread_index-- == start)
					start = (start + 1) % core.m_running_threads.size();

				break;
			default:
				break;
			}
		} while (core.m_current_thread_index != start);

		return &core.m_idle_thread;
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
		CPU::Processor::sleep();
	}

	void CoreScheduler::terminate_current()
	{
		terminate(CPU::Processor::current().get_current_thread());
	}

	__noreturn void CoreScheduler::idle()
	{
		for (;;)
			CPU::Processor::sleep();
	}
}