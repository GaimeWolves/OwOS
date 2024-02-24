#include <locking/Mutex.hpp>

#include <arch/Processor.hpp>

namespace Kernel::Locking
{
	bool Mutex::try_lock()
	{
		bool locked = m_lock.try_lock();

		if (!locked)
			return false;

		m_locked.test_and_set();
		m_lock.unlock();
		return true;
	}

	void Mutex::lock()
	{
		CPU::Processor &core = CPU::Processor::current();
		if (!core.is_scheduler_running() || !core.is_thread_running())
		{
			m_lock.lock();
			m_locked.test_and_set();
			m_lock.unlock();
			return;
		}

		if (m_locked.test())
			CoreScheduler::block(this);

		m_locked.test_and_set();
	}

	void Mutex::unlock()
	{
		CPU::Processor &core = CPU::Processor::current();
		if (!core.is_scheduler_running() || !core.is_thread_running())
		{
			m_lock.lock();
			m_locked.clear();
			m_lock.unlock();
			return;
		}

		m_locked.clear();
	}
}
