#include <tests.hpp>

#include <common_attributes.h>
#include <libk/kcstdio.hpp>

namespace Kernel::Tests
{
	static bool constructor_was_run = false;

	__constructor static void test_constructor()
	{
		constructor_was_run = true;
	}

	bool test_crtx()
	{
		LibK::printf_test_msg("Global constructors");

		if (constructor_was_run)
			LibK::printf_check_msg(true, "test_constructor was called");
		else
			LibK::printf_check_msg(false, "test_constructor was not called");

		return constructor_was_run;
	}
} // namespace Kernel::Tests