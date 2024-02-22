#pragma once

#include <libk/kcstddef.hpp>

#include <limits>
#include <type_traits>

#include <libk/__allocators.hpp>
#include <libk/__iterators.hpp>
#include <libk/kcassert.hpp>
#include <libk/kcmalloc.hpp>
#include <libk/kcstring.hpp>
#include <libk/kfunctional.hpp>
#include <libk/kmath.hpp>
#include <libk/kmemory.hpp>
#include <libk/kutility.hpp>

namespace Kernel::LibK
{
	template <typename T, typename Allocator = allocator<T>>
	class vector
	{
	public:
		typedef T value_type;
		typedef Allocator allocator_type;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef value_type &reference;
		typedef const value_type &const_reference;
		typedef T *pointer;
		typedef const T *const_pointer;
		typedef normal_iterator<vector<T>, T> iterator;
		typedef normal_iterator<const vector<T>, const T> const_iterator;

		constexpr vector() noexcept(noexcept(Allocator())) = default;

		constexpr explicit vector(const Allocator &alloc) noexcept
		    : m_allocator(alloc)
		{
		}

		constexpr vector(size_type count, const T &value, const Allocator &alloc = Allocator())
		    : m_allocator(alloc)
		    , m_capacity(next_power_of_two(count))
		    , m_size(count)
		{
			m_array = (T *)kmalloc(sizeof(T) * m_capacity, sizeof(T));

			for (size_t i = 0; i < count; i++)
				m_array[i] = value;
		}

		constexpr explicit vector(size_type count, const Allocator &alloc = Allocator())
		    : m_allocator(alloc)
		    , m_capacity(next_power_of_two(count))
		    , m_size(count)
		{
			m_array = (T *)kmalloc(sizeof(T) * m_capacity, sizeof(T));

			for (size_t i = 0; i < count; i++)
				std::allocator_traits<Allocator>::construct(m_allocator, &m_array[i]);
		}

		template <class InputIt>
		constexpr vector(InputIt first, InputIt last, const Allocator &alloc = Allocator())
		    : m_allocator(alloc)
		    , m_capacity(next_power_of_two(std::distance(first, last)))
		    , m_size(std::distance(first, last))
		{
			m_array = (T *)kmalloc(sizeof(T) * m_capacity, sizeof(T));

			size_t idx = 0;
			for (auto it = first; it != last; ++it)
				m_array[idx++] = *it;
		}

		constexpr vector(const vector &other)
		    : vector(other, std::allocator_traits<Allocator>::select_on_container_copy_construction(other.get_allocator()))
		{
		}

		constexpr vector(const vector &other, const Allocator &alloc)
		    : m_allocator(alloc)
		    , m_capacity(other.m_capacity)
		    , m_size(other.m_size)
		{
			m_array = (T *)kmalloc(sizeof(T) * m_capacity, sizeof(T));

			for (size_t i = 0; i < m_size; i++)
				m_array[i] = other[i];
		}

		constexpr vector(vector &&other) noexcept
		    : vector(other, other.m_allocator)
		{
		}

		constexpr vector(vector &&other, const Allocator &alloc) noexcept
		    : m_allocator(std::move_if_noexcept(alloc))
		    , m_capacity(other.m_capacity)
		    , m_size(other.m_size)
		{
			if constexpr (m_allocator == other.m_allocator)
			{
				m_array = other.m_array;
				other.m_array = nullptr;
				other.m_capacity = 0;
				other.m_size = 0;
			}
			else
			{
				m_array = std::allocator_traits<Allocator>::allocate(m_allocator, m_capacity);
				for (size_t i = 0; i < m_size; i++)
					std::allocator_traits<Allocator>::construct(m_allocator, m_array[i], std::move_if_noexcept(other[i]));
				other.~vector();
			}
		}

		constexpr vector(std::initializer_list<T> init, const Allocator &alloc = Allocator())
		    : m_allocator(alloc)
		    , m_capacity(next_power_of_two(init.size()))
		    , m_size(init.size())
		{
			m_array = std::allocator_traits<Allocator>::allocate(m_allocator, m_capacity);

			size_t idx = 0;
			for (auto it = init.begin(); it != init.end(); ++it)
				m_array[idx++] = *it;
		}

