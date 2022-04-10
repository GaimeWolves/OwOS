#pragma once

#include <libk/kcstdio.hpp>

namespace Kernel
{
	uint32_t syscall$test(const char *str)
	{
		LibK::printf_debug_msg(str);
		return 0;
	}
}