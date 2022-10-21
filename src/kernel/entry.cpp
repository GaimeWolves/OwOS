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
#include <filesystem/VirtualFileSystem.hpp>
#include <logging/logger.hpp>
#include <elf/elf.hpp>

namespace Kernel
{
	class TestMessage : public CPU::ProcessorMessage
	{
	public:
		void handle() override
		{
			log("DEBUG", "Handling test message", CPU::Processor::current().id());
		}
	};

	static Locking::Mutex s_dummy_mutex;

	static void dummy_thread_A()
	{
		log("DEBUG", "Thread A starting");
		log("DEBUG", "Thread A acquiring lock");
		s_dummy_mutex.lock();
		log("DEBUG", "Thread A acquired lock");
		Time::EventManager::instance().sleep(1000);
		log("DEBUG", "Thread A releasing lock");
		s_dummy_mutex.unlock();
		log("DEBUG", "Thread A released lock");
		Time::EventManager::instance().sleep(500);
		log("DEBUG", "Thread A acquiring lock");
		s_dummy_mutex.lock();
		log("DEBUG", "Thread A acquired lock");

		CoreScheduler::terminate_current();
	}

	__noreturn static void dummy_thread_B()
	{
		log("DEBUG", "Thread B starting");
		Time::EventManager::instance().sleep(500);
		log("DEBUG", "Thread B acquiring lock");
		s_dummy_mutex.lock();
		log("DEBUG", "Thread B acquired lock");
		Time::EventManager::instance().sleep(1000);
		log("DEBUG", "Thread B releasing lock");
		s_dummy_mutex.unlock();
		log("DEBUG", "Thread B released lock");

		for (;;)
		{
			uintptr_t esp = 0;
			asm("mov %%esp, %0" : "=m"(esp) ::);

			log("DEBUG", "Heap: %d bytes used --- esp: %p", Heap::getStatistics().used, esp);
			Time::EventManager::instance().sleep(1000);
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

		// TODO: Get root partition through cmdline arguments
		VirtualFileSystem::instance().initialize(AHCIManager::instance().devices()[0].partitions()[0]);

		log("SMP", "Starting scheduler and sleeping until first tick...");

		auto threadA = GlobalScheduler::create_kernel_only_thread(nullptr, reinterpret_cast<uintptr_t>(dummy_thread_A));
		auto threadB = GlobalScheduler::create_kernel_only_thread(nullptr, reinterpret_cast<uintptr_t>(dummy_thread_B));
		GlobalScheduler::start_thread(threadA);
		GlobalScheduler::start_thread(threadB);

		auto dtc_mem_space = Memory::VirtualMemoryManager::create_memory_space();
		Memory::VirtualMemoryManager::load_memory_space(&dtc_mem_space);
		Memory::mapping_config_t config;
		config.userspace = true;
		Memory::VirtualMemoryManager::instance().allocate_region_at(0x55555000, 4096, config);
		memmove((void *)0x55555000, (void *)dummy_thread_C, 100);

		static char str[] = "Hello from Ring 3 using a syscall :D";
		memmove((void *)0x55555500, (void *)str, 100);

		auto threadC = GlobalScheduler::create_userspace_thread(nullptr, 0x55555000, dtc_mem_space);
		GlobalScheduler::start_thread(threadC);

		Memory::VirtualMemoryManager::load_memory_space(Memory::VirtualMemoryManager::instance().get_kernel_memory_space());

		auto file = VirtualFileSystem::instance().find_by_path("/lib/ld-owos.so");
		auto process = ELF::load(file);
		process->start();

		CoreScheduler::initialize();

		start_logger_thread();

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
