#include <storage/ata/AHCIController.hpp>

#include <libk/kcstdio.hpp>

namespace Kernel
{
	AHCIController::AHCIController(PCI::Function &function)
		: Interrupts::IRQHandler(function.get_interrupt_line()) // TODO: Let's just assume it is in PIC mode for now
	{
		m_controller = function;
		LibK::printf_debug_msg("[AHCI] Controller - ABAR: %p Pin: %X Line: %X", m_controller.get_bar5(), m_controller.get_interrupt_pin(), m_controller.get_interrupt_line());

		m_controller.set_command(0b0000001101010110);

		uint16_t cap_offset = m_controller.get_capabilities_ptr();
		uint16_t current_cap = m_controller.read_16(cap_offset);
		while ((current_cap & 0xFF) != 0x05 && (current_cap >> 8))
		{
			cap_offset = (current_cap >> 8) & 0xFC;
			current_cap = m_controller.read_16(cap_offset);
		}

		// Disable MSI for now
		if ((current_cap & 0xFF) == 0x05)
		{
			uint16_t mc = m_controller.read_16(cap_offset + 2);
			m_controller.write_16(cap_offset + 2, mc & 0xFFFE);
		}

		m_hba_memory = Memory::MMIO<hba_memory_registers_t>(m_controller.get_bar5(), sizeof(hba_memory_registers_t));
		LibK::printf_debug_msg("[AHCI] Ports: %lu Implemented: %lX", (uint32_t)m_hba_memory->cap.np + 1, (uint32_t)m_hba_memory->pi);

		m_hba_memory->ghc.dword = 1;
		while (m_hba_memory->ghc.hr)
			;

		m_hba_memory->ghc.ae = 1;

		for (size_t i = 0; i < 32; i++)
		{
			if (m_hba_memory->pi & (1 << i))
			{
				m_ports[i] = AHCIPort(&m_hba_memory->ports[i], m_hba_memory->cap.ncs);
				if (m_ports[i].attached())
					LibK::printf_debug_msg("[AHCI] Port %d attached - Type: %d", i, m_ports[i].type());
			}
		}

		m_ports[0].identify();

		m_hba_memory->is = m_hba_memory->is;
		m_hba_memory->ghc.ie = 1;
		enable_irq();

		// TODO: BIOS/OS Handoff

		Memory::mapping_config_t config;
		config.cacheable = false;
		auto region = Memory::VirtualMemoryManager::instance().allocate_region(1024, config);
		[[maybe_unused]] auto data  = reinterpret_cast<char *>(region.virt_address);

		m_ports[0].read(1, 1024, region.phys_address);
	}

	void AHCIController::handle_interrupt(const CPU::interrupt_frame_t &)
	{
		uint32_t is = m_hba_memory->is;
		m_hba_memory->is = is;

		for (size_t i = 0; i < 32; i++)
		{
			if (m_ports[i].implemented() && is & (1 << i))
				m_ports[i].handle_interrupt();
		}
	}
}