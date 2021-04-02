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

    void load_page_directory(uintptr_t page_directory)
    {
        asm volatile("mov %%eax, %%cr3" ::"a"(page_directory)
                     : "memory");
    }

    uintptr_t get_page_directory()
    {
        uintptr_t page_directory = 0;

        asm volatile("mov %%cr3, %%eax"
                     : "=a"(page_directory));

        return page_directory;
    }

    void flush_page_directory()
    {
        load_page_directory(get_page_directory());
    }

    void invalidate_address(uintptr_t address)
    {
        asm volatile("invlpg (%0)" ::"r"(address)
                     : "memory");
    }

} // namespace Kernel::Processor