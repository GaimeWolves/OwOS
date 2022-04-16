#pragma once

#include <stddef.h>

#include <storage/ata/definitions.hpp>
#include <storage/ata/AHCICommandSlot.hpp>
#include <memory/MMIO.hpp>
#include <locking/Mutex.hpp>

namespace Kernel
{
	namespace AHCI
	{
		enum class DeviceType
		{
			None,
			SATA,
			SEMB,
			PM,
			SATAPI,
		};
	}

	class AHCIPort
	{
	public:
		AHCIPort() = default;

		void initialize(AHCI::hba_port_t *port, size_t slot_count);

		void identify();

		size_t transfer(AHCI::TransferAction action, uint64_t start_sector, size_t byte_count, uintptr_t buffer);

		void handle_interrupt();

		[[nodiscard]] bool implemented() const { return m_implemented; }
		[[nodiscard]] bool attached() const { return m_device_attached; }
		[[nodiscard]] AHCI::DeviceType type() const { return m_type; }

	private:
		uint32_t find_slot();

		AHCI::ata_identify_block_t *m_identify_block{nullptr};
		Memory::memory_region_t m_identify_block_region{};
		volatile AHCI::hba_port_t *m_hba_port{nullptr};
		bool m_implemented{false};
		bool m_device_attached{false};
		AHCI::DeviceType m_type{AHCI::DeviceType::None};
		AHCICommandSlot *m_command_slots;
		volatile AHCI::command_header_t *m_command_list{nullptr};
		Memory::memory_region_t m_command_list_region{};
		volatile AHCI::received_fis_t *m_received_fis{nullptr};
		Memory::memory_region_t m_received_fis_region{};
		Locking::Mutex m_command_slot_lock{};
	};
}
