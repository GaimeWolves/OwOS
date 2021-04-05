#ifndef PANIC_H
#define PANIC_H 1

namespace Kernel
{
	__attribute__((noreturn)) void panic();
	__attribute__((noreturn)) void panic(const char *fmt, ...);
} // namespace Kernel

#endif // PANIC_H