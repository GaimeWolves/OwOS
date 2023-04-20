#include <storage/ata/AHCIPort.hpp>

#include <arch/Processor.hpp>
#include <time/EventManager.hpp>

#define SATA_SIG_SATA   0x00000101 // sata device
#define SATA_SIG_SATAPI 0xEB140101 // satapi device
#define SATA_SIG_SEMB   0xC33C0101 // enclosure management bridge
#define SATA_SIG_PM     0x96690101 // port multiplier

#define PORT_INT_CPDS (1 << 31) // cold port detect
#define PORT_INT_TFES (1 << 30) // task file error
#define PORT_INT_HBFS (1 << 29) // host bus fatal error
#define PORT_INT_HBDS (1 << 28) // host bus data error
#define PORT_INT_IFS  (1 << 27) // interface fatal error
#define PORT_INT_INFS (1 << 26) // interface non-fatal error
#define PORT_INT_OFS  (1 << 24) // overflow
#define PORT_INT_IPMS (1 << 23) // incorrect port multiplier
#define PORT_INT_PRCS (1 << 22) // PhyRdy change
#define PORT_INT_DMPS (1 << 7)  // device mechanical presence
#define PORT_INT_PCS  (1 << 6)  // port connect change
#define PORT_INT_DPS  (1 << 5)  // descriptor processed
#define PORT_INT_UFS  (1 << 4)  // unknown FIS
#define PORT_INT_SDBS (1 << 3)  // set device bits
#define PORT_INT_DSS  (1 << 2)  // DMA setup FIS
#define PORT_INT_PSS  (1 << 1)  // PIO setup FIS
#define PORT_INT_DHRS 1         // D2H register FIS
#define PORT_INT_ERR (PORT_INT_TFES | PORT_INT_HBFS | PORT_INT_HBDS | PORT_INT_IFS | PORT_INT_OFS | PORT_INT_IPMS)

#define PORT_TFD_DRQ 0x08
#define PORT_TFD_BSY 0x80

