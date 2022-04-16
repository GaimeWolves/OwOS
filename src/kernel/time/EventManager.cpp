#include <time/EventManager.hpp>

#include <libk/kcstdio.hpp>

#include <arch/Processor.hpp>

namespace Kernel::Time
{
	void EventManager::register_timer(Timer *timer)
	{
		m_available_timers.push_back(timer);
		timer->set_callback([this](Timer &timer){ handle_event(timer); });
	}

	void EventManager::usleep(uint64_t usecs)
	{
		thread_t *current_thread = CPU::Processor::current().get_current_thread();

		schedule_event([current_thread](){
			current_thread->state = CPU::Processor::current().get_current_thread() == current_thread ? ThreadState::Running : ThreadState::Ready;
		}, usecs * 1000,true);

		current_thread->state = ThreadState::Sleeping;
		while (current_thread->state == ThreadState::Sleeping)
			;
	}

	void EventManager::sleep(uint64_t millis)
	{
		usleep(millis * 1000);
	}

	void EventManager::early_sleep(uint64_t usecs)
	{
		static LibK::atomic_bool s_sleeping;
		s_sleeping = true;

		schedule_event([](){
			s_sleeping = false;
		}, usecs * 1000,false);

		while (s_sleeping)
			;
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
		auto queue = event_queue.get();
		schedule_on(event, *queue);
	}

	void EventManager::schedule_on(event_t &event, LibK::vector<event_t> &event_queue)
	{
		for (auto it = event_queue.begin(); it != event_queue.end(); ++it)
		{
			if (event.nanoseconds == it->nanoseconds)
			{
				// Merge the two events into one, calling both callback methods
				auto firstCall = it->callback;
				auto secondCall = event.callback;
				it->callback = [=]() {
					firstCall();
					secondCall();
				};

				return;
			}
			else if (event.nanoseconds < it->nanoseconds)
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

			event.nanoseconds -= it->nanoseconds;
		}

		event_queue.push_back(event);
		if (event_queue.size() == 1)
			schedule_next_event(event_queue);
	}

	void EventManager::handle_event(Timer &timer)
	{
		event_t event;

		{
			CPU::Processor &core = CPU::Processor::current();
			EventQueue &event_queue = timer.timer_type() == TimerType::CPU ? core.get_event_queue() : m_scheduled_events;
			auto queue = event_queue.get();

			event = queue->front();
			queue->erase(queue->begin());
			schedule_next_event(*queue);
		}

		event.callback();
	}

	void EventManager::schedule_next_event(LibK::vector<event_t> &event_queue)
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

		event_queue.insert(event_queue.begin(), needed_events, new_event);
		best_timer->start(best_timer->get_maximum_interval());
	}
} // namespace Kernel::Time