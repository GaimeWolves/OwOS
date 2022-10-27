#include <syscall/SyscallDispatcher.hpp>

#include <libk/kvector.hpp>

#include <syscall/syscalls.hpp>

// TODO: Replace when out libc is mature enough
#include "../../userland/libc/sys/arch/i386/syscall.h"

namespace Kernel
{
	LibK::vector<void (*)()> SyscallDispatcher::s_syscall_table;

	static const char *s_syscall_names[] = {
#define SYSCALL(name) #name,
	    __ENUM_SYSCALL(SYSCALL)
#undef SYSCALL
	};

	uint32_t SyscallDispatcher::handle_syscall(uint32_t id, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
	{
		assert(id < s_syscall_table.size());
		if (id != __SC_write)
			log("SYSCALL", "%s(%p,%p,%p,%p,%p)", s_syscall_names[id], arg1, arg2, arg3, arg4, arg5);
		return reinterpret_cast<syscall_t>(s_syscall_table[id])(arg1, arg2, arg3, arg4, arg5);
	}

	void SyscallDispatcher::initialize()
	{
#define SYSCALL(name) s_syscall_table.push_back(reinterpret_cast<void (*)()>(syscall$##name));
		__ENUM_SYSCALL(SYSCALL)
#undef SYSCALL
	}
}