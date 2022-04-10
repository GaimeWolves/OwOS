#pragma once

#include <interrupts/IRQHandler.hpp>

namespace Kernel
{
	class SyscallDispatcher
	{
	public:
		static void initialize();
		static uint32_t handle_syscall(uint32_t id, uint32_t args...);

	private:
		typedef uint32_t (*syscall_t)(uint32_t args...);
		static LibK::vector<void (*)()> s_syscall_table;
	};
}