#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <filesystem/File.hpp>

namespace Kernel
{
	uintptr_t syscall$fork()
	{
		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);

		Process *new_process = process->fork();

		if (!new_process)
			return -EAGAIN;

		return new_process->get_pid();
	}
}