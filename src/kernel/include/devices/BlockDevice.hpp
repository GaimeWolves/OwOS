#pragma once

#include <devices/Device.hpp>
#include <memory/definitions.hpp>

namespace Kernel
{
	class BlockDevice : public Device
	{
	public:
		BlockDevice(size_t major, size_t minor)
		    : Device(major, minor)
		{
		}

		virtual size_t read_blocks(size_t block, size_t count, char *buffer) = 0;
		virtual size_t write_blocks(size_t block, size_t count, char *buffer) = 0;

		[[nodiscard]] virtual size_t block_size() const = 0;
	};
}
