#include <IO.hpp>

namespace Kernel::IO
{
    void out8(size_t port, uint8_t value)
    {
        asm volatile("outb %0, %1" ::"a"(value), "Nd"((uint16_t)port));
    }

    void out16(size_t port, uint16_t value)
    {
        asm volatile("outw %0, %1" ::"a"(value), "Nd"((uint16_t)port));
    }

    void out32(size_t port, uint32_t value)
    {
        asm volatile("outl %0, %1" ::"a"(value), "Nd"((uint16_t)port));
    }

    uint8_t in8(size_t port)
    {
        uint8_t value;

        asm volatile("inb %1, %0"
                     : "=a"(value)
                     : "Nd"((uint16_t)port));

        return value;
    }

    uint16_t in16(size_t port)
    {
        uint16_t value;

        asm volatile("inw %1, %0"
                     : "=a"(value)
                     : "Nd"((uint16_t)port));

        return value;
    }

    uint32_t in32(size_t port)
    {
        uint32_t value;

        asm volatile("inl %1, %0"
                     : "=a"(value)
                     : "Nd"((uint16_t)port));

        return value;
    }

} // namespace Kernel::IO