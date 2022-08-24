#pragma once

namespace Kernel::Tests
{
	inline const char *get_tag(bool ok) { return ok ? "PASSED" : "FAILED"; }
}