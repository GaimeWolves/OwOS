#pragma once

#include <locking/Mutex.hpp>

namespace Kernel::LibK
{
	template <typename T>
	class ConcurrentRef
	{
	public:
		ConcurrentRef(T *data, Locking::Mutex *lock)
		    : m_data(data), m_lock(lock)
		{
			m_lock->lock();
		}

		ConcurrentRef &operator=(const ConcurrentRef &) volatile = delete;
		ConcurrentRef &operator=(ConcurrentRef &&) volatile = delete;
		ConcurrentRef(const ConcurrentRef &) = delete;
		ConcurrentRef(ConcurrentRef &&) = delete;

		~ConcurrentRef()
		{
			m_lock->unlock();
		}

		always_inline T *operator->()
		{
			return m_data;
		}

		always_inline T &operator*()
		{
			return *m_data;
		}

	private:
		T *m_data;
		Locking::Mutex *m_lock{};
	};

	template <typename T>
	class Concurrent
	{
	public:
		ConcurrentRef<T> get() { return ConcurrentRef(&m_data, &m_lock); }

	private:
		T m_data{};
		Locking::Mutex m_lock{};
	};
}
