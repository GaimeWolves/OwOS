#pragma once

#include <libk/ktype_traits.hpp>

namespace Kernel::LibK
{
	template <typename T>
	auto declval() -> T;

	template <class T>
	constexpr T &&forward(typename remove_reference<T>::type &t) noexcept
	{
		return static_cast<T &&>(t);
	}

	template <class T>
	constexpr T &&forward(typename remove_reference<T>::type &&t) noexcept
	{
		static_assert(!is_lvalue_reference<T>::value, "Cannot forward an rvalue as an lvalue.");
		return static_cast<T &&>(t);
	}

	template <class T>
	constexpr typename remove_reference<T>::type &&move(T &&t) noexcept
	{
		return static_cast<typename remove_reference<T>::type &&>(t);
	}
} // namespace Kernel::LibK
