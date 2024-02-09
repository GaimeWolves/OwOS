#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <filesystem/File.hpp>
#include <elf/elf.hpp>

namespace Kernel
{
	uintptr_t syscall$exec(const char *path, const char *argv[], const char *envp[])
	{
		auto &core = CPU::Processor::current();
		auto process = core.get_current_thread()->parent_process;
		assert(process);

		File *file = VirtualFileSystem::instance().find_by_path(path, process->get_cwd());

		if (!file)
			return -ENOENT;

		if (!file->is_type(FileType::RegularFile))
			return -EACCES;

		if (!ELF::is_executable(file))
			return -ENOEXEC;

		LibK::vector<const char *> argv_copy;
		LibK::vector<const char *> envp_copy;

		while (*argv)
		{
			char *dest = static_cast<char *>(kmalloc(strlen(*argv) + 1));
			strcpy(dest, *argv);
			argv_copy.push_back(dest);
			argv++;
		}
		argv_copy.push_back(NULL);

		while (*envp)
		{
			char *dest = static_cast<char *>(kmalloc(strlen(*envp) + 1));
			strcpy(dest, *envp);
			envp_copy.push_back(dest);
			envp++;
		}
		envp_copy.push_back(NULL);

		process->exec(file, const_cast<const char **>(argv_copy.data()), const_cast<const char **>(envp_copy.data()));

		for (auto str : argv_copy)
			kfree((void *)str);

		for (auto str : envp_copy)
			kfree((void *)str);

		return 0;
	}
}