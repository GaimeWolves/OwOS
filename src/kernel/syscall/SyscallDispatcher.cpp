#include <syscall/SyscallDispatcher.hpp>

#include <libk/kvector.hpp>

#include <syscall/syscalls.hpp>

#define SYSCALL(name) s_syscall_table.push_back(reinterpret_cast<void (*)()>(syscall$##name))

namespace Kernel
{
	LibK::vector<void (*)()> SyscallDispatcher::s_syscall_table;

	uint32_t SyscallDispatcher::handle_syscall(uint32_t id, uint32_t args...)
	{
		assert(id < s_syscall_table.size());
		return reinterpret_cast<syscall_t>(s_syscall_table[id])(args);
	}

	void SyscallDispatcher::initialize()
	{
		SYSCALL(test);
	}
}