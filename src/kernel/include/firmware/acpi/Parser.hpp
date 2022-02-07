#pragma once

#include <stddef.h>
#include <stdint.h>

#include <common_attributes.h>
#include <firmware/acpi/definitions.hpp>

namespace Kernel
{
	namespace ACPI
	{
		typedef struct rsdp_descriptor_t
		{
			char signature[8];
			uint8_t checksum;
			char oem_id[6];
			uint8_t revision;
			uint32_t rsdt_address;

			// RSDP 2.0 fields
			uint32_t length;
			uint64_t xsdt_address;
			uint8_t extended_checksum;
			uint8_t reserved[3];
		} __packed rsdp_descriptor_t;

		class Parser
		{
		public:
			static Parser &instance()
			{
				static Parser *instance{nullptr};

				if (!instance)
					instance = new Parser();

				return *instance;
			}

			Parser(Parser &) = delete;
			void operator=(const Parser &) = delete;

			void init();
			uintptr_t find_table_by_signature(const char *signature);

		private:
			Parser() = default;
			~Parser() = default;

			rsdp_descriptor_t *m_rsdp{nullptr};
			RSDT m_rsdt{0};
		};
	} // namespace ACPI
} // namespace Kernel
