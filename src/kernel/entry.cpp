#include <stdint.h>

#include <arch/Processor.hpp>
#include <common_attributes.h>
#include <devices/FramebufferDevice.hpp>
#include <elf/elf.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <firmware/acpi/Parser.hpp>
#include <interrupts/InterruptManager.hpp>
#include <interrupts/LAPIC.hpp>
#include <locking/Mutex.hpp>
#include <logging/logger.hpp>
#include <memory/VirtualMemoryManager.hpp>
#include <pci/pci.hpp>
#include <processes/CoreScheduler.hpp>
#include <processes/GlobalScheduler.hpp>
#include <storage/ata/AHCIManager.hpp>
#include <syscall/SyscallDispatcher.hpp>
#include <tests.hpp>
#include <time/EventManager.hpp>
#include <time/PIT.hpp>
#include <tty/VirtualConsole.hpp>
#include <vga/textmode.hpp>

namespace Kernel
{
	extern "C" __noreturn void entry(multiboot_info_t *multiboot_info)
	{
		ACPI::Parser::instance().init();

		Interrupts::InterruptManager::instance().initialize();
		CPU::Processor::initialize(0);
		CPU::Processor::current().smp_initialize_messaging();
		CPU::Processor::current().enable_interrupts();

		Time::EventManager::instance().register_timer(&Time::PIT::instance());

		Interrupts::APICTimer::instance().initialize();
		Time::EventManager::instance().register_timer(&Interrupts::APICTimer::instance());

		FramebufferDevice::instance().configure(multiboot_info);

		PCI::HostBridge::instance().init();
		AHCIManager::instance().initialize();

		Tests::test_crtx();
		Tests::test_heap();
		Tests::test_printf();
		Tests::test_vmm();

		Interrupts::LAPIC::instance().start_smp_boot();

		SyscallDispatcher::initialize();

		// TODO: Get root partition through cmdline arguments
		VirtualFileSystem::instance().initialize(AHCIManager::instance().devices()[0].partitions()[1]);

		VirtualConsole::initialize();

		log("SMP", "Starting scheduler and sleeping until first tick...");

		auto process = ELF::load("/bin/test");
		process->start();

		start_logger_thread();

		CoreScheduler::initialize();

		for (;;)
			CPU::Processor::sleep();
	}

	static Locking::Mutex mutex;

	extern "C" __noreturn void ap_entry(uint32_t cpu_id)
	{
		mutex.lock();
		CPU::Processor::early_initialize(cpu_id);
		Interrupts::LAPIC::instance().set_ap_id(cpu_id);
		Interrupts::LAPIC::instance().initialize_ap();
		Interrupts::LAPIC::instance().enable();
		CPU::Processor::initialize(cpu_id);
		CPU::Processor::current().set_memory_space(Memory::VirtualMemoryManager::instance().get_kernel_memory_space());
		CPU::Processor::current().smp_initialize_messaging();
		Interrupts::APICTimer::instance().initialize();
		mutex.unlock();

		CPU::Processor::current().enable_interrupts();

		log("SMP", "Starting scheduler and sleeping until first tick...");

		CoreScheduler::initialize();

		for (;;)
			CPU::Processor::sleep();
	}
} // namespace Kernel
