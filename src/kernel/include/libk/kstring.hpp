#pragma once

#include <libk/kcmalloc.hpp>
#include <libk/kcstring.hpp>
#include <libk/kmath.hpp>
#include <libk/kvector.hpp>

namespace Kernel::LibK
{
	// TODO: char_traits
	// TODO: string_view

	template <class CharT, class Allocator = allocator<CharT>>
	class basic_string
	{
	public:
		using value_type = CharT;
		using allocator_type = Allocator;
		using size_type = typename std::allocator_traits<Allocator>::size_type;
		using difference_type = typename std::allocator_traits<Allocator>::difference_type;
		using pointer = typename std::allocator_traits<Allocator>::pointer;
		using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
		using reference = value_type &;
		using const_reference = const value_type &;

		using iterator = typename vector<CharT>::iterator;
		using const_iterator = typename vector<CharT>::const_iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		static constexpr size_type npos = size_type(-1);

		constexpr basic_string() noexcept(noexcept(Allocator()))
		    : basic_string(Allocator())
		{
		}

		constexpr explicit basic_string(const Allocator &alloc) noexcept
		    : m_vector(alloc)
		{
		}

		constexpr basic_string(size_type count, CharT ch, const Allocator &alloc = Allocator())
		    : m_vector(count, ch, alloc)
		{
			terminate();
		}

		constexpr basic_string(const basic_string &other, size_type pos, const Allocator &alloc = Allocator())
		    : basic_string(other, pos, npos, alloc)
		{
		}

		constexpr basic_string(basic_string &&other, size_type pos, const Allocator &alloc = Allocator())
		    : basic_string(std::move(other), pos, npos, alloc)
		{
		}

		constexpr basic_string(const basic_string &other, size_type pos, size_type count, const Allocator &alloc = Allocator())
		    : basic_string(other.begin() + pos, other.begin() + min(count, other.size() - pos), alloc)
		{
		}

		constexpr basic_string(basic_string &&other, size_type pos, size_type count, const Allocator &alloc = Allocator())
		    : basic_string(&other, pos, count, alloc)
		{
			~other();
		}

		constexpr basic_string(const CharT *s, size_type count, const Allocator &alloc = Allocator())
		    : basic_string(strnlen_s(s, count), '\0', alloc)
		{
			strncpy(m_vector.data(), s, size());
		}

		/* implicit */ constexpr basic_string(const CharT *s, const Allocator &alloc = Allocator())
		    : basic_string(s, strlen(s), alloc)
		{
		}

		template <class InputIt>
		constexpr basic_string(InputIt first, InputIt last, const Allocator &alloc = Allocator())
		    : m_vector(first, last, alloc)
		{
			terminate();
		}

		constexpr basic_string(const basic_string &other) = default;
		constexpr basic_string(const basic_string &other, const Allocator &alloc)
		    : m_vector(other.m_vector, alloc)
		{
		}

		constexpr basic_string(basic_string &&other) noexcept = default;
		constexpr basic_string(basic_string &&other, const Allocator &alloc)
		    : m_vector(std::move(other.m_vector), alloc)
		{
		}

		constexpr basic_string(std::initializer_list<CharT> ilist, const Allocator &alloc = Allocator())
		    : m_vector(ilist, alloc)
		{
			terminate();
		}

		basic_string(std::nullptr_t) = delete;

		constexpr ~basic_string() = default;

		constexpr basic_string &operator=(const basic_string &str) = default;
		constexpr basic_string &operator=(basic_string &&str) noexcept(
		    std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
		    std::allocator_traits<Allocator>::is_always_equal::value) = default;
		constexpr basic_string &operator=(const CharT *s) { return assign(s, strlen(s)); }
		basic_string &operator=(std::nullptr_t) = delete;
		constexpr basic_string &operator=(CharT ch) { return assign(std::addressof(ch), 1); }
		constexpr basic_string &operator=(std::initializer_list<CharT> ilist) { return assign(ilist.begin(), ilist.size()); }

		constexpr basic_string &assign(size_type count, CharT ch)
		{
			m_vector.assign(count, ch);
			terminate();
			return *this;
		}

		constexpr basic_string &assign(const basic_string &str) { assign(str.begin(), str.end()); }
		constexpr basic_string &assign(const basic_string &str, size_type pos, size_type count = npos)
		{
			return assign(str.begin() + pos, str.begin() + min(count, str.size() - pos));
		}

		constexpr basic_string &assign(basic_string &&str) noexcept(
		    std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
		    std::allocator_traits<Allocator>::is_always_equal::value)
		{
			assign(&str);
			~str();
			return *this;
		}

		constexpr basic_string &assign(const CharT *s, size_type count)
		{
			m_vector.resize(0);
			m_vector.resize(count + 1, 0);

			// Spec: This range can contain null characters.
			memcpy(m_vector.data(), s, count);
			return *this;
		}

		constexpr basic_string &assign(const CharT *s) { return assign(s, strlen(s)); }

		template <class InputIt>
		constexpr basic_string &assign(InputIt first, InputIt last)
		{
			m_vector.assign(first, last);
			terminate();
			return *this;
		}

		constexpr basic_string &assign(std::initializer_list<CharT> ilist) { return assign(ilist.begin(), ilist.end()); }

		constexpr allocator_type get_allocator() const noexcept { return m_vector.get_allocator(); }

		constexpr CharT &at(size_t n)
		{
			assert(n < m_vector.size() - 1);
			return m_vector[n];
		}

		constexpr const CharT &at(size_t n) const
		{
			assert(n < m_vector.size() - 1);
			return m_vector[n];
		}

		constexpr CharT &operator[](size_t n) { return at(n); }
		constexpr const CharT &operator[](size_t n) const { return at(n); }

