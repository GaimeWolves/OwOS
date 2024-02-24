#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <filesystem/File.hpp>
#include <filesystem/FileContext.hpp>

namespace Kernel
{
	uintptr_t syscall$open(const char *path, int oflag, unsigned mode)
	{
		(void)oflag;
		(void)mode;

		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);

		auto *file = VirtualFileSystem::instance().find_by_path(path, process->get_cwd());

		if (!file)
			return -ENOENT;

		auto context = file->open(O_RDWR);

		if (context.is_null())
			return -EACCES;

		int fd = process->add_file(std::move(context));

		return fd;
	}
}
