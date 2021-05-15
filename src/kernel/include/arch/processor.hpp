#pragma once

#include <stddef.h>
#include <stdint.h>

#include <common_attributes.h>

namespace Kernel::Processor
{
	void init();

	__noreturn void halt();
	void sleep();

	void clear_interrupts();
	void enable_interrupts();

	void load_page_directory(uintptr_t page_directory);
	uintptr_t get_page_directory();
	void flush_page_directory();
	void invalidate_address(uintptr_t address);
} // namespace Kernel::Processor
