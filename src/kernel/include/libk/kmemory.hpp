#pragma once

#include <libk/kshared_ptr.hpp>
#include <libk/kutility.hpp>

namespace Kernel::LibK
{
	template <class T, class... Args>
	shared_ptr<T> make_shared(Args &&...args)
	{
		return shared_ptr<T>(new T(forward<Args>(args)...));
	}
}
