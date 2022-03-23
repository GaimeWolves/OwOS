#pragma once

#include <stdint.h>

#include <libk/kfunctional.hpp>

#include <arch/i686/interrupts.hpp>
#include <arch/i686/gdt.hpp>
#include <arch/smp.hpp>

#include <interrupts/InterruptHandler.hpp>

namespace Kernel::CPU
{
	class Processor
	{
	public:
		[[nodiscard]] static Processor &current();
		static void set_core_count(uint32_t core_count);

		static void early_initialize(uint32_t id);
		static void initialize(uint32_t id);

		always_inline void enable_interrupts()
		{
			m_interrupts_enabled = true;
			sti();
		}

		always_inline void disable_interrupts()
		{
		    m_interrupts_enabled = false;
			cli();
		}

		always_inline void enter_critical()
		{
			cli();
			m_in_critical = true;
		}

		always_inline void leave_critical()
		{
			m_in_critical = false;
			if (m_interrupts_enabled)
				sti();
		}

		[[nodiscard]] always_inline bool in_critical() const { return m_in_critical; }

		[[nodiscard]] Interrupts::InterruptHandler *get_interrupt_handler(uint32_t int_number) const { return m_handlers[int_number]; }
		void register_interrupt_handler(Interrupts::InterruptHandler &handler);
		void unregister_interrupt_handler(Interrupts::InterruptHandler &handler);
		[[nodiscard]] uint32_t get_used_interrupt_count() const;

		void smp_initialize_messaging();
		void smp_poke();
		static void smp_poke_all(bool excluding_self);
		void smp_enqueue_message(ProcessorMessage *message);
		void smp_process_messages();

		[[nodiscard]] static uint32_t count();
		[[nodiscard]] uint32_t id() const { return m_id; }
		[[nodiscard]] static Processor &by_id(uint32_t id);
		static void enumerate(const LibK::function<bool(Processor &)> &callback);

		[[nodiscard]] always_inline static uintptr_t cr2()
		{
			uintptr_t cr2 = 0;

			asm volatile("mov %%cr2, %%eax"
			             : "=a"(cr2));

			return cr2;
		}
		__noreturn always_inline static void halt()
		{
			for (;;)
				asm volatile("cli\n"
				             "hlt");
		}

		always_inline static void sleep()
		{
			asm volatile("hlt");
		}

		always_inline static void pause()
		{
			asm volatile("pause");
		}

		always_inline static void load_page_directory(uintptr_t page_directory)
		{
			asm volatile("mov %%eax, %%cr3" ::"a"(page_directory)
			             : "memory");
		}

		[[nodiscard]] always_inline static uintptr_t get_page_directory()
		{
			uintptr_t page_directory = 0;

			asm volatile("mov %%cr3, %%eax"
			             : "=a"(page_directory));

			return page_directory;
		}

		always_inline static void flush_page_directory()
		{
			load_page_directory(get_page_directory());
		}

		always_inline static void invalidate_address(uintptr_t address)
		{
			asm volatile("invlpg (%0)" ::"r"(address)
			             : "memory");
		}

		always_inline uintptr_t get_gdtr_address() const { return (uintptr_t)&m_gdtr; }
		always_inline uintptr_t get_idtr_address() const { return (uintptr_t)&m_idtr; }

	private:
		void init_gdt();
		void init_idt();
		void init_fault_handlers();

		always_inline static void sti()
		{
			asm volatile("sti");
		}

		always_inline static void cli()
		{
			asm volatile("cli");
		}

		uint32_t m_id{0};
		bool m_interrupts_enabled{false};
		bool m_in_critical{false};

		CPU::idt_entry_t m_idt[CPU::MAX_INTERRUPTS]{};
		CPU::idt_descriptor_t m_idtr{};
		Interrupts::InterruptHandler *m_handlers[CPU::MAX_INTERRUPTS]{};

		CPU::gdt_entry_t m_gdt[CPU::GDT_ENTRY_COUNT]{};
		CPU::gdt_descriptor_t m_gdtr{};
		CPU::tss_entry_t m_tss{};

		// TODO: Implement an actual FIFO data structure
		//       Refcount ProcessorMessage
		LibK::vector<ProcessorMessage *> m_queued_messages{};
	};
}