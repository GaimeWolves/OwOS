#pragma once

#include <stdint.h>

#include <libk/kfunctional.hpp>
#include <libk/kshared_ptr.hpp>
#include <libk/kstack.hpp>
#include <libk/srmw_queue.hpp>

#include <arch/i686/interrupts.hpp>
#include <arch/i686/gdt.hpp>
#include <arch/smp.hpp>
#include <interrupts/InterruptHandler.hpp>
#include <time/EventManager.hpp>
#include <processes/definitions.hpp>
#include <processes/CoreScheduler.hpp>
#include <processes/GlobalScheduler.hpp>
#include <processes/Process.hpp>
#include <syscall/SyscallDispatcher.hpp>

namespace Kernel::CPU
{
	class PageFaultHandler;

	class Processor
	{
	private:
		friend SyscallDispatcher;
		friend CoreScheduler;
		friend GlobalScheduler;
		friend PageFaultHandler;
	public:
		[[nodiscard]] static Processor &current();
		static void set_core_count(uint32_t core_count);

		static void early_initialize(uint32_t id);
		static void initialize(uint32_t id);

		always_inline void enable_interrupts()
		{
			m_interrupts_enabled = true;
			if (!m_in_critical)
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
			m_in_critical++;
		}

		always_inline void leave_critical()
		{
			assert(m_in_critical > 0);
			m_in_critical--;
			if (m_interrupts_enabled && m_in_critical == 0)
				sti();
		}

		[[nodiscard]] always_inline bool in_critical() const { return m_in_critical > 0; }

		[[nodiscard]] Interrupts::InterruptHandler *get_interrupt_handler(uint32_t int_number) const { return m_handlers[int_number]; }
		void register_interrupt_handler(Interrupts::InterruptHandler &handler);
		void unregister_interrupt_handler(Interrupts::InterruptHandler &handler);
		[[nodiscard]] uint32_t get_used_interrupt_count() const;

		void increment_irq_counter() { m_irq_counter++; }
		void decrement_irq_counter() { m_irq_counter--; }
		[[nodiscard]] uint32_t get_irq_counter() const { return m_irq_counter; }

		always_inline LibK::stack<interrupt_frame_t *> &get_interrupt_frame_stack() { return m_frame_stack; }
		always_inline LibK::stack<LibK::function<void()>> &get_exit_function_stack() { return m_exit_function_stack; }

		always_inline void set_memory_space(Memory::memory_space_t *memory_space) { m_memory_space = memory_space; }
		[[nodiscard]] always_inline Memory::memory_space_t *get_memory_space() const {
			return m_memory_space;
		}

		void smp_initialize_messaging();
		void smp_poke();
		static void smp_poke_all(bool excluding_self);
		void smp_broadcast(LibK::shared_ptr<ProcessorMessage> message, bool excluding_self);
		void smp_enqueue_message(LibK::shared_ptr<ProcessorMessage> message);
		void smp_process_messages();

		void defer_call(LibK::function<void()> &&callback);
		void process_deferred_queue();

		Time::EventManager::EventQueue &get_event_queue() { return m_scheduled_events; }

		static thread_t *create_kernel_thread(uintptr_t main_function);
		static thread_t *create_userspace_thread(Memory::memory_space_t &memorySpace);
		static void initialize_userspace_thread(thread_t *thread, uintptr_t main_function, Memory::memory_space_t &memory_space);
		static thread_registers_t create_state_for_exec(uintptr_t main_function, uintptr_t userspace_stack_ptr, Memory::memory_space_t &memory_space);
		static uintptr_t thread_push_userspace_data(thread_t *thread, const char *data, size_t count);
		static uintptr_t thread_align_userspace_stack(thread_t *thread, uintptr_t alignment);

		template <typename T>
		static uintptr_t thread_push_userspace_data(thread_t *thread, T data)
		{
			current().enter_critical();
			uintptr_t esp = thread->has_started ? current().get_interrupt_frame_stack().top()->old_esp : thread->registers.esp;

			if (thread->has_started)
			{
				current().get_interrupt_frame_stack().top()->old_esp -= sizeof(T);
				esp = current().get_interrupt_frame_stack().top()->old_esp;
			}
			else
			{
				thread->registers.esp -= sizeof(T);
				esp = thread->registers.esp;
			}
			current().leave_critical();

			new (reinterpret_cast<void *>(esp)) T(data);
			return esp;
		}

