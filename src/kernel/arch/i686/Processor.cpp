#include <arch/i686/Processor.hpp>

#include <libk/kcmalloc.hpp>

#include <interrupts/LAPIC.hpp>

namespace Kernel::CPU
{
	static Processor *s_cores;
	static uint32_t s_core_count = 1;

	[[nodiscard]] Processor &Processor::current()
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

	[[nodiscard]] Processor &Processor::by_id(uint32_t id) const
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
}