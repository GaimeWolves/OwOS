#include <arch/i686/Processor.hpp>

#include <libk/kcmalloc.hpp>
#include <libk/kcstdio.hpp>
#include <libk/kutility.hpp>

#include <interrupts/LAPIC.hpp>
#include <logging/logger.hpp>
#include <memory/VirtualMemoryManager.hpp>

#define APIC_IPI_INTERRUPT (CPU::MAX_INTERRUPTS - 3)

extern "C"
{
	extern uintptr_t _virtual_addr;
}

namespace Kernel::CPU
{
	static LibK::atomic_uint64_t s_nanoseconds_since_boot;

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
			// log("APIC", "Got IPI interrupt");
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

		for (uint32_t i = 0; i < s_core_count - 1; i++)
		{
			if (!callback(s_aps[i]))
				return;
		}
	}

	uint64_t Processor::get_nanoseconds_since_boot()
	{
		return s_nanoseconds_since_boot;
	}

	void Processor::increment_nanoseconds_since_boot(uint64_t increment)
	{
		assert(m_id == 0);
		s_nanoseconds_since_boot += increment;
	}

	void Processor::smp_initialize_messaging()
	{
		s_ipi_handler.register_handler();
	}

	void Processor::smp_poke()
	{
		if (Interrupts::LAPIC::instance().is_initialized())
			Interrupts::LAPIC::instance().send_ipi(APIC_IPI_INTERRUPT, m_id);
	}

	void Processor::smp_poke_all(bool excluding_self)
	{
		if (Interrupts::LAPIC::instance().is_initialized())
			Interrupts::LAPIC::instance().broadcast_ipi(APIC_IPI_INTERRUPT, excluding_self);
	}

	void Processor::smp_broadcast(LibK::shared_ptr<ProcessorMessage> message, bool excluding_self)
	{
		enumerate([&](Processor &processor) {
			if (!excluding_self || &processor != this)
				processor.smp_enqueue_message(message);

			return true;
		});

		smp_poke_all(excluding_self);
	}

	void Processor::smp_enqueue_message(LibK::shared_ptr<ProcessorMessage> message)
	{
		// log("SMP", "Enqueueing message on (#%d)", this->id());
		m_queued_messages.put(LibK::move(message));
	}

	void Processor::smp_process_messages()
	{
		// log("SMP", "Processing SMP messages");

		while (!m_queued_messages.empty())
		{
			m_queued_messages.get()->handle();
		}
	}

	void Processor::defer_call(LibK::function<void()> &&callback)
	{
		m_deferred_calls.put(move(callback));
	}

	void Processor::process_deferred_queue()
	{
		while (!m_deferred_calls.empty())
		{
			m_deferred_calls.get()();
		}
	}

	thread_registers_t Processor::create_initial_state(uintptr_t stack_ptr, uintptr_t main_ptr, bool is_userspace_thread, Memory::Arch::paging_space_t paging_space)
	{
		uint32_t cs = is_userspace_thread ? 0x18 : 0x08;
		uint32_t ds = is_userspace_thread ? 0x20 : 0x10;

		return {
		    .cs = cs,
		    .ss = ds,
		    .gs = ds,
		    .fs = ds,
		    .es = ds,
		    .ds = ds,
		    .edi = 0,
		    .esi = 0,
		    .ebp = 0,
		    .esp = stack_ptr,
		    .ebx = 0,
		    .edx = 0,
		    .ecx = 0,
		    .eax = 0,
		    .eflags = eflags() | 0x200,
		    .cr3 = paging_space.physical_pd_address,
		    .eip = main_ptr,
		    .frame = 0,
		};
	}

	void Processor::frame_set_initial_state(uintptr_t stack_ptr, uintptr_t main_ptr)
	{
		current().enter_critical();
		interrupt_frame_t *frame = current().get_interrupt_frame_stack().top();
		current().leave_critical();

		// frame->cs = 0x18 | 0x03,
		// frame->ss = 0x20 | 0x03,
		frame->gs = 0x20 | 0x03,
		frame->fs = 0x20 | 0x03,
		frame->es = 0x20 | 0x03,
		frame->ds = 0x20 | 0x03,
		frame->edi = 0,
		frame->esi = 0,
		frame->ebx = 0,
		frame->edx = 0,
		frame->ecx = 0,
		frame->eax = 0,
		frame->eip = main_ptr;
		frame->eflags = eflags() | 0x200;
		frame->old_esp = stack_ptr;
		frame->old_ss = 0x20 | 0x03;
	}

	thread_t *Processor::create_kernel_thread(uintptr_t main_function)
	{
		auto stack_region = Memory::VirtualMemoryManager::instance().allocate_region(KERNEL_STACK_SIZE);
		uintptr_t stack = stack_region.virt_address + KERNEL_STACK_SIZE - sizeof(uintptr_t);

		return new thread_t{
		    .registers = create_initial_state(stack, main_function, false, Memory::Arch::get_kernel_paging_space()),
		    .has_started = false,
		    .kernel_stack = stack,
		    .kernel_stack_region = stack_region,
		    .state = ThreadState::Ready,
		    .lock = nullptr,
		    .parent_process = nullptr,
		};
	}

	thread_t *Processor::create_userspace_thread(Memory::memory_space_t &memory_space)
	{
		Memory::mapping_config_t config;
		config.userspace = false;
		auto stack_region = Memory::VirtualMemoryManager::instance().allocate_region_at_for(&memory_space, (uintptr_t)&_virtual_addr - KERNEL_STACK_SIZE, KERNEL_STACK_SIZE, config);
		uintptr_t kernel_stack = stack_region.virt_address + KERNEL_STACK_SIZE - sizeof(uintptr_t);

		return new thread_t{
		    .registers = {},
		    .has_started = false,
		    .kernel_stack = kernel_stack,
		    .kernel_stack_region = stack_region,
		    .state = ThreadState::Ready,
		    .lock = nullptr,
		    .parent_process = nullptr,
		};
	}

	void Processor::initialize_userspace_thread(thread_t *thread, uintptr_t main_function, Memory::memory_space_t &memory_space)
	{
		Memory::mapping_config_t config;
		config.userspace = true;
		uintptr_t userspace_stack = (uintptr_t)Memory::VirtualMemoryManager::instance().allocate_region_at_for(&memory_space, 0xb0000000, KERNEL_STACK_SIZE, config).virt_address + KERNEL_STACK_SIZE - sizeof(uintptr_t);

		thread->registers.cs = 0x18 | 0x03;
		thread->registers.ss = 0x20 | 0x03;
		thread->registers.gs = 0x20 | 0x03;
		thread->registers.fs = 0x20 | 0x03;
		thread->registers.es = 0x20 | 0x03;
		thread->registers.ds = 0x20 | 0x03;
		thread->registers.edi = 0;
		thread->registers.esi = 0;
		thread->registers.ebp = 0;
		thread->registers.esp = userspace_stack;
		thread->registers.ebx = 0;
		thread->registers.edx = 0;
		thread->registers.ecx = 0;
		thread->registers.eax = 0;
		thread->registers.eflags = eflags() | 0x200;
		thread->registers.cr3 = memory_space.paging_space.physical_pd_address;
		thread->registers.eip = main_function;

		if (!current().m_frame_stack.empty())
			frame_set_initial_state(userspace_stack, main_function);
	}

	thread_registers_t Processor::create_state_for_exec(uintptr_t main_function, uintptr_t userspace_stack_ptr, Memory::memory_space_t &memory_space)
	{
		thread_registers_t registers;

		registers.cs = 0x18 | 0x03;
		registers.ss = 0x20 | 0x03;
		registers.gs = 0x20 | 0x03;
		registers.fs = 0x20 | 0x03;
		registers.es = 0x20 | 0x03;
		registers.ds = 0x20 | 0x03;
		registers.edi = 0;
		registers.esi = 0;
		registers.ebp = 0;
		registers.esp = userspace_stack_ptr;
		registers.ebx = 0;
		registers.edx = 0;
		registers.ecx = 0;
		registers.eax = 0;
		registers.eflags = eflags() | 0x200;
		registers.cr3 = memory_space.paging_space.physical_pd_address;
		registers.eip = main_function;

		return registers;
	}

	uintptr_t Processor::thread_push_userspace_data(thread_t *thread, const char *data, size_t count)
	{
		current().enter_critical();
		uintptr_t esp = thread->has_started ? current().get_interrupt_frame_stack().top()->old_esp : thread->registers.esp;

		if (thread->has_started)
		{
			current().get_interrupt_frame_stack().top()->old_esp -= count;
			esp = current().get_interrupt_frame_stack().top()->old_esp;
		}
		else
		{
			thread->registers.esp -= count;
			esp = thread->registers.esp;
		}
		current().leave_critical();

		memcpy(reinterpret_cast<char *>(esp), data, count);
		return esp;
	}

	void Processor::enter_thread_context(thread_t &thread)
	{
		uint32_t esp0 = (uint32_t)thread.registers.frame + sizeof(interrupt_frame_t);

		// Adjust in case we are not entering ring 3
		if (thread.registers.cs == 0x10)
			esp0 -= 8;

		update_tss(esp0);

		uintptr_t old_cr3 = get_page_directory();

		assert(thread.kernel_stack_region.virt_region().contains((uintptr_t)thread.registers.frame));

		asm volatile(
		    "cmpl %[old_cr3], %[new_cr3]\n"
		    "jz 1f\n"
		    "movl %[new_cr3], %%cr3\n"
		    "1:\n"
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
		    : [esp] "d"((uintptr_t)thread.registers.frame),
		      [new_cr3] "a"(thread.registers.cr3),
		      [old_cr3] "b"(old_cr3)
		    : "memory");
	}

	void Processor::initial_enter_thread_context(thread_t thread)
	{
		update_tss(thread.kernel_stack);

		asm volatile(
		    "movl %[ds], %%eax\n"
		    "cmpl $0x20, %%eax\n"
		    "jnz 1f\n"
		    "or $3, %%eax\n"
		    "1:\n"
		    "mov %%ax, %%ds\n"
		    "mov %%ax, %%es\n"
		    "mov %%ax, %%fs\n"
		    "mov %%ax, %%gs\n"
		    "movl %[ss], %%eax\n"
		    "cmp $0x10, %%eax\n"
		    "jnz 1f\n"
		    "mov %[esp], %%esp\n"
		    "1:\n"
		    "cmp $16, %%eax\n"
		    "jz 2f\n"
		    "movl %[ss], %%eax\n"
		    "cmpl $0x20, %%eax\n"
		    "jnz 1f\n"
		    "or $3, %%eax\n"
		    "1:"
		    "pushl %%eax\n"
		    "pushl %[esp]\n"
		    "2:\n"
		    "pushl %[flag]\n"
		    "movl %[cs], %%eax\n"
		    "cmpl $0x18, %%eax\n"
		    "jnz 1f\n"
		    "or $3, %%eax\n"
		    "1:"
		    "pushl %%eax\n"
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
		    : [ds] "m"(thread.registers.ds),
		      [ebp] "m"(thread.registers.ebp),
		      [ss] "m"(thread.registers.ss),
		      [esp] "m"(thread.registers.esp),
		      [flag] "m"(thread.registers.eflags),
		      [cs] "m"(thread.registers.cs),
		      [eip] "m"(thread.registers.eip),
		      [cr3] "m"(thread.registers.cr3),
		      [eax] "m"(thread.registers.eax),
		      [ebx] "m"(thread.registers.ebx),
		      [ecx] "m"(thread.registers.ecx),
		      [edx] "m"(thread.registers.edx),
		      [esi] "m"(thread.registers.esi),
		      [edi] "m"(thread.registers.edi)
		    : "memory");
	}

	void Processor::enter_thread_after_exec(thread_t *thread, thread_registers_t registers)
	{
		cli(); // NOTE: gets set again by registers.eflags through iret
		update_tss(thread->kernel_stack);

		asm volatile(
		    "movl %[ds], %%eax\n"
		    "mov %%ax, %%ds\n"
		    "mov %%ax, %%es\n"
		    "mov %%ax, %%fs\n"
		    "mov %%ax, %%gs\n"
		    "pushl %[ss]\n"
		    "pushl %[esp]\n"
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
		    : [ds] "m"(registers.ds),
		      [ebp] "m"(registers.ebp),
		      [ss] "m"(registers.ss),
		      [esp] "m"(registers.esp),
		      [flag] "m"(registers.eflags),
		      [cs] "m"(registers.cs),
		      [eip] "m"(registers.eip),
		      [cr3] "m"(registers.cr3),
		      [eax] "m"(registers.eax),
		      [ebx] "m"(registers.ebx),
		      [ecx] "m"(registers.ecx),
		      [edx] "m"(registers.edx),
		      [esi] "m"(registers.esi),
		      [edi] "m"(registers.edi)
		    : "memory");
	}

	void Processor::update_thread_context(thread_t &thread)
	{
		assert(!m_frame_stack.empty());
		auto frame = m_frame_stack.top();
		thread.registers.cs = frame->cs;
		thread.registers.ss = frame->ss;
		thread.registers.gs = frame->gs;
		thread.registers.fs = frame->fs;
		thread.registers.es = frame->es;
		thread.registers.ds = frame->ds;
		thread.registers.edi = frame->edi;
		thread.registers.esi = frame->esi;
		thread.registers.ebp = frame->ebp;
		thread.registers.esp = frame->esp;
		thread.registers.ebx = frame->ebx;
		thread.registers.edx = frame->edx;
		thread.registers.ecx = frame->ecx;
		thread.registers.eax = frame->eax;
		thread.registers.eflags = frame->eflags;
		thread.registers.cr3 = get_page_directory();
		thread.registers.eip = frame->eip;
		thread.registers.frame = frame;
	}

