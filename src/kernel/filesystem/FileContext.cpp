#include <filesystem/FileContext.hpp>

#include <filesystem/File.hpp>
#include <filesystem/VirtualFileSystem.hpp>

#include "../../userland/libc/dirent.h"

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

	size_t FileContext::read_directory(size_t count, char *buffer)
	{
		m_file->lock();

		LibK::vector<File *> children = VirtualFileSystem::instance().get_children(m_file);

		dirent entry{};

		size_t written = 0;

		for (size_t i = m_offset; i < children.size() && count > 0; i++, count--)
		{
			entry.d_ino = children[i]->inode_number();
			strncpy(entry.d_name, children[i]->name().c_str(), NAME_MAX);
			entry.d_name[NAME_MAX] = '\0';
			memcpy(buffer, &entry, sizeof(entry));
			buffer += sizeof(entry);
			written++;
			m_offset++;
		}

		m_file->unlock();

		return written;
	}

	size_t FileContext::size()
	{
		return m_file->size();
	}
}
