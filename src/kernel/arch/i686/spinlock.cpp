#include <arch/i686/spinlock.hpp>

#include <arch/Processor.hpp>

namespace Kernel::Locking
{
	bool Spinlock::try_lock()
	{
		CPU::Processor::current().enter_critical();
		bool lock_succeeded = m_lock.exchange(1, LibK::memory_order_acquire) != 0;

		if (!lock_succeeded)
			CPU::Processor::current().leave_critical();

		return lock_succeeded;
	}

	void Spinlock::lock()
	{
		CPU::Processor::current().enter_critical();
		while (m_lock.exchange(1, LibK::memory_order_acquire) != 0)
		{
			CPU::Processor::pause();
		}
	}

	void Spinlock::unlock()
	{
		assert(is_locked());

		m_lock.store(0, LibK::memory_order_release);
		CPU::Processor::current().leave_critical();
	}
}
