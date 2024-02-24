#pragma once

#include <memory>

#include <libk/kshared_ptr.hpp>
#include <libk/kutility.hpp>

namespace Kernel::LibK
{
	template <class Pointer, class SizeType = std::size_t>
	struct allocation_result
	{
		Pointer ptr;
		SizeType count;
	};

	template <class T, class... Args>
	shared_ptr<T> make_shared(Args &&...args) { return shared_ptr<T>(new T(std::forward<Args>(args)...)); }

	template <class T, class... Args>
	constexpr T *construct_at(T *p, Args &&...args) { return ::new (static_cast<void *>(p)) T(std::forward<Args>(args)...); }

	template <class T>
	constexpr void destroy_at(T *p)
	{
		if constexpr (std::is_array_v<T>)
			for (auto &elem : *p)
				(destroy_at)(std::addressof(elem));
		else
			p->~T();
	}
} // namespace Kernel::LibK
