#ifndef ARCH_i686_PIC_H
#define ARCH_i686_PIC_H 1

namespace Kernel::Processor
{
	void init_pic();
	void pic_eoi(int id);
} // namespace Kernel::Processor

#endif // ARCH_i686_PIC_H