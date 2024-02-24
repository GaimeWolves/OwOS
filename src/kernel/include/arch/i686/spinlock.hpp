#pragma once

#include <atomic>

#include <libk/kcassert.hpp>

#include <common_attributes.h>

namespace Kernel::Locking
{
	class Spinlock
	{
	public:
		Spinlock() = default;
		Spinlock &operator=(const Spinlock &) = delete;
		Spinlock &operator=(Spinlock &&) = delete;
		Spinlock(const Spinlock &) = delete;
		Spinlock(Spinlock &&) = delete;

		bool try_lock();
		void lock();
		void unlock();

		always_inline bool is_locked() const noexcept
		{
			return m_lock.test(std::memory_order_relaxed);
		}

	private:
		alignas(64) std::atomic_flag m_lock{false}; // lock will be aligned on cache line boundary
	};
} // namespace Kernel::Locking
