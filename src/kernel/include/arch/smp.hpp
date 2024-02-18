#pragma once

#include <stdint.h>

#include <libk/kvector.hpp>

namespace Kernel::CPU
{
	// These methods handle creation and deletion of the SMP boot environment
	// For x86 this would be copying the AP boot code to 0x8000 and setting the variables in there.
	void prepare_smp_boot_environment();
	void initialize_smp_boot_environment(const LibK::vector<uintptr_t> &kernel_stacks, uint32_t *cpu_id_ptr, uint32_t *do_continue_ptr);
	void finalize_smp_boot_environment();

	class ProcessorMessage
	{
	public:
		virtual void handle() = 0;
	};
}
