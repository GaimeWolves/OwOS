#include <firmware/acpi/Parser.hpp>

#include <stdint.h>

#include "logging/logger.hpp"
#include <arch/memory.hpp>
#include <common_attributes.h>
#include <firmware/BIOS.hpp>
#include <libk/kcstdio.hpp>
#include <libk/kcstring.hpp>
#include <memory/VirtualMemoryManager.hpp>

namespace Kernel::ACPI
{
		static rsdp_descriptor_t *find_rsdp();
		static bool check_rsdp_checksum(rsdp_descriptor_t *rsdp);

		static rsdp_descriptor_t *find_rsdp()
		{
			static const char *rsdp_signature = "RSD PTR ";

			auto ebda_region = BIOS::map_ebda_rom();

			rsdp_descriptor_t *rsdp = nullptr;

			for (uintptr_t i = 0; i < ebda_region.size - 8; i++)
			{
				if (strncmp((char *)(ebda_region.virt_address + i), rsdp_signature, 8) == 0)
				{
					rsdp = (rsdp_descriptor_t *)(ebda_region.virt_address + i);

					if (!check_rsdp_checksum(rsdp))
					{
						rsdp = nullptr;
						break;
					}

					rsdp = (rsdp_descriptor_t *)(ebda_region.phys_address + i);

					break;
				}
			}

			Memory::VirtualMemoryManager::instance().free(ebda_region);

			if (!rsdp)
			{
				auto bios_region = BIOS::map_bios_rom();

				for (uintptr_t i = 0; i < bios_region.size - 8; i++)
				{
					if (strncmp((char *)(bios_region.virt_address + i), rsdp_signature, 8) == 0)
					{
						rsdp = (rsdp_descriptor_t *)(bios_region.virt_address + i);

						if (!check_rsdp_checksum(rsdp))
						{
							rsdp = nullptr;
							break;
						}

						rsdp = (rsdp_descriptor_t *)(bios_region.phys_address + i);

						break;
					}
				}

				Memory::VirtualMemoryManager::instance().free(bios_region);
			}

			if (rsdp)
			{
				log("ACPI", "RSDP found at %p", rsdp);

				rsdp = Memory::VirtualMemoryManager::instance().map_typed<rsdp_descriptor_t>((uintptr_t)rsdp);
			}

			return rsdp;
		}

		static bool check_rsdp_checksum(rsdp_descriptor_t *rsdp)
		{
			uint32_t checksum = 0;
			auto start = (uint8_t *)rsdp;
			auto end = (uint8_t *)(&rsdp->length); // First attribute of ACPI 2.0 table

			for (auto byte = start; byte < end; byte++)
				checksum += *byte;

			if (checksum & 0xFF)
				return false;

			// Is ACPI version 2.0?
			if (rsdp->revision == 2)
			{
				checksum = 0;
				start = (uint8_t *)(&rsdp->length);
				end = ((uint8_t *)rsdp + sizeof(rsdp_descriptor_t));

				for (auto byte = start; byte < end; byte++)
					checksum += *byte;

				if (checksum & 0xFF)
					return false;
			}

			return true;
		}

		SDT::SDT(uintptr_t phys_header_addr)
		{
			if (!phys_header_addr)
				return;

			auto *header = Memory::VirtualMemoryManager::instance().map_typed<acpi_sdt_table_t>(phys_header_addr);
			size_t actual_size = header->length;
			Memory::VirtualMemoryManager::instance().free(header);
		    auto table = Memory::VirtualMemoryManager::instance().map_region(phys_header_addr, actual_size);

			m_header = (acpi_sdt_table_t *)table.virtual_offset(phys_header_addr);

			uint32_t checksum = 0;
			auto start = (uint8_t *)m_header;
			auto end = start + m_header->length;

			for (auto byte = start; byte < end; byte++)
				checksum += *byte;

			m_is_valid = (checksum & 0xFF) == 0;
		}

		void SDT::free() {
			if (m_header)
				Memory::VirtualMemoryManager::instance().free(m_header);
		}

		uintptr_t RSDT::find_table_by_signature(const char *signature)
		{
			for (size_t i = 0; i < m_table_count; i++)
			{
				SDT sdt(m_rsdt->tables[i]);

				if (sdt.is_valid() && strncmp(sdt.header()->signature, signature, 4) == 0)
				{
					sdt.free();
					return m_rsdt->tables[i];
				}

				sdt.free();
			}

			return 0;
		}

		uintptr_t Parser::find_table_by_signature(const char *signature)
		{
			assert(m_rsdp);
			assert(m_rsdt.is_valid());

			auto address = m_rsdt.find_table_by_signature(signature);

			return address;
		}

		void Parser::init()
		{
			m_rsdp = find_rsdp();

			if (!m_rsdp)
				return;

			m_rsdt = RSDT((uintptr_t) m_rsdp->rsdt_address);	
		}
} // namespace Kernel::ACPI