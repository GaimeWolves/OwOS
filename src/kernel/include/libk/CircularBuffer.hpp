#pragma once

#include <libk/kcmalloc.hpp>
#include <libk/kcassert.hpp>
#include <stddef.h>

namespace Kernel::LibK
{
	template <typename T>
	class CircularBuffer
	{
	public:
		CircularBuffer(size_t size)
		{
			m_capacity = size;
			m_size = 0;
			m_begin = reinterpret_cast<T *>(kmalloc(m_capacity * sizeof(T)));
			m_end = m_begin + m_capacity;
			m_head = m_begin;
			m_tail = m_begin;
		}

		T pop()
		{
			assert(m_size > 0);
			T val = *m_tail++;

			if (m_tail == m_end)
				m_tail = m_begin;

			m_size--;

			return val;
		}

		void push(T val)
		{
			*m_head++ = val;

			if (m_head == m_end)
				m_head = m_begin;

			if (m_size == m_capacity)
				m_tail = m_head;
			else
				m_size++;
		}

		[[nodiscard]] bool is_empty() { return m_size == 0; }
		[[nodiscard]] bool is_full() { return m_size == m_capacity;}

		[[nodiscard]] size_t size() { return m_size; }

	private:
		T *m_begin{nullptr};
		T *m_end{nullptr};
		T *m_head{nullptr};
		T *m_tail{nullptr};
		size_t m_capacity{0};
		size_t m_size{0};
	};
}