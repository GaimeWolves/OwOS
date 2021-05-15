#include <arch/processor.hpp>

#include <arch/i686/gdt.hpp>
#include <common_attributes.h>

namespace Kernel::Processor
{
	static gdt_entry_t gdt[5];
	static gdt_descriptor_t gdtr;

	void init()
	{
		gdt[0] = gdt_entry_t();
		gdt[1] = create_gdt_selector(false, true);
		gdt[2] = create_gdt_selector(false, false);
		gdt[3] = create_gdt_selector(true, true);
		gdt[4] = create_gdt_selector(true, false);

		gdtr.size = sizeof(gdt);
		gdtr.offset = (uint32_t)&gdt;

		asm volatile("lgdt %0" ::"m"(gdtr)
		             : "memory");
	}

	__noreturn void halt()
	{
		for (;;)
			asm volatile("cli\n"
			             "hlt");
	}

	void sleep()
	{
		asm volatile("hlt");
	}

	void clear_interrupts()
	{
		asm volatile("cli");
	}

	void enable_interrupts()
	{
		asm volatile("sti");
	}

	void load_page_directory(uintptr_t page_directory)
	{
		asm volatile("mov %%eax, %%cr3" ::"a"(page_directory)
		             : "memory");
	}

	uintptr_t get_page_directory()
	{
		uintptr_t page_directory = 0;

		asm volatile("mov %%cr3, %%eax"
		             : "=a"(page_directory));

		return page_directory;
	}

	void flush_page_directory()
	{
		load_page_directory(get_page_directory());
	}

	void invalidate_address(uintptr_t address)
	{
		asm volatile("invlpg (%0)" ::"r"(address)
		             : "memory");
	}
} // namespace Kernel::Processor