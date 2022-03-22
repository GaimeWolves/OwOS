#pragma once

#include <arch/spinlock.hpp>
#include <arch/Processor.hpp>

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

		always_inline void lock()
		{
			if (CPU::Processor::count() == 1)
				return;

			while (true)
			{
				// TODO: With a scheduler we could suspend the running code here until the resource gets freed
				//       For now this is basically just a wrapper around a spinlock

				CPU::Processor::pause();

				m_lock.lock();
				if (!m_locked)
				{
					m_locked = true;
					m_lock.unlock();
					return;
				}
				m_lock.unlock();
			}
		}

		always_inline void unlock()
		{
			if (CPU::Processor::count() == 1)
				return;

			m_lock.lock();
			m_locked = false;
			m_lock.unlock();
		}

	private:
		Spinlock m_lock;
		LibK::atomic_bool m_locked{false};
	};
}