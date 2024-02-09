#pragma once

#include <libk/kcmalloc.hpp>
#include <libk/kcstring.hpp>
#include <libk/kmath.hpp>
#include <libk/kvector.hpp>

namespace Kernel::LibK
{
	template <typename charT>
	class basic_string
	{
	public:
		basic_string()
		{
		    resize(0);
		}

		basic_string(const charT *s)
		{
			assert(s);

			resize(strlen(s));

			assert(this->m_array);

			memcpy(this->m_array, s, this->m_size);
		}

		~basic_string()
		{
			if (m_array)
			{
				kfree(m_array);
				m_array = nullptr;
			}
		}

		basic_string(const basic_string &other) = delete;
		basic_string &operator=(const basic_string &other) = delete;

		basic_string(basic_string &&other) noexcept
		    : m_capacity(other.m_capacity)
		    , m_size(other.m_size)
		    , m_array(other.m_array)
		{
			other.m_capacity = 0;
			other.m_size = 0;
			other.m_array = nullptr;
		}

		basic_string &operator=(basic_string &&other) noexcept
		{
			this->~basic_string();

			m_capacity = other.m_capacity;
			m_size = other.m_size;
			m_array = other.m_array;

			other.m_capacity = 0;
			other.m_size = 0;
			other.m_array = nullptr;

			return *this;
		}

		const charT *c_str() const { return this->data(); }

		basic_string &operator+=(const basic_string &str)
		{
			for (charT ch : str)
				this->push_back(ch);

			return *this;
		}

		basic_string &operator+=(const charT *str)
		{
			while (*str)
				this->push_back(*str++);

			return *this;
		}

		basic_string &operator+=(charT c)
		{
			this->push_back(c);
			return *this;
		}

		charT &operator[](size_t n) { return at(n); }
		const charT &operator[](size_t n) const { return at(n); }

		charT &at(size_t n)
		{
			assert(n < m_size - 1);
			return m_array[n];
		}

		const charT &at(size_t n) const
		{
			assert(n < m_size - 1);
			return m_array[n];
		}

		charT &front() { return at(0); }
		const charT &front() const { return at(0); }

		charT &back() { return at(m_size - 2); }
		const charT &back() const { return at(m_size - 2); }

		[[nodiscard]] size_t size() const { return m_size - 1; }
		[[nodiscard]] bool empty() const { return m_size <= 1; }

		void resize(size_t n, charT val = 0)
		{
			if (m_size > 0 && n < m_size - 1)
			{
				for (size_t i = n; i < m_size - 1; i++)
					m_array[i] = 0;

				m_size = n + 1;
				return;
			}

			ensure_capacity(n);

			for (size_t i = m_size - 1; i < n; i++)
				m_array[i] = val;

			m_size = n + 1;
		}

		void ensure_capacity(size_t n)
		{
			if (m_capacity == 0 || n > m_capacity - 1)
			{
				if (m_capacity == 0)
					m_capacity = 4;

				while (m_capacity - 1 < n)
					m_capacity *= 2;

				auto new_array = (charT *)kmalloc(sizeof(charT) * m_capacity, sizeof(charT));
				assert(new_array);
				memset(new_array, 0, sizeof(charT) * m_capacity);
				for (size_t i = 0; i < m_size; i++)
					new_array[i] = m_array[i];
				m_array = new_array;
			}
		}

		void push_back(charT val)
		{
			ensure_capacity(m_size + 1);
			m_array[m_size - 1] = val;
			m_size++;
		}

		void pop_back() { resize(m_size - 2); }

		const charT *data() const { return m_array; }

		typedef normal_iterator<basic_string<charT>, charT> iterator;
		typedef normal_iterator<const basic_string<charT>, const charT> const_iterator;

		iterator begin() { return iterator{*this, 0}; }
		iterator end() { return iterator{*this, m_size - 1}; }

		const_iterator begin() const { return const_iterator{*this, 0}; }
		const_iterator end() const { return const_iterator{*this, m_size - 1}; }

		iterator insert(const_iterator position, const charT &val)
		{
			ensure_capacity(m_size + 1);
			memmove(&m_array[position.m_index + 1], &m_array[position.m_index], (m_size - position.m_index) * sizeof(charT));
			m_array[position.m_index] = val;
			m_size++;
			return iterator{*this, position.m_index};
		}

		iterator insert(const_iterator position, size_t n, const charT &val)
		{
			ensure_capacity(m_size + n);
			memmove(&m_array[position.m_index + n], &m_array[position.m_index], (m_size - position.m_index) * sizeof(charT));

			for (size_t i = 0; i < n; i++)
				m_array[position.m_index + i] = val;

			m_size += n;
			return iterator{*this, position.m_index};
		}

		iterator insert(const_iterator position, const basic_string &str)
		{
			ensure_capacity(m_size + str.size());
			memmove(&m_array[position.m_index + str.size()], &m_array[position.m_index], (m_size - position.m_index) * sizeof(charT));

			for (size_t i = 0; i < str.size(); i++)
				m_array[position.m_index + i] = str[i];

			m_size += str.size();
			return iterator{*this, position.m_index};
		}

		iterator insert(const_iterator position, const charT *str)
		{
			size_t n = strlen(str);

			ensure_capacity(m_size + n);
			memmove(&m_array[position.m_index + n], &m_array[position.m_index], (m_size - position.m_index) * sizeof(charT));

			for (size_t i = 0; i < n; i++)
				m_array[position.m_index + i] = str[i];

			m_size += n;
			return iterator{*this, position.m_index};
		}

		iterator insert(iterator position, const basic_string &str) { return insert(const_iterator{*this, position.m_index}, str); }
		iterator insert(iterator position, const charT *str) { return insert(const_iterator{*this, position.m_index}, str); }
		iterator insert(iterator position, const charT &val) { return insert(const_iterator{*this, position.m_index}, val); }
		iterator insert(iterator position, size_t n, const charT &val) { return insert(const_iterator{*this, position.m_index}, n, val); }

		iterator erase(iterator position)
		{
			assert(position.m_index >= 0 && position.m_index < m_size - 1);

			m_array[position.m_index] = 0;
			memmove(&m_array[position.m_index], &m_array[position.m_index + 1], (m_size - position.m_index) * sizeof(charT));
			m_size--;

			return position;
		}

		iterator erase(iterator first, iterator last)
		{
			if (first >= last)
				return last;

			size_t num = last - first;
			memmove(&m_array[first.m_index], &m_array[last.m_index], (m_size - last.m_index) * sizeof(charT));
			m_size -= num;

			return first;
		}

	private:
		size_t m_capacity{0};
		// m_size refers to the size of the string including the null-byte
		size_t m_size{0};

		charT *m_array{nullptr};
	};

	typedef basic_string<char> string;
} // namespace Kernel::LibK
