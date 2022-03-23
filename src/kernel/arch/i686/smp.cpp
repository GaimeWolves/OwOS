#include <arch/smp.hpp>

#include <libk/kcstring.hpp>

#include <memory/PhysicalMemoryManager.hpp>
#include <memory/VirtualMemoryManager.hpp>
#include <arch/Processor.hpp>

namespace Kernel::CPU
{
	extern "C"
	{
		extern uintptr_t _start_ap_boot;
		extern uintptr_t _end_ap_boot;
		extern uintptr_t ap_bsp_cr3;
		extern uintptr_t ap_bsp_idtr;
		extern uintptr_t ap_bsp_gdtr;
		extern uintptr_t ap_cpu_id;
		extern uintptr_t ap_do_continue;
		extern uintptr_t ap_code_virt_addr;
		extern uintptr_t ap_stacks;
	}

	constexpr uintptr_t boot_address = 0x8000;
	static size_t ap_boot_code_size = LibK::round_up_to_multiple<uintptr_t>((uintptr_t)&_end_ap_boot - (uintptr_t)&_start_ap_boot, PAGE_SIZE);

	static uintptr_t *variable(uintptr_t start, uintptr_t variable)
	{
		return (uintptr_t *)(variable - (uintptr_t)&_start_ap_boot + start);
	}

	void prepare_smp_boot_environment()
	{
		// Reserve memory space for boot code
		Memory::PhysicalMemoryManager::instance().alloc(ap_boot_code_size, boot_address);
		assert(Memory::VirtualMemoryManager::instance().try_identity_map(boot_address, ap_boot_code_size));

		// Copy AP boot code to 0x8000
		memcpy((void *)boot_address, (void *)(&_start_ap_boot), ap_boot_code_size);
	}

	void initialize_smp_boot_environment(const LibK::vector<uintptr_t> &kernel_stacks, uint32_t *cpu_id_ptr, uint32_t *do_continue_ptr)
	{
		// Set the variables needed for initialization in the AP boot code
		*(variable(boot_address, (uintptr_t)&ap_bsp_cr3)) = Processor::get_page_directory();
		*(variable(boot_address, (uintptr_t)&ap_bsp_idtr)) = Processor::current().get_idtr_address();
		*(variable(boot_address, (uintptr_t)&ap_bsp_gdtr)) = Processor::current().get_gdtr_address();
		*(variable(boot_address, (uintptr_t)&ap_cpu_id)) = (uintptr_t)cpu_id_ptr;
		*(variable(boot_address, (uintptr_t)&ap_do_continue)) = (uintptr_t)do_continue_ptr;
		*(variable(boot_address, (uintptr_t)&ap_code_virt_addr)) = (uintptr_t)&_start_ap_boot;
		for (size_t i = 0; i < kernel_stacks.size(); i++)
			*(variable(boot_address, (uintptr_t)&ap_stacks + i * sizeof(uintptr_t))) = kernel_stacks[i];
	}

	void finalize_smp_boot_environment()
	{
		// Free the used memory again
		Memory::VirtualMemoryManager::instance().free((void *)boot_address);
		Memory::PhysicalMemoryManager::instance().free((void *)boot_address, ap_boot_code_size);
	}
}
