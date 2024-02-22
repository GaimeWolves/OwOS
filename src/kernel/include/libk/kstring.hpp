#pragma once

#include <libk/kcmalloc.hpp>
#include <libk/kcstring.hpp>
#include <libk/kmath.hpp>
#include <libk/kvector.hpp>

namespace Kernel::LibK
{
	template <class charT>
	class basic_string
	{
	public:
		constexpr basic_string() = default;

		constexpr /*implicit*/ basic_string(const charT *s)
		{
			assert(s);
			std::size_t len = strlen(s);
			m_vector.resize(len + 1);
			memcpy(m_vector.data(), s, len);
			ensure_terminated();
		}

		constexpr ~basic_string() = default;

		const charT *c_str() const { return this->data(); }

		constexpr basic_string &operator+=(const basic_string &str)
		{
			for (charT ch : str)
				push_back(ch);

			return *this;
		}

		constexpr basic_string &operator+=(const charT *str)
		{
			while (*str)
				push_back(*str++);

			return *this;
		}

		constexpr basic_string &operator+=(charT c)
		{
			push_back(c);
			return *this;
		}

		constexpr charT &at(size_t n)
		{
			assert(n < m_vector.size() - 1);
			return m_vector[n];
		}

		constexpr const charT &at(size_t n) const
		{
			assert(n < m_vector.size() - 1);
			return m_vector[n];
		}

		constexpr charT &operator[](size_t n) { return at(n); }
		constexpr const charT &operator[](size_t n) const { return at(n); }

		constexpr charT &front() { return at(0); }
		constexpr const charT &front() const { return at(0); }

		constexpr charT &back() { return at(size() - 1); }
		constexpr const charT &back() const { return at(size() - 1); }

		[[nodiscard]] constexpr std::size_t size() const noexcept { return m_vector.size() - 1; }
		[[nodiscard]] constexpr bool empty() const { return m_vector.size() <= 1; }

		constexpr void reserve(size_t n) { m_vector.reserve(n + 1); }
		constexpr void resize(size_t n, const charT val = 0) { m_vector.resize(n + 1, val); }

		constexpr void push_back(charT val)
		{
			m_vector.resize(max(2, m_vector.size() + 1), 0);
			m_vector[m_vector.size() - 2] = val;
		}

		constexpr void pop_back()
		{
			m_vector.pop_back();
			ensure_terminated();
		}

		constexpr const charT *data() const { return m_vector.data(); }

		typedef normal_iterator<vector<charT>, charT> iterator;
		typedef normal_iterator<const vector<charT>, const charT> const_iterator;

		constexpr iterator begin() { return iterator{m_vector, 0}; }
		constexpr const_iterator begin() const { return const_iterator{m_vector, 0}; }

		constexpr iterator end() { return iterator{m_vector, max(m_vector.size(), 1) - 1}; }
		constexpr const_iterator end() const { return const_iterator{m_vector, max(m_vector.size(), 1) - 1}; }

		constexpr iterator insert(const_iterator pos, const charT &val) { return m_vector.insert(pos, val); }
		constexpr iterator insert(const_iterator pos, size_t n, const charT &val) { return m_vector.insert(pos, n, val); }
		constexpr iterator insert(const_iterator pos, const basic_string &str) { return insert(pos, str.data()); }

		constexpr iterator insert(const_iterator pos, const charT *str)
		{
			size_t n = strlen(str);

			m_vector.insert(pos, n, 0);

			for (size_t i = 0; i < n; i++)
				m_vector[pos.__index() + i] = str[i];

			return iterator{m_vector, pos.__index()};
		}

		constexpr iterator erase(const_iterator pos) { return m_vector.erase(pos); }
		constexpr iterator erase(const_iterator first, const_iterator last) { return m_vector.erase(first, last); }

	private:
		constexpr void ensure_terminated() { m_vector[m_vector.size() - 1] = '\0'; }

		vector<charT> m_vector{};
	};

	typedef basic_string<char> string;
} // namespace Kernel::LibK
