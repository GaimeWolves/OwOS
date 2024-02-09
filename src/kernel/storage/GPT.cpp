#include <storage/GPT.hpp>

#include <stdint.h>

#include <libk/GUID.hpp>

#include <memory/VirtualMemoryManager.hpp>
#include <storage/StorageDevice.hpp>

namespace Kernel::GPT
{
	typedef struct partition_table_header_t
	{
		char signature[8];
		uint32_t revision;
		uint32_t header_size;
		uint32_t checksum_header;
		uint32_t rsv1;
		uint64_t header_lba;
		uint64_t header_mirror_lba;
		uint64_t first_usable_block;
		uint64_t last_usable_block;
		uint8_t guid[16];
		uint64_t partition_array_lba;
		uint32_t partition_entry_count;
		uint32_t partition_entry_size;
		uint32_t checksum_partition_array;
	} __packed partition_table_header_t;

	typedef struct partition_entry_t
	{
		uint8_t type_guid[16];
		uint8_t unique_guid[16];
		uint64_t start_lba;
		uint64_t end_lba;
		uint64_t attributes;
		uint16_t name[]; // UNICODE-16 encoded
	} __packed partition_entry_t;

	static constexpr LibK::GUID UNUSED = LibK::GUID((const uint8_t[]){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
	static constexpr char SIGNATURE[] = "EFI PART";

	bool try_parse(StorageDevice &device)
	{
		auto header_region = Memory::VirtualMemoryManager::instance().allocate_region(device.block_size());
		device.read_blocks(1, 1, reinterpret_cast<char *>(header_region.virt_address));
		auto header = reinterpret_cast<partition_table_header_t *>(header_region.virt_address);

		if (strncmp(header->signature, SIGNATURE, 8) != 0)
		{
			Memory::VirtualMemoryManager::instance().free(header_region);
			return false;
		}

		size_t table_size = LibK::round_up_to_multiple(header->partition_entry_count * header->partition_entry_size, device.block_size());
		size_t block_count = table_size / device.block_size();

		auto table_region = Memory::VirtualMemoryManager::instance().allocate_region(table_size);
		device.read_blocks(header->partition_array_lba, block_count, reinterpret_cast<char *>(table_region.virt_address));
		auto partition_table = reinterpret_cast<char *>(table_region.virt_address);

		for (size_t i = 0; i < header->partition_entry_count; i++)
		{
			auto partition_entry = *reinterpret_cast<partition_entry_t *>(&partition_table[i * header->partition_entry_size]);
			auto type_guid = LibK::GUID(partition_entry.type_guid);

			if (type_guid == UNUSED)
				continue;

			device.add_partition(partition_entry.start_lba, partition_entry.end_lba - partition_entry.start_lba);
		}

		Memory::VirtualMemoryManager::instance().free(header_region);
		Memory::VirtualMemoryManager::instance().free(table_region);

		return true;
	}
}