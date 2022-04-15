#include <stdint.h>

#include <arch/Processor.hpp>
#include <common_attributes.h>
#include <firmware/acpi/Parser.hpp>
#include <pci/pci.hpp>
#include <tests.hpp>
#include <vga/textmode.hpp>
#include <time/EventManager.hpp>
#include <interrupts/LAPIC.hpp>
#include <interrupts/InterruptManager.hpp>
#include <locking/Mutex.hpp>
#include <time/PIT.hpp>
#include <processes/CoreScheduler.hpp>
#include <processes/GlobalScheduler.hpp>
#include <memory/VirtualMemoryManager.hpp>
#include <syscall/SyscallDispatcher.hpp>
#include <storage/ata/AHCIManager.hpp>

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

	static Locking::Mutex s_dummy_mutex;

	static void dummy_thread_A()
	{
		LibK::printf_debug_msg("Thread A starting");
		LibK::printf_debug_msg("Thread A acquiring lock");
		s_dummy_mutex.lock();
		LibK::printf_debug_msg("Thread A acquired lock");
		Time::EventManager::instance().sleep(1000);
		LibK::printf_debug_msg("Thread A releasing lock");
		s_dummy_mutex.unlock();
		LibK::printf_debug_msg("Thread A released lock");
		Time::EventManager::instance().sleep(500);
		LibK::printf_debug_msg("Thread A acquiring lock");
		s_dummy_mutex.lock();
		LibK::printf_debug_msg("Thread A acquired lock");

		CoreScheduler::terminate_current();
	}

	__noreturn static void dummy_thread_B()
	{
		LibK::printf_debug_msg("Thread B starting");
		Time::EventManager::instance().sleep(500);
		LibK::printf_debug_msg("Thread B acquiring lock");
		s_dummy_mutex.lock();
		LibK::printf_debug_msg("Thread B acquired lock");
		Time::EventManager::instance().sleep(1000);
		LibK::printf_debug_msg("Thread B releasing lock");
		s_dummy_mutex.unlock();
		LibK::printf_debug_msg("Thread B released lock");

		for (;;)
		{
			uintptr_t esp = 0;
			asm("mov %%esp, %0" : "=m"(esp) ::);

			LibK::printf_debug_msg("Heap: %d bytes used --- esp: %p", Heap::getStatistics().used, esp);
			Time::EventManager::instance().sleep(1000); // TODO: Fix memory leak when sleeping for 1000ms (probably caused by a missing free when merging two events)
		}
	}

	__noreturn void dummy_thread_C()
	{
		asm("int $0x80" : : "b" (0x55555500), "a" (0) : "memory");

		for (;;)
			;
	}

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
		AHCIManager::instance().initialize();

		Tests::test_crtx();
		Tests::test_heap();
		Tests::test_printf();
		Tests::test_vmm();

		Interrupts::LAPIC::instance().start_smp_boot();

		SyscallDispatcher::initialize();

#ifdef _DEBUG
		LibK::printf_debug_msg("[BSP] Starting scheduler and sleeping until first tick...");
#endif

		GlobalScheduler::start_kernel_only_thread(reinterpret_cast<uintptr_t>(dummy_thread_A));
		GlobalScheduler::start_kernel_only_thread(reinterpret_cast<uintptr_t>(dummy_thread_B));

		auto dtc_mem_space = Memory::VirtualMemoryManager::instance().create_memory_space();
		Memory::VirtualMemoryManager::instance().load_memory_space(&dtc_mem_space);
		Memory::mapping_config_t config;
		config.userspace = true;
		Memory::VirtualMemoryManager::instance().allocate_region_at(0x55555000, 4096, config);
		memmove((void *)0x55555000, (void *)dummy_thread_C, 100);

		static char str[] = "Hello from Ring 3 using a syscall :D";
		memmove((void *)0x55555500, (void *)str, 100);

		GlobalScheduler::start_userspace_thread(0x55555000, dtc_mem_space);

		Memory::VirtualMemoryManager::instance().load_kernel_space();

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
		CPU::Processor::current().smp_initialize_messaging();
		CPU::Processor::current().enable_interrupts();
		Interrupts::APICTimer::instance().initialize();
		mutex.unlock();

		LibK::printf_debug_msg("[SMP] Starting scheduler and sleeping until first tick...");

		CoreScheduler::initialize();

		for (;;)
			CPU::Processor::sleep();
	}
} // namespace Kernel
