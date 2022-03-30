#pragma once

#include <libk/katomic.hpp>
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
			return m_lock.load(LibK::memory_order_relaxed);
		}

	private:
		alignas(64) LibK::atomic_uint32_t m_lock{0}; // lock will be aligned on cache line boundary
	};
} // namespace Kernel::Locking
