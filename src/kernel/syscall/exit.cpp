#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <filesystem/File.hpp>

namespace Kernel
{
	[[noreturn]] uintptr_t syscall$exit(int exit_code)
	{
		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);

		process->exit((int8_t)(exit_code & 0xFF), 0);

		CoreScheduler::yield();
	}
}