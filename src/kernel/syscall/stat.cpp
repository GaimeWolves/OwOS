#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <filesystem/FileContext.hpp>

namespace Kernel
{
	uintptr_t syscall$stat(int fd, struct stat *buf)
	{
		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);
		auto &file = process->get_file_by_index(fd);

		buf->st_size = file.size();

		return 0;
	}
};