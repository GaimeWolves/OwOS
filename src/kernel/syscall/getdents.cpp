#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <filesystem/File.hpp>
#include <filesystem/VirtualFileSystem.hpp>

#include "../../userland/libc/dirent.h"

namespace Kernel
{
	uintptr_t syscall$getdents(int fd, void *buffer, size_t count)
	{
		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);
		auto &file = process->get_file_by_index(fd);

		size_t written = file.read_directory(count, static_cast<char *>(buffer));

		return written;
	}
}