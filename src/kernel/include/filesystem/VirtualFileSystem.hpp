#pragma once

#include <stddef.h>

#include <libk/kstring.hpp>

#include <filesystem/definitions.hpp>
#include <storage/PartitionDevice.hpp>

namespace Kernel
{
	class VirtualFileSystem
	{
		typedef struct __vfs_node_t
		{
			File *file;
			LibK::vector<__vfs_node_t *> children; // TODO: Replace with HashMap maybe?
			__vfs_node_t *parent;
		} vfs_node_t;

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

		bool initialize(BlockDevice &root_device);

		FileSystem *mount(BlockDevice &device, File &mount_point);
		bool unmount(FileSystem *fileSystem);

		File *find_by_path(const LibK::string &path);

	private:
		VirtualFileSystem() = default;
		~VirtualFileSystem() = default;

		FileSystem *initialize_filesystem_on(BlockDevice &device);

		FileSystem *m_root_fs{nullptr};
		vfs_node_t m_root_node{};
	};
}