#pragma once

#include <filesystem/File.hpp>

namespace Kernel
{
	class FileSystem : public File
	{
	public:
		virtual bool mount() = 0;
		virtual bool unmount() = 0;
	};
}