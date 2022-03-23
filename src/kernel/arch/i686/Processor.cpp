#include <arch/i686/Processor.hpp>

#include <libk/kcmalloc.hpp>
#include <libk/kutility.hpp>
#include <libk/kcstdio.hpp>

#include <interrupts/LAPIC.hpp>

#define APIC_IPI_INTERRUPT (CPU::MAX_INTERRUPTS - 3)

namespace Kernel::CPU
{
	class APICIPIInterruptHandler final : public Interrupts::InterruptHandler
	{
	public:
		APICIPIInterruptHandler()
		    : InterruptHandler(APIC_IPI_INTERRUPT)
		{
		}

		~APICIPIInterruptHandler() override = default;

		void handle_interrupt(const CPU::registers_t &reg __unused) override
		{
			LibK::printf_debug_msg("[APIC] Got IPI interrupt");
			Processor::current().smp_process_messages();
		}

		void eoi() override
		{
			Interrupts::LAPIC::instance().eoi();
		}

		Interrupts::InterruptType type() const override { return Interrupts::InterruptType::GenericInterrupt; }
	};

	static Processor *s_cores;
	static uint32_t s_core_count = 1;
	static APICIPIInterruptHandler s_ipi_handler;

	Processor &Processor::current()
	{
		// TODO: Fix dependency on the LAPIC for the CPU id
		if (s_core_count == 1)
			return s_cores[0];

		return s_cores[Interrupts::LAPIC::instance().get_ap_id()];
	}

	void Processor::set_core_count(uint32_t core_count)
	{
		s_core_count = core_count;

		if (!s_cores)
			s_cores = static_cast<Processor *>(kmalloc(s_core_count * sizeof(Processor)));
		else
			s_cores = static_cast<Processor *>(krealloc(s_cores, s_core_count * sizeof(Processor)));
	}

	void Processor::initialize(uint32_t id)
	{
		Processor &core = s_cores[id];
		core.init_fault_handlers();
	}

	void Processor::early_initialize(uint32_t id)
	{
		if (!s_cores)
			set_core_count(1);

		s_cores[id] = Processor();

		Processor &core = s_cores[id];
		core.m_id = id;
		core.init_gdt();
		core.init_idt();
	}

	uint32_t Processor::count()
	{
		return s_core_count;
	}

	Processor &Processor::by_id(uint32_t id)
	{
		return s_cores[id];
	}

	void Processor::enumerate(const LibK::function<bool(Processor &)> &callback)
	{
		for (uint32_t i = 0; i < s_core_count; i++)
		{
			if (!callback(s_cores[i]))
				return;
		}
	}

	void Processor::smp_initialize_messaging()
	{
		s_ipi_handler.register_handler();
	}

	void Processor::smp_poke()
	{
		Interrupts::LAPIC::instance().send_ipi(APIC_IPI_INTERRUPT, m_id);
	}

	void Processor::smp_poke_all(bool excluding_self)
	{
		Interrupts::LAPIC::instance().broadcast_ipi(APIC_IPI_INTERRUPT, excluding_self);
	}

	void Processor::smp_enqueue_message(ProcessorMessage *message)
	{
		m_queued_messages.push_back(message);
	}

	void Processor::smp_process_messages()
	{
		LibK::printf_debug_msg("[SMP] Processing SMP messages");

		while (!m_queued_messages.empty()) {
			m_queued_messages.back()->handle();
			m_queued_messages.pop_back();
		}
	}
}