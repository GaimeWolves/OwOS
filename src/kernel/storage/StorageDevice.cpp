#include <storage/StorageDevice.hpp>

#include <storage/ata/AHCIPort.hpp>
#include <storage/GPT.hpp>

namespace Kernel
{
	StorageDevice::StorageDevice(AHCIPort *port)
	    : BlockDevice(65, 0) // TODO: Learn about how this should be assigned
	    , m_type(Type::AHCI)
	    , m_ahci_port(port)
	{
	}

	void StorageDevice::initialize()
	{
		GPT::try_parse(*this);
	}

	size_t StorageDevice::read_blocks(size_t block, size_t count, const Memory::memory_region_t &region)
	{
		switch (m_type)
		{
		case Type::AHCI:
			return m_ahci_port->transfer(AHCI::TransferAction::Read, block, count * m_ahci_port->block_size(), region.phys_address);
		default:
			return 0;
		}
	}

	size_t StorageDevice::write_blocks(size_t block, size_t count, const Memory::memory_region_t &region)
	{
		switch (m_type)
		{
		case Type::AHCI:
			return m_ahci_port->transfer(AHCI::TransferAction::Write, block, count * m_ahci_port->block_size(), region.phys_address);
		default:
			return 0;
		}
	}

	size_t StorageDevice::block_size() const
	{
		switch (m_type)
		{
		case Type::AHCI:
			return m_ahci_port->block_size();
		default:
			return 0;
		}
	}
}