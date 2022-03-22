#include <time/TimeManager.hpp>

#include <time/PIT.hpp>

namespace Kernel::Time
{
	void TimeManager::initialize()
	{
		m_available_timers.push_back(&PIT::instance());
		PIT::instance().set_callback([&](Timer &timer __unused) {
			handle_event(timer);
		});
	}

	void TimeManager::usleep(uint64_t usecs)
	{
		// TODO: This is some incredibly naive code that doesn't take much into consideration
		for (auto timer : m_available_timers)
		{
			uint64_t interval = (usecs * 1000) / timer->get_time_quantum_in_ns();
			if (interval <= timer->get_maximum_interval())
			{
				m_sleeping = true;
				timer->start(interval, false);
				break;
			}
		}

		while (m_sleeping)
			;
	}

	void TimeManager::sleep(uint64_t millis)
	{
		usleep(millis * 1000);
	}

	void TimeManager::handle_event(Timer &timer __unused)
	{
		m_sleeping = false;
	}
} // namespace Kernel::Time