#pragma once

#include <stddef.h>

#include <libk/kcassert.hpp>
#include <libk/__iterators.hpp>

namespace Kernel::LibK
{
	template <typename T>
	class ArrayView
	{
	public:
		ArrayView() = default;

		ArrayView(const T *array, size_t size)
		    : m_data(array)
		    , m_size(size)
		{
		}

		const T &operator[](size_t n) const { return at(n); }

		const T &at(size_t n) const
		{
			assert(n < m_size);
			return m_data[n];
		}

		const T *data() const { return m_data; }

		[[nodiscard]] size_t size() const { return m_size; }

		typedef normal_iterator<const ArrayView<T>, const T> const_iterator;

		const_iterator begin() const { return const_iterator{*this, 0}; }
		const_iterator end() const { return const_iterator{*this, m_size}; }

	private:
		const T *m_data{nullptr};
		const size_t m_size{0};
	};
}