#pragma once

#ifdef ARCH_i686
#	include <arch/i686/Processor.hpp>
#else
#	error No arch-specific Processor.hpp included
#endif

namespace Kernel::CPU
{
	bool is_bsp_initialization_finished();
	void set_bsp_initialization_finished();

	void print_registers();

	void halt_aps();
}