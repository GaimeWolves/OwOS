#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <arch/memory.hpp>
#include <filesystem/File.hpp>

namespace Kernel
{
	uintptr_t syscall$read(int fd, void *buf, size_t count)
	{
		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);
		auto &file = process->get_file_by_index(fd);

		file.file().lock();
		uintptr_t read = file.read(count, static_cast<char *>(buf));
		file.file().unlock();

		return read;
	}
}
