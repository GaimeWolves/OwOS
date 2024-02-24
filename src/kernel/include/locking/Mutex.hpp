#pragma once

#include <arch/spinlock.hpp>

namespace Kernel::Locking
{
	class Mutex
	{
	public:
		Mutex() = default;
		Mutex &operator=(const Mutex &) = delete;
		Mutex &operator=(Mutex &&) = delete;
		Mutex(const Mutex &) = delete;
		Mutex(Mutex &&) = delete;

		bool try_lock();
		void lock();
		void unlock();

		[[nodiscard]] always_inline bool is_locked() const { return m_locked.test(std::memory_order_relaxed); }

	private:
		Spinlock m_lock;
		std::atomic_flag m_locked{false};
	};
} // namespace Kernel::Locking