		constexpr ~vector()
		{
			if (!m_array)
				return;

			std::allocator_traits<Allocator>::deallocate(m_allocator, m_array, m_capacity);
			m_array = nullptr;
			m_size = 0;
			m_capacity = 0;
		}

		constexpr vector &operator=(const vector &other)
		{
			if (this == &other)
				return *this;

			this->~vector();

			if constexpr (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value)
				m_allocator = std::allocator_traits<Allocator>::select_on_container_copy_construction(other.m_allocator);

			m_capacity = other.m_capacity;
			m_size = other.m_size;

			if (m_capacity == 0)
				return *this;

			m_array = std::allocator_traits<Allocator>::allocate(m_allocator, m_capacity);

			for (size_t i = 0; i < m_size; i++)
				m_array[i] = other[i];

			return *this;
		};

		constexpr vector &operator=(vector &&other) noexcept(
		    std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value)
		{
			this->~vector();

			m_capacity = other.m_capacity;
			m_size = other.m_size;

			if constexpr (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || m_allocator == other.m_allocator)
			{
				m_array = other.m_array;
				other.m_array = nullptr;
				other.m_capacity = 0;
				other.m_size = 0;
			}
			else if (m_capacity > 0)
			{
				m_array = std::allocator_traits<Allocator>::allocate(m_allocator, m_capacity);
				for (size_t i = 0; i < m_size; i++)
					std::allocator_traits<Allocator>::construct(m_allocator, &m_array[i], std::move_if_noexcept(other[i]));
				other.~vector();
			}

			return *this;
		}

		constexpr vector &operator=(std::initializer_list<T> ilist)
		{
			this->~vector();

			m_capacity = next_power_of_two(ilist.size());
			m_size = ilist.size();
			m_array = std::allocator_traits<Allocator>::allocate(m_allocator, m_capacity);

			size_t idx = 0;
			for (auto it = ilist.begin(); it != ilist.end(); ++it)
				m_array[idx++] = *it;
		}

		constexpr allocator_type get_allocator() const noexcept { return m_allocator; }

		constexpr reference at(size_type pos)
		{
			assert(pos < m_size);
			return m_array[pos];
		}

		constexpr const_reference &at(size_type pos) const
		{
			assert(pos < m_size);
			return m_array[pos];
		}

		constexpr reference operator[](size_type pos) { return at(pos); }
		constexpr const_reference operator[](size_type pos) const { return at(pos); }

		constexpr reference front() { return at(0); }
		constexpr const_reference front() const { return at(0); }

		constexpr reference back() { return at(m_size - 1); }
		constexpr const_reference back() const { return at(m_size - 1); }

		constexpr T *data() noexcept { return m_array; }
		constexpr const T *data() const { return m_array; }

		constexpr iterator begin() noexcept { return iterator{*this, 0}; }
		constexpr const_iterator begin() const noexcept { return const_iterator{*this, 0}; }

		constexpr iterator end() noexcept { return iterator{*this, m_size}; }
		constexpr const_iterator end() const noexcept { return const_iterator{*this, m_size}; }

