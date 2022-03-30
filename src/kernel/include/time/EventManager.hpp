#pragma once

#include <stdint.h>

#include <libk/kvector.hpp>
#include <libk/Concurrent.hpp>
#include <libk/katomic.hpp>

#include <time/Timer.hpp>
#include <arch/interrupts.hpp>

namespace Kernel::Time
{
	class EventManager
	{
	private:
		typedef struct event_t
		{
			LibK::function<void()> callback;
			uint64_t nanoseconds;
			bool core_local;
			Timer *used_timer;
		} event_t;

	public:
		static EventManager &instance()
		{
			static EventManager *instance{nullptr};

			if (!instance)
				instance = new EventManager();

			return *instance;
		}

		void register_timer(Timer *timer);

		void usleep(uint64_t usecs);
		void sleep(uint64_t millis);

		// Early-use version of sleep() that utilizes a global timer instead of a local timer.
		// Used in BSP initialization as local timers are yet to be initialized.
		void early_sleep(uint64_t usecs);

		void schedule_event(const LibK::function<void()> &callback, uint64_t nanoseconds, bool core_sensitive);

		typedef LibK::Concurrent<LibK::vector<event_t>> EventQueue;

	private:
		EventManager() = default;
		~EventManager() = default;

		void handle_event(Timer &timer);
		void schedule_next_event(LibK::vector<event_t> &event_queue);

		void schedule_on(event_t &event, LibK::vector<event_t> &event_queue);

		LibK::vector<Timer *> m_available_timers{};
		LibK::atomic_uint32_t m_sleeping{0}; // Bitmap of sleeping cores
		EventQueue m_scheduled_events{}; // TODO: This should use some sort of linked list
	};
}