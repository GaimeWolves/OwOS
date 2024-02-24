#include <arch/i686/spinlock.hpp>

#include <arch/Processor.hpp>

namespace Kernel::Locking
{
	bool Spinlock::try_lock()
	{
		CPU::Processor::current().enter_critical();
		bool lock_succeeded = m_lock.test_and_set(std::memory_order_acquire);

		if (!lock_succeeded)
			CPU::Processor::current().leave_critical();

		return lock_succeeded;
	}

	void Spinlock::lock()
	{
		CPU::Processor::current().enter_critical();
		while (m_lock.test_and_set(std::memory_order_acquire))
		{
			while (m_lock.test(std::memory_order_relaxed))
				CPU::Processor::pause();
		}
	}

	void Spinlock::unlock()
	{
		assert(is_locked());

		m_lock.clear(std::memory_order_release);
		CPU::Processor::current().leave_critical();
	}
}
