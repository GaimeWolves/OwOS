#include <storage/StorageDevice.hpp>

#include <storage/ata/AHCIPort.hpp>
#include <storage/GPT.hpp>
#include <arch/memory.hpp>

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

	size_t StorageDevice::read_blocks(size_t block, size_t count, char *buffer)
	{
		return transfer(TransferType::Read, block, count, buffer);
	}

	size_t StorageDevice::write_blocks(size_t block, size_t count, char *buffer)
	{
		return transfer(TransferType::Write, block, count, buffer);
	}

	size_t StorageDevice::transfer(TransferType type, size_t block, size_t count, char *buffer)
	{
		size_t transferred = 0;

		auto pages = LibK::ceil_div<size_t>(count, PAGE_SIZE);
		for (size_t i = 0; i < pages; i++)
		{
			uintptr_t physical = Memory::Arch::as_physical((uintptr_t)buffer + i * PAGE_SIZE);
			size_t count_for_transfer;
			size_t actually_transferred;
			AHCI::TransferAction action;

			switch (m_type)
			{
			case Type::AHCI:
				count_for_transfer = LibK::min(count * m_ahci_port->block_size(), PAGE_SIZE);
				count -= count_for_transfer;
				action = type == TransferType::Read ? AHCI::TransferAction::Read : AHCI::TransferAction::Write;
				actually_transferred = m_ahci_port->transfer(action, block + i * PAGE_SIZE / m_ahci_port->block_size(), count_for_transfer, physical);

				transferred += actually_transferred;

				if (actually_transferred < count_for_transfer)
					return transferred;

				break;
			default:
				return 0;
			}
		}

		return transferred;
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
