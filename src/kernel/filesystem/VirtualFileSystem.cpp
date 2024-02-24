#include <filesystem/VirtualFileSystem.hpp>

#include <filesystem/Ext2FileSystem.hpp>
#include <arch/Processor.hpp>
#include <memory/VirtualMemoryManager.hpp>

namespace Kernel
{
	bool VirtualFileSystem::initialize(BlockDevice &root_device)
	{
		m_root_fs = initialize_filesystem_on(root_device);
		m_root_fs->mount();

		m_root_node.file = m_root_fs;
		m_root_node.parent = nullptr;
		m_root_fs->set_vfs_node(&m_root_node);

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

	File *VirtualFileSystem::find_by_path(const LibK::string &path_str, const LibK::string &cwd)
	{
		LibK::string path = LibK::string(path_str.c_str());

		prepare_path(path, cwd);
		normalize_path(path);

		vfs_node_t *current_file = &m_root_node;

		while(!path.empty())
		{
			if (path[0] == '/')
			{
				if (!current_file->file->is_type(FileType::Directory))
					return nullptr;

				path.erase(path.begin());
				continue;
			}

			if (strncmp(path.c_str(), ".", 1) == 0 && (path.c_str()[1] == '\0' || path.c_str()[1] == '/'))
			{
				path.erase(path.begin());
				continue;
			}

			if (strncmp(path.c_str(), "..", 2) == 0 && (path.c_str()[2] == '\0' || path.c_str()[2] == '/'))
			{
				path.erase(path.begin(), path.begin() + 2);

				if (current_file != &m_root_node)
					current_file = current_file->parent;

				continue;
			}

			// current path entry has to be searched in directory

			assert(current_file->file->is_type(FileType::Directory));

			if (current_file->children.empty())
				read_directory(current_file);

			bool found = false;

			for (auto node : current_file->children)
			{
				const char *entry_name = node->file->name().c_str();
				size_t name_size = node->file->name().size();
				if (strncmp(path.c_str(), entry_name, name_size) == 0 && (path.size() == name_size || path[name_size] == '/'))
				{
					if (node->file->is_type(FileType::SoftLink))
					{
						// TODO: this is very wasteful with memory
						// TODO: employ maximum recursion depth with ELOOP
						handle_softlink(node, path);
						LibK::string new_cwd = get_full_path(node->parent->file);
						new_cwd += '/';
						return find_by_path(path, new_cwd);
					}

					current_file = node;
					path.erase(path.begin(), path.begin() + (int)name_size);

					found = true;

					break;
				}
			}

			if (!found)
				return nullptr;
		}

		return current_file->file;
	}

	LibK::string VirtualFileSystem::get_full_path(File *file)
	{
		if (!file)
			return "";

		LibK::string path;
		LibK::stack<vfs_node_t *> path_entry_stack;

		vfs_node_t *current = file->m_vfs_node;
		do
		{
			path_entry_stack.push(current);
			current = current->parent;
		} while (current);

		do
		{
			if (path_entry_stack.top() != &m_root_node)
			{
				path += '/';
				path += path_entry_stack.top()->file->name().c_str();
			}

			path_entry_stack.pop();
		} while (!path_entry_stack.empty());

		if (path.empty())
			path += '/';

		return path;
	}

	LibK::vector<File *> VirtualFileSystem::get_children(File *file)
	{
		if (!file->is_type(FileType::Directory))
			return {};

		vfs_node_t *node = file->m_vfs_node;

		if (node->children.empty())
			read_directory(node);

		LibK::vector<File *> children;
		for (auto &child : node->children) {
			children.push_back(child->file);
		}

		return children;
	}

	void VirtualFileSystem::read_directory(vfs_node_t *node)
	{
		// TODO: think about changes in directory
		assert(node->children.empty());

		auto directory = node->file->read_directory();
		node->children = LibK::vector<vfs_node_t *>();
		node->children.reserve(directory.size());
		for (auto *file : directory)
		{
			auto child = new vfs_node_t;
			child->file = file;
			child->parent = node;
			file->set_vfs_node(child);
			node->children.push_back(child);
		}
	}

	void VirtualFileSystem::prepare_path(LibK::string &path, const LibK::string &cwd)
	{
		if (path.empty())
		{
			path += '/';
			return;
		}

		if (path[0] != '/')
		{
			path.insert(0, cwd);
		}
	}

	void VirtualFileSystem::normalize_path(LibK::string &path)
	{
		assert(!path.empty() && path[0] == '/');

		size_t index = 0;

		while (index < path.size())
		{
			while (index < path.size() && path[index] != '/')
				index++;

			if (index == path.size())
				break;

			assert(path[index] == '/');

			size_t last_slash = index;

			while (last_slash + 1 < path.size() && path[last_slash + 1] == '/')
				last_slash++;

			path.erase(path.begin() + (int)index, path.begin() + (int)last_slash);
			index++;
		}
	}

	void VirtualFileSystem::handle_softlink(vfs_node_t *softlink, LibK::string &path)
	{
		assert(softlink->file->is_type(FileType::SoftLink));

		char *prefix = static_cast<char *>(kmalloc(softlink->file->size()));
		softlink->file->read(0, softlink->file->size(), prefix);

		path.erase(path.begin(), path.begin() + (int)softlink->file->name().size());
		path.insert(0, prefix);

		kfree(prefix);
	}

	FileSystem *VirtualFileSystem::initialize_filesystem_on(BlockDevice &device)
	{
		// TODO: This should do some enumeration on the device, to check if it has a filesystem
		return new Ext2FileSystem(device);
	}
}
