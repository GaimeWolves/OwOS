#pragma once

#include <libk/kfunctional.hpp>
#include <libk/kstring.hpp>
#include <libk/kvector.hpp>

#include <panic.hpp>
#include <common_attributes.h>

#ifdef ARCH_i686
#	include <arch/i686/interrupts.hpp>
#endif

namespace Kernel::Processor
{
	typedef LibK::function<void(int)> irq_ack_t;
	typedef LibK::function<void(int, registers_t *)> irqaction_t;

	typedef struct irq_descriptor_t
	{
		irq_ack_t acknowledge;
		LibK::vector<irqaction_t> actions;
		LibK::string name;
	} irq_descriptor_t;

	void init_interrupts();

	irq_descriptor_t &get_irq(int id);
	bool register_irq(int id, irqaction_t action);

	inline void unhandled_interrupt(int id, __unused registers_t *regs)
	{
		panic("Unhandled Interrupt - %s", get_irq(id).name.c_str());
	}
} // namespace Kernel::Processor
