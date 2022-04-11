#pragma once

#include <stddef.h>

#include <libk/kvector.hpp>
#include <libk/kstring.hpp>

#include <filesystem/definitions.hpp>

namespace Kernel
{
	class File
	{
		friend class VirtualFileSystem;
	public:
		// Basic file operations
		virtual FileContext open() = 0;
		virtual bool close(FileContext &context) = 0;
		virtual size_t read(size_t offset, size_t bytes, char *buffer) = 0;
		virtual size_t write(size_t offset, size_t bytes, const char *buffer) = 0;
		virtual bool remove() = 0;
		virtual bool rename(const LibK::string &new_file_name) = 0;

		// Directory operations
		virtual LibK::vector<File> read_directory() = 0;
		virtual File *find_file(const LibK::string &file_name) = 0;
		virtual File *make_file(const LibK::string &file_name) = 0;
		virtual bool is_directory() = 0;

		virtual const LibK::string &name() = 0;

	protected:
		size_t m_open_write_contexts{0};
		size_t m_open_read_contexts{0};
	};
}