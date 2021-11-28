#pragma once

#include <arch/processor.hpp>
#include <common_attributes.h>

#include <libk/katomic.hpp>
#include <libk/kcassert.hpp>

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

		always_inline void lock()
		{
			Processor::clear_interrupts();
			while (m_lock.exchange(1, LibK::memory_order_acquire))
			{
				Processor::pause();
			}
		}

		always_inline void unlock()
		{
			assert(is_locked());

			m_lock.store(0);
			Processor::enable_interrupts();
		}

		always_inline bool is_locked() const noexcept
		{
			return m_lock.load(LibK::memory_order_relaxed);
		}

	private:
		alignas(64) LibK::atomic_uint32_t m_lock{0}; // lock will be aligned on cache line boundary
	};
} // namespace Kernel::Locking
