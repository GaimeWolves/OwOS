#include <stddef.h>
#include <stdint.h>

#include <arch/interrupts.hpp>
#include <arch/processor.hpp>
#include <devices/SerialPort.hpp>
#include <memory/VirtualMemoryManager.hpp>
#include <multiboot.h>
#include <common_attributes.h>
#include <pci/pci.hpp>
#include <tests.hpp>
#include <vga/textmode.hpp>

#include <libk/kcmalloc.hpp>
#include <libk/kcstdio.hpp>
#include <libk/kcstring.hpp>

namespace Kernel
{
	extern "C" __noreturn void entry(uint32_t magic, multiboot_info_t *multiboot_info)
	{
		assert(magic == MULTIBOOT_BOOTLOADER_MAGIC);
		assert(multiboot_info);

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