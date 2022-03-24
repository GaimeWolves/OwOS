#pragma once

#include <stdint.h>

#include <libk/kvector.hpp>
#include <libk/katomic.hpp>

#include <time/Timer.hpp>

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

		void initialize();
		void register_timer(Timer *timer);

		void usleep(uint64_t usecs);
		void sleep(uint64_t millis);

		void schedule_event(const LibK::function<void()> &callback, uint64_t millis, bool core_sensitive);

	private:
		EventManager() = default;
		~EventManager() = default;

		void handle_event();
		void schedule_next_event();

		LibK::vector<Timer *> m_available_timers{};
		LibK::atomic_uint32_t m_sleeping{0}; // Bitmap of sleeping cores
		LibK::vector<event_t> m_scheduled_events{}; // TODO: This should use some sort of linked list
	};
}