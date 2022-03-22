#include <stdint.h>

#include <arch/interrupts.hpp>
#include <arch/Processor.hpp>
#include <common_attributes.h>
#include <firmware/acpi/Parser.hpp>
#include <multiboot.h>
#include <pci/pci.hpp>
#include <tests.hpp>
#include <vga/textmode.hpp>
#include <time/TimeManager.hpp>
#include <interrupts/LAPIC.hpp>
#include <interrupts/IRQHandler.hpp>
#include <interrupts/InterruptManager.hpp>
#include <locking/Mutex.hpp>

#include <libk/kcstdio.hpp>

namespace Kernel
{
	extern "C"
	{
		uintptr_t _kernel_start;
		uintptr_t _kernel_end;
	}

	extern "C" __noreturn void entry(uint32_t magic, multiboot_info_t *multiboot_info)
	{
		auto start = (uintptr_t)&_kernel_start;
		auto end = (uintptr_t)&_kernel_end;
		uintptr_t size = end - start;

		// TODO: Fix initialization order
		// LibK::printf_debug_msg("[KERNEL] Kernel image size: %x", size);

		assert(size <= 0x300000);
		assert(magic == MULTIBOOT_BOOTLOADER_MAGIC);
		assert(multiboot_info);

		ACPI::Parser::instance().init();

		Interrupts::InterruptManager::instance().initialize();
		CPU::Processor::initialize(0);
		CPU::Processor::current().enable_interrupts();

		Time::TimeManager::instance().initialize();

		VGA::Textmode::init();

		PCI::HostBridge::instance().init();

		Tests::test_crtx();
		Tests::test_heap();
		Tests::test_printf();
		Tests::test_vmm();

		Interrupts::LAPIC::instance().start_smp_boot();

#ifdef _DEBUG
		LibK::printf_debug_msg("Reached end of entry");
#endif

		for (;;)
			CPU::Processor::halt();
	}

	static Locking::Mutex mutex;

	extern "C" __noreturn void ap_entry(uint32_t cpu_id)
	{
		mutex.lock();
		CPU::Processor::early_initialize(cpu_id);
		CPU::Processor::initialize(cpu_id);
		mutex.unlock();

		mutex.lock();
		LibK::printf_debug_msg("Hello from AP %d", cpu_id);
		mutex.unlock();

		for (;;)
			CPU::Processor::halt();
	}
} // namespace Kernel
