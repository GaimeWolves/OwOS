#pragma once

#include <stdint.h>

namespace Kernel::ACPI
{
    typedef struct acpi_sdt_table_t
    {
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char oemId[6];
        char oemTableId[8];
        uint32_t oemRevision;
        uint32_t creatorId;
        uint32_t creatorRevision;
    } __packed acpi_sdt_table_t;

    class SDT {
    public:
        SDT(uintptr_t phys_header_addr);

        void free(); // Find a way to safely call this in the destructor
        bool is_valid() const { return m_is_valid; }
        acpi_sdt_table_t *header() const { return m_header; }
    
    private:
        acpi_sdt_table_t *m_header{nullptr};
        bool m_is_valid{false};
    };

        typedef struct rsdt_t {
        acpi_sdt_table_t header;
        uintptr_t tables[];
    } __packed rsdt_t;

    class RSDT : public SDT {
    public:
        RSDT &operator=(RSDT &&) = default;

        RSDT(uintptr_t phys_table_address)
            : SDT(phys_table_address)
        {
            m_rsdt = (rsdt_t *)header();

            if (is_valid())
                m_table_count = (m_rsdt->header.length - sizeof(acpi_sdt_table_t)) / sizeof(uint32_t);
        }

        uintptr_t find_table_by_signature(const char *signature);

    private:
        rsdt_t *m_rsdt{nullptr};
        size_t m_table_count{0};
    };
} // namespace Kernel::ACPI
