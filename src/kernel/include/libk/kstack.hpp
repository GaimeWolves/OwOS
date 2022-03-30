#pragma once

#include <stddef.h>

#include "kvector.hpp"

namespace Kernel::LibK
{
	// TODO: Use correct container type (e.g. deque)
	template<typename T>
	class stack
	{
	public:
		typedef T value_type;
		typedef vector<T> container_type;
		typedef T &reference;
		typedef const T &const_reference;
		typedef size_t size_type;

		void push(const_reference value) { m_container.push_back(value); }

		void pop() { m_container.pop_back(); }

		reference top() { return m_container[m_container.size() - 1]; }
		const_reference top() const { return m_container[m_container.size() - 1]; }

		[[nodiscard]] size_type size() const { return m_container.size(); }

		[[nodiscard]] bool empty() const { return m_container.empty(); }

	private:
		vector<T> m_container;
	};
}