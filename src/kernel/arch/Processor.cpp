#include <arch/Processor.hpp>

namespace Kernel::CPU
{
	static bool s_bsp_finished = false;

	bool is_bsp_initialization_finished()
	{
		return s_bsp_finished;
	}

	void set_bsp_initialization_finished()
	{
		s_bsp_finished = true;
	}
}