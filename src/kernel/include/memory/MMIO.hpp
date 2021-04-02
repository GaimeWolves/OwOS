#ifndef MMIO_H
#define MMIO_H 1

#include <stddef.h>
#include <stdint.h>

#include <libk/kassert.hpp>

#include <memory/MemoryManager.hpp>

namespace Kernel::Memory
{

    template <typename T>
    class MMIO
    {
    public:
        MMIO() = default;

        MMIO(uintptr_t address, size_t size)
            : m_size(size)
        {
            m_ref = (T *)MemoryManager::instance().map_mmio_region(address, size);
            assert(m_ref);
        }

        ~MMIO()
        {
            MemoryManager::instance().unmap_region((uintptr_t)m_ref, m_size);
        }

        T &operator*() { return *m_ref; }

        T &operator[](int n)
        {
            assert(n >= 0);
            assert(n * sizeof(T) < m_size);
            return m_ref[n];
        }

        T *operator+(int offset)
        {
            assert(offset >= 0);
            assert(offset * sizeof(T) < m_size);
            return &m_ref[offset];
        }

        T *operator()() { return m_ref; }

    private:
        T *m_ref{0};

        size_t m_size{0};
    };

}; // namespace Kernel::Memory

#endif // MMIO_H