#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Kernel::Memory
{
	enum class CachingMode
	{
		Uncacheable,
		WriteCombining,
		WriteThrough,
		WriteProtect,
		WriteBack,
	};

	typedef struct region_t
	{
		uintptr_t address;
		size_t size;

		[[nodiscard]] void *pointer() const { return (void *)address; }

		[[nodiscard]] bool overlaps(const region_t &other) const
		{
			uintptr_t t_start = address;
			uintptr_t t_end = end();

			uintptr_t o_start = other.address;
			uintptr_t o_end = other.end();

			return t_start <= o_end && o_start <= t_end;
		}

		[[nodiscard]] bool contains(uintptr_t ptr) const
		{
			return ptr >= address && ptr <= end();
		}

		bool operator==(const region_t &other) const { return this->address == other.address; }
		bool operator<(const region_t &other) const { return this->address < other.address; }
		bool operator>(const region_t &other) const { return this->address > other.address; }
		bool operator<=(const region_t &other) const { return this->address <= other.address; }
		bool operator>=(const region_t &other) const { return this->address >= other.address; }

		size_t end() const { return address + size - 1; }
	} region_t;

	typedef struct mapping_config_t
	{
		bool readable = true;
		bool writeable = true;
		bool userspace = false;
		CachingMode caching_mode = CachingMode::WriteThrough;

		region_t bounds = {
		    .address = 0,
		    .size = SIZE_MAX,
		};

		size_t alignment = 0;
	} mapping_config_t;

	typedef struct memory_region_t
	{
		uintptr_t virt_address;
		uintptr_t phys_address;
		size_t size;

		bool mapped;
		bool present;
		bool allocated;

		mapping_config_t config;

		region_t virt_region() const { return {virt_address, size}; };
		region_t phys_region() const { return {phys_address, size}; };
		void *virtual_offset(uintptr_t orig_phys_addr) const { return reinterpret_cast<void *>(virt_address + (orig_phys_addr - phys_address)); }

		// TODO: Maybe refactor this (no separate types for region and memory region)
		bool operator==(const memory_region_t &other) const { return this->virt_address == other.virt_address; }
		bool operator<(const memory_region_t &other) const { return this->virt_address < other.virt_address; }
		bool operator>(const memory_region_t &other) const { return this->virt_address > other.virt_address; }
		bool operator<=(const memory_region_t &other) const { return this->virt_address <= other.virt_address; }
		bool operator>=(const memory_region_t &other) const { return this->virt_address >= other.virt_address; }
	} memory_region_t;
} // namespace Kernel::Memory
