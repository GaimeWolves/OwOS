#include <arch/processor.hpp>

namespace Kernel::Processor
{

    __attribute__((noreturn)) void halt()
    {
        for (;;)
            asm volatile("hlt");
    }

    void clear_interrupts()
    {
        asm volatile("cli");
    }

    void enable_interrupts()
    {
        asm volatile("sti");
    }

} // namespace Kernel::Processor