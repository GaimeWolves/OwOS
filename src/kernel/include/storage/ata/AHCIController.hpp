#pragma once

#include <libk/ArrayView.hpp>

#include <pci/pci.hpp>
#include <common_attributes.h>
#include <memory/MMIO.hpp>
#include <interrupts/IRQHandler.hpp>
#include <storage/ata/definitions.hpp>
#include <storage/ata/AHCIPort.hpp>

namespace Kernel
{
	class AHCIController final : public Interrupts::IRQHandler
	{
		typedef volatile struct __hba_capabilities_t
		{
			uint32_t np : 5;       // number of ports
			uint32_t sxs : 1;      // supports external SATA
			uint32_t ems : 1;      // enclosure management supported
			uint32_t cccs : 1;     // command completion coalescing supported
			uint32_t ncs : 5;      // number of command slots
			uint32_t psc : 1;      // partial state capable
			uint32_t ssc : 1;      // slumber state capable
			uint32_t pmd : 1;      // supports PIO multiple DRQ block
			uint32_t fbss : 1;     // supports FIS-based switching
			uint32_t spm : 1;      // supports port multiplier
			uint32_t sam : 1, : 1; // supports AHCIManager mode only
			uint32_t iss : 4;      // interface speed support
			uint32_t sclo : 1;     // supports command list override
			uint32_t sal : 1;      // supports activity LED
			uint32_t salp : 1;     // supports aggressive link power management
			uint32_t sss : 1;      // supports staggered spin-up
			uint32_t smps : 1;     // supports mechanical presence switch
			uint32_t ssntf : 1;    // supports SNotification register
			uint32_t sncq : 1;     // supports native command queueing
			uint32_t s64a : 1;     // supports 64 bit addressing
		} __packed hba_capabilities_t;

		typedef volatile union __global_hba_control_t
		{
			struct
			{
				uint32_t hr : 1;         // HBA reset
				uint32_t ie : 1;         // interrupt enable
				uint32_t mrsm : 1, : 28; // MSI revert to single message
				uint32_t ae : 1;         // AHCIManager enable
			};
			uint32_t dword;
		} __packed global_hba_control_t;

		typedef volatile struct __command_completion_coalescing_control_t
		{
			uint32_t en : 1, : 2; // enable
			uint32_t intr : 5; // interrupt
			uint32_t cc : 8; // command completions
			uint32_t tv : 16; // timeout value
		} __packed command_completion_coalescing_control_t;

		typedef volatile struct __hba_memory_registers_t
		{
			hba_capabilities_t cap;
			global_hba_control_t ghc;
			uint32_t is; // interrupt status register
			uint32_t pi; // ports implemented
			uint32_t vs; // version
			command_completion_coalescing_control_t ccc_ctl;
			uint32_t ccc_ports; // commands completion coalescing ports

			// TODO : split up as structs
			uint32_t em_loc; // enclosure management location
			uint32_t em_ctl; // enclosure management control
			uint32_t cap2; // HBA capabilities extended
			uint32_t bohc; // BIOS/OS handoff control and status

			uint8_t  rsv[0xA0 - 0x2C]; // reserved

			uint8_t  vendor[0x100 - 0xA0]; // vendor specific

			AHCI::hba_port_t ports[32];
		} __packed hba_memory_registers_t;
	public:
		explicit AHCIController(PCI::Function &function);
		void initialize();

		[[nodiscard]] AHCIPort *ports() const { return m_ports; };

	private:
		PCI::Function m_controller{};
		Memory::MMIO<hba_memory_registers_t> m_hba_memory{};
		AHCIPort *m_ports;

		void handle_interrupt(const CPU::interrupt_frame_t &regs) override;
	};
}
