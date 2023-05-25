#include <arch/i686/Processor.hpp>

#include <libk/kcmalloc.hpp>
#include <libk/kcstdio.hpp>
#include <libk/kutility.hpp>

#include <interrupts/LAPIC.hpp>
#include <logging/logger.hpp>
#include <memory/VirtualMemoryManager.hpp>

#define APIC_IPI_INTERRUPT (CPU::MAX_INTERRUPTS - 3)

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
		    .eflags = eflags(),
		    .cr3 = paging_space.physical_pd_address,
		    .eip = main_ptr,
		    .frame = 0,
		};
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

	thread_t *Processor::create_userspace_thread(uintptr_t main_function, Memory::memory_space_t &memory_space)
	{
		auto stack_region = Memory::VirtualMemoryManager::instance().allocate_region(KERNEL_STACK_SIZE);
		uintptr_t kernel_stack = stack_region.virt_address + KERNEL_STACK_SIZE - sizeof(uintptr_t);

		auto old_memory_space = CPU::Processor::current().get_memory_space();
		Memory::VirtualMemoryManager::load_memory_space(&memory_space);

		// TODO: Do this in the process/thread spawning code
		Memory::mapping_config_t config;
		config.userspace = true;
		uintptr_t userspace_stack = (uintptr_t)Memory::VirtualMemoryManager::instance().allocate_region_at(0xb0000000, KERNEL_STACK_SIZE, config).virt_address + KERNEL_STACK_SIZE - sizeof(uintptr_t);

		Memory::VirtualMemoryManager::load_memory_space(old_memory_space);

		return new thread_t{
		    .registers = create_initial_state(userspace_stack, main_function, true, memory_space.paging_space),
		    .has_started = false,
		    .kernel_stack = kernel_stack,
		    .kernel_stack_region = stack_region,
		    .state = ThreadState::Ready,
		    .lock = nullptr,
		    .parent_process = nullptr,
		};
	}

	uintptr_t Processor::thread_push_userspace_data(thread_t *thread, const char *data, size_t count)
	{
		thread->registers.esp -= count;
		memcpy(reinterpret_cast<char *>(thread->registers.esp), data, count);
		return thread->registers.esp;
	}

	void Processor::enter_thread_context(thread_t &thread)
	{
		uint32_t esp0 = (uint32_t)thread.registers.frame + sizeof(interrupt_frame_t);

		// Adjust in case we are not entering ring 3
		if (thread.registers.cs == 0x10)
			esp0 -= 8;

		update_tss(esp0);

		uintptr_t old_cr3 = get_page_directory();

		/*
		log("DBG", "RETURNING FRAME at %p for THREAD %p at TIME %lld:\nss: 0x%.2x\ngs: 0x%.2x\nfs: 0x%.2x\nes: 0x%.2x\nds: 0x%.2x\nedi: %p\nesi: %p\nebp: %p\nesp: %p\nebx: %p\nedx: %p\necx: %p\neax: %p\nisr: 0x%.2x\nerr: %p\neip: %p\ncs: 0x%.2x\neflags: %p\nold_esp: %p\nold_ss: %p\nold_cr3: %p\nnew_cr3: %p\nstack: %p - %p",
		    thread.registers.frame,
		    &thread,
		    read_tsc(),
		    thread.registers.frame->ss,
		    thread.registers.frame->gs,
		    thread.registers.frame->fs,
		    thread.registers.frame->es,
		    thread.registers.frame->ds,
		    thread.registers.frame->edi,
		    thread.registers.frame->esi,
		    thread.registers.frame->ebp,
		    thread.registers.frame->esp,
		    thread.registers.frame->ebx,
		    thread.registers.frame->edx,
		    thread.registers.frame->ecx,
		    thread.registers.frame->eax,
		    thread.registers.frame->isr_number,
		    thread.registers.frame->error_code,
		    thread.registers.frame->eip,
		    thread.registers.frame->cs,
		    thread.registers.frame->eflags,
		    thread.registers.frame->old_esp,
		    thread.registers.frame->old_ss,
		    old_cr3,
		    thread.registers.cr3,
		    thread.kernel_stack_region.virt_address,
		    thread.kernel_stack_region.virt_address + KERNEL_STACK_SIZE
		);
		 */

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