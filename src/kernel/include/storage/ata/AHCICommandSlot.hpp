#pragma once

#include <stdint.h>

#include <storage/ata/definitions.hpp>
#include <memory/VirtualMemoryManager.hpp>

namespace Kernel
{
	class AHCICommandSlot
	{
		typedef struct prdt_t
		{
			uint32_t dba;           // data base address
			uint32_t dbau;          // dba high dword
			uint32_t rsv;           // reserved
			uint32_t dbc : 22, : 9; // data byte count
			uint32_t i : 1;         // interrupt on completion
		} __packed prdt_t;

		typedef struct command_table_t
		{
			uint8_t cfis[64];
			uint8_t acmd[16];
			uint8_t rsv[48];
			prdt_t prdt[];
		} __packed command_table_t;

	public:
		AHCICommandSlot() = default;
		explicit AHCICommandSlot(bool supported, AHCI::command_header_t *header);

		[[nodiscard]] bool supported() const { return m_supported; }
		[[nodiscard]] bool issued() const { return m_issued; }

		void identify(uint32_t buffer);

		size_t prepare_read(uint64_t start_sector, size_t byte_count, uintptr_t buffer);

	private:
		bool m_issued{false};
		bool m_supported{false};
		AHCI::command_header_t *m_command_header{nullptr};
		Memory::memory_region_t m_command_table_region{};
		command_table_t *m_command_table{nullptr};
	};
}
