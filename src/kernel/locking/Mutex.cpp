#include <locking/Mutex.hpp>

#include <arch/Processor.hpp>

namespace Kernel::Locking
{
	bool Mutex::try_lock()
	{
		bool locked = m_lock.try_lock();

		if (!locked)
			return false;

		m_locked = true;
		m_lock.unlock();
		return true;
	}

	void Mutex::lock()
	{
		CPU::Processor &core = CPU::Processor::current();
		if (!core.is_scheduler_running() || !core.is_thread_running())
		{
			m_lock.lock();
			m_locked = true;
			m_lock.unlock();
			return;
		}

		if (m_locked)
			CoreScheduler::block(this);

		m_locked = true;
	}

	void Mutex::unlock()
	{
		CPU::Processor &core = CPU::Processor::current();
		if (!core.is_scheduler_running() || !core.is_thread_running())
		{
			m_lock.lock();
			m_locked = false;
			m_lock.unlock();
			return;
		}

		m_locked = false;
	}
}
