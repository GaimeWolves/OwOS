#include <interrupts/LAPIC.hpp>

#include <arch/interrupts.hpp>
#include <arch/Processor.hpp>
#include <interrupts/InterruptHandler.hpp>
#include <memory/VirtualMemoryManager.hpp>
#include <time/TimeManager.hpp>
#include <arch/smp.hpp>

#include <libk/katomic.hpp>
#include <libk/kcstdio.hpp>

#define APIC_SPURIOUS_INTERRUPT (CPU::MAX_INTERRUPTS - 1)
#define APIC_IPI_INTERRUPT      (CPU::MAX_INTERRUPTS - 2)
#define APIC_ERROR_INTERRUPT    (CPU::MAX_INTERRUPTS - 3)

#define APIC_ENABLE (1 << 8)

#define APIC_DELIV_FIXED    0b000
#define APIC_DELIV_LOWEST   0b001
#define APIC_DELIV_SMI      0b010
#define APIC_DELIV_NMI      0b100
#define APIC_DELIV_INIT     0b101
#define APIC_DELIV_START_UP 0b110
#define APIC_DELIV_ExtINT   0b111

#define APIC_DEST_PHYSICAL 0
#define APIC_DEST_LOGICAL  1

#define APIC_STATUS_IDLE    0
#define APIC_STATUS_PENDING 1

#define APIC_LEVEL_DEASSERT 0
#define APIC_LEVEL_ASSERT   1

#define APIC_SHORTHAND_NONE     0b00
#define APIC_SHORTHAND_SELF     0b01
#define APIC_SHORTHAND_ALL_INCL 0b10
#define APIC_SHORTHAND_ALL_EXCL 0b11

#define APIC_ACTIVE_LOW  1
#define APIC_ACTIVE_HIGH 0

#define APIC_TRIGGER_EDGE  0
#define APIC_TRIGGER_LEVEL 1

#define APIC_REGISTER_SIZE 0x400

#define APIC_REG_ID              0x020
#define APIC_REG_EOI             0x0B0
#define APIC_REG_SPV             0x0F0
#define APIC_REG_LVT_CMCI        0x2F0
#define APIC_REG_ICR_LOW         0x300
#define APIC_REG_ICR_HIGH        0x310
#define APIC_REG_LVT_TIMER       0x320
#define APIC_REG_LVT_THERMAL     0x330
#define APIC_REG_LVT_PERFORMANCE 0x340
#define APIC_REG_LVT_LINT0       0x350
#define APIC_REG_LVT_LINT1       0x360
#define APIC_REG_LVT_ERROR       0x370

namespace Kernel::Interrupts
{
	class APICIPIInterruptHandler final : public InterruptHandler
	{
	public:
		APICIPIInterruptHandler(uint32_t interrupt_number)
		    : InterruptHandler(interrupt_number)
		{
		}

		virtual ~APICIPIInterruptHandler()
		{
		}

		virtual void handle_interrupt(const CPU::registers_t &reg __unused) override
		{
			LibK::printf_debug_msg("[APIC] Got IPI interrupt");
		}

		virtual void eoi()
		{
			LAPIC::instance().eoi();
		}

		virtual InterruptType type() const { return InterruptType::GenericInterrupt; }
	};

	class APICErrorInterruptHandler final : public InterruptHandler
	{
	public:
		APICErrorInterruptHandler(uint32_t interrupt_number)
		    : InterruptHandler(interrupt_number)
		{
		}

		virtual ~APICErrorInterruptHandler()
		{
		}

		virtual void handle_interrupt(const CPU::registers_t &reg __unused) override
		{
			LibK::printf_debug_msg("[APIC] Got error interrupt");
		}

		virtual void eoi()
		{
			LAPIC::instance().eoi();
		}

		virtual InterruptType type() const { return InterruptType::GenericInterrupt; }
	};

	// TODO: Move into separate class later
	class APICSpuriousInterruptHandler final : public InterruptHandler
	{
	public:
		APICSpuriousInterruptHandler(uint32_t interrupt_number)
		    : InterruptHandler(interrupt_number)
		{
		}

		virtual ~APICSpuriousInterruptHandler()
		{
		}

		virtual void handle_interrupt(const CPU::registers_t &reg __unused) override
		{
			LibK::printf_debug_msg("[APIC] Got spurious interrupt");
		}

		virtual void eoi()
		{
		}

		virtual InterruptType type() const { return InterruptType::SpuriousInterrupt; }
	};

	typedef union lvt_entry_t
	{
		struct
		{
			uint32_t vector_num : 8;
			uint32_t delivery_mode : 3;
			uint32_t reserved : 1;
			uint32_t delivery_status : 1;
			uint32_t polarity : 1;
			uint32_t irr_flag;
			uint32_t trigger_mode : 1;
			uint32_t mask : 1;
		} __packed;
		uint32_t value;
	} lvt_entry_t;

	typedef union icr_command_t
	{
		struct
		{
			uint64_t vector_num : 8;
			uint64_t delivery_mode : 3;
			uint64_t destination_mode : 1;
			uint64_t delivery_status : 1;
			uint64_t reserved1 : 1;
			uint64_t level : 1;
			uint64_t trigger_mode : 1;
			uint64_t reserved2 : 2;
			uint64_t shorthand : 2;
			uint64_t reserved3 : 36;
			uint64_t destination : 8;
		} __packed;
		struct
		{
			uint32_t low_dword;
			uint32_t high_dword;
		} __packed;
	} icr_command_t;

	static lvt_entry_t make_lvt_entry(uint8_t vector, uint8_t delivery_mode, uint8_t polarity, uint8_t trigger_mode, bool masked);

