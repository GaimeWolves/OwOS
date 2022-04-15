#pragma once

#include <devices/Device.hpp>

namespace Kernel
{
	class BlockDevice : public Device
	{
	public:
		virtual size_t read_blocks(size_t offset, size_t count, char *buffer) = 0;
		virtual size_t write_blocks(size_t offset, size_t count, const char *buffer) = 0;

		[[nodiscard]] virtual size_t block_size() const = 0;
	};
}