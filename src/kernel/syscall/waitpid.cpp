#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <filesystem/File.hpp>

namespace Kernel
{
	uintptr_t syscall$waitpid(pid_t pid, int *stat_loc, int options)
	{
		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);

		LibK::ErrorOr<pid_t> ret = process->waitpid(pid, stat_loc, options);

		if (ret.has_error())
			return -ret.error();

		return ret.data();
	}
}