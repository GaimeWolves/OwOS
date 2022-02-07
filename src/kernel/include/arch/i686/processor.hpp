#pragma once

#include <stdint.h>

#include <common_attributes.h>

namespace Kernel::Processor
{
	always_inline uintptr_t cr2()
	{
		uintptr_t cr2 = 0;

		asm volatile("mov %%cr2, %%eax"
		             : "=a"(cr2));

		return cr2;
	}
}
