#include <stdint.h>

#include <arch/interrupts.hpp>
#include <arch/Processor.hpp>
#include <common_attributes.h>
#include <firmware/acpi/Parser.hpp>
#include <multiboot.h>
#include <pci/pci.hpp>
#include <tests.hpp>
#include <vga/textmode.hpp>
#include <time/EventManager.hpp>
#include <interrupts/LAPIC.hpp>
#include <interrupts/InterruptManager.hpp>
#include <locking/Mutex.hpp>
#include <time/PIT.hpp>

#include <libk/kcstdio.hpp>

namespace Kernel
{
	class TestMessage : public CPU::ProcessorMessage
	{
	public:
		void handle() override
		{
			LibK::printf_debug_msg("[SMP] Handling test message", CPU::Processor::current().id());
		}
	};

	extern "C" __noreturn void entry()
	{
		ACPI::Parser::instance().init();

		Interrupts::InterruptManager::instance().initialize();
		CPU::Processor::initialize(0);
		CPU::Processor::current().smp_initialize_messaging();
		CPU::Processor::current().enable_interrupts();

		Time::EventManager::instance().register_timer(&Time::PIT::instance());

		Interrupts::APICTimer::instance().initialize();
		Time::EventManager::instance().register_timer(&Interrupts::APICTimer::instance());

		VGA::Textmode::init();

		PCI::HostBridge::instance().init();

		Tests::test_crtx();
		Tests::test_heap();
		Tests::test_printf();
		Tests::test_vmm();

		Interrupts::LAPIC::instance().start_smp_boot();

#ifdef _DEBUG
		LibK::printf_debug_msg("[BSP] Reached end of entry! Halting!");
#endif

		Time::EventManager::instance().sleep(1000);

		TestMessage message;

		CPU::Processor::enumerate([&](CPU::Processor &core) {
			core.smp_enqueue_message(&message);
			return true;
		});

		// This should trigger all cores to process their message queues
		CPU::Processor::smp_poke_all(true);

		for (;;)
			CPU::Processor::halt();
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
		CPU::Processor::current().smp_initialize_messaging();
		CPU::Processor::current().enable_interrupts();
		Interrupts::APICTimer::instance().initialize();
		mutex.unlock();

		LibK::printf_debug_msg("[SMP] Initialized AP", cpu_id);

		LibK::printf_debug_msg("[SMP] Reached end of entry! Entering sleep...", cpu_id);

		for (;;)
			CPU::Processor::sleep();
	}
} // namespace Kernel