		[[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }
		/* discard */ constexpr size_type size() const noexcept { return m_size; }
		/* discard */ constexpr size_type max_size() const noexcept { return std::numeric_limits<difference_type>::max(); }

		constexpr void reserve(size_type new_cap)
		{
			if (m_capacity >= new_cap)
				return;

			size_type old_capacity = m_capacity;

			m_capacity = max(4, next_power_of_two(new_cap));

			T *new_array = std::allocator_traits<Allocator>::allocate(m_allocator, m_capacity);
			assert(new_array);

			for (size_t i = 0; i < m_size; i++)
				new (&new_array[i]) T(move(m_array[i]));

			std::allocator_traits<Allocator>::deallocate(m_allocator, m_array, old_capacity);
			m_array = new_array;
		}

		/* discard */ constexpr size_type capacity() const noexcept { return m_capacity; }

		constexpr void clear() noexcept { resize(0); }

		constexpr iterator insert(const_iterator pos, const T &value) { return insert(pos, 1, value); }

		constexpr iterator insert(const_iterator pos, T &&value)
		{
			assert(pos.m_index >= 0 && pos.m_index < m_size);
			reserve(m_size + 1);

			// move all elements after pos.m_index 1 step right
			size_type current = m_size - pos.m_index;
			while (current-- > 0)
				std::allocator_traits<Allocator>::construct(m_allocator, &m_array[pos.m_index + current + 1], std::move(m_array[pos.m_index + current]));

			new (&m_array[pos.m_index]) T(std::move(value));

			m_size++;
			return iterator{*this, pos.m_index};
		}

		constexpr iterator insert(const_iterator pos, size_t n, const T &value)
		{
			assert(pos.m_index >= 0 && pos.m_index < m_size);
			reserve(m_size + n);

			// move all elements after pos.m_index n steps right
			size_type current = m_size - pos.m_index;
			while (current-- > 0)
				std::allocator_traits<Allocator>::construct(m_allocator, &m_array[pos.m_index + current + n], std::move(m_array[pos.m_index + current]));

			for (size_t i = 0; i < n; i++)
				m_array[pos.m_index + i] = value;

			m_size += n;
			return iterator{*this, pos.m_index};
		}

		constexpr iterator erase(const_iterator pos)
		{
			assert(pos.m_index >= 0 && pos.m_index < m_size);
			m_array[pos.m_index].~T();

			// move all elements after pos.m_index 1 step left
			for (size_type i = 1; i < m_size - pos.m_index; i++)
				std::allocator_traits<Allocator>::construct(m_allocator, &m_array[pos.m_index + i - 1], std::move(m_array[pos.m_index + i]));

			m_size--;
			return {*this, min(m_size, pos.m_index)};
		}

		constexpr iterator erase(const_iterator first, const_iterator last)
		{
			assert(first.m_index >= 0 && last.m_index < m_size && first <= last);

			if (first == last)
				return {*this, first.m_index};

			size_t n = std::distance(first, last);

			for (size_type i = first.m_index; i < last.m_index; i++)
				m_array[i].~T();

			// move all elements after last.m_index - 1 n step left
			for (size_type i = 0; i < m_size - last.m_index; i++)
				std::allocator_traits<Allocator>::construct(m_allocator, &m_array[last.m_index + i - n], std::move(m_array[last.m_index + i]));

			m_size -= n;
			return {*this, min(m_size, first.m_index)};
		}

		constexpr void push_back(const T &val) { resize(m_size + 1, val); }
		constexpr void push_back(T &&val)
		{
			reserve(m_size + 1);
			new (&m_array[m_size++]) T(move(val));
		}

		template <class... Args>
		constexpr reference emplace_back(Args &&...args)
		{
			reserve(m_size + 1);
			std::allocator_traits<Allocator>::construct(m_allocator, &m_array[m_size], std::forward<Args>(args)...);
			return m_array[m_size++];
		}

		constexpr void pop_back() { shrink(m_size - 1); }

		constexpr void resize(size_type count)
		{
			if (shrink(count))
				return;

			reserve(count);

			for (size_t i = m_size; i < count; i++)
				std::allocator_traits<Allocator>::construct(m_allocator, &m_array[i]);

			m_size = count;
		}

		constexpr void resize(size_t count, const value_type &value)
		{
			if (shrink(count))
				return;

			reserve(count);

			for (size_t i = m_size; i < count; i++)
				m_array[i] = value;

			m_size = count;
		}

		// TODO: maybe start using concepts

		template <class Functor>
		[[nodiscard]] constexpr bool any_of(const Functor &callback) const
		{
			for (const auto &element : *this)
			{
				if (callback(element))
					return true;
			}

			return false;
		}

		template <class Functor>
		[[nodiscard]] constexpr bool all_of(const Functor &callback) const
		{
			for (const auto &element : *this)
			{
				if (!callback(element))
					return false;
			}

			return true;
		}

	protected:
		constexpr bool shrink(size_type count)
		{
			if (count < m_size)
			{
				for (size_t i = count; i < m_size; i++)
					m_array[i].~T();

				m_size = count;
				return true;
			}

			return count <= m_size;
		}

		Allocator m_allocator{};

		size_t m_capacity{0};
		size_t m_size{0};

		T *m_array{nullptr};
	};
} // namespace Kernel::LibK
