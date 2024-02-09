#pragma once

#include <stddef.h>

#include <libk/kstring.hpp>

#include <filesystem/definitions.hpp>
#include <storage/PartitionDevice.hpp>

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

		bool initialize(BlockDevice &root_device);

		FileSystem *mount(BlockDevice &device, File &mount_point);
		bool unmount(FileSystem *fileSystem);

		File *find_by_path(const LibK::string &path_str, const LibK::string &cwd = "");

		LibK::string get_full_path(File *file);
		LibK::vector<File *> get_children(File *file);

	private:
		VirtualFileSystem() = default;
		~VirtualFileSystem() = default;

		// path traversal
		void prepare_path(LibK::string &path, const LibK::string &cwd);
		void normalize_path(LibK::string &path);
		void handle_softlink(vfs_node_t *softlink, LibK::string &path);

		FileSystem *initialize_filesystem_on(BlockDevice &device);

		void read_directory(vfs_node_t *node);

		FileSystem *m_root_fs{nullptr};
		vfs_node_t m_root_node{};
	};
}