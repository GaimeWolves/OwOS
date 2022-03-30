#pragma once

#include <stddef.h>

#include <arch/process.hpp>
#include <locking/Mutex.hpp>

namespace Kernel
{
	constexpr size_t KERNEL_STACK_SIZE = 32 * 1024;

	enum class ThreadState
	{
		Running,
		Waiting,
		Blocked,
		Sleeping,
		Terminated,
	};

	// Currently, this only holds the register state to use when switching contexts.
	typedef struct thread_t
	{
		thread_registers_t registers;
		bool has_started;
		uintptr_t kernel_stack;
		ThreadState state;
		Locking::Mutex *lock;
	} thread_t;
}