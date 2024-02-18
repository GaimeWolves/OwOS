#pragma once

#include <arch/spinlock.hpp>

#include <libk/katomic.hpp>

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

		[[nodiscard]] always_inline bool is_locked() const { return m_locked; }

	private:
		Spinlock m_lock;
		LibK::atomic_bool m_locked{false};
	};
}
