#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <filesystem/File.hpp>

namespace Kernel
{
	uintptr_t syscall$getcwd(char *buf, size_t size)
	{
		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);

		const LibK::string &cwd = process->get_cwd();

		if (size == 0)
			return -EINVAL;

		if (size < cwd.size() + 1)
			return -ERANGE;

		strcpy(buf, cwd.c_str());

		return (uintptr_t)buf;
	}
}