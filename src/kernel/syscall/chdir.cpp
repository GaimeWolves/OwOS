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

		// TODO: Move check for absolute/relative path somewhere logical
		// TODO: Handle . and .. correctly inside VFS

		LibK::string full_path;

		if (path[0] != '/')
		{
			for (auto ch : process->get_cwd())
				full_path += ch;

			char ch = *(full_path.end() - 1);
			if (ch != '/')
				full_path += '/';
		}

		size_t len = strlen(path);
		for (size_t i = 0; i < len; i++)
			full_path += path[i];

		File *file = VirtualFileSystem::instance().find_by_path(full_path);

		if (!file)
			return -ENOENT;

		if (!file->is_directory())
			return -ENOTDIR;

		process->set_cwd(full_path.c_str());

		return -ESUCCESS;
	}
}