		void enter_thread_context(thread_t &thread);
		void initial_enter_thread_context(thread_t thread);
		void update_thread_context(thread_t &thread);
		void enter_thread_after_exec(thread_t *thread, thread_registers_t registers);
		[[nodiscard]] bool is_scheduler_running() const { return m_scheduler_initialized; }
		[[nodiscard]] bool is_thread_running() const { return m_current_thread; }
		[[nodiscard]] thread_t *get_current_thread() const { return m_current_thread; }

		[[nodiscard]] static uint64_t get_nanoseconds_since_boot();
		void increment_nanoseconds_since_boot(uint64_t nanoseconds);
		always_inline void set_remaining_time_to_tick(uint64_t remaining_time_to_tick) { m_remaining_time_to_tick = remaining_time_to_tick; }
		[[nodiscard]] always_inline uint64_t get_remaining_time_to_tick() const { return m_remaining_time_to_tick; }
		always_inline void set_next_timer_tick(uint64_t next_timer_tick) { m_next_timer_tick = next_timer_tick; }
		[[nodiscard]] always_inline uint64_t get_next_timer_tick() const { return m_next_timer_tick; }

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

		always_inline static uint64_t read_tsc()
		{
			uint32_t eax;
			uint32_t edx;

			asm volatile("rdtsc" : "=a" (eax), "=d" (edx));

			return ((uint64_t)edx << 32 | (uint64_t)eax);
		}

	private:
		void init_gdt();
		void init_idt();
		void init_fault_handlers();

		void update_tss(uint32_t esp0);

		static thread_registers_t create_initial_state(uintptr_t stack_ptr, uintptr_t main_ptr, bool is_userspace_thread, Memory::Arch::paging_space_t paging_space);
		static void frame_set_initial_state(uintptr_t stack_ptr, uintptr_t main_ptr);

		always_inline static void sti()
		{
			asm volatile("sti");
		}

		always_inline static void cli()
		{
			asm volatile("cli");
		}

		always_inline static uint32_t eflags()
		{
			uintptr_t eflags = 0;

			asm volatile("pushf\n"
			             "pop %%eax"
			             : "=a"(eflags));

			return eflags;
		}

		uint32_t m_id{0};
		bool m_interrupts_enabled{false};
		uint32_t m_in_critical{0};
		LibK::atomic_uint32_t m_irq_counter{0};
		LibK::stack<interrupt_frame_t *>m_frame_stack{};
		LibK::stack<LibK::function<void()>> m_exit_function_stack{};

		Memory::memory_space_t *m_memory_space{nullptr};

		CPU::idt_entry_t m_idt[CPU::MAX_INTERRUPTS]{};
		CPU::idt_descriptor_t m_idtr{};
		Interrupts::InterruptHandler *m_handlers[CPU::MAX_INTERRUPTS]{};

		CPU::gdt_entry_t m_gdt[CPU::GDT_ENTRY_COUNT]{};
		CPU::gdt_descriptor_t m_gdtr{};
		CPU::tss_entry_t m_tss{};
		CPU::tss_entry_t m_abort_tss{};
		CPU::tss_entry_t m_page_fault_tss{};
		char *m_page_fault_stack{};

		// TODO: Implement an actual FIFO data structure
		LibK::SRMWQueue<LibK::shared_ptr<ProcessorMessage>> m_queued_messages{};
		Time::EventManager::EventQueue m_scheduled_events{};
		LibK::SRMWQueue<LibK::function<void()>> m_deferred_calls{};

		bool m_scheduler_initialized{false};
		uint64_t m_remaining_time_to_tick{};
		uint64_t m_next_timer_tick{};
		LibK::vector<thread_t *> m_running_threads{};
		thread_t *m_current_thread{nullptr};
		size_t m_current_thread_index{0};
		thread_t *m_idle_thread{nullptr};
		thread_t m_thread_enter_store{};
	};
}