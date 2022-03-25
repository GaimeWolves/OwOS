#include <time/EventManager.hpp>

#include <arch/Processor.hpp>

namespace Kernel::Time
{
	void EventManager::register_timer(Timer *timer)
	{
		m_available_timers.push_back(timer);
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

		EventQueue &event_queue = core_sensitive ? CPU::Processor::current().get_event_queue() : m_scheduled_events;
		schedule_on(event, event_queue);
	}

	void EventManager::schedule_on(const event_t &event, EventQueue &event_queue)
	{
		for (auto it = event_queue.begin(); it != event_queue.end(); ++it)
		{
			if (event.nanoseconds < it->nanoseconds)
			{
				bool reschedule = it == event_queue.begin();

				if (reschedule)
					it->used_timer->stop();

				it->nanoseconds -= event.nanoseconds;
				event_queue.insert(it, event);

				if (reschedule)
					schedule_next_event(event_queue);

				return;
			}
		}

		event_queue.push_back(event);
		if (event_queue.size() == 1)
			schedule_next_event(event_queue);
	}

	void EventManager::handle_event(EventQueue &event_queue)
	{
		event_queue.front().callback();
		event_queue.erase(event_queue.begin());

		schedule_next_event(event_queue);
	}

	void EventManager::schedule_next_event(EventQueue &event_queue)
	{
		if (event_queue.empty())
			return;

		event_t &event = event_queue.front();
		uint64_t largest_possible_time = 0;
		Timer *best_timer;

		for (auto timer : m_available_timers)
		{
			if ((timer->timer_type() != TimerType::CPU && event.core_local) || (timer->timer_type() != TimerType::Global && !event.core_local))
				continue;

			uint64_t interval = (event.nanoseconds) / timer->get_time_quantum_in_ns();
			if (interval <= timer->get_maximum_interval())
			{
				timer->set_callback([&event_queue, this](){ handle_event(event_queue); });
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
		best_timer->set_callback([&event_queue, this](){ handle_event(event_queue); });
		best_timer->start(best_timer->get_maximum_interval());
	}
} // namespace Kernel::Time