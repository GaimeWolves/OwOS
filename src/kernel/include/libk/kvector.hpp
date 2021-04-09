#pragma once

#include <libk/__iterators.hpp>
#include <libk/kcassert.hpp>
#include <libk/kcmalloc.hpp>
#include <libk/kmath.hpp>

namespace Kernel::LibK
{
	template <typename T>
	class vector
	{
	public:
		vector()
		{
			m_array = (T *)kmalloc(sizeof(T) * m_capacity, sizeof(T));
		}

		vector(size_t n, const T &val = T())
		{
			m_capacity = next_power_of_two(n);
			m_size = n;

			m_array = (T *)kmalloc(sizeof(T) * m_capacity, sizeof(T));

			for (int i = 0; i < n; i++)
				m_array[i] = val;
		}

		~vector()
		{
			kfree(m_array);
		}

		T &operator[](size_t n) { return at(n); }
		const T &operator[](size_t n) const { return at(n); }

		T &at(size_t n)
		{
			assert(n < m_size);
			return m_array[n];
		}

		const T &at(size_t n) const
		{
			assert(n < m_size);
			return m_array[n];
		}

		size_t size() const { return m_size; }

		void resize(size_t n, T val = T())
		{
			if (n < m_size)
			{
				for (size_t i = n; i < m_size; i++)
					m_array[i].~T();

				m_size = n;
				return;
			}

			if (n > m_capacity)
			{
				while (m_capacity < n)
					m_capacity *= 2;

				m_array = (T *)krealloc(m_array, sizeof(T) * m_capacity, sizeof(T));
				assert(m_array);
			}

			for (size_t i = m_size; i < n; i++)
				m_array[i] = val;

			m_size = n;
		}

		void push_back(const T &val) { resize(m_size + 1, val); }
		void pop_back() { m_array[m_size-- - 1].~T(); }

		const T *data() const { return m_array; }

		typedef normal_iterator<vector<T>, T> iterator;
		typedef normal_iterator<const vector<T>, const T> const_iterator;

		iterator begin() { return iterator{*this, 0}; }
		iterator end() { return iterator{*this, m_size}; }

		const const_iterator begin() const { return const_iterator{*this, 0}; }
		const const_iterator end() const { return const_iterator{*this, m_size}; }

	protected:
		size_t m_capacity{4};
		size_t m_size{0};

		alignas(T) T *m_array{nullptr};
	};
} // namespace Kernel::LibK
