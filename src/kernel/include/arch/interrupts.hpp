#pragma once

#include <libk/kfunctional.hpp>
#include <libk/kstring.hpp>
#include <libk/kvector.hpp>

#include <common_attributes.h>
#include <panic.hpp>

#ifdef ARCH_i686
#	include <arch/i686/interrupts.hpp>
#endif

namespace Kernel::Interrupts
{
	class InterruptHandler;
} // namespace Kernel::Interrupts


namespace Kernel::Processor
{
	void init_interrupts();

	bool register_interrupt(Interrupts::InterruptHandler *handler);
	bool unregister_interrupt(Interrupts::InterruptHandler *handler);
} // namespace Kernel::Processor
