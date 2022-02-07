#include <stdint.h>

#include <arch/interrupts.hpp>
#include <arch/processor.hpp>
#include <arch/spinlock.hpp>
#include <common_attributes.h>
#include <firmware/acpi/Parser.hpp>
#include <multiboot.h>
#include <pci/pci.hpp>
#include <tests.hpp>
#include <vga/textmode.hpp>

#include <libk/kcstdio.hpp>

namespace Kernel
{
	extern "C" __noreturn void entry(uint32_t magic, multiboot_info_t *multiboot_info)
	{
		auto start = (uintptr_t)&_kernel_start;
		auto end = (uintptr_t)&_kernel_end;
		uintptr_t size = end - start;
		LibK::printf_debug_msg("[KERNEL] Kernel image size: %x", size);
		assert(magic == MULTIBOOT_BOOTLOADER_MAGIC);
		assert(multiboot_info);

		ACPI::Parser::instance().init();

		Processor::init();
		Processor::init_interrupts();
		Processor::enable_interrupts();

		VGA::Textmode::init();

		PCI::HostBridge::instance().init();

		Tests::test_crtx();
		Tests::test_heap();
		Tests::test_printf();
		Tests::test_vmm();

#ifdef _DEBUG
		LibK::printf_debug_msg("Reached end of entry");
#endif

		for (;;)
			Processor::sleep();
	}
} // namespace Kernel