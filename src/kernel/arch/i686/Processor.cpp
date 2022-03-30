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

		void handle_interrupt(const CPU::interrupt_frame_t &reg __unused) override
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

	static Processor s_bsp{}; // Preallocate BSP instance
	static Processor *s_aps;
	static uint32_t s_core_count = 1;
	static APICIPIInterruptHandler s_ipi_handler;

	Processor &Processor::current()
	{
		// TODO: Fix dependency on the LAPIC for the CPU id
		if (s_core_count == 1)
			return s_bsp;

		size_t id = Interrupts::LAPIC::instance().get_ap_id();
		return by_id(id);
	}

	void Processor::set_core_count(uint32_t core_count)
	{
		uint32_t old_count = s_core_count;
		s_core_count = core_count;

		if (!s_aps)
			s_aps = new Processor[s_core_count - 1];
		else
		{
			auto *new_aps = new Processor[s_core_count - 1];
			memmove((void *)new_aps, (void *)s_aps, (old_count - 1) * sizeof(Processor));
			s_aps = new_aps;
		}
	}

	void Processor::initialize(uint32_t id)
	{
		Processor &core = by_id(id);
		core.init_fault_handlers();
	}

	void Processor::early_initialize(uint32_t id)
	{
		Processor &core = by_id(id);
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
		if (id == 0)
			return s_bsp;

		return s_aps[id - 1];
	}

	void Processor::enumerate(const LibK::function<bool(Processor &)> &callback)
	{
		if (!callback(s_bsp))
			return;

		for (uint32_t i = 0; i < s_core_count; i++)
		{
			if (!callback(s_aps[i]))
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

	thread_registers_t Processor::create_initial_state(uintptr_t stack_ptr, uintptr_t main_ptr)
	{
		return {
		    .cs = 8,
		    .ss = 16,
		    .gs = 16,
		    .fs = 16,
		    .es = 16,
		    .ds = 16,
		    .edi = 0,
		    .esi = 0,
		    .ebp = 0,
		    .esp = stack_ptr,
		    .ebx = 0,
		    .edx = 0,
		    .ecx = 0,
		    .eax = 0,
		    .eflags = eflags(),
		    .cr3 = get_page_directory(),
		    .eip = main_ptr,
		    .frame = 0,
		};
	}

	thread_t Processor::create_kernel_thread(uintptr_t main_function)
	{
		uintptr_t stack = (uintptr_t)kmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE - sizeof(uintptr_t);

		return thread_t{
		    .registers = create_initial_state(stack, main_function),
		    .has_started = false,
		    .kernel_stack = stack,
		    .state = ThreadState::Waiting,
		    .lock = nullptr,
		};
	}

	void Processor::enter_thread_context(thread_t thread)
	{
		asm volatile(
		    "movl %[cr3], %%eax\n"
		    "movl %%eax, %%cr3\n"
		    "movl %[esp], %%esp\n"
		    "popl %%ss\n"
		    "popl %%gs\n"
		    "popl %%fs\n"
		    "popl %%es\n"
		    "popl %%ds\n"
		    "popa\n"
		    "addl $0x8, %%esp\n"
		    "iret\n"
		    :
		    : [esp] "m" (thread.registers.frame),
		      [cr3] "m" (thread.registers.cr3)
		    : "memory");
	}

	void Processor::initial_enter_thread_context(thread_t thread)
	{
		asm volatile(
		    "movl %[ds], %%eax\n"
		    "mov %%ax, %%ds\n"
		    "mov %%ax, %%es\n"
		    "mov %%ax, %%fs\n"
		    "mov %%ax, %%gs\n"
		    "movl %[ss], %%eax\n"
		    "cmp $16, %%eax\n"
		    "jnz 1f\n"
		    "mov %[esp], %%esp\n"
		    "1:\n"
		    "cmp $16, %%eax\n"
		    "jz 1f\n"
		    "pushl %[ss]\n"
		    "pushl %[esp]\n"
		    "1:\n"
		    "pushl %[flag]\n"
		    "pushl %[cs]\n"
		    "pushl %[eip]\n"
		    "movl %[cr3], %%eax\n"
		    "movl %%eax, %%cr3\n"
		    "movl %[eax], %%eax\n"
		    "movl %[ebx], %%ebx\n"
		    "movl %[ecx], %%ecx\n"
		    "movl %[edx], %%edx\n"
		    "movl %[esi], %%esi\n"
		    "movl %[edi], %%edi\n"
		    "movl %[ebp], %%ebp\n"
		    "iret\n"
		    :
		    : [ds] "m" (thread.registers.ds),
		      [ebp] "m" (thread.registers.ebp),
		      [ss] "m" (thread.registers.ss),
		      [esp] "m" (thread.registers.esp),
		      [flag] "m" (thread.registers.eflags),
		      [cs] "m" (thread.registers.cs),
		      [eip] "m" (thread.registers.eip),
		      [cr3] "m" (thread.registers.cr3),
		      [eax] "m" (thread.registers.eax),
		      [ebx] "m" (thread.registers.ebx),
		      [ecx] "m" (thread.registers.ecx),
		      [edx] "m" (thread.registers.edx),
		      [esi] "m" (thread.registers.esi),
		      [edi] "m" (thread.registers.edi)
		    : "memory");
	}

	void Processor::update_thread_context(thread_t &thread)
	{
		assert(m_current_frame);
		thread.registers.cs = m_current_frame->cs;
		thread.registers.ss = m_current_frame->ss;
		thread.registers.gs = m_current_frame->gs;
		thread.registers.fs = m_current_frame->fs;
		thread.registers.es = m_current_frame->es;
		thread.registers.ds = m_current_frame->ds;
		thread.registers.edi = m_current_frame->edi;
		thread.registers.esi = m_current_frame->esi;
		thread.registers.ebp = m_current_frame->ebp;
		thread.registers.esp = m_current_frame->esp;
		thread.registers.ebx = m_current_frame->ebx;
		thread.registers.edx = m_current_frame->edx;
		thread.registers.ecx = m_current_frame->ecx;
		thread.registers.eax = m_current_frame->eax;
		thread.registers.eflags = m_current_frame->eflags;
		thread.registers.cr3 = get_page_directory();
		thread.registers.eip = m_current_frame->eip;
		thread.registers.frame = (uintptr_t)m_current_frame;
	}
}