#pragma once

#include <libk/kvector.hpp>

#include <devices/BlockDevice.hpp>
#include <storage/ata/definitions.hpp>
#include <storage/PartitionDevice.hpp>

namespace Kernel
{
	class StorageDevice final : public BlockDevice
	{
	public:
		enum class Type
		{
			Unknown,
			AHCI,
		};

		StorageDevice()
		    : BlockDevice(0, 0)
		{
		}

		explicit StorageDevice(AHCIPort *port);
		void initialize();

		size_t read(size_t, size_t, Memory::memory_region_t) override { return 0; };
		size_t write(size_t, size_t, Memory::memory_region_t) override { return 0; };

		size_t read_blocks(size_t offset, size_t count, const Memory::memory_region_t &region) override;
		size_t write_blocks(size_t offset, size_t count, const Memory::memory_region_t &region) override;

		[[nodiscard]] size_t block_size() const override;

		LibK::StringView name() override { return LibK::StringView(m_name); }

		void add_partition(size_t offset, size_t length) { m_partitions.emplace_back(this, offset, length); }

		LibK::vector<PartitionDevice> &partitions() { return m_partitions; }

	protected:
		[[nodiscard]] bool can_open_for_read() const override { return true; };
		[[nodiscard]] bool can_open_for_write() const override { return true; };

	private:
		Type m_type{Type::Unknown};
		AHCIPort *m_ahci_port{nullptr};
		LibK::vector<PartitionDevice> m_partitions{};
		LibK::string m_name{};
	};
}