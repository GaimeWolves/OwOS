#pragma once

#include "memory/VirtualMemoryManager.hpp"
#include "filesystem/definitions.hpp"
#include <processes/Process.hpp>

namespace Kernel::ELF
{
	// Basic loader to load the dynamic loader which loads the actual program
	thread_t *load(Process *parent_process, File *file, const char **argv, const char **envp, bool is_exec_syscall);

	bool is_executable(File *file);
}
