#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <filesystem/FileContext.hpp>
#include <filesystem/VirtualFileSystem.hpp>

namespace Kernel
{
	uintptr_t syscall$stat(int type, int cwd, const char *path, int fd, struct stat *buf, int flags)
	{
		(void)cwd;
		(void)flags;

		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		File *file;

		switch (type)
		{
		case __SC_stat_TYPE_STAT:
			if (!path)
				return -ENOENT;

			file = VirtualFileSystem::instance().find_by_path(path);
			break;
		case __SC_stat_TYPE_FSTAT:
			assert(process);
			file = &process->get_file_by_index(fd).file();
			break;
		default:
			return -ENOTSUP;
		}

		if (!file)
			return -ENOENT;

		memset(buf, 0, sizeof(struct stat));
		buf->st_size = file->size();

		if (file->is_directory())
			buf->st_mode = S_IFDIR;
		else
			buf->st_mode = S_IFREG;

		return 0;
	}
};