#pragma once

#include <stddef.h>

#include <libk/kcassert.hpp>

namespace Kernel::LibK
{
	template <class T, std::size_t N>
	struct array
	{
	public:
		using value_type = T;
		using pointer = T *;
		using const_pointer = const T *;
		using reference = T &;
		using const_reference = const T &;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using iterator = normal_iterator<array<T, N>, T>;
		using const_iterator = normal_iterator<const array<T, N>, const T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		constexpr reference at(size_type pos)
		{
			assert(pos < size());
			return m_data[pos];
		}

		constexpr const_reference at(size_type pos) const
		{
			assert(pos < size());
			return m_data[pos];
		}

		constexpr reference operator[](size_type pos) { return at(pos); }
		constexpr const_reference operator[](size_type pos) const { return at(pos); }

		constexpr reference front() { return m_data[0]; }
		constexpr const_reference front() const { return m_data[0]; }

		constexpr reference back() { return m_data[size() - 1]; }
		constexpr const_reference back() const { return m_data[size() - 1]; }

		constexpr T *data() noexcept { return m_data; }
		constexpr const T *data() const { return m_data; }

		constexpr iterator begin() noexcept { return iterator{*this, 0}; }
		constexpr const_iterator begin() const noexcept { return const_iterator{*this, 0}; }
		constexpr iterator end() noexcept { return iterator{*this, size()}; }
		constexpr const_iterator end() const noexcept { return const_iterator{*this, size()}; }

		constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{*this, size()}; }
		constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{*this, size()}; }
		constexpr reverse_iterator rend() noexcept { return reverse_iterator{*this, 0}; }
		constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{*this, 0}; }

		constexpr const_iterator cbegin() const noexcept { return const_iterator{*this, 0}; }
		constexpr const_iterator cend() const noexcept { return const_iterator{*this, size()}; }
		constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{*this, 0}; }
		constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{*this, size()}; }

		[[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
		[[nodiscard]] constexpr size_type size() const noexcept { return N; }
		/* discard */ constexpr size_type max_size() const noexcept { return size(); }

		constexpr void fill(const T &value)
		{
			for (size_type i = 0; i < size(); i++)
				m_data[i] = value;
		}

		constexpr void swap(array &other) noexcept(std::is_nothrow_swappable_v<T>) { std::swap(m_data, other.m_data); }

	private:
		alignas(T) T m_data[N];
	};

	template <class T, class... U>
	array(T, U...) -> array<T, 1 + sizeof...(U)>;

	template <std::size_t I, class T, std::size_t N>
	constexpr T &get(array<T, N> &a) noexcept { return a[I]; }

	template <std::size_t I, class T, std::size_t N>
	constexpr T &&get(array<T, N> &&a) noexcept { return std::move(a[I]); }

	template <std::size_t I, class T, std::size_t N>
	constexpr const T &get(const array<T, N> &a) noexcept { return a[I]; }

	template <std::size_t I, class T, std::size_t N>
	constexpr const T &&get(const array<T, N> &&a) noexcept { return std::move(a[I]); }

	template <class T, std::size_t N>
	constexpr void swap(array<T, N> &lhs, array<T, N> &rhs) noexcept(noexcept(lhs.swap(rhs)))
	{
		lhs.swap(rhs);
	}

	template <class T, std::size_t N>
	constexpr array<std::remove_cv_t<T>, N> to_array(T (&a)[N]) { return array<T, N>(a); }

	template <class T, std::size_t N>
	constexpr std::array<std::remove_cv_t<T>, N> to_array(T (&&a)[N]) { return array<T, N>(std::move(a)); };
} // namespace Kernel::LibK
