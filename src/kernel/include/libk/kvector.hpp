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

		explicit vector(size_t n, const T &val = T())
		{
			m_capacity = next_power_of_two(n);
			m_size = n;

			m_array = (T *)kmalloc(sizeof(T) * m_capacity, sizeof(T));

			for (size_t i = 0; i < n; i++)
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

		[[nodiscard]] size_t size() const { return m_size; }
		[[nodiscard]] bool empty() const { return m_size == 0; }

		void resize(size_t n, T val = T())
		{
			if (n < m_size)
			{
				for (size_t i = n; i < m_size; i++)
					m_array[i].~T();

				m_size = n;
				return;
			}

			ensure_capacity(n);

			for (size_t i = m_size; i < n; i++)
				m_array[i] = val;

			m_size = n;
		}

		void ensure_capacity(size_t n)
		{
			if (n > m_capacity)
			{
				while (m_capacity < n)
					m_capacity *= 2;

				m_array = (T *)krealloc(m_array, sizeof(T) * m_capacity, sizeof(T));
				assert(m_array);
			}
		}

		void push_back(const T &val) { resize(m_size + 1, val); }
		void pop_back() { m_array[m_size-- - 1].~T(); }

		const T *data() const { return m_array; }

		typedef normal_iterator<vector<T>, T> iterator;
		typedef normal_iterator<const vector<T>, const T> const_iterator;

		iterator begin() { return iterator{*this, 0}; }
		iterator end() { return iterator{*this, m_size}; }

		const_iterator begin() const { return const_iterator{*this, 0}; }
		const_iterator end() const { return const_iterator{*this, m_size}; }

		iterator erase(iterator position)
		{
			assert(position.m_index >= 0 && position.m_index < m_size);

			m_array[position.m_index].~T();
			memmove(&m_array[position.m_index], &m_array[position.m_index + 1], m_size - position.m_index - 1);
			m_size--;

			return position;
		}

	protected:
		size_t m_capacity{4};
		size_t m_size{0};

		T *m_array{nullptr};
	};
} // namespace Kernel::LibK
