#pragma once

#include <stdint.h>

#include <libk/kfunctional.hpp>

namespace Kernel::Time
{
	enum class TimerType
	{
		Global, CPU
	};

	class Timer
	{
	public:
		void set_callback(const LibK::function<void(Timer &)> &callback) { m_callback = callback; }

		virtual void start(uint64_t interval, bool repeat) = 0;
		virtual void stop() = 0;

		[[nodiscard]] virtual uint64_t get_time_quantum_in_ns() const = 0;
		[[nodiscard]] virtual uint64_t get_maximum_interval() const = 0;
		[[nodiscard]] virtual TimerType timer_type() const = 0;

	protected:
		LibK::function<void(Timer &)> m_callback;
	};
}