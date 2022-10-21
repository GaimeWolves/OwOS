#pragma once

#include <stddef.h>
#include <stdint.h>

#include <libk/kstring.hpp>

#include <logging/logger.hpp>
#include <devices/CharacterDevice.hpp>

namespace Kernel
{
	// Temporary stdio device; only one should be in use
	class StdioDevice : public CharacterDevice
	{
	public:
		static StdioDevice *get() { return s_device; }

		LibK::StringView name() override { return LibK::StringView(m_name); };

		size_t read(size_t, size_t bytes, Memory::memory_region_t region) override
		{
			memset(region.virt_region().pointer(), 0, bytes);
			return bytes;
		}

		size_t write(size_t, size_t bytes, Memory::memory_region_t region) override
		{
			char *buf = static_cast<char *>(region.virt_region().pointer());

			size_t left = bytes;
			while (left--)
			{
				if (*buf && *buf != '\n')
				{
					m_buf += *buf;
				}
				else
				{
					log("STDIO", m_buf.c_str());
					m_buf.resize(0);
				}
				buf++;
			}

			return bytes;
		}

	protected:
		[[nodiscard]] bool can_open_for_read() const override { return true; };
		[[nodiscard]] bool can_open_for_write() const override { return true; };

	private:
		StdioDevice()
		    : CharacterDevice(0, 0)
		{
		}

		LibK::string m_name{"stdio"};
		LibK::string m_buf{};
		static StdioDevice s_device[1];
	};
} // namespace Kernel::Devices
