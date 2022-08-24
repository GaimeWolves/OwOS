#include <tests.hpp>

#include "definitions.hpp"

#include <logging/logger.hpp>
#include <common_attributes.h>

namespace Kernel::Tests
{
	static bool constructor_was_run = false;

	__constructor static void test_constructor()
	{
		constructor_was_run = true;
	}

	bool test_crtx()
	{
		log("TEST", "Global constructors");
		log(get_tag(constructor_was_run), "test_constructor was called");

		return constructor_was_run;
	}
} // namespace Kernel::Tests