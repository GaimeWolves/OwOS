#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <filesystem/File.hpp>

namespace Kernel
{
	uintptr_t syscall$chdir(char *path)
	{
		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);

		if (!path || path[0] == '\0')
			return -ENOENT;

		File *file = VirtualFileSystem::instance().find_by_path(path, process->get_cwd());

		if (!file)
			return -ENOENT;

		if (!file->is_type(FileType::Directory))
			return -ENOTDIR;

		LibK::string full_path = VirtualFileSystem::instance().get_full_path(file);
		if (full_path[full_path.size() - 1] != '/')
			full_path += '/';
		process->set_cwd(full_path.c_str());

		return -ESUCCESS;
	}
}
