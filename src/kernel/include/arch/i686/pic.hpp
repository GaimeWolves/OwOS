#pragma once

namespace Kernel::Processor
{
	void init_pic();
	void pic_eoi(int id);
} // namespace Kernel::Processor
