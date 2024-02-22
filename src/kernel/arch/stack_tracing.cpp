#include <arch/stack_tracing.hpp>

#include <libk/kcstring.hpp>
#include <libk/kcctype.hpp>
#include <panic.hpp>

extern "C"
{
	extern uintptr_t _end_kernel_symbols;
	extern uintptr_t _end_text;
}

namespace Kernel::CPU
{
	// TODO: Somehow check if the actual symbol file exceeds this limit
	__section(".kernel_symbols") static char kernel_symbols[0x70000];

	LibK::vector<symbol_t> symbol_table;

	void init_stacktracing()
	{
		char *current = kernel_symbols;
		size_t count = strtoul(current, &current, 16);
		symbol_table.reserve(count);
		current++;

		while(count-- > 0)
		{
			if ((uintptr_t)current > (uintptr_t)&_end_kernel_symbols)
				panic("Symbol map exceeds section size!");

			uintptr_t address = strtoul(current, &current, 16);
			current++;

			char *name = current;
			current += strlen(current) + 1;

			char *file = nullptr;

			if (!isxdigit(*current))
			{
				file = current;
				current += strlen(current) + 1;
			}

			symbol_table.push_back({
			    .address = address,
			    .name = name,
			    .file = file,
			});
		}
	}

	symbol_t get_symbol_by_address(uintptr_t address)
	{
		for (size_t i = 0; i < symbol_table.size() - 1; i++)
		{
			if (address < symbol_table[i + 1].address && address >= symbol_table[i].address)
			{
				return symbol_table[i];
			}
		}

		if (address <= (uintptr_t)&_end_text)
		{
			return symbol_table[symbol_table.size() - 1];
		}

		return {
			.address = 0,
			.name = nullptr,
			.file = nullptr,
		};
	}
}
