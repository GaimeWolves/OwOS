#include <arch/i686/gdt.hpp>

#include <arch/i686/Processor.hpp>

extern "C" void isr_0x08_entry();
extern "C" void isr_0x0E_entry();

namespace Kernel::CPU
{
	static char *abort_stack[PAGE_SIZE];

	always_inline static gdt_entry_t create_gdt_selector(uint8_t privilege, bool is_code)
	{
		return gdt_entry_t{
		    .limit_low = 0xFFFF,
		    .base_low = 0,
		    .accessed = 0,
		    .read_write = 1,
		    .direction_conforming = 0,
		    .executable = is_code,
		    .type = 1,
		    .privilege = privilege,
		    .present = 1,
		    .limit_high = 0x0F,
		    .size = 1,
		    .granularity = 1,
		    .base_high = 0,
		};
	}

	always_inline static gdt_entry_t create_tss(uint32_t base, uint32_t limit)
	{
		return gdt_entry_t{
		    .limit_low = limit & 0xFFFF,
		    .base_low = base & 0xFFFFFF,
		    .accessed = 1,
		    .read_write = 0,
		    .direction_conforming = 0,
		    .executable = 1,
		    .type = 0,
		    .privilege = 0,
		    .present = 1,
		    .limit_high = (limit >> 16) & 0x0F,
		    .size = 0,
		    .granularity = 0,
		    .base_high = (base >> 24) & 0xFF,
		};
	}

	void Processor::init_gdt()
	{
		memset(&m_tss, 0, sizeof(tss_entry_t));
		m_tss.ss0 = 0x10;
		m_tss.iopb = sizeof(tss_entry_t);

		m_gdt[0] = gdt_entry_t();
		m_gdt[1] = create_gdt_selector(0, true);
		m_gdt[2] = create_gdt_selector(0, false);
		m_gdt[3] = create_gdt_selector(3, true);
		m_gdt[4] = create_gdt_selector(3, false);
		m_gdt[5] = create_tss((uint32_t)&m_tss, sizeof(tss_entry_t));
		m_gdt[6] = create_tss((uint32_t)&m_abort_tss, sizeof(tss_entry_t));
		m_gdt[7] = create_tss((uint32_t)&m_page_fault_tss, sizeof(tss_entry_t));

		m_gdtr.size = sizeof(m_gdt);
		m_gdtr.offset = (uint32_t)&m_gdt;

		asm volatile("lgdt %0" ::"m"(m_gdtr)
		             : "memory");

		asm volatile(
		    "ltr %%ax"
		    : : "a" (0x28)
		    :);

		asm volatile(
		    "push $0x08\n"
		    "lea %=f, %%eax\n"
		    "push %%eax\n"
		    "retf\n"
		    "%=:\n"
		    "mov $0x10, %%ax\n"
		    "mov %%ax, %%ss\n"
		    "mov %%ax, %%ds\n"
		    "mov %%ax, %%es\n"
		    "mov %%ax, %%fs\n"
		    "mov %%ax, %%gs\n"
		    : : "a"(0) : );

		memset(&m_abort_tss, 0, sizeof(tss_entry_t));
		m_abort_tss.cs = 0x08;
		m_abort_tss.ss = 0x10;
		m_abort_tss.ss0 = 0x10;
		m_abort_tss.ds = 0x08;
		m_abort_tss.es = 0x08;
		m_abort_tss.fs = 0x08;
		m_abort_tss.gs = 0x08;
		m_abort_tss.esp = reinterpret_cast<uint32_t>(abort_stack + PAGE_SIZE - sizeof(uintptr_t));
		m_abort_tss.ebp = m_abort_tss.esp;
		m_abort_tss.eflags = Processor::eflags();
		m_abort_tss.eip = reinterpret_cast<uint32_t>(isr_0x08_entry);
		m_abort_tss.cr3 = Processor::get_page_directory();
		m_abort_tss.iopb = sizeof(tss_entry_t);

		memcpy(&m_page_fault_tss, &m_abort_tss, sizeof(tss_entry_t));
		m_page_fault_tss.esp = reinterpret_cast<uint32_t>(m_page_fault_stack + PAGE_SIZE - sizeof(uintptr_t));
		m_page_fault_tss.eip = reinterpret_cast<uint32_t>(isr_0x0E_entry);
		m_page_fault_tss.ebp = m_page_fault_tss.esp;
	}

	void Processor::update_tss(uint32_t esp0)
	{
		m_tss.esp0 = esp0;
	}
}
