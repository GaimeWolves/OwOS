#pragma once

#include <stdint.h>

#include <libk/kfunctional.hpp>

namespace Kernel::Time
{
	enum class TimerType
	{
		Global, CPU
	};

	// For now all timers function as one-shot versions. This should be expanded upon later
	class Timer
	{
	public:
		void set_handle_callback(const LibK::function<void(Timer &)> &callback) { m_handle_callback = callback; }
		void set_reduce_callback(const LibK::function<uint64_t(Timer &, uint64_t)> &callback) { m_reduce_callback = callback; }

		virtual void start(uint64_t interval) = 0;
		virtual uint64_t stop() = 0;

		[[nodiscard]] virtual uint64_t get_time_quantum_in_ns() const = 0;
		[[nodiscard]] virtual uint64_t get_maximum_interval() const = 0;
		[[nodiscard]] virtual TimerType timer_type() const = 0;

	protected:
		LibK::function<void(Timer &timer)> m_handle_callback;
		LibK::function<uint64_t(Timer &, uint64_t)> m_reduce_callback;
	};
}