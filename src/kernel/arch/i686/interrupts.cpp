#include <arch/interrupts.hpp>

#include "interrupt_stubs.hpp"
#include "logging/logger.hpp"

#include <arch/Processor.hpp>
#include <arch/i686/interrupts.hpp>
#include <arch/stack_tracing.hpp>
#include <common_attributes.h>
#include <interrupts/SharedIRQHandler.hpp>
#include <interrupts/UnhandledInterruptHandler.hpp>
#include <syscall/SyscallDispatcher.hpp>

#include <libk/kcstdio.hpp>
#include <libk/kstring.hpp>

#include "../../userland/libc/signal.h"

#define INIT_INTERRUPT(i)                                                             \
	do                                                                                \
	{                                                                                 \
		m_idt[(i)] = create_idt_entry(isr_##i##_entry, IDTEntryType::INTERRUPT_GATE); \
		m_handlers[(i)] = nullptr;                                                    \
	} while (0)

#define INIT_TRAP(i)                                                             \
	do                                                                           \
	{                                                                            \
		m_idt[(i)] = create_idt_entry(isr_##i##_entry, IDTEntryType::TRAP_GATE); \
		m_handlers[(i)] = nullptr;                                               \
	} while (0)

#define INIT_TASK(i)                                                             \
	do                                                                           \
	{                                                                            \
		m_idt[(i)] = create_idt_entry(isr_##i##_entry, IDTEntryType::TASK_GATE); \
		m_handlers[(i)] = nullptr;                                               \
	} while (0)

#define PRINT_REGISTER(tag, name, var) log(tag, #var ": 0x%.8x", name.var)
#define PRINT_REGISTERS(tag, name)      \
	PRINT_REGISTER(tag, name, eax);     \
	PRINT_REGISTER(tag, name, ebx);     \
	PRINT_REGISTER(tag, name, ecx);     \
	PRINT_REGISTER(tag, name, edx);     \
	PRINT_REGISTER(tag, name, esi);     \
	PRINT_REGISTER(tag, name, edi);     \
	PRINT_REGISTER(tag, name, ebp);     \
	PRINT_REGISTER(tag, name, old_esp); \
	PRINT_REGISTER(tag, name, eip)

extern "C"
{
	extern uintptr_t _virtual_addr;
}

namespace Kernel::CPU
{
	bool in_kernel_space(uintptr_t virt_address)
	{
		return virt_address >= reinterpret_cast<uintptr_t>(&_virtual_addr);
	}

	// TODO: move this out of architecture-specific code

	class ExceptionHandler : public Interrupts::InterruptHandler
	{
	public:
		explicit ExceptionHandler(uint32_t interrupt_number, int8_t signal_number)
		    : InterruptHandler(interrupt_number)
		    , m_interrupt_number(interrupt_number)
		    , m_signal_number(signal_number)
		{
		}

		void handle_interrupt(const CPU::interrupt_frame_t &reg) override
		{
			auto &core = Processor::current();
			if (!core.get_current_thread() || in_kernel_space(reg.eip))
				handle_kernel_exception(reg);
			else
				handle_userspace_exception(reg);
		}

		virtual void handle_kernel_exception(const CPU::interrupt_frame_t &reg __unused)
		{
			halt_aps();

			critical_empty_logger();
			critical_printf("CRITICAL ERROR: received exception 0x%.2x in kernel space\n", m_interrupt_number);

			print_stacktrace(Processor::current().m_tss.ebp);

			Processor::halt();
		}

		virtual void handle_userspace_exception(const CPU::interrupt_frame_t &reg __unused)
		{
			Processor::current().get_current_thread()->parent_process->exit(0, m_signal_number);
			CoreScheduler::yield();
		}

		void eoi() override {}

		[[nodiscard]] Interrupts::InterruptType type() const override { return Interrupts::InterruptType::GenericInterrupt; }

	private:
		uint32_t m_interrupt_number{0};
		int8_t m_signal_number{0};
	};

	class PageFaultHandler final : public ExceptionHandler
	{
	public:
		explicit PageFaultHandler()
		    : ExceptionHandler(0x0E, SIGSEGV)
		{
		}

		void handle_kernel_exception(const CPU::interrupt_frame_t &reg) override
		{
			uintptr_t address = Processor::cr2();

			halt_aps();

			critical_empty_logger();
			critical_printf("CRITICAL ERROR: Got page fault at address %p executing %p\n", address, Processor::current().m_tss.eip);
			critical_printf("CRITICAL ERROR: With error code %i\n", reg.error_code);

			print_stacktrace(Processor::current().m_tss.ebp);

			Processor::halt();
		}

		void handle_userspace_exception(const CPU::interrupt_frame_t &reg __unused) override
		{
			auto process = Processor::current().get_current_thread()->parent_process;
			process->exit(0, SIGSEGV);

			// TODO: handle page faults correctly

			log("FAULT", "Segmentation fault for PID %d:", process->get_pid());
			PRINT_REGISTERS("FAULT", reg);

			CoreScheduler::yield();
		}
	};

	class SyscallHandler final : public Interrupts::InterruptHandler
	{
	public:
		explicit SyscallHandler()
		    : InterruptHandler(0x80)
		{
		}

		~SyscallHandler() override = default;

		void handle_interrupt(const CPU::interrupt_frame_t &reg) override
		{
			uint32_t ret_code = SyscallDispatcher::handle_syscall(reg.eax, reg.ebx, reg.ecx, reg.edx, reg.edi, reg.esi);
			Processor::current().get_interrupt_frame_stack().top()->set_return_value(ret_code);
		}

		void eoi() override {}

		[[nodiscard]] Interrupts::InterruptType type() const override { return Interrupts::InterruptType::GenericInterrupt; }
	};

	// TODO: ugly way of doing this
	static ExceptionHandler division_fault_handler = ExceptionHandler(0x00, SIGFPE);
	static ExceptionHandler invalid_instruction_handler = ExceptionHandler(0x06, SIGILL);
	static ExceptionHandler gpf_fault_handler = ExceptionHandler(0x0D, SIGILL);
	static PageFaultHandler page_fault_handler = PageFaultHandler();
	static SyscallHandler syscall_handler = SyscallHandler();

	static idt_entry_t create_idt_entry(void (*entry)(), IDTEntryType type);

	extern "C" void common_interrupt_handler_entry();
	extern "C" void common_interrupt_handler(interrupt_frame_t *regs);

	static __noreturn void unhandled_interrupt_handler(interrupt_frame_t *regs);

	static idt_entry_t create_idt_entry(void (*entry)(), IDTEntryType type)
	{
		static_assert(sizeof(size_t) == sizeof(uint32_t));
		size_t address = (size_t)entry;
		uint16_t segment = 0x08;

		if (type == IDTEntryType::TASK_GATE)
		{
			address = 0;
			segment = entry == isr_0x08_entry ? 0x30 : 0x38;
		}

		return idt_entry_t{
		    .offset_low = (uint16_t)(address & 0xFFFF),
		    .segment = segment,
		    .type = (uint8_t)type,
		    .privilege = 0,
		    .present = 1,
		    .offset_high = (uint16_t)(address >> 16),
		};
	}

	extern "C" __naked __noreturn void common_interrupt_handler_entry()
	{
		asm(
		    "pusha\n"
		    "pushl %ds\n"
		    "pushl %es\n"
		    "pushl %fs\n"
		    "pushl %gs\n"
		    "pushl %ss\n"
		    "mov $0x10, %ax\n"
		    "mov %ax, %ds\n"
		    "mov %ax, %es\n"
		    "mov %ax, %fs\n"
		    "mov %ax, %gs\n"
		    "pushl %esp\n"
		    "cld\n"
		    "call common_interrupt_handler\n"
		    "popl %esp\n"
		    "popl %ss\n"
		    "popl %gs\n"
		    "popl %fs\n"
		    "popl %es\n"
		    "popl %ds\n"
		    "popa\n"
		    "addl $0x8, %esp\n"
		    "iret\n");
	}

	extern "C" void common_interrupt_handler(interrupt_frame_t *regs)
	{
		if (regs->isr_number == 0x08)
		{
			halt_aps();

			critical_empty_logger();
			critical_printf("CRITICAL ERROR: Double fault\n");

			Processor::halt();
		}

		Processor &core = Processor::current();
		if (regs->isr_number != 0x80)
			core.increment_irq_counter();

		core.enter_critical();
		core.get_exit_function_stack().push([]() {});
		core.get_interrupt_frame_stack().push(regs);
		core.leave_critical();
		auto handler = core.get_interrupt_handler(regs->isr_number);

		if (!handler)
			unhandled_interrupt_handler(regs);

		Memory::Arch::check_page_directory();

		handler->handle_interrupt(*regs);
		core.enter_critical();
		core.get_interrupt_frame_stack().pop();
		auto exit = core.get_exit_function_stack().top();
		core.get_exit_function_stack().pop();
		core.leave_critical();

		if (regs->isr_number != 0x80)
			core.decrement_irq_counter();

		asm("cli");
		handler->eoi();
		exit();
	}

	static __noreturn void unhandled_interrupt_handler(interrupt_frame_t *regs)
	{
		// TODO: For now we halt on every unhandled interrupt.
		//       This is probably undesired for anything other than faults. (i.e. interrupts 0 to 31)
		panic("Unhandled interrupt 0x%.2x at %p", regs->isr_number, regs->eip);
	}

	void Processor::init_idt()
	{
		INIT_INTERRUPT(0x00);
		INIT_INTERRUPT(0x01);
		INIT_INTERRUPT(0x02);
		INIT_INTERRUPT(0x03);
		INIT_TRAP(0x04);
		INIT_INTERRUPT(0x05);
		INIT_INTERRUPT(0x06);
		INIT_INTERRUPT(0x07);
		INIT_TASK(0x08);
		INIT_INTERRUPT(0x09);
		INIT_INTERRUPT(0x0A);
		INIT_INTERRUPT(0x0B);
		INIT_INTERRUPT(0x0C);
		INIT_INTERRUPT(0x0D);
		INIT_INTERRUPT(0x0E); // TODO: Hardware Task Switching messes with returning from the page fault handler // INIT_TASK(0x0E);
		INIT_INTERRUPT(0x0F);
		INIT_INTERRUPT(0x10);
		INIT_INTERRUPT(0x11);
		INIT_INTERRUPT(0x12);
		INIT_INTERRUPT(0x13);
		INIT_INTERRUPT(0x14);
		INIT_INTERRUPT(0x15);
		INIT_INTERRUPT(0x16);
		INIT_INTERRUPT(0x17);
		INIT_INTERRUPT(0x18);
		INIT_INTERRUPT(0x19);
		INIT_INTERRUPT(0x1A);
		INIT_INTERRUPT(0x1B);
		INIT_INTERRUPT(0x1C);
		INIT_INTERRUPT(0x1D);
		INIT_INTERRUPT(0x1E);
		INIT_INTERRUPT(0x1F);

		INIT_INTERRUPT(0x20);
		INIT_INTERRUPT(0x21);
		INIT_INTERRUPT(0x22);
		INIT_INTERRUPT(0x23);
		INIT_INTERRUPT(0x24);
		INIT_INTERRUPT(0x25);
		INIT_INTERRUPT(0x26);
		INIT_INTERRUPT(0x27);

		INIT_INTERRUPT(0x28);
		INIT_INTERRUPT(0x29);
		INIT_INTERRUPT(0x2A);
		INIT_INTERRUPT(0x2B);
		INIT_INTERRUPT(0x2C);
		INIT_INTERRUPT(0x2D);
		INIT_INTERRUPT(0x2E);
		INIT_INTERRUPT(0x2F);

		INIT_INTERRUPT(0x30);
		INIT_INTERRUPT(0x31);
		INIT_INTERRUPT(0x32);
		INIT_INTERRUPT(0x33);
		INIT_INTERRUPT(0x34);
		INIT_INTERRUPT(0x35);
		INIT_INTERRUPT(0x36);
		INIT_INTERRUPT(0x37);
		INIT_INTERRUPT(0x38);
		INIT_INTERRUPT(0x39);
		INIT_INTERRUPT(0x3A);
		INIT_INTERRUPT(0x3B);
		INIT_INTERRUPT(0x3C);
		INIT_INTERRUPT(0x3D);
		INIT_INTERRUPT(0x3E);
		INIT_INTERRUPT(0x3F);

		INIT_INTERRUPT(0x40);
		INIT_INTERRUPT(0x41);
		INIT_INTERRUPT(0x42);
		INIT_INTERRUPT(0x43);
		INIT_INTERRUPT(0x44);
		INIT_INTERRUPT(0x45);
		INIT_INTERRUPT(0x46);
		INIT_INTERRUPT(0x47);
		INIT_INTERRUPT(0x48);
		INIT_INTERRUPT(0x49);
		INIT_INTERRUPT(0x4A);
		INIT_INTERRUPT(0x4B);
		INIT_INTERRUPT(0x4C);
		INIT_INTERRUPT(0x4D);
		INIT_INTERRUPT(0x4E);
		INIT_INTERRUPT(0x4F);

		INIT_INTERRUPT(0x50);
		INIT_INTERRUPT(0x51);
		INIT_INTERRUPT(0x52);
		INIT_INTERRUPT(0x53);
		INIT_INTERRUPT(0x54);
		INIT_INTERRUPT(0x55);
		INIT_INTERRUPT(0x56);
		INIT_INTERRUPT(0x57);
		INIT_INTERRUPT(0x58);
		INIT_INTERRUPT(0x59);
		INIT_INTERRUPT(0x5A);
		INIT_INTERRUPT(0x5B);
		INIT_INTERRUPT(0x5C);
		INIT_INTERRUPT(0x5D);
		INIT_INTERRUPT(0x5E);
		INIT_INTERRUPT(0x5F);

		INIT_INTERRUPT(0x60);
		INIT_INTERRUPT(0x61);
		INIT_INTERRUPT(0x62);
		INIT_INTERRUPT(0x63);
		INIT_INTERRUPT(0x64);
		INIT_INTERRUPT(0x65);
		INIT_INTERRUPT(0x66);
		INIT_INTERRUPT(0x67);
		INIT_INTERRUPT(0x68);
		INIT_INTERRUPT(0x69);
		INIT_INTERRUPT(0x6A);
		INIT_INTERRUPT(0x6B);
		INIT_INTERRUPT(0x6C);
		INIT_INTERRUPT(0x6D);
		INIT_INTERRUPT(0x6E);
		INIT_INTERRUPT(0x6F);

		INIT_INTERRUPT(0x70);
		INIT_INTERRUPT(0x71);
		INIT_INTERRUPT(0x72);
		INIT_INTERRUPT(0x73);
		INIT_INTERRUPT(0x74);
		INIT_INTERRUPT(0x75);
		INIT_INTERRUPT(0x76);
		INIT_INTERRUPT(0x77);
		INIT_INTERRUPT(0x78);
		INIT_INTERRUPT(0x79);
		INIT_INTERRUPT(0x7A);
		INIT_INTERRUPT(0x7B);
		INIT_INTERRUPT(0x7C);
		INIT_INTERRUPT(0x7D);
		INIT_INTERRUPT(0x7E);
		INIT_INTERRUPT(0x7F);

		INIT_INTERRUPT(0x80);
		m_idt[0x80].privilege = 3;

		INIT_INTERRUPT(0x81);
		INIT_INTERRUPT(0x82);
		INIT_INTERRUPT(0x83);
		INIT_INTERRUPT(0x84);
		INIT_INTERRUPT(0x85);
		INIT_INTERRUPT(0x86);
		INIT_INTERRUPT(0x87);
		INIT_INTERRUPT(0x88);
		INIT_INTERRUPT(0x89);
		INIT_INTERRUPT(0x8A);
		INIT_INTERRUPT(0x8B);
		INIT_INTERRUPT(0x8C);
		INIT_INTERRUPT(0x8D);
		INIT_INTERRUPT(0x8E);
		INIT_INTERRUPT(0x8F);

		INIT_INTERRUPT(0x90);
		INIT_INTERRUPT(0x91);
		INIT_INTERRUPT(0x92);
		INIT_INTERRUPT(0x93);
		INIT_INTERRUPT(0x94);
		INIT_INTERRUPT(0x95);
		INIT_INTERRUPT(0x96);
		INIT_INTERRUPT(0x97);
		INIT_INTERRUPT(0x98);
		INIT_INTERRUPT(0x99);
		INIT_INTERRUPT(0x9A);
		INIT_INTERRUPT(0x9B);
		INIT_INTERRUPT(0x9C);
		INIT_INTERRUPT(0x9D);
		INIT_INTERRUPT(0x9E);
		INIT_INTERRUPT(0x9F);

		INIT_INTERRUPT(0xA0);
		INIT_INTERRUPT(0xA1);
		INIT_INTERRUPT(0xA2);
		INIT_INTERRUPT(0xA3);
		INIT_INTERRUPT(0xA4);
		INIT_INTERRUPT(0xA5);
		INIT_INTERRUPT(0xA6);
		INIT_INTERRUPT(0xA7);
		INIT_INTERRUPT(0xA8);
		INIT_INTERRUPT(0xA9);
		INIT_INTERRUPT(0xAA);
		INIT_INTERRUPT(0xAB);
		INIT_INTERRUPT(0xAC);
		INIT_INTERRUPT(0xAD);
		INIT_INTERRUPT(0xAE);
		INIT_INTERRUPT(0xAF);

		INIT_INTERRUPT(0xB0);
		INIT_INTERRUPT(0xB1);
		INIT_INTERRUPT(0xB2);
		INIT_INTERRUPT(0xB3);
		INIT_INTERRUPT(0xB4);
		INIT_INTERRUPT(0xB5);
		INIT_INTERRUPT(0xB6);
		INIT_INTERRUPT(0xB7);
		INIT_INTERRUPT(0xB8);
		INIT_INTERRUPT(0xB9);
		INIT_INTERRUPT(0xBA);
		INIT_INTERRUPT(0xBB);
		INIT_INTERRUPT(0xBC);
		INIT_INTERRUPT(0xBD);
		INIT_INTERRUPT(0xBE);
		INIT_INTERRUPT(0xBF);

		INIT_INTERRUPT(0xC0);
		INIT_INTERRUPT(0xC1);
		INIT_INTERRUPT(0xC2);
		INIT_INTERRUPT(0xC3);
		INIT_INTERRUPT(0xC4);
		INIT_INTERRUPT(0xC5);
		INIT_INTERRUPT(0xC6);
		INIT_INTERRUPT(0xC7);
		INIT_INTERRUPT(0xC8);
		INIT_INTERRUPT(0xC9);
		INIT_INTERRUPT(0xCA);
		INIT_INTERRUPT(0xCB);
		INIT_INTERRUPT(0xCC);
		INIT_INTERRUPT(0xCD);
		INIT_INTERRUPT(0xCE);
		INIT_INTERRUPT(0xCF);

		INIT_INTERRUPT(0xD0);
		INIT_INTERRUPT(0xD1);
		INIT_INTERRUPT(0xD2);
		INIT_INTERRUPT(0xD3);
		INIT_INTERRUPT(0xD4);
		INIT_INTERRUPT(0xD5);
		INIT_INTERRUPT(0xD6);
		INIT_INTERRUPT(0xD7);
		INIT_INTERRUPT(0xD8);
		INIT_INTERRUPT(0xD9);
		INIT_INTERRUPT(0xDA);
		INIT_INTERRUPT(0xDB);
		INIT_INTERRUPT(0xDC);
		INIT_INTERRUPT(0xDD);
		INIT_INTERRUPT(0xDE);
		INIT_INTERRUPT(0xDF);

		INIT_INTERRUPT(0xE0);
		INIT_INTERRUPT(0xE1);
		INIT_INTERRUPT(0xE2);
		INIT_INTERRUPT(0xE3);
		INIT_INTERRUPT(0xE4);
		INIT_INTERRUPT(0xE5);
		INIT_INTERRUPT(0xE6);
		INIT_INTERRUPT(0xE7);
		INIT_INTERRUPT(0xE8);
		INIT_INTERRUPT(0xE9);
		INIT_INTERRUPT(0xEA);
		INIT_INTERRUPT(0xEB);
		INIT_INTERRUPT(0xEC);
		INIT_INTERRUPT(0xED);
		INIT_INTERRUPT(0xEE);
		INIT_INTERRUPT(0xEF);

		INIT_INTERRUPT(0xF0);
		INIT_INTERRUPT(0xF1);
		INIT_INTERRUPT(0xF2);
		INIT_INTERRUPT(0xF3);
		INIT_INTERRUPT(0xF4);
		INIT_INTERRUPT(0xF5);
		INIT_INTERRUPT(0xF6);
		INIT_INTERRUPT(0xF7);
		INIT_INTERRUPT(0xF8);
		INIT_INTERRUPT(0xF9);
		INIT_INTERRUPT(0xFA);
		INIT_INTERRUPT(0xFB);
		INIT_INTERRUPT(0xFC);
		INIT_INTERRUPT(0xFD);
		INIT_INTERRUPT(0xFE);
		INIT_INTERRUPT(0xFF);

		m_idtr.base = (uint32_t)m_idt;
		m_idtr.limit = sizeof(m_idt);

		asm volatile("lidt %0" ::"m"(m_idtr)
		             : "memory");
	}

	void Processor::init_fault_handlers()
	{
		division_fault_handler.register_handler();
		invalid_instruction_handler.register_handler();
		gpf_fault_handler.register_handler();
		page_fault_handler.register_handler();
		syscall_handler.register_handler();
	}

	void Processor::register_interrupt_handler(Interrupts::InterruptHandler &handler)
	{
		enter_critical();
		uint32_t int_num = handler.interrupt_number();

		if (m_handlers[int_num] && m_handlers[int_num]->type() == Interrupts::InterruptType::IRQHandler)
		{
			assert(handler.type() == Interrupts::InterruptType::IRQHandler);

			static_cast<Interrupts::SharedIRQHandler *>(m_handlers[int_num])->add_handler(static_cast<Interrupts::IRQHandler *>(&handler));

			leave_critical();
			return;
		}

		if (handler.type() != Interrupts::InterruptType::IRQHandler)
			m_handlers[int_num] = &handler;
		else
		{
			auto irq_handler = static_cast<Interrupts::IRQHandler *>(&handler);
			uint32_t original_interrupt_number = irq_handler->original_interrupt_number();
			auto shared_handler = new Interrupts::SharedIRQHandler(original_interrupt_number);
			shared_handler->add_handler(irq_handler);
			m_handlers[int_num] = shared_handler;
		}

		leave_critical();
	}

	void Processor::unregister_interrupt_handler(Interrupts::InterruptHandler &handler)
	{
		enter_critical();
		uint32_t int_num = handler.interrupt_number();

		if (m_handlers[int_num]->type() == Interrupts::InterruptType::IRQHandler)
		{
			assert(handler.type() == Interrupts::InterruptType::IRQHandler);

			auto shared_handler = static_cast<Interrupts::SharedIRQHandler *>(m_handlers[int_num]);
			auto irq_handler = static_cast<Interrupts::IRQHandler *>(&handler);
			shared_handler->remove_handler(irq_handler);

			if (shared_handler->empty())
				delete shared_handler;
			else
			{
				leave_critical();
				return;
			}
		}

		m_handlers[int_num] = nullptr;
		leave_critical();
	}

	[[nodiscard]] uint32_t Processor::get_used_interrupt_count() const
	{
		uint32_t count = 0;

		for (size_t i = FIRST_USABLE_INTERRUPT; i < MAX_INTERRUPTS; i++)
		{
			if (m_handlers[i]->type() != Interrupts::InterruptType::UnhandledInterrupt)
				count++;
		}

		return count;
	}

} // namespace Kernel::CPU
