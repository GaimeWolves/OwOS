#ifndef INTERRUPTS_H
#define INTERRUPTS_H 1

#include <libk/kstring.hpp>
#include <libk/kvector.hpp>

#include <panic.hpp>

#ifdef ARCH_i686
#	include <arch/i686/interrupts.hpp>
#endif

namespace Kernel::Processor
{
	typedef void (*irq_ack_t)(int id);
	typedef void (*irqaction_t)(int id, registers_t *regs);

	typedef struct irq_descriptor_t
	{
		irq_ack_t acknowledge;
		LibK::Vector<irqaction_t> actions;
		LibK::String name;
	} irq_descriptor_t;

	void init_interrupts();

	irq_descriptor_t &get_irq(int id);
	bool register_irq(int id, irqaction_t action);

	inline void unhandled_interrupt(int id, registers_t *regs __attribute__((unused)))
	{
		panic("Unhandled Interrupt - %s", get_irq(id).name.c_str());
	}
} // namespace Kernel::Processor

#endif // INTERRUPTS_H