#include <storage/ata/AHCIPort.hpp>

#include <time/EventManager.hpp>

#define	SATA_SIG_SATA   0x00000101  // sata device
#define	SATA_SIG_SATAPI 0xEB140101  // satapi device
#define	SATA_SIG_SEMB   0xC33C0101  // enclosure management bridge
#define	SATA_SIG_PM     0x96690101  // port multiplier

namespace Kernel
{
	AHCIPort::AHCIPort(AHCI::hba_port_t *port, size_t slot_count)
		: m_hba_port(port)
		, m_implemented(true) // This constructor should only be executed on implemented ports
	{
		m_command_list_region = Memory::VirtualMemoryManager::instance().allocate_region(sizeof (AHCI::command_header_t[32]));
		m_received_fis_region = Memory::VirtualMemoryManager::instance().allocate_region(sizeof (AHCI::received_fis_t));

		m_command_list = reinterpret_cast<AHCI::command_header_t *>(m_command_list_region.virt_address);
		m_received_fis = reinterpret_cast<AHCI::received_fis_t *>(m_received_fis_region.virt_address);

		m_hba_port->clb = m_command_list_region.phys_address;
		m_hba_port->clbu = 0;

		m_hba_port->fb = m_received_fis_region.phys_address;
		m_hba_port->fbu = 0;

		memset((void *)m_command_list, 0, sizeof (AHCI::command_header_t[32]));
		memset((void *)m_received_fis, 0, sizeof (AHCI::received_fis_t));

		if (m_hba_port->cmd.st || m_hba_port->cmd.cr || m_hba_port->cmd.fre || m_hba_port->cmd.fr)
		{
			m_hba_port->cmd.st = 0;
			while (m_hba_port->cmd.cr)
				;

			m_hba_port->cmd.fre = 0;
			while (m_hba_port->cmd.fr)
				;
		}

		m_hba_port->cmd.fre	= 1;

		if (m_hba_port->ssts.det != AHCI::DeviceDetection::Present || m_hba_port->ssts.ipm != AHCI::DevicePowerState::Active)
		{
			Time::EventManager::instance().early_sleep(1000);

			if (m_hba_port->ssts.det != AHCI::DeviceDetection::Present || m_hba_port->ssts.ipm != AHCI::DevicePowerState::Active)
			{
				m_hba_port->clb = 0xFFFFFFFF;
				m_hba_port->fb = 0xFFFFFFFF;
				Memory::VirtualMemoryManager::instance().free(m_command_list_region);
				Memory::VirtualMemoryManager::instance().free(m_received_fis_region);
				return;
			}
		}

		switch (m_hba_port->sig)
		{
		case SATA_SIG_SATA:
			m_type = AHCI::DeviceType::SATA;
			break;
		case SATA_SIG_SATAPI:
			m_type = AHCI::DeviceType::SATAPI;
			break;
		case SATA_SIG_SEMB:
			m_type = AHCI::DeviceType::SEMB;
			break;
		case SATA_SIG_PM:
			m_type = AHCI::DeviceType::PM;
			break;
		default:
			break;
		}

		if (m_type == AHCI::DeviceType::None)
		{
			m_hba_port->clb = 0xFFFFFFFF;
			m_hba_port->fb = 0xFFFFFFFF;
			Memory::VirtualMemoryManager::instance().free(m_command_list_region);
			Memory::VirtualMemoryManager::instance().free(m_received_fis_region);
			return;
		}

		m_device_attached = true;

		m_hba_port->serr = m_hba_port->serr;
		m_hba_port->is = m_hba_port->is;
		m_hba_port->ie = 0x7C000001;

		m_command_slots = static_cast<AHCICommandSlot *>(kmalloc(sizeof(AHCICommandSlot[32])));

		for (size_t i = 0; i < 32; i++)
			m_command_slots[i] = AHCICommandSlot(i <= slot_count, &m_command_list[i]);

		m_hba_port->cmd.st = 1;
	}

	void AHCIPort::identify()
	{
		Memory::mapping_config_t config;
		config.cacheable = false;
		m_identify_block_region = Memory::VirtualMemoryManager::instance().allocate_region(512, config);
		m_identify_block = reinterpret_cast<AHCI::ata_identify_block_t *>(m_identify_block_region.virt_address);
		m_command_slots[0].identify(m_identify_block_region.phys_address);

		while(m_hba_port->tfd & 0x88)
			;

		m_hba_port->ci = 1;

		while (m_hba_port->ci & 1)
			;

		m_hba_port->is = m_hba_port->is; // Clear unwanted interrupts
	}

	size_t AHCIPort::read(uint64_t start_sector, size_t byte_count, uintptr_t buffer)
	{
		// TODO: Check and utilize multiple command slots if necessary
		m_command_slots[0].prepare_read(start_sector, byte_count, buffer);

		// TODO: suspend thread if possible
		while(m_hba_port->tfd & 0x88)
			;

		m_hba_port->ci = 1;

		// TODO: suspend thread if possible
		while (m_hba_port->ci & 1)
			;

		return byte_count;
	}

	void AHCIPort::handle_interrupt()
	{
		uint32_t is = m_hba_port->is;
		m_hba_port->is = is;

		if (is == 0)
			return;
	}
}