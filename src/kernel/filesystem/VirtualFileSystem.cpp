#include <filesystem/VirtualFileSystem.hpp>

#include <filesystem/Ext2FileSystem.hpp>

namespace Kernel
{
	bool VirtualFileSystem::initialize(BlockDevice &root_device)
	{
		m_root_fs = initialize_filesystem_on(root_device);
		m_root_fs->mount();

		m_root_node.file = m_root_fs;

		return true;
	}

	FileSystem *VirtualFileSystem::mount(BlockDevice &, File &)
	{
		return nullptr;
	}

	bool VirtualFileSystem::unmount(FileSystem *)
	{
		return false;
	}

	File *VirtualFileSystem::find_by_path(const LibK::string &path)
	{
		auto current_offset = path.c_str();

		// TODO: Relative paths using process CWD
		if (*current_offset++ != '/') {
			return nullptr;
		}

		vfs_node_t *current_file = &m_root_node;

		while(*current_offset)
		{
			if (strcmp(current_offset, ".") == 0)
			{
				current_offset++;

				if (*current_offset == '/')
					current_offset++;
				else
					return current_file->file;

				continue;
			}

			if (strcmp(current_offset, "..") == 0)
			{
				if (current_file->parent)
					current_file = current_file->parent;

				current_offset += 2;

				if (*current_offset == '/')
					current_offset++;
				else
					return current_file->file;

				continue;
			}

			if (current_file->file->is_directory() && current_file->children.empty())
			{
				auto directory = current_file->file->read_directory();
				current_file->children = LibK::vector<vfs_node_t *>();
				current_file->children.ensure_capacity(directory.size());
				for (auto *file : directory)
				{
					auto node = new vfs_node_t;
					node->file = file;
					node->parent = current_file;
					current_file->children.push_back(node);
				}
			}

			auto next_delim = strchr(current_offset, '/');

			if (next_delim)
			{
				size_t len = (uintptr_t) next_delim - (uintptr_t) current_offset;

				for (auto node : current_file->children)
				{
					if (strncmp(node->file->name().c_str(), current_offset, len) == 0)
					{
						current_file = node;
						break;
					}
				}

				current_offset += len + 1;
			}
			else
			{
				for (auto node : current_file->children)
				{
					if (strcmp(node->file->name().c_str(), current_offset) == 0)
						return node->file;
				}

				return nullptr;
			}
		}

		return nullptr;
	}

	FileSystem *VirtualFileSystem::initialize_filesystem_on(BlockDevice &device)
	{
		// TODO: This should do some enumeration on the device, to check if it has a filesystem
		return new Ext2FileSystem(device);
	}
}