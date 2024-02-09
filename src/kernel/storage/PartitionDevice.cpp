#include <storage/PartitionDevice.hpp>

#include <storage/StorageDevice.hpp>

namespace Kernel
{
	PartitionDevice::PartitionDevice(StorageDevice *device, size_t offset, size_t length)
		: BlockDevice(65, 0)
	    , m_offset(offset)
		, m_length(length)
		, m_storage_device(device)
	{
	}

	size_t PartitionDevice::read_blocks(size_t block, size_t count, char *buffer)
	{
		if (block + count >= m_offset + m_length)
			return 0;

		return m_storage_device->read_blocks(m_offset + block, count, buffer);
	}

	size_t PartitionDevice::write_blocks(size_t block, size_t count, char *buffer)
	{
		if (block + count >= m_offset + m_length)
			return 0;

		return m_storage_device->write_blocks(m_offset + block, count, buffer);
	}

	size_t PartitionDevice::block_size() const { return m_storage_device->block_size(); }
}