#pragma once

#include <libk/kvector.hpp>

namespace Kernel
{
	class File;
	class FileContext;
	class FileSystem;
	class Device;
	class VirtualFileSystem;

	typedef struct __vfs_node_t
	{
		File *file;
		LibK::vector<__vfs_node_t *> children; // TODO: Replace with HashMap maybe?
		__vfs_node_t *parent;
	} vfs_node_t;
}