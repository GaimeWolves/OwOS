#include <interrupts/IOAPIC.hpp>

#include <libk/kmath.hpp>

#include <memory/VirtualMemoryManager.hpp>
#include <interrupts/LAPIC.hpp>

// TODO: Consolidate with identical LAPIC constants
#define APIC_DELIV_FIXED    0b000
#define APIC_DELIV_LOWEST   0b001
#define APIC_DELIV_SMI      0b010
#define APIC_DELIV_NMI      0b100
#define APIC_DELIV_INIT     0b101
#define APIC_DELIV_ExtINT   0b111

#define APIC_DEST_PHYSICAL 0
#define APIC_DEST_LOGICAL  1

#define APIC_ACTIVE_LOW  1
#define APIC_ACTIVE_HIGH 0

#define APIC_TRIGGER_EDGE  0
#define APIC_TRIGGER_LEVEL 1

#define REGISTER_IOAPICVER 1
#define REGISTER_IOREDTLB_LOW(n) (0x10 + (n) * 2)
#define REGISTER_IOREDTLB_HIGH(n) (0x10 + (n) * 2 + 1)

namespace Kernel::Interrupts
{
	typedef union redirection_entry_t
	{
		struct
		{
			uint64_t vector_num : 8;
			uint64_t delivery_mode : 3;
			uint64_t destination_mode : 1;
			uint64_t delivery_status : 1;
			uint64_t pin_polarity : 1;
			uint64_t remote_irr : 1;
			uint64_t trigger_mode : 1;
			uint64_t mask : 1;
			uint64_t reserved : 39;
			uint64_t destination : 8;
		} __packed;
		struct
		{
			uint32_t low_dword;
			uint32_t high_dword;
		} __packed;
	} redirection_entry_t;

	IOAPIC::IOAPIC(uintptr_t address, uint32_t gsi_base)
		: m_gsi_base(gsi_base)
	{
		m_registers = Memory::MMIO<uint32_t>(address, 0x20);
		uint32_t reg = read_register(REGISTER_IOAPICVER);
		m_gsi_count = ((reg >> 16) & 0xFF) + 1;
		m_interrupt_mask = new uint32_t[LibK::round_up_to_multiple<uint32_t>(m_gsi_count, 32) / 32];

		for (size_t i = 0; i < LibK::round_up_to_multiple<uint32_t>(m_gsi_count, 32) / 32; i++)
			m_interrupt_mask[i] = 0xFFFFFFFF;

		assert(m_interrupt_mask);

		for (size_t i = 0; i < m_gsi_count; i++)
		{
			// TODO: Read spec about logical addressing
			// TODO: Implement interrupt balancing
			redirection_entry_t entry {
				.vector_num = 0,
				.delivery_mode = APIC_DELIV_FIXED,
				.destination_mode = APIC_DEST_PHYSICAL,
				.delivery_status = 0,
				.pin_polarity = APIC_ACTIVE_HIGH,
				.remote_irr = 0,
				.trigger_mode = APIC_TRIGGER_EDGE,
				.mask = 1,
			    .reserved = 0,
				.destination = 0,
			};

			write_register(REGISTER_IOREDTLB_LOW(i), entry.low_dword);
			write_register(REGISTER_IOREDTLB_HIGH(i), entry.high_dword);
		}
	}

	void IOAPIC::add_redirection(madt_ioapic_override_entry_t redirection)
	{
		m_redirections.push_back(redirection);

		uint32_t local_int_num = redirection.gsi - m_gsi_base;
		uint32_t low = read_register(REGISTER_IOREDTLB_LOW(local_int_num));
		uint32_t high = read_register(REGISTER_IOREDTLB_HIGH(local_int_num));

		redirection_entry_t entry{
		    .low_dword = low,
		    .high_dword = high,
		};

		entry.pin_polarity = redirection.active_low;
		entry.trigger_mode = redirection.level_triggered;

		write_register(REGISTER_IOREDTLB_LOW(local_int_num), entry.low_dword);
		write_register(REGISTER_IOREDTLB_LOW(local_int_num), entry.high_dword);
	}

	void IOAPIC::set_nmi(madt_ioapic_nmi_entry_t nmi)
	{
		// TODO: Actually map NMI interrupt

		uint32_t local_int_num = nmi.gsi - m_gsi_base;
		uint32_t low = read_register(REGISTER_IOREDTLB_LOW(local_int_num));
		uint32_t high = read_register(REGISTER_IOREDTLB_HIGH(local_int_num));

		redirection_entry_t entry{
		    .low_dword = low,
		    .high_dword = high,
		};

		entry.pin_polarity = nmi.active_low;
		entry.trigger_mode = nmi.level_triggered;

		write_register(REGISTER_IOREDTLB_LOW(local_int_num), entry.low_dword);
		write_register(REGISTER_IOREDTLB_LOW(local_int_num), entry.high_dword);
	}

	void IOAPIC::enable()
	{
		for (size_t i = 0; i < m_gsi_count; i++)
		{
			redirection_entry_t entry;
			entry.low_dword = read_register(REGISTER_IOREDTLB_LOW(i));
			entry.mask = m_interrupt_mask[i / 32] >> (i % 32);
			write_register(REGISTER_IOREDTLB_LOW(i), entry.low_dword);
		}
	}

	void IOAPIC::disable()
	{
		for (size_t i = 0; i < m_gsi_count; i++)
		{
			redirection_entry_t entry;
			entry.low_dword = read_register(REGISTER_IOREDTLB_LOW(i));
			entry.mask = 1;
			write_register(REGISTER_IOREDTLB_LOW(i), entry.low_dword);
		}
	}

	bool IOAPIC::eoi(IRQHandler &handler __unused)
	{
		LAPIC::instance().eoi();
		return true;
	}

	bool IOAPIC::enable_interrupt(IRQHandler &handler)
	{
		uint32_t local_int_num = handler.original_interrupt_number() - m_gsi_base;

		for (auto redirection : m_redirections)
		{
			if (redirection.irq_src == handler.original_interrupt_number())
			{
				local_int_num = redirection.gsi - m_gsi_base;
				break;
			}
		}

		redirection_entry_t entry;
		entry.low_dword = read_register(REGISTER_IOREDTLB_LOW(local_int_num));
		entry.mask = 0;
		entry.vector_num = handler.interrupt_number();
		entry.destination = 0;
		write_register(REGISTER_IOREDTLB_LOW(local_int_num), entry.low_dword);
		write_register(REGISTER_IOREDTLB_HIGH(local_int_num), entry.high_dword);
		m_interrupt_mask[local_int_num / 32] &= ~(1 << (local_int_num % 32));

		return true;
	}

	bool IOAPIC::disable_interrupt(IRQHandler &handler)
	{
		uint32_t local_int_num = handler.original_interrupt_number() - m_gsi_base;

		for (auto redirection : m_redirections)
		{
			if (redirection.irq_src == handler.original_interrupt_number())
			{
				local_int_num = redirection.gsi - m_gsi_base;
				break;
			}
		}

		redirection_entry_t entry;
		entry.low_dword = read_register(REGISTER_IOREDTLB_LOW(local_int_num));
		entry.mask = 1;
		write_register(REGISTER_IOREDTLB_LOW(local_int_num), entry.low_dword);
		m_interrupt_mask[local_int_num / 32] |= 1 << (local_int_num % 32);

		return true;
	}
}
