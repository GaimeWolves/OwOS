#pragma once

#include <libk/kiterator.hpp>
#include <libk/ktype_traits.hpp>

namespace Kernel::LibK
{
	template <typename Container, typename T, typename Iterator = iterator<random_access_iterator_tag, typename remove_reference<T>::type>>
	class normal_iterator : public Iterator
	{
	public:
		friend Container;

		typedef typename Iterator::value_type value_type;
		typedef typename Iterator::difference_type difference_type;
		typedef typename Iterator::pointer pointer;
		typedef typename Iterator::reference reference;
		typedef typename Iterator::iterator_category iterator_category;

		normal_iterator() = default;
		normal_iterator(const normal_iterator &) = default;
		~normal_iterator() = default;

		normal_iterator &operator=(const normal_iterator &rhs)
		{
			m_index = rhs.m_index;
			return *this;
		}

		bool operator==(const normal_iterator &rhs) const { return m_index == rhs.m_index; }
		bool operator!=(const normal_iterator &rhs) const { return m_index != rhs.m_index; }
		bool operator<(const normal_iterator &rhs) const { return m_index < rhs.m_index; }
		bool operator>(const normal_iterator &rhs) const { return m_index > rhs.m_index; }
		bool operator<=(const normal_iterator &rhs) const { return m_index <= rhs.m_index; }
		bool operator>=(const normal_iterator &rhs) const { return m_index >= rhs.m_index; }

		normal_iterator &operator++()
		{
			m_index++;
			return *this;
		}

		normal_iterator operator++(int)
		{
			m_index++;
			return normal_iterator{m_container, m_index - 1};
		}

		normal_iterator &operator--()
		{
			m_index--;
			return *this;
		}

		normal_iterator operator--(int)
		{
			m_index--;
			return normal_iterator{m_container, m_index + 1};
		}

		normal_iterator &operator+=(difference_type offset)
		{
			m_index += offset;
			return *this;
		}

		normal_iterator operator+(difference_type offset) const
		{
			return normal_iterator{m_container, m_index + offset};
		}

		friend normal_iterator operator+(difference_type offset, const normal_iterator &rhs)
		{
			return normal_iterator{rhs.m_container, rhs.m_index + offset};
		}

		normal_iterator &operator-=(difference_type offset)
		{
			m_index -= offset;
			return *this;
		}

		normal_iterator operator-(difference_type offset) const
		{
			return normal_iterator{m_container, m_index - offset};
		}

		difference_type operator-(normal_iterator rhs) const
		{
			return normal_iterator{m_container, m_index - rhs.m_index};
		}

		reference operator*() const { return m_container.at(m_index); }
		pointer operator->() const { return &m_container.at(m_index); }
		reference operator[](difference_type offset) const { return m_container.at(m_index + offset); }

	private:
		normal_iterator(Container &container, size_t index)
		    : m_container(container), m_index(index)
		{
		}

		Container &m_container;
		size_t m_index{0};
	};
} // namespace Kernel::LibK