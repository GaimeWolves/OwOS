#pragma once

#include <stddef.h>

#include <storage/definitions.hpp>
#include <devices/BlockDevice.hpp>

namespace Kernel
{
	class PartitionDevice final : public BlockDevice
	{
	public:
		PartitionDevice()
		    : BlockDevice(0, 0)
		{
		}

		PartitionDevice(StorageDevice *device, size_t offset, size_t length);

		size_t read(size_t, size_t, char *) override { return 0; };
		size_t write(size_t, size_t, char *) override { return 0; };

		size_t read_blocks(size_t block, size_t count, char *buffer) override;
		size_t write_blocks(size_t block, size_t count, char *buffer) override;

		[[nodiscard]] size_t size() override { return m_length; }

		    size_t block_size() const override;

		LibK::StringView name() override { return LibK::StringView(m_name); }

	protected:
		[[nodiscard]] bool can_open_for_read() const override { return true; };
		[[nodiscard]] bool can_open_for_write() const override { return true; };

	private:
		size_t m_offset{};
		size_t m_length{};
		StorageDevice *m_storage_device{nullptr};
		LibK::string m_name{};
	};
}
