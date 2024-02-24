#pragma once

#include <iterator>
#include <type_traits>

namespace Kernel::LibK
{
	template <typename Container, typename T>
	class normal_iterator
	{
	public:
		friend Container;

		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T *;
		using reference = T &;

		constexpr normal_iterator() = default;
		constexpr normal_iterator(const normal_iterator &) = default;
		constexpr ~normal_iterator() = default;

		constexpr normal_iterator &operator=(const normal_iterator &rhs)
		{
			m_index = rhs.m_index;
			return *this;
		}

		/* implicit */ constexpr operator normal_iterator<const Container, const T>() const { return {m_container, m_index}; }

		constexpr bool operator==(const normal_iterator &rhs) const { return m_index == rhs.m_index; }
		constexpr difference_type operator<=>(const normal_iterator &rhs) const { return static_cast<difference_type>(m_index) - static_cast<difference_type>(rhs.m_index); }

		constexpr normal_iterator &operator++()
		{
			m_index++;
			return *this;
		}

		constexpr normal_iterator operator++(int)
		{
			m_index++;
			return normal_iterator{m_container, m_index - 1};
		}

		constexpr normal_iterator &operator--()
		{
			m_index--;
			return *this;
		}

		constexpr normal_iterator operator--(int)
		{
			m_index--;
			return normal_iterator{m_container, m_index + 1};
		}

		constexpr normal_iterator &operator+=(difference_type offset)
		{
			m_index += offset;
			return *this;
		}

		constexpr normal_iterator operator+(difference_type offset) const
		{
			return normal_iterator{m_container, m_index + offset};
		}

		constexpr friend normal_iterator operator+(difference_type offset, const normal_iterator &rhs)
		{
			return normal_iterator{rhs.m_container, rhs.m_index + offset};
		}

		constexpr normal_iterator &operator-=(difference_type offset)
		{
			m_index -= offset;
			return *this;
		}

		constexpr normal_iterator operator-(difference_type offset) const
		{
			return normal_iterator{m_container, m_index - offset};
		}

		constexpr difference_type operator-(normal_iterator rhs) const
		{
			return (difference_type)m_index - (difference_type)rhs.m_index;
		}

		constexpr reference operator*() const { return m_container.at(m_index); }
		constexpr pointer operator->() const { return &m_container.at(m_index); }
		constexpr reference operator[](difference_type offset) const { return m_container.at(m_index + offset); }

		constexpr normal_iterator(Container &container, size_t index)
		    : m_container(container), m_index(index)
		{
		}

	private:
		Container &m_container;
		size_t m_index{0};
	};
} // namespace Kernel::LibK
