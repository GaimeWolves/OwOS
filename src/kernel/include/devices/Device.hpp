#pragma once

#include <filesystem/File.hpp>
#include <filesystem/FileContext.hpp>

namespace Kernel
{
	class Device : public File
	{
	public:
		Device(size_t major, size_t minor)
		    : m_major_number(major), m_minor_number(minor)
		{
		}

		// TODO: actually implement
		FileContext open(int options __unused) override { return FileContext(this, true, true); }
		bool close(FileContext &context __unused) override { return false; }
		bool remove() override { return false; }
		bool rename(const LibK::string &new_file_name __unused) override { return false; }

		LibK::vector<File *> read_directory() override { return {}; }
		File *find_file(const LibK::string &file_name __unused) override { return nullptr; }
		File *make_file(const LibK::string &file_name __unused) override { return nullptr; }
		bool is_directory() override { return false; }

	private:
		size_t m_major_number;
		size_t m_minor_number;
	};
}