#pragma once

#include <stdint.h>

#include <libk/kvector.hpp>

namespace Kernel::Processor
{
	typedef struct symbol_t
	{
		uintptr_t address;
		char *name;
		char *file;
	} symbol_t;

	extern LibK::vector<symbol_t> symbol_table;

	void init_stacktracing();
	symbol_t get_symbol_by_address(uintptr_t address);
	void print_stacktrace();
}