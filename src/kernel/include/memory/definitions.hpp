#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Kernel::Memory
{
	typedef struct region_t
	{
		uintptr_t address;
		size_t size;

		bool overlaps(region_t &other) const
		{
			uintptr_t t_start = address;
			uintptr_t t_end = end();

			uintptr_t o_start = other.address;
			uintptr_t o_end = other.end();

			return t_start <= o_end && o_start <= t_end;
		}

		bool contains(uintptr_t ptr) const
		{
			return ptr >= address && ptr <= end();
		}

		bool operator==(region_t &other) const { return this->address == other.address; }
		bool operator<(region_t &other) const { return this->address < other.address; }
		bool operator>(region_t &other) const { return this->address > other.address; }
		bool operator<=(region_t &other) const { return this->address <= other.address; }
		bool operator>=(region_t &other) const { return this->address >= other.address; }

		size_t end() const { return address + size - 1; }
	} region_t;

	typedef struct memory_region_t
	{
		region_t region;
		uintptr_t phys_address;

		bool mapped;
		bool present;
		bool kernel;
		bool is_mmio;
	} memory_region_t;
} // namespace Kernel::Memory