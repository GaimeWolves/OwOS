#ifndef KCSTDARG_H
#define KCSTDARG_H 1

#include <stdarg.h>

namespace Kernel::LibK
{
	template <typename T, typename V>
	struct va_arg_next_argument
	{
		T operator()(V ap) const
		{
			return va_arg(ap, T);
		}
	};
} // namespace Kernel::LibK

#endif // KCSTDARG_H