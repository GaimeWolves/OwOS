#pragma once

namespace Kernel
{
	__attribute__((noreturn)) void panic();
	__attribute__((noreturn)) void panic(const char *fmt, ...);
} // namespace Kernel