#define PRINT_REGISTER(name)                                 \
	critical_printf(" %10s ", #name);                        \
	Processor::enumerate([](Processor &core) {               \
		if (core.get_interrupt_frame_stack().empty())        \
			return true;                                     \
		auto frame = core.get_interrupt_frame_stack().top(); \
		critical_printf("| 0x%.8x ", frame->name);           \
		return true;                                         \
	});                                                      \
	critical_putc('\n');

	void print_registers()
	{
		critical_printf("            ");

		Processor::enumerate([](Processor &core) {
			critical_printf("| Core %5d ", core.id());
			return true;
		});

		critical_putc('\n');

		critical_printf("------------");

		Processor::enumerate([](Processor &) {
			critical_printf("-------------");
			return true;
		});

		critical_putc('\n');

		PRINT_REGISTER(ss)
		PRINT_REGISTER(gs)
		PRINT_REGISTER(fs)
		PRINT_REGISTER(es)
		PRINT_REGISTER(ds)
		PRINT_REGISTER(edi)
		PRINT_REGISTER(esi)
		PRINT_REGISTER(ebp)
		PRINT_REGISTER(esp)
		PRINT_REGISTER(eax)
		PRINT_REGISTER(ebx)
		PRINT_REGISTER(ecx)
		PRINT_REGISTER(edx)
		PRINT_REGISTER(isr_number)
		PRINT_REGISTER(error_code)
		PRINT_REGISTER(eip)
		PRINT_REGISTER(cs)
		PRINT_REGISTER(eflags)
		PRINT_REGISTER(old_esp)
		PRINT_REGISTER(old_ss)

		critical_printf("    old_cr3 ");

		Processor::enumerate([](Processor &core) {
			critical_printf("| 0x%.8x ", core.get_memory_space()->paging_space.physical_pd_address);
			return true;
		});

		critical_putc('\n');
	}
} // namespace Kernel::CPU