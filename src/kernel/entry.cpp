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
#include <interrupts/PIC.hpp>
#include <interrupts/IRQHandler.hpp>
#include <interrupts/InterruptManager.hpp>

#include <libk/kcstdio.hpp>

namespace Kernel
{
	extern "C"
	{
		uintptr_t _kernel_start;
		uintptr_t _kernel_end;
	}

	class TempKeyboardHandler : public Interrupts::IRQHandler
	{
	public:
		TempKeyboardHandler()
		    : Interrupts::IRQHandler(1)
		{}

		void handle_interrupt(const Processor::registers_t &regs __unused) override
		{
			IO::in<uint8_t>(0x60);
			LibK::printf_debug_msg("Got kbd interrupt");
		}
	};

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
		Interrupts::InterruptManager::instance().initialize();
		Processor::enable_interrupts();

		VGA::Textmode::init();

		TempKeyboardHandler handlerA, handlerB;
		handlerA.enable_irq();
		handlerB.enable_irq();

		PCI::HostBridge::instance().init();

		Tests::test_crtx();
		Tests::test_heap();
		Tests::test_printf();
		Tests::test_vmm();

#ifdef _DEBUG
		LibK::printf_debug_msg("Reached end of entry");
#endif

		for (;;)
			Processor::halt();
	}
} // namespace Kernel