		constexpr CharT &front() { return at(0); }
		constexpr const CharT &front() const { return at(0); }

		constexpr CharT &back() { return at(size() - 1); }
		constexpr const CharT &back() const { return at(size() - 1); }

		constexpr CharT *data() noexcept { return m_vector.data(); }
		constexpr const CharT *data() const { return m_vector.data(); }
		constexpr const CharT *c_str() const { return this->data(); }

		constexpr iterator begin() noexcept { return iterator{m_vector, 0}; }
		constexpr const_iterator begin() const noexcept { return const_iterator{m_vector, 0}; }
		constexpr iterator end() noexcept { return iterator{m_vector, size()}; }
		constexpr const_iterator end() const noexcept { return const_iterator{m_vector, size()}; }

		constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{m_vector, size()}; }
		constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{m_vector, size()}; }
		constexpr reverse_iterator rend() noexcept { return reverse_iterator{m_vector, 0}; }
		constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{m_vector, 0}; }

		constexpr const_iterator cbegin() const noexcept { return const_iterator{m_vector, 0}; }
		constexpr const_iterator cend() const noexcept { return const_iterator{m_vector, size()}; }
		constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{m_vector, 0}; }
		constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{m_vector, size()}; }

		[[nodiscard]] constexpr bool empty() const { return m_vector.size() <= 1; }
		/* discard */ constexpr std::size_t size() const noexcept { return max<size_type>(1, m_vector.size()) - 1; }
		/* discard */ constexpr size_type max_size() const noexcept { return m_vector.max_size(); }
		constexpr void reserve(size_t n) { m_vector.reserve(n + 1); }
		/* discard */ constexpr size_type capacity() const noexcept { return max<size_type>(1, m_vector.capacity()) - 1; }
		constexpr void shrink_to_fit() {}

		constexpr void clear() noexcept { m_vector.clear(); }

		constexpr basic_string &insert(size_type index, size_type count, CharT ch)
		{
			insert(begin() + index, count, ch);
			return *this;
		}

		constexpr basic_string &insert(size_type index, const CharT *s) { return insert(index, s, strlen(s)); }

		constexpr basic_string &insert(size_type index, const CharT *s, size_type count)
		{
			ensure_not_null();
			m_vector.insert(begin() + index, count, '\0');
			// Spec: This range can contain null characters.
			memcpy(m_vector.data() + index, s, count);
			return *this;
		}

		constexpr basic_string &insert(size_type index, const basic_string &str)
		{
			insert(begin() + index, str.begin(), str.end());
			return *this;
		}

		constexpr basic_string &insert(size_type index, const basic_string &str, size_type s_index, size_type count = npos)
		{
			// TODO: Spec says: Inserts a string, obtained by str.substr(s_index, count) at the position index.
			insert(begin() + index, str.begin() + s_index, str.begin() + min(count, str.size() - s_index));
			return *this;
		}

		constexpr iterator insert(const_iterator pos, CharT ch)
		{
			ensure_not_null();
			return m_vector.insert(pos, ch);
		}

		constexpr iterator insert(const_iterator pos, size_type count, CharT ch)
		{
			ensure_not_null();
			return m_vector.insert(pos, count, ch);
		}

		template <class InputIt>
		constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
		{
			ensure_not_null();
			return m_vector.insert(pos, first, last);
		}

		constexpr iterator insert(const_iterator pos, std::initializer_list<CharT> ilist)
		{
			ensure_not_null();
			return m_vector.insert(pos, ilist);
		};

		constexpr basic_string &erase(size_type index = 0, size_type count = npos)
		{
			erase(begin() + index, min(count, size() - index));
			return *this;
		}

		constexpr iterator erase(const_iterator pos) { return m_vector.erase(pos); }
		constexpr iterator erase(const_iterator first, const_iterator last) { return m_vector.erase(first, last); }

		constexpr void push_back(CharT val) { append(1, val); }

		constexpr void pop_back()
		{
			m_vector.pop_back();
			ensure_terminated();
		}

		constexpr basic_string &append(size_type count, CharT ch) { return insert(size(), count, ch); }
		constexpr basic_string &append(const basic_string &str) { return insert(size(), str); }
		constexpr basic_string &append(const basic_string &str, size_type pos, size_type count = npos) { return insert(size(), str, pos, count); }
		constexpr basic_string &append(const CharT *s, size_type count) { return insert(size(), s, count); }
		constexpr basic_string &append(const CharT *s) { return insert(size(), s); }

		template <class InputIt>
		constexpr basic_string &append(InputIt first, InputIt last)
		{
			insert(end(), first, last);
			return *this;
		}

		constexpr basic_string &append(std::initializer_list<CharT> ilist)
		{
			insert(end(), ilist);
			return *this;
		}

		constexpr basic_string &operator+=(const basic_string &str) { return append(str); };
		constexpr basic_string &operator+=(CharT ch) { return append(1, ch); }
		constexpr basic_string &operator+=(const CharT *s) { return append(s); }
		constexpr basic_string &operator+=(std::initializer_list<CharT> ilist) { return append(ilist); }

		constexpr void resize(size_t n) { m_vector.resize(n + 1, CharT()); }
		constexpr void resize(size_t n, const CharT val) { m_vector.resize(n + 1, val); }

		// TODO: ranges, stringview, traits, replace, copy, resize_and_overwrite, swap, find, compare, starts/ends-with, contains, substr, non-member functions

	private:
		constexpr void ensure_terminated() { m_vector[m_vector.size() - 1] = '\0'; }
		constexpr void terminate() { push_back('\0'); }

		constexpr void ensure_not_null()
		{
			if (!m_vector.data())
				m_vector.resize(1, CharT());
		}

		vector<CharT> m_vector{};
	};

	using string = basic_string<char>;
} // namespace Kernel::LibK
