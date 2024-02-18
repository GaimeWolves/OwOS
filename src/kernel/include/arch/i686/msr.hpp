#pragma once

#include <stdint.h>

#include <common_attributes.h>

#define IA32_PAT_MSR 0x277

namespace Kernel
{
	always_inline uint64_t read_msr(uint32_t msr)
	{
		uint32_t lo;
		uint32_t hi;

		asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));

		return ((uint64_t)hi << 32 | (uint64_t)lo);
	}

	always_inline void write_msr(uint32_t msr, uint64_t value)
	{
		asm volatile("wrmsr" : : "a"((uint32_t)(value & 0xFFFFFFFF)), "d"((uint32_t)(value >> 32)), "c"(msr));
	}
}
