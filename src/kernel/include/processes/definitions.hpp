#pragma once

#include <stddef.h>

#include <arch/process.hpp>
#include <locking/Mutex.hpp>
#include <memory/definitions.hpp>

namespace Kernel
{
	constexpr size_t KERNEL_STACK_SIZE = 32 * 1024;

	class Process;

	enum class ThreadState
	{
		Running,
		Ready,
		Blocked,
		Suspended,
		Sleeping,
		Terminated,
	};

	typedef struct thread_t
	{
		thread_registers_t registers;
		bool has_started;
		uintptr_t kernel_stack;
		Memory::memory_region_t kernel_stack_region;
		ThreadState state;
		Locking::Mutex *lock;
		Process *parent_process;
	} thread_t;
}
