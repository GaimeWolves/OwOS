#ifndef PROCESSOR_H
#define PROCESSOR_H 1

namespace Kernel::Processor
{

    __attribute__((noreturn)) void halt();

    void clear_interrupts();
    void enable_interrupts();

} // namespace Kernel::Processor

#endif