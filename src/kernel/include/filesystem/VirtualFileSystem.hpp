#pragma once

#include <stddef.h>

#include <libk/kstring.hpp>

#include <filesystem/definitions.hpp>

namespace Kernel
{
	class VirtualFileSystem
	{
	public:
		static VirtualFileSystem &instance()
		{
			static VirtualFileSystem *instance{nullptr};

			if (!instance)
				instance = new VirtualFileSystem();

			return *instance;
		}

		VirtualFileSystem(VirtualFileSystem &) = delete;
		void operator=(const VirtualFileSystem &) = delete;

		bool initialize(Device &root_device);

		FileSystem *mount(Device &device, File &mount_point);
		bool unmount(FileSystem *fileSystem);

		File *find_by_path(const LibK::string &path);

	private:
		VirtualFileSystem() = default;
		~VirtualFileSystem() = default;

		FileSystem &initialize_filesystem_on(Device &device);

		FileSystem *m_root_node;
	};
}