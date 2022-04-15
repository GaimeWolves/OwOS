#pragma once

#include <storage/StorageDevice.hpp>

namespace Kernel
{
	class PartitionDevice : public BlockDevice
	{
	private:
		StorageDevice *m_storage_device;
	};
}