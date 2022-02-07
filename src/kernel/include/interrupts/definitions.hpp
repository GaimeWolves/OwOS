#pragma once

#include <common_attributes.h>
#include <firmware/acpi/definitions.hpp>

namespace Kernel::Interrupts
{
    typedef struct madt_entry_t {
        uint8_t type;
        uint8_t length;
    } __packed madt_entry_t;

    typedef struct madt_lapic_entry_t {
        madt_entry_t header;
        uint8_t processor_id;
        uint8_t apic_id;
        uint32_t enabled : 1;
        uint32_t capable : 1;
        uint32_t reserved : 30;
    } __packed madt_lapic_entry_t;

    typedef struct madt_ioapic_entry_t {
        madt_entry_t header;
        uint8_t apic_id;
        uint8_t reserved;
        uint32_t apic_addr;
        uint32_t gsi_base;
    } __packed madt_ioapic_entry_t;

    typedef struct madt_ioapic_override_entry_t {
        madt_entry_t header;
        uint8_t bus_src;
        uint8_t irq_src;
        uint32_t gsi;
        uint16_t reserved1 : 2;
        uint16_t active_low : 1;
        uint16_t reserved2 : 5;
        uint16_t level_triggered : 1;
        uint16_t reserved3 : 7;
    } __packed madt_ioapic_override_entry_t;

    typedef struct madt_ioapic_nmi_entry_t {
        madt_entry_t header;
        uint8_t nmi_src;
        uint8_t reserved;
        uint16_t reserved1 : 2;
        uint16_t active_low : 1;
        uint16_t reserved2 : 5;
        uint16_t level_triggered : 1;
        uint16_t reserved3 : 7;
        uint32_t gsi;
    } __packed madt_ioapic_nmi_entry_t;

    typedef struct madt_lapic_nmi_entry_t {
        madt_entry_t header;
        uint8_t processor_id;
        uint16_t reserved1 : 2;
        uint16_t active_low : 1;
        uint16_t reserved2 : 5;
        uint16_t level_triggered : 1;
        uint16_t reserved3 : 7;
        uint8_t lint_number;
    } __packed madt_lapic_nmi_entry_t;

    typedef struct madt_lapic_override_entry_t {
        madt_entry_t header;
        uint16_t reserved;
        uint64_t address_override;
    } __packed madt_lapic_override_entry_t;

    typedef struct madt_x2apic_entry_t {
        madt_entry_t header;
        uint16_t reserved1;
        uint32_t apic_id;
        uint32_t enabled : 1;
        uint32_t capable : 1;
        uint32_t reserved2 : 30;
        uint32_t processor_id;
    } __packed madt_x2apic_entry_t;

    typedef struct madt_t {
        ACPI::acpi_sdt_table_t header;
        uint32_t lapic_addr;
        uint32_t dual_pics : 1;
        uint32_t reserved : 31;
        madt_entry_t entries[];
    } __packed madt_t;

    class MADT : public ACPI::SDT
    {
    public:
        MADT(uintptr_t phys_table_address)
            : SDT(phys_table_address)
        {
            m_madt = (madt_t *)header();
        }

        madt_t *get_table() { return m_madt; }

    private:
        madt_t *m_madt{nullptr};
    };
} // namespace Kernel::Interrupts
