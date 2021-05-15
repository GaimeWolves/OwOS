#pragma once

#include <common_attributes.h>

namespace Kernel
{
	__noreturn void panic();
	__noreturn void panic(const char *fmt, ...);
} // namespace Kernel
