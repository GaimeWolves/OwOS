#include <storage/ata/AHCIController.hpp>

#include <logging/logger.hpp>

namespace Kernel
{
	AHCIController::AHCIController(PCI::Function &function)
	    : Interrupts::IRQHandler(function.get_interrupt_line()) // TODO: Let's just assume it is in PIC mode for now
	{
		m_controller = function;
	}

	void AHCIController::initialize()
	{
		log("AHCI", "Controller - ABAR: %p Pin: %c Line: 0x%x", m_controller.get_bar5(), 'A' + m_controller.get_interrupt_pin(), m_controller.get_interrupt_line());

		m_controller.set_command(PCI_CMD_MSE | PCI_CMD_BME | PCI_CMD_MWIE | PCI_CMD_PEE | PCI_CMD_SEE | PCI_CMD_FBE);

		uint16_t cap_offset = m_controller.get_capabilities_ptr();
		uint16_t current_cap = m_controller.read_16(cap_offset);
		while (PCI_CAP_ID(current_cap) != PCI_MSI_CAP_ID && PCI_CAP_NEXT(current_cap))
		{
			cap_offset = PCI_CAP_NEXT(current_cap);
			current_cap = m_controller.read_16(cap_offset);
		}

		// Disable MSI for now
		if (PCI_CAP_ID(current_cap) == PCI_MSI_CAP_ID)
		{
			uint16_t mc = m_controller.read_16(cap_offset + PCI_MSI_CONTROL);
			m_controller.write_16(cap_offset + PCI_MSI_CONTROL, mc & ~PCI_MSI_ENABLE);
		}

		m_hba_memory = Memory::MMIO<hba_memory_registers_t>(m_controller.get_bar5(), sizeof(hba_memory_registers_t));
		log("AHCI", "Ports: %lu Implemented: 0x%lx", (uint32_t)m_hba_memory->cap.np + 1, (uint32_t)m_hba_memory->pi);

		m_hba_memory->ghc.dword = 1;
		while (m_hba_memory->ghc.hr)
			;

		m_hba_memory->ghc.ae = 1;

		m_ports = new AHCIPort[AHCI::NUM_PORTS];

		for (size_t i = 0; i < AHCI::NUM_PORTS; i++)
		{
			if (m_hba_memory->pi & (1 << i))
			{
				m_ports[i].initialize(&m_hba_memory->ports[i], m_hba_memory->cap.ncs);
				if (m_ports[i].attached())
					log("AHCI", "Port %d attached - Type: %d", i, m_ports[i].type());
			}
		}

		m_ports[0].identify();

		m_hba_memory->is = m_hba_memory->is;
		m_hba_memory->ghc.ie = 1;
		enable_irq();

		// TODO: BIOS/OS Handoff
	}

	void AHCIController::handle_interrupt(const CPU::interrupt_frame_t &)
	{
		uint32_t is = m_hba_memory->is;
		m_hba_memory->is = is;

		for (size_t i = 0; i < AHCI::NUM_PORTS; i++)
		{
			if (m_ports[i].implemented() && is & (1 << i))
				m_ports[i].handle_interrupt();
		}
	}
} // namespace Kernel