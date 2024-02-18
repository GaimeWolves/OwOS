#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <filesystem/File.hpp>

namespace Kernel
{
	uintptr_t syscall$ioctl(int fd, uint32_t request, uintptr_t arg)
	{
		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);

		auto file = process->get_file_by_index(fd);
		auto return_code = file.file().ioctl(request, arg).error();

		return -return_code;
	}
}
