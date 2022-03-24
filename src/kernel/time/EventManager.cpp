#include <time/EventManager.hpp>

#include <time/PIT.hpp>
#include <arch/Processor.hpp>

namespace Kernel::Time
{
	void EventManager::initialize()
	{
		register_timer(&PIT::instance());
	}

	void EventManager::register_timer(Timer *timer)
	{
		m_available_timers.push_back(timer);
		timer->set_callback([&]() {
			handle_event();
		});
	}

	void EventManager::usleep(uint64_t usecs)
	{
		uint32_t id = CPU::Processor::current().id();

		m_sleeping |= (1 << id);

		schedule_event([this, id](){
			m_sleeping &= ~(1 << id);
		}, usecs * 1000,false);

		while (m_sleeping & (1 << id))
			;
	}

	void EventManager::sleep(uint64_t millis)
	{
		usleep(millis * 1000);
	}

	void EventManager::schedule_event(const LibK::function<void()> &callback, uint64_t nanoseconds, bool core_sensitive)
	{
		event_t event{
			.callback = callback,
			.nanoseconds = nanoseconds,
			.core_local = core_sensitive,
			.used_timer = nullptr,
		};

		for (auto it = m_scheduled_events.begin(); it != m_scheduled_events.end(); ++it)
		{
			if (event.nanoseconds < it->nanoseconds)
			{
				bool reschedule = it == m_scheduled_events.begin();

				if (reschedule)
					it->used_timer->stop();

				it->nanoseconds -= event.nanoseconds;
				m_scheduled_events.insert(it, event);

				if (reschedule)
					schedule_next_event();

				return;
			}
		}

		m_scheduled_events.push_back(event);
		if (m_scheduled_events.size() == 1)
			schedule_next_event();
	}

	void EventManager::handle_event()
	{
		m_scheduled_events.front().callback();
		m_scheduled_events.erase(m_scheduled_events.begin());

		schedule_next_event();
	}

	void EventManager::schedule_next_event()
	{
		if (m_scheduled_events.empty())
			return;

		event_t &event = m_scheduled_events.front();
		uint64_t largest_possible_time = 0;
		Timer *best_timer;

		for (auto timer : m_available_timers)
		{
			if (timer->timer_type() == TimerType::Global && event.core_local)
				continue;

			uint64_t interval = (event.nanoseconds) / timer->get_time_quantum_in_ns();
			if (interval <= timer->get_maximum_interval())
			{
				timer->start(interval);
				event.used_timer = timer;
				return;
			}
			else if (timer->get_maximum_interval() > largest_possible_time)
			{
				largest_possible_time = timer->get_maximum_interval() * timer->get_time_quantum_in_ns();
				best_timer = timer;
			}
		}

		assert(best_timer);

		size_t needed_events = event.nanoseconds / largest_possible_time;
		event.nanoseconds -= needed_events * largest_possible_time;

		event_t new_event{
		    .callback = [](){},
		    .nanoseconds = largest_possible_time,
		    .core_local = event.core_local,
		    .used_timer = best_timer,
		};

		m_scheduled_events.insert(m_scheduled_events.begin(), needed_events, new_event);
		best_timer->start(best_timer->get_maximum_interval());
	}
} // namespace Kernel::Time