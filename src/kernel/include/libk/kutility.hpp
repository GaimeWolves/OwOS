#pragma once

#include <libk/kcstddef.hpp>
#include <utility>

namespace Kernel::LibK
{
	template <class T, size_t N>
	constexpr size_t size(T (&)[N]) { return N; }
} // namespace Kernel::LibK
