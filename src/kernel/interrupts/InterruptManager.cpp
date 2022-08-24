#include <interrupts/InterruptManager.hpp>

#include <libk/kcstdio.hpp>

#include "logging/logger.hpp"
#include <firmware/acpi/Parser.hpp>
#include <interrupts/IOAPIC.hpp>
#include <interrupts/LAPIC.hpp>
#include <interrupts/PIC.hpp>
#include <interrupts/definitions.hpp>

namespace Kernel::Interrupts
{
	void InterruptManager::initialize()
	{
		m_controllers.push_back(new PIC());
		parse_madt();
	}

	void InterruptManager::parse_madt()
	{
		MADT madt = MADT(ACPI::Parser::instance().find_table_by_signature("APIC"));

		// Assume no APIC is available
		// TODO: We could also check the MSR register or check for the MP table here
		if (!madt.is_valid())
		{
			kprintf("[ INT ]: No valid MADT found, falling back to legacy 8259 PIC");
			m_active_controllers.push_back(m_controllers[0]);
			return;
		}

		log("APIC", "LAPIC Addr: %p", madt.get_table()->lapic_addr);
		log("APIC", "Dual Legacy PIC: %d", madt.get_table()->dual_pics);

		LAPIC::instance().initialize(madt.get_table()->lapic_addr);

		LibK::vector<madt_ioapic_override_entry_t> ioapic_redirections;
		LibK::vector<madt_ioapic_nmi_entry_t> ioapic_nmis;

		uint32_t ap_count = 0;

		madt_entry_t *current_entry = madt.get_table()->entries;

		while ((uintptr_t)current_entry - (uintptr_t)madt.get_table() < madt.get_table()->header.length)
		{
			switch (current_entry->type)
			{
			case 0:
			{
				madt_lapic_entry_t *entry = (madt_lapic_entry_t *)current_entry;
				log(
				    "APIC",
				    "LAPIC: ProcID: %d ApicID: %d Enable: %d Capable: %d",
				    entry->processor_id,
				    entry->apic_id,
				    entry->enabled,
				    entry->capable);

				if (entry->apic_id != 0)
					ap_count++;
				break;
			}
			case 1:
			{
				madt_ioapic_entry_t *entry = (madt_ioapic_entry_t *)current_entry;
				log(
				    "APIC",
				    "IOAPIC: IOApicID: %d IOApicAddr: %p GSIBase: 0x%.2X",
				    entry->apic_id,
				    entry->apic_addr,
				    entry->gsi_base);

				auto ioapic = new IOAPIC(entry->apic_addr, entry->gsi_base);
				ioapic->enable();
				m_controllers.push_back(ioapic);
				m_active_controllers.push_back(ioapic);
				break;
			}
			case 2:
			{
				madt_ioapic_override_entry_t *entry = (madt_ioapic_override_entry_t *)current_entry;
				log(
				    "APIC",
				    "IOAPIC Override: Bus: %d IRQ: 0x%.2X GSI: 0x%.2X LT: %d AL: %d",
				    entry->bus_src,
				    entry->irq_src,
				    entry->gsi,
				    entry->level_triggered,
				    entry->active_low);

				ioapic_redirections.push_back(*entry);
				break;
			}
			case 3:
			{
				madt_ioapic_nmi_entry_t *entry = (madt_ioapic_nmi_entry_t *)current_entry;
				log(
				    "APIC",
				    "IOAPIC NMI: Src: 0x%.2X GSI: 0x%.2X LTrig: %d ActLow: %d",
				    entry->nmi_src,
				    entry->gsi,
				    entry->level_triggered,
				    entry->active_low);

				ioapic_nmis.push_back(*entry);
				break;
			}
			case 4:
			{
				madt_lapic_nmi_entry_t *entry = (madt_lapic_nmi_entry_t *)current_entry;
				log(
				    "APIC",
				    "LAPIC NMI: ProcId: 0x%.2X LINT%d LTrig: %d ActLow: %d",
				    entry->processor_id,
				    entry->lint_number,
				    entry->level_triggered,
				    entry->active_low);
				break;
			}
			case 5:
			{
				madt_lapic_override_entry_t *entry = (madt_lapic_override_entry_t *)current_entry;
				log(
				    "APIC",
				    "LAPIC Address Override: 0x%.64llX",
				    entry->address_override);
				break;
			}
			case 9:
			{
				madt_x2apic_entry_t *entry = (madt_x2apic_entry_t *)current_entry;
				log(
				    "APIC",
				    "x2APIC: ProcID: %d ApicID: %d Enable: %d Capable: %d",
				    entry->processor_id,
				    entry->apic_id,
				    entry->enabled,
				    entry->capable);
				break;
			}
			default:
				log("APIC","Unknown entry");
				break;
			}

			current_entry = (madt_entry_t *)((char *)current_entry + current_entry->length);
		}

		madt.free();

		if (m_active_controllers.empty())
		{
			log("APIC","No IOAPICs found, falling back to legacy 8259 PIC");
			m_active_controllers.push_back(m_controllers[0]);
			return;
		}

		for (auto redirection : ioapic_redirections)
		{
			for (auto ioapic : m_active_controllers)
			{
				if (redirection.gsi >= ioapic->get_gsi_base() && redirection.gsi < ioapic->get_gsi_base() + ioapic->get_gsi_count())
				{
					((IOAPIC *)ioapic)->add_redirection(redirection);
					break;
				}
			}
		}

		for (auto nmi : ioapic_nmis)
		{
			for (auto ioapic : m_active_controllers)
			{
				if (nmi.gsi >= ioapic->get_gsi_base() && nmi.gsi < ioapic->get_gsi_base() + ioapic->get_gsi_count())
				{
					((IOAPIC *)ioapic)->set_nmi(nmi);
					break;
				}
			}
		}

		LAPIC::instance().enable();
		LAPIC::instance().set_available_ap_count(ap_count);
	}

	InterruptController *InterruptManager::get_responsible_controller(IRQHandler &handler) const
	{
		uint32_t int_num = handler.original_interrupt_number();
		for (auto &controller : m_active_controllers)
		{
			if (int_num >= controller->get_gsi_base() && int_num < controller->get_gsi_base() + controller->get_gsi_count())
				return controller;
		}

		return nullptr;
	}

	uint32_t InterruptManager::get_mapped_interrupt_number(uint32_t interrupt_number)
	{
		// TODO: Implement CPU balancing
		return interrupt_number + CPU::FIRST_USABLE_INTERRUPT;
	}
}