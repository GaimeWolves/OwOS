#include <filesystem/FileContext.hpp>

#include <filesystem/File.hpp>

namespace Kernel
{
	FileContext::FileContext(File *file, bool readable, bool writeable)
		: m_file(file)
		, m_readable(readable)
		, m_writeable(writeable)
	{
	}

	void FileContext::close()
	{
		m_file->close(*this);
	}

	size_t FileContext::seek(size_t offset)
	{
		m_offset = offset;
		return m_offset;
	}

	size_t FileContext::read(size_t count, Memory::memory_region_t region)
	{
		size_t actual_count = m_file->read(m_offset, count, region);
		m_offset += actual_count;
		return actual_count;
	}

	size_t FileContext::write(size_t count, Memory::memory_region_t region)
	{
		size_t actual_count = m_file->write(m_offset, count, region);
		m_offset += actual_count;
		return actual_count;
	}

	size_t FileContext::size()
	{
		return m_file->size();
	}
}