	static lvt_entry_t make_lvt_entry(uint8_t vector, uint8_t delivery_mode, uint8_t polarity, uint8_t trigger_mode, bool masked)
	{
		lvt_entry_t entry;

		entry.value = 0;
		entry.vector_num = vector;
		entry.delivery_mode = delivery_mode;
		entry.polarity = polarity;
		entry.trigger_mode = trigger_mode;
		entry.mask = masked;

		return entry;
	}

	void LAPIC::write_icr(uint8_t vector, uint8_t delivery_mode, uint8_t destination_mode, uint8_t shorthand, uint8_t destination)
	{
		// TODO handle INIT and Start-Up IPI correctly

		icr_command_t command;
		command.low_dword = 0;
		command.high_dword = 0;

		command.vector_num = vector;
		command.delivery_mode = delivery_mode;
		command.destination_mode = destination_mode;
		command.shorthand = shorthand;
		command.destination = destination;

		if (delivery_mode == APIC_DELIV_INIT || delivery_mode == APIC_DELIV_START_UP)
		{
			command.trigger_mode = APIC_TRIGGER_EDGE;
			command.level = APIC_LEVEL_ASSERT;
		}

		write_register(APIC_REG_ICR_HIGH, command.high_dword);
		write_register(APIC_REG_ICR_LOW, command.low_dword);
	}

	void LAPIC::initialize(uintptr_t physical_addr)
	{
		m_physical_addr = physical_addr;
		m_virtual_addr = (uintptr_t)Memory::VirtualMemoryManager::instance().map_physical(m_physical_addr, APIC_REGISTER_SIZE);

		m_ipi_interrupt_handler = new APICIPIInterruptHandler(APIC_IPI_INTERRUPT);
		m_error_interrupt_handler = new APICErrorInterruptHandler(APIC_ERROR_INTERRUPT);
		m_spurious_interrupt_handler = new APICSpuriousInterruptHandler(APIC_SPURIOUS_INTERRUPT);

		m_ipi_interrupt_handler->register_handler();
		m_error_interrupt_handler->register_handler();
		m_spurious_interrupt_handler->register_handler();

		write_register(APIC_REG_SPV, APIC_SPURIOUS_INTERRUPT);
		write_register(APIC_REG_LVT_CMCI, make_lvt_entry(0, 0, 0, 0, true).value);
		write_register(APIC_REG_LVT_TIMER, make_lvt_entry(0, 0, 0, 0, true).value);
		write_register(APIC_REG_LVT_THERMAL, make_lvt_entry(0, 0, 0, 0, true).value);
		write_register(APIC_REG_LVT_PERFORMANCE, make_lvt_entry(0, 0, 0, 0, true).value);
		write_register(APIC_REG_LVT_LINT0, make_lvt_entry(0, APIC_DELIV_NMI, 0, 0, true).value); // Generally NMI interrupt
		write_register(APIC_REG_LVT_LINT1, make_lvt_entry(0, 0, 0, 0, true).value);
		write_register(APIC_REG_LVT_ERROR, make_lvt_entry(APIC_ERROR_INTERRUPT, APIC_DELIV_FIXED, APIC_ACTIVE_LOW, APIC_TRIGGER_EDGE, false).value);
	}

	void LAPIC::enable()
	{
		write_register(APIC_REG_SPV, APIC_SPURIOUS_INTERRUPT | APIC_ENABLE);
	}

	void LAPIC::eoi()
	{
		write_register(APIC_REG_EOI, 0);
	}

	void LAPIC::start_smp_boot()
	{
		if (m_available_aps == 0)
			return;

		CPU::Processor::set_core_count(m_available_aps + 1);

		auto kernel_stack_addr = (uintptr_t)Memory::VirtualMemoryManager::instance().alloc_kernel_buffer(PAGE_SIZE * 8 * m_available_aps);

		LibK::vector<uintptr_t> kernel_stacks;
		for (size_t i = 0; i < m_available_aps; i++)
			kernel_stacks.push_back(kernel_stack_addr + PAGE_SIZE * 8 * (i + 1));

		auto cpu_id = LibK::atomic_uint32_t(0);
		auto do_continue = LibK::atomic_uint32_t(0);

		CPU::initialize_smp_boot_environment(kernel_stacks, cpu_id.raw_ptr(), do_continue.raw_ptr());

		write_icr(0, APIC_DELIV_INIT, APIC_DEST_PHYSICAL, APIC_SHORTHAND_ALL_EXCL, 0);

		Time::TimeManager::instance().sleep(10);

		for (int i = 0; i < 2; i++)
		{
			write_icr(0x08, APIC_DELIV_START_UP, APIC_DEST_PHYSICAL, APIC_SHORTHAND_ALL_EXCL, 0);
			Time::TimeManager::instance().usleep(200);
		}

		while(cpu_id < m_available_aps)
			Time::TimeManager::instance().usleep(200);

		do_continue = 1;

		CPU::finalize_smp_boot_environment();
	}

	[[nodiscard]] uint32_t LAPIC::get_ap_id()
	{
		uint32_t id = read_register(APIC_REG_ID) >> 24;
		return id;
	}

	void LAPIC::write_register(uintptr_t offset, uint32_t value)
	{
		assert(offset <= APIC_REGISTER_SIZE);
		auto volatile *apic_register = (uint32_t volatile *)(m_virtual_addr + offset);
		*apic_register = value;
	}

	uint32_t LAPIC::read_register(uintptr_t offset)
	{
		assert(offset <= APIC_REGISTER_SIZE);
		auto volatile *apic_register = (uint32_t volatile *)(m_virtual_addr + offset);
		return *apic_register;
	}
} // namespace Kernel::Interrupts
