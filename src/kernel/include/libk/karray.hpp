#pragma once

#include <stddef.h>

#include <libk/kcassert.hpp>

namespace Kernel::LibK
{
	template <typename T, size_t Size>
	class array
	{
	public:
		typedef T value_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef T &reference;
		typedef const T &const_reference;
		typedef T *pointer;
		typedef const T *const_pointer;

		constexpr reference operator[](size_type pos) { assert(pos < Size); return data[pos]; }
		constexpr const_reference operator[](size_type pos) const { assert(pos < Size); return data[pos]; }

		constexpr explicit operator pointer() { return data; }

		[[nodiscard]] constexpr size_type size() const noexcept { return Size; }

	private:
		alignas(T) T data[Size];
	};
} // namespace Kernel::LibK