#pragma once

#include <stddef.h>
#include <stdint.h>

#include <libk/kcassert.hpp>

#include <memory/VirtualMemoryManager.hpp>

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
			m_ref = (T *)VirtualMemoryManager::instance().alloc_mmio_buffer(address, size);
			assert(m_ref);
		}

		~MMIO()
		{
			VirtualMemoryManager::instance().free(m_ref);
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
		alignas(T) T *m_ref{nullptr};
		size_t m_size{0};
	};
} // namespace Kernel::Memory
