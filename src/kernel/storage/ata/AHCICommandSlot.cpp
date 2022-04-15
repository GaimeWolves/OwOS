#include <storage/ata/AHCICommandSlot.hpp>

#include <memory/VirtualMemoryManager.hpp>

#define NUM_PRTDS 8

namespace Kernel
{
	AHCICommandSlot::AHCICommandSlot(bool supported, AHCI::command_header_t *header)
		: m_supported(supported)
	    , m_command_header(header)
	{
		Memory::mapping_config_t config;
		config.cacheable = false;
		m_command_table_region = Memory::VirtualMemoryManager::instance().allocate_region(sizeof (command_table_t) + NUM_PRTDS * sizeof (prdt_t));
		m_command_table = reinterpret_cast<command_table_t *>(m_command_table_region.virt_address);
		memset(m_command_table, 0, sizeof (command_table_t) + NUM_PRTDS * sizeof (prdt_t));

		m_command_header->ctba = m_command_table_region.phys_address;
		m_command_header->cbtau = 0;
		m_command_header->a = 0;
		m_command_header->w = 0;
		m_command_header->p = 1;
		m_command_header->r = 0;
		m_command_header->b = 0;
		m_command_header->c = 0;
		m_command_header->pmp = 0;
		m_command_header->prdtl = NUM_PRTDS;
	}

	// TODO: implement using multiple buffers, a dynamic number of PRDTs, bounds checking
	size_t AHCICommandSlot::prepare_read(uint64_t start_sector, size_t byte_count, uintptr_t buffer)
	{
		assert(!(byte_count & 1)); // byte count must be word aligned
		size_t prtd_count = LibK::round_up_to_multiple<size_t>(byte_count, 4 * 1024 * 1024) / (4 * 1024 * 1024); // each PRDT can handle at max 4MiB
		prtd_count = LibK::min(prtd_count, NUM_PRTDS); // no dynamic number of PRDTs for now
		size_t actual_byte_count = LibK::min(prtd_count * (4 * 1024 * 1024), byte_count);
		auto sector_count = LibK::round_up_to_multiple<size_t>(actual_byte_count, 512) / 512;

		memset(m_command_table, 0, sizeof (command_table_t) + prtd_count * sizeof (prdt_t));
		auto cfis = reinterpret_cast<volatile AHCI::h2d_register_fis_t *>(m_command_table->cfis);
		cfis->type = AHCI::FisType::RegisterH2D;
		cfis->command = 0x25; // READ DMA EXT
		cfis->device = 0;
		cfis->c = 1;
		cfis->lba0 = start_sector & 0xFF;
		cfis->lba1 = (start_sector >> 8) & 0xFF;
		cfis->lba2 = (start_sector >> 16) & 0xFF;
		cfis->device = 1 << 6; // LBA mode
		cfis->lba3 = (start_sector >> 24) & 0xFF;
		cfis->lba4 = (start_sector >> 32) & 0xFF;
		cfis->lba5 = (start_sector >> 40) & 0xFF;
		cfis->count = sector_count;

		m_command_header->prdtl = prtd_count;
		m_command_header->prdbc = actual_byte_count;
		m_command_header->cfl = sizeof (AHCI::h2d_register_fis_t) / sizeof (uint32_t);

		size_t ret = actual_byte_count;

		for (size_t i = 0; i < prtd_count - 1; i++)
		{
			m_command_table->prdt[i].dba = buffer;
			m_command_table->prdt[i].dbau = 0;
			m_command_table->prdt[i].dbc = 4 * 1024 * 1024 - 1;
			buffer += 4 * 1024 * 1024;
			actual_byte_count -= 4 * 1024 * 1024;
		}

		m_command_table->prdt[prtd_count - 1].dba = buffer;
		m_command_table->prdt[prtd_count - 1].dbau = 0;
		m_command_table->prdt[prtd_count - 1].dbc = actual_byte_count - 1;

		m_issued = true;

		return ret;
	}

	void AHCICommandSlot::identify(uint32_t buffer)
	{
		memset(m_command_table, 0, sizeof (command_table_t) + sizeof (prdt_t));
		auto cfis = reinterpret_cast<volatile AHCI::h2d_register_fis_t *>(m_command_table->cfis);
		cfis->type = AHCI::FisType::RegisterH2D;
		cfis->command = 0xEC; // ATA IDENTIFY
		cfis->device = 0;
		cfis->c = 1;
		m_command_header->prdtl = 1;
		m_command_header->prdbc = 512;
		m_command_header->cfl = sizeof (AHCI::h2d_register_fis_t) / sizeof (uint32_t);
		m_command_table->prdt[0].dba = buffer;
		m_command_table->prdt[0].dbau = 0;
		m_command_table->prdt[0].dbc = 512 - 1;
	}
}