#pragma once

#include <stdint.h>

#include <libk/kvector.hpp>

#include <time/Timer.hpp>

namespace Kernel::Time
{
	class TimeManager
	{
	public:
		static TimeManager &instance()
		{
			static TimeManager *instance{nullptr};

			if (!instance)
				instance = new TimeManager();

			return *instance;
		}

		void initialize();

		void usleep(uint64_t usecs);
		void sleep(uint64_t millis);

	private:
		TimeManager() = default;
		~TimeManager() = default;

		void handle_event(Timer &timer);

		LibK::vector<Timer *> m_available_timers;
		volatile bool m_sleeping{false};
	};
}