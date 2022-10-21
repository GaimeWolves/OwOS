#include <syscall/syscalls.hpp>

namespace Kernel
{
	uint32_t syscall$test(const char *str)
	{
		log("SYSCALL", str);
		return 0;
	}
}