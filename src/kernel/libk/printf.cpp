#include "libk/printf.hpp"

namespace Kernel::LibK::__Printf
{
	void parse_number(state_t &state, char *buffer, uint64_t num, size_t base, bool is_ptr)
	{
		static const char *alphabet = "0123456789abcdef";
		size_t iteration = 0;
		size_t written = 0;

		char *current = buffer;

		while (!is_ptr || iteration < sizeof(void *) * 2)
		{
			if (!num && !is_ptr)
			{
				if (written == 0 && state.has_precision && state.precision == 0)
					break;

				if (state.has_precision && written >= (size_t)state.precision)
					break;

				if (!state.has_precision && written > 0)
					break;
			}

			char ch = alphabet[num % base];

			*current++ = ch;
			written++;

			num /= base;
			iteration++;
		}

		strrev(buffer);
	}
}
