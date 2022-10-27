#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <filesystem/File.hpp>

namespace Kernel
{
	uintptr_t syscall$close(int fd)
	{
		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);

		process->remove_file(fd);

		return 0;
	}
}