namespace Kernel
{
	void AHCIPort::initialize(AHCI::hba_port_t *port, size_t slot_count)
	{
		m_hba_port = port;
		m_implemented = true;

		m_command_list_region = Memory::VirtualMemoryManager::instance().allocate_region(sizeof(AHCI::command_header_t[32]));
		m_received_fis_region = Memory::VirtualMemoryManager::instance().allocate_region(sizeof(AHCI::received_fis_t));

		m_command_list = reinterpret_cast<AHCI::command_header_t *>(m_command_list_region.virt_address);
		m_received_fis = reinterpret_cast<AHCI::received_fis_t *>(m_received_fis_region.virt_address);

		m_hba_port->clb = m_command_list_region.phys_address;
		m_hba_port->clbu = 0;

		m_hba_port->fb = m_received_fis_region.phys_address;
		m_hba_port->fbu = 0;

		memset((void *)m_command_list, 0, sizeof(AHCI::command_header_t[AHCI::NUM_SLOTS]));
		memset((void *)m_received_fis, 0, sizeof(AHCI::received_fis_t));

		if (m_hba_port->cmd.st || m_hba_port->cmd.cr || m_hba_port->cmd.fre || m_hba_port->cmd.fr)
		{
			m_hba_port->cmd.st = 0;
			while (m_hba_port->cmd.cr)
				;

			m_hba_port->cmd.fre = 0;
			while (m_hba_port->cmd.fr)
				;
		}

		m_hba_port->cmd.fre = 1;

		if (m_hba_port->ssts.det != AHCI::DeviceDetection::Present || m_hba_port->ssts.ipm != AHCI::DevicePowerState::Active)
		{
			Time::EventManager::instance().early_sleep(1000);

			if (m_hba_port->ssts.det != AHCI::DeviceDetection::Present || m_hba_port->ssts.ipm != AHCI::DevicePowerState::Active)
			{
				// set to invalid addresses and free memory
				m_hba_port->clb = UINT32_MAX;
				m_hba_port->fb = UINT32_MAX;
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
			// set to invalid addresses and free memory
			m_hba_port->clb = UINT32_MAX;
			m_hba_port->fb = UINT32_MAX;
			Memory::VirtualMemoryManager::instance().free(m_command_list_region);
			Memory::VirtualMemoryManager::instance().free(m_received_fis_region);
			return;
		}

		m_device_attached = true;

		m_hba_port->serr = m_hba_port->serr;
		m_hba_port->is = m_hba_port->is;
		m_hba_port->ie = PORT_INT_TFES | PORT_INT_HBFS | PORT_INT_HBDS | PORT_INT_IFS | PORT_INT_OFS | PORT_INT_IPMS | PORT_INT_DHRS;

		m_command_slots = static_cast<AHCICommandSlot *>(kmalloc(sizeof(AHCICommandSlot[32])));

		for (size_t i = 0; i < AHCI::NUM_SLOTS; i++)
			m_command_slots[i] = AHCICommandSlot(i <= slot_count, &m_command_list[i]);

		m_hba_port->cmd.st = 1;
	}

	void AHCIPort::identify()
	{
		Memory::mapping_config_t config;
		config.caching_mode = Memory::CachingMode::Uncacheable;
		m_identify_block_region = Memory::VirtualMemoryManager::instance().allocate_region(512, config);
		m_identify_block = reinterpret_cast<AHCI::ata_identify_block_t *>(m_identify_block_region.virt_address);
		m_command_slots[0].identify(m_identify_block_region.phys_address);

		while (m_hba_port->tfd & (PORT_TFD_BSY | PORT_TFD_DRQ))
			;

		m_hba_port->ci = 1;

		while (m_hba_port->ci & 1)
			;

		m_hba_port->is = m_hba_port->is; // Clear unwanted interrupts
	}

	size_t AHCIPort::transfer(AHCI::TransferAction action, uint64_t start_sector, size_t byte_count, uintptr_t buffer)
	{
		m_command_slot_lock.lock();
		size_t slot = find_slot();

		if (slot == AHCI::NUM_SLOTS)
			return 0; // TODO: Implement retrying

		// TODO: Check and utilize multiple command slots if necessary
		size_t processed_count = m_command_slots[slot].prepare_transfer(action, start_sector, byte_count, buffer);

		// TODO: suspend thread if possible
		while (m_hba_port->tfd & (PORT_TFD_BSY | PORT_TFD_DRQ))
			;

		m_command_slots[slot].issue();
		m_hba_port->ci = m_hba_port->ci | (1 << slot);

		m_command_slot_lock.unlock();

		auto running_thread = CPU::Processor::current().get_current_thread();

		if (false)
		{
			m_command_slots[slot].attach_thread(running_thread);

			// TODO: This introduces a race when the command finishes before suspension occurs
			if (m_command_slots[slot].issued())
				CoreScheduler::suspend(running_thread);
		}
		else
		{
			while (m_hba_port->ci & (1 << slot))
				;
		}

		if (m_command_slots[slot].failed())
			return 0; // TODO: implement retrying

		return processed_count;
	}

	size_t AHCIPort::find_slot()
	{
		for (size_t i = 0; i < AHCI::NUM_SLOTS; i++)
		{
			if (!(m_hba_port->ci & (1 << i)) && !m_command_slots[i].issued())
				return i;
		}

		return AHCI::NUM_SLOTS;
	}

	void AHCIPort::handle_interrupt()
	{
		uint32_t is = m_hba_port->is;
		m_hba_port->is = is;

		if (is == 0)
			return;

		if (is & PORT_INT_ERR)
		{
			for (size_t i = 0; i < AHCI::NUM_SLOTS; i++)
			{
				if (m_command_slots[i].issued())
				{
					m_command_slots[i].fail();

					if (m_command_slots[i].attached_thread())
						CoreScheduler::resume(m_command_slots[i].attached_thread());
				}
			}

			return;
		}

		for (size_t i = 0; i < AHCI::NUM_SLOTS; i++)
		{
			if (!(m_hba_port->ci & (1 << i)) && m_command_slots[i].issued())
			{
				m_command_slots[i].finish();

				if (m_command_slots[i].attached_thread())
					CoreScheduler::resume(m_command_slots[i].attached_thread());
			}
		}
	}
} // namespace Kernel