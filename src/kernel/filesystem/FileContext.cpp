#include <filesystem/FileContext.hpp>

#include <filesystem/File.hpp>

namespace Kernel
{
	FileContext::FileContext(File *file)
		: m_file(file)
	{

	}

	size_t FileContext::seek(size_t offset)
	{
		m_offset = offset;
		return m_offset;
	}

	size_t FileContext::read(size_t count, char *buffer)
	{
		size_t actual_count = m_file->read(m_offset, count, buffer);
		m_offset += actual_count;
		return actual_count;
	}

	size_t FileContext::write(size_t count, char *buffer)
	{
		size_t actual_count = m_file->write(m_offset, count, buffer);
		m_offset += actual_count;
		return actual_count;
	}
}