#include <filesystem/File.hpp>

#include <filesystem/FileContext.hpp>

namespace Kernel
{
	FileContext File::open(int options)
	{
		// TODO: parse options correctly
		bool wants_read = options & (O_RDONLY | O_RDWR);
		bool wants_write = options & (O_WRONLY | O_RDWR | O_APPEND);

		if ((wants_read & !can_open_for_read()) || (wants_write && !can_open_for_write()))
			return {};

		if (wants_read)
			m_open_read_contexts++;

		if (wants_write)
			m_open_write_contexts++;

		return FileContext(this, wants_read, wants_write);
	}

	bool File::close(FileContext &context)
	{
		if (context.is_readable())
			m_open_read_contexts--;

		if (context.is_writeable())
			m_open_write_contexts--;

		return true;
	}
}
