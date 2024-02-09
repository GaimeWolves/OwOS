#pragma once

#include <stddef.h>

#include <filesystem/definitions.hpp>
#include <memory/VirtualMemoryManager.hpp>

namespace Kernel
{
	class FileContext
	{
	public:
		FileContext() = default;
		explicit FileContext(File *file, bool readable, bool writeable);

		void close();

		// Todo: replace with off_t type
		size_t seek(size_t offset);

		// TODO: maybe use a ring buffer for reads? Or think of something else
		size_t read(size_t count, char *buffer);
		size_t write(size_t count, char *buffer);

		size_t read_directory(size_t count, char *buffer);

		File &file() { return *m_file; }

		[[nodiscard]] size_t size();

		[[nodiscard]] bool is_readable() const { return m_readable; }
		[[nodiscard]] bool is_writeable() const { return m_writeable; }

		[[nodiscard]] bool is_null() const { return !m_file; }

	private:
		File *m_file{nullptr};
		size_t m_offset{0};

		bool m_readable{false};
		bool m_writeable{false};
	};
}