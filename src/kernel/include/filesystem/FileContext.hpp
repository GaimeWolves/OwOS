#pragma once

#include <stddef.h>

#include <filesystem/definitions.hpp>

namespace Kernel
{
	class FileContext
	{
	public:
		FileContext() = default;
		explicit FileContext(File *file);

		// Todo: replace with off_t type
		size_t seek(size_t offset);

		size_t read(size_t count, char *buffer);
		size_t write(size_t count, char *buffer);

		File &file() { return *m_file; }

	private:
		File *m_file{nullptr};
		size_t m_offset{0};
	};
}