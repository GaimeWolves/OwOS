#include <arch/i686/spinlock.hpp>

#include <arch/Processor.hpp>

namespace Kernel::Locking
{
	bool Spinlock::try_lock()
	{
		//CPU::Processor::current().enter_critical();
		bool locked = !m_lock.exchange(1, LibK::memory_order_acquire);

		//if (!locked)
		//	CPU::Processor::current().leave_critical();

		return locked;
	}

	void Spinlock::lock()
	{
		CPU::Processor::current().enter_critical();
		while (m_lock.exchange(1, LibK::memory_order_acquire))
		{
			CPU::Processor::pause();
		}
	}

	void Spinlock::unlock()
	{
		assert(is_locked());

		m_lock.store(0);
		CPU::Processor::current().leave_critical();
	}
}